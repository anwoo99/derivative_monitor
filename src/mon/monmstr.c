#include "main.h"

void usage(const char *who)
{
    if (isatty(1))
        printf("usage : %s exchange_name\n", who);

    exit(1);
}

int main(int argc, char **argv)
{
    FEP *fep = NULL;
    char *whoami;
    char exnm[32];
    MDARCH *arch;
    int ii;
    FOLDER *fold;
    MDMSTR *mstr;

    whoami = basename(argv[0]);

    if (argc < 2)
        usage(whoami);

    strcpy(exnm, argv[1]);

    fep = fep_open(exnm, MD_RDONLY);

    if (fep == NULL)
    {
        fep_log(fep, FL_ERROR, GET_CALLER_FUNCTION(), "fep_open() for %s is failed!", exnm);
        return (0);
    }

    arch = (MDARCH *)fep->arch;
    fold = (FOLDER *)fep->fold;

    printf("############## %s ##############\n", exnm);
    printf("MAX[%d] COUNT[%d]\n", mdarch->mrec, mdarch->vrec);

    for (ii = 0; ii < arch->vrec;)
    {
        mstr = &folder[ii].mstr;

        printf("[%6d] ", ii);
        printf("symb[%.*s] ", sizeof(mstr->symb), mstr->symb);
        printf("root[%.*s] ", sizeof(mstr->root), mstr->root);
        printf("desc[%.*s] ", sizeof(mstr->symb_desc), mstr->symb_desc);
        printf("exch[%.*s] ", sizeof(mstr->exch_code), mstr->exch_code);
        printf("curr[%.*s] ", sizeof(mstr->curr), mstr->curr);
        printf("ftdt[%d] ", mstr->ftdt);
        printf("ltdt[%d] ", mstr->ltdt);
        printf("lddt[%d] ", mstr->lddt);
        printf("bsdt[%d] ", mstr->bsdt);
        printf("stdt[%d] ", mstr->stdt);
        printf("setp[%.*g] ", mstr->zdiv, mstr->setp);
        printf("pinc[%.*g] ", mstr->zdiv, mstr->pinc);
        printf("pmul[%.*g] ", mstr->zdiv, mstr->pmul);
        printf("update[%s] ", ctime(mstr->updated_at));
        printf("\n");
    }

    return (0);
}