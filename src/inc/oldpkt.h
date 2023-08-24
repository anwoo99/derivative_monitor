/*******************************************************************************
 * (C) COPYRIGHT Winway Co., Ltd. 2010
 * All Rights Reserved
 * Licensed Materials - Property of Winway
 *
 * This program contains proprietary information of Winway System.
 * All embodying confidential information, ideas and expressions can't be
 * reproceduced, or transmitted in any form or by any means, electronic,
 * mechanical, or otherwise without the written permission of Winway System.
 *
 *  Components   : mdfmstr.h	-- Market Data Format Master (Definition)
 *  Rev. History :
 *		Ver		Date	Description
 *		-------	-------	-----------------------------------------------
 *		1.00	2010-08	Winway initial version.
 ******************************************************************************/
#ifndef MDFPKT_H
#define MDFPKT_H 1

typedef struct
{
    char code[16]; /* Market symbol code                   */
    char ticd[8];  /* Commodity(Ticker) code               */
    char enam[40]; /* Market symbol name                   */
    char exch[3];  /* Exchange section                     */
    char curr[3];  /* currency code                        */
    char prod[3];  /* Product section                      */
    char pind[1];  /* Price indicator                      */
    char ftdt[8];  /* First trading date                   */
    char ltdt[8];  /* Last trading date(expire date)       */
    char fddt[8];  /* First Delivery date                  */
    char lddt[8];  /* Last  Delivery date                  */
    char bsdt[8];  /* Business date                        */
    char stdt[8];  /* Settlement date                      */
    char setp[15]; /* Settlement price(TAG:1150)           */
    char matm[6];  /* Maturity MonthYear                   */
    char zdiv[2];  /* Number of Decimal Place              */
    char fil2[2];  /* Filler                               */
    char ndig[1];  /* Number of Digits                     */
    char pinc[20]; /* MinPriceIncrement                    */
    char pmul[12]; /* price multiplier                     */
    char fil4[70]; /* Filler                               */
} MD_MSTT, *Md_Mstt;
#define MD_MSTT_SZ sizeof(MD_MSTT)

typedef struct
{
    char code[16]; /* Market symbol code                   */
    char ticd[8];  /* Commodity(Ticker) code               */
    char enam[60]; /* Market symbol name                   */
    char type[1];  /* Call/Put				               */
    char exch[3];  /* Exchange section                     */
    char curr[3];  /* currency code                        */
    char prod[3];  /* Product section                      */
    char pind[1];  /* Price indicator                      */
    char ftdt[8];  /* First trading date                   */
    char ltdt[8];  /* Last trading date(expire date)       */
    char fddt[8];  /* First Delivery date                  */
    char lddt[8];  /* Last  Delivery date                  */
    char bsdt[8];  /* Business date                        */
    char stdt[8];  /* Settlement date                      */
    char setp[15]; /* Settlement price(TAG:1150)           */
    char matm[6];  /* Maturity MonthYear                   */
    char strp[20]; /* Strike Price                         */
    char strc[3];  /* Strike Currency                      */
    char pinc[20]; /* MinPriceIncrement                    */
    char disf[20]; /* DisplayFactor                        */
    char cvol[12]; /* ClearedVolume                        */
    char oint[12]; /* OpenInterrestQty                     */
    char ucod[16]; /* Underlying Symbol Code               */
    char zdiv[2];  /* Number of decimal place              */
    char fill[23]; /* Filler                               */
} MD_OSTR, *Md_Ostr;
#define MD_OSTR_SZ sizeof(MD_OSTR)

typedef struct
{
    char code[16]; /* Market symbol code                   */
    char ticd[8];  /* Commodity(Ticke) code                */
    char enam[40]; /* Market symbol name                   */
    char exch[3];  /* Exchange section                     */
    char curr[3];  /* currency code                        */
    char prod[3];  /* Product section                      */
    char pind[1];  /* Price indicator                      */
    char ftdt[8];  /* First trading date                   */
    char ltdt[8];  /* Last trading date(expire date)       */
    char bsdt[8];  /* Business date                        */
    char bctk[8];  /* Base Commodity Ticker                */
    char flsm[16]; /* Leg1 Futures Symbol                  */
    char flmy[6];  /* Leg1 Futures Maturity MonthYear      */
    char slsm[16]; /* Leg2 Futures Symbol                  */
    char slmy[6];  /* Leg2 Futures Maturity MonthYear      */
    char zdiv[2];  /* Number of decimal place              */
    char fil2[3];  /* FILLER                               */
    char pinc[20]; /* minimum price increment              */
    char pmul[12]; /* price multiplier                     */
    char ndig[1];  /* Number of Digits                     */
    char fil3[69]; /* FILLER                               */
} MD_MSTR_SPREAD, *Md_Mstr_Spread;
#define MD_MSTR_SPREAD_SZ sizeof(MD_MSTR_SPREAD)

