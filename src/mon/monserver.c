#include "main.h"

#define MAX_EXCHANGE 64
#define _KILL_ALL 1
#define _CONFIG_UPDATE 2
#define _RUN_ALL 3

typedef struct
{
    char exchange[128];
    int running;
} PROCESS_CONFIG;

PROCESS_CONFIG process_config[MAX_EXCHANGE];

typedef struct
{
    char exchange[128];
    char procname[512];
    pid_t pid;
    time_t uptime;
} RUNNING_TABLE;
static int RUNNING_TABLE_VREC = 0;

RUNNING_TABLE running_table[MAX_EXCHANGE];

static int EXCHANGE_VREC = 0;
static int RUNNING = 1;

static int CONFIG_UPDATE_FLAG = 0;

char RECV_PIPE[128];
char SEND_PIPE[128];
char COMMAND1[512] = "monrecv";
char COMMAND2[512] = "monfep";
pthread_mutex_t lock;

void writelog(const char *format, ...);
int get_process_config();

int main()
{
    pthread_t thread;
    int result;

    writelog("Main server for monitor start..!");

    memset(running_table, 0x00, sizeof(running_table));
    memset(process_config, 0x00, sizeof(process_config));

    if (-1 == get_process_config())
        return (-1);

    if (-1 == create_named_pipe())
        return (-1);

    if (-1 == run_all_process())
        return (-1);

    atexit(kill_all_process);
    signal(SIGTERM, exit);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, exit);
    signal(SIGCHLD, clean_zombie_process);

    pthread_mutex_init(&lock);

    pthread_create(&thread, NULL, read_named_pipe, NULL);

    while (1)
    {
        switch (CONFIG_UPDATE_FLAG)
        {
        case _KILL_ALL:
            pthread_mutex_lock(lock);
            RUNNING = 0;
            CONFIG_UPDATE_FLAG = 0;
            pthread_mutex_unlock(lock);
            break;
        case _CONFIG_UPDATE:
            if (-1 == get_process_config())
                return (-1);

            pthread_mutex_lock(lock);
            CONFIG_UPDATE_FLAG = 0;
            pthread_mutex_unlock(lock);
            break;
        case _RUN_ALL:
            pthread_mutex_lock(lock);
            RUNNING = 1;
            CONFIG_UPDATE_FLAG = 0;
            pthread_mutex_unlock(lock);
        default:
            break;
        }

        if (RUNNING)
            check_all_process();
    }
}

int cmp_run_table(RUNNING_TABLE *a, RUNNING_TABLE *b)
{
    int exch_cmp = strcmp(a->exchange, b->exchange);

    if (exch_cmp != 0)
        return exch_cmp;

    return (strcmp(a->procname, b->procname));
}

RUNNING_TABLE *get_running_table(char *exnm, char *procname)
{
    RUNNING_TABLE temp;

    strcpy(temp.exchange, exnm);

    return (bsearch(&temp, running_table, sizeof(RUNNING_TABLE) * MAX_EXCHANGE, sizeof(RUNNING_TABLE), cmp_run_table));
}

RUNNING_TABLE *new_running_table(char *exnm, char *procname)
{
    RUNNING_TABLE temp;

    memset(&temp, 0x00, sizeof(temp));

    strcpy(temp.exchange, exnm);
    strcpy(temp.procname, procname);
    temp.uptime = time(NULL);

    memcpy(&running_table[RUNNING_TABLE_VREC++], &temp, sizeof(RUNNING_TABLE));
    qsort(running_table, RUNNING_TABLE_VREC, sizeof(RUNNING_TABLE), cmp_run_table);

    return (get_running_table(exnm));
}

void del_running_table(char *exnm, char *procname)
{
    RUNNING_TABLE *target, *remain;
    char seqn;
    int size;

    target = get_running_table(exnm, procname);
    remain = &target[1];

    if (target == NULL)
        return;

    seqn = (int)(((unsigned long)target - (unsigned long)running_table) / sizeof(RUNNING_TABLE));
    size = (RUNNING_TABLE_VREC - seqn - 1) * sizeof(RUNNING_TABLE);

    memmove(target, remain, size);
    RUNNING_TABLE_VREC--;
}

