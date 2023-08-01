#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <fcntl.h>
#include <string.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/un.h>
#include <mysql/mysql.h>
#include "parson.h"
#include "config.h"
#include "database.h"

/* Logging Flag */
#define FL_MUST 0
#define FL_ERROR 1
#define FL_WARNING 2
#define FL_PROGRESS 3
#define FL_DEBUG 4

/* FEP OPEN MODE */
#define MD_RDONLY 0x01 // 0000 0001
#define MD_RDWR 0x02   // 0000 0010

/* Variables */
typedef struct
{
    char exnm[16];     // Exchange Name
    char procname[32]; // Process Name
    CONFIG config;
    int whoami;       // FEP Open Mode
    MYSQL *connector; // Database Connector
    DATABASE database;
} FEP;

/* Functions */
int fep_config(FEP *fep);
MYSQL *get_database(FEP *fep);
void fep_log(FEP *fep, int level, const char *format, ...);
