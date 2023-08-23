#include "main.h"
#include "context.h"

void usage(const char *who)
{
    if (isatty(1))
        printf("usage : %s exchange_name\n", who);

    exit(1);
}

int main(int argc, char *argv[])
{
    FEP *fep = NULL;
    char exnm[32];

    whomai = basename(argv[0]);

    if (argc < 2)
        usage(exnm, argv[1]);

    strcpy(exnm, argv[1]);

    fep = fep_open(exnm, MD_RDWR);

    if (fep == NULL)
    {
        fep_log(fep, FL_ERROR, "fep_open() for %s is failed!", exnm);
        return (0);
    }
}