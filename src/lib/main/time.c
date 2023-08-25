#include "main.h"
#include "context.h"

int fep_utc2kst(time_t utc_time, time_t *korean_time, struct tm *korean_tm)
{
    *korean_time = utc_time + (60 * 60 * 9); // KST (UTC+9)
    *korean_tm = *gmtime(korean_time);

    return (0);
}

void _false_holiday(HOLIDAY *holiday)
{
    holiday->is_holiday = 0;
    memset(holiday->name, 0x00, sizeof(holiday->name));
}

void _true_holiday(HOLIDAY *holiday, char *name)
{
    holiday->is_holiday = 1;
    strcpy(holiday->name, name);
}

int fep_holiday(FEP *fep, time_t *current, struct tm *current_tm)
{
    MDARCH *arch = (MDARCH *)fep->arch;
    HOLIDAY *holiday = &arch->holiday;
    char filename[128];
    char holiday_name[128];
    char current_date[9];
    FILE *fp;

    strftime(current_date, sizeof(current_date), "%Y%m%d", current_tm);

    /* 같은 날이면 return */
    if (strcmp(holiday->date, current_date) == 0)
        return (0);

    holiday->is_alert = 0;

    memset(holiday->date, 0x00, sizeof(holiday->date));
    strcpy(holiday->date, current_date);
    sprintf(filename, "%s/holidays/%s-%d.txt", ETC_DIR, &fep->exnm[1], current_tm->wday);

    if (access(filename, F_OK) == -1)
    {
        fep_log(fep, FL_ERROR, "There is no '%s' file", filename);
        _false_holiday(holiday);
        return (-1);
    }

    fp = fopen(filename, "r");

    if (fp <= 0)
    {
        fep_log(fep, FL_ERROR, "Cannot open '%s' file", filename);
        _false_holiday(holiday);
        return (-1);
    }

    fgets(holiday_name, sizeof(holiday_name), fp);

    if (strlen(holiday_name) > 0)
    {
        _true_holiday(holiday, holiday_name);
    }
    else
    {
        _false_holiday(holiday);
    }

    fclose(fp);
    return (0);
}

void fep_sleep(int microseconds)
{
    struct timeval timeval;

    timeval.tv_sec = 0;
    timeval.tv_usec = microseconds;
    select(0, NULL, NULL, NULL, &timeval);
}