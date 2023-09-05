#include <string.h>
#include <libgen.h>
#include <limits.h>
#include "rqsvr.h"
#include "config.h"

#define	K	1024

extern	int	ifcheck(char *ipad, char *net_mask, char *broad_ipad); 

//
// set default value for configurable variable
//
int	no_of_clients	 = 1024;
int	msg_que_size[3]  = { 128*K, 256*K, 64*K };
int	no_of_threads[4] = { 1, 1, 32, 2 };
int	no_of_symbols    = 4096;
int	no_of_buffers	 = 512;
int	heartbeat_time   = 30;
int	dynamic_mode     = 1;
int	do_restart 	 = 0;
int	tm_restart	 = 0;
int	logging_level	 = 0;
char	rq_log_file[128];
char	symbol_file[128];
char	preload_file[128];

int	n_cpus = 0;
int	cpu_affinity[MAX_CPUS];

static	int cfg_tuning(), cfg_symbol(), cfg_log();

struct cfgtags {
	int	mark;
	char	tags[32];	
	int	(*proc)();
} cfgtags[] = {
	{ 1,	"tuning",		cfg_tuning    },
	{ 2,	"log",			cfg_log       },
	{ 3,	"symbol",		cfg_symbol    },
	{ 0,	"",			NULL	      }
};

struct variable {
	int	mark;
	char	name[16];
	int	*pval;
};


//
// config()
//
int config(char *argv)
{
	struct	xmltag xmltag[128];
	char	xml_path[128];
	int	n_conf, n_chck, done;
	char	*base;
	int	ii;

	base = basename(argv);
	sprintf(xml_path, "%s/%s.cfg", ETC_DIR, base);
	if ((n_conf = getxmlcfg(xml_path, xmltag)) <= 0)
		return(-1);

	n_chck = 0;
	while (n_conf > n_chck)
	{
		for (ii = 0; cfgtags[ii].mark != 0; ii++)
		{
			if (strcasecmp(xmltag[n_chck].tags, cfgtags[ii].tags) == 0)
				break;
		}
		if (cfgtags[ii].mark <= 0)
		{
			n_chck++;
			continue;
		}
		done = (*cfgtags[ii].proc)(&xmltag[n_chck]);
		if (done <= 0)
			done++;
		n_chck += done;
	}
	for (ii = 0; ii < 3; ii++)
	{
		if (no_of_threads[ii] <= 0)
			no_of_threads[ii] = 1;
		if (no_of_threads[ii] > MAX_THREADS)
			no_of_threads[ii] = MAX_THREADS;
	}
	return(0);
}

static	struct variable tuning_parm[] = {
	{ 1,	"no_of_clients",	&no_of_clients	    },		// no of client
	{ 1,	"no_of_symbols",	&no_of_symbols      },		// no of sysmbols
	{ 1,	"no_of_buffers",	&no_of_buffers      },		// no of buffers
	{ 1,	"heartbeat",		&heartbeat_time	    },		// heartbeat time interval
	{ 2,	"no_of_threads",	NULL		    },
	{ 3,	"size_of_queue",	NULL		    },
	{ 4,	"restart",		&do_restart	    },
	{ 5,	"cpu_affinity",		NULL		    },
	{ 0,	"",			NULL		    },
};

