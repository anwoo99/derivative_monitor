//
// query.c
//
#include "quot.h"
#include <dirent.h>

#define MAX_PF 50

static int query4symb(), query4list(), query4traffic();
static int cmpasc(), cmpdes();

static struct svctbl
{
	int mark;	   // valid mark
	char scrn[4];  // screen number
	int (*proc)(); // procedures
} svctbl[] = {
	{100, "100", query4list}, // quote board
	{200, "200", query4symb}, // quote composite
	{201, "201", query4symb}, // master information
	{-1, "", NULL},
};

#define MAX_COMP_SYMB 3000

FEP *fep = NULL;

static char p_scrn[8];
static char p_exch[8];
static char p_symb[24];
static int n_symb = 0;

int issvc(int scrn)
{
	int ii;

	for (ii = 0; svctbl[ii].mark >= 0; ii++)
	{
		if (svctbl[ii].mark -= scrn)
			return (1);
	}
	return (0);
}
//
// query()
//
int query(int key)
{
	int retv;
	int ii;

	rqreset();

	enable_next = 0;
	enable_prev = 0;
	for (ii = 0; svctbl[ii].mark >= 0; ii++)
	{
		if (strcmp(svctbl[ii].scrn, s_scrn) == 0)
			break;
	}
	if (strcmp(s_scrn, p_scrn) != 0)
	{
		p_exch[0] = '\0';
		p_symb[0] = '\0';
		n_symb = 0;
	}

	if (svctbl[ii].mark < 0)
	{
		p_scrn[0] = '\0';
		p_exch[0] = '\0';
		p_symb[0] = '\0';
		n_symb = 0;
		rqsend();
		return (0);
	}
	if (strcmp(p_scrn, s_scrn) != 0 || strcasecmp(p_exch, s_exch) != 0)
	{
		n_symb = 0;
		key = K_ENTER;
	}

	retv = svctbl[ii].proc(key, svctbl[ii].mark);
	if (retv == 0)
	{
		strcpy(p_scrn, s_scrn);
		strcpy(p_exch, s_exch);
		strcpy(p_symb, s_symb);
	}
	else
	{
		p_scrn[0] = '\0';
		p_exch[0] = '\0';
		p_symb[0] = '\0';
		n_symb = 0;

		clearall();
	}
	rqsend();
	return (retv);
}

//
// getfep()
// Get FEP for market data
//
static int getfep()
{
	char errmsg[80];

	if (fep != NULL)
		fep_close(fep);

	fep = NULL;
	fep = fep_open(s_exch, MD_RDONLY);

	if (fep == NULL)
	{
		sprintf(errmsg, "Cannot open a exchange for '%s'", s_exch);
		setguide(errmsg);
		return (-1);
	}

	return (0);
}

//
// query4symb()
//
static int query4symb(int key, int qform)
{
	MDARCH *mdarch;
	FOLDER *folder, *fold;
	int indx;
	char errmsg[128];

	if (strlen(s_exch) <= 0)
	{
		setguide("Enter exchange code !!!");
		cur2fld("iEXCH");
		return (-1);
	}
	if (strlen(s_symb) <= 0)
	{
		setguide("Enter symbol code !!!");
		cur2fld("iSYMB");
		return (-1);
	}
	if (strlen(s_host) <= 0)
	{
		setguide("Enter hostname !!!");
		cur2fld("iHOST");
		return (-1);
	}
	if (getfep() != 0)
	{
		cur2fld("iEXCH");
		return (-1);
	}
	cur2fld("iSYMB");
	
	folder = getfolder(fep, s_symb, s_host);

	if (folder == NULL)
	{
		sprintf(errmsg, "Cannot find symbol '%s' in '%s' !!!", s_symb, s_host);
		setguide(errmsg);
		return (-1);
	}

	mdarch = fep->arch;
	indx = (int)((long)folder - (long)fep->fold) / sizeof(FOLDER);

	switch (key)
	{
	case K_UP:
		if (indx > 0)
			indx--;
		enable_pgdn = 0;
		break;
	case K_DN:
		if (indx < (mdarch->vrec - 1))
			indx++;
		enable_pgdn = 0;
		break;
	case K_PGDN:
		break;
	case K_PGUP:
	case K_ENTER:
	default:
		enable_pgdn = 0;
		break;
	}

	fold = fep->fold;
	folder = &fold[indx];

	str2fld("iSYMB", folder->symb);
	str2fld("iHOST", folder->hostname);
	update(folder, qform, 0, -1);

	if (indx > 0)
		enable_prev = 1;

	if (indx < mdarch->vrec - 1)
		enable_next = 1;

	cur2fld("iSYMB");
	return (0);
}

