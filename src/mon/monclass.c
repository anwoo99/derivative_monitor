#include "main.h"

#define OLD_TRXC_SIZE 3

int old_master_class(FEP *fep, uint32_t *class_tag)
{
    char exchange_name[128];

    strcpy(exchange_name, fep->config.settings.name);

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
    case 'N':
        *class_tag |= WAREHOUSE;
        break;
    default:
        return (-1);
    }

    return (0);
}

int old_trade_class(FEP *fep, char *msgb, uint32_t *class_tag)
{
    char trxc[8];

    memcpy(trxc, msgb, OLD_TRXC_SIZE);
    trxc[OLD_TRXC_SIZE] = '\0';

    if (strcmp(trxc, "T60") == 0)
    {
        if (*class_tag & LME)
            *class_tag |= OFFI;
        else
            *class_tag |= STATUS;
    }
    else if (strcmp(trxc, "T21") == 0)
    {
        *class_tag |= QUOTE;
    }
    else if (strcmp(trxc, "T24") == 0)
    {
        *class_tag |= CANCEL;
    }
    else if (strcmp(trxc, "T40") == 0)
    {
        *class_tag |= SETTLE;
    }
    else if (strcmp(trxc, "T41") == 0)
    {
        *class_tag |= CLOSE;
    }
    else if (strcmp(trxc, "T50") == 0)
    {
        *class_tag |= OINT;
    }
    else if (strcmp(trxc, "T31") == 0)
    {
        *class_tag |= DEPTH;
    }
    else if (strcmp(trxc, "T80") == 0)
    {
        *class_tag |= FND;
    }
    else if (strcmp(trxc, "T52") == 0)
    {
        *class_tag |= MAVG;
    }
    else if (strcmp(trxc, "T62") == 0)
    {
        *class_tag |= WARE;
    }
    else if (strcmp(trxc, "T63") == 0)
    {
        *class_tag |= VOLM;
    }
    else
    {
        return (-1);
    }

    return (0);
}

/*************************/
/* mon_class()           */
/* 데이터 클래스 구분 함수 */
/*************************/
int mon_classify(FEP *fep, PORT *port, char *msgb, uint32_t *class_tag)
{
    char exchange_name[128];

    strcpy(exchange_name, fep->config.settings.name);

    if (strcmp(exchange_name, "FLME") == 0 || strcmp(exchange_name, "NLME") == 0 || strcmp(exchange_name, "SLME") == 0)
    {
        *class_tag |= LME;
    }

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

            if (-1 == old_trade_class(fep, msgb, class_tag))
                return (-1);
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
