//
// update.c
// Update quote field
//
#include "quot.h"

#define YYMMDD "%04d-%02d-%02d"
#define HHMMSS "%02d:%02d:%02d"
#define HHMM "%02d:%02d"
#define YEAR(x) ((x) / 10000)
#define MONTH(x) ((x) % 10000) / 100
#define MDAY(x) ((x) % 100)
#define HOUR(x) ((x) / 10000)
#define MINUTE(x) ((x) % 10000) / 100
#define SECOND(x) ((x) % 100)

static char wday[7][8] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

static void pushstr(char *name, char *str, int color, int push, int seqn);
static void pushchr(char *name, int chr, int color, int push, int seqn);
static void pushymd(char *name, int ymd, int color, int push, int seqn);
static void pushksttime(char *name, time_t time, int color, int push, int seqn);
static void pushlocaltime(char *name, time_t time, int color, int push, int seqn);
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

static void get_status(int status_code, char *status_str);
static void get_styp(uint32_t styp, char *styp_str);

static char form[16];
static int main_f, sub_f, zdiv;
static double base;
char s_next[80];

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
        MDQUOT *quot = &fold->quote;

        main_f = fold->mstr.main_f;
        sub_f = fold->mstr.sub_f;
        zdiv = fold->mstr.zdiv;

        sprintf(form, "%%'.%df", mstr->zdiv);
        base = mstr->setp;

        pushstr("SYMB", quot->symb, FC_WHITE, push, seqn);
        pushstr("HOST", fold->hostname, FC_WHITE, push, seqn);
        pushchk("OPEN", quot->open, FC_WHITE, push, seqn);
        pushchk("HIGH", quot->high, FC_WHITE, push, seqn);
        pushchk("LOW", quot->low, FC_WHITE, push, seqn);
        pushchk("LAST", quot->last, FC_WHITE, push, seqn);
        pushlng("TVOL", quot->tvol, FC_WHITE, push, seqn);

        if (kst4time)
        {
            pushksttime("QUOT_UTIM", quot->updated_at, FC_WHITE, push, seqn); // 업데이트 시각
        }
        else
        {
            pushlocaltime("QUOT_UTIM", quot->updated_at, FC_WHITE, push, seqn); // 업데이트 시각
        }

        if (!push)
            rqsymb(fep->exnm, quot->symb, fold->hostname, PUSH_QUOT, seqn);

        break;
    }
    case 200: // price composite
    {
        FOLDER *fold = qptr;
        MDMSTR *mstr = &fold->mstr;
        MDQUOT *quot = &fold->quote;
        MDQUOT *settle = fold->settle;
        MDQUOT *close = &fold->close;
        MDQUOT *cancel = &fold->cancel;
        MDQUOT *oint = &fold->oint;
        MDQUOT *mavg = &fold->mavg;
        MDQUOT *offi = &fold->offi;
        MDQUOT *ware = &fold->ware;
        MDQUOT *volm = &fold->depth;
        MDSTAT *status = &fold->status;
        MDDEPT *dept = &fold->depth;
        char status_str[256];
        int ii, jj;

        if (!push)
            str2fld("oNAME", mstr->symb_desc);

        main_f = mstr->main_f;
        sub_f = mstr->sub_f;
        zdiv = mstr->zdiv;
        sprintf(form, "%%'.%df", mstr->zdiv);
        base = mstr->setp;

        if (push == 0 || push == 1)
        {
            /*************************************************************************/
            /* Master */
            pushstr("MAST_SYMB", mstr->symb, FC_WHITE, push, seqn);     // Symbol
            pushint("MAST_MAIN_F", mstr->main_f, FC_WHITE, push, seqn); // Main Fraction
            pushint("MAST_SUB_F", mstr->sub_f, FC_WHITE, push, seqn);   // Sub Fraction
            pushint("MAST_ZDIV", mstr->zdiv, FC_WHITE, push, seqn);     // 소수점자릿수

            if (kst4time)
            {
                pushksttime("MAST_UTIM", mstr->updated_at, FC_WHITE, push, seqn); // 업데이트 시각
                pushksttime("MAST_CTIM", mstr->created_at, FC_WHITE, push, seqn); // 생성 시각
            }
            else
            {
                pushlocaltime("MAST_UTIM", mstr->updated_at, FC_WHITE, push, seqn); // 업데이트 시각
                pushlocaltime("MAST_CTIM", mstr->created_at, FC_WHITE, push, seqn); // 생성 시각
            }
            /*************************************************************************/

            /*************************************************************************/
            /* Quote(T21) */
            pushchk("QUOT_LAST", quot->last, FC_WHITE, push, seqn); // 마지막 체결가
            pushint("QUOT_TQTY", quot->tqty, FC_WHITE, push, seqn); // 실시간 체결량
            pushlng("QUOT_TVOL", quot->tvol, FC_WHITE, push, seqn); // 총 체결량
            pushchk("QUOT_OPEN", quot->open, FC_WHITE, push, seqn); // 시가
            pushchk("QUOT_HIGH", quot->high, FC_WHITE, push, seqn); // 고가
            pushchk("QUOT_LOW", quot->low, FC_WHITE, push, seqn);   // 저가

            if (kst4time)
            {
                pushksttime("QUOT_OTIM", quot->open_time, FC_WHITE, push, seqn);  // 업데이트 시각
                pushksttime("QUOT_HTIM", quot->high_time, FC_WHITE, push, seqn);  // 업데이트 시각
                pushksttime("QUOT_LTIM", quot->low_time, FC_WHITE, push, seqn);   // 업데이트 시각
                pushksttime("QUOT_UTIM", quot->updated_at, FC_WHITE, push, seqn); // 업데이트 시각
            }
            else
            {
                pushlocaltime("QUOT_OTIM", quot->open_time, FC_WHITE, push, seqn);  // 업데이트 시각
                pushlocaltime("QUOT_HTIM", quot->high_time, FC_WHITE, push, seqn);  // 업데이트 시각
                pushlocaltime("QUOT_LTIM", quot->low_time, FC_WHITE, push, seqn);   // 업데이트 시각
                pushlocaltime("QUOT_UTIM", quot->updated_at, FC_WHITE, push, seqn); // 업데이트 시각
            }
            /*************************************************************************/

            /*************************************************************************/
            /* Cancel(T24) */
            pushchk("CANC_LAST", cancel->last, FC_WHITE, push, seqn); // 마지막 체결가
            pushint("CANC_TQTY", cancel->tqty, FC_WHITE, push, seqn); // 실시간 체결량
            pushlng("CANC_TVOL", cancel->tvol, FC_WHITE, push, seqn); // 총 체결량
            pushchk("CANC_OPEN", cancel->open, FC_WHITE, push, seqn); // 시가
            pushchk("CANC_HIGH", cancel->high, FC_WHITE, push, seqn); // 고가
            pushchk("CANC_LOW", cancel->low, FC_WHITE, push, seqn);   // 저가

            if (kst4time)
            {
                pushksttime("CANC_OTIM", cancel->open_time, FC_WHITE, push, seqn);  // 업데이트 시각
                pushksttime("CANC_HTIM", cancel->high_time, FC_WHITE, push, seqn);  // 업데이트 시각
                pushksttime("CANC_LTIM", cancel->low_time, FC_WHITE, push, seqn);   // 업데이트 시각
                pushksttime("CANC_UTIM", cancel->updated_at, FC_WHITE, push, seqn); // 업데이트 시각
            }
            else
            {
                pushlocaltime("CANC_OTIM", cancel->open_time, FC_WHITE, push, seqn);  // 업데이트 시각
                pushlocaltime("CANC_HTIM", cancel->high_time, FC_WHITE, push, seqn);  // 업데이트 시각
                pushlocaltime("CANC_LTIM", cancel->low_time, FC_WHITE, push, seqn);   // 업데이트 시각
                pushlocaltime("CANC_UTIM", cancel->updated_at, FC_WHITE, push, seqn); // 업데이트 시각
            }
            /*************************************************************************/

            /*************************************************************************/
            /* Settlement(T40) */
            pushprc("SETT_PRIC_A1", settle[0].last, FC_WHITE, push, seqn); // 정산가
            pushlng("SETT_TVOL_A1", settle[0].tvol, FC_WHITE, push, seqn); // 총 체결량
            pushprc("SETT_OPEN_A1", settle[0].open, FC_WHITE, push, seqn); // 시가
            pushprc("SETT_HIGH_A1", settle[0].high, FC_WHITE, push, seqn); // 고가
            pushprc("SETT_LOW_A1", settle[0].low, FC_WHITE, push, seqn);   // 저가

            if (kst4time)
            {
                pushksttime("SETT_UTIM_A1", settle[0].updated_at, FC_WHITE, push, seqn); // 업데이트 시각
            }
            else
            {
                pushlocaltime("SETT_UTIM_A1", settle[0].updated_at, FC_WHITE, push, seqn); // 업데이트 시각
            }

            pushprc("SETT_PRIC_B1", settle[1].last, FC_WHITE, push, seqn); // 정산가
            pushlng("SETT_TVOL_B1", settle[1].tvol, FC_WHITE, push, seqn); // 총 체결량
            pushprc("SETT_OPEN_B1", settle[1].open, FC_WHITE, push, seqn); // 시가
            pushprc("SETT_HIGH_B1", settle[1].high, FC_WHITE, push, seqn); // 고가
            pushprc("SETT_LOW_B1", settle[1].low, FC_WHITE, push, seqn);   // 저가

            if (kst4time)
            {
                pushksttime("SETT_UTIM_B1", settle[1].updated_at, FC_WHITE, push, seqn); // 업데이트 시각
            }
            else
            {
                pushlocaltime("SETT_UTIM_B1", settle[1].updated_at, FC_WHITE, push, seqn); // 업데이트 시각
            }

            pushprc("SETT_PRIC_A2", settle[2].last, FC_WHITE, push, seqn); // 정산가
            pushlng("SETT_TVOL_A2", settle[2].tvol, FC_WHITE, push, seqn); // 총 체결량
            pushprc("SETT_OPEN_A2", settle[2].open, FC_WHITE, push, seqn); // 시가
            pushprc("SETT_HIGH_A2", settle[2].high, FC_WHITE, push, seqn); // 고가
            pushprc("SETT_LOW_A2", settle[2].low, FC_WHITE, push, seqn);   // 저가

            if (kst4time)
            {
                pushksttime("SETT_UTIM_A2", settle[2].updated_at, FC_WHITE, push, seqn); // 업데이트 시각
            }
            else
            {
                pushlocaltime("SETT_UTIM_A2", settle[2].updated_at, FC_WHITE, push, seqn); // 업데이트 시각
            }

            pushprc("SETT_PRIC_B2", settle[3].last, FC_WHITE, push, seqn); // 정산가
            pushlng("SETT_TVOL_B2", settle[3].tvol, FC_WHITE, push, seqn); // 총 체결량
            pushprc("SETT_OPEN_B2", settle[3].open, FC_WHITE, push, seqn); // 시가
            pushprc("SETT_HIGH_B2", settle[3].high, FC_WHITE, push, seqn); // 고가
            pushprc("SETT_LOW_B2", settle[3].low, FC_WHITE, push, seqn);   // 저가

            if (kst4time)
            {
                pushksttime("SETT_UTIM_B2", settle[3].updated_at, FC_WHITE, push, seqn); // 업데이트 시각
            }
            else
            {
                pushlocaltime("SETT_UTIM_B2", settle[3].updated_at, FC_WHITE, push, seqn); // 업데이트 시각
            }
            /*************************************************************************/

            /*************************************************************************/
            /* Close Price(T41) */
            pushprc("CLOS_PRIC", close->last, FC_WHITE, push, seqn); // 정산가
            pushlng("CLOS_TVOL", close->tvol, FC_WHITE, push, seqn); // 총 체결량
            pushprc("CLOS_OPEN", close->open, FC_WHITE, push, seqn); // 시가
            pushprc("CLOS_HIGH", close->high, FC_WHITE, push, seqn); // 고가
            pushprc("CLOS_LOW", close->low, FC_WHITE, push, seqn);   // 저가

            if (kst4time)
            {
                pushksttime("SETT_UTIM", close->updated_at, FC_WHITE, push, seqn); // 업데이트 시각
            }
            else
            {
                pushlocaltime("SETT_UTIM", close->updated_at, FC_WHITE, push, seqn); // 업데이트 시각
            }
            /*************************************************************************/

            /*************************************************************************/
            /* Open Interest(T50) */
            pushint("OINT_OPIN", oint->tvol, FC_WHITE, push, seqn); // 미결제약정

            if (kst4time)
            {
                pushksttime("OINT_UTIM", oint->updated_at, FC_WHITE, push, seqn); // 업데이트 시각
            }
            else
            {
                pushlocaltime("OINT_UTIM", oint->updated_at, FC_WHITE, push, seqn); // 업데이트 시각
            }
            /*************************************************************************/

            /*************************************************************************/
            /* LME Month Average(T52) */
            pushprc("MAVG_PBID", mavg->pbid, FC_WHITE, push, seqn); // Monthly Average LME Official Buyer Price
            pushprc("MAVG_PASK", mavg->pask, FC_WHITE, push, seqn); // Monthly Average LME Official Seller Price
            pushymd("MAVG_DATE", mavg->date, FC_WHITE, push, seqn); // Report Date

            if (kst4time)
            {
                pushksttime("MAVG_UTIM", mavg->updated_at, FC_WHITE, push, seqn); // 업데이트 시각
            }
            else
            {
                pushlocaltime("MAVG_UTIM", mavg->updated_at, FC_WHITE, push, seqn); // 업데이트 시각
            }
            /*************************************************************************/

            /*************************************************************************/
            /* LME Official Unofficial Price(T60) */
            pushprc("OFFI_UOFFIB", offi->uoffi_b, FC_WHITE, push, seqn);
            pushprc("OFFI_UOFFIS", offi->uoffi_s, FC_WHITE, push, seqn);
            pushprc("OFFI_OFFIB", offi->offi_b, FC_WHITE, push, seqn);
            pushprc("OFFI_OFFIS", offi->offi_s, FC_WHITE, push, seqn);

            if (kst4time)
            {
                pushksttime("OFFI_UTIM", offi->updated_at, FC_WHITE, push, seqn); // 업데이트 시각
            }
            else
            {
                pushlocaltime("OFFI_UTIM", offi->updated_at, FC_WHITE, push, seqn); // 업데이트 시각
            }
            /*************************************************************************/

            /*************************************************************************/
            /* LME Ware Trade(T62) */
            pushint("WARE_WINCDAY", ware->wincday, FC_WHITE, push, seqn);
            pushint("WARE_WOUTCDAY", ware->woutcday, FC_WHITE, push, seqn);
            pushint("WARE_WOPEN", ware->wopen, FC_WHITE, push, seqn);
            pushint("WARE_WCLOSE", ware->wclose, FC_WHITE, push, seqn);
            pushint("WARE_WCHG", ware->wchg, FC_WHITE, push, seqn);
            pushint("WARE_WWCLOS", ware->wwclos, FC_WHITE, push, seqn);
            pushint("WARE_WWCANL", ware->wwcanl, FC_WHITE, push, seqn);

            if (kst4time)
            {
                pushksttime("WARE_UTIM", offi->updated_at, FC_WHITE, push, seqn); // 업데이트 시각
            }
            else
            {
                pushlocaltime("WARE_UTIM", offi->updated_at, FC_WHITE, push, seqn); // 업데이트 시각
            }
            /*************************************************************************/

            /*************************************************************************/
            /* LME Volume Summary(T63) */
            pushint("VOLM_CULM", volm->culmvol, FC_WHITE, push, seqn);
            pushint("VOLM_PREV", volm->prevvol, FC_WHITE, push, seqn);
            pushymd("VOLM_DATE", volm->date, FC_WHITE, push, seqn);

            if (kst4time)
            {
                pushksttime("VOLM_UTIM", volm->updated_at, FC_WHITE, push, seqn); // 업데이트 시각
            }
            else
            {
                pushlocaltime("VOLM_UTIM", volm->updated_at, FC_WHITE, push, seqn); // 업데이트 시각
            }
            /*************************************************************************/

            /*************************************************************************/
            /* Status(T60) */
            pushymd("STAT_BSDT", status->bsdt, FC_WHITE, push, seqn);
            get_status(status->status, status_str);
            pushstr("STAT_STATUS", status_str, FC_WHITE, push, seqn);

            if (kst4time)
            {
                pushksttime("STAT_UTIM", status->updated_at, FC_WHITE, push, seqn);
            }
            else
            {
                pushlocaltime("STAT_UTIM", status->updated_at, FC_WHITE, push, seqn);
            }
            /*************************************************************************/
        }

        /*************************************************************************/
        /* Depth(T31) */
        if (push == 0 || push == 2)
        {
            if (kst4time)
            {
                pushksttime("DEPT_UTIM", dept->updated_at, FC_WHITE, push, seqn);
            }
            else
            {
                pushlocaltime("DEPT_UTIM", dept->updated_at, FC_WHITE, push, seqn);
            }

            for (ii = 0, jj = 9; ii < 10; ii++, jj--)
            {
                if (dept->ask[jj].total != 0)
                {
                    pushdept("ASK_PRICE", dept->ask[jj].price, FC_WHITE, push, ii); // 매도 가격
                    pushint("ASK_TOTAL", dept->ask[jj].total, FC_WHITE, push, ii);  // 매도 전체 주문량

                    if (dept->ask[jj].qtqy != 0)
                        pushint("ASK_QTQY", dept->ask[jj].qtqy, FC_WHITE, push, ii); // 매도 실시간 주문량
                    else
                        pushstr("ASK_QTQY", " ", FC_WHITE, push, ii);
                }
                else
                {
                    pushstr("ASK_PRICE", " ", FC_WHITE, push, ii);
                    pushstr("ASK_TOTAL", " ", FC_WHITE, push, ii);
                    pushstr("ASK_QTQY", " ", FC_WHITE, push, ii);
                }

                if (dept->bid[ii].total != 0)
                {
                    pushdept("BID_PRICE", dept->bid[ii].price, FC_WHITE, push, ii); // 매수 가격
                    pushint("BID_TOTAL", dept->bid[ii].total, FC_WHITE, push, ii);  // 매수 전체 주문량

                    if (dept->bid[ii].qtqy != 0)
                        pushint("BID_QTQY", dept->bid[ii].qtqy, FC_WHITE, push, ii); // 매수 실시간 주문량
                    else
                        pushstr("BID_QTQY", " ", FC_WHITE, push, ii);
                }
                else
                {
                    pushstr("BID_PRICE", " ", FC_WHITE, push, ii);
                    pushstr("BID_TOTAL", " ", FC_WHITE, push, ii);
                    pushstr("BID_QTQY", " ", FC_WHITE, push, ii);
                }
            }
        }
        /*************************************************************************/

        /*************************************************************************/
        /* Quote(T21) & Cancel(T24) */
        if (push == 1)
        {
            insline(40, 1);

            /* Quote */
            pushhms("T.XHMS", quot->trtm / 1000000, FC_WHITE, 0, 0);
            pushchk("T.LAST", quot->last, FC_WHITE, 0, 0);
            pushint("T.TQTY", quot->tqty, FC_WHITE, 0, 0);
            pushlng("T.TVOL", quot->tvol, FC_WHITE, 0, 0);
            pushchk("T.OPEN", quot->open, FC_WHITE, 0, 0);
            pushchk("T.HIGH", quot->high, FC_WHITE, 0, 0);
            pushchk("T.LOW", quot->low, FC_WHITE, 0, 0);

            /* Cancel */
            pushhms("C.XHMS", cancel->trtm / 1000000, FC_WHITE, 0, 0);
            pushchk("C.LAST", cancel->last, FC_WHITE, 0, 0);
            pushint("C.TQTY", cancel->tqty, FC_WHITE, 0, 0);
            pushlng("C.TVOL", cancel->tvol, FC_WHITE, 0, 0);
            pushchk("C.OPEN", cancel->open, FC_WHITE, 0, 0);
            pushchk("C.HIGH", cancel->high, FC_WHITE, 0, 0);
            pushchk("C.LOW", cancel->low, FC_WHITE, 0, 0);
            break;
        }
        /*************************************************************************/

        if (push)
            break;

        clreos(40);
        rqsymb(fep->exnm, mstr->symb, fold->hostname, PUSH_QUOT | PUSH_DEPT, seqn);
    }
    break;
    case 201: // master
    {
        FOLDER *fold = qptr;
        MDMSTR *mstr = &fold->mstr;
        char styp_str[256];

        if (!push)
            str2fld("oNAME", mstr->symb_desc);

        main_f = mstr->main_f;
        sub_f = mstr->sub_f;
        zdiv = mstr->zdiv;
        sprintf(form, "%%'.%df", mstr->zdiv);

        pushstr("MSTR_SYMB", mstr->symb, FC_WHITE, push, seqn);
        pushstr("MSTR_DESC", mstr->symb_desc, FC_WHITE, push, seqn);
        pushstr("MSTR_ROOT", mstr->root, FC_WHITE, push, seqn);
        pushstr("MSTR_EXCH", mstr->exch_code, FC_WHITE, push, seqn);
        pushstr("MSTR_ISIN", mstr->isin, FC_WHITE, push, seqn);
        pushstr("MSTR_SEDO", mstr->sedo, FC_WHITE, push, seqn);
        pushstr("MSTR_CURR", mstr->curr, FC_WHITE, push, seqn);
        pushstr("MSTR_PROD", mstr->prod, FC_WHITE, push, seqn);
        pushstr("MSTR_OTYP", mstr->otyp, FC_WHITE, push, seqn);
        pushprc("MSTR_STRP", mstr->strp, FC_WHITE, push, seqn);
        pushstr("MSTR_STCU", mstr->stcu, FC_WHITE, push, seqn);
        pushstr("MSTR_UCOD", mstr->ucod, FC_WHITE, push, seqn);
        pushint("MSTR_OINT", mstr->oint, FC_WHITE, push, seqn);
        pushint("MSTR_CVOL", mstr->cvol, FC_WHITE, push, seqn);
        pushstr("MSTR_FLSM", mstr->flsm, FC_WHITE, push, seqn);
        pushstr("MSTR_SLSM", mstr->slsm, FC_WHITE, push, seqn);
        pushymd("MSTR_FLMY", mstr->flmy * 100, FC_WHITE, push, seqn);
        pushymd("MSTR_SLMY", mstr->slmy * 100, FC_WHITE, push, seqn);
        pushstr("MSTR_LEGS", mstr->legs, FC_WHITE, push, seqn);

        get_styp(mstr->styp, styp_str);
        pushstr("MSTR_STYP", styp_str, FC_WHITE, push, seqn);

        pushymd("MSTR_FTDT", mstr->ftdt, FC_WHITE, push, seqn);
        pushymd("MSTR_LTDT", mstr->ltdt, FC_WHITE, push, seqn);
        pushymd("MSTR_FDDT", mstr->fddt, FC_WHITE, push, seqn);
        pushymd("MSTR_LDDT", mstr->fddt, FC_WHITE, push, seqn);
        pushymd("MSTR_BSDT", mstr->bsdt, FC_WHITE, push, seqn);
        pushymd("MSTR_PTDT", mstr->ptdt, FC_WHITE, push, seqn);
        pushymd("MSTR_STDT", mstr->stdt, FC_WHITE, push, seqn);
        pushprc("MSTR_LAST", mstr->last, FC_WHITE, push, seqn);
        pushprc("MSTR_SETP", mstr->setp, FC_WHITE, push, seqn);
        pushprc("MSTR_PINC", mstr->pinc, FC_WHITE, push, seqn);
        pushprc("MSTR_PMUL", mstr->pmul, FC_WHITE, push, seqn);

        if (kst4time)
        {
            pushksttime("MSTR_UTIM", mstr->updated_at, FC_WHITE, push, seqn);
        }
        else
        {
            pushlocaltime("MSTR_UTIM", mstr->updated_at, FC_WHITE, push, seqn);
        }
        break;
    }
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

