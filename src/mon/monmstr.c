#include "context.h"

void usage(const char *who)
{
    if (isatty(1))
        printf("usage : %s <exchange_name> <hostname>\n", who);

    exit(1);
}

int main(int argc, char **argv)
{
    FEP *fep = NULL;
    char *whoami;
    char exnm[32];
    char hostname[32];
    MDARCH *mdarch;
    int ii, jj = 0;
    FOLDER *folder;
    MDMSTR *mstr;

    whoami = basename(argv[0]);

    if (argc < 2)
        usage(whoami);

    strcpy(exnm, argv[1]);

    if (argc == 3)
        strcpy(hostname, argv[2]);

    fep = fep_open(exnm, MD_RDONLY);

    if (fep == NULL)
    {
        fep_log(fep, FL_ERROR, GET_CALLER_FUNCTION(), "fep_open() for %s is failed!", exnm);
        return (0);
    }

    mdarch = (MDARCH *)fep->arch;
    folder = (FOLDER *)fep->fold;

    printf("############## %s ##############\n", fep->exnm);
    printf("MAX[%d] COUNT[%d]\n", mdarch->mrec, mdarch->vrec);

    for (ii = 0; ii < mdarch->vrec; ii++)
    {
        mstr = &folder[ii].mstr;

        if((strlen(hostname) > 0 && strcmp(hostname, folder[ii].hostname) != 0))
            continue;

        printf("[%6d] ", jj++);
        printf("host[%.*s] ", strlen(folder[ii].hostname), folder[ii].hostname);
        printf("symb[%.*s] ", strlen(mstr->symb), mstr->symb);
        printf("root[%.*s] ", strlen(mstr->root), mstr->root);
        printf("desc[%.*s] ", strlen(mstr->symb_desc), mstr->symb_desc);
        printf("exch[%.*s] ", strlen(mstr->exch_code), mstr->exch_code);
        printf("curr[%.*s] ", strlen(mstr->curr), mstr->curr);
        printf("ftdt[%d] ", mstr->ftdt);
        printf("ltdt[%d] ", mstr->ltdt);
        printf("lddt[%d] ", mstr->lddt);
        printf("bsdt[%d] ", mstr->bsdt);
        printf("stdt[%d] ", mstr->stdt);
        printf("setp[%f] ", mstr->setp);
        printf("pinc[%f] ", mstr->pinc);
        printf("pmul[%f] ", mstr->pmul);
        printf("update[%s] ", ctime(&mstr->updated_at));
        printf("\n");
    }

    return (0);
}