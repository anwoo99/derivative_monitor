#include "rqsvr.h"

extern	int	msg_que_size[];

struct	pollfd	  *pollfd;			/* for connextions checking	*/
struct	rt_board  *rt_board;			/* header of shared memory	*/
struct	rt_symbol *rt_symbol;			/* MyRQ symbol		*/
struct	rt_fifo   *rt_fifo;			/* MyRQ fifo ring buffer	*/
struct	rq_cross  *rq_cross;			/* cross network RQ demon	*/
struct	rq_client *rq_client;			/* client information		*/
int	rq_qid[3];				/* 0: push (by cooker)		*/
						/* 1: push (by client)		*/
						/* 2: checker			*/
/*
 * init_shm()	
 * Initialize MyRQ  system
 */
int init_shm()
{
	int	shmidx, shmsiz;
	struct	shmid_ds shmid_ds;
	char	*shmad, *maskb;
	int	hsize, usize, ssize, qsize;
	struct	rt_symbol rt_symb;
	struct	rt_fifo	  xq_fifo;
	int	ii;

	no_of_clients += 80;			/* for room, internal file	*/

	hsize = sizeof(struct rt_board) + ((no_of_symbols + 128) * sizeof(struct rt_hooked));
	ssize = sizeof(rt_symb) + (sizeof(rt_symb.symb[0]) * no_of_symbols);	
	qsize = sizeof(xq_fifo) + (sizeof(xq_fifo.push[0]) * no_of_buffers);
	shmsiz = hsize + ssize + qsize;

	usize = sizeof(struct rq_client) * no_of_clients;
	rq_client = (struct rq_client *)malloc(usize);
	if (rq_client == NULL)
	{
		dolog(__func__, LL_ERROR, "No enough memory for client's room ");
		return(-1);
	}
	memset(rq_client, 0, usize);
	pollfd = (struct pollfd *)malloc(sizeof(struct pollfd) * no_of_clients);
	if (pollfd == NULL)
	{
		dolog(__func__, LL_ERROR, "No enough memory for client's room ");
		return(-1);
	}

	shmidx = shmget(PUSHER_IPCK(0), 0, 0666);
	if(shmidx >= 0)
	{
		shmctl(shmidx, IPC_STAT, &shmid_ds);
		if (shmid_ds.shm_segsz != shmsiz)
		{
			if ((shmad = shmat(shmidx, (char *)0, 0)) != (char *)-1)
			{
				rt_board = (struct rt_board *)&shmad[0];
				rt_board->alive = 0;
				shmdt(shmad);
			}
			shmctl(shmidx, IPC_RMID, &shmid_ds);
		}
	}
	if ((shmidx = shmget(PUSHER_IPCK(0), shmsiz, 0666|IPC_CREAT)) < 0)
	{
		dolog(__func__, LL_ERROR, "Cannot create shared memory for MyRQ");
		return(-1);
	}
	if ((shmad = shmat(shmidx, (char *)0, 0)) == (char *)-1)
	{
		dolog(__func__, LL_ERROR, "Cannot attach shared memory for MyRQ");
		return(-1);
	}

	memset(shmad, 0, shmsiz);

	rt_board = (struct rt_board *)&shmad[0];
	rt_symbol = (struct rt_symbol *)&shmad[hsize];
	rt_fifo = (struct rt_fifo *)&shmad[hsize+ssize];

	rt_board->symbs = hsize;
	rt_board->fifos = hsize + ssize;

	rt_board->svcp = rq_port;
	rt_board->dynamic_mode  = dynamic_mode;
	rt_board->no_of_clients = no_of_clients;
	rt_board->no_of_symbols = no_of_symbols;

	rt_symbol->maxr = no_of_symbols;
	rt_fifo->maxr = no_of_buffers;
	rt_board->rqpid = getpid();

	maskb = malloc(no_of_clients * no_of_symbols * sizeof(mask_t));
	if (maskb == NULL)
	{
		dolog(__func__, LL_ERROR, "No enough memory for %d clients", no_of_clients);
		return(-1);
	}
	
	//
	// attach mask buff to client
	//
	qsize = no_of_symbols * sizeof(mask_t);
	for (ii = 0, usize = 0; ii < no_of_clients; ii++, usize += qsize)
	{
		rq_client[ii].sock = -1;
		rq_client[ii].mask = (mask_t *)&maskb[usize];
		pthread_mutex_init(&rq_client[ii].lock, NULL);
	}
	rt_board->alive = 1;
	return(0);
}

/*
 * init_msg()
 * Initialize message queue
 */
int init_msg()
{
	struct	msqid_ds msqid_ds;
	char	clean_b[32];
	int	clean_l;
	int	ii;

	for (ii = 0; ii < 3; ii++)
	{
		rq_qid[ii] = msgget(RTD_IPCK(ii), 0666|IPC_CREAT);
		if (rq_qid[ii] < 0)
			return(-1);
	
		while (1)
		{	
			clean_l = msgrcv(rq_qid[ii], clean_b, 10, 0, IPC_NOWAIT|MSG_NOERROR);
			if (clean_l < 0)
				break;
		}	
		if (msgctl(rq_qid[ii], IPC_STAT, &msqid_ds) != 0)
			continue;

		if (msg_que_size[ii] != 0 && msqid_ds.msg_qbytes != msg_que_size[ii])
		{
			msqid_ds.msg_qbytes = msg_que_size[ii];
			msgctl(rq_qid[ii], IPC_SET, &msqid_ds);
		}
	}

	rt_board->mqid[0] = rq_qid[0];
	rt_board->mqid[1] = rq_qid[1];
	rt_board->mqid[2] = rq_qid[2];
	return(0);
}
