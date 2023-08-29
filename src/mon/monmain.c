/****************************************************************************************/
/*  Rev. History: Ver   Date       Description                                          */
/*                ----  ---------- -----------------------------------------------------*/
/*                1.0   2022-07-18 select() can monitor csock                           */
/****************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <libgen.h>
#include <fcntl.h>
#include <errno.h>
#include <pwd.h>
#include <time.h>
#include <wait.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "main.h"

#define MAX_PROCESS_TABLE 2048
#define MAX_MULTI_PROCESS 32
#define INIT_PORT 9991

#define ONCE 1
#define RESPAWN 2
#define WAIT 3
#define OFF 4

static struct process
{
    char name[16];  /* process name			*/
    char path[128]; /* program path			*/
    int mode;       /* invoke mode			*/
    int level;      /* run level			*/
    int diff;       /* modified tags		*/
    int nofp;       /* no of precess		*/
    struct
    {
        pid_t xpid;  /* process id			*/
        time_t xtim; /* execution time stamp		*/
    } proc[MAX_MULTI_PROCESS];
} process[MAX_PROCESS_TABLE];

int admin_command_issue(int, char *);
int check_process_table();
int fork_and_exec_process(struct process *);
void clean_zombie_process(int);
int reset_all_process();
void kill_all_process();
int get_process_table(int);
int setup_fep_profile();
int trace_log(const char *format, ...);
int _str2words(char *, char *[], int);
char *_getwords(char *, char *);

static char path4me[128];        // run path for me
static int n_process;            // no of process configured
static int run_level = 9;        // default run level = 9
static int stepbystep = 1;       // step by step with run leve;
static int suspended = 0;        // suspended procee exec
static char table_file[128];     // table file name (default = init.tab)
static time_t modified_time = 0; // tbale modified time stamp
static char userid[40];          // user's id

/*
 * main()
 *  main procedure for FOX system demon
 */