typedef struct
{
    char code[16]; /* Market symbol code                    */
    char enam[80]; /* Market symbol name                    */
    char exch[3];  /* Exchange section                      */
    char curr[3];  /* currency code                         */
    char prod[3];  /* Product section                       */
    char pind[1];  /* Price indicator                       */
    char isin[16]; /* ISIN Code			                    */
    char sedo[16]; /* Sedol CODE			                */
    char styp[6];  /* Instrument Type			            */
    char bsdt[8];  /* Business date                         */
    char pymd[8];  /* Previous Trading Date		            */
    char clos[15]; /* Previous Close Price		            */
    char base[15]; /* Base Price			                */
    char pinc[20]; /* MinPriceIncrement                     */
    char pmul[12]; /* price multiplier                      */
    char fil3[78]; /* Filler                                */
} MD_ESTR, *Md_Estr;

/* md_mstr.exch	*/
/* --------------------------------------------------
    E01 : CME Group(include CME/eCBOT/NYMEX)
    E02 : Singapore Exchange
    E03 : Hong Kong Exchage
    E04 : European Exchange
    E05 : Tokyo Stock Exchange
    E06 : Chicago Board Options	Exchange
    E07 : Austailia Securities Exchange
    E12 : Osaka Securities Exchange
-------------------------------------------------- */
#define MD_EXCH_CME "E01"
#define MD_EXCH_SGX "E02"
#define MD_EXCH_HKE "E03"
#define MD_EXCH_ERX "E04"
#define MD_EXCH_TCM "E05"
#define MD_EXCH_OSE "E12"

/* md_mstr.prod	*/
/* --------------------------------------------------
   P10: Foreign exchange(Currency)	: Currency
   P20: Interest rate (Financial)	: Iterest
   P30: Index(Equity)				: Index
   P40: Commodity (Agriculture, ...)	: Commodity
   P50: Metals						: Metals
   P60: Energy						: Energy
   P80: Single Stock
   P90: Etc commodity
-------------------------------------------------- */

/* md_mstr.sstp */
#define MD_MSTP_RTH 1 /* Only RTH								*/
#define MD_MSTP_ETH 2 /* Only ETH								*/
#define MD_MSTP_XTH 3 /* RTH+ETH(T+1):��ð��� �ٸ� 			*/

/* md_mstr.pind */
#define T10_0 '0'  /* 1/1       	999999999  	*/
#define T10_1 '1'  /* 1/10     	99999999.9  */
#define T10_2 '2'  /* 1/100    	9999999.99  */
#define T10_3 '3'  /* 1/1000   	999999.999  */
#define T10_4 '4'  /* 1/10000  	99999.9999  */
#define T10_5 '5'  /* 1/100000 	9999.99999  */
#define T10_6 '6'  /* 1/1000000    999.999999  */
#define T10_7 '7'  /* 1/10000000   99.9999999  */
#define T10_8 '8'  /* 1/100000000  9.99999999  */
#define T2_1 'A'   /* 1/2      	99999999'9  */
#define T4_1 'B'   /* 1/4      	99999999'9  */
#define T8_1 'C'   /* 1/8      	99999999'9  */
#define T16_1 'D'  /* 1/16     	9999999'99  */
#define T32_1 'E'  /* 1/32     	9999999'99  */
#define T64_1 'F'  /* 1/64     	9999999'99  */
#define T128_1 'G' /* 1/128    	999999'999  */
#define T256_1 'H' /* 1/256    	999999'999  */
#define TH32_1 'I' /* 0.5/32   	999999'99.9 */
#define TH64_1 'J' /* 0.5/64   	999999'99.9 */
#define TQ32_1 'K' /* 0.25/32  	999999'99.9 */

/* md_mcurr.chck */
#define MD_TRAD_T1 0x01    /* T and T+1 trading in a day            */
#define MD_TRAD_D2 0x01    /* trading in 2 days				        */
#define MD_TRXC_MAST "T10" /* Symbol Master				            */
#define MD_TRXC_TRAD "T21" /* Market Trade					        */
#define MD_TRXC_STRD "T22" /* SnapShot Trade				        */
#define MD_TRXC_CANC "T24" /* Cancel Trade					        */
#define MD_TRXC_DEPT "T31" /* Market Depth					        */
#define MD_TRXC_SDEP "T32" /* SnapShot Depth				        */
#define MD_TRXC_SETL "T40" /* SettlePrice					        */
#define MD_TRXC_LSET "T41" /* LastPrice or SettlePrice (Sewon)		*/
#define MD_TRXC_OINT "T50" /* Open Interest				            */
#define MD_TRXC_CLOS "T51" /* Official Close Price			        */
#define MD_TRXC_MAVG "T52" /* Monthly Average  			            */
#define MD_TRXC_STAT "T60" /* Market Trade Status				    */
#define MD_TRXC_OFFI "T60" /* Official Unofficial price	            */
#define MD_WARE_MAST "T61" /* WareHouse Master				        */
#define MD_WARE_DATA "T62" /* WareHouse Trad				        */
#define MD_VOLM_SUMM "T63" /* Volume Summary				        */
#define MD_STAT_A1 "A1" /* Calculated Price				            */

