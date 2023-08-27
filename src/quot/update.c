//
// update.c
// Update quote field
//
#include "quot.h"

#define YYMMDD "%04d-%02d-%02d"
#define HHMMSS "%02d:%02d:%02d"
#define HHMM "%02d:%02d"

static char wday[7][8] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

static void pushstr(char *name, char *str, int color, int push, int seqn);
static void pushchr(char *name, int chr, int color, int push, int seqn);
static void pushymd(char *name, int ymd, int color, int push, int seqn);
static void pushhms(char *name, int hms, int color, int push, int seqn);
static void pushint(char *name, int val, int color, int push, int seqn);
static void pushlng(char *name, long val, int color, int push, int seqn);
static void pushflt(char *name, double val, int color, int push, int seqn);
static void pushprc(char *name, double val, int color, int push, int seqn);
static void pushany(char *name, double val, int color, int push, int seqn);
static void pushdif(char *name, double val, int color, int push, int seqn);
static void pushrat(char *name, double val, int color, int push, int seqn);
static void pushchk(char *name, double val, int color, int push, int seqn);
static void pushdept(char *name, double val, int color, int push, int seqn);
static void editprc(char *fldb, double val, int flag);

static char form[16];
static int xdiv, ydiv, zdiv;
static double base;
char s_next[80];

// static int str2int(const char *str, int len)
// {
//	char	tmpb[256];
//
//	memcpy(tmpb, str, len);
//	tmpb[len] = '\0';
//	return(atoi(tmpb));
//}

