#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "main.h"
#include "push.h"

static struct rt_board *__rt_board = NULL;
static struct rt_fifo *__rt_fifo = NULL;
static struct rt_symbol *__rt_symbol = NULL;

static void *rt_recver(void *argv);
static int rt_connect(RTIS *, int);
static int rt_attach();
static int _cmp_symb();

//
// attcch_shm()
// Attach shared memory
//
static int rt_attach()
{
	int shmidx;
	char *shmad;

	if (__rt_board == NULL || !__rt_board->alive)
	{
		if (__rt_board != NULL)
			shmdt(__rt_board);
		__rt_board = NULL;
		__rt_fifo = NULL;
		__rt_symbol = NULL;
	}
	if (__rt_board != NULL)
		return (0);

	if ((shmidx = shmget(RTD_IPCK(0), 0, 0666)) >= 0)
	{
		if ((shmad = shmat(shmidx, (char *)0, SHM_RDONLY)) != (char *)-1)
		{
			__rt_board = (struct rt_board *)shmad;
			__rt_symbol = (struct rt_symbol *)&shmad[__rt_board->symbs];
			__rt_fifo = (struct rt_fifo *)&shmad[__rt_board->fifos];
		}
	}
	return (0);
}

//
// rtd_open()
// Open Realtime interface
//
RTD *rtd_open(int rq_port, const char *ipad)
{
	RTD *rtd;
	RTIS *self;
	struct pushsymb pushsymb;
	int port;
	pid_t pid;
	in_addr_t addr;

	// get RQ message broker port number
	if ((rtd = malloc(sizeof(RTD) + sizeof(RTIS))) == NULL)
		return (NULL);
	self = (RTIS *)&rtd[1];
	memset(rtd, 0, sizeof(RTD));
	memset(self, 0, sizeof(RTIS));
	self->sock = -1;
	rtd->flag = 1;	  // valid descriptor
	rtd->self = self; // myself
	if ((port = rq_port) <= 0)
		port = RTD_PORT;
	if (rt_connect(self, port) != 0)
	{
		rtd_close(rtd);
		errno = EIO;
		return (NULL);
	}

	if (rt_attach() != 0 || __rt_board == NULL)
	{
		rtd_close(rtd);
		errno = ESRCH;
		return (NULL);
	}
	self->rt_board = __rt_board;
	self->rt_fifo = __rt_fifo;

	// open socket with RQ message broker foxpush
	self->flag = _OPEN_;
	self->alive = 1;
	signal(SIGPIPE, SIG_IGN);

	// send my process id to RQ message broker
	pid = getpid();
	addr = inet_addr(ipad);
	memset(&pushsymb, 0, sizeof(pushsymb));
	pushsymb.many = 1;
	pushsymb.push[0].func = RT_SET_PID;
	memcpy(pushsymb.push[0].symb, &pid, sizeof(pid_t));
	memcpy(&pushsymb.push[0].symb[0], &addr, sizeof(in_addr_t));

	rtd_send(rtd, &pushsymb);

	if (rt_packet_init(&self->rt_pkth) != 0)
	{
		rtd_close(rtd);
		errno = ENOMEM;
		return (NULL);
	}
	self->flag |= _PKTB_; // packet initialized...
	if (pthread_create(&self->rcver, NULL, rt_recver, self) != 0)
	{
		rtd_close(rtd);
		return (NULL);
	}
	self->flag |= _RECV_;
	return (rtd);
}

//
// rtd_send()
//
int rtd_send(RTD *rtd, struct pushsymb *pushsymb)
{
	RTIS *self = rtd->self;
	int size, many, retc;

	if (self->alive)						 // if alive sessiob
		self->alive = self->rt_board->alive; // realtime board is alive also ?
	if (!self->alive)
	{
		errno = ENOENT;
		return (-1);
	}
	if ((many = pushsymb->many) <= 0)
		return (0);
	size = sizeof(pushsymb->push[0]) * many;
	pushsymb->many = self->rqid; // this is msgbuf->mtype, this is me on foxpush
	retc = msgsnd(self->rt_board->mqid[1], (char *)pushsymb, size, IPC_NOWAIT);
	pushsymb->many = many; // resotre mtype to no of symbol to be checked
	return (retc);
}

//
// _rtd_recv()
//
static int _rtd_recv(RTD *rtd, struct pushmsg *pushmsg, int howmany, int timeout)
{
	RTIS *self = rtd->self;
	int many;

	if (!self->alive || !self->rt_board->alive)
	{
		errno = ENOENT;
		return (-1);
	}
	if ((many = rt_packet_full(&self->rt_pkth, pushmsg, howmany, timeout)) <= 0)
	{
		if (!self->alive)
		{
			errno = ENOENT;
			return (-1);
		}
		return (0);
	}
	return (many);
}

int rtd_recv(RTD *rtd, struct pushmsg *pushmsg, int howmany)
{
	return (_rtd_recv(rtd, pushmsg, howmany, 0));
}

int rtd_timedrecv(RTD *rtd, struct pushmsg *pushmsg, int howmany, int timeout)
{
	return (_rtd_recv(rtd, pushmsg, howmany, timeout));
}

