#ifndef _CONTEXT_H
#define _CONTEXT_H

#include "main.h"

typedef struct
{
    int seqn;
    pthread_t thread;
    FEP *fep;
} RECVCTX;

typedef struct
{
    char name[256];
    char date[9];
    int is_holiday;
    int is_alert;
} HOLIDAY;

typedef struct
{
    CONFIG config;
    HOLIDAY holiday;
    int flag;
    int mrec;
    int vrec;
} MDARCH;

#endif
