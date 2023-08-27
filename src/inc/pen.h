#ifndef _PEN_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <libgen.h>
#include <signal.h>
#include <fcntl.h>
#include <termio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <pthread.h>

#define MyLOCALE "en_US"

#define _YEAR(ymd) (ymd / 10000)
#define _MONTH(ymd) ((ymd % 10000) / 100)
#define _MDAY(ymd) (ymd % 100)
#define _HOUR(ymd) (ymd / 10000)
#define _MINUTE(ymd) ((ymd % 10000) / 100)
#define _SECOND(ymd) (ymd % 100)

#define DEF_ROW 80
#define DEF_COL 250
#define DEF_SIZ (DEF_ROW * DEF_COL)

extern int MAX_ROW;
extern int MAX_COL;
extern int MAX_SIZ;

struct field
{
    int type;
    char name[16];
    int row;
    int col;
    int len;
    char msg[80];
    int att;
    int seq;
    int chk;
};

#define FT_INPUT 1
#define FT_OUTPUT 2
#define FT_INOUT 3

// field.chk
#define FLD_IS_SYMB 0x01
#define FLD_IS_LEFT 0x02

#define FA_UNDERLINE 0x01
#define FA_BOLD 0x02
#define FA_REVERSE 0x04
#define FA_BLINK 0x08

#define F_COLOR(x) (x << 8)
#define B_COLOR(x) (x << 12)
#define COLOR_F(x) ((x >> 8) & 0x0f)
#define COLOR_B(x) ((x >> 12) & 0x0f)
#define IS_UNDERLINE(x) (x & FA_UNDERLINE)
#define IS_BOLD(x) (x & FA_BOLD)
#define IS_REVERSE(x) (x & FA_REVERSE)

#define C_BLACK 1
#define C_RED 2
#define C_GREEN 3
#define C_YELLOW 4
#define C_BLUE 5
#define C_MAGENTA 6
#define C_CYAN 7
#define C_WHITE 8

#define FC_COLOR 0x0f00
#define BC_COLOR 0xf000
#define FC_DEFAULT F_COLOR(0)
#define FC_BLACK F_COLOR(C_BLACK)
#define FC_RED F_COLOR(C_RED)
#define FC_GREEN F_COLOR(C_GREEN)
#define FC_YELLOW F_COLOR(C_YELLOW)
#define FC_BLUE F_COLOR(C_BLUE)
#define FC_MAGENTA F_COLOR(C_MAGENTA)
#define FC_CYAN F_COLOR(C_CYAN)
#define FC_WHITE F_COLOR(C_WHITE)

#define BC_DEFAULT B_COLOR(0)
#define BC_BLACK B_COLOR(C_BLACK)
#define BC_RED B_COLOR(C_RED)
#define BC_GREEN B_COLOR(C_GREEN)
#define BC_YELLOW B_COLOR(C_YELLOW)
#define BC_BLUE B_COLOR(C_BLUE)
#define BC_MAGENTA B_COLOR(C_MAGENTA)
#define BC_CYAN B_COLOR(C_CYAN)
#define BC_WHITE B_COLOR(C_WHITE)

#define K_ESC 0x00000
#define K_ENTER 0x10001
#define K_INS 0x10002
#define K_DEL 0x10003
#define K_BS 0x10004
#define K_TAB 0x10005
#define K_HOME 0x10010
#define K_END 0x10011
#define K_PGUP 0x10012
#define K_PGDN 0x10013
#define K_UP 0x10014
#define K_DN 0x10015
#define K_LEFT 0x00016
#define K_RIGHT 0x00017
#define K_FOCUS 0x10018
#define K_MOUSE_L K_FOCUS
#define K_MOUSE_C K_FOCUS + 1
#define K_MOUSE_R K_FOCUS + 2

#define K_F1 0x10021
#define K_F2 0x10022
#define K_F3 0x10023
#define K_F4 0x10024
#define K_F5 0x10025
#define K_F6 0x10026
#define K_F7 0x10027
#define K_F8 0x10028
#define K_F9 0x10029
#define K_F10 0x10030
#define K_F11 0x10031
#define K_F12 0x10032
#define K_F13 0x10033
#define K_F14 0x10034
#define K_F15 0x10035
#define K_F16 0x10036
#define K_F17 0x10037
#define K_F18 0x10038
#define K_F19 0x10039
#define K_F20 0x10040

#define POS(r, c) (((r - 1) * DEF_COL) + c - 1)
#define ROW(x) (x / DEF_COL) + 1
#define COL(x) (x % DEF_COL) + 1

int openterm();
void closeterm(int);

void putfld(struct field *, int);
int endfld();
void pushfld(char *, char *, int, int);
void str2fld(char *, char *);
void cur2fld(char *);
void int2fld(char *, int);
void uint2fld(char *, unsigned int);
void ymd2fld(char *, int);
void hms2fld(char *, int);
void clrfld(char *);
int getfld(char *, char *, int);
int curfld(char *, int, int);

void attr2fld(char *, int);
int fldattr(char *);

int initpush();
void clrscrn(int);
void clreos(int);
void clreol(int, int);
void insline(int, int);

void cha2str(int, int, unsigned char);
void str2scr(int, int, char *);
void setpos(int, int);
void savepos();
void restorepos();
void cursor(int);
void setattr(int);
void setfocus(int, int);

int getkey(int *, int *, int);

#endif
