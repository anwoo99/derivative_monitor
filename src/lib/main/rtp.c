#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "main.h"
#include "push.h"

#define MAX_PKTN 200

//
// initialize RQ/packet
//
int rt_packet_init(struct rt_pkth *pkth)
{
	struct rt_pktb *pktb;
	char *pktbuff;
	int pktsize;
	int ii;

	pktsize = sizeof(struct rt_pktb) * MAX_PKTN;
	pktbuff = malloc(pktsize);
	if (pktbuff == NULL)
		return (-1);
	memset(pktbuff, 0, pktsize);
	pktb = (struct rt_pktb *)pktbuff;
	for (ii = 0; ii < MAX_PKTN - 1; ii++)
		pktb[ii].next = &pktb[ii + 1];

	memset(pkth, 0, sizeof(struct rt_pkth));
	pkth->pktb = pktbuff;
	pthread_mutex_init(&pkth->alloc.mutex, NULL);
	pthread_cond_init(&pkth->alloc.cond, NULL);
	pthread_mutex_init(&pkth->queue.mutex, NULL);
	pthread_cond_init(&pkth->queue.cond, NULL);

	pkth->pktn[0] = MAX_PKTN;
	pkth->pktn[1] = MAX_PKTN;
	pkth->alloc.pktb = pktb;
	return (0);
}

//
// rt_packet_get()
// Get free packet to use
//
static struct rt_pktb *rt_packet_get(struct rt_pkth *pkth, int timeout)
{
	struct rt_pktb *pktb;
	struct timespec timespec;

	/////////////////////////////////////////
	pthread_mutex_lock(&pkth->alloc.mutex);
	while ((pktb = pkth->alloc.pktb) == NULL)
	{
		pkth->alloc.wait = 1;
		if (timeout > 0)
		{
			timespec.tv_sec = time(0) + timeout;
			timespec.tv_nsec = 0;
			pthread_cond_timedwait(&pkth->alloc.cond, &pkth->alloc.mutex, &timespec);
			if ((pktb = pkth->alloc.pktb) == NULL)
			{
				pthread_mutex_unlock(&pkth->alloc.mutex);
				return (pktb);
			}
		}
		else
			pthread_cond_wait(&pkth->alloc.cond, &pkth->alloc.mutex);
	}
	pkth->alloc.wait = 0;
	pkth->alloc.pktb = pktb->next;
	pkth->pktn[1]--;
	pktb->next = NULL;
	pthread_mutex_unlock(&pkth->alloc.mutex);
	//////////////////////////////////////////
	return (pktb);
}

//
// rt_packet_free()
// Free packet
//
static void rt_packet_free(struct rt_pkth *pkth, struct rt_pktb *pktb)
{

	////////////////////////////////////////
	pthread_mutex_lock(&pkth->alloc.mutex);
	pktb->next = pkth->alloc.pktb;
	pkth->alloc.pktb = pktb;
	pkth->pktn[1]++;
	if (pkth->alloc.wait)
		pthread_cond_signal(&pkth->alloc.cond);
	pthread_mutex_unlock(&pkth->alloc.mutex);
	///////////////////////////////////////////
}

//
// rt_packet_push()
// Set pushmsg to queeu for a reader(fuller)
//
int rt_packet_push(struct rt_pkth *pkth, struct pushmsg *pushmsg, int many)
{
	struct rt_pktb *pktb, *b_pktb;
	int ii;

	for (ii = 0; ii < many; ii++)
	{
		pktb = rt_packet_get(pkth, 2);
		if (pktb == NULL) // Oh !! full packet buffer
			break;		  // Ignore real-time data ... The data will be lost
		/////////////////////////////////////////
		pthread_mutex_lock(&pkth->queue.mutex);
		memcpy(&pktb->pushmsg, &pushmsg[ii], sizeof(struct pushmsg));

		if ((b_pktb = pkth->queue.pktb) == NULL)
		{
			pkth->queue.pktb = pktb;
			pthread_cond_signal(&pkth->queue.cond);
		}
		else
		{
			for (; b_pktb->next != NULL; b_pktb = b_pktb->next)
				;
			b_pktb->next = pktb;
		}
		pthread_mutex_unlock(&pkth->queue.mutex);
		////////////////////////////////////////
	}
	return (0);
}

//
// rt_packet_full()
// Full message from packe buff
//
int rt_packet_full(struct rt_pkth *pkth, struct pushmsg *pushmsg, int howmany, int timeout)
{
	struct rt_pktb *pktb;
	struct timespec timespec;
	int many = 0;

	pthread_mutex_lock(&pkth->queue.mutex);
	while ((pktb = pkth->queue.pktb) == NULL)
	{
		if (pkth->chck)
			break;
		if (timeout > 0)
		{
			timespec.tv_sec = time(0) + timeout;
			timespec.tv_nsec = 0;
			pthread_cond_timedwait(&pkth->queue.cond, &pkth->queue.mutex, &timespec);
			break;
		}
		else
			pthread_cond_wait(&pkth->queue.cond, &pkth->queue.mutex);
	}
	while ((pktb = pkth->queue.pktb) != NULL && many < howmany)
	{
		pkth->queue.pktb = pktb->next;
		pktb->next = NULL;
		memcpy(&pushmsg[many], &pktb->pushmsg, sizeof(struct pushmsg));
		many++;
		rt_packet_free(pkth, pktb);
	}
	pthread_mutex_unlock(&pkth->queue.mutex);
	return (many);
}

//
// rt_packet_sig()
// Wake waiting thread by error
//
void rt_packet_sig(struct rt_pkth *pkth)
{
	pkth->chck = 1;
	pthread_cond_signal(&pkth->queue.cond);
}

//
// rt_packet_reset()
// Reset all packet buffer
//
void rt_packet_reset(struct rt_pkth *pkth)
{
	////////////////////////////////////////
	pthread_mutex_lock(&pkth->alloc.mutex);
	pkth->alloc.pktb = NULL;
	free(pkth->pktb);
	pthread_mutex_unlock(&pkth->alloc.mutex);
	///////////////////////////////////////////

	pthread_mutex_destroy(&pkth->alloc.mutex);
	pthread_cond_destroy(&pkth->alloc.cond);
	pthread_mutex_destroy(&pkth->queue.mutex);
	pthread_cond_destroy(&pkth->queue.cond);
}
