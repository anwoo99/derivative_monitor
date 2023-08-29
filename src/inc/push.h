/*******************************************************************************
 * (C) COPYRIGHT Promentor Co., Ltd. 2010
 * All Rights Reserved
 * Licensed Materials - Property of WINWAY Co., Ltd.
 *
 * This program contains proprietary information of WINWAY Co., Ltd
 * All embodying confidential information, ideas and expressions can't be
 * reproceduced, or transmitted in any form or by any means, electronic,
 * mechanical, or otherwise without the written permission of WINWAY.
 *
 *  Components   : push.h - Definition for realtime pusher
 *  Release Ver  : 1.0.0
 ******************************************************************************/
#ifndef _PUSH_H_
#define _PUSH_H_

#define RTD_KEY (('P' << 24) + ('Q' << 16))
#define RTD_APP 32
#define RTD_IPCK(x) (RTD_KEY + x)
#define LOCAL_HOST "127.0.0.1"

#define RT_SET_PID 0
#define RT_RST_SYMB 1
#define RT_INS_SYMB 2
#define RT_SET_MASK 3
#define RT_DEL_MASK 4
#define RT_DEL_SYMB 5
#define RT_PUSH_MSG 6
#define RT_INI_SYMB 9

#define MAX_PUSH_LEN 800
#define L_SYMB 20

typedef unsigned int mask_t;

typedef struct
{
    int flag;      /* general flags             */
    void *self;    /* myself                    */
    void *myrq;    /* mymq interface            */
    char errm[80]; /* general error message */
} RTD;

struct pushmsg
{
    uint32_t seqn;           /* sequencial number(=func)    */
    char symb[L_SYMB];       /* symbol                      */
    mask_t mask;             /* event mask                  */
    uint8_t type;            /* message type                        */
    uint8_t flag;            /* ticker or scroll flags      */
    uint16_t msgl;           /* length of rmsg[]            */
    char msgb[MAX_PUSH_LEN]; /* real-time message           */
};
typedef struct pushmsg pushmsg_t;

struct pushdata
{
    long mkid;         /* market id                 */
    pushmsg_t pushmsg; /* real message to push              */
};
typedef struct pushdata pushdata_t;

struct pushsymb
{
    long many;
    struct realsymb
    {
        uint32_t func;
        mask_t mask;
        char symb[L_SYMB];
    } push[1];
};
typedef struct pushsymb pushsymb_t;

struct rt_hooked
{
    char symb[L_SYMB]; /* symbol code                  */
    uint16_t many;     /* # of linked clients          */
    short mark;        /* valid mark                   */
};

struct rt_board
{
    int svcp;                 /* service port			*/
    int mqid[3];              /* message qid 			*/
                              /* [0] push (by cooker)		*/
                              /* [1] push (by client)		*/
                              /* [2] chck (by system)		*/
    int symbs;                /* offset of rt_symbol		*/
    int fifos;                /* offset of rt_fifo		*/
    pid_t rqpid;              /* process id			*/
    int alive;                /* alive checking 		*/
    int dynamic_mode;         /* if TRUE, dynamic_mode	*/
    int no_of_clients;        /* no of clients		*/
    int no_of_symbols;        /* no of symbols		*/
    struct rt_hooked hook[1]; /* hooked symbol information	*/
};

struct rt_symbol
{
    int maxr; /* maximum symbols		*/
    int valr; /* valid record number		*/
    struct symb
    {
        char symb[L_SYMB]; /* symbol code			*/
        short seqn;        /* sequencial number		*/
        short mark;        /* valid mark			*/
    } symb[1];
};

struct rt_fifo
{
    int maxr;               /* maximum records		*/
    int widx;               /* push index			*/
    struct pushmsg push[1]; /* push contents		*/
};

struct rt_alarm
{
    uint16_t sidx; /* symbol sequencial number	*/
    uint16_t indx; /* index of push queue		*/
    uint32_t seqn; /* sequencial number		*/
};

#define ALARM_ID 0x7890 /* value for realq_rt_alarm.sidx */

struct rt_cmd
{             /* MyMQ/RQ  command struct	*/
    int func; /* must be = pushsymb.push	*/
    mask_t mask;
    char symb[L_SYMB];
};

struct rt_msg
{
    uint8_t type;            /* message type			*/
    uint8_t flag;            /* ticker or scroll flags	*/
    short msgl;              /* length of rmsg[]		*/
    char msgb[MAX_PUSH_LEN]; /* real-time message		*/
};

struct rt_pktb
{
    struct pushmsg pushmsg; /* push msg			*/
    struct rt_pktb *next;   /* next rpktb			*/
};

struct rt_pkth
{
    int pktn[2]; /* # of packet (alloc'd, free'd)*/
    int chck;    /* check session status		*/
    void *pktb;  /* allocated pktbuf		*/
    struct
    {
        pthread_mutex_t mutex; /* for locking			*/
        pthread_cond_t cond;   /* for events			*/
        struct rt_pktb *pktb;  /* free packet			*/
        int wait;              /* wait rpktb			*/
    } alloc;
    struct
    {
        pthread_mutex_t mutex; /* for locking			*/
        pthread_cond_t cond;   /* for events			*/
        struct rt_pktb *pktb;  /* scheduing rpktb		*/
    } queue;
};

typedef struct
{
    int flag; /* general flags		*/
#define _OPEN_ 0x01
#define _RECV_ 0x02
#define _PKTB_ 0x04
    int sock;                  /* connected socket		*/
    int rqid;                  /* Real time queue id		*/
    int alive;                 /* alive checking		*/
    struct rt_board *rt_board; /* shared memory		*/
    struct rt_fifo *rt_fifo;   /* FIFO message ring buffer	*/
    struct rt_pkth rt_pkth;    /* packet header		*/
    pthread_t rcver;           /* receiver thread..		*/
} RTIS;

/* log level */
#define LL_NONE 0
#define LL_MUST 1
#define LL_ERROR 1
#define LL_WARNING 2
#define LL_PROGRESS 3
#define LL_DEBUG 4

#ifdef __cplusplus
extern "C"
{
#endif

    int rt_packet_init(struct rt_pkth *);
    int rt_packet_push(struct rt_pkth *pkth, struct pushmsg *pushmsg, int many);
    int rt_packet_full(struct rt_pkth *pkth, struct pushmsg *pushmsg, int howmany, int timeout);
    void rt_packet_sig(struct rt_pkth *);
    void rt_packet_reset(struct rt_pkth *);

    RTD *rtd_open(int port, const char *ipad);
    int rtd_send(RTD *rtd, struct pushsymb *pushsymb);
    int rtd_recv(RTD *rtd, struct pushmsg *pushmsg, int howmany);
    int rtd_timedrecv(RTD *rtd, struct pushmsg *pushmsg, int howmany, int timeout);
    int rtd_close(RTD *rtd);
    int rtd_push(struct pushdata *pushdata);

#ifdef __cplusplus
}
#endif

#endif
