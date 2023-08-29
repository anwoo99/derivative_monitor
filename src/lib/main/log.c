#include "main.h"

void fep_log(FEP *fep, int level, const char *caller_function, const char *format, ...)
{
    time_t korean_time, make_time, current;
    struct tm korean_tm, make_tm;
    char logpath[128];
    char mode[2] = "a";
    struct stat lstat;
    char logmsg[1024 * 8];
    FILE *logF;

    if (level > fep->config.settings.logLevel)
        return;

    korean_time = time(NULL);
    fep_utc2kst(current, &korean_time, &korean_tm);

    snprintf(logpath, sizeof(logpath), "%s/%s-%d.log", LOG_DIR, fep->exnm, korean_tm.tm_wday);

    if (stat(logpath, &lstat) == 0)
    {
        fep_utc2kst(lstat.st_mtime, &make_time, &make_tm);

        if (make_tm.tm_yday != korean_tm.tm_yday)
            mode[0] = 'w';
    }

    snprintf(logmsg, sizeof(logmsg), "%02d/%02d %02d:%02d:%02d [%s] [%s()] ", korean_tm.tm_mon + 1, korean_tm.tm_mday, korean_tm.tm_hour, korean_tm.tm_min, korean_tm.tm_sec, fep->procname, caller_function);

    va_list vl;
    va_start(vl, format);
    vsnprintf(&logmsg[strlen(logmsg)], sizeof(logmsg) - strlen(logmsg), format, vl);
    va_end(vl);

    logF = fopen(logpath, mode);
    if (!logF)
        return;

    if (level != FL_ERROR)
        fprintf(logF, "%s\n", logmsg);
    else
        fprintf(logF, "%s | error(%d|%s)\n", logmsg, errno, strerror(errno));

    fclose(logF);
}

void null_to_space(char *msgb, int msgl)
{
    int i;

    for (i = 0; i < msgl; i++)
    {
        if (msgb[i] == '\0')
        {
            msgb[i] = ' ';
        }
    }
}