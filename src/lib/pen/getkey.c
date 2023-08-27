#include "pen.h"
#include <term.h>

extern unsigned char *scrn_char;
extern unsigned char *scrn_type;
extern unsigned short *scrn_xpos;
extern int no_of_input; // no of input fields
extern int focused;     // dummy cursor (on input)

static struct funckey
{
    char esch[16];
    int keyc;
} funckey[] = {
    {"[A", K_UP}, {"[B", K_DN}, {"[C", K_RIGHT}, {"[D", K_LEFT}, {"[1~", K_HOME}, {"[1~", K_INS}, {"[2~", K_DEL}, {"OF", K_END}, {"[5~", K_PGUP}, {"[6~", K_PGDN}, {"[M", K_FOCUS}, {"[20~", K_F9}, {"", -1}};

// getkey()
// Read input key code
//
int getkey(int *f_row, int *f_col, int timeout)
{
    fd_set clist;
    struct timeval timeval;
    int nch, key;
    char cha[16], escb[16];
    int escchk, escl;
    int row, col, button = 0;
    int seqn;
    int ii, jj;

    escchk = 0;
    escl = 0;

    row = 0;
    col = 0;
    if (focused >= 0 && focused < MAX_SIZ &&
        (scrn_type[focused] == FT_INPUT || scrn_type[focused] == FT_INOUT))
    {
        row = ROW(focused);
        col = COL(focused);
    }
    else
    {
        for (ii = 0; ii < MAX_SIZ; ii++)
        {
            if (scrn_type[ii] == FT_INPUT || scrn_type[ii] == FT_INOUT)
            {
                row = ROW(ii);
                col = COL(ii);
                break;
            }
        }
    }
    setfocus(row, col);

    while (1)
    {
        FD_ZERO(&clist);
        FD_SET(0, &clist);
        timeval.tv_sec = timeout;
        timeval.tv_usec = 0;
        if (timeout <= 0)
        {
            if (select(1, &clist, NULL, NULL, NULL) <= 0)
                return (0);
        }
        else
        {
            if (select(1, &clist, NULL, NULL, &timeval) <= 0)
                return (0);
        }

        if ((nch = read(0, cha, 1)) <= 0)
            continue;
        //
        // Please keymap to xterm keyboard emulation
        //
        key = 0;
        switch (escchk)
        {
        case 0:
            switch (cha[0])
            {
            case 0x1b: // function key
                escchk = 1;
                escl = 0;
                break;
            case 0x08: // backspace
            case 0x7f: // DEL
                if (!no_of_input || focused == -1)
                    break;

                for (ii = focused - 1; ii >= 0; ii--)
                {
                    if (scrn_xpos[focused] == scrn_xpos[ii])
                        break;
                }
                if (ii >= 0)
                    setfocus(ROW(ii), COL(ii));
                break;
            case 0x09: // tab
                if (!no_of_input)
                    break;
                seqn = scrn_xpos[focused];
                for (ii = 0, jj = focused + 1; ii < MAX_SIZ; ii++, jj++)
                {
                    if (jj >= MAX_SIZ)
                        jj = 0;
                    if (scrn_xpos[jj] == seqn)
                        continue;
                    seqn = -1;
                    switch (scrn_type[jj])
                    {
                    case FT_INPUT:
                    case FT_INOUT:
                        break;
                    default:
                        continue;
                    }

                    row = ROW(jj);
                    col = COL(jj);
                    setfocus(row, col);
                    break;
                }
                break;
            case 0x0a:
            case 0x0d:
                return (K_ENTER);
            case 0x14: // ^T
            case 0x18: // ^X
                return (cha[0]);
            default:
                if (!no_of_input || focused == -1)
                    break;

                if (cha[0] >= 'a' && cha[0] <= 'z')
                    cha[0] -= ' ';
                scrn_char[focused] = cha[0];
                for (ii = 0, jj = focused + 1; ii < MAX_SIZ; ii++, jj++)
                {
                    if (jj >= MAX_SIZ)
                        jj = 0;
                    switch (scrn_type[jj])
                    {
                    case FT_INPUT:
                    case FT_INOUT:
                        break;
                    default:
                        continue;
                    }
                    row = ROW(jj);
                    col = COL(jj);
                    setfocus(row, col);
                    break;
                }
                break;
            }
            break;
        case 1:
            escb[escl++] = cha[0];
            escb[escl] = '\0';
            key = 0;
            for (ii = 0; funckey[ii].keyc > 0; ii++)
            {
                if (strcmp(escb, funckey[ii].esch) == 0)
                {
                    key = funckey[ii].keyc;
                    break;
                }
            }
            if (escl >= 4 || key != 0)
                escchk = 0;

            switch (key)
            {
            case K_RIGHT:
            case K_LEFT:
            case K_UP:
            case K_DN:
            case K_DEL:
            case K_INS:
            case K_PGUP:
            case K_PGDN:
            case K_END:
            case K_F1:
            case K_F2:
            case K_F3:
            case K_F4:
            case K_F5:
            case K_F6:
            case K_F7:
            case K_F8:
            case K_F9:
            case K_F10:
            case K_F11:
            case K_F12:
            case K_F13:
            case K_F14:
            case K_F15:
            case K_F16:
            case K_F17:
            case K_F18:
            case K_F19:
            case K_F20:
                return (key);
            case K_FOCUS:
                escchk = 2;
                break;
            case K_HOME:
                if (!no_of_input)
                    break;
                for (ii = 0; ii < MAX_SIZ; ii++)
                {
                    if (scrn_xpos[ii] == 0)
                        continue;
                    if (scrn_type[ii] != FT_INPUT && scrn_type[ii] != FT_INOUT)
                        continue;
                    row = ROW(ii);
                    col = COL(ii);
                    setfocus(row, col);
                    break;
                }
                break;
            default:
                break;
            }
            break;
        case 2: // mouse tracking
            button = cha[0];
            escchk++;
            break;
        case 3: // mouse tracking
            col = cha[0] - ' ';
            escchk++;
            break;
        case 4: // mouse click
            escchk = 0;
            row = cha[0] - ' ';
            *f_row = row;
            *f_col = col;
            switch (button)
            {
            case 0x20:
                return (K_FOCUS);
            case 0x21:
                return (K_MOUSE_C);
            case 0x22:
                return (K_MOUSE_R);
            }
        }
    }
}