//
// recv thread : receive real tiime message from RQ message broker
//
static void *rt_recver(void *argv)
{
	RTIS *self = argv;
	char recv_b[1024], alarm_b[80];
	int recv_i, recv_l, alarm_l;
	struct rt_alarm *rt_alarm = (struct rt_alarm *)alarm_b;
	struct pushmsg pushmsg[40];
	int many;

	alarm_l = 0;
	while (1)
	{
		recv_l = read(self->sock, recv_b, sizeof(recv_b));
		if (recv_l <= 0)
		{
			if (recv_l < 0 && (errno == EINTR || errno == EWOULDBLOCK))
				continue;
			break;
		}
		many = 0;
		for (recv_i = 0; recv_i < recv_l; recv_i++)
		{
			alarm_b[alarm_l++] = recv_b[recv_i];
			if (alarm_l < sizeof(struct rt_alarm))
				continue;

			// 1 record receive completed....
			alarm_l = 0;
			if (rt_alarm->sidx != ALARM_ID)
			{
				// Sorry !!! maybe broken connection.....
				many = -1;
				break;
			}
			if (rt_alarm->seqn != self->rt_fifo->push[rt_alarm->indx].seqn)
				continue;
			memcpy(&pushmsg[many++], &self->rt_fifo->push[rt_alarm->indx], sizeof(struct pushmsg));
			if (many >= 40)
			{
				rt_packet_push(&self->rt_pkth, pushmsg, many);
				many = 0;
			}
		}
		if (many < 0)
			break;
		if (many > 0)
			rt_packet_push(&self->rt_pkth, pushmsg, many);
	}
	self->alive = 0;
	rt_packet_sig(&self->rt_pkth);

	// wait to cancel from parent thread......
	while (1)
		sleep(60);
	pthread_exit(NULL);
}

//
// rtd_close()
// Close RQ interface
//
int rtd_close(RTD *rtd)
{
	RTIS *self;

	if (rtd == NULL)
		return (0);
	if (!rtd->flag)
	{
		free(rtd);
		return (0);
	}
	self = rtd->self;
	if (self->flag & _OPEN_)
	{
		if (self->sock >= 0)
		{
			shutdown(self->sock, 2);
			close(self->sock);
		}
	}
	self->sock = -1;
	self->rqid = -1;
	self->rt_board = NULL;
	self->rt_fifo = NULL;
	if (self->flag & _RECV_)
		pthread_cancel(self->rcver);
	if (self->flag & _PKTB_)
		rt_packet_reset(&self->rt_pkth);
	self->flag = 0;
	free(rtd);
	return (0);
}

//
// rt_connect()
// connect Real-time server
//
static int rt_connect(RTIS *self, int port)
{
	struct sockaddr_in sockaddr_in;
	struct pollfd pollfd;
	int sock, opts, peer;

	memset(&sockaddr_in, 0, sizeof(struct sockaddr_in));
	sockaddr_in.sin_family = AF_INET;
	sockaddr_in.sin_port = htons(port);
	sockaddr_in.sin_addr.s_addr = inet_addr(LOCAL_HOST);
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
		return (-1);

	if (connect(sock, (struct sockaddr *)&sockaddr_in, sizeof(struct sockaddr_in)) != 0)
	{
		close(sock);
		return (-1);
	}
	opts = 1;
	setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &opts, sizeof(opts));
	for (;;)
	{
		pollfd.fd = sock;
		pollfd.events = POLLIN | POLLHUP;
		pollfd.revents = 0;
		if (poll(&pollfd, 1, 5000) <= 0)
		{
			close(sock);
			return (-1);
		}
		if (read(sock, (char *)&peer, sizeof(int)) == sizeof(int))
			break;
		close(sock);
		return (-1);
	}
	self->sock = sock;
	self->rqid = peer;
	return (0);
}

static int initflag = 0;

//
// RQ shared memory initializer
//
static void *rt_init(void *argp)
{
	while (1)
	{
		rt_attach();
		sleep(3);
	}
	pthread_exit(NULL);
}

//
// rtd_init()
// Initialize RTD interface
//
static int rtd_init()
{
	pthread_t i_thread;

	rt_attach();
	if (pthread_create(&i_thread, NULL, rt_init, NULL) != 0)
		return (1);
	return (1);
}

int rtd_push(struct pushdata *pushdata)
{
	struct pushmsg *pushmsg = &pushdata->pushmsg;
	int pushlen;
	struct symb s_symb, *f_symb;
	int seqn, mqid;

	if (!initflag)
		initflag = rtd_init();
	if (pushmsg->msgl > MAX_PUSH_LEN)
	{
		errno = E2BIG;
		return (-1);
	}
	if (__rt_board == NULL)
		return (0);
	pushmsg->seqn = RT_PUSH_MSG; // = rq_cmd->func
	pushlen = sizeof(struct pushmsg) - MAX_PUSH_LEN + pushmsg->msgl;
	errno = ESRCH;
	strcpy(s_symb.symb, pushmsg->symb);
	f_symb = (struct symb *)bsearch(&s_symb, __rt_symbol->symb, __rt_symbol->valr, sizeof(__rt_symbol->symb[0]), _cmp_symb);

	if (f_symb == NULL)
		return (0);

	seqn = f_symb->seqn;
	if (seqn < 0 || seqn >= __rt_board->no_of_symbols)
		return (-1);
	if (!f_symb->mark || !__rt_board->hook[seqn].many)
		return (0);
	pushdata->mkid = seqn + 1;
	mqid = __rt_board->mqid[0];
	msgsnd(mqid, pushdata, pushlen, IPC_NOWAIT);
	return (0);
}

static int _cmp_symb(struct symb *s1, struct symb *s2)
{
	return (strcmp(s1->symb, s2->symb));
}
