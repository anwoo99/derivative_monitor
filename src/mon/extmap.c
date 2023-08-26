#include "main.h"

extern int empty_alert(FEP *fep, PORT *port, char *msgb, char *field_buffer, char *field_name, int send_flag);
extern int exch_alert(FEP *fep, PORT *port, char *msgb, char *field_buffer, int send_flag);
extern int cross_check(FEP *fep, PORT *port, MDDEPT *depth, int send_flag);

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
    if (*class_tag & MASTER)
    {
        ext_master_map(fep, port, msgb, msgl, class_tag);
    }
    else if (*class_tag & TRADE)
    {
        ext_trade_map(fep, port, msgb, msgl, class_tag);
    }
}

int ext_master_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
    if (*class_tag & FUTURE)
    {
        ext_future_master_map(fep, port, msgb, msgl, class_tag)
    }
    else if (*class_tag & OPTION)
    {
        ext_option_master_map(fep, port, msgb, msgl, class_tag)
    }
    else if (*class_tag & SPREAD)
    {
        ext_spread_master_map(fep, port, msgb, msgl, class_tag)
    }

    return (0);
}

int ext_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
    if (*class_tag & QUOTE)
    {
        ext_quote_trade_map(fep, port, msgb, msgl, class_tag)
    }
    else if (*class_tag & SETTLE)
    {
        ext_settle_trade_map(fep, port, msgb, msgl, class_tag)
    }
    else if (*class_tag & OINT)
    {
        ext_oint_trade_map(fep, port, msgb, msgl, class_tag)
    }
    else if (*class_tag & DEPTH)
    {
        ext_depth_trade_map(fep, port, msgb, msgl, class_tag)
    }

    return (0);
}

int ext_future_master_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
    EXT_FMST *master = (EXT_FMST *)msgb;
    char error_buffer[512];
    int is_valid = 0;
    FOLDER *folder;
    MDMSTR *mstr;

    /* Validation */
    if (-1 == empty_alert(fep, port, msgb, master->code, "code", 0))
        return (-1);
    if (-1 == empty_alert(fep, port, msgb, master->ticd, "ticd", 0))
        return (-1);
    if (-1 == empty_alert(fep, port, msgb, master->exch, "exch", 0))
        return (-1);
    if (-1 == exch_alert(fep, port, msgb, master->exch, 0))
        return (-1);

    /* Mapping */
    folder = newfolder(fep, master->code, port->host);

    if (folder == NULL)
    {
        fep_log(fep, FL_ERROR, "Cannot create a new folder for '%s'", master->code);
        return (-1);
    }

    mstr = &folder->mstr;

    strcpy(mstr->root, master->ticd);
    strcpy(mstr->symb_desc, master->enam);
    strcpy(mstr->exch_code, master->exch);
    strcpy(mstr->curr, master->curr);
    strcpy(mstr->prod, master->prod);
    parse_pind(mstr, master->pind);
    mstr->ftdt = atoi(master->ftdt);
    mstr->ltdt = atoi(master->ltdt);
    mstr->fddt = atoi(master->fddt);
    mstr->lddt = atoi(master->lddt);
    mstr->bsdt = atoi(master->bsdt);
    mstr->stdt = atoi(master->stdt);
    mstr->setp = atof(master->setp);
    mstr->pinc = atof(master->pinc);
    mstr->pmul = atof(master->pmul);

    mstr->updated_at = time(NULL);

    return (0);
}

