#include "context.h"

extern int empty_alert(FEP *fep, PORT *port, char *msgb, char *field_buffer, int field_size, char *field_name, int send_flag);
extern int exch_alert(FEP *fep, PORT *port, char *msgb, char *field_buffer, int field_size, int send_flag);
extern int cross_check(FEP *fep, PORT *port, MDDEPT *depth, int send_flag);
extern int parse_pind(MDMSTR *mstr, char *buffer, int size);
extern int STR2STR(char *to, char *from, int size);
extern int STR2INT(int *to, char *from, int size);
extern int STR2UINT(uint32_t *to, char *from, int size);
extern int STR2FLOAT(double *to, char *from, int size);

int ext_master_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag);
int ext_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag);

/* MASTER */
int ext_future_master_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag);
int ext_option_master_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag);
int ext_spread_master_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag);

/* TRADE */
int ext_quote_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag);
int ext_settle_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag);
int ext_oint_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag);
int ext_depth_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag);

int ext_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
    MDARCH *arch = (MDARCH *)fep->arch;

    if (*class_tag & MASTER)
    {
        ext_master_map(fep, port, msgb, msgl, class_tag);
    }
    else if (*class_tag & TRADE)
    {
        if (arch->mstr_flag == 0)
            ext_trade_map(fep, port, msgb, msgl, class_tag);
    }

    return (0);
}

int ext_master_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
    if (*class_tag & FUTURE)
    {
        ext_future_master_map(fep, port, msgb, msgl, class_tag);
    }
    else if (*class_tag & OPTION)
    {
        ext_option_master_map(fep, port, msgb, msgl, class_tag);
    }
    else if (*class_tag & SPREAD)
    {
        ext_spread_master_map(fep, port, msgb, msgl, class_tag);
    }

    return (0);
}

int ext_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
    if (*class_tag & QUOTE)
    {
        ext_quote_trade_map(fep, port, msgb, msgl, class_tag);
    }
    else if (*class_tag & SETTLE)
    {
        ext_settle_trade_map(fep, port, msgb, msgl, class_tag);
    }
    else if (*class_tag & OINT)
    {
        ext_oint_trade_map(fep, port, msgb, msgl, class_tag);
    }
    else if (*class_tag & DEPTH)
    {
        ext_depth_trade_map(fep, port, msgb, msgl, class_tag);
    }

    return (0);
}

int ext_future_master_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
    EXT_FMST *master = (EXT_FMST *)msgb;
    FOLDER *folder;
    MDMSTR *mstr;
    char code[sizeof(master->code) + 1];

    /* Validation */
    if (-1 == empty_alert(fep, port, msgb, master->code, sizeof(master->code), "code", 0))
        return (-1);
    if (-1 == empty_alert(fep, port, msgb, master->ticd, sizeof(master->ticd), "ticd", 0))
        return (-1);
    if (-1 == empty_alert(fep, port, msgb, master->exch, sizeof(master->exch), "exch", 0))
        return (-1);
    if (-1 == exch_alert(fep, port, msgb, master->exch, sizeof(master->exch), 0))
        return (-1);

    /* Mapping */
    STR2STR(code, master->code, sizeof(master->code));

    folder = newfolder(fep, code, port->host);

    if (folder == NULL)
    {
        fep_log(fep, FL_ERROR, GET_CALLER_FUNCTION(), "Cannot create a new folder for '%s'", code);
        return (-1);
    }

    mstr = &folder->mstr;

    STR2STR(mstr->root, master->ticd, sizeof(master->ticd));
    STR2STR(mstr->symb_desc, master->enam, sizeof(master->enam));
    STR2STR(mstr->exch_code, master->exch, sizeof(master->exch));
    STR2STR(mstr->curr, master->curr, sizeof(master->curr));
    STR2STR(mstr->prod, master->prod, sizeof(master->prod));
    parse_pind(mstr, master->pind, sizeof(master->pind));

    STR2UINT(&mstr->ftdt, master->ftdt, sizeof(master->ftdt));
    STR2UINT(&mstr->ltdt, master->ltdt, sizeof(master->ltdt));
    STR2UINT(&mstr->fddt, master->fddt, sizeof(master->fddt));
    STR2UINT(&mstr->lddt, master->lddt, sizeof(master->lddt));
    STR2UINT(&mstr->bsdt, master->bsdt, sizeof(master->bsdt));
    STR2UINT(&mstr->stdt, master->stdt, sizeof(master->stdt));

    STR2FLOAT(&mstr->setp, master->setp, sizeof(master->setp));
    STR2FLOAT(&mstr->pinc, master->pinc, sizeof(master->pinc));
    STR2FLOAT(&mstr->pmul, master->pmul, sizeof(master->pmul));

    mstr->updated_at = time(NULL);

    return (0);
}

