#include "main.h"

#define MAX_CROSS_ERROR 1000

extern int old_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag);
extern int ext_map(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag);
extern int mon_send_cmefnd(FEP *fep, PORT *port, char *message, int send_flag);

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
    else if (*class_tag & EXT_FORMAT)
    {
        ext_map(fep, port, msgb, msgl, class_tag);
    }
    else
    {
        return (0);
    }

    return (0);
}

int STR2STR(char *to, char *from, int size)
{
    if (to == NULL || from == NULL || size <= 0)
        return (-1);

    memcpy(to, from, size);
    to[size] = '\0';

    return (0);
}

int STR2INT(int *to, char *from, int size)
{
    char temp[size + 1];

    if (to == NULL || from == NULL || size <= 0)
        return (-1);

    STR2STR(temp, from, size);

    *to = atoi(temp);

    return (0);
}

int STR2UINT(uint32_t *to, char *from, int size)
{
    char temp[size + 1];

    if (to == NULL || from == NULL || size <= 0)
        return (-1);

    STR2STR(temp, from, size);

    *to = atoi(temp);

    return (0);
}

int STR2FLOAT(double *to, char *from, int size)
{
    char temp[size + 1];

    if (to == NULL || from == NULL || size <= 0)
        return (-1);

    STR2STR(temp, from, size);

    *to = atof(temp);

    return (0);
}

int empty_alert(FEP *fep, PORT *port, char *msgb, char *field_buffer, int field_size, char *field_name, int send_flag)
{
    char message[1024];
    char variable[1024];

    STR2STR(variable, field_buffer, field_size);

    if (strlen(variable) <= 0)
    {
        sprintf(message, "'%s' field empty..! | [%s]", field_name, msgb);
        mon_send_cmefnd(fep, port, message, send_flag);
        return (-1);
    }

    return (0);
}

int exch_alert(FEP *fep, PORT *port, char *msgb, char *field_buffer, int field_size, int send_flag)
{
    char message[1024];
    char variable[1024];
    int exch_id = 0;
    int error = 0;

    STR2STR(variable, field_buffer, field_size);

    exch_id = atoi(&variable[1]);

    switch (exch_id)
    {
    case 1: // CME
        if (memcmp(&fep->exnm[1], "CME", 3) != 0)
            error = -1;
        break;
    case 2: // SGX
        if (memcmp(&fep->exnm[1], "SGX", 3) != 0 && memcmp(&fep->exnm[1], "SG2", 3) != 0 && memcmp(&fep->exnm[1], "SGC", 3) != 0)
            error = -1;
        break;
    case 3: // HKFE
        if (memcmp(&fep->exnm[1], "HKF", 3) != 0 && memcmp(&fep->exnm[1], "HK2", 3) != 0 && memcmp(&fep->exnm[1], "HKW", 3) != 0)
            error = -1;
        break;
    case 4: // Eurex
        if (memcmp(&fep->exnm[1], "ERX", 3) != 0 && memcmp(&fep->exnm[1], "ER2", 3) != 0)
            error = -1;
        break;
    case 6: // CBOE
        if (memcmp(&fep->exnm[1], "CBO", 3) != 0)
            error = -1;
        break;
    case 7: // ASX
        if (memcmp(&fep->exnm[1], "ASX", 3) != 0)
            error = -1;
        break;
    case 8: // BM&F
        if (memcmp(&fep->exnm[1], "BMF", 3) != 0)
            error = -1;
        break;
    case 9: // TAIFEX
        if (memcmp(&fep->exnm[1], "TFU", 3) != 0)
            error = -1;
        break;
    case 10: // LME
        if (memcmp(&fep->exnm[1], "LME", 3) != 0)
            error = -1;
        break;
    case 11: // ICE US
        if (memcmp(&fep->exnm[1], "ICE", 3) != 0)
            error = -1;
        break;
    case 12: // OSE
        if (memcmp(&fep->exnm[1], "OSE", 3) != 0)
            error = -1;
        break;
    case 13: // ICE EUROPE
        if (memcmp(&fep->exnm[1], "ICR", 3) != 0)
            error = -1;
        break;
    case 14: // Euronext
        if (memcmp(&fep->exnm[1], "ENT", 3) != 0)
            error = -1;
        break;
    case 15: // ICE Endex
        if (memcmp(&fep->exnm[1], "ICX", 3) != 0)
            error = -1;
        break;
    case 16: // OPRA
        if (memcmp(&fep->exnm[1], "PRA", 3) != 0 && memcmp(&fep->exnm[1], "PRT", 3) != 0 && memcmp(&fep->exnm[1], "PR2", 3) != 0)
            error = -1;
        break;
    case 17: // ENYS
        if (memcmp(&fep->exnm[1], "NYS", 3) != 0)
            error = -1;
        break;
    case 20: // NSE
        if (memcmp(&fep->exnm[1], "NSE", 3) != 0)
            error = -1;
        break;
    default:
        error = -2;
        break;
    }

    if (error == -1)
    {
        sprintf(message, "'exch' field(%s) mismatch..! | [%s]", variable, msgb);
        mon_send_cmefnd(fep, port, message, send_flag);
    }
    else if (error == -2)
    {
        sprintf(message, "'exch' field(%s) unknwon..! | [%s]", variable, msgb);
        mon_send_cmefnd(fep, port, message, send_flag);
    }

    return (error);
}