int run_process(char *exnm, char *procname)
{
    RUNNING_TABLE *run_table;
    char cmd[512];
    pid_t pid;
    char *args[] = {procname, exnm, NULL};
    int ii;

    sprintf(cmd, "%s/%s", BIN_DIR, procname);

    run_table = get_running_table(exnm, procname);

    /* 이미 실행 중인 경우 */
    if (run_table != NULL)
    {
        return (0);
    }

    pid = fork();

    if (pid < 0)
    {
        writelog("Failed to fork for '%s'", exnm);
        return (-1);
    }
    else if (pid == 0) // Child
    {
        signal(SIGINT, SIG_DFL);
        signal(SIGCHLD, SIG_DFL);
        sigrelse(SIGCHLD);

        if (execv(cmd, args) == -1)
        {
            del_running_table(exnm, procname);
            writelog("%s is aborted..!", cmd);
            exit(0);
        }

        del_running_table(exnm, procname);
        exit(0);
    }
    else // Parent
    {
        run_table = new_running_table(exnm, procname);
        run_table->pid = pid;
        return (0);
    }

    return (0);
}

int kill_process(char *exnm, char *procname)
{
    RUNNING_TABLE *run_table;

    run_table = get_running_table(exnm, procname);

    /* 러닝 테이블에 없는 경우 */
    if (run_table == NULL)
        return (0);

    if (kill(run_table->pid, SIGTERM) == 0)
    {
        writelog("%s for %s process is killed.", run_table->procname, run_table->exchange);
        del_running_table(exnm, procname);
        return (0);
    }

    return (0);
}

int kill_all_process()
{
    int ii;

    for (ii = 0; ii < EXCHANGE_VREC; ii++)
    {
        kill_process(process_config[ii].exchange, COMMAND1);
        kill_process(process_config[ii].exchange, COMMAND2);
    }

    return (0);
}

int run_all_process()
{
    int ii;

    for (ii = 0; ii < EXCHANGE_VREC; ii++)
    {
        if (process_config[ii].running == 0)
            continue;

        if (-1 == run_process(process_config[ii].exchange, COMMAND1))
        {
            writelog("Cannot run '%s' process for '%s'", COMMAND1, process_config[ii].exchange);

            return (-1);
        }

        if (-1 == run_process(process_config[ii].exchange, COMMAND2))
        {
            writelog("Cannot run '%s' process for '%s'", COMMAND2, process_config[ii].exchange);

            return (-1);
        }
    }

    return (0);
}

int check_all_process()
{
    RUNNING_TABLE *run_table;
    int ii;

    for (ii = 0; ii < EXCHANGE_VREC; ii++)
    {
        /* Config는 실행 중, Running Table에 없는 경우 => 실행 */
        if (process_config[ii].running == 1)
        {
            if (get_running_table(process_config[ii].exchange, COMMAND1) == NULL)
                run_process(process_config[ii].exchange, COMMAND1);

            if (get_running_table(process_config[ii].exchange, COMMAND2) == NULL)
                run_process(process_config[ii].exchange, COMMAND2);
        }
        /* Config는 종료, Running Table에 있는 경우 => 종료 */
        else
        {
            if (get_running_table(process_config[ii].exchange, COMMAND1) != NULL)
                kill_process(process_config[ii].exchange, COMMAND1);

            if (get_running_table(process_config[ii].exchange, COMMAND2) != NULL)
                kill_process(process_config[ii].exchange, COMMAND2);
        }
    }
}

int create_named_pipe()
{
    sprintf(RECV_PIPE, "%s/RECV_PIPE", TMP_DIR);
    sprintf(SEND_PIPE, "%s/SEND_PIPE", TMP_DIR);

    if (mkfifo(RECV_PIPE, 0666) != 0 || mkfifo(SEND_PIPE, 0666) != 0)
    {
        writelog("Failed to create pipes.");
        return (-1);
    }

    writelog("Pipes successfully created.");
    return (0);
}