static void pushksttime(char *name, time_t time, int color, int push, int seqn)
{
    char fldb[256];
    time_t korean_time;
    struct tm korean_tm;

    fep_utc2kst(time, &korean_time, &korean_tm);
    ctime_r(&korean_time, fldb);
    sprintf(fldb, "%s (KST)", fldb);
    pushstr(name, fldb, color, push, seqn);
}

static void pushlocaltime(char *name, time_t time, int color, int push, int seqn)
{
    char fldb[256];

    ctime_r(time, fldb);
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
    int integerPart;
    double decimalPart;
    double fractionalPart;
    int A, B, C;

    if (val == 0.0 && !flag)
        sprintf(fldb, " ");
    else
    {
        if (!display10)
        {
            /* Step1. price 에서 정수 값과 소수점 뒤의 값을 도출한다. */
            integerPart = (int)(val > 0 ? val : -val);
            decimalPart = (val > 0 ? val : -val) - integerPart;

            switch (main_f)
            {
            case 2:
                /* 예외 케이스 */
                fractionalPart = decimalPart == 0.5 ? 5. : 0.;

                if (val > 0)
                    sprintf(fldb, "%d'%1.0f", integerPart, fractionalPart);
                else
                    sprintf(fldb, "-%d'%1.0f", integerPart, fractionalPart);
                break;
            case 4:
                /* 예외 케이스 */
                if (decimalPart == 0)
                    fractionalPart = 0.;
                else if (decimalPart == 0.25)
                    fractionalPart = 2.;
                else if (decimalPart == 0.5)
                    fractionalPart = 5.;
                else if (decimalPart == 0.75)
                    fractionalPart = 7.;

                if (val > 0)
                    sprintf(fldb, "%d'%1.0f", integerPart, fractionalPart);
                else
                    sprintf(fldb, "-%d'%1.0f", integerPart, fractionalPart);
                break;
            default:
                /* Step2. 소수점 뒤의 값과 main_f 값을 곱한다. */
                fractionalPart = decimalPart * main_f;

                /* Step3. 도출된 값에서 Fractional Format에 맞는 값(A, B, C)를 가져온다. */
                A = (int)fractionalPart / 10;        // 십의자리(A)
                B = (int)fractionalPart % 10;        // 일의자리(B)
                C = (int)(fractionalPart * 10) % 10; // 소수점 1자리(C)

                /* Step4. 정수 값과 A, B, C를 Fractional Part Format에 맞추어 합친다.*/
                switch (main_f)
                {
                case 8: // disp = 1
                    if (val > 0)
                        sprintf(fldb, "%d'%d", integerPart, B);
                    else
                        sprintf(fldb, "-%d'%d", integerPart, B);
                    break;
                case 16:
                case 32:
                case 64:
                    if (sub_f == 1) // disp = 2
                    {
                        if (val > 0)
                            sprintf(fldb, "%d'%d%d", integerPart, A, B);
                        else
                            sprintf(fldb, "-%d'%d%d", integerPart, A, B);
                    }
                    else // disp = 3
                    {
                        if (val > 0)
                            sprintf(fldb, "%d'%d%d%d", integerPart, A, B, C);
                        else
                            sprintf(fldb, "-%d'%d%d%d", integerPart, A, B, C);
                    }
                    break;
                default:
                    sprintf(fldb, form, val);
                    break;
                }
                break;
            }
        }
        else
            sprintf(fldb, form, val);
    }
}