static int cfg_tuning(struct xmltag *xmltag)
{
	int	done = 0;
	int	v_parm;
	char	argstr[80], sub[2][16];
	char	*lptr, argb[16][16];
	int	hh, mm;
	int	ii, jj, kk, ll;

	if (strcasecmp(xmltag->tags, "tuning") != 0)
		return(0);

	xmltag++;
	for (done = 1; ; done++, xmltag++)
	{
		if (strlen(xmltag->tags) <= 0)
			break;
		if (strcasecmp(xmltag->tags, "/tuning") == 0)
		{
			done++;
			break;
		}
		if (strcasecmp(xmltag->tags, "define") != 0 && strcasecmp(xmltag->tags, "def") != 0)
			continue;
		for (ii = 0; ii < xmltag->many; ii++)
		{
			for (jj = 0; tuning_parm[jj].mark > 0; jj++)
			{
				if (strcasecmp(xmltag->defs[ii].name, tuning_parm[jj].name) == 0)
					break;
			}
			if (tuning_parm[jj].mark <= 0)
				continue;
			v_parm = atoi(xmltag->defs[ii].vals);
			switch (tuning_parm[jj].mark)
			{
			case 1: *tuning_parm[jj].pval = v_parm; 	
				break;
			case 2: // no of threads
				while ((lptr = strstr(xmltag->defs[ii].vals, ",")) != NULL)
					*lptr = ' ';
				argb[0][0] = '\0';
				argb[1][0] = '\0';
				argb[2][0] = '\0';
				argb[3][0] = '\0';
				sscanf(xmltag->defs[ii].vals, "%s %s %s %s", argb[0], argb[1], argb[2], argb[3]);
				no_of_threads[0] = atoi(argb[0]);
				no_of_threads[1] = atoi(argb[1]);
				no_of_threads[2] = atoi(argb[2]);
				no_of_threads[3] = atoi(argb[3]);
				break;
			case 3: // msg queue size
				while ((lptr = strstr(xmltag->defs[ii].vals, ",")) != NULL)
					*lptr = ' ';
				argb[0][0] = '\0';
				argb[1][0] = '\0';
				argb[2][0] = '\0';
				sscanf(xmltag->defs[ii].vals, "%s %s %s", argb[0], argb[1], argb[2]);
				
				msg_que_size[0] = atoi(argb[0]) * K;
				msg_que_size[1] = atoi(argb[1]) * K;
				msg_que_size[2] = atoi(argb[2]) * K;
				break;
			case 4: // restart time
				if ((lptr = strstr(xmltag->defs[ii].vals, ":")) == NULL)
					break;
				*lptr = ' ';
				argb[0][0] = '\0';
				argb[1][0] = '\0';
				sscanf(xmltag->defs[ii].vals, "%s %s", argb[0], argb[1]);
				hh = atoi(argb[0]);
				mm = atoi(argb[1]);
				tm_restart = (hh * 100) + mm;
				if (tm_restart != 0)
					do_restart = 1;
				break;
			case 5: // CPU affinity
				n_cpus = 0;
				for (jj = 0, kk = 0; jj < strlen(xmltag->defs[ii].vals); jj++)
				{
					if (xmltag->defs[ii].vals[jj] != ' ')
						argstr[kk++] = xmltag->defs[ii].vals[jj];
				}
				argstr[kk] = '\0';
				if (strlen(argstr) <= 0)
					break;

				while ((lptr = strchr(argstr, ',')) != NULL)
					*lptr = ' ';
				for (jj = 0; jj < 16; jj++)
					argb[jj][0] = '\0';
				sscanf(argstr, "%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s",
			      		argb[0], argb[1], argb[2], argb[3], argb[4], argb[5], 
					argb[6], argb[7], argb[8], argb[9], argb[10], argb[11], 
					argb[12],argb[13],argb[14],argb[15]); 
				for (jj = 0; jj < 16 && n_cpus < MAX_CPUS; jj++)
				{
					if (strlen(argb[jj]) <= 0)
						break;
					if ((lptr = strchr(argb[jj], '-')) != NULL)
					{
						*lptr = ' ';
						sub[0][0] = '\0';
						sub[1][0] = '\0';
						sscanf(argb[jj], "%s %s", sub[0], sub[1]);
						kk = atoi(sub[0]);
						ll = atoi(sub[1]);
						for (; kk <= ll && n_cpus < MAX_CPUS; kk++)
							cpu_affinity[n_cpus++] = kk;
					}
					else
					{
						kk = atoi(argb[jj]);
						cpu_affinity[n_cpus++] = kk;
					}
				}
				break;
			default:
				break;
			}
		}
	}
	if (no_of_symbols > USHRT_MAX)
		no_of_symbols = USHRT_MAX;
	return(done);
}