int main(int argc, char *argv[])
{
    char *whoami;
    struct sockaddr_in sockaddr_in;
    socklen_t socklen;
    fd_set clist;
    struct timeval timeval;
    int lsock, csock, options;
    char command[32], rspb[256];
    pid_t pid;
    int sock;
    int ii, jj, nn;

    whoami = basename(argv[0]);
    if (strcmp(whoami, "moninit") == 0)
    {
        memset(command, 0, sizeof(command));
        while ((options = getopt(argc, argv, "qnv0123456789")) != EOF)
        {
            switch (options)
            {
            case 'q':
            case 'n':
            case 'v':
                command[0] = options;
                break;
            default:
                command[0] = options;
                break;
            }
        }
        switch (command[0])
        {
        case '\0':
            if (argc >= 2 && strlen(argv[1]) == 1)
                command[0] = argv[1][0];
            else
                command[0] = 'q';
            break;
        default:
            break;
        }

        /* connect to monmain demon process. */
        memset(&sockaddr_in, 0, sizeof(sockaddr_in));
        sockaddr_in.sin_family = AF_INET;
        sockaddr_in.sin_port = htons(INIT_PORT);
        sockaddr_in.sin_addr.s_addr = inet_addr("127.0.0.1");
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0)
        {
            fprintf(stderr, "Cannot connect to the monmain.\n");
            return (-1);
        }
        if (connect(sock, (struct sockaddr *)&sockaddr_in, sizeof(sockaddr_in)) != 0)
        {
            close(sock);
            fprintf(stderr, "Cannot connect to the monmain.\n");
            return (-1);
        }
        write(sock, command, 16);
        nn = 0;
        while (1)
        {

            ii = read(sock, &rspb[nn], sizeof(rspb) - nn);
            if (ii <= 0)
                break;
            nn += ii;
        }
        close(sock);
        rspb[nn] = '\0';
        if (nn <= 0)
            fprintf(stderr, "No response from a monmain !!!!\n");
        else
            fprintf(stdout, "%s\n", rspb);
        return (0);
    }

    // monmain
    if (isatty(1) == 1)
    {
        if ((pid = fork()))
            return (pid);
        close(0);
        close(1);
        close(2);
    }
    strcpy(path4me, argv[0]);
    while ((options = getopt(argc, argv, "u:l:d")) != EOF)
    {
        switch (options)
        {
        case 'u':
            strcpy(userid, optarg);
            break;
        case 'l':
            run_level = atoi(optarg);
            break;
        default:
            break;
        }
    }
    sprintf(table_file, "%s/MONMAIN.tab", ETC_DIR);
    
    if (get_process_table(1) != 0)
        return (-1);

    atexit(kill_all_process);
    signal(SIGTERM, exit);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, exit);
    signal(SIGCHLD, clean_zombie_process);

    /* bind local network for issued command */
    memset(&sockaddr_in, 0, sizeof(sockaddr_in));
    sockaddr_in.sin_family = AF_INET;
    sockaddr_in.sin_port = htons(INIT_PORT);
    sockaddr_in.sin_addr.s_addr = inet_addr("127.0.0.1");
    lsock = socket(AF_INET, SOCK_STREAM, 0);
    if (lsock < 0)
    {
        trace_log("Sorry ! fail to initialize session for system command.\n");
        return (-1);
    }
    options = 1;
    setsockopt(lsock, SOL_SOCKET, SO_REUSEADDR, &options, sizeof(int));
    if (bind(lsock, (struct sockaddr *)&sockaddr_in, sizeof(sockaddr_in)) != 0)
    {
        close(lsock);
        trace_log("Sorry ! fail to bind session for system command.\n");
        return (-1);
    }
    listen(lsock, 20);
    fcntl(lsock, F_SETFD, FD_CLOEXEC);

    //
    // invoke all process
    //
    reset_all_process();
    sighold(SIGCHLD);
    for (ii = 0; ii <= run_level; ii++)
    {
        for (jj = 0, nn = 0; jj < n_process; jj++)
        {
            if (process[jj].level != ii)
                continue;
            process[jj].diff = 0;
            switch (process[jj].mode)
            {
            case OFF:
                continue;
            default:
                break;
            }
            fork_and_exec_process(&process[jj]);
            nn++;
        }
        if (nn)
            sleep(3);
    }
    sigrelse(SIGCHLD);

    //
    // service loop
    //
    for (;;)
    {
        clean_zombie_process(0);
        FD_ZERO(&clist);
        FD_SET(lsock, &clist);
        timeval.tv_sec = 300;
        timeval.tv_usec = 0;
        if (select(lsock + 1, &clist, NULL, NULL, &timeval) > 0)
        {
            socklen = sizeof(struct sockaddr_in);
            csock = accept(lsock, (struct sockaddr *)&sockaddr_in, &socklen);
            if (csock >= 0)
            {
                FD_ZERO(&clist);
                FD_SET(csock, &clist);
                timeval.tv_sec = 3;
                timeval.tv_usec = 0;
                if (select(csock + 1, &clist, NULL, NULL, &timeval) > 0)
                {
                    if (read(csock, command, sizeof(command)) == 16)
                    {
                        command[16] = '\0';
                        admin_command_issue(csock, command);
                    }
                }
                close(csock);
            }
        }
        sighold(SIGCHLD);
        get_process_table(0);
        check_process_table();
        sigrelse(SIGCHLD);
    }
}

