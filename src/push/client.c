#include "rqsvr.h"

#define	MAX_COMMAND_SIZE	(1024*64)
#define	MAX_CLIENTS		20000

extern	pthread_t remote[];
extern	pthread_mutex_t lock4chcker[];

static int bind_client(), accept_client(int);
static int check_command_for_client(struct rq_client *, struct rt_cmd *, int, mask_t *);

static	int	today = -1;
int	no_of_linked = 0;

/*
 * liset_client()
 *  listen client connection request...
 */
int listen_client()
{
	time_t	clock, hhmm;
	int	timeout;
	struct	tm *tm;
	int	lsock, sock;
	int	maxfd, check;
	int	ii;

	if ((lsock = bind_client()) < 0)
		return(-1);
	clock = time(0);
	tm = localtime(&clock);
	today = tm->tm_yday;
	while (1)
	{
		clock = time(0);
		tm = localtime(&clock);
		if (tm->tm_yday != today)
		{
			today = tm->tm_yday;
			if (do_restart)
			{
				hhmm = (tm->tm_hour * 100) + tm->tm_min;
				if (hhmm >= tm_restart)
					return(0);
			}
		}
		board_symbol_to_file();

		pollfd[0].fd = lsock;
		pollfd[0].events = POLLIN | POLLHUP;
		pollfd[0].revents = 0;
		maxfd = 1;
		for (ii = 0; ii < no_of_linked; ii++)
		{
			if (rq_client[ii].sock != ii || rq_client[ii].flag != RQ_LOCAL)
				continue;
			pollfd[maxfd].fd = rq_client[ii].sock;
			pollfd[maxfd].events = POLLIN | POLLHUP;
			pollfd[maxfd].revents = 0;
			maxfd++;
		}
		timeout = 300000;
		check = poll(pollfd, maxfd, timeout);
		if (check <= 0)
		{
			clean_log_file();
			continue;
		}

		for (ii = 1; ii < maxfd; ii++)
		{
			if (pollfd[ii].revents == 0)
				continue;
			sock = pollfd[ii].fd;
			close_client(sock);
		}
		if (pollfd[0].revents != 0)
			accept_client(lsock);
	}
}

/*
 * bind_client()
 * Bind session...
 */
static int bind_client()
{
	struct	sockaddr_in sockaddr_in;
	int	sock, options;

	memset(&sockaddr_in, 0, sizeof(sockaddr_in));
	sockaddr_in.sin_family = AF_INET;
	sockaddr_in.sin_port = htons(rq_port);
	sockaddr_in.sin_addr.s_addr = INADDR_ANY;
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
		return(-1);
	options = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &options, sizeof(int));
	if (bind(sock, (struct sockaddr *)&sockaddr_in, sizeof(sockaddr_in)) != 0)
	{
		close(sock);
		return(-1);
	}
	listen(sock, 256);
	fcntl(sock, F_SETFL, O_NONBLOCK);
	return(sock);
}

/*
 * accept_client()
 * Accept client connection request....
 */
static int accept_client(int lsock)
{
	struct	sockaddr_in sockaddr_in;
	socklen_t socklen;
	int	sock;
	char	*ipad;
	struct	linger linger;

	socklen = sizeof(sockaddr_in);
	sock = accept(lsock, (struct sockaddr *)&sockaddr_in, &socklen);
#ifdef	DEBUG
	dolog(__func__, LL_DEBUG, "socket=%d", sock);
#endif
	if (sock < 0)
		return(0);

	linger.l_onoff = 1;
	linger.l_linger = 0;
	setsockopt(sock, SOL_SOCKET, SO_LINGER, &linger, sizeof(struct linger));

	if (sock >= no_of_clients)
	{
		close(sock);
		dolog(__func__, LL_ERROR, "The rooms for clients are full. please reconfigure for a myrqd");
		return(0);
	}
	memset(rq_client[sock].mask, 0, sizeof(mask_t)*no_of_symbols);
	rq_client[sock].sock = sock;
	ipad = inet_ntoa(sockaddr_in.sin_addr);
	if (strcmp(ipad, LOCAL_HOST) != 0)
	{
		close(sock);
		dolog(__func__, LL_ERROR, "No support remote clients...");
		return(0);
	}

	rq_client[sock].flag = RQ_LOCAL;

	if (write(sock, &sock, sizeof(int)) != sizeof(int))
	{
		rq_client[sock].sock = -1;
		close(sock);
		dolog(__func__, LL_WARNING, "Cannot send connected information to a client");
		return(0);
	}
	fcntl(sock, F_SETFL, O_NONBLOCK);
	rq_client[sock].poll_snd = time(0);
	rq_client[sock].poll_rcv = rq_client[sock].poll_snd;
	if (no_of_linked <= sock)
		no_of_linked = sock + 1;
#ifdef	DEBUG
	dolog(__func__, LL_DEBUG, "allocated socket=%d  maximum clients=%d", sock, no_of_linked);
#endif
	return(0);
}

/*
 * close_client()
 * disconnect client session
 */
