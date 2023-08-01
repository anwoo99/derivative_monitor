#include "main.h"

void get_procname(char *procname)
{
    char procinfo[128];
    char cmdline[256];
    FILE *pFile;
    char *base;
    pid_t pid;

    pid = getpid();
    procname[0] = '\0';
    sprintf(procinfo, "/proc/%d/cmdline", pid);
    pFile = fopen(procinfo, "r");

    if (pFile == NULL)
        return;

    cmdline[0] = '\0';

    if (fgets(cmdline, sizeof(cmdline), pFile) == cmdline)
    {
        base = basename(cmdline);
        strcpy(procname, base);
    }

    fclose(pFile);
}

FEP *fep_open(const char *exnm, int flag)
{
    FEP *fep;

    switch (flag)
    {
    case MD_RDONLY:
    case MD_RDWR:
        break;
    default:
        return NULL;
    }

    fep = (FEP *)malloc(sizeof(FEP));

    if (!fep)
        return NULL;

    strcpy(fep->exnm, exnm);
    fep->whoami = flag;

    /* Get the configuration from etc/xxxx.json */
    if (-1 == fep_config(fep))
    {
        free(fep);
        return NULL;
    }

    /* Timezone Setting */
    putenv(fep->config.settings.timezone);
    tzset();

    /* Get the running process name */
    get_procname(fep->procname);
    
    /* Get the databse connector */
    if(-1 == get_database(fep))
    {
        fep_log(fep, FL_ERROR, "Cannot get the database connector..!");
        return NULL;
    }


    return (fep);
}