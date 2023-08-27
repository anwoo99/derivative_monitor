//
// query.c
//
#include "quot.h"
#include <dirent.h>

#define MAX_PF 50

static int query4symb(), query4list(), query4rank(), query4sett();
static int query4styp(), query4mseg();
static int cmpasc(), cmpdes();

static struct svctbl
{
	int mark;	   // valid mark
	char scrn[4];  // screen number
	int (*proc)(); // procedures
} svctbl[] = {
	{100, "100", query4list}, // quote board
	{101, "101", query4styp}, // quote board(by security type)
	{102, "102", query4mseg}, // quote board (by market segment)
	{105, "105", query4rank}, // most active
	{106, "106", query4rank}, // Top advances
	{107, "107", query4rank}, // Top declines
	{109, "109", query4sett}, // Settlement Prices
	{200, "200", query4symb}, // qquote composite
	{201, "201", query4symb}, // master information
	{202, "202", query4symb}, // quote
	{203, "203", query4symb}, // market depth
	{204, "204", query4symb}, // time & sales
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
	MDFOLD *folder, *fold;
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
	if (getfep() != 0)
	{
		cur2fld("iEXCH");
		return (-1);
	}
	cur2fld("iSYMB");
	folder = getfolder(fep, s_symb);
	if (folder == NULL)
	{
		sprintf(errmsg, "Cannot find symbol '%s' !!!", s_symb);
		setguide(errmsg);
		return (-1);
	}

	mdarch = fep->arch;
	strcpy(s_path, fep->dirp);
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
// query4rank()
//
static int query4rank(int key, int qform)
{
	MDARCH *mdarch;
	MDFOLD *folder, *fold;
	MDQUOT *quot;
	int ii, jj;

	enable_pgup = 0;
	enable_pgdn = 0;
	enable_prev = 0;
	enable_next = 0;

	if (strlen(s_exch) <= 0)
	{
		setguide("Enter exchange code !!!");
		cur2fld("iEXCH");
		return (-1);
	}
	if (getfep() != 0)
	{
		cur2fld("iEXCH");
		return (-1);
	}
	cur2fld("iEXCH");
	mdarch = fep->arch;
	fold = fep->fold;
	for (ii = 0, jj = 0; ii < mdarch->vrec && jj < MAX_RANK; ii++)
	{
		if (fold[ii].mstr.styp == ST_INDEX)
			continue;
		s_rank[jj].indx = ii;
		quot = &fold[ii].quot;
		switch (qform)
		{
		case 105: // most active
			if (quot->tvol <= 0.)
				continue;
			s_rank[jj++].rval = quot->tvol;
			break;
		case 106: // top advances
			if (quot->rate <= 0.0)
				continue;
			s_rank[jj++].rval = quot->rate;
			break;
		case 107:
			if (quot->rate >= 0.0)
				continue;
			s_rank[jj++].rval = quot->rate;
			break;
		}
	}
	clreos(5);
	if ((n_rank = jj) <= 0)
		return (0);
	switch (qform)
	{
	case 105:
		qsort(s_rank, n_rank, sizeof(struct rank), cmpdes);
		break;
	case 106:
		qsort(s_rank, n_rank, sizeof(struct rank), cmpdes);
		break;
	case 107:
		qsort(s_rank, n_rank, sizeof(struct rank), cmpasc);
		break;
	}

	for (ii = 0; ii < 45 && ii < n_rank; ii++)
	{
		jj = s_rank[ii].indx;
		folder = &fold[jj];
		update(folder, qform, 0, ii);
	}
	cur2fld("iEXCH");
	return (0);
}

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
	if (getfep() != 0)
	{
		cur2fld("iEXCH");
		return (-1);
	}
	cur2fld("iSYMB");
	mdarch = fep->arch;
	fold = fep->fold;
	if (strlen(s_symb) > 0)
	{
		for (ii = 0; ii < mdarch->vrec; ii++)
		{
			if (strcmp(fold[ii].symb, s_symb) >= 0)
				break;
		}
		indx = ii;
	}
	else
		indx = 0;

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
	if (indx < 0)
		indx = 0;
	if (indx % 45)
		indx -= (indx % 45);
	if (indx < 0 || indx >= mdarch->vrec)
		return (0);
	sprintf(buff, "%d of %d  / n_root[%d]", indx + 1, mdarch->vrec, mdarch->n_root);
	pushfld("MANY", buff, FC_WHITE, 0);

	for (ii = 0; ii < 45 && indx < mdarch->vrec; ii++, indx++)
	{
		folder = &fold[indx];
		if (ii == 0)
		{
			strcpy(s_symb, folder->symb);
			str2fld("iSYMB", folder->symb);
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
// query4styp()
//
static int query4styp(int key, int qform)
{
	static int where = 0;
	static int lcode = 0;
	static int page = 0;
	MDARCH *mdarch;
	FOLDER *folder, *fold;
	int indx, code = 0, many;
	char buff[128];
	int ii;

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
	if (getfep() != 0)
	{
		cur2fld("iEXCH");
		return (-1);
	}
	cur2fld("iSYMB");
	mdarch = fep->arch;
	fold = fep->fold;

	if (strlen(s_symb) <= 0)
	{
		setguide("Enter code of instrument type !!!");
		cur2fld("iSYMB");
		return (-1);
	}
	switch (key)
	{
	case K_DN:
		indx = 0;
		break;
	case K_UP:
		indx = 0;
		break;
	case K_PGDN:
		indx = where;
		code = lcode;
		page++;
		break;
	default:
		page = 0;
		code = atoi(s_symb);
		indx = 0;
		break;
	}
	sprintf(buff, "%02d", code);
	str2fld("iSYMB", buff);

	switch (code)
	{
	case ST_INDEX:
		sprintf(buff, "INDEX");
		break;
	case ST_FUTURE:
		sprintf(buff, "Futures");
		break;
	case ST_OPTION:
		sprintf(buff, "Options");
		break;
	case ST_FOREX:
		sprintf(buff, "Forex");
		break;
	default:
		setguide("Invalid code for instrument type !!!");
		cur2fld("iSYMB");
		return (-1);
	}
	str2fld("oNAME", buff);

	lcode = code;
	if (indx < 0 || indx >= mdarch->vrec)
		return (0);

	for (ii = 0, many = 0; ii < mdarch->vrec; ii++)
	{
		if (fold[ii].mstr.styp == code)
			many++;
	}
	if (many > 0)
	{
		sprintf(buff, "%d of %d", (page * 45) + 1, many);
		pushfld("MANY", buff, FC_WHITE, 0);
	}
	for (ii = 0; ii <= 45 && indx < mdarch->vrec; indx++)
	{
		folder = &fold[indx];
		if (folder->mstr.styp != code)
			continue;
		if (ii >= 45)
			break;
		update(folder, qform, 0, ii);
		ii++;
	}
	if (ii >= 45)
	{
		enable_pgdn = 1;
		where = indx;
	}
	else
		where = 0;
	cur2fld("iSYMB");
	return (0);
}
//
// query4mseq()
//
static int query4mseg(int key, int qform)
{
	static int where = 0;
	static char lcode[20] = "EQTY";
	MDARCH *mdarch;
	FOLDER *folder, *fold;
	int indx, many;
	char code[20], buff[80];
	int chck;
	int ii;

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
	if (getfep() != 0)
	{
		cur2fld("iEXCH");
		return (-1);
	}
	cur2fld("iSYMB");
	mdarch = fep->arch;
	fold = fep->fold;
	if (strlen(s_symb) <= 0)
	{
		setguide("Enter market segment string !!!");
		cur2fld("iSYMB");
		return (-1);
	}
	switch (key)
	{
	case K_PGDN:
		indx = where;
		strcpy(code, lcode);
		break;
	default:
		if (strcmp(s_scrn, p_scrn) != 0)
			strcpy(code, lcode);
		else
			strcpy(code, s_symb);
		indx = 0;
		break;
	}
	str2fld("iSYMB", code);

	strcpy(lcode, code);
	chck = atoi(code);
	if (indx < 0 || indx >= mdarch->vrec)
		return (0);
	for (ii = 0, many = 0; ii < mdarch->vrec; ii++)
	{
		if (fold[ii].mstr.unpd == chck)
			many++;
	}
	if (many > 0)
	{
		sprintf(buff, "%d of %d", indx + 1, many);
		pushfld("MANY", buff, FC_WHITE, 0);
	}
	for (ii = 0; ii <= 45 && indx < mdarch->vrec; indx++)
	{
		folder = &fold[indx];
		if (folder->mstr.unpd != chck)
			continue;
		if (ii >= 45)
			break;
		update(folder, qform, 0, ii);
		ii++;
	}
	if (ii >= 45)
	{
		enable_pgdn = 1;
		where = indx;
	}
	else
		where = 0;
	cur2fld("iSYMB");
	return (0);
}

//
// query4sett()
//
static int query4sett(int key, int qform)
{
	MDARCH *mdarch;
	FOLDER *folder, *fold;
	int indx;
	char buff[128];
	int ii;

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
	if (getfep() != 0)
	{
		cur2fld("iEXCH");
		return (-1);
	}
	cur2fld("iSYMB");
	mdarch = fep->arch;
	fold = fep->fold;
	if (strlen(s_symb) > 0)
	{
		for (ii = 0; ii < mdarch->vrec; ii++)
		{
			if (strcmp(fold[ii].symb, s_symb) >= 0)
				break;
		}
		indx = ii;
	}
	else
		indx = 0;

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
	if (indx % 45)
		indx -= (indx % 45);
	if (indx < 0)
		indx = 0;
	if (indx < 0 || indx >= mdarch->vrec)
		return (0);
	sprintf(buff, "%d of %d", indx + 1, mdarch->vrec);
	pushfld("MANY", buff, FC_WHITE, 0);

	for (ii = 0; ii < 45 && indx < mdarch->vrec; ii++, indx++)
	{
		folder = &fold[indx];
		if (ii == 0)
			str2fld("iSYMB", folder->symb);
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
