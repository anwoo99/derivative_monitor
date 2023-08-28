#ifndef _MYQUOT_H_
#define _MYQUOT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include "context.h"
#include "push.h"
#include "pen.h"

#define MAX_SYMB 100
#define MAX_FLDS (60 * 20)
#define MYMQ_EXCHANGE "mentor"

#define PUSH_QUOT 1
#define PUSH_DEPT 2

#define EQ 0
#define GT 1
#define LT 2

    extern char i_scrn[], s_scrn[];
    extern char i_exch[], s_exch[];
    extern char i_symb[], s_symb[];
    extern char s_path[], s_next[];
    extern int delayed;
    extern int nfld;
    extern int insert_mode;
    extern int enable_pgup;
    extern int enable_pgdn;
    extern int enable_next;
    extern int enable_prev;
    extern int forwarding;
    extern int display10;
    extern int kst4time;

    struct realque
    {
        int many;
        struct
        {
            char exnm[32];
            char code[40];     // code
            char symb[32];     // symbol code
            char hostname[32]; // Host Name
            mask_t mask;       // event mask
            int seqn;          // sequencial number
        } q[MAX_SYMB];
    };

    extern FEP *fep;
    extern struct realque realque;
    extern pthread_mutex_t real_lock;
    extern struct field field[];

    int issvc(int);
    int query(int);
    int update(void *, int, int, int);
    int header4scrn(char *scrn, char *name);
    void setguide(char *);
    void clrguide();

    int rqinit();
    void rqreset();
    void rqsymb(char *, char *, char *, mask_t, int);
    void rqsend();

    void clearall();

    int mapopen(char *);

#ifdef __cplusplus
}
#endif

#endif