int ext_option_master_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
    EXT_OMST *master = (EXT_OMST *)msgb;
    FOLDER *folder;
    MDMSTR *mstr;
    char code[sizeof(master->code) + 1];

    /* Validation */
    if (-1 == empty_alert(fep, port, msgb, master->code, sizeof(master->code), "code", 0))
        return (-1);
    if (-1 == empty_alert(fep, port, msgb, master->ticd, sizeof(master->ticd), "ticd", 0))
        return (-1);
    if (-1 == empty_alert(fep, port, msgb, master->exch, sizeof(master->exch), "exch", 0))
        return (-1);
    if (-1 == exch_alert(fep, port, msgb, master->exch, sizeof(master->exch), 0))
        return (-1);

    /* Mapping */
    STR2STR(code, master->code, sizeof(master->code));

    folder = newfolder(fep, code, port->host);

    if (folder == NULL)
    {
        fep_log(fep, FL_ERROR, GET_CALLER_FUNCTION(), "Cannot create a new folder for '%s'", code);
        return (-1);
    }

    mstr = &folder->mstr;

    STR2STR(mstr->root, master->ticd, sizeof(master->ticd));
    STR2STR(mstr->symb_desc, master->enam, sizeof(master->enam));
    STR2STR(mstr->otyp, master->type, sizeof(master->type));
    STR2STR(mstr->exch_code, master->exch, sizeof(master->exch));
    STR2STR(mstr->curr, master->curr, sizeof(master->curr));
    STR2STR(mstr->prod, master->prod, sizeof(master->prod));
    parse_pind(mstr, master->pind, sizeof(master->pind));

    STR2UINT(&mstr->ftdt, master->ftdt, sizeof(master->ftdt));
    STR2UINT(&mstr->ltdt, master->ltdt, sizeof(master->ltdt));
    STR2UINT(&mstr->fddt, master->fddt, sizeof(master->fddt));
    STR2UINT(&mstr->lddt, master->lddt, sizeof(master->lddt));
    STR2UINT(&mstr->bsdt, master->bsdt, sizeof(master->bsdt));
    STR2UINT(&mstr->stdt, master->stdt, sizeof(master->stdt));

    STR2FLOAT(&mstr->setp, master->setp, sizeof(master->setp));
    STR2FLOAT(&mstr->strp, master->strp, sizeof(master->strp));
    STR2STR(mstr->stcu, master->strc, sizeof(master->strc));
    STR2FLOAT(&mstr->pinc, master->pinc, sizeof(master->pinc));
    STR2FLOAT(&mstr->pmul, master->disf, sizeof(master->disf));

    STR2UINT(&mstr->cvol, master->cvol, sizeof(master->cvol));
    STR2UINT(&mstr->oint, master->oint, sizeof(master->oint));
    STR2STR(mstr->ucod, master->ucod, sizeof(master->ucod));

    mstr->updated_at = time(NULL);

    return (0);
}

int ext_spread_master_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
    EXT_SMST *master = (EXT_SMST *)msgb;
    FOLDER *folder;
    MDMSTR *mstr;
    char code[sizeof(master->code) + 1];

    /* Validation */
    if (-1 == empty_alert(fep, port, msgb, master->code, sizeof(master->code), "code", 0))
        return (-1);
    if (-1 == empty_alert(fep, port, msgb, master->ticd, sizeof(master->ticd), "ticd", 0))
        return (-1);
    if (-1 == empty_alert(fep, port, msgb, master->exch, sizeof(master->exch), "exch", 0))
        return (-1);
    if (-1 == exch_alert(fep, port, msgb, master->exch, sizeof(master->exch), 0))
        return (-1);

    /* Mapping */
    STR2STR(code, master->code, sizeof(master->code));

    folder = newfolder(fep, code, port->host);

    if (folder == NULL)
    {
        fep_log(fep, FL_ERROR, GET_CALLER_FUNCTION(), "Cannot create a new folder for '%s'", code);
        return (-1);
    }

    mstr = &folder->mstr;

    STR2STR(mstr->root, master->ticd, sizeof(master->ticd));
    STR2STR(mstr->symb_desc, master->enam, sizeof(master->enam));
    STR2STR(mstr->exch_code, master->exch, sizeof(master->exch));
    STR2STR(mstr->curr, master->curr, sizeof(master->curr));
    STR2STR(mstr->prod, master->prod, sizeof(master->prod));
    parse_pind(mstr, master->pind, sizeof(master->pind));

    STR2UINT(&mstr->ftdt, master->ftdt, sizeof(master->ftdt));
    STR2UINT(&mstr->ltdt, master->ltdt, sizeof(master->ltdt));
    STR2UINT(&mstr->bsdt, master->bsdt, sizeof(master->bsdt));

    STR2STR(mstr->flsm, master->flsm, sizeof(master->flsm));
    STR2UINT(&mstr->flmy, master->flmy, sizeof(master->flmy));
    STR2STR(mstr->slsm, master->slsm, sizeof(master->slsm));
    STR2UINT(&mstr->slmy, master->slmy, sizeof(master->slmy));

    STR2FLOAT(&mstr->pinc, master->pinc, sizeof(master->pinc));
    STR2FLOAT(&mstr->pmul, master->pmul, sizeof(master->pmul));

    STR2STR(mstr->legs, master->legs, sizeof(master->legs));

    mstr->updated_at = time(NULL);

    return (0);
}

