#include "main.h"
#include "context.h"

#define MULTICAST_F "224.0.0.0"
#define MULTICAST_T "239.255.255.255"
#define MSGBUFF 1024 * 16

int port_confiuration(FEP *fep, int seqn);
int domain_socket_configuration(FEP *fep, int seqn);
void recv_to_send(void *argv);

void usage(const char *who)
{
    if (isatty(1))
        printf("usage : %s exchange_name\n", who);

    exit(1);
}

int main(int argc, char *argv[])
{
    FEP *fep = NULL;
    RECVCTX recvctx[MAX_PORT];
    char *whoami;
    char exnm[32];
    int ii;
    int result;

    whoami = basename(argv[0]);

    if (argc < 2)
        usage(whoami);

    strcpy(exnm, argv[1]);

    fep = fep_open(exnm, MD_RDWR);
    
    if (fep == NULL)
    {
        fep_log(fep, FL_ERROR, GET_CALLER_FUNCTION(), "fep_open() for %s is failed!", exnm);
        return (0);
    }

    for (ii = 0; ii < fep->config.nport; ii++)
    {
        if (fep->config.ports[ii].running == false)
            continue;

        recvctx[ii].seqn = ii;
        recvctx[ii].fep = fep;

        fep_log(fep, FL_MUST, GET_CALLER_FUNCTION(), "%s_%s_%s process start..!", fep->exnm, fep->config.ports[ii].host, fep->config.ports[ii].name);
        
        pthread_create(&recvctx[ii].thread, NULL, recv_to_send, &recvctx[ii]);
    }

    for (ii = 0; ii < fep->config.nport; ii++)
    {
        if (fep->config.ports[ii].running == false)
            continue;

        pthread_join(recvctx[ii].thread, (void *)&result);
        fep_log(fep, FL_ERROR, GET_CALLER_FUNCTION(), "thread join() (%s:%s) return: %d", fep->config.ports[ii].host, fep->config.ports[ii].name, result);
    }

    fep_close(fep);
    return (0);
}

int port_confiuration(FEP *fep, int seqn)
{
    PORT *port = &fep->config.ports[seqn];
    in_addr_t f, t, a;
    int multicast;
    int on = 1;
    int sock;
    struct sockaddr_in sockin;
    struct ip_mreq mreq;
    struct timeval intv;

    f = ntohl(inet_addr(MULTICAST_F));
    t = ntohl(inet_addr(MULTICAST_T));
    a = ntohl(inet_addr(port->ipad));

    if (a >= f && a <= t)
        multicast = 1;

    if (!multicast)
    {
        fep_log(fep, FL_ERROR, GET_CALLER_FUNCTION(), "Multicast IP is error %s:%d ", port->ipad, port->port);
        return (-1);
    }

    if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        fep_log(fep, FL_ERROR, GET_CALLER_FUNCTION(), "Cannot open socket for %s:%d", port->ipad, port->port);
        return (-1);
    }

    memset(&sockin, 0, sizeof(sockin));
    sockin.sin_family = AF_INET;
    // sockin.sin_addr.s_addr = htonl(INADDR_ANY);
    sockin.sin_addr.s_addr = inet_addr(port->ipad);
    sockin.sin_port = htons(port->port);

    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    if (bind(sock, (struct sockaddr *)&sockin, sizeof(sockin)) != 0)
    {
        fep_log(fep, FL_ERROR, GET_CALLER_FUNCTION(), "Cannot bind for %s:%d", port->ipad, port->port);
        close(sock);
        return (-1);
    }

    memset(&mreq, 0, sizeof(mreq));
    mreq.imr_multiaddr.s_addr = inet_addr(port->ipad);
    mreq.imr_interface.s_addr = inet_addr(port->nic_address);

    if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void *)&mreq, sizeof(mreq)) < 0)
    {
        fep_log(fep, FL_ERROR, GET_CALLER_FUNCTION(), "setsockopt error(1) %s:%d", port->ipad, port->port);
        close(sock);
        return (-1);
    }

    intv.tv_sec = port->intv;
    intv.tv_usec = 0;

    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &intv, sizeof(intv)) < 0)
    {
        fep_log(fep, FL_ERROR, GET_CALLER_FUNCTION(), "setsockopt error(2) %s:%d", port->ipad, port->port);
        close(sock);
        return (-1);
    }

    port->sock = sock;

    fep_log(fep, FL_MUST, GET_CALLER_FUNCTION(), "Start to receive from '%s:%s'(%s:%d)", port->host, port->name, port->ipad, port->port);
    return (0);
}

