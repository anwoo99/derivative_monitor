#include "main.h"

#define DELIM "\001"
#define MARKET_ALERT_TOKEN "1"

#define CMEFND_IP "49.247.7.123"
#define CMEFND_PORT 57237

int mon_send_cmefnd(FEP *fep, PORT *port, char *message, int send_flag)
{
    int sln = 0, rc;
    char msgb[1024], encoded_data[1024];
    struct sockaddr_in cmefnd;
    int tcp_socket = 0;

    if (port->alert == false)
        return (0);

    sprintf(msgb, "[%s] [%s-%s] %s", port->host, fep->exnm, port->name, message);
    sprintf(encoded_data, "%s%s=%s%s", DELIM, MARKET_ALERT_TOKEN, msgb, DELIM);

    fep_log(fep, FL_PROGRESS, GET_CALLER_FUNCTION(), "%s", msgb);

    if (!send_flag)
        return (0);

    memset(&cmefnd, 0x00, sizeof(cmefnd));
    cmefnd.sin_family = AF_INET;
    cmefnd.sin_addr.s_addr = inet_addr(CMEFND_IP);
    cmefnd.sin_port = htons(CMEFND_PORT);
    tcp_socket = tcp_socket_configuration(fep);

    if (tcp_socket < 0)
        return (-1);

    if (connect(tcp_socket, (struct sockaddr *)&cmefnd, sizeof(cmefnd)) == -1)
    {
        fep_log(fep, FL_ERROR, GET_CALLER_FUNCTION(), "Cannot connect to CMEFND Server");
        return (0);
    }

    while (sln < (strlen(encoded_data) + 1))
    {
        rc = write(tcp_socket, encoded_data, strlen(encoded_data) + 1 - sln);

        if (rc < 0)
        {
            if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
            {
                fep_sleep(200);
                continue;
            }
            else
            {
                fep_log(fep, FL_ERROR, GET_CALLER_FUNCTION(), "Failed to write() data");
                close(tcp_socket);
                return (0);
            }
        }
        else
        {
            sln += rc;
        }
    }

    close(tcp_socket);
    return (0);
}