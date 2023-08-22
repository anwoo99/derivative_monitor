#include "context.h"

void fep_close(FEP *fep)
{
    if (fep == NULL)
        return;

    free(fep);
    putenv("TZ=KST-9");
    tzset();
}