static void get_status(int status_code, char *status_str)
{
    if (strcmp(fep->exnm, "ENYS") != 0)
    {
        sprintf(status_str, "N/A");
        return;
    }

    switch (status_code)
    {
    case 0:
        sprintf(status_str, "Resume(%d)", status_code);
        break;
    case 1:
        sprintf(status_str, "Trading halt(%d)", status_code);
        break;
    case 2:
        sprintf(status_str, "Opening Delay(%d)", status_code);
        break;
    case 3:
        sprintf(status_str, "No open / No resume(%d)", status_code);
        break;
    case 5:
        sprintf(status_str, "Early(%d)", status_code);
        break;
    case 6:
        sprintf(status_str, "Late(%d)", status_code);
        break;
    case 10:
        sprintf(status_str, "Short Sale Restriction Activated (Day 1)(%d)", status_code);
        break;
    case 11:
        sprintf(status_str, "Short Sale Restriction Continued (Day 2)(%d)", status_code);
        break;
    case 12:
        sprintf(status_str, "Short Sale Restriction Deactivated(%d)", status_code);
        break;
    case 13:
        sprintf(status_str, "Opened(%d)", status_code);
        break;
    case 14:
        sprintf(status_str, "Pre-opening(%d)", status_code);
        break;
    case 15:
        sprintf(status_str, "Closed(%d)", status_code);
        break;
    case 17:
        sprintf(status_str, "Price Indication(%d)", status_code);
        break;
    case 18:
        sprintf(status_str, "Pre-Opening Price Indication(%d)", status_code);
        break;
    default:
        sprintf(status_str, "N/A");
        break;
    }
}