int close_client(int sock)
{
	int	 valr;
	int	 ii;

	pthread_mutex_lock(&rq_client[sock].lock);
	if (sock < 0 || rq_client[sock].sock != sock)
	{
		pthread_mutex_unlock(&rq_client[sock].lock);
		return(0);
	}
	rq_client[sock].sock = -1;
	rq_client[sock].pid  = 0;
	rq_client[sock].addr = 0;
	for (ii = 0; ii < no_of_symbols; ii++)
	{
		if (rq_client[sock].mask[ii] == 0)
			continue;
		if (rt_board->hook[ii].many > 0)
		{
			rt_board->hook[ii].many--;
		}
		rq_client[sock].mask[ii] = 0;
	}
	close(sock);

	for (ii = 0, valr = 0; ii < no_of_clients; ii++)
	{
		if (rq_client[ii].sock == ii)
			valr = ii + 1;
	}
	no_of_linked = valr;
#ifdef	DEBUG
	dolog(__func__, LL_DEBUG, "close socket=%d, maximum client record=%d", sock, valr);
#endif
	pthread_mutex_unlock(&rq_client[sock].lock);
	return(0);
}

/*
 * recv_from_client()
 *  Check RTS command from clients
 */
void *recv_from_client(void *args)
{
	char	whoami[40];
	struct	msgbuf *msgbuf;
	struct	rt_cmd *rt_cmd;
	mask_t	*mask_b;
	char	*command_b;
	int	command_l;
	int	sock, many;
	int	thread_n, *thread_p;

	thread_p = args;
	thread_n = *thread_p;
	sprintf(whoami, "recv_from_client(%d)", thread_n);
	if ((command_b = malloc(MAX_COMMAND_SIZE)) == NULL)
	{
		dolog(whoami, LL_ERROR, "No enough memory for command buffer from clients");
		pthread_exit(NULL);
	}
	if ((mask_b = malloc(sizeof(mask_t)*(no_of_symbols+16))) == NULL)
	{
		dolog(whoami, LL_ERROR, "No enough memory for working mask buffer");
		pthread_exit(NULL);
	}
		
	msgbuf = (struct msgbuf *)command_b;
	rt_cmd = (struct rt_cmd *)msgbuf->mtext;
	while (1)
	{
		command_l = msgrcv(rq_qid[1], command_b, MAX_COMMAND_SIZE-8, 0, MSG_NOERROR);
		if (command_l < 0)
			break;
		sock = msgbuf->mtype;
		if (sock < 0 || sock >= no_of_clients || rq_client[sock].sock != sock || rq_client[sock].mask == NULL)
			continue;
		if (rq_client[sock].flag != RQ_LOCAL)
			continue;
		many = command_l / sizeof(struct rt_cmd);
		if (many <= 0)
			continue;
#ifdef	DEBUG
		dolog(whoami, LL_DEBUG, "command from sock=%d, no of symbs=%d", sock, many);
#endif
		pthread_mutex_lock(&rq_client[sock].lock);
		check_command_for_client(&rq_client[sock], rt_cmd, many, mask_b);
		pthread_mutex_unlock(&rq_client[sock].lock);
	}
	pthread_exit(NULL);
}

/*
 * int check_command_for_client()
 * Check client command
 */
static int check_command_for_client(struct rq_client *client, struct rt_cmd *rt_cmd, int many, mask_t *mask_b)
{
	pid_t	*pid;
	in_addr_t addr;
	int	seqn, ii;

	if (client->sock < 0)
		return(0);

	memcpy(mask_b, client->mask, sizeof(mask_t) * no_of_symbols);	// save previous mask value
	client->chck = mask_b;
	for (ii = 0; ii < many; ii++)
	{
		switch (rt_cmd[ii].func)
		{
		case RT_RST_SYMB:
			memset(mask_b, 0, sizeof(mask_t)*no_of_symbols);
		case RT_INS_SYMB:
			if (!rt_cmd[ii].mask)
				break;
			seqn = insert_symbol_to_board(rt_cmd[ii].symb, client, rt_cmd[ii].mask);
			break;
		case RT_SET_MASK:
			seqn = insert_symbol_to_board(rt_cmd[ii].symb, client, rt_cmd[ii].mask);
			if (seqn < 0 || seqn >= no_of_symbols)
				break;
			mask_b[seqn] = rt_cmd[ii].mask;
			break;
		case RT_DEL_MASK:
			seqn = search_symbol_on_board(rt_cmd[ii].symb);
			if (seqn < 0 || seqn >= no_of_symbols)
				break;
			mask_b[seqn] &= ~rt_cmd[ii].mask;
			break;
		case RT_DEL_SYMB:
			seqn = search_symbol_on_board(rt_cmd[ii].symb);
			if (seqn < 0 || seqn >= no_of_symbols)
				break;
			mask_b[seqn] = 0;
			break;
		case RT_SET_PID:
			pid = (pid_t *)rt_cmd[ii].symb;
			memcpy(&addr, &rt_cmd[ii].symb[10], sizeof(in_addr_t));
			client->pid = *pid;
			client->addr = addr;
			break;
		}
	}

	// check removed symbol
	for (ii = 0; ii < no_of_symbols; ii++)
	{
		if (mask_b[ii] == 0 && client->mask[ii] != 0 && rt_board->hook[ii].many > 0)
		{
			rt_board->hook[ii].many--;
		}
		client->mask[ii] = mask_b[ii];
	}
	return(0);
}