int domain_socket_configuration(FEP *fep, int seqn)
{
    PORT *port = &fep->config.ports[seqn];
    int domain_socket;

    domain_socket = socket(PF_FILE, SOCK_DGRAM, 0);
    if (domain_socket < 0)
    {
        fep_log(fep, FL_ERROR, GET_CALLER_FUNCTION(), "Failed to set domain socket for '%s'", port->name);
        return (-1);
    }

    fep_log(fep, FL_MUST, GET_CALLER_FUNCTION(), "socket() for '%s' complete ..!", port->name);

    return (domain_socket);
}

void recv_to_send(void *argv)
{
    RECVCTX *recvctx = argv;
    int seqn = recvctx->seqn;
    FEP *fep = recvctx->fep;
    PORT *port = &fep->config.ports[seqn];
    int msgl;
    char msgb[MSGBUFF];
    int domain_socket;
    struct sockaddr_in sockin;
    socklen_t slen;
    struct sockaddr_un target_addr;
    struct sockaddr_un target_addr_for_test;

    // Set the socket for receiving
    if (-1 == port_confiuration(fep, seqn))
    {
        fep_log(fep, FL_ERROR, GET_CALLER_FUNCTION(), "Failed to set port configuration");
        pthread_exit(NULL);
    }

    // Set the socket for Unix Domain Socket (monrecv -> monfep)
    if (-1 == (domain_socket = domain_socket_configuration(fep, seqn)))
    {
        fep_log(fep, FL_ERROR, GET_CALLER_FUNCTION(), "Failed to set domain socket configuration");
        pthread_exit(NULL);
    }

    /* Target address setting */
    // REAL Address
    memset(&target_addr, 0, sizeof(target_addr));
    target_addr.sun_family = AF_UNIX;
    strcpy(target_addr.sun_path, port->ipc_name);

    // TEST Address
    memset(&target_addr_for_test, 0, sizeof(target_addr));
    target_addr_for_test.sun_family = AF_UNIX;
    sprintf(target_addr_for_test.sun_path, "%s_test", port->ipc_name);

    while (1)
    {
        slen = sizeof(sockin);
        memset(msgb, 0x00, sizeof(msgb));

        msgl = recvfrom(port->sock, msgb, sizeof(msgb), 0, (struct sockaddr *)&sockin, &slen);

        null_to_space(msgb, msgl);

        if (msgl > 0) // RECEIVED
        {
            /* FOR REAL */
            if (sendto(domain_socket, msgb, strlen(msgb), 0, (struct sockaddr *)&target_addr, sizeof(target_addr)) < 0)
            {
                // fep_log(fep, FL_ERROR, GET_CALLER_FUNCTION(), "%s sendto() error(1)", port->ipc_name);
                continue;
            }

            /* FOR TEST */
            sendto(domain_socket, msgb, strlen(msgb), 0, (struct sockaddr *)&target_addr_for_test, sizeof(target_addr_for_test));
        }
        else // NOT RECEIVED
        {
            if (sendto(domain_socket, NOT_RECEIVED, strlen(NOT_RECEIVED), 0, (struct sockaddr *)&target_addr, sizeof(target_addr)) < 0)
            {
                // fep_log(fep, FL_ERROR, GET_CALLER_FUNCTION(), "%s sendto() error(2)", port->ipc_name);
                continue;
            }
        }
    }

    close(port->sock);
    close(domain_socket);
    pthread_exit(NULL);
}
