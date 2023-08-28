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
char i_host[8], s_host[8];
char s_path[128];
char g_scrn[8] = "000";
char g_symb[64];
char g_host[8];

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
    {FT_INPUT, "iHOST", 2, 18, 10, "", FA_BOLD | FC_YELLOW, -1, 0},
    {FT_INPUT, "iSYMB", 2, 29, 32, "", FA_BOLD | FC_YELLOW, -1, 0},
    {FT_OUTPUT, "oNAME", 2, 52, 128, "", FA_BOLD | FC_YELLOW, -1, FLD_IS_LEFT},
    {-1, "", 0, 0, 0, "", 0, -1, 0}};

int main(int argc, char *argv[])
{
    int row, col, flg;
    int key, scrn, dirf = 0;
    char tmpb[40];
    int ii;
    FEP *fep;

    if (argc > 1)
        delayed = 1;

    pthread_mutex_init(&real_lock, NULL);
    setlocale(LC_ALL, MyLOCALE);
    openterm();
    cursor(0);
    rqinit();

    mapopen((char *)g_scrn);
    str2fld("iSCRN", g_scrn);

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
                str2fld("iHOST", g_host);
                str2fld("iSYMB", g_symb);
                doaction(K_ENTER);
                break;
            }

            if (row != 1)
            {
                ii = row - 5;

                if (realque.many <= ii)
                    break;

                fep = fep_open(realque.q[ii].exnm, MD_RDONLY);

                if (fep == NULL)
                    break;

                str2fld("iEXCH", (char *)fep->exnm);
                str2fld("iHOST", realque.q[ii].hostname);
                str2fld("iSYMB", realque.q[ii].symb);
                strcpy(g_symb, realque.q[ii].symb);

                if (key == K_MOUSE_R)
                    str2fld("iSCRN", "201");
                else
                    str2fld("iSCRN", "200");

                fep_close(fep);
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
    FEP *fep;

    pthread_mutex_lock(&real_lock);
    clrguide();

    getfld("iSCRN", i_scrn, 1);
    getfld("iEXCH", i_exch, 1);
    getfld("iHOST", i_host, 1);
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
    strcpy(s_host, i_host);
    strcpy(s_symb, i_symb);
    strcpy(s_scrn, i_scrn);

    fep = fep_open(s_exch, MD_RDONLY);

    if (fep == NULL)
    {
        setguide("Invalid Exchange !!!!");
        pthread_mutex_unlock(&real_lock);
        return (0);
    }

    if (atoi(s_scrn) < 200)
    {
        strcpy(g_scrn, s_scrn);
        strcpy(g_host, s_host);
        strcpy(g_symb, s_symb);
    }

    fep_close(fep);

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
    str2fld("iHOST", i_host);
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

    sprintf(errmsg, "@@@ %s", msg);
    pushfld("oNAME", errmsg, FC_RED | FA_BOLD | FA_BLINK, 0);
}

void clrguide()
{
    pushfld("oNAME", " ", FC_YELLOW | FA_BOLD, 0);
}