//
// update()
//
int update(void *qptr, int what, int push, int seqn)
{
    switch (what)
    {
    case 100: // quote board
    case 101: // quote board - by security type
    case 102: // quote board - by market segment
    case 105: // most active
    case 106: // top advances
    case 107: // top decline
    case 109: // settlement prices
    {
        FOLDER *fold = qptr;
        MDMSTR *mstr = &fold->mstr;
        MDQUOT *quot = &fold->quot;

        xdiv = fold->mstr.xdiv;
        ydiv = fold->mstr.ydiv;
        zdiv = fold->mstr.zdiv;
        sprintf(form, "%%'.%df", mstr->zdiv);
        base = mstr->base;
        pushstr("SYMB", quot->symb, FC_WHITE, push, seqn);
        pushstr("NAME", mstr->enam, FC_WHITE, push, seqn);
        pushymd("XYMD", quot->xymd, FC_WHITE, push, seqn);
        pushhms("XHMS", quot->xhms, FC_WHITE, push, seqn);
        pushchk("OPEN", quot->open, FC_WHITE, push, seqn);
        pushchk("HIGH", quot->high, FC_WHITE, push, seqn);
        pushchk("LOW", quot->low, FC_WHITE, push, seqn);
        if (quot->last != 0.)
            pushchk("LAST", quot->last, FC_WHITE, push, seqn);
        else
            pushchk("LAST", quot->setp, FC_WHITE, push, seqn);
        pushdif("DIFF", quot->diff, FC_WHITE, push, seqn);
        pushrat("RATE", quot->rate, FC_WHITE, push, seqn);
        pushlng("TVOL", quot->tvol, FC_WHITE, push, seqn);
        if (mstr->prev.symd != 0)
            pushany("SETT", mstr->prev.setp, FC_WHITE, push, seqn);
        else
            pushchk("SETT", mstr->prev.setp, FC_WHITE, push, seqn);
        pushymd("SETD", mstr->prev.symd, FC_WHITE, push, seqn);
        pushymd("UYMD", mstr->prev.uymd, FC_WHITE, push, seqn);
        pushhms("UHMS", mstr->prev.uhms, FC_WHITE, push, seqn);
        pushymd("KYMD", quot->kymd, FC_WHITE, push, seqn);
        pushhms("KHMS", quot->khms, FC_WHITE, push, seqn);
        if (!push)
            rqsymb(mstr->exid, mstr->symb, PUSH_QUOT, seqn);
    }
    break;
    case 200: // price composite
    {
        FOLDER *fold = qptr;
        MDMSTR *mstr = &fold->mstr;
        MDQUOT *quot = &fold->quot;
        MDDEPT *dept = &fold->dept;
        uint32_t kymd, khms;
        int ii, jj;
        char str_hms[20];

        if (!push)
            str2fld("oNAME", mstr->enam);
        xdiv = mstr->xdiv;
        ydiv = mstr->ydiv;
        zdiv = mstr->zdiv;
        sprintf(form, "%%'.%df", mstr->zdiv);
        base = mstr->base;
        pushint("RSEQ", quot->rseq, FC_WHITE, push, seqn);
        pushint("SSEQ", quot->sseq, FC_WHITE, push, seqn);
        if (push == 0 || push == 1)
        {
            pushstr("SYMB", quot->symb, FC_WHITE, push, seqn);
            pushstr("CODE", mstr->code, FC_WHITE, push, seqn);
            pushstr("SCID", mstr->scid, FC_WHITE, push, seqn);
            pushint("XDIV", mstr->xdiv, FC_WHITE, push, seqn);
            pushint("YDIV", mstr->ydiv, FC_WHITE, push, seqn);
            pushint("ZDIV", mstr->zdiv, FC_WHITE, push, seqn);
            pushint("CDIV", mstr->cdiv, FC_WHITE, push, seqn);
            pushprc("BASE", quot->base, FC_WHITE, push, seqn);
            pushymd("PYMD", mstr->pymd, FC_WHITE, push, seqn);
            pushprc("UPLP", mstr->uplp, FC_WHITE, push, seqn);
            pushprc("DNLP", mstr->dnlp, FC_WHITE, push, seqn);
            pushflt("PMUL", mstr->pmul, FC_WHITE, push, seqn);
            if (quot->symd != 0)
                pushany("SETP", quot->setp, FC_WHITE, push, seqn);
            else
                pushprc("SETP", quot->setp, FC_WHITE, push, seqn);
            pushint("SFLG", quot->sflg, FC_WHITE, push, seqn);
            pushymd("SYMD", quot->symd, FC_WHITE, push, seqn);
            if (kst4time)
            {
                fep_kortime(fep, quot->symd, mstr->prev.uhms, &kymd, &khms);
                sprintf(str_hms, "%02d:%02d:%02d (KST)", HOUR(khms), MINUTE(khms), SECOND(khms));
                pushstr("SHMS", str_hms, FC_WHITE, push, seqn);
            }
            else
                pushhms("SHMS", mstr->prev.uhms, FC_WHITE, push, seqn);
            pushymd("TYMD", quot->tymd, FC_WHITE, push, seqn);
            pushymd("XYMD", quot->xymd, FC_WHITE, push, seqn);
            pushhms("XHMS", quot->xhms, FC_WHITE, push, seqn);
            pushint("SEQN", quot->seqn, FC_WHITE, push, seqn);
            pushymd("KYMD", quot->kymd, FC_WHITE, push, seqn);
            pushhms("KHMS", quot->khms, FC_WHITE, push, seqn);
            pushchk("OPEN", quot->open, FC_WHITE, push, seqn);
            pushchk("HIGH", quot->high, FC_WHITE, push, seqn);
            pushchk("LOW", quot->low, FC_WHITE, push, seqn);
            pushchk("LAST", quot->last, FC_WHITE, push, seqn);
            pushdif("DIFF", quot->diff, FC_WHITE, push, seqn);
            pushrat("RATE", quot->rate, FC_WHITE, push, seqn);
            pushint("SIGN", quot->sign, FC_WHITE, push, seqn);
            pushint("TICK", quot->tick, FC_WHITE, push, seqn);
            pushchr("DIRF", quot->dirf, FC_WHITE, push, seqn);
            pushchr("SIDE", quot->side, FC_WHITE, push, seqn);
            pushint("EVOL", quot->evol, FC_WHITE, push, seqn);
            pushlng("TVOL", quot->tvol, FC_WHITE, push, seqn);
            pushint("OPIN", quot->opin, FC_WHITE, push, seqn);
            pushint("OIDF", quot->oidf, FC_WHITE, push, seqn);
            if (kst4time)
            {
                fep_kortime(fep, quot->xymd, quot->otim, &kymd, &khms);
                sprintf(str_hms, "%02d:%02d:%02d (KST)", HOUR(khms), MINUTE(khms), SECOND(khms));
                pushstr("OTIM", str_hms, FC_WHITE, push, seqn);

                fep_kortime(fep, quot->xymd, quot->htim, &kymd, &khms);
                sprintf(str_hms, "%02d:%02d:%02d (KST)", HOUR(khms), MINUTE(khms), SECOND(khms));
                pushstr("HTIM", str_hms, FC_WHITE, push, seqn);

                fep_kortime(fep, quot->xymd, quot->ltim, &kymd, &khms);
                sprintf(str_hms, "%02d:%02d:%02d (KST)", HOUR(khms), MINUTE(khms), SECOND(khms));
                pushstr("LTIM", str_hms, FC_WHITE, push, seqn);
            }
            else
            {
                pushhms("OTIM", quot->otim, FC_WHITE, push, seqn);
                pushhms("HTIM", quot->htim, FC_WHITE, push, seqn);
                pushhms("LTIM", quot->ltim, FC_WHITE, push, seqn);
            }
            pushint("TSID", quot->tsid, FC_WHITE, push, seqn);
            pushint("STAT", quot->stat, FC_WHITE, push, seqn);
            switch (quot->stat)
            {
            case 2:
                pushstr("SSTR", "Trading Halt", FC_WHITE, push, seqn);
                break;
            case 5:
                pushstr("SSTR", "Price Indication", FC_WHITE, push, seqn);
                break;
            case 17:
                pushstr("SSTR", "Ready To Trade", FC_WHITE, push, seqn);
                break;
            case 18:
                pushstr("SSTR", "End of Session", FC_WHITE, push, seqn);
                break;
            case 20:
                pushstr("SSTR", "Unknown Or Invalid", FC_WHITE, push, seqn);
                break;
            case 21:
                pushstr("SSTR", "Pre Open", FC_WHITE, push, seqn);
                break;
            case 24:
                pushstr("SSTR", "Pre Cross", FC_WHITE, push, seqn);
                break;
            case 25:
                pushstr("SSTR", "Cross", FC_WHITE, push, seqn);
                break;
            case 26:
                pushstr("SSTR", "NoCancel", FC_WHITE, push, seqn);
                break;
            default:
                pushstr("SSTR", "", FC_WHITE, push, seqn);
                break;
            }
        }
        quot = &fold->quat;
        // if (quot->xymd != 0 && (push == 0 || push == 1))
        if ((push == 0 || push == 1))
        {
            pushymd("TYMD2", quot->tymd, FC_WHITE, push, seqn);
            pushymd("XYMD2", quot->xymd, FC_WHITE, push, seqn);
            pushhms("XHMS2", quot->xhms, FC_WHITE, push, seqn);
            pushint("SEQN2", quot->seqn, FC_WHITE, push, seqn);
            pushymd("KYMD2", quot->kymd, FC_WHITE, push, seqn);
            pushhms("KHMS2", quot->khms, FC_WHITE, push, seqn);
            pushchk("OPEN2", quot->open, FC_WHITE, push, seqn);
            pushchk("HIGH2", quot->high, FC_WHITE, push, seqn);
            pushchk("LOW2", quot->low, FC_WHITE, push, seqn);
            pushchk("LAST2", quot->last, FC_WHITE, push, seqn);
            pushdif("DIFF2", quot->diff, FC_WHITE, push, seqn);
            pushrat("RATE2", quot->rate, FC_WHITE, push, seqn);
            pushint("SIGN2", quot->sign, FC_WHITE, push, seqn);
            pushint("TICK2", quot->tick, FC_WHITE, push, seqn);
            pushchr("DIRF2", quot->dirf, FC_WHITE, push, seqn);
            pushchr("SIDE2", quot->side, FC_WHITE, push, seqn);
            pushint("EVOL2", quot->evol, FC_WHITE, push, seqn);
            pushlng("TVOL2", quot->tvol, FC_WHITE, push, seqn);
            pushint("OPIN2", quot->opin, FC_WHITE, push, seqn);
            if (kst4time)
            {
                fep_kortime(fep, quot->xymd, quot->otim, &kymd, &khms);
                pushhms("OTIM2", khms, FC_WHITE, push, seqn);
                fep_kortime(fep, quot->xymd, quot->htim, &kymd, &khms);
                pushhms("HTIM2", khms, FC_WHITE, push, seqn);
                fep_kortime(fep, quot->xymd, quot->ltim, &kymd, &khms);
                pushhms("LTIM2", khms, FC_WHITE, push, seqn);
            }
            else
            {
                pushhms("OTIM2", quot->otim, FC_WHITE, push, seqn);
                pushhms("HTIM2", quot->htim, FC_WHITE, push, seqn);
                pushhms("LTIM2", quot->ltim, FC_WHITE, push, seqn);
            }
            pushint("TSID2", quot->tsid, FC_WHITE, push, seqn);
            pushint("STAT2", quot->stat, FC_WHITE, push, seqn);
        }

        // market depth
        pushint("NDPT", mstr->ndpt[0], FC_WHITE, push, seqn);
        pushint("IDPT", mstr->ndpt[1], FC_WHITE, push, seqn);
        if (push == 0 || push == 2)
        {
            if (kst4time)
                pushhms("HHMS", dept->khms, FC_WHITE, push, seqn);
            else
                pushhms("HHMS", dept->xhms, FC_WHITE, push, seqn);
            for (ii = 0, jj = 9; ii < 10; ii++, jj--)
            {
                if (dept->ask[jj].vask != 0)
                {
                    pushdept("PASK", dept->ask[jj].pask, FC_WHITE, push, ii);
                }
                else
                {
                    pushstr("PASK", " ", FC_WHITE, push, ii);
                }
                if (dept->ask[jj].vask != 0.0)
                    pushint("VASK", dept->ask[jj].vask, FC_WHITE, push, ii);
                else
                    pushstr("VASK", " ", FC_WHITE, push, ii);
                if (dept->ask[jj].vask != 0.0 && dept->ask[jj].cask != 0)
                    pushint("CASK", dept->ask[jj].cask, FC_WHITE, push, ii);
                else
                    pushstr("CASK", " ", FC_WHITE, push, ii);
                if (dept->ask[jj].vask != 0.0 && dept->ask[jj].nask != 0)
                    pushint("NASK", dept->ask[jj].nask, FC_WHITE, push, ii);
                else
                    pushstr("NASK", " ", FC_WHITE, push, ii);

                if (dept->bid[ii].vbid != 0)
                {
                    pushdept("PBID", dept->bid[ii].pbid, FC_WHITE, push, ii);
                }
                else
                {
                    pushstr("PBID", " ", FC_WHITE, push, ii);
                }
                if (dept->bid[ii].vbid != 0.0)
                    pushint("VBID", dept->bid[ii].vbid, FC_WHITE, push, ii);
                else
                    pushstr("VBID", " ", FC_WHITE, push, ii);
                if (dept->bid[ii].vbid != 0.0 && dept->bid[ii].cbid != 0)
                    pushint("CBID", dept->bid[ii].cbid, FC_WHITE, push, ii);
                else
                    pushstr("CBID", " ", FC_WHITE, push, ii);
                if (dept->bid[ii].vbid != 0.0 && dept->bid[ii].nbid != 0)
                    pushint("NBID", dept->bid[ii].nbid, FC_WHITE, push, ii);
                else
                    pushstr("NBID", " ", FC_WHITE, push, ii);
            }
            pushint("D.VBID", dept->vbid, FC_WHITE, push, -1);
            pushint("D.VASK", dept->vask, FC_WHITE, push, -1);
            pushint("D.NBID", dept->nbid, FC_WHITE, push, -1);
            pushint("D.NASK", dept->nask, FC_WHITE, push, -1);
            pushint("D.CBID", dept->cbid, FC_WHITE, push, -1);
            pushint("D.CASK", dept->cask, FC_WHITE, push, -1);
        }
        if (fold->quat.xymd > fold->quot.xymd ||
            (fold->quat.xymd == fold->quot.xymd && fold->quat.xhms > fold->quot.xhms))
            quot = &fold->quat;
        else
            quot = &fold->quot;

        if (push == 0 || push == 1)
        {
            pushint("BVOL", quot->bvol, FC_WHITE, push, seqn);
            pushint("SVOL", quot->svol, FC_WHITE, push, seqn);
            pushint("BCNT", quot->bcnt, FC_WHITE, push, seqn);
            pushint("SCNT", quot->scnt, FC_WHITE, push, seqn);
        }

        if (push == 1 && quot->tick)
        {
            // by quote, insert to 1st line.
            insline(40, 1);
            if (kst4time)
                pushhms("T.XHMS", quot->khms, FC_WHITE, 0, 0);
            else
                pushhms("T.XHMS", quot->xhms, FC_WHITE, 0, 0);
            pushchk("T.LAST", quot->last, FC_WHITE, 0, 0);
            pushdif("T.DIFF", quot->diff, FC_WHITE, 0, 0);
            pushrat("T.RATE", quot->rate, FC_WHITE, 0, 0);
            pushchk("T.PBID", dept->bid[0].pbid, FC_WHITE, 0, 0);
            pushchk("T.PASK", dept->ask[0].pask, FC_WHITE, 0, 0);
            pushint("T.EVOL", quot->evol, FC_WHITE, 0, 0);
            pushlng("T.TVOL", quot->tvol, FC_WHITE, 0, 0);
            pushchr("T.DIRF", quot->dirf, FC_WHITE, 0, 0);
            pushchr("T.SIDE", quot->side, FC_WHITE, 0, 0);
            break;
        }
        if (push)
            break;

        clreos(40);
        rqsymb(mstr->exid, mstr->symb, PUSH_QUOT | PUSH_DEPT, seqn);
    }
    break;
    case 201: // master
    {
        FOLDER *fold = qptr;
        MDMSTR *mstr = &fold->mstr;
        uint32_t kymd, khms;
        char tmpb[40];
        char str_ymd[20];
        char str_hms[20];

        if (!push)
            str2fld("oNAME", mstr->enam);
        xdiv = mstr->xdiv;
        ydiv = mstr->ydiv;
        zdiv = mstr->zdiv;
        sprintf(form, "%%'.%df", mstr->zdiv);

        pushstr("SYMB", mstr->symb, FC_WHITE, push, seqn);
        pushstr("SCID", mstr->scid, FC_WHITE, push, seqn);
        pushint("SEQN", mstr->seqn, FC_WHITE, push, seqn);
        pushstr("ROOT", mstr->root, FC_WHITE, push, seqn);
        pushstr("CLRS", mstr->clrs, FC_WHITE, push, seqn);
        pushstr("UNPS", mstr->unps, FC_WHITE, push, seqn);
        pushstr("UNID", mstr->unid, FC_WHITE, push, seqn);
        pushstr("GSYM", mstr->gsym, FC_WHITE, push, seqn);
        pushstr("CFIC", mstr->cfic, FC_WHITE, push, seqn);
        pushstr("CODE", mstr->code, FC_WHITE, push, seqn);
        pushstr("ENAM", mstr->enam, FC_WHITE, push, seqn);
        pushstr("SNAM", mstr->snam, FC_WHITE, push, seqn);
        pushstr("INAM", mstr->inam, FC_WHITE, push, seqn);
        pushstr("KNAM", mstr->knam, FC_WHITE, push, seqn);
        pushstr("ECYM", mstr->ecym, FC_WHITE, push, seqn);
        pushstr("KCYM", mstr->kcym, FC_WHITE, push, seqn);
        pushint("EXID", mstr->exid, FC_WHITE, push, seqn);
        pushstr("EXNM", mstr->exnm, FC_WHITE, push, seqn);
        pushint("STAT", mstr->stat, FC_WHITE, push, seqn);
        pushint("CMON", mstr->cmon, FC_WHITE, push, seqn);
        switch (mstr->stat)
        {
        case 2:
            pushstr("SSTR", "Trading Halt", FC_WHITE, push, seqn);
            break;
        case 5:
            pushstr("SSTR", "Price Indication", FC_WHITE, push, seqn);
            break;
        case 17:
            pushstr("SSTR", "Ready To Trade", FC_WHITE, push, seqn);
            break;
        case 18:
            pushstr("SSTR", "End of Session", FC_WHITE, push, seqn);
            break;
        case 20:
            pushstr("SSTR", "Unknown Or Invalid", FC_WHITE, push, seqn);
            break;
        case 21:
            pushstr("SSTR", "Pre Open", FC_WHITE, push, seqn);
            break;
        case 24:
            pushstr("SSTR", "Pre Cross", FC_WHITE, push, seqn);
            break;
        case 25:
            pushstr("SSTR", "Cross", FC_WHITE, push, seqn);
            break;
        case 26:
            pushstr("SSTR", "NoCancel", FC_WHITE, push, seqn);
            break;
        default:
            pushstr("SSTR", "", FC_WHITE, push, seqn);
            break;
        }
        pushint("UNPD", mstr->unpd, FC_WHITE, push, seqn);
        pushflt("PMUL", mstr->pmul, FC_WHITE, push, seqn);
        pushint("XDIV", mstr->xdiv, FC_WHITE, push, seqn);
        pushint("YDIV", mstr->ydiv, FC_WHITE, push, seqn);
        pushint("ZDIV", mstr->zdiv, FC_WHITE, push, seqn);
        pushint("STYP", mstr->styp, FC_WHITE, push, seqn);
        pushstr("CURR1", mstr->curr[0], FC_WHITE, push, seqn);
        pushstr("CURR2", mstr->curr[1], FC_WHITE, push, seqn);
        pushstr("CURR3", mstr->curr[2], FC_WHITE, push, seqn);
        pushymd("EXYM", mstr->exym, FC_WHITE, push, seqn);
        pushymd("LYMD", mstr->lymd, FC_WHITE, push, seqn);
        pushymd("ZYMD", mstr->zymd, FC_WHITE, push, seqn);
        pushint("JJIS", mstr->jjis, FC_WHITE, push, seqn);
        pushymd("FTDT", mstr->ftdt, FC_WHITE, push, seqn);
        pushymd("LTDT", mstr->ltdt, FC_WHITE, push, seqn);
        pushymd("FNDT", mstr->fndt, FC_WHITE, push, seqn);
        pushymd("LNDT", mstr->lndt, FC_WHITE, push, seqn);
        pushymd("FDDT", mstr->fddt, FC_WHITE, push, seqn);
        pushymd("LDDT", mstr->lddt, FC_WHITE, push, seqn);
        pushstr("FLMY", mstr->flmy, FC_WHITE, push, seqn);
        pushstr("SLMY", mstr->slmy, FC_WHITE, push, seqn);
        pushint("MINV", mstr->minv, FC_WHITE, push, seqn);
        pushint("MAXV", mstr->maxv, FC_WHITE, push, seqn);
        pushint("CSIZ", mstr->csiz, FC_WHITE, push, seqn);
        pushint("CDIV", mstr->cdiv, FC_WHITE, push, seqn);
        pushprc("UPLP", mstr->uplp, FC_WHITE, push, seqn);
        pushprc("DNLP", mstr->dnlp, FC_WHITE, push, seqn);
        pushprc("PINC", mstr->pinc, FC_WHITE, push, seqn);
        pushprc("BASE", mstr->base, FC_WHITE, push, seqn);
        pushprc("CLOS", mstr->clos, FC_WHITE, push, seqn);
        pushstr("TZ", fep->xchg->TZ, FC_WHITE, push, seqn);
        pushstr("VTCK", mstr->vtck, FC_WHITE, push, seqn);
        pushstr("PIND", mstr->pind, FC_WHITE, push, seqn);
        pushint("CVOL", mstr->cvol, FC_WHITE, push, seqn);
        pushint("NDPT", mstr->ndpt[0], FC_WHITE, push, seqn);
        pushint("IDPT", mstr->ndpt[1], FC_WHITE, push, seqn);
        pushymd("P.SYMD", mstr->prev.symd, FC_WHITE, push, seqn);
        pushymd("P.PYMD", mstr->prev.pymd, FC_WHITE, push, seqn);
        if (mstr->prev.symd != 0)
            pushany("P.SETP", mstr->prev.setp, FC_WHITE, push, seqn);
        else
            pushprc("P.SETP", mstr->prev.setp, FC_WHITE, push, seqn);
        if (mstr->prev.pymd != 0)
            pushany("P.BASE", mstr->prev.base, FC_WHITE, push, seqn);
        else
            pushprc("P.BASE", mstr->prev.base, FC_WHITE, push, seqn);
        base = mstr->prev.base;
        pushchk("P.OPEN", mstr->prev.open, FC_WHITE, push, seqn);
        pushchk("P.HIGH", mstr->prev.high, FC_WHITE, push, seqn);
        pushchk("P.LOW", mstr->prev.low, FC_WHITE, push, seqn);
        pushchk("P.LAST", mstr->prev.last, FC_WHITE, push, seqn);
        pushint("P.SIGN", mstr->prev.sign, FC_WHITE, push, seqn);
        pushdif("P.DIFF", mstr->prev.diff, FC_WHITE, push, seqn);
        pushrat("P.RATE", mstr->prev.rate, FC_WHITE, push, seqn);
        pushint("P.TVOL", mstr->prev.tvol, FC_WHITE, push, seqn);
        pushint("P.OPIN", mstr->prev.opin, FC_WHITE, push, seqn);
        pushint("SFLG", mstr->sflg, FC_WHITE, push, seqn);
        if (kst4time)
        {
            fep_kortime(fep, mstr->prev.uymd, mstr->prev.uhms, &kymd, &khms);
            sprintf(str_ymd, "%04d-%02d-%02d (KST)", YEAR(kymd), MONTH(kymd), MDAY(kymd));
            sprintf(str_hms, "%02d:%02d:%02d (KST)", HOUR(khms), MINUTE(khms), SECOND(khms));
            pushstr("P.UYMD", str_ymd, FC_WHITE, push, seqn);
            pushstr("P.UHMS", str_hms, FC_WHITE, push, seqn);
        }
        else
        {
            pushymd("P.UYMD", mstr->prev.uymd, FC_WHITE, push, seqn);
            pushhms("P.UHMS", mstr->prev.uhms, FC_WHITE, push, seqn);
        }
        pushint("OPTS", mstr->opts, FC_WHITE, push, seqn);
        pushchr("CORP", mstr->corp, FC_WHITE, push, seqn);
        pushany("STRK", mstr->strk, FC_WHITE, push, seqn);
        sprintf(tmpb, "%s,%02d:%02d-%s,%02d:%02d",
                wday[mstr->session.fwdy], mstr->session.frhm / 100, mstr->session.frhm % 100,
                wday[mstr->session.twdy], mstr->session.tohm / 100, mstr->session.tohm % 100);
        pushstr("TRHM", tmpb, FC_WHITE, push, seqn);
        sprintf(tmpb, "%02d:%02d-%02d:%02d",
                mstr->session.hfhm / 100, mstr->session.hfhm % 100,
                mstr->session.hthm / 100, mstr->session.hthm % 100);
        pushstr("HTHM", tmpb, FC_WHITE, push, seqn);
        pushint("FEED", mstr->feed, FC_WHITE, push, seqn);
        pushint("CHID", mstr->chid, FC_WHITE, push, seqn);
        pushymd("PYMD", mstr->pymd, FC_WHITE, push, seqn);
        pushymd("TYMD", mstr->tymd, FC_WHITE, push, seqn);
        pushymd("OYMD", mstr->oymd, FC_WHITE, push, seqn);
        pushymd("CYMD", mstr->cymd, FC_WHITE, push, seqn);
        pushint("JCHK", mstr->jchk, FC_WHITE, push, seqn);
        pushstr("FLSM", mstr->flsm, FC_WHITE, push, seqn);
        pushstr("SLSM", mstr->slsm, FC_WHITE, push, seqn);
        pushstr("RCST", mstr->rcst, FC_WHITE, push, seqn);
        pushstr("LEGS", mstr->legs, FC_WHITE, push, seqn);
        pushstr("PROD", mstr->prod, FC_WHITE, push, seqn);
        pushint("XAGE", mstr->xage, FC_WHITE, push, seqn);
        if (kst4time)
        {
            fep_kortime(fep, mstr->uymd, mstr->uhms, &kymd, &khms);
            sprintf(str_ymd, "%04d-%02d-%02d (KST)", YEAR(kymd), MONTH(kymd), MDAY(kymd));
            sprintf(str_hms, "%02d:%02d:%02d (KST)", HOUR(khms), MINUTE(khms), SECOND(khms));
            pushstr("UYMD", str_ymd, FC_WHITE, push, seqn);
            pushstr("UHMS", str_hms, FC_WHITE, push, seqn);
        }
        else
        {
            pushymd("UYMD", mstr->uymd, FC_WHITE, push, seqn);
            pushhms("UHMS", mstr->uhms, FC_WHITE, push, seqn);
        }
        pushstr("CONV", "N/A", FC_WHITE, push, seqn);
        pushstr("IMPV", "N/A", FC_WHITE, push, seqn);
        pushstr("LEVG", "N/A", FC_WHITE, push, seqn);
        pushstr("GEAR", "N/A", FC_WHITE, push, seqn);
        pushstr("DELT", "N/A", FC_WHITE, push, seqn);
        pushstr("GAMA", "N/A", FC_WHITE, push, seqn);
        pushstr("VEGA", "N/A", FC_WHITE, push, seqn);
        pushstr("TETA", "N/A", FC_WHITE, push, seqn);
        pushstr("HI52", "N/A", FC_WHITE, push, seqn);
        pushstr("LO52", "N/A", FC_WHITE, push, seqn);
        pushstr("HIAN", "N/A", FC_WHITE, push, seqn);
        pushstr("LOAN", "N/A", FC_WHITE, push, seqn);
    }
    break;
    case 202: // quote
    {
        FOLDER *fold = qptr;
        MDMSTR *mstr = &fold->mstr;
        MDQUOT *quot = &fold->quot;

        if (!push)
            str2fld("oNAME", mstr->enam);
        if (push != 0 && push != 1)
            break;
        xdiv = mstr->xdiv;
        ydiv = mstr->ydiv;
        zdiv = mstr->zdiv;
        sprintf(form, "%%'.%df", mstr->zdiv);
        base = mstr->base;
        pushstr("SYMB", quot->symb, FC_WHITE, push, seqn);
        pushymd("TYMD", quot->tymd, FC_WHITE, push, seqn);
        pushymd("XYMD", quot->xymd, FC_WHITE, push, seqn);
        pushhms("XHMS", quot->xhms, FC_WHITE, push, seqn);
        pushint("SEQN", quot->seqn, FC_WHITE, push, seqn);
        pushymd("KYMD", quot->kymd, FC_WHITE, push, seqn);
        pushhms("KHMS", quot->khms, FC_WHITE, push, seqn);
        pushprc("SETP", quot->setp, FC_WHITE, push, seqn);
        pushymd("SYMD", quot->symd, FC_WHITE, push, seqn);

        pushprc("BASE", quot->base, FC_WHITE, push, seqn);
        pushchk("OPEN", quot->open, FC_WHITE, push, seqn);
        pushchk("HIGH", quot->high, FC_WHITE, push, seqn);
        pushchk("LOW", quot->low, FC_WHITE, push, seqn);
        pushchk("LAST", quot->last, FC_WHITE, push, seqn);
        pushdif("DIFF", quot->diff, FC_WHITE, push, seqn);
        pushrat("RATE", quot->rate, FC_WHITE, push, seqn);
        pushint("SIGN", quot->sign, FC_WHITE, push, seqn);
        pushint("TICK", quot->tick, FC_WHITE, push, seqn);
        pushchr("DIRF", quot->dirf, FC_WHITE, push, seqn);
        pushchr("SIDE", quot->side, FC_WHITE, push, seqn);
        pushint("TSID", quot->tsid, FC_WHITE, push, seqn);
        pushint("EVOL", quot->evol, FC_WHITE, push, seqn);
        pushint("TVOL", quot->tvol, FC_WHITE, push, seqn);

        pushint("BVOL", quot->bvol, FC_WHITE, push, seqn);
        pushint("SVOL", quot->svol, FC_WHITE, push, seqn);
        pushint("BCNT", quot->bcnt, FC_WHITE, push, seqn);
        pushint("SCNT", quot->scnt, FC_WHITE, push, seqn);
        pushhms("OTIM", quot->otim, FC_WHITE, push, seqn);
        pushhms("HTIM", quot->htim, FC_WHITE, push, seqn);
        pushhms("LTIM", quot->ltim, FC_WHITE, push, seqn);

        if (!push)
            rqsymb(mstr->exid, mstr->symb, PUSH_QUOT, seqn);
    }
    break;
    case 203: // market depth
    {
        FOLDER *fold = qptr;
        MDMSTR *mstr;
        MDDEPT *dept;
        int ii, jj;

        mstr = &fold->mstr;
        dept = &fold->dept;
        base = mstr->base;

        if (!push)
            str2fld("oNAME", mstr->enam);
        if (push != 0 && push != 2)
            break;
        xdiv = mstr->xdiv;
        ydiv = mstr->ydiv;
        zdiv = mstr->zdiv;
        sprintf(form, "%%'.%df", mstr->zdiv);

        pushint("LEVL", dept->levl[0], FC_WHITE, push, -1);
        pushint("GBIL", dept->levl[1], FC_WHITE, push, -1);
        pushymd("TYMD", dept->tymd, FC_WHITE, push, -1);
        pushymd("XYMD", dept->xymd, FC_WHITE, push, -1);
        pushhms("XHMS", dept->xhms, FC_WHITE, push, -1);
        pushymd("KYMD", dept->kymd, FC_WHITE, push, -1);
        pushhms("KHMS", dept->khms, FC_WHITE, push, -1);
        for (ii = 0, jj = 9; ii < 11; ii++, jj--)
        {
            pushchk("PASK", dept->ask[jj].pask, FC_WHITE, push, ii);
            if (dept->ask[jj].pask != 0.0)
                pushint("VASK", dept->ask[jj].vask, FC_WHITE, push, ii);
            else
                pushstr("VASK", " ", FC_WHITE, push, ii);
            if (dept->ask[jj].pask != 0.0 && dept->ask[jj].nask != 0)
                pushint("NASK", dept->ask[jj].nask, FC_WHITE, push, ii);
            else
                pushstr("NASK", " ", FC_WHITE, push, ii);
            if (dept->ask[jj].pask != 0.0 && dept->ask[jj].cask != 0)
                pushint("CASK", dept->ask[jj].cask, FC_WHITE, push, ii);
            else
                pushstr("CASK", " ", FC_WHITE, push, ii);

            pushchk("PBID", dept->bid[ii].pbid, FC_WHITE, push, ii);
            if (dept->bid[ii].pbid != 0.0)
                pushint("VBID", dept->bid[ii].vbid, FC_WHITE, push, ii);
            else
                pushstr("VBID", " ", FC_WHITE, push, ii);
            if (dept->bid[ii].pbid != 0.0 && dept->bid[ii].nbid != 0)
                pushint("NBID", dept->bid[ii].nbid, FC_WHITE, push, ii);
            else
                pushstr("NBID", " ", FC_WHITE, push, ii);
            if (dept->bid[ii].pbid != 0.0 && dept->bid[ii].cbid != 0)
                pushint("CBID", dept->bid[ii].cbid, FC_WHITE, push, ii);
            else
                pushstr("CBID", " ", FC_WHITE, push, ii);
        }
        pushint("T.VBID", dept->vbid, FC_WHITE, push, -1);
        pushint("T.VASK", dept->vask, FC_WHITE, push, -1);
        pushint("T.CBID", dept->cbid, FC_WHITE, push, -1);
        pushint("T.CASK", dept->cask, FC_WHITE, push, -1);
        pushint("T.NBID", dept->nbid, FC_WHITE, push, -1);
        pushint("T.NASK", dept->nask, FC_WHITE, push, -1);

        for (ii = 0, jj = 4; ii < 6; ii++, jj--)
        {
            pushchk("IPASK", dept->iask[jj].pask, FC_WHITE, push, ii);
            if (dept->iask[jj].pask != 0.0)
                pushint("IVASK", dept->iask[jj].vask, FC_WHITE, push, ii);
            else
                pushstr("IVASK", " ", FC_WHITE, push, ii);
            if (dept->iask[jj].pask != 0.0 && dept->iask[jj].nask != 0)
                pushint("INASK", dept->iask[jj].nask, FC_WHITE, push, ii);
            else
                pushstr("INASK", " ", FC_WHITE, push, ii);
            if (dept->iask[jj].pask != 0.0 && dept->iask[jj].cask != 0)
                pushint("ICASK", dept->iask[jj].cask, FC_WHITE, push, ii);
            else
                pushstr("ICASK", " ", FC_WHITE, push, ii);

            pushchk("IPBID", dept->ibid[ii].pbid, FC_WHITE, push, ii);
            if (dept->ibid[ii].pbid != 0.0)
                pushint("IVBID", dept->ibid[ii].vbid, FC_WHITE, push, ii);
            else
                pushstr("IVBID", " ", FC_WHITE, push, ii);
            if (dept->ibid[ii].pbid != 0.0 && dept->ibid[ii].nbid != 0)
                pushint("INBID", dept->bid[ii].nbid, FC_WHITE, push, ii);
            else
                pushstr("INBID", " ", FC_WHITE, push, ii);
            if (dept->ibid[ii].pbid != 0.0 && dept->ibid[ii].cbid != 0)
                pushint("ICBID", dept->ibid[ii].cbid, FC_WHITE, push, ii);
            else
                pushstr("ICBID", " ", FC_WHITE, push, ii);
        }

        if (!push)
            rqsymb(mstr->exid, mstr->symb, PUSH_DEPT, -1);
    }
    break;
    case 204: // time & sales
    {
        FOLDER *fold = qptr;
        MDMSTR *mstr = &fold->mstr;
        MDQUOT *quot = &fold->quot;
        MDDEPT *dept = &fold->dept;

        base = mstr->base;
        if (!push)
            str2fld("oNAME", mstr->enam);
        if (push != 0 && push != 1)
            break;
        xdiv = mstr->xdiv;
        ydiv = mstr->ydiv;
        zdiv = mstr->zdiv;
        sprintf(form, "%%'.%df", mstr->zdiv);

        if (push && quot->tick)
        {
            // insert quote to 1st line
            insline(5, 1);
            pushymd("TYMD", quot->tymd, FC_WHITE, 0, 0);
            if (kst4time)
            {
                pushymd("XYMD", quot->kymd, FC_WHITE, 0, 0);
                pushhms("XHMS", quot->khms, FC_WHITE, 0, 0);
            }
            else
            {
                pushymd("XYMD", quot->xymd, FC_WHITE, 0, 0);
                pushhms("XHMS", quot->xhms, FC_WHITE, 0, 0);
            }
            pushchk("LAST", quot->last, FC_WHITE, 0, 0);
            pushdif("DIFF", quot->diff, FC_WHITE, 0, 0);
            pushrat("RATE", quot->rate, FC_WHITE, 0, 0);
            pushchk("PBID", dept->bid[0].pbid, FC_WHITE, 0, 0);
            pushchk("PASK", dept->ask[0].pask, FC_WHITE, 0, 0);
            pushint("EVOL", quot->evol, FC_WHITE, 0, 0);
            pushint("TVOL", quot->tvol, FC_WHITE, 0, 0);
            pushchr("DIRF", quot->dirf, FC_WHITE, 0, 0);
            pushchr("SIDE", quot->side, FC_WHITE, 0, 0);
            break;
        }
        if (push)
            break;
        clreos(5);
        rqsymb(mstr->exid, mstr->symb, PUSH_QUOT, seqn);
    }
    break;
    }
    return (0);
}

