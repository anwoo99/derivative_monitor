#include "context.h"

static int cmpfold(FOLDER *folder_a, FOLDER *folder_b)
{
    // Compare hostname first
    int symb_cmp = strcmp(folder_a->hostname, folder_b->hostname);

    if (symb_cmp != 0)
        return symb_cmp;

    // If hostname values are the same, compare symbol
    return strcmp(folder_a->symb, folder_b->symb);
}

FOLDER *getfolder(FEP *fep, const char *symb, const char *hostname)
{
    MDARCH *mdarch;
    FOLDER *folder, f;

    if ((mdarch = fep->arch) == NULL)
    {
        return (NULL);
    }

    folder = fep->fold;

    strcpy(f.symb, symb);
    strcpy(f.hostname, hostname);

    return (bsearch(&f, folder, mdarch->vrec, sizeof(FOLDER), cmpfold));
}

FOLDER *newfolder(FEP *fep, const char *symb, const char *hostname)
{
    MDARCH *mdarch;
    FOLDER *folder, f;
    time_t current;

    if ((mdarch = fep->arch) == NULL)
    {
        return (NULL);
    }

    if ((folder = getfolder(fep, symb, hostname)) != NULL)
    {
        return (folder);
    }
 
    if (mdarch->vrec + 1 >= mdarch->mrec)
    {
        fep_log(fep, FL_ERROR, GET_CALLER_FUNCTION(), "The shared memory for quote is full %d/%d", mdarch->vrec, mdarch->mrec);
        return (NULL);
    }

    folder = fep->fold;
;
    memset(&f, 0x00, sizeof(f));
    strcpy(f.symb, symb);
    strcpy(f.hostname, hostname);
    strcpy(f.mstr.symb, symb);
    strcpy(f.quote.symb, symb);
    strcpy(f.settle[0].symb, symb);
    strcpy(f.settle[1].symb, symb);
    strcpy(f.settle[2].symb, symb);
    strcpy(f.settle[3].symb, symb);
    strcpy(f.close.symb, symb);
    strcpy(f.cancel.symb, symb);
    strcpy(f.oint.symb, symb);
    strcpy(f.mavg.symb, symb);
    strcpy(f.offi.symb, symb);
    strcpy(f.depth.symb, symb);
    strcpy(f.status.symb, symb);
    current = time(NULL);

    f.created_at = current;
    f.mstr.created_at = current;

    memcpy(&folder[mdarch->vrec++], &f, sizeof(FOLDER));
    qsort(folder, mdarch->vrec, sizeof(FOLDER), cmpfold);

    return (getfolder(fep, symb, hostname));
}

void delfolder(FEP *fep, FOLDER *folder)
{
    MDARCH *mdarch = fep->arch;
    char *first, *second;
    char seqn;
    int size;

    first = (char *)&folder[0];
    second = (char *)&folder[1];
    seqn = (int)(((unsigned long)folder - (unsigned long)fep->fold) / sizeof(FOLDER));
    size = (mdarch->vrec - seqn - 1) * sizeof(FOLDER);

    memmove(first, second, size);

    mdarch->vrec--;
}