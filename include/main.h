#ifndef _MAIN_H_
#define _MAIN_H_

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
#include <stdbool.h>
#include <ifaddrs.h>
#include <dirent.h>
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

/* Print Function's name for logging */
#define GET_CALLER_FUNCTION() __func__

/* DATA FORMAT */
#define OLD_FORMAT 0x000001
#define EXT_FORMAT 0x000002
#define HANA_FORMAT 0x000004
#define NEW_FORMAT 0x000008

/* DATA TYPE */
#define MASTER 0x000010
#define TRADE 0x000020

/* MASTER TYPE */
#define STOCK 0x000040
#define FUTURE 0x000080
#define OPTION 0x000100
#define SPREAD 0x000200
#define WAREHOUSE 0x000400

/* TRADE TYPE */
#define STATUS 0x000800
#define QUOTE 0x001000
#define CANCEL 0x002000
#define SETTLE 0x004000
#define CLOSE 0x008000
#define OINT 0x010000
#define DEPTH 0x020000
#define FND 0x040000
#define MAVG 0x080000
#define OFFI 0x100000
#define WARE 0x200000
#define VOLM 0x400000

/* EXCHANGE */
#define LME 0x800000

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

/* open.c */
FEP *fep_open(const char *exnm, int flag);

/* config.c */
int fep_config(FEP *fep);

/* log.c */
void fep_log(FEP *fep, int level, const char *caller_function, const char *format, ...);
void null_to_space(char *msgb, int msgl);

/* file.c */
int create_directory(char *dirname);

/* time.c */
int fep_utc2kst(time_t utc_time, time_t *korean_time, struct tm *korean_tm);

/* shm.c */
unsigned long djb2(const char *str);
int fep_shminit(FEP *fep);

/* close.c */
void fep_close(FEP *fep);

#endif