void read_named_pipe(void *argv)
{
    int fd = open(RECV_PIPE, O_RDONLY);
    fd_set read_set;
    int ready;
    struct timeval timeout;

    timeout.tv_sec = 300;
    timeout.tv_usec = 0;

    if (fd == -1)
    {
        writelog("Cannot open '%s' pipe", RECV_PIPE);
        pthread_exit(NULL);
    }

    while (1)
    {
        FD_ZERO(&read_set);
        FD_SET(fd, &read_set);

        ready = select(fd + 1, &read_set, NULL, NULL, &timeout);

        if (ready && FD_ISSET(fd, &read_set))
        {
            char msgb[1024];
            ssize_t length = read(fd, msgb, sizeof(msgb) - 1);

            if (length)
            {
                msgb[length] = '\0';

                if (strcmp(msgb, CONFIG_UPDATE_MSG) == 0)
                {
                    pthread_mutex_lock(lock);
                    CONFIG_UPDATE_FLAG = _CONFIG_UPDATE;
                    pthread_mutex_unlock(lock);
                }
                else if (strcmp(msgb, KILL_ALL_MSG) == 0)
                {
                    pthread_mutex_lock(lock);
                    CONFIG_UPDATE_FLAG = _KILL_ALL;
                    pthread_mutex_unlock(lock);
                }
                else if (strcmp(msgb, RUN_ALL_MSG) == 0)
                {
                    pthread_mutex_lock(lock);
                    CONFIG_UPDATE_FLAG = _RUN_ALL;
                    pthread_mutex_unlock(lock);
                }
            }
        }
    }
}

int get_process_config()
{
    char filename[64];
    JSON_Value *rootValue;
    JSON_Array *rootArray;
    size_t rootArrayCount;
    size_t i;

    sprintf(filename, "%s/MONMAIN.json", ETC_DIR);

    rootValue = json_parse_file(filename);

    if (!rootValue)
    {
        main_log("Cannot get the root value from '%s' file", filename);
        return -1;
    }

    rootArray = json_value_get_array(rootValue);
    rootArrayCount = json_array_get_count(rootArray);

    EXCHANGE_VREC = (rootArrayCount > MAX_EXCHANGE) ? MAX_EXCHANGE : rootArrayCount;

    for (i = 0; i < EXCHANGE_VREC; i++)
    {
        JSON_Object *object = json_array_get_object(rootArray, i);
        const char *exchange = json_object_get_string(object, "exchange");
        int running = (int)json_object_get_number(object, "running");

        strncpy(process_config[i].exchange, exchange, sizeof(process_config[i].exchange));
        process_config[i].running = running;
    }

    json_value_free(rootValue);
    return 0;
}

void clean_zombie_process(int signo)
{
    pid_t xpid;
    int checked;
    int ii, jj;
    char cmd[512];

    for (;;)
    {
        xpid = waitpid(-1, &checked, WNOHANG);

        if (xpid <= 0)
            break;

        for (ii = 0, checked = 0; ii < RUNNING_TABLE_VREC && checked == 0; ii++)
        {
            if (running_table[ii].pid == xpid)
            {
                checked = 1;
                del_running_table(running_table[ii].exchange, running_table[ii].procname);

                sprintf(cmd, "%s/%s", BIN_DIR, running_table[ii].procname);
                writelog("%s is aborted..!", cmd);
            }
        }
    }
    signal(SIGCHLD, clean_zombie_process);
}

void writelog(const char *format, ...)
{
    time_t korean_time, make_time, current;
    struct tm korean_tm, make_tm;
    char logpath[128];
    char mode[2] = "a";
    struct stat lstat;
    char logmsg[1024 * 8];
    FILE *logF;

    current = time(NULL);
    fep_utc2kst(current, &korean_time, &korean_tm);

    snprintf(logpath, sizeof(logpath), "%s/init-%d.log", LOG_DIR, korean_tm.tm_wday);

    if (stat(logpath, &lstat) == 0)
    {
        fep_utc2kst(lstat.st_mtime, &make_time, &make_tm);

        if (make_tm.tm_yday != korean_tm.tm_yday)
            mode[0] = 'w';
    }

    snprintf(logmsg, sizeof(logmsg), "%02d/%02d %02d:%02d:%02d ", korean_tm.tm_mon + 1, korean_tm.tm_mday, korean_tm.tm_hour, korean_tm.tm_min, korean_tm.tm_sec);

    va_list vl;
    va_start(vl, format);
    vsnprintf(&logmsg[strlen(logmsg)], sizeof(logmsg) - strlen(logmsg), format, vl);
    va_end(vl);

    logF = fopen(logpath, mode);

    if (!logF)
    {
        printf("Failed..!(%s)\n", strerror(errno));
        return;
    }
    fprintf(logF, "%s\n", logmsg);

    fclose(logF);
}