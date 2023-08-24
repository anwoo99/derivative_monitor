/* extpkt.h */
/* 기존 MDF_OLD_FORMAT 의 확장 버전입니다 */

#ifndef EXTPKT_H
#define EXTPKT_H 1

typedef struct
{
    char code[32];  /* Market symbol code                   */
    char ticd[24];  /* Commodity(Ticker) code               */
    char enam[128]; /* Market symbol name                   */
    char exch[3];   /* Exchange section                     */
    char curr[3];   /* currency code                        */
    char prod[3];   /* Product section                      */
    char pind[1];   /* Price indicator                      */
    char ftdt[8];   /* First trading date                   */
    char ltdt[8];   /* Last trading date(expire date)       */
    char fddt[8];   /* First Delivery date                  */
    char lddt[8];   /* Last  Delivery date                  */
    char bsdt[8];   /* Business date                        */
    char stdt[8];   /* Settlement date                      */
    char setp[15];  /* Settlement price(TAG:1150)           */
    char matm[6];   /* Maturity MonthYear                   */
    char pinc[20];  /* MinPriceIncrement                    */
    char pmul[12];  /* price multiplier                     */
    char fill[128]; /* Filler                               */
} EXT_FMST, *Ext_Fmst;
#define EXT_FMST_SZ sizeof(EXT_FMST)

typedef struct
{
    char code[32];  /* Market symbol code                   */
    char ticd[24];  /* Commodity(Ticker) code               */
    char enam[128]; /* Market symbol name                   */
    char type[1];   /* Call/Put				                */
    char exch[3];   /* Exchange section                     */
    char curr[3];   /* currency code                        */
    char prod[3];   /* Product section                      */
    char pind[1];   /* Price indicator                      */
    char ftdt[8];   /* First trading date                   */
    char ltdt[8];   /* Last trading date(expire date)       */
    char fddt[8];   /* First Delivery date                  */
    char lddt[8];   /* Last  Delivery date                  */
    char bsdt[8];   /* Business date                        */
    char stdt[8];   /* Settlement date                      */
    char setp[15];  /* Settlement price(TAG:1150)           */
    char matm[6];   /* Maturity MonthYear                   */
    char strp[20];  /* Strike Price                         */
    char strc[3];   /* Strike Currency                      */
    char pinc[20];  /* MinPriceIncrement                    */
    char disf[20];  /* DisplayFactor                        */
    char cvol[12];  /* ClearedVolume                        */
    char oint[12];  /* OpenInterrestQty                     */
    char ucod[32];  /* Underlying Symbol Code               */
    char fill[128]; /* Filler                               */
} EXT_OMST, *Ext_Omst;
#define EXT_OMST_SZ sizeof(EXT_OMST)

typedef struct
{
    char code[32];  /* Market symbol code                   */
    char ticd[24];  /* Commodity(Ticke) code                */
    char enam[128]; /* Market symbol name                   */
    char exch[3];   /* Exchange section                     */
    char curr[3];   /* currency code                        */
    char prod[3];   /* Product section                      */
    char pind[1];   /* Price indicator                      */
    char ftdt[8];   /* First trading date                   */
    char ltdt[8];   /* Last trading date(expire date)       */
    char bsdt[8];   /* Business date                        */
    char bctk[8];   /* Base Commodity Ticker                */
    char flsm[32];  /* Leg1 Futures Symbol                  */
    char flmy[6];   /* Leg1 Futures Maturity MonthYear      */
    char slsm[32];  /* Leg2 Futures Symbol                  */
    char slmy[6];   /* Leg2 Futures Maturity MonthYear      */
    char pinc[20];  /* minimum price increment              */
    char pmul[12];  /* price multiplier                     */
    char legs[3];   /* Leg Sides (S:Sell / B:Buy)           */
    char fill[128]; /* FILLER                               */
} EXT_SMST, *Ext_Smst;
#define EXT_SMST_SZ sizeof(EXT_SMST)

/* EXT_mcurr.chck */
#define EXT_TRXC_MAST "T10" /* Symbol Master				    */
#define EXT_TRXC_TRAD "T21" /* Market Trade					    */
#define EXT_TRXC_AUCT "T23" /* Auction Trade */
#define EXT_TRXC_DEPT "T31" /* Market Depth					    */
#define EXT_TRXC_SETL "T40" /* SettlePrice					    */
#define EXT_TRXC_STRD "T22" /* SnapShot Trade				    */
#define EXT_TRXC_SDEP "T32" /* SnapShot Depth				    */
#define EXT_TRXC_OINT "T50" /* Open Interest				    */
#define EXT_TRXC_LSET "T41" /* LastPrice or SettlePrice (Sewon)	*/
#define EXT_TRXC_STAT "T60" /* Market Trade Status				*/