int ext_option_master_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
    EXT_OMST *master = (EXT_OMST *)msgb;
    char error_buffer[512];
    int is_valid = 0;
    FOLDER *folder;
    MDMSTR *mstr;

    /* Validation */
    if (-1 == empty_alert(fep, port, msgb, master->code, "code", 0))
        return (-1);
    if (-1 == empty_alert(fep, port, msgb, master->ticd, "ticd", 0))
        return (-1);
    if (-1 == empty_alert(fep, port, msgb, master->exch, "exch", 0))
        return (-1);
    if (-1 == exch_alert(fep, port, msgb, master->exch, 0))
        return (-1);

    /* Mapping */
    folder = newfolder(fep, master->code, port->host);

    if (folder == NULL)
    {
        fep_log(fep, FL_ERROR, "Cannot create a new folder for '%s'", master->code);
        return (-1);
    }

    mstr = &folder->mstr;

    strcpy(mstr->root, master->ticd);
    strcpy(mstr->symb_desc, master->enam);
    strcpy(mstr->otyp, master->type);
    strcpy(mstr->exch_code, master->exch);
    strcpy(mstr->curr, master->curr);
    strcpy(mstr->prod, master->prod);
    parse_pind(mstr, master->pind);
    mstr->ftdt = atoi(master->ftdt);
    mstr->ltdt = atoi(master->ltdt);
    mstr->fddt = atoi(master->fddt);
    mstr->lddt = atoi(master->lddt);
    mstr->bsdt = atoi(master->bsdt);
    mstr->stdt = atoi(master->stdt);
    mstr->setp = atof(master->setp);
    mstr->strp = atof(master->strp);
    strcpy(mstr->stcu, master->strc);
    mstr->pinc = atof(master->pinc);
    mstr->pmul = atof(master->disp);
    mstr->cvol = atoi(master->cvol);
    mstr->oint = atoi(master->oint);
    strcpy(mstr->ucod, master->ucod);

    mstr->updated_at = time(NULL);

    return (0);
}

int ext_spread_master_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
    EXT_SMST *master = (EXT_SMST *)msgb;
    char error_buffer[512];
    int is_valid = 0;
    FOLDER *folder;
    MDMSTR *mstr;

    /* Validation */
    if (-1 == empty_alert(fep, port, msgb, master->code, "code", 0))
        return (-1);
    if (-1 == empty_alert(fep, port, msgb, master->ticd, "ticd", 0))
        return (-1);
    if (-1 == empty_alert(fep, port, msgb, master->exch, "exch", 0))
        return (-1);
    if (-1 == exch_alert(fep, port, msgb, master->exch, 0))
        return (-1);

    /* Mapping */
    folder = newfolder(fep, master->code, port->host);

    if (folder == NULL)
    {
        fep_log(fep, FL_ERROR, "Cannot create a new folder for '%s'", master->code);
        return (-1);
    }

    mstr = &folder->mstr;

    strcpy(mstr->root, master->ticd);
    strcpy(mstr->symb_desc, master->enam);
    strcpy(mstr->exch_code, master->exch);
    strcpy(mstr->curr, master->curr);
    strcpy(mstr->prod, master->prod);
    parse_pind(mstr, master->pind);
    mstr->ftdt = atoi(master->ftdt);
    mstr->ltdt = atoi(master->ltdt);
    mstr->bsdt = atoi(master->bsdt);
    strcpy(mstr->flsm, master->flsm);
    mstr->flmy = atoi(master->flmy);
    strcpy(mstr->slsm, master->slsm);
    mstr->slmy = atoi(master->slmy);
    mstr->pinc = atof(master->pinc);
    mstr->pmul = atof(master->pmul);
    strncpy(mstr->legs, &master->fill3[1], 3);

    mstr->updated_at = time(NULL);

    return (0);
}

int ext_quote_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
    EXT_HEAD *head = (EXT_HEAD *)msgb;
    EXT_TRADE *trade = (EXT_TRADE *)&msgb[EXT_HEAD_SZ];
    FOLDER *folder;
    MDQUOT *quote;
    time_t current = time(NULL);

    /* Validation */
    if (-1 == empty_alert(fep, port, msgb, head->code, "code", 0))
        return (-1);
    if (-1 == empty_alert(fep, port, msgb, head->exch, "exch", 0))
        return (-1);
    if (-1 == exch_alert(fep, port, msgb, head->exch, 0))
        return (-1);

    /* Mapping */
    folder = getfolder(fep, head->code, port->host);

    if (folder == NULL)
    {
        fep_log(fep, FL_ERROR, "Cannot get a folder for '%s'", master->code);
        return (-1);
    }

    quote = &folder->quote;

    quote->trdt = atoi(trade->trdt);
    quote->trtm = atoi(trade->trtm);
    quote->last = atof(trade->last);
    quote->tqty = atoi(trade->tqty);

    if (quote->open != atof(trade->open))
    {
        quote->open = atof(trade->open);
        quote->open_time = current;
    }

    if (quote->high != atof(trade->high))
    {
        quote->high = atof(trade->high);
        quote->high_time = current;
    }

    if (quote->low != atof(trade->lowp))
    {
        quote->low = atof(trade->lowp);
        quote->low_time = current;
    }

    quote->date = atoi(trade->date);

    quote->updated_at = current;

    return (0);
}