//
// pushstr()
//
static void pushstr(char *name, char *str, int color, int push, int seqn)
{
    char fldn[40];

    if (seqn < 0)
    {
        pushfld(name, str, color, push);
        return;
    }
    sprintf(fldn, "%s%03d", name, seqn);
    pushfld(fldn, str, color, push);
}

//
// pushchr()
//
static void pushchr(char *name, int chr, int color, int push, int seqn)
{
    char fldb[8];

    sprintf(fldb, "%c", chr);
    pushstr(name, fldb, color, push, seqn);
}
//
// pushymd()
//
static void pushymd(char *name, int ymd, int color, int push, int seqn)
{
    char fldb[256];

    sprintf(fldb, YYMMDD, YEAR(ymd), MONTH(ymd), MDAY(ymd));
    pushstr(name, fldb, color, push, seqn);
}
static void pushhms(char *name, int hms, int color, int push, int seqn)
{
    char fldb[256];

    sprintf(fldb, HHMMSS, HOUR(hms), MINUTE(hms), SECOND(hms));
    pushstr(name, fldb, color, push, seqn);
}
static void pushint(char *name, int val, int color, int push, int seqn)
{
    char fldb[256];

    sprintf(fldb, "%'d", val);
    pushstr(name, fldb, color, push, seqn);
}

