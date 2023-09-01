#include "context.h"

extern int empty_alert(FEP *fep, PORT *port, char *msgb, char *field_buffer, int field_size, char *field_name, int send_flag);
extern int exch_alert(FEP *fep, PORT *port, char *msgb, char *field_buffer, int field_size, int send_flag);
extern int cross_check(FEP *fep, PORT *port, MDDEPT *depth, int send_flag);
extern int parse_pind(MDMSTR *mstr, char *buffer, int size);
extern int STR2STR(char *to, char *from, int size);
extern int STR2INT(int *to, char *from, int size);
extern int STR2UINT(uint32_t *to, char *from, int size);
extern int STR2FLOAT(double *to, char *from, int size);

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
int old_lme_quote_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag);
int old_cancel_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag);
int old_settle_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag);
int old_lme_settle_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag);
int old_close_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag);
int old_oint_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag);
int old_lme_oint_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag);
int old_depth_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag);
int old_fnd_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag);
int old_mavg_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag);
int old_offi_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag);
int old_ware_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag);
int old_volm_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag);

int old_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
   MDARCH *arch = (MDARCH *)fep->arch;

   if (*class_tag & MASTER)
   {
      old_master_map(fep, port, msgb, msgl, class_tag);
   }
   else if (*class_tag & TRADE)
   {
      if (arch->mstr_flag == 0)
      {
         old_trade_map(fep, port, msgb, msgl, class_tag);
      }
   }

   return (0);
}

int old_master_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
   if (*class_tag & LME) // LME 마스터
   {
      old_lme_master_map(fep, port, msgb, msgl, class_tag);
   }
   else if (*class_tag & STOCK) // 주식 마스터
   {
      old_stock_master_map(fep, port, msgb, msgl, class_tag);
   }
   else if (*class_tag & FUTURE) // 선물 마스터
   {
      old_future_master_map(fep, port, msgb, msgl, class_tag);
   }
   else if (*class_tag & OPTION) // 옵션 마스터
   {
      old_option_master_map(fep, port, msgb, msgl, class_tag);
   }
   else if (*class_tag & SPREAD) // 스프레드 마스터
   {
      old_spread_master_map(fep, port, msgb, msgl, class_tag);
   }

   return (0);
}

int old_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
   if (*class_tag & STATUS) // 시세(STATUS)
   {
      old_status_trade_map(fep, port, msgb, msgl, class_tag);
   }
   else if (*class_tag & QUOTE) // 시세(T21)
   {
      if (*class_tag & LME)
         old_lme_quote_trade_map(fep, port, msgb, msgl, class_tag);
      else
         old_quote_trade_map(fep, port, msgb, msgl, class_tag);
   }
   else if (*class_tag & CANCEL) // 시세(T24)
   {
      old_cancel_trade_map(fep, port, msgb, msgl, class_tag);
   }
   else if (*class_tag & SETTLE) // 시세(T40)
   {
      if (*class_tag & LME)
         old_lme_settle_trade_map(fep, port, msgb, msgl, class_tag);
      else
         old_settle_trade_map(fep, port, msgb, msgl, class_tag);
   }
   else if (*class_tag & CLOSE) // 시세(T41)
   {
      old_close_trade_map(fep, port, msgb, msgl, class_tag);
   }
   else if (*class_tag & OINT) // 시세(T50)
   {
      if (*class_tag & LME)
         old_lme_oint_trade_map(fep, port, msgb, msgl, class_tag);
      else
         old_oint_trade_map(fep, port, msgb, msgl, class_tag);
   }
   else if (*class_tag & DEPTH) // 시세(T31)
   {
      old_depth_trade_map(fep, port, msgb, msgl, class_tag);
   }
   else if (*class_tag & FND) // 시세(T80)
   {
      old_fnd_trade_map(fep, port, msgb, msgl, class_tag);
   }
   else if (*class_tag & MAVG) // 시세(T52)
   {
      old_mavg_trade_map(fep, port, msgb, msgl, class_tag);
   }
   else if (*class_tag & OFFI) // 시세(T60)
   {
      old_offi_trade_map(fep, port, msgb, msgl, class_tag);
   }
   else if (*class_tag & WARE) // 시세(T62)
   {
      old_ware_trade_map(fep, port, msgb, msgl, class_tag);
   }
   else if (*class_tag & VOLM) // 시세(T63)
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
   char code[sizeof(master->code) + 1];

   /* Validation */
   if (-1 == empty_alert(fep, port, msgb, master->code, sizeof(master->code), "code", 0))
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

   STR2STR(mstr->symb_desc, master->enam, sizeof(master->enam));
   STR2STR(mstr->exch_code, master->exch, sizeof(master->exch));
   STR2STR(mstr->curr, master->curr, sizeof(master->curr));
   STR2STR(mstr->prod, master->prod, sizeof(master->prod));
   parse_pind(mstr, master->pind, sizeof(master->pind));
   STR2STR(mstr->isin, master->isin, sizeof(master->isin));
   STR2STR(mstr->sedo, master->sedo, sizeof(master->sedo));
   STR2UINT(&mstr->styp, master->styp, sizeof(master->styp));
   STR2UINT(&mstr->bsdt, master->bsdt, sizeof(master->bsdt));
   STR2UINT(&mstr->ptdt, master->pymd, sizeof(master->pymd));
   STR2FLOAT(&mstr->last, master->clos, sizeof(master->clos));
   STR2FLOAT(&mstr->setp, master->base, sizeof(master->base));
   STR2FLOAT(&mstr->pinc, master->pinc, sizeof(master->pinc));
   STR2FLOAT(&mstr->pmul, master->pmul, sizeof(master->pmul));

   mstr->updated_at = time(NULL);

   return (0);
}

