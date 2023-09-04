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
#include <netinet/tcp.h>
#include <math.h>

#include "parson.h"
#include "config.h"
#include "schema.h"
#include "oldpkt.h"
#include "extpkt.h"

#define RTD_PORT 7971

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
#define CONFIG_UPDATE_MSG "AAADADAWLDMNALKSNDAKLNWDLKQANL"
#define KILL_ALL_MSG "ADASD123KLNDA00VKLKNK3ADC"
#define RUN_ALL_MSG "GSLGEKNGLSKDNSLBBBLAD1123"

/* Print Function's name for logging */
#define GET_CALLER_FUNCTION() __func__

/* DATA FORMAT */
#define OLD_FORMAT 0x0000001
#define EXT_FORMAT 0x0000002
#define HANA_FORMAT 0x0000004
#define NEW_FORMAT 0x0000008

/* DATA TYPE */
#define MASTER 0x0000010
#define TRADE 0x0000020

/* MASTER TYPE */
#define STOCK 0x0000040
#define FUTURE 0x0000080
#define OPTION 0x0000100
#define SPREAD 0x0000200
#define WAREHOUSE 0x0000400

/* TRADE TYPE */
#define STATUS 0x0000800
#define QUOTE 0x0001000
#define CANCEL 0x0002000
#define SETTLE 0x0004000
#define CLOSE 0x0008000
#define OINT 0x0010000
#define DEPTH 0x0020000
#define FND 0x0040000
#define MAVG 0x0080000
#define OFFI 0x0100000
#define WARE 0x0200000
#define VOLM 0x0400000

/* EXCHANGE */
#define LME 0x0800000
#define NONE 0x1000000

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
void quote_log(FEP *fep, char *hostname, int class_tag, const char *format, ...);

/* file.c */
int create_directory(char *dirname);

/* time.c */
int fep_utc2kst(time_t utc_time, time_t *korean_time, struct tm *korean_tm);
void fep_sleep(int microseconds);

/* shm.c */
unsigned long djb2(const char *str);
int fep_shminit(FEP *fep);

/* close.c */
void fep_close(FEP *fep);

/* folder.c */
FOLDER *getfolder(FEP *fep, const char *symb, const char *hostname);
FOLDER *newfolder(FEP *fep, const char *symb, const char *hostname);
void delfolder(FEP *fep, FOLDER *folder);

/* push.c type=0(QUOTE) 1(CANCEL) 2(DEPTH) */
int fep_push(FEP *fep, FOLDER *folder, int type);

#endif
