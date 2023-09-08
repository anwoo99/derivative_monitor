#include "context.h"

#define MAX_ALERT 3
#define _DEFAULT_RECEIVE_FLAG 0
#define _NOT_RECEIVE_FLAG 1
#define _RECEIVE_FLAG 2

int tcp_socket_configuration(FEP *fep)
{
    int keepalive = 1; // Keep Alive ON
    int keepidle = 60; // 최초로 세션체크를 시작하는 시간(sec)
    int keepcnt = 2;   // 최초로 세션 체크 패킷을 보낸 후, 응답이 없을 경우 다시 발송하는 횟수
    int keepintv = 10; // keepidle 시간 이후 패킷을 보냈을 때 응답이 없는 경우 다음 패킷을 발송하는 주기
    int tsock = 0;

    tsock = socket(PF_INET, SOCK_STREAM, 0);

    if (tsock < 0)
        fep_log(fep, FL_ERROR, GET_CALLER_FUNCTION(), "socket() error");

    if (-1 == setsockopt(tsock, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(keepalive)))
    {
        fep_log(fep, FL_ERROR, GET_CALLER_FUNCTION(), "setsockopt() keepalive error");
        close(tsock);
        return (-1);
    }
    if (-1 == setsockopt(tsock, SOL_TCP, TCP_KEEPIDLE, &keepidle, sizeof(keepidle)))
    {
        fep_log(fep, FL_ERROR, GET_CALLER_FUNCTION(), "setsockopt() keepidle error");
        close(tsock);
        return (-1);
    }
    if (-1 == setsockopt(tsock, SOL_TCP, TCP_KEEPCNT, &keepcnt, sizeof(keepcnt)))
    {
        fep_log(fep, FL_ERROR, GET_CALLER_FUNCTION(), "setsockopt() keepcnt error ");
        close(tsock);
        return (-1);
    }
    if (-1 == setsockopt(tsock, SOL_TCP, TCP_KEEPINTVL, &keepintv, sizeof(keepintv)))
    {
        fep_log(fep, FL_ERROR, GET_CALLER_FUNCTION(), "setsockopt() keepintv error");
        close(tsock);
        return (-1);
    }

    return tsock;
}

int _is_check_time(PORT *port, struct tm *current_tm)
{
    if (port->recv_switch[current_tm->tm_wday][current_tm->tm_hour][current_tm->tm_min])
    {
        return (1);
    }
    else
    {
        return (0);
    }
}

int _not_received(FEP *fep, PORT *port, uint32_t *class_tag, time_t *current, struct tm *current_tm)
{
    int is_check = _is_check_time(port, current_tm);
    double diff;
    int max_alert;
    char message[256];

    switch (is_check)
    {
    case 1:                      // 모니터링 해당 시간대
        if (*class_tag & MASTER) // 마스터 데이터인 경우
        {
            /* 마스터데이터를 이미 수신한 상태라면 스킵 */
            if (port->master_status == _RECEIVE_FLAG)
                return (0);

            port->master_status = _NOT_RECEIVE_FLAG;
            max_alert = 1;
            sprintf(message, "마스터 데이터가 수신되지 않았습니다.");
        }
        else // 그 외 데이터인 경우
        {
            port->trade_status = _NOT_RECEIVE_FLAG;
            max_alert = MAX_ALERT;
            diff = difftime(*current, port->last_received);
            sprintf(message, "데이터가 %.3f초 동안 수신되지 않습니다.", diff);
        }

        if (port->alert_count < max_alert)
        {
            mon_send_cmefnd(fep, port, message, 1);
            port->alert_count += 1;
        }
        break;
    case 0: // 모니터링 미 해당 시간대(초기화)
        port->last_received = *current;
        port->trade_status = _DEFAULT_RECEIVE_FLAG;
        port->master_status = _DEFAULT_RECEIVE_FLAG;
        port->alert_count = 0;
        break;
    default:
        break;
    }

    return (0);
}

int _received(FEP *fep, PORT *port, uint32_t *class_tag, time_t *current, struct tm *current_tm)
{
    int is_check = _is_check_time(port, current_tm);
    char message[256];
    double diff;

    switch (is_check)
    {
    case 1:
        if (*class_tag & MASTER)
        {
            port->master_status = _RECEIVE_FLAG;
            port->alert_count = 0;
            port->last_received = *current;
            return (0);
        }
        else
        {
            if (port->trade_status == _NOT_RECEIVE_FLAG)
            {
                diff = difftime(*current, port->last_received);
                sprintf(message, "데이터가 복구되었습니다(소요시간 %ld 초).", diff);
                mon_send_cmefnd(fep, port, message, 1);
            }

            port->trade_status = _RECEIVE_FLAG;
            port->alert_count = 0;
            port->last_received = *current;
        }
        break;
    case 0:
        port->last_received = *current;
        port->trade_status = _DEFAULT_RECEIVE_FLAG;
        port->master_status = _DEFAULT_RECEIVE_FLAG;
        port->alert_count = 0;
        break;
    default:
        break;
    }

    return (0);
}

int mon_recv_check(FEP *fep, PORT *port, uint32_t *class_tag)
{
    MDARCH *arch = (MDARCH *)fep->arch;
    HOLIDAY *holiday = &arch->holiday;
    time_t current;
    struct tm current_tm;

    current = time(NULL);

    if (localtime_r(&current, &current_tm) == NULL)
    {
        fep_log(fep, FL_ERROR, GET_CALLER_FUNCTION(), "Cannot get the current time for '%s'", fep->exnm);
        return (-1);
    }

    /* 휴장일 체크 */
    fep_holiday(fep, &current, &current_tm);

    /* 금일 휴장일인 경우 수신 체크 X */
    if (holiday->is_holiday)
    {
        if (holiday->is_alert == 0)
        {
            holiday->is_alert = 1;
            fep_log(fep, FL_MUST, GET_CALLER_FUNCTION(), "금일 '%s' 거래소 휴장일 입니다(%s)", fep->exnm, holiday->name);
        }
        return (0);
    }

    /* 수신 체크 */
    if (*class_tag & NONE)
        _not_received(fep, port, class_tag, &current, &current_tm);
    else
        _received(fep, port, class_tag, &current, &current_tm);

    return (0);
}