int old_future_master_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
   MD_MSTT *master = (MD_MSTT *)msgb;
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

int old_option_master_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
   MD_OSTR *master = (MD_OSTR *)msgb;
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

int old_spread_master_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
   MD_MSTR_SPREAD *master = (MD_MSTR_SPREAD *)msgb;
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

   STR2STR(mstr->legs, &master->fil3[1], 3);

   mstr->updated_at = time(NULL);

   return (0);
}

int old_lme_master_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
   MD_MSTT *master = (MD_MSTT *)msgb;
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

int old_status_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
   MD_HEAD *head = (MD_HEAD *)msgb;
   MD_TRADE_STATUS *trade = (MD_TRADE_STATUS *)&msgb[MD_HEAD_SZ];
   FOLDER *folder;
   MDSTAT *status;
   char code[sizeof(head->code) + 1];

   /* Validation */
   if (-1 == empty_alert(fep, port, msgb, head->code, sizeof(head->code), "code", 0))
      return (-1);
   if (-1 == empty_alert(fep, port, msgb, head->exch, sizeof(head->exch), "exch", 0))
      return (-1);
   if (-1 == empty_alert(fep, port, msgb, trade->bsdt, sizeof(trade->bsdt), "bsdt", 0))
      return (-1);
   if (-1 == empty_alert(fep, port, msgb, trade->stat, sizeof(trade->stat), "stat", 0))
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

   status = &folder->status;

   STR2UINT(&status->bsdt, trade->bsdt, sizeof(trade->bsdt));
   STR2INT(&status->status, trade->stat, sizeof(trade->stat));

   status->updated_at = time(NULL);

   quote_log(fep, folder->hostname, *class_tag, "%s bsdt[%d] status[%d]", code, status->bsdt, status->status);

   return (0);
}

int old_quote_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
   MD_HEAD *head = (MD_HEAD *)msgb;
   MD_TRADE *trade = (MD_TRADE *)&msgb[MD_HEAD_SZ];
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

int old_cancel_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
   MD_HEAD *head = (MD_HEAD *)msgb;
   MD_TRADE *trade = (MD_TRADE *)&msgb[MD_HEAD_SZ];
   FOLDER *folder;
   MDQUOT *quote;
   MDQUOT *cancel;
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
   cancel = &folder->cancel;

   STR2UINT(&quote->trdt, trade->trdt, sizeof(trade->trdt));
   STR2UINT(&quote->trtm, trade->trtm, sizeof(trade->trtm));
   STR2FLOAT(&quote->last, trade->last, sizeof(trade->last));
   STR2UINT(&quote->tqty, trade->tqty, sizeof(trade->tqty));
   STR2UINT(&quote->tvol, trade->tvol, sizeof(trade->tvol));

   STR2UINT(&cancel->trdt, trade->trdt, sizeof(trade->trdt));
   STR2UINT(&cancel->trtm, trade->trtm, sizeof(trade->trtm));
   STR2FLOAT(&cancel->last, trade->last, sizeof(trade->last));
   STR2UINT(&cancel->tqty, trade->tqty, sizeof(trade->tqty));
   STR2UINT(&cancel->tvol, trade->tvol, sizeof(trade->tvol));

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

   if (cancel->open != open)
   {
      cancel->open = open;
      cancel->open_time = current;
   }

   if (cancel->high != high)
   {
      cancel->high = high;
      cancel->high_time = current;
   }

   if (cancel->low != low)
   {
      cancel->low = low;
      cancel->low_time = current;
   }

   STR2UINT(&quote->date, trade->date, sizeof(trade->date));
   STR2UINT(&cancel->date, trade->date, sizeof(trade->date));

   quote->updated_at = current;
   cancel->updated_at = current;

   quote_log(fep, folder->hostname, *class_tag, "%s V:%d T:%u L:%f O:%f H:%f L:%f", code, cancel->tqty, cancel->tvol, cancel->last, cancel->open, cancel->high, cancel->low);

   return (0);
}