int ext_settle_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
    EXT_HEAD *head = (EXT_HEAD *)msgb;
    EXT_TRADE *trade = (EXT_TRADE *)&msgb[EXT_HEAD_SZ];
    FOLDER *folder;
    MDQUOT *settle;
    time_t current = time(NULL);

    /* Validation */
    if (-1 == empty_alert(fep, port, msgb, head->code, "code", 0))
        return (-1);
    if (-1 == empty_alert(fep, port, msgb, head->exch, "exch", 0))
        return (-1);
    if (-1 == exch_alert(fep, port, msgb, head->exch, 0))
        return (-1);

    /* Mapping */
    folder = getfolder(fep, head->code, port->host);

    if (folder == NULL)
    {
        fep_log(fep, FL_ERROR, "Cannot get a folder for '%s'", master->code);
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

    settle->trdt = atoi(trade->trdt);
    settle->trtm = atoi(trade->trtm);
    settle->last = atof(trade->last);
    settle->tvol = atoi(trade->tvol);
    settle->open = atof(trade->open);
    settle->high = atof(trade->high);
    settle->low = atof(trade->lowp);
    settle->date = atoi(trade->date);

    settle->updated_at = current;

    return (0);
}

int ext_oint_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
    EXT_HEAD *head = (EXT_HEAD *)msgb;
    EXT_TRADE *trade = (EXT_TRADE *)&msgb[EXT_HEAD_SZ];
    FOLDER *folder;
    MDQUOT *oint;
    time_t current = time(NULL);

    /* Validation */
    if (-1 == empty_alert(fep, port, msgb, head->code, "code", 0))
        return (-1);
    if (-1 == empty_alert(fep, port, msgb, head->exch, "exch", 0))
        return (-1);
    if (-1 == exch_alert(fep, port, msgb, head->exch, 0))
        return (-1);

    /* Mapping */
    folder = getfolder(fep, head->code, port->host);

    if (folder == NULL)
    {
        fep_log(fep, FL_ERROR, "Cannot get a folder for '%s'", master->code);
        return (-1);
    }

    oint = &folder->oint;

    oint->trdt = atoi(trade->trdt);
    oint->trtm = atoi(trade->trtm);
    oint->last = atof(trade->last);
    oint->tvol = atoi(trade->tvol);
    oint->open = atof(trade->open);
    oint->high = atof(trade->high);
    oint->low = atof(trade->lowp);
    oint->date = atoi(trade->date);

    settle->updated_at = current;

    return (0);
}

int ext_depth_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
    EXT_HEAD *head = (EXT_HEAD *)msgb;
    EXT_DEPTH *trade = (EXT_DEPTH *)&msgb[EXT_HEAD_SZ];
    FOLDER *folder;
    MDDEPT *depth;
    time_t current = time(NULL);
    int ii;

    /* Validation */
    if (-1 == empty_alert(fep, port, msgb, head->code, "code", 0))
        return (-1);
    if (-1 == empty_alert(fep, port, msgb, head->exch, "exch", 0))
        return (-1);
    if (-1 == exch_alert(fep, port, msgb, head->exch, 0))
        return (-1);

    /* Mapping */
    folder = getfolder(fep, head->code, port->host);

    if (folder == NULL)
    {
        fep_log(fep, FL_ERROR, "Cannot get a folder for '%s'", master->code);
        return (-1);
    }

    depth = &folder->depth;

    depth->dpdt = atoi(trade->dpdt);
    depth->dptm = atoi(trade->dptm);

    for (ii = 0; ii < MD_MAX_DEPTH; ii++)
    {
        depth->ask[ii].price = atof(trade->book[ii].askp);
        depth->ask[ii].total = atoi(trade->book[ii].askn);
        depth->ask[ii].qtqy = atoi(trade->book[ii].askq);

        depth->bid[ii].price = atof(trade->book[ii].bidp);
        depth->bid[ii].total = atoi(trade->book[ii].bidn);
        depth->bid[ii].qtqy = atoi(trade->book[ii].bidq);
    }

    /* Depth Cross Check */
    cross_check(fep, port, depth, fep->config.raw_data.cross_check);

    depth->updated_at = current;

    return (0);
}