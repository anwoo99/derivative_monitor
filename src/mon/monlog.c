#include "main.h"
#include "context.h"

#define MASTER_NEW_INTERVAL 30.0

int mon_log_path(FEP *fep, PORT *port, uint32_t *class_tag, char *logdir, char *filename)
{
    SETTINGS *settings = &fep->config.settings;
    char data_type[32];
    char date[32];
    time_t current;
    struct tm timeinfo;

    /* Set the data type */
    if (*class_tag & MASTER)
        strcpy(data_type, "master");
    else if (*class_tag & STATUS)
        strcpy(data_type, "status");
    else if (*class_tag & QUOTE)
        strcpy(data_type, "quote");
    else if (*class_tag & CANCEL)
        strcpy(data_type, "cancel");
    else if (*class_tag & SETTLE)
        strcpy(data_type, "settle");
    else if (*class_tag & OINT)
        strcpy(data_type, "oint");
    else if (*class_tag & DEPTH)
        strcpy(data_type, "depth");
    else if (*class_tag & FND)
        strcpy(data_type, "fnd");
    else if (*class_tag & MAVG)
        strcpy(data_type, "mavg");
    else if (*class_tag & OFFI)
        strcpy(data_type, "offi");
    else if (*class_tag & WARE)
        strcpy(data_type, "ware");
    else if (*class_tag & VOLM)
        strcpy(data_type, "volm");

    /* Get the date */
    current = time(0);
    localtime_r(&current, &timeinfo);
    strftime(date, sizeof(date), "%Y%m%d", &timeinfo);

    sprintf(logdir, "%s/%s/%s/%s/%s/%s", LOG_DIR, port->host, settings->name, port->name, port->format, data_type);

    sprintf(filename, "%s_%03d.dump", date, timeinfo.tm_hour);

    /* Create Directory */
    if (-1 == create_directory(logdir))
    {
        fep_log(fep, FL_ERROR, GET_CALLER_FUNCTION(), "Cannot create direcotry '%s'", logdir);
        return (-1);
    }
    return (0);
}

void mon_log_write(FEP *fep, char *logpath, const char *format, ...)
{
    time_t korean_time, utc_time, modified_time;
    struct tm korean_tm, modify_tm;
    char mode[2] = "a";
    struct stat lstat;
    char logmsg[1024 * 8];
    FILE *logF;

    utc_time = time(0);
    fep_utc2kst(utc_time, &korean_time, &korean_tm);

    if (stat(logpath, &lstat) == 0 && strcmp(logpath, "MASTER.dump") == 0)
    {
        fep_utc2kst(lstat.st_mtime, &modified_time, &modify_tm);

        mode[0] = (difftime(korean_time, modified_time) > MASTER_NEW_INTERVAL) ? 'w' : 'a';
    }

    snprintf(logmsg, sizeof(logmsg), "[%02d:%02d:%02d] ", korean_tm.tm_hour, korean_tm.tm_min, korean_tm.tm_sec);

    va_list vl;
    va_start(vl, format);
    vsnprintf(&logmsg[strlen(logmsg)], sizeof(logmsg) - strlen(logmsg), format, vl);
    va_end(vl);

    logF = fopen(logpath, mode);

    if (!logF)
        return;

    fprintf(logF, "%s\n", logmsg);
    fclose(logF);
}

int mon_log_remove(FEP *fep, char *logdir, int date_limit)
{
    time_t korean_time, utc_time, modified_time;
    struct tm korean_tm, modify_tm;
    struct dirent *entry;
    struct stat file_stat;
    char logpath[256];
    DIR *dir;
    double date_difference;

    utc_time = time(0);
    fep_utc2kst(utc_time, &korean_time, &korean_tm);

    dir = opendir(logdir);

    if (dir == NULL)
    {
        return (-1);
    }

    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        if (strstr(entry->d_name, ".dump") == NULL)
        {
            continue;
        }

        snprintf(logpath, sizeof(logpath), "%s/%s", logdir, entry->d_name);

        if (stat(logpath, &file_stat) == 0)
        {
            fep_utc2kst(file_stat.st_mtime, &modified_time, &modify_tm);
        }
        else
        {
            fep_log(fep, FL_ERROR, GET_CALLER_FUNCTION(), "Cannot get the status of '%s' file", logpath);
            continue;
        }

        // Convert seconds to days
        date_difference = difftime(korean_time, modified_time) / (60 * 60 * 24);

        // If date difference is greater than 5 days, delete the file
        if (date_difference > date_limit)
        {
            if (remove(logpath) != 0)
            {
                fep_log(fep, FL_ERROR, GET_CALLER_FUNCTION(), "Cannot remove '%s' file", logpath);
            }
        }
    }

    closedir(dir);
    return (0);
}

/*************************/
/* mon_log()             */
/* 데이터 로깅 함수       */
/*************************/
int mon_log(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
    char logdir[256];
    char filename[128];
    char logpath[256];
    char master_dump[256];
    SETTINGS *settings = &fep->config.settings;
    RAW_DATA *raw_data = &fep->config.raw_data;
    int log_mode = FL_DEBUG;
    int max_date = 1;

    /* 호가 로그 필터링 */
    if ((*class_tag & DEPTH) && !raw_data->depth_log)
        return (0);

    /* Make Log Path */
    if (-1 == mon_log_path(fep, port, class_tag, logdir, filename))
    {
        fep_log(fep, FL_ERROR, GET_CALLER_FUNCTION(), "Failed to get log path");
        return (-1);
    }

    memset(logpath, 0x00, sizeof(logpath));
    sprintf(logpath, "%s/%s", logdir, filename);

    /* Write the log */
    mon_log_write(fep, logpath, "%s", msgb);

    /* Write Master Dump */
    if (*class_tag & MASTER)
    {
        sprintf(master_dump, "%s/%s", logdir, "MASTER.dump");
        mon_log_write(fep, master_dump, "%s", msgb);
    }

    /* Remove the old logs */
    if (!(*class_tag & DEPTH))
        max_date = raw_data->max_date;

    if (-1 == mon_log_remove(fep, logdir, max_date))
    {
        fep_log(fep, FL_ERROR, GET_CALLER_FUNCTION(), "Failed to remove the old log in '%s'", logdir);
        return (-1);
    }

    return (0);
}