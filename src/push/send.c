#include "rqsvr.h"


/*
 * send_to_client()
 *  Send RQ message to clients
 */
void *send_to_client(void *args)
{
	char	whoami[32];
	struct	msgbuf	*msgbuf;
	char	b_push[128];
	struct	pushmsg pushmsg;
	struct	rt_alarm *qalarm;
	int	l_push,  l_send, s_seqn, e_mask;
	int	mythrd, *argp;
	int	ii;

	argp   = args;
	mythrd = *argp;
	sprintf(whoami, "%s(%d)", __func__, mythrd);
	msgbuf  = (struct msgbuf *)b_push;
	while (1)
	{
		l_push = msgrcv(rq_qid[2], b_push, sizeof(b_push), mythrd+1, MSG_NOERROR);
		if (l_push < 0)
		{
			if (errno == EINTR || errno == EAGAIN)
				continue;
			break;
		}
		if (l_push < sizeof(struct rt_alarm))
			continue;
		qalarm = (struct rt_alarm *)msgbuf->mtext;		// alarm message buffer
		memcpy(&pushmsg, &rt_fifo->push[qalarm->indx], sizeof(struct pushmsg));	
		e_mask = rt_fifo->push[qalarm->indx].mask;		// event mask
		s_seqn = qalarm->sidx;					// symbol seq #
		if (s_seqn < 0 || s_seqn >= no_of_symbols)
			continue;
#ifdef	DEBUG
		dolog(whoami, LL_DEBUG, "wakeuped thrd# %d symbol=(%s,%d,%04x)  clients=%d/%d", mythrd,
			rt_fifo->push[qalarm->indx].symb, s_seqn, e_mask, no_of_linked, no_of_clients);
#endif
		qalarm->sidx = ALARM_ID;
		for (ii = mythrd; ii < no_of_linked; ii += no_of_threads[2])
		{
			if (rq_client[ii].sock != ii || rq_client[ii].mask == NULL || !(rq_client[ii].mask[s_seqn] & e_mask))
				continue;
#ifdef	DEBUG
			dolog(whoami, LL_DEBUG, "alarm to client sock=%d", ii);
#endif
			switch (rq_client[ii].flag)
			{
			case RQ_LOCAL:
				l_send = write(ii, qalarm, sizeof(struct rt_alarm));
				if (l_send == sizeof(struct rt_alarm) || (l_send == -1 && errno == EWOULDBLOCK))
					break;
				close_client(ii);
				break;
			case RQ_FOREIGN:
				if (rq_client[ii].mqid >= 0)
				{
					msgbuf->mtype = 1;
					msgsnd(rq_client[ii].mqid, msgbuf, sizeof(struct rt_alarm), IPC_NOWAIT|MSG_NOERROR);
				}
				break;
			}
		}
	}
	exit(0);
}
