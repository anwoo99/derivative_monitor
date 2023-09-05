#include "rqsvr.h"
#include <stdarg.h>
#include <sched.h>
#include <sys/resource.h>

static void reset_alive();

extern	void *recv_from_client(void *);
extern	void *recv_from_pusher(void *), *recv_for_markets(void *);
extern	void *send_to_client(void *);
extern	void *admin_command(void *);

extern	int   listen_client();
extern	int   init_shm();
extern	int   init_msg();

static	int	   sendid[MAX_THREADS], recvid[MAX_THREADS], chckid[MAX_THREADS];
static	pthread_t  sender[MAX_THREADS], recver[MAX_THREADS], chcker[MAX_THREADS];
pthread_t	   remote[MAX_THREADS];
pthread_t	   cadmin;

int	rq_port = RTD_PORT;

/*
 * main()
 *  main procedure for axis system
 */
int main(int argc, char *argv[])
{
	struct	rlimit rlimit;
	pthread_attr_t attr;
	size_t	stacksize;
	int	ii;
#ifdef	LINUX
	cpu_set_t *mask;
	struct	sched_param sp;
	int	cmask;
	int	size;
#endif


	signal(SIGPIPE, SIG_IGN);
	signal(SIGTERM, exit);
	if (argc > 1)
		rq_port = atoi(optarg);

	config(argv[0]);
	if (init_shm() != 0 || init_msg() != 0 || init_board() != 0 || packet_init() != 0)
		return(0);

#if 0
#ifdef	LINUX
	if (n_cpus > 0)
	{
		mask = CPU_ALLOC(MAX_CPUS);
		size = CPU_ALLOC_SIZE(MAX_CPUS);
		CPU_ZERO_S(size, mask);
		for (ii = 0, cmask = 0; ii < n_cpus; ii++)
		{
			if (cpu_affinity[ii] < 0 || cpu_affinity[ii] > MAX_CPUS)
				continue;
			CPU_SET(cpu_affinity[ii], mask);
			cmask++;
		}
		if (cmask)
			sched_setaffinity(getpid(), size, mask);
	}
	sp.sched_priority = sched_get_priority_max(SCHED_FIFO);
	sched_setscheduler(0, SCHED_FIFO, &sp);
#endif
#endif

	getrlimit(RLIMIT_NOFILE, &rlimit);
	if (rlimit.rlim_cur < no_of_clients)
	{
		rlimit.rlim_cur = no_of_clients + 80;
		if (rlimit.rlim_cur > rlimit.rlim_max)
		{
			rlimit.rlim_cur =  rlimit.rlim_max;
			dolog(__func__, LL_WARNING, "The number of files to be open is less than number of clients");
		}
		setrlimit(RLIMIT_NOFILE, &rlimit);
	}
	atexit(reset_alive);

	//
	// If preload file is configured, load symbol file
	//
	symbol_pre_load();

#if	defined (HPUX)
	pthread_attr_create(&attr);
	stacksize = pthread_attr_getstacksize(&attr);
#else
	pthread_attr_init(&attr);
	pthread_attr_getstacksize(&attr, &stacksize);
#endif
	if (stacksize < (512*1024))
	{
		stacksize = 512 * 1024;
		pthread_attr_setstacksize(&attr, stacksize);
	}
	//
	// create threads to receive Realtime/SYMBOL information from client
	//
	for (ii = 0; ii < no_of_threads[0]; ii++)
	{
		chckid[ii] = ii;
		if (pthread_create(&chcker[ii], &attr, recv_from_client, &chckid[ii]) != 0)
			exit(0);
		pthread_detach(chcker[ii]);
	}
	//
	// create threads receive Realtime/MESSAGE from pusher
	//
	for (ii = 0; ii < no_of_threads[1]; ii++)
	{
		recvid[ii] = ii;
		if (pthread_create(&recver[ii], &attr, recv_from_pusher, &recvid[ii]) != 0)
			exit(0);
		pthread_detach(recver[ii]);
	}

	//
	// create threds to send Realtime/MESSAGE to client
	//
	for (ii = 0; ii < no_of_threads[2]; ii++)
	{
		sendid[ii] = ii;
		if (pthread_create(&sender[ii], &attr, send_to_client, &sendid[ii]) != 0)
			exit(0);
		pthread_detach(sender[ii]);
	}
	if (pthread_create(&cadmin, &attr, admin_command, NULL) == 0)
		pthread_detach(cadmin);

	dolog(__func__, LL_MUST, "foxpush initialized, %d clients, %d symbols", no_of_clients, no_of_symbols);
	listen_client();
	return(0);
}

//
// xsleep()
// usleep for thread-safe
//
void xsleep(int many)
{
	struct	timeval timeval;

	timeval.tv_sec = 0;
	timeval.tv_usec = many;
	select(0, NULL, NULL, NULL, &timeval);
}


//
// reset_alive()
// Rset alive mark
//
static void reset_alive()
{
	if (rt_board != NULL)
		rt_board->alive = 0;
}

//
// clean_log_file()
// Remove old log file
//
void clean_log_file()
{
	static	int  xday = -1;
	time_t	clock;
	struct 	tm *tm;
	char	logpath[128];
	int	ii;

	if (strlen(rq_log_file) <= 0)
		return;
	clock = time(0);
	tm = localtime(&clock);
	if (tm->tm_yday == xday)
		return;
	xday = tm->tm_yday;
	clock -= (10*24*60*60);
	for (ii = 0; ii < 30; ii++, clock -= (24*60*60))
	{
		tm = localtime(&clock);
		sprintf(logpath, "%s%04d%02d%02d", rq_log_file, tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday);
		remove(logpath);
	}
}


//
// dolog()
// Trace logging
//
void dolog(const char *whoami, int level, const char *format, ... )
{
	char	caller[80];
	char	logpath[128];
	char	logmsg[256];
	FILE	*logfile;
	time_t	clock;
	struct	tm *tm;
	va_list	vl;
	int	tty;

	if (!logging_level || level > logging_level)
		return;

	clock = time(0);
	tm    = localtime(&clock);
	logfile = NULL;

	if (strlen(rq_log_file) > 0)
	{
		sprintf(logpath, "%s%04d%02d%02d", rq_log_file, tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday);
		logfile = fopen(logpath, "a");
	}
	tty = isatty(1);
	if (logfile == NULL && tty <= 0)
		return;

	if (strstr(whoami, "(") == NULL)
		sprintf(caller, "%s()", whoami);
	else
		strcpy(caller, whoami);

	if (logging_level > 1)
		sprintf(logmsg, "%02d/%02d %02d:%02d:%02d %s ", tm->tm_mon+1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, caller);
	else
		sprintf(logmsg, "%02d/%02d %02d:%02d:%02d ", tm->tm_mon+1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
	va_start(vl, format);
	vsprintf(&logmsg[strlen(logmsg)], format, vl);
	va_end(vl);
	
	if (logfile != NULL)
	{
		fprintf(logfile, "%s\n", logmsg);
		fclose(logfile);
	}
	if (tty == 1)
		fprintf(stdout, "%s\n", logmsg);
}
