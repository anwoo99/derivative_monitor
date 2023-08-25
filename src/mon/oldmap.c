#include "main.h"

int old_master_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag);
int old_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag);

/* MASTER */
int old_stock_master_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag);
int old_future_master_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag);
int old_option_master_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag);
int old_spread_master_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag);
int old_warehouse_master_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag);

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
}

int old_master_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
   if (*class_tag & STOCK)
   {
      old_stock_master_map(fep, port, msgb, msgl, class_tag)
   }
   else if (*class_tag & FUTURE)
   {
      old_future_master_map(fep, port, msgb, msgl, class_tag)
   }
   else if (*class_tag & OPTION)
   {
      old_option_master_map(fep, port, msgb, msgl, class_tag)
   }
   else if (*class_tag & SPREAD)
   {
      old_spread_master_map(fep, port, msgb, msgl, class_tag)
   }
   else if (*class_tag & WAREHOUSE)
   {
      old_warehouse_master_map(fep, port, msgb, msgl, class_tag)
   }

   return (0);
}

int old_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
   if (*class_tag & STATUS)
   {
      old_status_trade_map(fep, port, msgb, msgl, class_tag)
   }
   else if (*class_tag & QUOTE)
   {
      old_quote_trade_map(fep, port, msgb, msgl, class_tag)
   }
   else if (*class_tag & CANCEL)
   {
      old_cancel_trade_map(fep, port, msgb, msgl, class_tag)
   }
   else if (*class_tag & SETTLE)
   {
      old_settle_trade_map(fep, port, msgb, msgl, class_tag)
   }
   else if (*class_tag & CLOSE)
   {
      old_close_trade_map(fep, port, msgb, msgl, class_tag)
   }
   else if (*class_tag & OINT)
   {
      old_oint_trade_map(fep, port, msgb, msgl, class_tag)
   }
   else if (*class_tag & DEPTH)
   {
      old_depth_trade_map(fep, port, msgb, msgl, class_tag)
   }
   else if (*class_tag & FND)
   {
      old_fnd_trade_map(fep, port, msgb, msgl, class_tag)
   }
   else if (*class_tag & MAVG)
   {
      old_mavg_trade_map(fep, port, msgb, msgl, class_tag)
   }
   else if (*class_tag & OFFI)
   {
      old_offi_trade_map(fep, port, msgb, msgl, class_tag)
   }
   else if (*class_tag & WARE)
   {
      old_ware_trade_map(fep, port, msgb, msgl, class_tag)
   }
   else if (*class_tag & VOLM)
   {
      old_volm_trade_map(fep, port, msgb, msgl, class_tag)
   }

   return (0);
}

int old_stock_master_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
   MD_ESTR *emaster = (MD_ESTR *)msgb;
   char error_buffer[512];
   int is_valid = 0;

   memset(message, 0x00, sizeof(message));

   /* Validation */
   if (strlen(emaster->code) < 0)
   {
      is_valid = -1;
   }
   if (strlen())
}

int old_future_master_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
}
int old_option_master_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
}

int old_spread_master_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
}

int old_warehouse_master_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
}

int old_status_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
}

int old_quote_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
}

int old_cancel_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
}

int old_settle_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
}

int old_close_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
}

int old_oint_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
}

int old_depth_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
}

int old_fnd_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
}
int old_mavg_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
}
int old_offi_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
}
int old_ware_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
}
int old_volm_trade_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
}