static void get_styp(uint32_t styp, char *styp_str)
{
    if (strcmp(fep->exnm, "ENYS") != 0)
    {
        sprintf(styp_str, "N/A");
        return;
    }

    switch (styp)
    {
    case 0:
        sprintf(styp_str, "UNDEFINED(%d)", styp);
        break;
    case 256:
        sprintf(styp_str, "EQUITY(%d)", styp);
        break;
    case 257:
        sprintf(styp_str, "COMMON_STOCK(%d)", styp);
        break;
    case 258:
        sprintf(styp_str, "PREFERRED_STOCK(%d)", styp);
        break;
    case 260:
        sprintf(styp_str, "WARRANT(%d)", styp);
        break;
    case 264:
        sprintf(styp_str, "PREMIUM(%d)", styp);
        break;
    case 268:
        sprintf(styp_str, "TRUST(%d)", styp);
        break;
    case 270:
        sprintf(styp_str, "RIGHT(%d)", styp);
        break;
    case 271:
        sprintf(styp_str, "WARRANT_RIGHT(%d)", styp);
        break;
    case 512:
        sprintf(styp_str, "INDEX(%d)", styp);
        break;
    case 768:
        sprintf(styp_str, "UNIT(%d)", styp);
        break;
    case 1024:
        sprintf(styp_str, "COMMODITY(%d)", styp);
        break;
    case 1025:
        sprintf(styp_str, "FUTURE_SPREAD(%d)", styp);
        break;
    case 1280:
        sprintf(styp_str, "FUTURE(%d)", styp);
        break;
    case 1290:
        sprintf(styp_str, "FORWARD(%d)", styp);
        break;
    case 1306:
        sprintf(styp_str, "SPOT(%d)", styp);
        break;
    case 1536:
        sprintf(styp_str, "DEPOSITORY_RECEIPT(%d)", styp);
        break;
    case 2048:
        sprintf(styp_str, "OPTION(%d)", styp);
        break;
    case 2049:
        sprintf(styp_str, "OPTION SPREAD(%d)", styp);
        break;
    case 2304:
        sprintf(styp_str, "EQUITY_OPTION(%d)", styp);
        break;
    case 2560:
        sprintf(styp_str, "INDEX_OPTION(%d)", styp);
        break;
    case 2816:
        sprintf(styp_str, "BINARY_OPTION(%d)", styp);
        break;
    case 3328:
        sprintf(styp_str, "COMMERCIAL_PAPER(%d)", styp);
        break;
    case 3072:
        sprintf(styp_str, "FUTURE_OPTION(%d)", styp);
        break;
    case 3584:
        sprintf(styp_str, "LIMITED_PARTNERSHIP(%d)", styp);
        break;
    case 3840:
        sprintf(styp_str, "NOTE(%d)", styp);
        break;
    case 4096:
        sprintf(styp_str, "FIXED_INCOME(%d)", styp);
        break;
    case 4097:
        sprintf(styp_str, "BOND(%d)", styp);
        break;
    case 4098:
        sprintf(styp_str, "CONVERTIBLE_BOND(%d)", styp);
        break;
    case 4099:
        sprintf(styp_str, "MORTGAGE_BACKED_SECURITY(%d)", styp);
        break;
    case 4100:
        sprintf(styp_str, "GOV_BOND(%d)", styp);
        break;
    case 4104:
        sprintf(styp_str, "CORP_BOND(%d)", styp);
        break;
    case 4112:
        sprintf(styp_str, "US_AGENCY_BOND(%d)", styp);
        break;
    case 4128:
        sprintf(styp_str, "TREASURY_BILL(%d)", styp);
        break;
    case 4160:
        sprintf(styp_str, "US_TREASURY_COUPON(%d)", styp);
        break;
    case 4224:
        sprintf(styp_str, "MONEY_MARKET(%d)", styp);
        break;
    case 4352:
        sprintf(styp_str, "CD(%d)", styp);
        break;
    case 5000:
        sprintf(styp_str, "STRUCTURED_PRODUCT(%d)", styp);
        break;
    case 8192:
        sprintf(styp_str, "MUTUAL_FUND(%d)", styp);
        break;
    case 5102:
        sprintf(styp_str, "INTEREST_RATE_SWAP(%d)", styp);
        break;
    case 5104:
        sprintf(styp_str, "FX_SWAP(%d)", styp);
        break;
    case 5106:
        sprintf(styp_str, "CREDIT_DEFAULT_SWAP(%d)", styp);
        break;
    case 5108:
        sprintf(styp_str, "ASSET_SWAP(%d)", styp);
        break;
    case 5110:
        sprintf(styp_str, "FUTURES_SWAP(%d)", styp);
        break;
    case 5112:
        sprintf(styp_str, "TOTAL_RETURN_SWAP(%d)", styp);
        break;
    case 5194:
        sprintf(styp_str, "SEC_144A(%d)", styp);
        break;
    case 8193:
        sprintf(styp_str, "EXCHANGE_TRADED_FUND(%d)", styp);
        break;
    case 8194:
        sprintf(styp_str, "EXCHANGE_TRADED_NOTE(%d)", styp);
        break;
    case 8196:
        sprintf(styp_str, "CLOSED_END_FUND(%d)", styp);
        break;
    case 8200:
        sprintf(styp_str, "REIT(%d)", styp);
        break;
    case 16384:
        sprintf(styp_str, "FOREX(%d)", styp);
        break;
    case 16385:
        sprintf(styp_str, "FOREX_FRA(%d)", styp);
        break;
    case 16386:
        sprintf(styp_str, "FOREX_DEPOSIT(%d)", styp);
        break;
    case 17408:
        sprintf(styp_str, "FOREX_FORWARD(%d)", styp);
        break;
    case 18432:
        sprintf(styp_str, "FOREX_OPTION(%d)", styp);
        break;
    case 20480:
        sprintf(styp_str, "STATISTICS(%d)", styp);
        break;
    case 65520:
        sprintf(styp_str, "ADMIN(%d)", styp);
        break;
    case 65534:
        sprintf(styp_str, "TEST(%d)", styp);
        break;
    case 65535:
        sprintf(styp_str, "MISCELLANEOUS(%d)", styp);
        break;
    default:
        sprintf(styp_str, "N/A");
        break;
    }
}