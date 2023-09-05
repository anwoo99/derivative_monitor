#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <libgen.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <curses.h>
#include <term.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "fep.h"
#include "push.h"

#define	MAX_SYMBOLS	10000

int 	shm4myrq();
int 	view4symb(char *symb);
int 	view4qstat();
int 	trace4rts(char *symb);
int	cmp_symb();

#define	QVIEW	1
#define	SVIEW	2
#define	TRACE	3


struct	rt_board  *rt_board;
struct	rt_symbol *rt_symbol;
struct	rt_fifo   *rt_fifo;
int	hook;
int	allsymb;
int	longmode;
int	maxcol = 80;

static int usage(const char *command)
{
	printf("\n");
	if (strcmp(command, "fepsymb") == 0)
	{
		printf("Usage : %s [OPTIONs] [SYMBOL]\n", command);
		printf("  -a             Display no active symbols also\n");
		printf("  -A             Display deleted symbols also\n");
		printf("  -l             Display sysmbol's status with long format\n");

		printf("\n");

		printf("\n");
		printf("%s -t [symbol]\n", command);
		printf("  -t             Trace report all REAL-TIME message data\n");
		printf("  -t symbol      Trace report REAL-TIME message data for a specified symbol\n");
	}
	else
	{
		printf("Usage : %s [OPTIONs]\n", command);
		printf("  -l             Display with long format\n");
	}
	printf("\n");
	return(0);
}

/*
 * NAME	: main()
 */
int main(int argc, char *argv[])
{
	char	*whoami;
	int	options;
	char	symb[32];
	int	isls = 0;
	int	func;

	whoami = basename(argv[0]);
	if (shm4myrq() != 0)
		return(-1);

	signal(SIGINT, exit);
	
	func = QVIEW;
	if (strcmp(whoami, "fepsymb") == 0)
	{
		isls = 1;
		func = SVIEW;
	}

	symb[0] = '\0';
	// opterr = 0;
	optind = 1;
	
	while((options = getopt(argc, argv, "aAstl?")) != EOF)
	{
		switch (options)
		{
		case 's': /* view symbols */
			func = SVIEW;
			break;
		case 'a': 
			if (isls)
				allsymb |= 0x01;
			break;
		case 'A':
			if (isls)
				allsymb |= 0x02;
			break;
		case 't': /* trace */
			if (isls)
				func = TRACE;
			break;
		case 'l':
			longmode = 1;
			break;
		case '?':
			return(usage(basename(argv[0])));
		default:
			break;
		}
	}

	if (optind > 0 && optind < argc)
		strcpy(symb, argv[optind]);
	switch (func)
	{
	case QVIEW: view4qstat();	break;
	case SVIEW: view4symb(symb); 	break;
	case TRACE: trace4rts(symb);	break;
	}
	return(0);
}

/*
 * view4qstat()
 * View realtime queue status
 */
int view4qstat()
{
	struct	msqid_ds qstat;
	int	total, used, reusable, attached, freed;
	float	size;
	int	ii;

	total = rt_board->no_of_symbols;
	used =  rt_symbol->valr;
	freed = rt_symbol->maxr - rt_symbol->valr;
	reusable = 0;
	attached = 0;

	for (ii = 0; ii < used; ii++)
	{
		if (rt_symbol->symb[ii].mark)
			attached++;
		else
			reusable++;
	}
	// reusable -= used;
	if (longmode)
	{
		printf("%d allocated, %d activated, %d usable(%d free, %d reusable)\n",  total, attached, freed+reusable, freed, reusable);
		printf("\n");
	}
	printf("%8.8s %10.10s %8.8s %8.8s %8.8s   MARKETs\n",
		"SIDE", "QID", "QSIZE", "QNUM", "CBYTES");
	for (ii = 0; ii < 3; ii++)
	{
		switch (ii)
		{
		case 0: printf("%8s ", "data"); break;
		case 1: printf("%8s ", "symb"); break;
		case 2: printf("%8s ", "ctrl"); break;
		}
		printf("%10d ", rt_board->mqid[ii]);
		if (msgctl(rt_board->mqid[ii], IPC_STAT, &qstat) != 0)
		{
			printf("ERR\n");
			continue;
		}
		size = qstat.msg_qbytes / 1024;
		printf("%7.fK %8ld %8ld   --", size, qstat.msg_qnum, qstat.msg_cbytes);
		printf("\n");
	}
	if (!longmode)
		return(0);

	printf("\n");	
	return(0);
}

#define	HEAD_1	"SEQN %16.16s %10.10s %8.8s"

/*
 * view4symb()
 * Printf RTS symbols
 */
