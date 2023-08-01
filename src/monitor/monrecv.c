#include "main.h"

void whoami(const char *who)
{
    if (isatty(1))
        printf("usage : %s exchange_name\n", who);

    exit(1);
}

int main(int argc, char *argv[])
{
    FEP *fep;
    char *whoami;
    char exnm[32];

    whoami = basename(argv[0]);

    if (argc < 2)
        usage(whoami);

    strcpy(exnm, argv[1]);

}