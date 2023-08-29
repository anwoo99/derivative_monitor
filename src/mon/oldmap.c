#include "main.h"

extern int empty_alert(FEP *fep, PORT *port, char *msgb, char *field_buffer, char *field_name, int send_flag);
extern int exch_alert(FEP *fep, PORT *port, char *msgb, char *field_buffer, int send_flag);
extern int cross_check(FEP *fep, PORT *port, MDDEPT *depth, int send_flag);

int old_master_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag);
int old_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag);

/* MASTER */
int old_stock_master_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag);
int old_future_master_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag);
int old_option_master_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag);
int old_spread_master_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag);
int old_lme_master_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag);

/* TRADE */
int old_status_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag);
int old_quote_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag);
int old_cancel_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag);
int old_settle_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag);
int old_close_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag);
int old_oint_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag);
int old_depth_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag);
int old_fnd_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag);
int old_mavg_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag);
int old_offi_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag);
int old_ware_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag);
int old_volm_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag);

int old_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
   if (*class_tag & MASTER)
   {
      old_master_map(fep, port, msgb, msgl, class_tag);
   }
   else if (*class_tag & TRADE)
   {
      old_trade_map(fep, port, msgb, msgl, class_tag);
   }

   return (0);
}

int old_master_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
   if (*class_tag & LME)
   {
      old_lme_master_map(fep, port, msgb, msgl, class_tag);
   }
   else if (*class_tag & STOCK)
   {
      old_stock_master_map(fep, port, msgb, msgl, class_tag);
   }
   else if (*class_tag & FUTURE)
   {
      old_future_master_map(fep, port, msgb, msgl, class_tag);
   }
   else if (*class_tag & OPTION)
   {
      old_option_master_map(fep, port, msgb, msgl, class_tag);
   }
   else if (*class_tag & SPREAD)
   {
      old_spread_master_map(fep, port, msgb, msgl, class_tag);
   }

   return (0);
}

int old_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
   if (*class_tag & STATUS)
   {
      old_status_trade_map(fep, port, msgb, msgl, class_tag);
   }
   else if (*class_tag & QUOTE)
   {
      old_quote_trade_map(fep, port, msgb, msgl, class_tag);
   }
   else if (*class_tag & CANCEL)
   {
      old_cancel_trade_map(fep, port, msgb, msgl, class_tag);
   }
   else if (*class_tag & SETTLE)
   {
      old_settle_trade_map(fep, port, msgb, msgl, class_tag);
   }
   else if (*class_tag & CLOSE)
   {
      old_close_trade_map(fep, port, msgb, msgl, class_tag);
   }
   else if (*class_tag & OINT)
   {
      old_oint_trade_map(fep, port, msgb, msgl, class_tag);
   }
   else if (*class_tag & DEPTH)
   {
      old_depth_trade_map(fep, port, msgb, msgl, class_tag);
   }
   else if (*class_tag & FND)
   {
      old_fnd_trade_map(fep, port, msgb, msgl, class_tag);
   }
   else if (*class_tag & MAVG)
   {
      old_mavg_trade_map(fep, port, msgb, msgl, class_tag);
   }
   else if (*class_tag & OFFI)
   {
      old_offi_trade_map(fep, port, msgb, msgl, class_tag);
   }
   else if (*class_tag & WARE)
   {
      old_ware_trade_map(fep, port, msgb, msgl, class_tag);
   }
   else if (*class_tag & VOLM)
   {
      old_volm_trade_map(fep, port, msgb, msgl, class_tag);
   }

   return (0);
}

