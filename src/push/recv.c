#include "rqsvr.h"

static	uint32_t	pushseqn = 0;

/*
 * recv_from_pusher()
 *  Check command from pusher
 */
void *recv_from_pusher(void *args)
{
	char	whoami[80];
	struct	pushdata *pushdata;
	struct	msgbuf *msgbuf;
	struct	rt_cmd *rt_cmd;
	char	command_b[1024];
	int	command_l;
	char	*msgmsg;
	int	msglen;
	int	thread, *argp;
	int	where;

	argp = args;
	thread = *argp;
	sprintf(whoami, "recv_from_pusher(%d)", thread);
	msgbuf = (struct msgbuf *)command_b;
	rt_cmd = (struct rt_cmd *)msgbuf->mtext;
	while (1)
	{
		command_l = msgrcv(rq_qid[0], command_b, sizeof(command_b), 0, MSG_NOERROR);
		if (command_l < 0)
			break;
		msgmsg = &msgbuf->mtext[sizeof(struct rt_cmd)];
		msglen = command_l - sizeof(struct rt_cmd);

		switch (rt_cmd->func)
		{
		case RT_PUSH_MSG: // rt_cmd->func = pushdata->pushmdg.seqn
			pushdata = (struct pushdata *)command_b;	// pushdata = msgbuf
			where = msgbuf->mtype - 1;			// = pushdata->mkid
			push2sender(where, &pushdata->pushmsg);
			break;
		default:    /* symbol management */
			if (dynamic_mode)
				break;
			switch (rt_cmd->func)
			{
			case RT_INS_SYMB: 
				insert_symbol_to_board(rt_cmd->symb, NULL, 0); 	
				break;
			case RT_DEL_SYMB:
				delete_symbol_from_board(rt_cmd->symb);
				break;
			case RT_RST_SYMB:
				reset_symbol_on_board();
				board_symbol_to_file();
				exit(0);
			case RT_INI_SYMB:
				exit(0);
			default:
				break;
			}
		}
	}
	exit(0);
}

/*
 * push2sender()
 * Push real-time message & send event to sender
 */
void push2sender(int where, struct pushmsg *pushmsg)
{
	struct	rt_alarm *qalarm;
	struct	msgbuf   *msgbuf;
	char	alarmb[128];
	int	ii;

#ifdef	DEBUG
	dolog(__func__, LL_DEBUG, "symbol=%s position=%d", pushmsg->symb, where);
#endif
	
	pushmsg->seqn = pushseqn++;				// func -> seqn

	msgbuf  = (struct msgbuf *)alarmb;
	qalarm = (struct rt_alarm *)msgbuf->mtext;
	qalarm->sidx = where;					// symbol sequencial # (= msgbuf->mtyp)
	qalarm->seqn = pushmsg->seqn;				// sequencail number on fifio queue
	qalarm->indx = rt_fifo->widx++;				// index to FIFO ring buffer
	rt_fifo->widx %= no_of_buffers;				// set next write pointer on fifo ring
	
	memcpy(&rt_fifo->push[qalarm->indx], pushmsg, sizeof(struct pushmsg));
#ifdef	DEBUG
	dolog(__func__, LL_DEBUG, "FIFO=(%d/%d), symbol=%s, mask=0x%04x, type=0x%02x", 
			qalarm->indx, no_of_buffers, pushmsg->symb, pushmsg->mask, pushmsg->type);
#endif
	for (ii = 0; ii < no_of_threads[2]; ii++)
	{
		msgbuf->mtype = ii+1;
		msgsnd(rq_qid[2], msgbuf, sizeof(struct rt_alarm), IPC_NOWAIT|MSG_NOERROR);
	}
}