/*---------------- Market Data Feed Header Format ------------------*/
typedef struct
{                  /* Market Data Transaction Header		        */
    char trxc[3];  /* Transaction Code						        */
                   /* T10 : Market Symbol Master				    */
                   /* T21 : Market Trade(realtime)-ü��	          */
                   /* T22 : Market Trade(snapshot)-ü��	          */
                   /* T31 : Market Depth(realtime)-ȣ��	          */
                   /* T32 : Market Depth(snapshot)-ȣ��	          */
                   /* T40 : Market SettlePrice-���갡		      */
                   /* T50 : Market Open Interest			        */
    char exch[3];  /* Exchange Code						            */
                   /* E01 : CME Group						        */
                   /*		(include CME/CBOT/NYMEX)		        */
                   /* E02 : Singapore Exchange     		            */
                   /* E03 : Hong Kong Exchage    			        */
                   /* E04 : European Exchange     			        */
                   /* E05 : Tokyo Exchange	(TOCOM)				    */
                   /* E06 : Chicago Board Options Exchange	(CBOE)  */
                   /* E07 : Austailia Securities Exchange	        */
                   /* E08 : London Mechardise Exchange	            */
                   /* E12 : Osaka Securities Exchange	            */
    char dven[1];  /* data vender							        */
                   /* NULL : Exchange Direct				        */
                   /* I : IDC (Interactive data)			        */
                   /* P : PAT Systems						        */
    char date[8];  /* Sending Date (KST)					        */
    char time[12]; /* Sending Time (KST)					        */
    char code[32]; /* Market Stock Code 					        */
    char mseq[10]; /* Message Sequence						        */
    char fill[64]; /* Filler                                        */
} EXT_HEAD, *Ext_Head;
#define EXT_HEAD_SZ sizeof(EXT_HEAD)

/*-------------- Market Data Feed Trade Data Format ----------------*/
typedef struct
{                  /* Market Trade Message					        */
    char trdt[8];  /* Traded Date(KST)						        */
    char trtm[12]; /* Traded Time(KST)						        */
    char last[15]; /* Last Traded Price					            */
    char tqty[9];  /* Last Traded Quantity					        */
    char tvol[9];  /* Last Traded Total Volume				        */
    char open[15]; /* Open Price             				        */
    char high[15]; /* High Price							        */
    char lowp[15]; /* Low Price							            */
    char stat[2];  /* Market Status(?)						        */
    char date[8];  /* Business Date 							    */
    char fill[64]; /* Filler                                        */
} EXT_TRADE, *Ext_Trade;
#define EXT_TRADE_SZ sizeof(EXT_TRADE)

/*-------------- Market Data Feed Depth Data Format ----------------*/
#define EXT_MAX_DEPTH 5

struct ext_book
{
    char bidn[8];  /* Bid Number							*/
    char bidq[8];  /* Bid Quantity							*/
    char bidp[15]; /* Bid Price							    */
    char askn[8];  /* Ask Number							*/
    char askq[8];  /* Ask Quantity							*/
    char askp[15]; /* Ask Number							*/
};

typedef struct
{                  /* Market Depth Message					*/
    char dpdt[8];  /* Trading Depth Date(KST)				*/
    char dptm[12]; /* Trading Depth Time(KST)				*/
    struct ext_book book[EXT_MAX_DEPTH];
    char fill[64]; /* Filler                                */
} EXT_DEPTH, *Ext_Depth;
#define EXT_DEPTH_SZ sizeof(EXT_DEPTH)

/*-------------- Market Data Feed Trade Data Format ----------------*/
typedef struct
{                   /* Market Trade Status Message					*/
    char bsdt[8];   /* Business date                                */
    char stat[2];   /* Trade Status                                 */
    char fill[100]; /* filler                                       */
} EXT_TRADE_STATUS, *Ext_Trade_Status;
#define EXT_TRADE_STATUS_SZ sizeof(EXT_TRADE_STATUS)

#endif