int view4symb(char *symb)
{
	struct	symb	*f_symb, x_symb;
	char	many[20], form[30], tmpb[40];
	int	lmax, ncol, hook, total;
	int	ii, jj, nn;

	if (rt_symbol->valr <= 0)
		return(0);

	if (symb != NULL && strlen(symb) > 0)
	{
		memset(&x_symb, 0, sizeof(struct symb));
		strcpy(x_symb.symb, symb);
		f_symb = bsearch(&x_symb, rt_symbol->symb, rt_symbol->valr, sizeof(struct symb), cmp_symb);
		if (f_symb == NULL)
		{
			fprintf(stderr, "No such symbol...\n");
			return(-1);
		}
		printf("Symbol = %s\n", f_symb->symb);
		printf("Seq #  = %d\n", f_symb->seqn);
		printf("Attach = %d\n", rt_board->hook[f_symb->seqn].many);
		printf("Status = ");
		if (f_symb->mark)
			printf("Active\n");
		else
			printf("Deleted\n");
		return(0);
	}
	for (ii = 0, lmax = 0; ii < rt_symbol->valr; ii++)
	{
		if ((jj = strlen(rt_symbol->symb[ii].symb)) >  lmax)
			lmax = jj;
	}
	if (lmax <= 0)
		lmax = 20;
	lmax += 7;			// "-9999  "
	sprintf(form, "%%-%d.%ds", lmax, lmax);
	ncol = maxcol / (lmax+1);

	if (longmode)
	{
		printf(HEAD_1, "SYMBOL", "ATTACHED", "STATUS");
		printf("\t");
		printf(HEAD_1, "SYMBOL", "ATTACHED", "STATUS");
		printf("\n");
	}

	for (ii = 0, nn = 0, total = 0; ii < rt_symbol->valr; ii++)
	{
		jj = rt_symbol->symb[ii].seqn;
		hook = rt_board->hook[jj].many;
		if (!allsymb  && hook <= 0)
			continue;
		if (!(allsymb & 0x02) && !rt_symbol->symb[ii].mark)
			continue;
		if (longmode)
		{
			printf("%4d ",  nn+1);
			sprintf(many, "%4d", rt_board->hook[jj].many);
			printf("%16s %10s ", rt_symbol->symb[ii].symb, many);
			if (rt_symbol->symb[ii].mark)
				printf("%8s", "Active");
			else
				printf("%8s", "Deleted");
			if ((nn%2) == 0)
				printf("\t");
			else
				printf("\n");
			nn++;
		}
		else
		{
			if (rt_symbol->symb[ii].mark)
				sprintf(tmpb, "%s-%d", rt_symbol->symb[ii].symb, hook);
			else
				sprintf(tmpb, "%s-", rt_symbol->symb[ii].symb);
			printf(form, tmpb);
			nn++;
			if (nn >= ncol)
			{
				printf("\n");
				nn = 0;
			}
		}
		total++;
	}
	if (longmode)
	{
		if (nn % 2)
			printf("\n");
	}
	else if (nn != 0)
		printf("\n");
	printf("Total %d symbols\n", total);
	return(0);
}

//
// trace push message
int trace4rts(char *symb)
{
	struct	pushmsg pushmsg;
	int	widx;
	int	ii;
	
	widx = rt_fifo->widx;
	while (1)
	{
		if (widx == rt_fifo->widx)
		{
			usleep(10000);
			continue;
		}
		memcpy(&pushmsg, &rt_fifo->push[widx], sizeof(struct pushmsg));
		widx++;
		if (widx >= rt_fifo->maxr)
			widx = 0;
		if (symb != NULL && strlen(symb) > 0 && strcmp(pushmsg.symb, symb) != 0)
			continue;
		printf("#%d %-16.16s [%02X,%08X,%02X] ", pushmsg.seqn, pushmsg.symb, pushmsg.type, pushmsg.mask, pushmsg.flag);
		for (ii = 0; ii < pushmsg.msgl; ii++)
		{
			if (!(pushmsg.msgb[ii] & 0x80) && pushmsg.msgb[ii] >= 0x20)
				printf("%c", pushmsg.msgb[ii]);
			else
				printf(".");
		}
		printf("\n");
	}
}

int shm4myrq()
{
	int	shmidx;
	char	*shmad;

	shmidx = shmget(RTD_IPCK(0), 0, 0666);
	if (shmidx < 0)
	{
		fprintf(stderr, "MyRQ system is not initialized....\n");
		return(-1);
	}
	if ((shmad = shmat(shmidx, (char *)0, SHM_RDONLY)) == (char *)-1)
	{
		fprintf(stderr, "MyRQ system is not initialized....\n");
		return(-1);
	}
	rt_board   = (struct rt_board *)shmad;
	rt_symbol = (struct rt_symbol *)&shmad[rt_board->symbs];
	rt_fifo   = (struct rt_fifo *)&shmad[rt_board->fifos];
	return(0);
}

int	cmp_symb(struct symb *symb1, struct symb *symb2)
{
	return(strcmp(symb1->symb, symb2->symb));
}

