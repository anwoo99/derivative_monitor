#include "main.h"

int old_master_class(FEP *fep, int *class_tag)
{
    char exchange_name[128];

    strcpy(exchange_name, fep->config.settings.name);

    if (strcmp(exchange_name, "FLME") == 0 || strcmp(exchange_name, "NLME") == 0 || strcmp(exchange_name, "SLME") == 0)
    {
        *class_tag |= LME;
    }
    else
    {
        switch (exchange_name[0])
        {
        case 'F':
            *class_tag |= FUTURE;
            break;
        case 'O':
            *class_tag |= OPTION;
            break;
        case 'S':
            *class_tag |= SPREAD;
            break;
        case 'E':
            *class_tag |= STOCK;
            break;
        default:
            return (-1);
        }
    }

    return (0);
}

/*************************/
/* mon_class()           */
/* 데이터 클래스 구분 함수 */
/*************************/
int mon_classify(FEP *fep, PORT *port, int *class_tag)
{
    if (strcmp(port->format, "OLD") == 0)
    {
        *class_tag |= OLD_FORMAT;

        if (strcmp(port->type, "M") == 0)
        {
            *class_tag |= MASTER;

            if (-1 == old_master_class(fep, class_tag))
                return (-1);
        }
        else if (strcmp(port->type, "T") == 0)
        {
            *class_tag |= TRADE;
        }
        else
        {
            return (-1);
        }
    }
    else if (strcmp(port->format, "EXT") == 0)
    {
        *class_tag |= EXT_FORMAT;
    }
    else
    {
        return (-1);
    }

    return (0);
}
