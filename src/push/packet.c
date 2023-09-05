#include "rqsvr.h"

static	pthread_mutex_t		packet_mutex;
static	struct	rq_packet	free_packet;

//
// initialize packet
//
int packet_init()
{
	pthread_mutex_init(&packet_mutex, NULL);
	return(0);
}

//
// packet_alloc()
// Allocate packet
//
struct rq_packet *packet_alloc()
{
	struct	rq_packet *n_packet;

	pthread_mutex_lock(&packet_mutex);
	if (free_packet.pkt_next != NULL)
	{
		n_packet = free_packet.pkt_next;
		free_packet.pkt_next = n_packet->pkt_next;
		n_packet->pkt_next = NULL;
	}
	else
		n_packet = (struct rq_packet *)malloc(sizeof(struct rq_packet));
	pthread_mutex_unlock(&packet_mutex);
	if (n_packet == NULL)
		dolog(__func__, LL_WARNING, "no enough memory for packet");
	else
		memset(n_packet, 0, sizeof(struct rq_packet));
	packet_reset(n_packet);
	return(n_packet);
}

//
// packet_reset()
// Rset packet
//
int packet_reset(struct rq_packet *packet)
{
	if (packet != NULL)
	{
		packet->pkt_chk = 0;
		packet->pkt_rcv = 0;
		packet->pkt_len = 0;
	}
	return(0);
}

//
// packet_free()
// Free packet
//
int packet_free(struct rq_packet *packet)
{
	if (packet == NULL)
		return(0);
	pthread_mutex_lock(&packet_mutex);
	packet->pkt_next = free_packet.pkt_next;
	free_packet.pkt_next = packet;
	pthread_mutex_unlock(&packet_mutex);
	return(0);
}