int old_stock_master_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
   MD_ESTR *master = (MD_ESTR *)msgb;
   FOLDER *folder;
   MDMSTR *mstr;

   /* Validation */
   if (-1 == empty_alert(fep, port, msgb, master->code, "code", 0))
      return (-1);
   if (-1 == empty_alert(fep, port, msgb, master->exch, "exch", 0))
      return (-1);
   if (-1 == exch_alert(fep, port, msgb, master->exch, 0))
      return (-1);

   /* Mapping */
   folder = newfolder(fep, master->code, port->host);

   if (folder == NULL)
   {
      fep_log(fep, FL_ERROR, GET_CALLER_FUNCTION(), "Cannot create a new folder for '%s'", master->code);
      return (-1);
   }

   mstr = &folder->mstr;

   strcpy(mstr->symb_desc, master->enam);
   strcpy(mstr->exch_code, master->exch);
   strcpy(mstr->curr, master->curr);
   strcpy(mstr->prod, master->prod);
   parse_pind(mstr, master->pind);
   strcpy(mstr->isin, master->isin);
   strcpy(mstr->sedo, master->sedo);
   mstr->styp = atoi(master->styp);
   mstr->bsdt = atoi(master->bsdt);
   mstr->ptdt = atoi(master->pymd);
   mstr->last = atof(master->clos);
   mstr->setp = atof(master->base);
   mstr->pinc = atof(master->pinc);
   mstr->pmul = atof(master->pmul);

   mstr->updated_at = time(NULL);

   return (0);
}

int old_future_master_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
   MD_MSTT *master = (MD_MSTT *)msgb;
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
      fep_log(fep, FL_ERROR, GET_CALLER_FUNCTION(), "Cannot create a new folder for '%s'", master->code);
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
int old_option_master_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
   MD_OSTR *master = (MD_OSTR *)msgb;
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
      fep_log(fep, FL_ERROR, GET_CALLER_FUNCTION(), "Cannot create a new folder for '%s'", master->code);
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
   mstr->pmul = atof(master->disf);
   mstr->cvol = atoi(master->cvol);
   mstr->oint = atoi(master->oint);
   strcpy(mstr->ucod, master->ucod);

   mstr->updated_at = time(NULL);

   return (0);
}

int old_spread_master_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
   MD_MSTR_SPREAD *master = (MD_MSTR_SPREAD *)msgb;
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
      fep_log(fep, FL_ERROR, GET_CALLER_FUNCTION(), "Cannot create a new folder for '%s'", master->code);
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
   strncpy(mstr->legs, &master->fil3[1], 3);

   mstr->updated_at = time(NULL);

   return (0);
}

int old_lme_master_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
   MD_MSTT *master = (MD_MSTT *)msgb;
   char error_buffer[512];
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
      fep_log(fep, FL_ERROR, GET_CALLER_FUNCTION(), "Cannot create a new folder for '%s'", master->code);
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

int old_status_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
   MD_HEAD *head = (MD_HEAD *)msgb;
   MD_TRADE_STATUS *trade = (MD_TRADE_STATUS *)&msgb[MD_HEAD_SZ];
   FOLDER *folder;
   MDSTAT *status;

   /* Validation */
   if (-1 == empty_alert(fep, port, msgb, head->code, "code", 0))
      return (-1);
   if (-1 == empty_alert(fep, port, msgb, head->exch, "exch", 0))
      return (-1);
   if (-1 == empty_alert(fep, port, msgb, trade->bsdt, "bsdt", 0))
      return (-1);
   if (-1 == empty_alert(fep, port, msgb, trade->stat, "stat", 0))
      return (-1);
   if (-1 == exch_alert(fep, port, msgb, head->exch, 0))
      return (-1);

   /* Mapping */
   folder = getfolder(fep, head->code, port->host);

   if (folder == NULL)
   {
      return (-1);
   }

   status = &folder->status;

   status->bsdt = atoi(trade->bsdt);
   status->status = atoi(trade->stat);

   status->updated_at = time(NULL);

   quote_log(fep, folder->hostname, *class_tag, "%s bsdt[%d] status[%d]", head->code, status->bsdt, status->status);

   return (0);
}

