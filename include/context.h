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
    CONFIG config;
    int flag;
    int mrec;
    int vrec;
} MDARCH;

#endif
