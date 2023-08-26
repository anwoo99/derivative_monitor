#ifndef _SCHEMA_H
#define _SCHEMA_H

#include <unistd.h>

#define SYMB_LEN 128
#define DEPTH_LEVEL 10

typedef struct
{
    char symb[SYMB_LEN]; /* Symbol */
    char root[SYMB_LEN]; /* Ticker */
    char symb_desc[256]; /* Symbol Description */
    char exch_code[8];   /* Exchange Code */
    char curr[8];        /* Currency Code */
    char prod[8];        /* Production Code */
    char otyp[8];        /* Option Type(Call/Put) */
    char stcu[8];        /* Strike Currency */
    char flsm[SYMB_LEN]; /* First Leg Symbol */
    char slsm[SYMB_LEN]; /* Second Leg Symbol */
    char legs[8];        /* Leg Side */
    char isin[32];       /* ISIN Code */
    char sedo[32];       /* Sedol Code */
    char styp[128];      /* Instrument Type */
    char ucod[SYMB_LEN]; /* Underlying Code */
    double pinc;         /* Price Increment */
    double pmul;         /* Price Multiplier */
    double setp;         /* Settlement Price */
    double last;         /* Last Price */
    double tick;         /* Tick Size */
    double strp;         /* Strike Price */
    uint32_t flmy;       /* First Leg Date */
    uint32_t slmy;       /* Secon Leg Date */
    uint32_t ftdt;       /* First Trading Date */
    uint32_t ltdt;       /* Last Trading Date */
    uint32_t fddt;       /* First Delivery Data*/
    uint32_t lddt;       /* Last Delivery Date */
    uint32_t fndt;       /* First Notice Date */
    uint32_t bsdt;       /* Buisness Date */
    uint32_t ptdt;       /* Previous Trading Date */
    uint32_t stdt;       /* Settlement Date */
    uint32_t cvol;       /* Cleared Volume */
    uint32_t oint;       /* Open Interest */
    uint32_t styp;       /* Instrument Type */
    int main_f;          /* Main Fraction */
    int sub_f;           /* Sub Fraction */
    int zdiv;            /* Decimal Place */
    int seqn;
    time_t updated_at; /* Update Time */
    time_t created_at; /* Created Time */
} MDMSTR;

typedef struct
{
    char symb[SYMB_LEN]; /* Symbol */
    char stat[128];      /* Trade Status */
    uint32_t tqty;       /* trade quantity               */
    uint32_t tvol;       /* trade total volume           */
    uint32_t opin;       /* open interest                */
    uint32_t trdt;       /* Traded Date Info             */
    uint32_t trtm;       /* Traded Time Info             */
    uint32_t date;       /* Date                         */
    double open;         /* open                         */
    double high;         /* High                         */
    double low;          /* Low                          */
    double last;         /* Last                         */
    double diff;         /* diff (signed)                */
    double pask;         /* best ask price               */
    double pbid;         /* best bid price               */
    double uoffi_b;      /* LME Unofficial Buyer Price   */
    double uoffi_s;      /* LME Unofficial Seller Price  */
    double offi_b;       /* LME Official Buyer Price     */
    double offi_s;       /* LME Official Seller Price    */
    int wclose;          /* LME Closing Stock total in warehouse Integer */
    int wopen;           /* LME Opening Stock total in warehouse Integer */
    int wincday;         /* LME Stock delivered into the warehouse on the current day Integer */
    int woutcday;        /* LME Stock delivered out of the warehouse on the current day Integer */
    int wchg;            /* LME Stock movement (difference between in/out) of the warehouse Integer */
    int wwclos;          /* LME Total number of warrants held for stock in the warehouse Integer */
    int wwcanl;          /* LME Total number of warrants that have been cancelled in the warehouse Integer */
    int ytvol;           /* LME Previous days total volume Integer */
    int culmvol;         /* LME Cumulative Trade Volume */
    int prevvol;         /* LME Previous Days Total Volume */
    time_t updated_at;   /* Update Time */
    time_t open_time;    /* Open Price Time */
    time_t high_time;
    time_t low_time;
} MDQUOT;

typedef struct
{
    double price;   /* PRICE */
    uint32_t total; /* Total Volume */
    uint32_t qtqy;  /* Quantity */
} BBOOK;

typedef struct
{
    char symb[SYMB_LEN];    /* Symbol */
    uint32_t dpdt;          /* Depth Date Info             */
    uint32_t dptm;          /* Depth Time Info             */
    BBOOK ask[DEPTH_LEVEL]; /* ASK Information */
    BBOOK bid[DEPTH_LEVEL]; /* BID Information */
    uint32_t cross_cnt;     /* Cross Order Count */
    time_t updated_at;      /* Update Time */
} MDDEPT;

typedef struct
{
    char symb[SYMB_LEN]; /* Symbol */
    uint32_t bsdt;       /* Business Date */
    int status;          /* Trade Status */
    time_t updated_at;   /* Update Time */
} MDSTAT;

typedef struct
{
    char symb[SYMB_LEN]; /* Symbol Name */
    char hostname[128];  /* Hostname */
    time_t created_at;   /* Created Time */
    MDMSTR mstr;         /* Master */
    MDQUOT quote;        /* Trade */
    MDQUOT settle[4];    /* Settlement(A1, B1, A2, B2) Price */
    MDQUOT close;        /* Close Price */
    MDQUOT cancel;       /* Cancel Trade */
    MDQUOT oint;         /* Open Interest */
    MDQUOT mavg;         /* LME Month Average */
    MDQUOT offi;         /* LME Official Unofficial Price */
    MDQUOT ware;         /* LME Warehouse Trade */
    MDQUOT volm;         /* LME Volume Summary */
    MDDEPT depth;        /* Depth */
    MDSTAT status;       /* Trade Status */
} FOLDER;

#endif