//
// check_process_table()
// Check process on table
//
int check_process_table()
{
    time_t clock;
    int sftsz;
    int level, many;
    int check;
    int ii, jj;

    if (suspended || !n_process)
        return (0);

    check = 0;
    //
    // kill outside level process
    //
    for (ii = 0; ii < n_process; ii++)
    {
        if (process[ii].level <= run_level)
            continue;
        for (jj = 0; jj < process[ii].nofp; jj++)
        {
            if (process[ii].proc[jj].xpid > 1)
            {
                kill(process[ii].proc[jj].xpid, SIGTERM);
                check = 1;
            }
        }
    }

    //
    // kill deteled or changed proceess
    //
    for (ii = 0; ii < n_process; ii++)
    {
        switch (process[ii].mode)
        {
        case OFF:
            for (jj = 0; jj < process[ii].nofp; jj++)
            {
                if (process[ii].proc[jj].xpid > 1 && kill(process[ii].proc[jj].xpid, 0) >= 0)
                {
                    check = 1;
                    kill(process[ii].proc[jj].xpid, SIGTERM);
                }
            }
            break;
        default:
            break;
        }
        switch (process[ii].diff)
        {
        case -1: /* deleted entry */
            for (jj = 0; jj < process[ii].nofp; jj++)
            {
                if (process[ii].proc[jj].xpid > 1 && kill(process[ii].proc[jj].xpid, 0) >= 0)
                    kill(process[ii].proc[jj].xpid, SIGKILL);
            }
            sftsz = (n_process - ii - 1) * sizeof(struct process);
            memcpy(&process[ii], &process[ii + 1], sftsz);
            ii--;
            n_process--;
            continue;
        case 1: /* changed entry */
            process[ii].diff = 0;
            for (jj = 0; jj < process[ii].nofp; jj++)
            {
                if (process[ii].proc[jj].xpid > 1 && kill(process[ii].proc[jj].xpid, 0) >= 0)
                    kill(process[ii].proc[jj].xpid, SIGKILL);
                process[ii].proc[jj].xpid = 0;
            }
            break;
        }
    }

    //
    // invoke dead or uninitialized process
    //
    for (level = 1; level <= run_level; level++)
    {
        many = 0;
        for (ii = 0; ii < n_process; ii++)
        {
            switch (process[ii].mode)
            {
            case OFF:
                continue;
            default:
                break;
            }
            if (process[ii].level > level)
                continue;

            for (jj = 0; jj < process[ii].nofp; jj++)
            {
                if (process[ii].proc[jj].xpid > 1 && kill(process[ii].proc[jj].xpid, 0) >= 0)
                    continue;
                clock = time(0);
                clock -= process[ii].proc[jj].xtim;
                if (clock > 2)
                    break;
            }
            if (jj >= process[ii].nofp)
                continue;
            switch (process[ii].mode)
            {
            case RESPAWN:
                break;
            default:
                continue;
            }
            fork_and_exec_process(&process[ii]);
            many++;
        }
        if (many && stepbystep)
            sleep(2);
    }
    stepbystep = 0;

    if (!check)
        return (0);
    sleep(1);
    for (ii = 0; ii < n_process; ii++)
    {
        switch (process[ii].mode)
        {
        case OFF:
            for (jj = 0; jj < process[ii].nofp; jj++)
            {
                if (process[ii].proc[jj].xpid > 1 && kill(process[ii].proc[jj].xpid, 0) >= 0)
                    kill(process[ii].proc[jj].xpid, SIGKILL);
            }
            break;
        default:
            break;
        }
        if (process[ii].level <= run_level)
            continue;
        for (jj = 0; jj < process[ii].nofp; jj++)
        {
            if (process[ii].proc[jj].xpid > 1)
                kill(process[ii].proc[jj].xpid, SIGKILL);
        }
    }

    return (0);
}

/*
 * fork_and_exec_process()
 * Execution process.
 */
int fork_and_exec_process(struct process *xproc)
{
    static char *pargv[10];
    static char pargs[12][80];
    struct stat xstat;
    pid_t xpid = 0, zpid;
    int okok, xs;
    int ii, jj;

    for (ii = 0; ii < 12; ii++)
        pargs[ii][0] = '\0';

    sscanf(xproc->path, "%s %s %s %s %s %s %s %s %s", pargs[0], pargs[1], pargs[2],
           pargs[3], pargs[4], pargs[5], pargs[6], pargs[7], pargs[8]);

    for (ii = 0, jj = 0; ii <= 8; ii++)
    {
        if (strlen(pargs[ii]) <= 0)
            break;
        pargv[jj++] = pargs[ii];
    }
    pargv[jj] = '\0';
    if (stat(pargs[0], &xstat) != 0 || (xstat.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) == 0)
        return (0);

    for (jj = 0, okok = 0; jj < xproc->nofp; jj++)
    {
        if (xproc->proc[jj].xpid > 0 && kill(xproc->proc[jj].xpid, 0) >= 0)
            continue;
        switch ((xpid = fork()))
        {
        case -1:
            continue;
        case 0:
            signal(SIGTERM, SIG_DFL);
            signal(SIGINT, SIG_DFL);
            signal(SIGCHLD, SIG_DFL);
            sigrelse(SIGCHLD);

            setgid(xstat.st_gid);
            setuid(xstat.st_uid);
            execv(pargs[0], pargv);
            exit(0);
        default:
            xproc->proc[jj].xpid = xpid;
            xproc->proc[jj].xtim = time(0);
            break;
        }
        okok++;
    }
    switch (xproc->mode)
    {
    case WAIT:
        if (!okok)
            break;
        for (;;)
        {
            zpid = waitpid(xpid, &xs, 0);
            if (zpid == xpid)
                break;
            if (zpid == -1 && errno == EINTR)
                continue;
            break;
        }
        break;
    default:
        break;
    }
    return (0);
}

