#include "quot.h"
#include <pthread.h>

extern int onhelp;

static RTD *rtd = NULL;

static struct pushmsg pushmsg[80];
static struct pushsymb *pushsymb;
static char pushbuf[1024 * 8];
static int stopped = 0;
static pthread_t real;

static void *poper(void *);
static void realpush(struct pushmsg *, int);

struct realque realque;

//
// rqinit()
//
int rqinit()
{
	initpush();
	pthread_create(&real, NULL, poper, NULL);
	return (0);
}

//
// rqreset()
// Rset realtime symbol
//
void rqreset()
{
	realque.many = 0;
	stopped = 1;
}

//
// rqsymb()
//
void rqsymb(int exid, char *symb, mask_t mask, int seqn)
{
	int nn;

	if ((nn = realque.many) >= MAX_SYMB || mask == 0 || strlen(symb) <= 0)
		return;
	if (delayed)
		mask <<= 16;
	realque.q[nn].exid = exid;
	sprintf(realque.q[nn].code, "%d@%.15s", exid, symb);
	strcpy(realque.q[nn].symb, symb);
	realque.q[nn].mask = mask;
	realque.q[nn].seqn = seqn;
	realque.many++;
}

//
// rqsend()
// Send  real-time symbol to MyMQ/rqdeamon
//
void rqsend()
{
	int ii;

	if (rtd == NULL)
		return;

	pushsymb = (struct pushsymb *)pushbuf;

	for (ii = 0; ii < realque.many; ii++)
	{
		pushsymb->push[ii].func = RT_INS_SYMB;
		pushsymb->push[ii].mask = realque.q[ii].mask;
		strcpy(pushsymb->push[ii].symb, realque.q[ii].code);
	}
	pushsymb->many = realque.many;
	pushsymb->push[0].func = RT_RST_SYMB;
	if (pushsymb->many <= 0)
	{
		pushsymb->push[0].mask = 0;
		pushsymb->push[0].symb[0] = '\0';
		pushsymb->many = 1;
	}
	else
		stopped = 0;
	rtd_send(rtd, pushsymb);
}

//
// pop real-time message
//
static void *poper(void *argv)
{
	int howmany;
	int scrn;
	int ii;
	while (1)
	{
		if (rtd == NULL)
		{
			rtd = rtd_open(0, "");
			if (rtd == NULL)
			{
				sleep(3);
				continue;
			}
		}
		howmany = rtd_recv(rtd, pushmsg, 80);
		if (howmany <= 0 || stopped)
			continue;
		for (ii = 0; ii < howmany; ii++)
		{
			pthread_mutex_lock(&real_lock);
			scrn = atoi(s_scrn);
			realpush(&pushmsg[ii], scrn);
			pthread_mutex_unlock(&real_lock);
		}
	}
	return (NULL);
}

//
// Push real-time data
//
static void realpush(struct pushmsg *pushmsg, int scrn)
{
	FOLDER *folder, myfold;
	int ii;

	for (ii = 0; ii < realque.many; ii++)
	{
		if (strcmp(pushmsg->symb, realque.q[ii].code) != 0 ||
			!(pushmsg->mask & realque.q[ii].mask))
			continue;
		if (fep == NULL)
			continue;
		folder = getfolder(fep, realque.q[ii].symb);
		if (folder == NULL)
			continue;
		memcpy(&myfold, folder, sizeof(FOLDER));
		if ((pushmsg->mask & PUSH_QUOT) || (pushmsg->mask & (PUSH_QUOT << 16)))
			update(&myfold, scrn, 1, realque.q[ii].seqn);
		if ((pushmsg->mask & PUSH_DEPT) || (pushmsg->mask & (PUSH_DEPT << 16)))
			update(&myfold, scrn, 2, realque.q[ii].seqn);
	}
}
