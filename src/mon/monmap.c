#include "main.h"

extern int old_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag);

/****************************/
/* mon_map()                */
/* 데이터 파싱 후 메모리 저장 */
/****************************/

int mon_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag)
{
    if (*class_tag & OLD_FORMAT)
    {
        old_map(fep, port, msgb, msgl, class_tag);
    }
    else
    {
        return (0);
    }

    return (0);
}