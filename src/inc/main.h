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
#include "schema.h"

/* Logging Flag */
#define FL_MUST 0
#define FL_ERROR 1
#define FL_WARNING 2
#define FL_PROGRESS 3
#define FL_DEBUG 4

/* FEP OPEN MODE */
#define MD_RDONLY 0x01 // 0000 0001
#define MD_RDWR 0x02   // 0000 0010

/* DEFINED MESSAGE */
#define NOT_RECEIVED "DAKLWalds1231ADLKNND455LADNL"

/* Variables */
typedef struct
{
    char exnm[16];     // Exchange Name
    char procname[32]; // Process Name
    CONFIG config;
    int whoami; // FEP Open Mode
    void *fold; // Folder Pointer
    void *arch; // Archive Pointer
} FEP;

/* config.c */
int fep_config(FEP *fep);

/* log.c */
void fep_log(FEP *fep, int level, const char *format, ...);

/* shm.c */
unsigned long djb2(const char *str);
int fep_shminit(FEP *fep);