int old_quote_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
   MD_HEAD *head = (MD_HEAD *)msgb;
   MD_TRADE *trade = (MD_TRADE *)&msgb[MD_HEAD_SZ];
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
      return (-1);
   }

   quote = &folder->quote;

   quote->trdt = atoi(trade->trdt);
   quote->trtm = atoi(trade->trtm);
   quote->last = atof(trade->last);
   quote->tqty = atoi(trade->tqty);
   quote->tvol = atoi(trade->tvol);

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

   quote_log(fep, folder->hostname, *class_tag, "%s V:%d T:%u L:%f O:%f H:%f L:%f", head->code, quote->tqty, quote->tvol, quote->last, quote->open, quote->high, quote->low);

   return (0);
}

int old_cancel_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
   MD_HEAD *head = (MD_HEAD *)msgb;
   MD_TRADE *trade = (MD_TRADE *)&msgb[MD_HEAD_SZ];
   FOLDER *folder;
   MDQUOT *quote;
   MDQUOT *cancel;
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
      return (-1);
   }

   quote = &folder->quote;
   cancel = &folder->cancel;

   quote->trdt = atoi(trade->trdt);
   quote->trtm = atoi(trade->trtm);
   quote->last = atof(trade->last);
   quote->tqty = atoi(trade->tqty);
   quote->tvol = atoi(trade->tvol);

   cancel->trdt = atoi(trade->trdt);
   cancel->trtm = atoi(trade->trtm);
   cancel->last = atof(trade->last);
   cancel->tqty = atoi(trade->tqty);
   cancel->tvol = atoi(trade->tvol);

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

   if (cancel->open != atof(trade->open))
   {
      cancel->open = atof(trade->open);
      cancel->open_time = current;
   }

   if (cancel->high != atof(trade->high))
   {
      cancel->high = atof(trade->high);
      cancel->high_time = current;
   }

   if (cancel->low != atof(trade->lowp))
   {
      cancel->low = atof(trade->lowp);
      cancel->low_time = current;
   }

   quote->date = atoi(trade->date);
   cancel->date = atoi(trade->date);

   quote->updated_at = current;
   cancel->updated_at = current;

   quote_log(fep, folder->hostname, *class_tag, "%s V:%d T:%u L:%f O:%f H:%f L:%f", head->code, cancel->tqty, cancel->tvol, cancel->last, cancel->open, cancel->high, cancel->low);

   return (0);
}

int old_settle_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
   MD_HEAD *head = (MD_HEAD *)msgb;
   MD_TRADE *trade = (MD_TRADE *)&msgb[MD_HEAD_SZ];
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

   quote_log(fep, folder->hostname, *class_tag, "%s T:%u L:%f O:%f H:%f L:%f", head->code, settle->tvol, settle->last, settle->open, settle->high, settle->low);

   return (0);
}

int old_close_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
   MD_HEAD *head = (MD_HEAD *)msgb;
   MD_TRADE *trade = (MD_TRADE *)&msgb[MD_HEAD_SZ];
   FOLDER *folder;
   MDQUOT *close;
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
      return (-1);
   }

   close = &folder->close;

   close->trdt = atoi(trade->trdt);
   close->trtm = atoi(trade->trtm);
   close->last = atof(trade->last);
   close->tvol = atoi(trade->tvol);
   close->open = atof(trade->open);
   close->high = atof(trade->high);
   close->low = atof(trade->lowp);
   close->date = atoi(trade->date);

   close->updated_at = current;

   quote_log(fep, folder->hostname, *class_tag, "%s T:%u L:%f O:%f H:%f L:%f", head->code, close->tvol, close->last, close->open, close->high, close->low);

   return (0);
}

int old_oint_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
   MD_HEAD *head = (MD_HEAD *)msgb;
   MD_TRADE *trade = (MD_TRADE *)&msgb[MD_HEAD_SZ];
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

   oint->updated_at = current;

   quote_log(fep, folder->hostname, *class_tag, "%s T:%u L:%f O:%f H:%f L:%f", head->code, oint->tvol, oint->last, oint->open, oint->high, oint->low);

   return (0);
}

