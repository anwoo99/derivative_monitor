#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <libgen.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <libgen.h>
#include <locale.h>
#include "quot.h"

int doaction(int key);

pthread_mutex_t real_lock;

char i_scrn[8], s_scrn[8];
char i_exch[8], s_exch[8];
char i_symb[64], s_symb[64];
char s_path[128];
char g_scrn[8] = "000";
char g_symb[64];

int enable_pgup = 0;
int enable_pgdn = 0;
int enable_next = 0;
int enable_prev = 0;
int forwarding = 0;
int backwarding = 0;
int delayed = 0;
int onhelp = 0;
int display10 = 0;
int kst4time = 0;

struct field form4hd[] = {
    {FT_OUTPUT, "", 1, 1, 0, "", FA_BOLD | FC_YELLOW, -1, 0},
    {FT_OUTPUT, "", 2, 1, 0, "Choice ", FA_BOLD | FC_YELLOW, -1, 0},
    {FT_INPUT, "iSCRN", 2, 9, 3, "", FA_BOLD | FC_YELLOW, -1, 0},
    {FT_INPUT, "iEXCH", 2, 13, 4, "", FA_BOLD | FC_YELLOW, -1, 0},
    {FT_INPUT, "iSYMB", 2, 18, 32, "", FA_BOLD | FC_YELLOW, -1, 0},
    {FT_OUTPUT, "oNAME", 2, 52, 128, "", FA_BOLD | FC_YELLOW, -1, FLD_IS_LEFT},
    {-1, "", 0, 0, 0, "", 0, -1, 0}};

int main(int argc, char *argv[])
{
    const XCHG *xchg;
    int row, col, flg;
    int key, scrn, dirf = 0;
    char tmpb[40];
    int ii;

    if (argc > 1)
        delayed = 1;
        
    pthread_mutex_init(&real_lock, NULL);
    setlocale(LC_ALL, MyLOCALE);
    openterm();
    cursor(0);
    rqinit();

    mapopen((char *)g_scrn);
    str2fld("iSCRN", g_scrn);
    xchg = fep_exchanges();
    if (xchg != NULL)
    {
        for (ii = 0; xchg[ii].exid > 0; ii++)
        {
            if (xchg[ii].type != FOREX)
                break;
        }
        if (xchg[ii].exid > 0)
        {
            str2fld("iEXCH", (char *)xchg[ii].exnm);
            doaction(K_ENTER);
        }
    }

    while (1)
    {
        key = getkey(&row, &col, 0);
        switch (key)
        {
        case K_HOME:
            cur2fld("iSCRN");
            break;
        case K_PGUP:
            if (!backwarding || !enable_pgup)
                break;
            doaction(key);
            break;
        case K_PGDN:
            if (!forwarding || !enable_pgdn)
                break;
            doaction(key);
            break;
        case K_DN:
            if (enable_next)
                doaction(key);
            break;
        case K_UP:
            if (enable_prev)
                doaction(key);
            break;
        case K_ENTER:
            doaction(key);
            break;
        case K_LEFT:
        case K_RIGHT:
            getfld("iSCRN", i_scrn, 1);
            scrn = atoi(i_scrn);
            switch (key)
            {
            case K_LEFT:
                dirf = -1;
                break;
            case K_RIGHT:
                dirf = 1;
                break;
            }
            clrguide();
            scrn += dirf;
            if (!issvc(scrn))
                break;

            sprintf(tmpb, "%03d", scrn);
            str2fld("iSCRN", tmpb);
            flg = doaction(K_ENTER);
            break;
        case K_FOCUS:
        case K_MOUSE_R:
        case K_MOUSE_C:
            scrn = atoi(s_scrn);
            if (scrn >= 200)
            {
                if (key != K_MOUSE_R)
                    break;
                str2fld("iSCRN", g_scrn);
                str2fld("iSYMB", g_symb);
                doaction(K_ENTER);
                break;
            }

            if (row != 1)
            {
                ii = row - 5;
                if (realque.many <= ii)
                    break;
                xchg = fep_exchange_by_exid(realque.q[ii].exid);
                if (xchg == NULL)
                    break;
                str2fld("iEXCH", (char *)xchg->exnm);
                str2fld("iSYMB", realque.q[ii].symb);
                strcpy(g_symb, realque.q[ii].symb); //
                if (key == K_MOUSE_R)
                    str2fld("iSCRN", "201");
                else
                    str2fld("iSCRN", "200");
                doaction(K_ENTER);
                break;
            }
            break;
        case 0x18: // ^X
            if (display10)
                display10 = 0;
            else
                display10 = 1;
            doaction(K_ENTER);
            break;
        case 0x14: // ^T
            if (kst4time)
                kst4time = 0;
            else
                kst4time = 1;
            doaction(K_ENTER);
            break;
        default:
            break;
        }
    }
}

//
// doaction()
//
int doaction(int key)
{
    XCHG xchg;

    pthread_mutex_lock(&real_lock);
    clrguide();

    getfld("iSCRN", i_scrn, 1);
    getfld("iEXCH", i_exch, 1);
    getfld("iSYMB", i_symb, 1);
    if (strcmp(i_scrn, s_scrn) != 0)
    {
        enable_next = 0;
        enable_prev = 0;
        enable_pgup = 0;
        enable_pgdn = 0;

        strcpy(s_scrn, i_scrn);
        if (mapopen(i_scrn) != 0)
        {
            setguide("No such screen numner !!!!!");
            pthread_mutex_unlock(&real_lock);
            return (0);
        }
    }
    strcpy(s_exch, i_exch);
    strcpy(s_symb, i_symb);
    strcpy(s_scrn, i_scrn);
    if (fep_exchange(s_exch, &xchg) != 0 || xchg.type == FOREX)
    {
        setguide("Invalid Exchange !!!!");
        pthread_mutex_unlock(&real_lock);
        return (0);
    }
    if (atoi(s_scrn) < 200)
    {
        strcpy(g_scrn, s_scrn);
        strcpy(g_symb, s_symb);
    }
    query(key);
    pthread_mutex_unlock(&real_lock);
    return (0);
}

//
// name4screen()
// Edit screen title
//
int header4scrn(char *scrn, char *name)
{
    char title[128];
    int col;

    sprintf(title, " [%s] %s ", scrn, name);

    col = (MAX_COL / 2) - (strlen(title) / 2);

    if (onhelp)
    {
        putfld(&form4hd[6], 1);
        setattr(FC_YELLOW | FA_BOLD);
        str2scr(1, col, title);
        return (0);
    }

    form4hd[0].col = col;
    strcpy(form4hd[0].msg, title);
    putfld(form4hd, 1);
    endfld();

    str2fld("iSCRN", i_scrn);
    str2fld("iEXCH", i_exch);
    str2fld("iSYMB", i_symb);

    return (0);
}

//
// setguid()
// Display guide message
//
void setguide(char *msg)
{
    char errmsg[128];

    // clrguide();
    // setattr(FC_RED|FA_BOLD);
    // str2scr(MAX_ROW, 1, msg);
    sprintf(errmsg, "@@@ %s", msg);
    pushfld("oNAME", errmsg, FC_RED | FA_BOLD | FA_BLINK, 0);
}

void clrguide()
{
    // clreol(MAX_ROW, 1);
    pushfld("oNAME", " ", FC_YELLOW | FA_BOLD, 0);
}
