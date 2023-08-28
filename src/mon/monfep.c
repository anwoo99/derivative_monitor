#include "main.h"
#include "context.h"

#define MSGBUFF 1024 * 16

extern int mon_log(FEP *fep, PORT *port, char *msgb, int msgl, uint32_t *class_tag);
extern int mon_classify(FEP *fep, PORT *port, char *msgb, uint32_t *class_tag);

int main_process(FEP *fep, PORT *port, char *msgb, int msgl);

void recv_to_fep(void *argv);

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
        pthread_create(&recvctx[ii].thread, NULL, recv_to_fep, &recvctx[ii]);
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

int domain_socket_configuration(FEP *fep, PORT *port)
{
    char unix_file[128];
    int domain_socket;
    int optlen;
    int options = 1;
    int rcvbuf_size, sndbuf_size;
    struct sockaddr_un target_addr;

    /* 도메인 소켓 파일 존재 여부 검사 */
    if (access(port->ipc_name, F_OK) == 0)
        unlink(port->ipc_name);

    if ((domain_socket = socket(PF_FILE, SOCK_DGRAM, 0)) == -1)
    {
        fep_log(fep, FL_ERROR, GET_CALLER_FUNCTION(), "Failed to set domain socket for '%s'", port->name);
        return (-1);
    }

    /* Domain Socket Address 세팅 */
    memset(&target_addr, 0, sizeof(target_addr));
    target_addr.sun_family = AF_UNIX;
    strcpy(target_addr.sun_path, port->ipc_name);

    /* 소켓 BUFFER 세팅 */
    optlen = sizeof(int);
    options = 1024 * 1024 * 16;

    setsockopt(domain_socket, SOL_SOCKET, SO_RCVBUF, &options, optlen);
    setsockopt(domain_socket, SOL_SOCKET, SO_SNDBUF, &options, optlen);

    getsockopt(domain_socket, SOL_SOCKET, SO_RCVBUF, &rcvbuf_size, &optlen);
    getsockopt(domain_socket, SOL_SOCKET, SO_SNDBUF, &sndbuf_size, &optlen);

    if ((bind(domain_socket, (struct sockaddr *)&target_addr, sizeof(target_addr))) == -1)
    {
        close(domain_socket);
        fep_log(fep, FL_ERROR, GET_CALLER_FUNCTION(), "Failed to bind() to '%s' domain socket", port->ipc_name);
        return (-1);
    }

    fep_log(fep, FL_MUST, GET_CALLER_FUNCTION(), "socket() for '%s' complete ..!", port->name);

    return (domain_socket);
}

void recv_to_fep(void *argv)
{
    RECVCTX *recvctx = argv;
    int seqn = recvctx->seqn;
    FEP *fep = recvctx->fep;
    PORT *port = &fep->config.ports[seqn];
    int domain_socket;
    struct sockaddr_un sender;
    socklen_t sender_len;
    char msgb[MSGBUFF];
    int msgl;

    if (-1 == (domain_socket = domain_socket_configuration(fep, port)))
    {
        fep_log(fep, FL_ERROR, GET_CALLER_FUNCTION(), "Failed to set domain socket configuration for '%s'", port->ipc_name);
        pthread_exit(NULL);
    }

    while (1)
    {
        sender_len = sizeof(sender);
        msgl = recvfrom(domain_socket, msgb, sizeof(msgb), 0, (struct sockaddr *)&sender, &sender_len);

        if (msgl < 0)
        {
            fep_log(fep, FL_ERROR, GET_CALLER_FUNCTION(), "Failed to recvfrom() for '%s'", port->ipc_name);

            if (errno = EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
                continue;
            else
                break;
        }
        else
        {
            if (-1 == main_process(fep, port, msgb, msgl))
                break;
        }
    }

    close(domain_socket);
    pthread_exit(NULL);
}

int main_process(FEP *fep, PORT *port, char *msgb, int msgl)
{
    uint32_t class_tag = 0x00;

    /* 데이터 분류 태그 */
    if (-1 == mon_classify(fep, port, msgb, &class_tag))
        return (0);

    /* 데이터 수신 여부 체크 후 Alert -> NaverWorks */
    if (-1 == mon_recv_check(fep, port, &class_tag))
        return (-1);

    /* 데이터 미수신 태그 스킵 */
    if (class_tag & NONE)
        return (0);

    /* 데이터 로그 남기기 */
    if (-1 == mon_log(fep, port, msgb, msgl, &class_tag))
        return (-1);

    /* 데이터 정합성 검사 & 매핑 => folder */
    if (-1 == mon_map(fep, port, msgb, msgl, &class_tag))
        return (-1);

    return (0);
}