static	struct variable symbol_parm[] = {
	{ 1,	"dynamic",		&dynamic_mode  	},		// dynamic or static
	{ 2,	"file",			NULL	    	},		// symbol log file for static mode
	{ 3,	"preload",		NULL	    	},		// preload file
	{ 0,	"",			NULL	    	},
};

static int cfg_symbol(struct xmltag *xmltag)
{
	int	done = 0;
	int	v_parm;
	int	ii, jj;


	if (strcasecmp(xmltag->tags, "symbol") != 0)
		return(0);

	xmltag++;	
	for (done = 1; ; done++, xmltag++)
	{
		if (strlen(xmltag->tags) <= 0)
			break;
		if (strcasecmp(xmltag->tags, "/symbol") == 0)
			break;
		if (strcasecmp(xmltag->tags, "define") != 0 && strcasecmp(xmltag->tags, "def") != 0)
			continue;
		for (ii = 0; ii < xmltag->many; ii++)
		{
			for (jj = 0; symbol_parm[jj].mark > 0; jj++)
			{
				if (strcasecmp(xmltag->defs[ii].name, symbol_parm[jj].name) == 0)
					break;
			}
			if (symbol_parm[jj].mark <= 0)
				continue;
			v_parm = atoi(xmltag->defs[ii].vals);
			switch (symbol_parm[jj].mark)
			{
			case 1: *symbol_parm[jj].pval = v_parm; 	
				break;
			case 2: // symbol file
				strcpy(symbol_file, xmltag->defs[ii].vals);
				break;
			case 3: // symbol preload file
				strcpy(preload_file, xmltag->defs[ii].vals);
				break;
			default:break;
			}
		}
	}
	return(done);
}

static struct variable log_parm[] = {
	{ 1,	"level",		NULL	            },		// debug tracing
	{ 2,	"file",			NULL		    },		// loging file
	{ 0,	"",			NULL		    }
};			


static int cfg_log(struct xmltag *xmltag)
{
	int	done = 0;
	int	v_parm;
	int	ii, jj;

	if (strcasecmp(xmltag->tags, "log") != 0)
		return(0);
	xmltag++;
	for (done = 1; ; done++, xmltag++)
	{
		if (strlen(xmltag->tags) <= 0)
			break;
		if (strcasecmp(xmltag->tags, "/log") == 0)
		{
			done++;
			break;
		}
		if (strcasecmp(xmltag->tags, "define") != 0 && strcasecmp(xmltag->tags, "def") != 0)
			continue;
		for (ii = 0; ii < xmltag->many; ii++)
		{
			for (jj = 0; log_parm[jj].mark > 0; jj++)
			{
				if (strcasecmp(xmltag->defs[ii].name, log_parm[jj].name) == 0)
					break;
			}
			if (log_parm[jj].mark <= 0)
				continue;
			v_parm = atoi(xmltag->defs[ii].vals);
			switch (log_parm[jj].mark)
			{
			case 1: if (strcasecmp(xmltag->defs[ii].vals, "error") == 0)
					logging_level = LL_ERROR;
				else if (strcasecmp(xmltag->defs[ii].vals, "warning") == 0)
					logging_level = LL_WARNING;
				else if (strcasecmp(xmltag->defs[ii].vals, "progress") == 0)
					logging_level = LL_PROGRESS;
				else if (strcasecmp(xmltag->defs[ii].vals, "debug") == 0)
					logging_level = LL_DEBUG;
				break;
			case 2: strcpy(rq_log_file, xmltag->defs[ii].vals);	break;
			default:						break;
			}
		}
	}
	return(done);
}