int old_settle_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
   MD_HEAD *head = (MD_HEAD *)msgb;
   MD_TRADE *trade = (MD_TRADE *)&msgb[MD_HEAD_SZ];
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

int old_close_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
   MD_HEAD *head = (MD_HEAD *)msgb;
   MD_TRADE *trade = (MD_TRADE *)&msgb[MD_HEAD_SZ];
   FOLDER *folder;
   MDQUOT *close;
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

   close = &folder->close;

   STR2UINT(&close->trdt, trade->trdt, sizeof(trade->trdt));
   STR2UINT(&close->trtm, trade->trtm, sizeof(trade->trtm));
   STR2FLOAT(&close->last, trade->last, sizeof(trade->last));
   STR2UINT(&close->tvol, trade->tvol, sizeof(trade->tvol));

   STR2FLOAT(&close->open, trade->open, sizeof(trade->open));
   STR2FLOAT(&close->high, trade->high, sizeof(trade->high));
   STR2FLOAT(&close->low, trade->lowp, sizeof(trade->lowp));
   STR2UINT(&close->date, trade->date, sizeof(trade->date));

   close->updated_at = current;

   quote_log(fep, folder->hostname, *class_tag, "%s T:%u L:%f O:%f H:%f L:%f", code, close->tvol, close->last, close->open, close->high, close->low);

   return (0);
}

int old_oint_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
   MD_HEAD *head = (MD_HEAD *)msgb;
   MD_TRADE *trade = (MD_TRADE *)&msgb[MD_HEAD_SZ];
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

int old_depth_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
   MD_HEAD *head = (MD_HEAD *)msgb;
   MD_DEPTH *trade = (MD_DEPTH *)&msgb[MD_HEAD_SZ];
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

int old_fnd_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
   return (0);
}

int old_lme_quote_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
   MD_HEAD *head = (MD_HEAD *)msgb;
   LME_TRADE *trade = (LME_TRADE *)&msgb[MD_HEAD_SZ];
   FOLDER *folder;
   MDQUOT *quote;
   char code[sizeof(head->code) + 1];
   time_t current = time(NULL);
   double open, high, low;

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

   STR2FLOAT(&quote->diff, trade->diff, sizeof(trade->diff));
   STR2FLOAT(&quote->pbid, trade->pbid, sizeof(trade->pbid));
   STR2FLOAT(&quote->pask, trade->pask, sizeof(trade->pask));

   STR2UINT(&quote->date, trade->date, sizeof(trade->date));

   quote->updated_at = current;

   quote_log(fep, folder->hostname, *class_tag, "%s V:%d T:%d L:%f O:%f H:%f L:%f diff:%f pb:%f pa:%f dt:%d", code, quote->tqty, quote->tvol, quote->last, quote->open, quote->high, quote->low, quote->diff, quote->pbid, quote->pask, quote->date);

   return (0);
}

int old_lme_settle_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
   MD_HEAD *head = (MD_HEAD *)msgb;
   LME_TRADE *trade = (LME_TRADE *)&msgb[MD_HEAD_SZ];
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

   settle = &folder->settle;

   STR2UINT(&settle->trdt, trade->trdt, sizeof(trade->trdt));
   STR2UINT(&settle->trtm, trade->trtm, sizeof(trade->trtm));
   STR2FLOAT(&settle->last, trade->last, sizeof(trade->last));
   STR2UINT(&settle->tvol, trade->tvol, sizeof(trade->tvol));

   STR2FLOAT(&settle->open, trade->open, sizeof(trade->open));
   STR2FLOAT(&settle->high, trade->high, sizeof(trade->high));
   STR2FLOAT(&settle->low, trade->lowp, sizeof(trade->lowp));

   STR2UINT(&settle->date, trade->date, sizeof(trade->date));

   settle->updated_at = current;

   quote_log(fep, folder->hostname, *class_tag, "%s T:%d L:%f O:%f H:%f L:%f dt:%d", code, settle->tvol, settle->last, settle->open, settle->high, settle->low, settle->date);

   return (0);
}

int old_lme_oint_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
   MD_HEAD *head = (MD_HEAD *)msgb;
   LME_TRADE *trade = (LME_TRADE *)&msgb[MD_HEAD_SZ];
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
   STR2UINT(&oint->tvol, trade->tvol, sizeof(trade->tvol));

   STR2FLOAT(&oint->open, trade->open, sizeof(trade->open));
   STR2FLOAT(&oint->high, trade->high, sizeof(trade->high));
   STR2FLOAT(&oint->low, trade->lowp, sizeof(trade->lowp));

   STR2UINT(&oint->date, trade->date, sizeof(trade->date));

   oint->updated_at = current;

   quote_log(fep, folder->hostname, *class_tag, "%s T:%d L:%f O:%f H:%f L:%f dt:%d", code, oint->tvol, oint->last, oint->open, oint->high, oint->low, oint->date);

   return (0);
}