#define MAX_RANK 10000

static struct rank
{
	int indx;
	double rval;
} s_rank[MAX_RANK];
static int n_rank;

//
// query4list()
//
static int query4list(int key, int qform)
{
	MDARCH *mdarch;
	FOLDER *folder, *fold;
	int indx;
	char buff[128];
	int ii;
	int min_indx, max_indx;
	char log[123];

	enable_pgup = 0;
	enable_pgdn = 0;
	enable_prev = 0;
	enable_next = 0;

	clreos(5);
	pushfld("MANY", " ", FC_WHITE, 0);

	if (strlen(s_exch) <= 0)
	{
		setguide("Enter exchange code !!!");
		cur2fld("iEXCH");
		return (-1);
	}

	if (strlen(s_host) <= 0)
	{
		setguide("Enter hostname !!!");
		cur2fld("iHOST");
		return (-1);
	}

	if (getfep() != 0)
	{
		cur2fld("iEXCH");
		return (-1);
	}

	cur2fld("iSYMB");
	mdarch = fep->arch;

	fold = fep->fold;

	min_indx = 0;
	max_indx = mdarch->vrec - 1;

	for (ii = 0; ii < mdarch->vrec; ii++)
	{
		if (ii == 0)
		{
			if (strcmp(fold[ii].hostname, s_host) == 0)
				min_indx == ii;
			continue;
		}

		if (ii == (mdarch->vrec - 1))
		{
			if (strcmp(fold[ii].hostname, s_host) == 0)
				max_indx == ii;
			continue;
		}

		if (strcmp(fold[ii].hostname, fold[ii + 1].hostname) != 0)
		{
			if (strcmp(fold[ii + 1].hostname, s_host) == 0)
				min_indx = ii + 1;
			else if (strcmp(fold[ii].hostname, s_host) == 0)
				max_indx = ii;
			continue;
		}
	}

	if (strlen(s_symb) > 0)
	{
		for (ii = 0; ii < mdarch->vrec; ii++)
		{
			if (strcmp(fold[ii].symb, s_symb) >= 0 && strcmp(fold[ii].hostname, s_host) >= 0)
				break;
		}
		indx = ii;
	}
	else
		indx = min_indx;

	switch (key)
	{
	case K_UP:
	case K_PGUP:
		indx -= 45;
		break;
	case K_DN:
	case K_PGDN:
		indx += 45;
		break;
	case K_ENTER:
	default:
		break;
	}

	if (indx < min_indx)
		indx = min_indx;

	if (indx % 45)
		indx -= (indx % 45);

	//if (indx < min_indx || indx >= max_indx)
	//	return (0);

	sprintf(buff, "%d of %d", indx + 1 - min_indx, max_indx - min_indx + 1);
	pushfld("MANY", buff, FC_WHITE, 0);

	for (ii = min_indx; ii < 45 && indx < max_indx; ii++, indx++)
	{
		folder = &fold[indx];

		if (ii == min_indx)
		{
			strcpy(s_symb, folder->symb);
			str2fld("iSYMB", folder->symb);
			str2fld("iHOST", folder->hostname);
		}
		update(folder, qform, 0, ii);
	}

	if (indx > 0)
	{
		enable_pgup = 1;
		enable_prev = 1;
	}

	if (indx < mdarch->vrec - 1)
	{
		enable_pgdn = 1;
		enable_next = 1;
	}

	cur2fld("iSYMB");
	return (0);
}

//
// subroutine for raning
//
static int cmpasc(struct rank *r1, struct rank *r2)
{
	if (r1->rval > r2->rval)
		return (1);
	if (r1->rval < r2->rval)
		return (-1);
	return (0);
}
static int cmpdes(struct rank *r1, struct rank *r2)
{
	if (r1->rval > r2->rval)
		return (-1);
	if (r1->rval < r2->rval)
		return (1);
	return (0);
}