int ext_quote_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
    EXT_HEAD *head = (EXT_HEAD *)msgb;
    EXT_TRADE *trade = (EXT_TRADE *)&msgb[EXT_HEAD_SZ];
    FOLDER *folder;
    MDQUOT *quote;
    char code[sizeof(head->code) + 1];
    double high, low, open;
    time_t current = time(NULL);

    /* Validation */
    if (-1 == empty_alert(fep, port, msgb, head->code, sizeof(head->code), "code", 0))
        return (-1);
    if (-1 == empty_alert(fep, port, msgb, head->exch, sizeof(head->exch), "exch", 0))
        return (-1);
    if (-1 == exch_alert(fep, port, msgb, head->exch, sizeof(head->exch), 0))
        return (-1);

    /* Mapping */
    STR2STR(code, head->code, sizeof(head->code));

    folder = getfolder(fep, code, port->host);

    if (folder == NULL)
    {
        return (-1);
    }

    quote = &folder->quote;

    STR2UINT(&quote->trdt, trade->trdt, sizeof(trade->trdt));
    STR2UINT(&quote->trtm, trade->trtm, sizeof(trade->trtm));
    STR2FLOAT(&quote->last, trade->last, sizeof(trade->last));
    STR2UINT(&quote->tqty, trade->tqty, sizeof(trade->tqty));
    STR2UINT(&quote->tvol, trade->tvol, sizeof(trade->tvol));

    STR2FLOAT(&open, trade->open, sizeof(trade->open));
    STR2FLOAT(&high, trade->high, sizeof(trade->high));
    STR2FLOAT(&low, trade->lowp, sizeof(trade->lowp));

    if (quote->open != open)
    {
        quote->open = open;
        quote->open_time = current;
    }

    if (quote->high != high)
    {
        quote->high = high;
        quote->high_time = current;
    }

    if (quote->low != low)
    {
        quote->low = low;
        quote->low_time = current;
    }

    STR2UINT(&quote->date, trade->date, sizeof(trade->date));

    quote->updated_at = current;

    quote_log(fep, folder->hostname, *class_tag, "%s V:%d T:%u L:%f O:%f H:%f L:%f", code, quote->tqty, quote->tvol, quote->last, quote->open, quote->high, quote->low);

    return (0);
}

int ext_settle_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
    EXT_HEAD *head = (EXT_HEAD *)msgb;
    EXT_TRADE *trade = (EXT_TRADE *)&msgb[EXT_HEAD_SZ];
    FOLDER *folder;
    MDQUOT *settle;
    char code[sizeof(head->code) + 1];
    time_t current = time(NULL);

    /* Validation */
    if (-1 == empty_alert(fep, port, msgb, head->code, sizeof(head->code), "code", 0))
        return (-1);
    if (-1 == empty_alert(fep, port, msgb, head->exch, sizeof(head->exch), "exch", 0))
        return (-1);
    if (-1 == exch_alert(fep, port, msgb, head->exch, sizeof(head->exch), 0))
        return (-1);

    /* Mapping */
    STR2STR(code, head->code, sizeof(head->code));

    folder = getfolder(fep, code, port->host);

    if (folder == NULL)
    {
        return (-1);
    }

    if (strcmp(trade->stat, "A1") == 0 || strlen(trade->stat) == 0)
    {
        settle = &folder->settle[0];
    }
    else if (strcmp(trade->stat, "A2") == 0)
    {
        settle = &folder->settle[1];
    }
    else if (strcmp(trade->stat, "B1") == 0)
    {
        settle = &folder->settle[2];
    }
    else if (strcmp(trade->stat, "B2") == 0)
    {
        settle = &folder->settle[3];
    }

    STR2UINT(&settle->trdt, trade->trdt, sizeof(trade->trdt));
    STR2UINT(&settle->trtm, trade->trtm, sizeof(trade->trtm));
    STR2FLOAT(&settle->last, trade->last, sizeof(trade->last));
    STR2UINT(&settle->tvol, trade->tvol, sizeof(trade->tvol));

    STR2FLOAT(&settle->open, trade->open, sizeof(trade->open));
    STR2FLOAT(&settle->high, trade->high, sizeof(trade->high));
    STR2FLOAT(&settle->low, trade->lowp, sizeof(trade->lowp));
    STR2UINT(&settle->date, trade->date, sizeof(trade->date));

    settle->updated_at = current;

    quote_log(fep, folder->hostname, *class_tag, "%s T:%u L:%f O:%f H:%f L:%f", code, settle->tvol, settle->last, settle->open, settle->high, settle->low);

    return (0);
}