static void pushlng(char *name, long val, int color, int push, int seqn)
{
    char fldb[256];

    sprintf(fldb, "%'ld", val);
    pushstr(name, fldb, color, push, seqn);
}

#if 0
static void pushdbl(char *name, double val, int color, int push, int seqn)
{
	char	fldb[256];

	sprintf(fldb, "%'.f", val);
	pushstr(name, fldb, color, push, seqn);
}
#endif

static void pushprc(char *name, double val, int color, int push, int seqn)
{
    char fldb[256];

    editprc(fldb, val, 0);
    pushstr(name, fldb, color, push, seqn);
}

static void pushany(char *name, double val, int color, int push, int seqn)
{
    char fldb[256];

    editprc(fldb, val, 1);
    pushstr(name, fldb, color, push, seqn);
}

static void pushdif(char *name, double val, int color, int push, int seqn)
{
    char fldb[256];

    if (val > 0.0)
    {
        fldb[0] = '+';
        color = FC_RED;
        editprc(&fldb[1], val, 1);
    }
    else if (val < 0.0)
    {
        color = FC_GREEN;
        editprc(&fldb[0], val, 1);
    }
    else
        editprc(&fldb[0], val, 1);
    pushstr(name, fldb, color, push, seqn);
}
static void pushrat(char *name, double val, int color, int push, int seqn)
{
    char fldb[256];

    if (val > 0.0)
    {
        color = FC_RED;
        sprintf(fldb, "+%.2f", val);
    }
    else if (val < 0.0)
    {
        sprintf(fldb, "%.2f", val);
        color = FC_GREEN;
    }
    else
        sprintf(fldb, "%.2f", val);
    pushstr(name, fldb, color, push, seqn);
}