//
// admin_command_issue()
//
int admin_command_issue(int peer, char *command)
{
    char rspb[128];

    trace_log("admin command receive %s", command);
    sprintf(rspb, "Your command is executed successfully");
    switch (command[0])
    {
    case 'Q':
    case 'q':
        suspended = 0;
        stepbystep = 1;
        break;
    case 'N':
    case 'n':
        suspended = 1;
        break;
    case 'V':
    case 'v':
        sprintf(rspb, "Run level is %d with normal mode", run_level);
        if (suspended)
            sprintf(&rspb[strlen(rspb)], "\nSuspended...");
        break;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        run_level = command[0] & 0x0f;
        stepbystep = 1;
        break;
    default:
        sprintf(rspb, "Sorry !!! unknown command");
        break;
    }
    write(peer, rspb, strlen(rspb));
    trace_log("admin command %c issued", command[0]);
    return (0);
}

/*
 * reset_all_process()
 * Kill all process registered process table
 */
int reset_all_process()
{
    FILE *chkf;
    char command[128], logfile[256];
    char line_b[512];
    char pid_b[128], xxx_b[128], tim_b[128], nam_b[128];
    char path[128];
    char *base;
    pid_t pid;
    int ii;

    sprintf(logfile, "/tmp/%d.p", getpid());
    sprintf(command, "/bin/ps -e > %s 2>&1", logfile);
    system(command);
    chkf = fopen(logfile, "r");
    if (chkf == NULL)
    {
        remove(logfile);
        return (0);
    }
    while ((fgets(line_b, sizeof(line_b), chkf)) == line_b)
    {
        pid_b[0] = '\0';
        xxx_b[0] = '\0';
        tim_b[0] = '\0';
        nam_b[0] = '\0';
        sscanf(line_b, "%s %s %s %s", pid_b, xxx_b, tim_b, nam_b);
        if (strlen(nam_b) <= 0 || strlen(pid_b) <= 0)
            continue;
        pid = atoi(pid_b);
        if (pid <= 1)
            continue;
        for (ii = 0; ii < n_process; ii++)
        {
            sscanf(process[ii].path, "%s", path);
            base = basename(path);
            if (base == NULL)
                continue;
            if (strcmp(base, nam_b) != 0)
                continue;
            kill(pid, SIGKILL);
        }
    }
    fclose(chkf);
    remove(logfile);
    return (0);
}

/*
 * kill_all_process()
 * Kill all running process
 */
void kill_all_process()
{
    int ii, jj;

    for (ii = 0; ii < n_process; ii++)
    {
        for (jj = 0; jj < process[ii].nofp; jj++)
        {
            if (process[ii].proc[jj].xpid <= 0)
                continue;
            kill(process[ii].proc[jj].xpid, SIGTERM);
        }
    }
    sleep(2);
    for (ii = 0; ii < n_process; ii++)
    {
        for (jj = 0; jj < process[ii].nofp; jj++)
        {
            if (process[ii].proc[jj].xpid <= 0)
                continue;
            if (kill(process[ii].proc[jj].xpid, 0) >= 0)
                kill(process[ii].proc[jj].xpid, SIGKILL);
        }
    }
}

/*
 * clean_zombie_process()
 * Check exit process.
 */
void clean_zombie_process(int signo)
{
    pid_t xpid;
    int checked;
    int ii, jj;

    for (;;)
    {
        xpid = waitpid(-1, &checked, WNOHANG);
        if (xpid <= 0)
            break;
        for (ii = 0, checked = 0; ii < n_process && checked == 0; ii++)
        {
            for (jj = 0; jj < process[ii].nofp && checked == 0; jj++)
            {
                if (process[ii].proc[jj].xpid == xpid)
                {
                    process[ii].proc[jj].xpid = 0;
                    checked = 1;
                    if (process[ii].level <= run_level)
                        trace_log("'%s' process aborted.", process[ii].path);
                }
            }
        }
    }
    signal(SIGCHLD, clean_zombie_process);
}