/*---------------- Market Data Feed Header Format ------------------*/
typedef struct
{                  /* Market Data Transaction Header		        */
    char trxc[3];  /* Transaction Code						        */
                   /* T10 : Market Symbol Master					*/
                   /* T21 : Market Trade(realtime)	                */
                   /* T22 : Market Trade(snapshot)	                */
                   /* T31 : Market Depth(realtime)	                */
                   /* T32 : Market Depth(snapshot)	                */
                   /* T40 : Market SettlePrice		                */
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
    char fill[1];  /* filler								        */
    char date[8];  /* Sending Date (KST)					        */
    char time[12]; /* Sending Time (KST)					        */
    char code[16]; /* Market Stock Code 					        */
    char mseq[10]; /* Message Sequence						        */
} MD_HEAD, *Md_Head;
#define MD_HEAD_SZ sizeof(MD_HEAD)

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
} MD_TRADE, *Md_Trade;
#define MD_TRADE_SZ sizeof(MD_TRADE)

/*-------------- Market Data Feed Trade Data Format ----------------*/
typedef struct
{                  /* Market Trade Message					        */
    char trdt[8];  /* Traded Date(KST)						        */
    char trtm[12]; /* Traded Time(KST)						        */
    char last[15]; /* Last Traded Price	unofficial buyer	        */
    char tqty[9];  /* Last Traded Quantity	 				        */
    char tvol[9];  /* Last Traded Total Volume				        */
    char open[15]; /* Open Price unofficial seller                  */
    char high[15]; /* High Price official buyer	                    */
    char lowp[15]; /* Low Price official seller	                    */
    char diff[15]; /* CHG                                           */
    char pbid[15]; /* average buyer Price				            */
    char pask[15]; /* average seller Price				            */
    char stat[2];  /* Market Status(?)						        */
    char date[8];  /* Business Date 						        */
} LME_TRADE, *Lme_Trade;
#define LME_TRADE_SZ sizeof(LME_TRADE)

/*-------------- Market Data Feed Trade Data Format ----------------*/
typedef struct
{                      /* Market Trade Message					    */
    char trdt[8];      /* Traded Date(KST)						    */
    char trtm[12];     /* Traded Time(KST)						    */
    char wincday[10];  /* Stock delivered into current day	        */
    char woutcday[10]; /* Stock delivered out  current day          */
    char wopen[10];    /* opening	 		                        */
    char wclose[10];   /* closing	                                */
    char wchg[10];     /* Change   		                            */
    char wwclos[10];   /* warrants close	                        */
    char wwcanl[10];   /* warrants cancel	                        */
    char stat[2];      /* Market Status 						    */
    char date[8];      /* Business Date 						    */
} WARE_TRADE, *Ware_Trade;
#define WARE_TRADE_SZ sizeof(WARE_TRADE)

/*-------------- Market Data Feed Depth Data Format ----------------*/
#define MD_MAX_DEPTH 5

struct book
{
    char bidn[8];  /* Bid Number							        */
    char bidq[8];  /* Bid Quantity							        */
    char bidp[15]; /* Bid Price							            */
    char askn[8];  /* Ask Number							        */
    char askq[8];  /* Ask Quantity							        */
    char askp[15]; /* Ask Number							        */
};

typedef struct
{                  /* Market Depth Message					        */
    char dpdt[8];  /* Trading Depth Date(KST)				        */
    char dptm[12]; /* Trading Depth Time(KST)				        */
    struct book book[MD_MAX_DEPTH];
} MD_DEPTH, *Md_Depth;
#define MD_DEPTH_SZ sizeof(MD_DEPTH)

/*-------------- Market Data Feed Trade Data Format ----------------*/
typedef struct
{                   /* Market Trade Status Message					*/
    char bsdt[8];   /* Business date                                */
    char stat[2];   /* Trade Status                                 */ 
    char fill[100]; /* filler                                       */
} MD_TRADE_STATUS, *Md_Trade_Status;
#define MD_TRADE_STATUS_SZ sizeof(MD_TRADE_STATUS)

#endif /* !defined(MDFMSTR_H) */
