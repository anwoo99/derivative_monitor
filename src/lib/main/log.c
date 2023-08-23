#include "main.h"

#define GET_CALLER_FUNCTION() __builtin_return_address(1)

void fep_log(FEP *fep, int level, const char *caller_function, const char *format, ...)
{
    time_t korean_time;
    struct tm korean_tm;
    char logpath[128];
    char mode[2] = "a";
    struct stat lstat;
    char logmsg[1024 * 8];
    FILE *logF;

    if (level > fep->config.settings.logLevel)
        return;

    korean_time = time(NULL) + (60 * 60 * 9); // KST (UTC+9)
    korean_tm = *gmtime(&korean_time);

    snprintf(logpath, sizeof(logpath), "%s/%s-%d.log", LOG_DIR, fep->exnm, korean_tm.tm_wday);

    if (stat(logpath, &lstat) == 0)
    {
        time_t make_time = lstat.st_mtime + (60 * 60 * 9); // KST (UTC+9)
        struct tm make_tm = *gmtime(&make_time);

        if (make_tm.tm_yday != korean_tm.tm_yday)
            mode[0] = 'w';
    }

    snprintf(logmsg, sizeof(logmsg), "%02d/%02d %02d:%02d:%02d [%s - %s]", korean_tm.tm_mon + 1, korean_tm.tm_mday, korean_tm.tm_hour, korean_tm.tm_min, korean_tm.tm_sec, fep->procname, caller_function);

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