int old_mavg_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
   MD_HEAD *head = (MD_HEAD *)msgb;
   LME_TRADE *trade = (LME_TRADE *)&msgb[MD_HEAD_SZ];
   FOLDER *folder;
   MDQUOT *mavg;
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

   mavg = &folder->mavg;

   STR2UINT(&mavg->trdt, trade->trdt, sizeof(trade->trdt));
   STR2UINT(&mavg->trtm, trade->trtm, sizeof(trade->trtm));
   STR2FLOAT(&mavg->pbid, trade->pbid, sizeof(trade->pbid));
   STR2FLOAT(&mavg->pask, trade->pask, sizeof(trade->pask));
   STR2UINT(&mavg->date, trade->date, sizeof(trade->date));

   mavg->updated_at = current;

   quote_log(fep, folder->hostname, *class_tag, "%s BID: %f ASK: %f", code, mavg->pbid, mavg->pask);

   return (0);
}

int old_offi_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
   MD_HEAD *head = (MD_HEAD *)msgb;
   LME_TRADE *trade = (LME_TRADE *)&msgb[MD_HEAD_SZ];
   FOLDER *folder;
   MDQUOT *offi;
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

   offi = &folder->offi;

   STR2UINT(&offi->trdt, trade->trdt, sizeof(trade->trdt));
   STR2UINT(&offi->trtm, trade->trtm, sizeof(trade->trtm));
   STR2FLOAT(&offi->uoffi_b, trade->last, sizeof(trade->last));
   STR2FLOAT(&offi->uoffi_s, trade->open, sizeof(trade->open));
   STR2FLOAT(&offi->offi_b, trade->high, sizeof(trade->high));
   STR2FLOAT(&offi->offi_s, trade->lowp, sizeof(trade->lowp));
   offi->updated_at = current;

   quote_log(fep, folder->hostname, *class_tag, "%s UOFFI_B: %f UOFFI_S: %f OFFI_B: %f OFFI_S: %f", code, offi->uoffi_b, offi->uoffi_s, offi->offi_b, offi->offi_s);

   return (0);
}

int old_ware_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
   MD_HEAD *head = (MD_HEAD *)msgb;
   WARE_TRADE *trade = (WARE_TRADE *)&msgb[MD_HEAD_SZ];
   FOLDER *folder;
   MDQUOT *ware;
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

   ware = &folder->ware;

   STR2UINT(&ware->trdt, trade->trdt, sizeof(trade->trdt));
   STR2UINT(&ware->trtm, trade->trtm, sizeof(trade->trtm));

   STR2INT(&ware->wincday, trade->wincday, sizeof(trade->wincday));
   STR2INT(&ware->woutcday, trade->woutcday, sizeof(trade->woutcday));
   STR2INT(&ware->wopen, trade->wopen, sizeof(trade->wopen));
   STR2INT(&ware->wclose, trade->wclose, sizeof(trade->wclose));
   STR2INT(&ware->wchg, trade->wchg, sizeof(trade->wchg));
   STR2INT(&ware->wwclos, trade->wwclos, sizeof(trade->wwclos));
   STR2INT(&ware->wwcanl, trade->wwcanl, sizeof(trade->wwcanl));
   STR2UINT(&ware->date, trade->date, sizeof(trade->date));

   ware->updated_at = current;

   quote_log(fep, folder->hostname, *class_tag, "%s WINCDAY: %d WOUTCDAY: %d WOPEN: %d WCLOSE: %d WCHG: %d WWCLOS: %d WWCANL: %d", code, ware->wincday, ware->woutcday, ware->wopen, ware->wclose, ware->wchg, ware->wwclos, ware->wwcanl);

   return (0);
}

int old_volm_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
   MD_HEAD *head = (MD_HEAD *)msgb;
   WARE_TRADE *trade = (WARE_TRADE *)&msgb[MD_HEAD_SZ];
   FOLDER *folder;
   MDQUOT *volm;
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

   volm = &folder->volm;

   STR2UINT(&volm->trdt, trade->trdt, sizeof(trade->trdt));
   STR2UINT(&volm->trtm, trade->trtm, sizeof(trade->trtm));

   STR2INT(&volm->culmvol, trade->wincday, sizeof(trade->wincday));
   STR2INT(&volm->prevvol, trade->woutcday, sizeof(trade->woutcday));
   STR2UINT(&volm->date, trade->date, sizeof(trade->date));

   volm->updated_at = current;

   quote_log(fep, folder->hostname, *class_tag, "%s CULMVOL: %d PREVVOL: %d", head->code, volm->culmvol, volm->prevvol);

   return (0);
}