int cross_check(FEP *fep, PORT *port, MDDEPT *depth, int send_flag)
{
    int ii;
    BBOOK *ask, *bid;
    char message[1024];
    int is_crossed = 0;

    ask = depth->ask;
    bid = depth->bid;

    for (ii = 0; ii < DEPTH_LEVEL; ii++)
    {
        if (((ask[ii].price - ask[ii + 1].price) >= 0.) && (fabsf(ask[ii].price - 1.) != 1.) && (fabsf(ask[ii + 1].price - 1.) != 1.))
        {
            sprintf(message, "[%s] 매도 [%d|%f] <=> [%d|%f]", depth->symb, ii, ask[ii].price, ii + 1, ask[ii + 1].price);
            mon_send_cmefnd(fep, port, message, 0);
            is_crossed = 1;
        }

        if (((bid[ii + 1].price - bid[ii].price) >= 0.) && (fabsf(bid[ii].price - 1.) != 1.) && (fabsf(bid[ii + 1].price - 1.) != 1.))
        {
            sprintf(message, "[%s] 매수 [%d|%f] <=> [%d|%f]", depth->symb, ii, bid[ii].price, ii + 1, bid[ii + 1].price);
            mon_send_cmefnd(fep, port, message, 0);
            is_crossed = 1;
        }
    }

    if ((bid[0].price - ask[0].price) >= 0. && (fabsf(bid[0].price - 0.) != 0.) && (fabsf(ask[0].price - 0.) != 0.))
    {
        sprintf(message, "[%s] 매수 1호가[%f] <=> 매도 1호가[%f]", depth->symb, bid[0].price, ask[0].price);
        mon_send_cmefnd(fep, port, message, 0);
        is_crossed = 1;
    }

    if (is_crossed)
    {
        depth->cross_cnt++;

        if (depth->cross_cnt > MAX_CROSS_ERROR)
        {
            sprintf(message, "[%s] 품목에서 호가역전이 과도하게 발생하고 있습니다.", depth->symb);
            mon_send_cmefnd(fep, port, message, send_flag);
            depth->cross_cnt = 0;
        }
    }
}

int parse_pind(MDMSTR *mstr, char *buffer, int size)
{
    int number;
    char pind[32];

    STR2STR(pind, buffer, size);

    switch (pind[0])
    {
    case 'A':
        mstr->main_f = 2;
        mstr->sub_f = 1;
        mstr->zdiv = 1;
        break;
    case 'B':
        mstr->main_f = 4;
        mstr->sub_f = 1;
        mstr->zdiv = 2;
        break;
    case 'C':
        mstr->main_f = 8;
        mstr->sub_f = 1;
        mstr->zdiv = 3;
        break;
    case 'D':
        mstr->main_f = 16;
        mstr->sub_f = 1;
        mstr->zdiv = 4;
        break;
    case 'E':
        mstr->main_f = 32;
        mstr->sub_f = 1;
        mstr->zdiv = 5;
        break;
    case 'F':
        mstr->main_f = 64;
        mstr->sub_f = 1;
        mstr->zdiv = 6;
        break;
    case 'G':
        mstr->main_f = 128;
        mstr->sub_f = 1;
        mstr->zdiv = 7;
        break;
    case 'H':
        mstr->main_f = 256;
        mstr->sub_f = 1;
        mstr->zdiv = 8;
        break;
    case 'I':
        mstr->main_f = 32;
        mstr->sub_f = 2;
        mstr->zdiv = 6;
        break;
    case 'J':
        mstr->main_f = 64;
        mstr->sub_f = 2;
        mstr->zdiv = 7;
        break;
    case 'K':
        mstr->main_f = 32;
        mstr->sub_f = 4;
        mstr->zdiv = 7;
        break;
    case 'L':
        mstr->main_f = 32;
        mstr->sub_f = 8;
        mstr->zdiv = 8;
        break;
    default:
        number = atoi(pind);

        if (number >= 0 && number <= 9)
        {
            mstr->main_f = pow(10, number);
            mstr->sub_f = 1;
            mstr->zdiv = number;
        }

        break;
    }

    return (0);
}