/*
 * get_process_table()
 * Initialize FOX process table.
 */
int get_process_table(int first)
{
    struct stat xstat;
    struct process xproc;
    char lineB[256], argsB[16][80], *argsP[16];
    char *lptr;
    int argsN;
    FILE *cFile;
    int ii, jj;

    if (stat(table_file, &xstat) != 0)
    {
        if (first)
            trace_log("Cannot check FOX home directory. Please run me with full path");
        return (-1);
    }
    if (first)
        setup_fep_profile();

    if (xstat.st_mtime == modified_time)
        return (0);
    modified_time = xstat.st_mtime;

    for (ii = 0; ii < n_process; ii++)
        process[ii].diff = -1; // anyway set modified

    cFile = fopen(table_file, "r");
    if (cFile == NULL)
        return (0);
    for (ii = 0; ii < 16; ii++)
        argsP[ii] = argsB[ii];

    while ((fgets(lineB, sizeof(lineB), cFile)) == lineB)
    {
        for (ii = 0; ii < 4; ii++)
        {
            if ((lptr = strstr(lineB, ":")) == NULL)
                break;
            *lptr = ' ';
        }
        argsN = _str2words(lineB, argsP, 16);
        if (argsN < 3)
            continue;
        if (strlen(argsP[0]) >= 16 || strlen(argsP[1]) >= 16) // 0-1 entity name, mode
            continue;
        memset(&xproc, 0, sizeof(struct process));
        if (strcmp(argsP[2], "wait") == 0)
            xproc.mode = WAIT;
        else if (strcmp(argsP[2], "respawn") == 0)
            xproc.mode = RESPAWN;
        else if (strcmp(argsP[2], "once") == 0)
            xproc.mode = ONCE;
        else if (strcmp(argsP[2], "off") == 0)
            xproc.mode = OFF;
        else
            continue;
        strcpy(xproc.name, argsP[0]); // 0  entity name
        xproc.level = atoi(argsP[1]); // 2  run level
        if (xproc.level <= 0)
            xproc.level = 1;
        else if (xproc.level > 9)
            xproc.level = 9;
        xproc.nofp = atoi(argsP[3]);   // 3  no of process
        strcpy(xproc.path, argsP[4]);  // 4  path
        for (ii = 5; ii < argsN; ii++) // 5~ argument
        {
            strcat(xproc.path, " ");
            strcat(xproc.path, argsP[ii]);
        }
        switch (xproc.mode)
        {
        case RESPAWN:
            break;
        case OFF:
            break;
        default:
            if (xproc.nofp > 1)
                xproc.nofp = 1;
            break;
        }
        if (xproc.nofp > MAX_MULTI_PROCESS)
            xproc.nofp = MAX_MULTI_PROCESS;
        xproc.diff = 1;
        for (ii = 0; ii < n_process; ii++)
        {
            if (strcmp(process[ii].name, xproc.name) != 0)
                continue;
            if (strcmp(process[ii].path, xproc.path) == 0 && process[ii].mode == xproc.mode &&
                process[ii].level == xproc.level && process[ii].nofp == xproc.nofp)
            {
                process[ii].diff = 0;
                break;
            }
            for (jj = 0; jj < process[ii].nofp; jj++)
            {
                xproc.proc[jj].xpid = process[ii].proc[jj].xpid;
                xproc.proc[jj].xtim = process[ii].proc[jj].xtim;
            }
            memcpy(&process[ii], &xproc, sizeof(struct process));
            break;
        }
        if (ii >= n_process)
        {
            memcpy(&process[n_process], &xproc, sizeof(struct process));
            n_process++;
            if (n_process >= MAX_PROCESS_TABLE)
                break;
        }
    }
    fclose(cFile);
    if (first)
        trace_log("Total %d process table entities", n_process);
    return (0);
}