int old_depth_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
   MD_HEAD *head = (MD_HEAD *)msgb;
   MD_DEPTH *trade = (MD_DEPTH *)&msgb[MD_HEAD_SZ];
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

int old_fnd_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
   return (0);
}

int old_mavg_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
   MD_HEAD *head = (MD_HEAD *)msgb;
   LME_TRADE *trade = (LME_TRADE *)&msgb[MD_HEAD_SZ];
   FOLDER *folder;
   MDQUOT *mavg;
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
      return (-1);
   }

   mavg = &folder->mavg;

   mavg->trdt = atoi(trade->trdt);
   mavg->trtm = atoi(trade->trtm);
   mavg->pbid = atof(trade->pbid);
   mavg->pask = atof(trade->pask);
   mavg->date = atoi(trade->date);

   mavg->updated_at = current;

   quote_log(fep, folder->hostname, *class_tag, "%s BID: %f ASK: %f", head->code, mavg->pbid, mavg->pask);

   return (0);
}

int old_offi_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
   MD_HEAD *head = (MD_HEAD *)msgb;
   LME_TRADE *trade = (LME_TRADE *)&msgb[MD_HEAD_SZ];
   FOLDER *folder;
   MDQUOT *offi;
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
      return (-1);
   }

   offi = &folder->offi;

   offi->trdt = atoi(trade->trdt);
   offi->trtm = atoi(trade->trtm);
   offi->uoffi_b = atof(trade->last);
   offi->uoffi_s = atof(trade->open);
   offi->offi_b = atof(trade->high);
   offi->offi_s = atof(trade->lowp);
   offi->updated_at = current;

   quote_log(fep, folder->hostname, *class_tag, "%s UOFFI_B: %f UOFFI_S: %f OFFI_B: %f OFFI_S: %f", head->code, offi->uoffi_b, offi->uoffi_s, offi->offi_b, offi->offi_s);

   return (0);
}

int old_ware_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
   MD_HEAD *head = (MD_HEAD *)msgb;
   WARE_TRADE *trade = (WARE_TRADE *)&msgb[MD_HEAD_SZ];
   FOLDER *folder;
   MDQUOT *ware;
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
      return (-1);
   }

   ware = &folder->ware;

   ware->trdt = atoi(trade->trdt);
   ware->trtm = atoi(trade->trtm);

   ware->wincday = atoi(trade->wincday);
   ware->woutcday = atoi(trade->woutcday);
   ware->wopen = atoi(trade->wopen);
   ware->wclose = atoi(trade->wclose);
   ware->wchg = atoi(trade->wchg);
   ware->wwclos = atoi(trade->wwclos);
   ware->wwcanl = atoi(trade->wwcanl);
   ware->date = atoi(trade->date);

   ware->updated_at = current;

   quote_log(fep, folder->hostname, *class_tag, "%s WINCDAY: %d WOUTCDAY: %d WOPEN: %d WCLOSE: %d WCHG: %d WWCLOS: %d WWCANL: %d", head->code, ware->wincday, ware->woutcday, ware->wopen, ware->wclose, ware->wchg, ware->wwclos, ware->wwcanl);

   return (0);
}

int old_volm_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
   MD_HEAD *head = (MD_HEAD *)msgb;
   WARE_TRADE *trade = (WARE_TRADE *)&msgb[MD_HEAD_SZ];
   FOLDER *folder;
   MDQUOT *volm;
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
      return (-1);
   }

   volm = &folder->volm;

   volm->trdt = atoi(trade->trdt);
   volm->trtm = atoi(trade->trtm);

   volm->culmvol = atoi(trade->wincday);
   volm->prevvol = atoi(trade->woutcday);
   volm->date = atoi(trade->date);

   volm->updated_at = current;

   quote_log(fep, folder->hostname, *class_tag, "%s CULMVOL: %d PREVVOL: %d", head->code, volm->culmvol, volm->prevvol);

   return (0);
}