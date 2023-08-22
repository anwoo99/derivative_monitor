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
    char exch_desc[64];  /* Exchange Description */
    char curr[8];        /* Currency Code */
    char prod[8];        /* Production Code */
    char ftdt[16];       /* First Trading Date */
    char ltdt[16];       /* Last Trading Date */
    char fddt[16];       /* First Delivery Data*/
    char lddt[16];       /* Last Delivery Date */
    char bsdt[16];       /* Buisness Date */
    char stdt[16];       /* Settlement Date */
    char updt[16];       /* Updated Date */
    char otyp[8];        /* Option Type(Call/Put) */
    char stcu[8];        /* Strike Currency */
    char flsm[SYMB_LEN]; /* First Leg Symbol */
    char slsm[SYMB_LEN]; /* Second Leg Symbol */
    char legs[8];        /* Leg Side */
    char isin[32];       /* ISIN Code */
    char sedo[32];       /* Sedol Code */
    char styp[128];      /* Instrument Type */
    double setp;         /* Settlement Price */
    double last;         /* Last Price */
    double tick;         /* Tick Size */
    double strp;         /* Strike Price */
    int main_f;          /* Main Fraction */
    int sub_f;           /* Sub Fraction */
    int zdiv;            /* Decimal Place */
    int seqn;
} MDMSTR;

typedef struct
{
    char symb[SYMB_LEN]; /* Symbol */
    char stat[128];      /* Trade Status */
    uint32_t xymd;       /* date                         */
    uint32_t xhms;       /* time                         */
    uint32_t kymd;       /* local date                   */
    uint32_t khms;       /* local time                   */
    uint32_t tqty;       /* trade quantity               */
    uint32_t tvol;       /* trade total volume           */
    uint32_t opin;       /* open interest                */
    uint32_t date;       /* Date Info                    */
    double open;         /* open                         */
    double high;         /* High                         */
    double low;          /* Low                          */
    double last;         /* Last                         */
    double diff;         /* diff (signed)                */
    double pask;         /* best ask price               */
    double pbid;         /* best bid price               */
} MDQUOT;

typedef struct
{
    double price;  /* PRICE */
    uint32_t tvol; /* Total Volume */
    uint32_t qty;  /* Quantity */
} BBOOK;

typedef struct
{
    char symb[SYMB_LEN];    /* Symbol */
    uint32_t xymd;          /* date                         */
    uint32_t xhms;          /* time                         */
    uint32_t kymd;          /* local date                   */
    uint32_t khms;          /* local time                   */
    BBOOK ask[DEPTH_LEVEL]; /* ASK Information */
    BBOOK bid[DEPTH_LEVEL]; /* BID Information */
} MDDEPT;

typedef struct
{
    char symb[SYMB_LEN]; /* Symbol Name */
    char hostname[128];  /* Hostname */
    uint32_t date;       /* Date */
    MDMSTR mstr;         /* Master */
    MDQUOT trade;        /* Trade */
    MDQUOT settle[4];    /* Settlement(A1, B1, A2, B2) Price */
    MDQUOT close;        /* Close Price */
    MDQUOT cancel;       /* Cancel Trade */
    MDQUOT oint;         /* Open Interest */
    MDQUOT mavg;         /* LME Month Average */
    MDQUOT offi;         /* LME Official Unofficial Price */
    MDDEPT dept;         /* Depth */
} FOLDER;

#endif