//
// setup_fep_profile()
// Put environments on specified file.
//
int setup_fep_profile()
{
    char profile[128];
    FILE *chckF;
    char lineX[512], lineB[512];
    char chckB[4][256];
    char *wordP[4], *equaL;
    char *cenvP, *tagsP, *defsP;
    int wordN;
    int ii, jj;

    sprintf(profile, "%s/profile", ETC_DIR);
    chckF = fopen(profile, "r");
    if (chckF == NULL)
        return (0);

    for (ii = 0; ii < 4; ii++)
        wordP[ii] = chckB[ii];
    while ((fgets(lineB, sizeof(lineB) - 1, chckF)) == lineB)
    {
        for (ii = 0, jj = 0; ii < strlen(lineB); ii++)
        {
            if (lineB[ii] == ' ')
                continue;
            if (lineB[ii] == '*' || lineB[ii] == '#')
                break;
            lineX[jj++] = lineB[ii];
        }
        lineX[jj] = '\0';
        wordN = _str2words(lineX, wordP, 4);
        if (wordN <= 0)
            continue;
        equaL = strstr(wordP[0], "=");
        if (equaL == NULL)
            continue;

        cenvP = malloc(strlen(wordP[0]) + 512);
        if (cenvP == NULL)
            continue;

        tagsP = wordP[0];
        defsP = equaL + 1;
        *equaL = '\0';

        sprintf(cenvP, "%s=%s", tagsP, defsP);
        putenv(cenvP);
    }
    fclose(chckF);
    return (0);
}

/*
 * int set_fep_user_id()
 * Set user's id
 */
int set_fep_user_id(char *usrname)
{
    uid_t uid;
    struct passwd *pwd;

    pwd = (struct passwd *)getpwnam(usrname);
    if (pwd == NULL)
        return (-1);
    uid = getuid();
    if (uid == 0)
    {
        setgid(pwd->pw_gid);
        setuid(pwd->pw_uid);
    }
    else if (uid != pwd->pw_uid)
        return (-1);
    uid = getuid();
    if (uid != pwd->pw_uid)
        return (-1);
    return (0);
}

/*
 * _str2words()
 * Seprator words
 */
int _str2words(char *lineB, char *wordB[], int wordN)
{
    char *nextP;
    char chckB[256];
    int ii;

    nextP = lineB;
    for (ii = 0; ii < wordN; ii++)
    {
        if (strlen(nextP) <= 0)
            break;
        nextP = _getwords(nextP, chckB);
        if (strlen(chckB) <= 0 || chckB[0] == '#')
            break;
        strcpy(wordB[ii], chckB);
    }
    return (ii);
}

/*
 * _getwords()
 * Get word string from input string
 */
char *_getwords(char *istr, char *word)
{
    int ii, jj;
    int ilen;

    ilen = strlen(istr);
    for (ii = 0, jj = 0; ii < ilen; ii++, istr++)
    {
        switch (*istr)
        {
        case ' ':
        case '\t':
        case '\n':
        case '\r':
            if (jj != 0)
                break;
            continue;
        default:
            word[jj++] = *istr;
            continue;
        }
        break;
    }
    word[jj] = '\0';
    if (ii < ilen && (*istr == '\n' || *istr == '\r'))
    {
        ii++;
        istr++;
    }
    if (ii < ilen && (*istr == '\n' || *istr == '\r'))
        istr++;
    return (istr);
}

#include <stdarg.h>

int trace_log(const char *format, ...)
{
    va_list vl;
    struct stat xstat;
    char path[128], logB[256], mode[9];
    time_t clock;
    struct tm *tm, tx;
    FILE *logF;

    va_start(vl, format);
    vsprintf(logB, format, vl);
    va_end(vl);

    clock = time(0);
    tm = localtime(&clock);
    memcpy(&tx, tm, sizeof(struct tm));
    sprintf(path, "%s/init-%d.log", LOG_DIR, tm->tm_wday);
    strcpy(mode, "a");
    if (stat(path, &xstat) == 0)
    {
        tm = localtime(&xstat.st_mtime);
        if (tm->tm_yday != tx.tm_yday)
            strcpy(mode, "w");
    }
    if ((logF = fopen(path, mode)) == NULL)
        return (0);
    fprintf(logF, "%02d:%02d:%02d %02d:%02d:%02d %s\n", tx.tm_year % 100, tx.tm_mon + 1, tx.tm_mday,
            tx.tm_hour, tx.tm_min, tx.tm_sec, logB);
    fclose(logF);
    return (0);
}
