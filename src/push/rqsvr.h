#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>
#include "main.h"
#include "push.h"

#define	PUSHER_KEY	(('P' << 24) + ('Q' << 16))
#define	PUSHER_APP	32
#define	PUSHER_IPCK(x)	(PUSHER_KEY + x)

#define	RQ_PKT_SIZE	4080
#define	LOCAL_HOST	"127.0.0.1"
#define	MAX_THREADS	80
#define	MAX_CPUS	512

#define	PACKET_NONE	 0
#define	PACKET_ERROR	-1
#define	PACKET_POLLED	 1
#define	PACKET_MESSAGE	 2

struct	rq_packet {
	int	pkt_chk;				/* checking flow		*/
	int	pkt_len;				/* packet length		*/
	int	pkt_rcv;				/* received packet length	*/
	uint8_t	pkt_buf[RQ_PKT_SIZE];			/* packet buffer		*/
	struct	rq_packet *pkt_next;			/* next packet buffer		*/
};

struct	rq_client {
	int	sock;					/* socket			*/
	int	flag;					/* general flags		*/
#define	RQ_LOCAL	1
#define	RQ_FOREIGN	2
	pid_t	pid;					/* process id			*/
	in_addr_t addr;					/* client address		*/
	mask_t	*mask;					/* event mask(=bitmask.mask)	*/
	mask_t	*chck;					/* event mask for checking	*/
							/* for remote foreign client	*/
	int	mqid;					/* message queue id		*/
	time_t	poll_snd;				/* polling time			*/
	time_t	poll_rcv;				/* poll received time		*/
	pthread_t	sender;				/* thread to send		*/
	pthread_mutex_t lock;				/* for send lock		*/
	struct	rq_packet *packet;			/* packet buffer for remote	*/
};

extern	struct rt_board	 *rt_board;
extern	struct rt_symbol *rt_symbol;	
extern	struct rt_fifo	 *rt_fifo;	

extern	struct pollfd *pollfd;
extern	struct rq_client *rq_client;	
extern	struct seqn2symb *seqn2symb;

extern	int	rq_qid[3];			/* message queue id		*/
extern	int	rq_port;			/* TCP port number		*/

extern	int	no_of_linked;			/* # of valid client's max	*/
extern	int	no_of_clients;			/* # of clients			*/
extern	int	no_of_foreign;			/* # of foreign RQ		*/
extern	int	no_of_threads[];		/* # of threads			*/
extern	int	no_of_symbols;			/* # of symbols			*/
extern	int	no_of_buffers;			/* push packet size		*/
extern	int	dynamic_mode;			/* symbol board control		*/
extern	int	do_restart;			/* auto restart ?		*/
extern	int	tm_restart;			/* time for restarting		*/
extern	int	heartbeat_time;			/* heartbeat time interval	*/
extern	int	private_qsiz[];			/* private qsz for markets	*/
extern	char	symbol_file[];			/* symbol file path		*/
extern	int	logging_level;
extern	char	rq_log_file[];
extern	char	symbol_file[];
extern	char	preload_file[];
extern	char	foreign_delimiter[];

extern	int	n_cpus;
extern	int	cpu_affinity[];

int	config(char *);
int	close_client(int sock);

int	packet_init();
struct  rq_packet *packet_alloc();
int	packet_free();
int 	packet_reset(struct rq_packet *packet);
void 	push2sender(int, pushmsg_t *);

int	init_board();
int	insert_symbol_to_board(char *, struct rq_client *, mask_t);
int	delete_symbol_from_board(char *);
int	reset_symbol_on_board();
int	cleaning_board(int *, int *);
int	search_symbol_on_board(char *);
int	board_symbol_to_file();
int	symbol_file_to_board();
int	symbol_pre_load();

int	init_foreign();
int	send_symbol_to_foreign(const char *, int, int);
int	send_msg_to_foreign(int, char *, int, int);
int	recv_cmd_from_foreign(int, struct rq_packet *, char *);
int	recv_msg_from_foreign(int, struct rq_packet *, const char *);

void 	dolog(const char *, int, const char *format, ... );
void	clean_log_file();
void	xsleep(int);
