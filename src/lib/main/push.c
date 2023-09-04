#include "context.h"
#include "push.h"

#define PUSH_QUOT 1
#define PUSH_DEPT 2

//
// fep_push()
// Push realtime market data
//
int fep_push(FEP *fep, FOLDER *folder, int type)
{
    struct pushdata pushdata;
    int pushlen = 0;
    char pushbuf[4096];

    sprintf(pushdata.pushmsg.symb, "%s@%.15s@%s", fep->exnm, folder->symb, folder->hostname);

    switch (type)
    {
    case 0: // QUOTE
        pushdata.pushmsg.mask = PUSH_QUOT;
        memcpy(pushbuf, &folder->quote, sizeof(MDQUOT));
        pushlen = sizeof(MDQUOT);
        break;
    case 1: // CANCEL
        pushdata.pushmsg.mask = PUSH_QUOT;
        memcpy(pushbuf, &folder->cancel, sizeof(MDQUOT));
        pushlen = sizeof(MDQUOT);
        break;
    case 2:
        pushdata.pushmsg.mask = PUSH_DEPT;
        memcpy(pushbuf, &folder->depth, sizeof(MDDEPT));
        pushlen = sizeof(MDDEPT);
        break;
    default:
        break;
    }

    if (pushlen > 0)
    {
        if (pushlen > MAX_PUSH_LEN)
            pushlen = MAX_PUSH_LEN;

        pushdata.pushmsg.msgl = pushlen;

        memcpy(pushdata.pushmsg.msgb, pushbuf, pushlen);
        rtd_push(&pushdata);
    }
    
    return (0);
}
