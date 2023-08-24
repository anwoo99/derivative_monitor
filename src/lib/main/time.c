#include "main.h"

int fep_utc2kst(time_t utc_time, time_t *korean_time, struct tm *korean_tm)
{
    *korean_time = utc_time + (60 * 60 * 9); // KST (UTC+9)
    *korean_tm = *gmtime(korean_time);

    return (0);
}