static void pushchk(char *name, double val, int color, int push, int seqn)
{
    char fldb[256];

    if (val > base && base != 0.0)
        color = FC_RED;
    else if (val < base && base != 0.0)
        color = FC_GREEN;
    editprc(fldb, val, 0);
    pushstr(name, fldb, color, push, seqn);
}

static void pushdept(char *name, double val, int color, int push, int seqn)
{
    char fldb[256];

    if (val > base)
        color = FC_RED;
    else if (val < base)
        color = FC_GREEN;
    editprc(fldb, val, 1);
    pushstr(name, fldb, color, push, seqn);
}

static void pushflt(char *name, double val, int color, int push, int seqn)
{
    char fldb[256];
    int len;
    int ii;

    len = sprintf(fldb, "%.7f", val);
    for (ii = len - 1; ii >= 0; ii--)
    {
        switch (fldb[ii])
        {
        case '0':
            fldb[ii] = '\0';
            continue;
        case '.':
            fldb[ii] = '\0';
            continue;
        default:
            break;
        }
        break;
    }
    pushstr(name, fldb, color, push, seqn);
}

static void editprc(char *fldb, double val, int flag)
{
    char *lptr, tmpb[256];
    double v1, v2;

    switch (xdiv)
    {
    case 8:
    case 32:
        if (val == 0.0 && !flag)
        {
            sprintf(fldb, " ");
            break;
        }
        if (!display10)
        {
            sprintf(tmpb, form, val);
            lptr = strchr(tmpb, '.');
            if (lptr != NULL)
                *lptr = '\0';
            v1 = atof(tmpb);
            v2 = val - v1;

            switch (xdiv)
            {
            case 8: // 1/8 2/8
                v2 *= 0.8;
                v1 += v2;
                switch (ydiv)
                {
                case 1:
                    sprintf(fldb, "%.1f", v1);
                    break;
                default:
                    sprintf(fldb, "%.2f", v1);
                    break;
                }
                break;
            case 32: // 1/32 2/32 4 /32
                v2 *= 0.32;
                v1 += v2;
                switch (ydiv)
                {
                case 1:
                    sprintf(fldb, "%.2f", v1);
                    break;
                case 2:
                    sprintf(fldb, "%.3f", v1);
                    break;
                case 4:
                    sprintf(fldb, "%.4f", v1);
                    break;
                case 8:
                    sprintf(fldb, "%.4f", v1);
                    break;
                }
            }
            if ((lptr = strchr(fldb, '.')) != NULL)
                *lptr = '\'';
            break;
        }
    default:
        if (val == 0.0 && !flag)
            sprintf(fldb, " ");
        else
            sprintf(fldb, form, val);
        break;
    }
}