int ext_oint_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
    EXT_HEAD *head = (EXT_HEAD *)msgb;
    EXT_TRADE *trade = (EXT_TRADE *)&msgb[EXT_HEAD_SZ];
    FOLDER *folder;
    MDQUOT *oint;
    char code[sizeof(head->code) + 1];
    time_t current = time(NULL);

    /* Validation */
    if (-1 == empty_alert(fep, port, msgb, head->code, sizeof(head->code), "code", 0))
        return (-1);
    if (-1 == empty_alert(fep, port, msgb, head->exch, sizeof(head->exch), "exch", 0))
        return (-1);
    if (-1 == exch_alert(fep, port, msgb, head->exch, sizeof(head->exch), 0))
        return (-1);

    /* Mapping */
    STR2STR(code, head->code, sizeof(head->code));

    folder = getfolder(fep, code, port->host);

    if (folder == NULL)
    {
        return (-1);
    }

    oint = &folder->oint;

    STR2UINT(&oint->trdt, trade->trdt, sizeof(trade->trdt));
    STR2UINT(&oint->trtm, trade->trtm, sizeof(trade->trtm));
    STR2FLOAT(&oint->last, trade->last, sizeof(trade->last));
    STR2UINT(&oint->tvol, trade->tvol, sizeof(trade->tvol));

    STR2FLOAT(&oint->open, trade->open, sizeof(trade->open));
    STR2FLOAT(&oint->high, trade->high, sizeof(trade->high));
    STR2FLOAT(&oint->low, trade->lowp, sizeof(trade->lowp));
    STR2UINT(&oint->date, trade->date, sizeof(trade->date));

    oint->updated_at = current;

    quote_log(fep, folder->hostname, *class_tag, "%s T:%u L:%f O:%f H:%f L:%f", code, oint->tvol, oint->last, oint->open, oint->high, oint->low);

    return (0);
}

int ext_depth_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
    EXT_HEAD *head = (EXT_HEAD *)msgb;
    EXT_DEPTH *trade = (EXT_DEPTH *)&msgb[EXT_HEAD_SZ];
    FOLDER *folder;
    MDDEPT *depth;
    char code[sizeof(head->code) + 1];
    time_t current = time(NULL);
    int ii;

    /* Validation */
    if (-1 == empty_alert(fep, port, msgb, head->code, sizeof(head->code), "code", 0))
        return (-1);
    if (-1 == empty_alert(fep, port, msgb, head->exch, sizeof(head->exch), "exch", 0))
        return (-1);
    if (-1 == exch_alert(fep, port, msgb, head->exch, sizeof(head->exch), 0))
        return (-1);

    /* Mapping */
    STR2STR(code, head->code, sizeof(head->code));

    folder = getfolder(fep, code, port->host);

    if (folder == NULL)
    {
        return (-1);
    }

    depth = &folder->depth;

    STR2UINT(&depth->dpdt, trade->dpdt, sizeof(trade->dpdt));
    STR2UINT(&depth->dptm, trade->dptm, sizeof(trade->dptm));

    for (ii = 0; ii < MD_MAX_DEPTH; ii++)
    {
        STR2FLOAT(&depth->ask[ii].price, trade->book[ii].askp, sizeof(trade->book[ii].askp));
        STR2UINT(&depth->ask[ii].total, trade->book[ii].askn, sizeof(trade->book[ii].askn));
        STR2UINT(&depth->ask[ii].qtqy, trade->book[ii].askq, sizeof(trade->book[ii].askq));

        STR2FLOAT(&depth->bid[ii].price, trade->book[ii].bidp, sizeof(trade->book[ii].bidp));
        STR2UINT(&depth->bid[ii].total, trade->book[ii].bidn, sizeof(trade->book[ii].bidn));
        STR2UINT(&depth->bid[ii].qtqy, trade->book[ii].bidq, sizeof(trade->book[ii].bidq));
    }

    /* Depth Cross Check */
    cross_check(fep, port, depth, fep->config.raw_data.cross_check);

    depth->updated_at = current;

    return (0);
}