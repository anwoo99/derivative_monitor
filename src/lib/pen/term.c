#include "pen.h"
#include <term.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <curses.h>

#define flush(x) write(1, x, strlen(x));

static void setfld(int row, int col, int len, int seq, int att, int type);
static int cmpmap();

static int cursaddr = 0;  // cursor address
static int attribute = 0; // current attribute
static char escbuf[128];
static int field_seq = 1;

int MAX_ROW;
int MAX_COL;
int MAX_SIZ;

unsigned char *scrn_char;
unsigned short *scrn_xref;
unsigned short *scrn_xpos;
unsigned char *scrn_type;
int no_of_input = 0;
int focused = -1;

static struct termios termio4o;
static struct termios termio4n;

static struct fldmap
{
    char name[32];
    unsigned char type;
    unsigned char row;
    unsigned char col;
    unsigned char len;
    unsigned int seq;
    unsigned int att;
    unsigned int chk;
    time_t when;
} *s_fldmap;

static int m_fldmap;
static int n_fldmap;

static pthread_t rth;
static pthread_mutex_t lock;

static void *cleaner();
static void redraw(int);

//
// openterm()
//
int openterm()
{
    tcgetattr(0, &termio4o);
    tcgetattr(0, &termio4n);
    termio4n.c_lflag &= ~ECHO;
    termio4n.c_lflag &= ~ICANON;
    termio4n.c_cc[VMIN] = 1;
    tcsetattr(0, TCSANOW, &termio4n);

    initscr();
    MAX_ROW = lines;
    MAX_COL = columns;
    MAX_SIZ = lines * columns;

    m_fldmap = DEF_ROW * 16;
    n_fldmap = 0;

    scrn_char = (unsigned char *)malloc(DEF_SIZ);
    scrn_xref = (unsigned short *)malloc(DEF_SIZ * sizeof(short));
    scrn_xpos = (unsigned short *)malloc(DEF_SIZ * sizeof(short));
    scrn_type = (unsigned char *)malloc(DEF_SIZ);

    s_fldmap = (struct fldmap *)malloc(sizeof(struct fldmap) * m_fldmap);

    setattr(0);
    clrscrn(1);
    write(1, "\033[?9h", 5); // mouse tracking
    signal(SIGINT, closeterm);
    signal(SIGWINCH, redraw);
    cursor(0);

    pthread_mutex_init(&lock, NULL);
    return (0);
}

void closeterm(int argn)
{
    cursor(1);
    setattr(0);
    write(1, "\033[?9l", 5);
    clrscrn(1);
    tcsetattr(0, TCSANOW, &termio4o);
    exit(0);
}

//
// initpush()
//
int initpush()
{
    pthread_create(&rth, NULL, cleaner, NULL);
    return (0);
}

//
// pushfld()
// Push field data
//
void pushfld(char *name, char *data, int color, int real)
{
    struct fldmap *fldmap, map;
    char msgb[256], form[20];
    int attr, pos;
    int ii;

    memset(&map, 0, sizeof(map));
    strcpy((char *)map.name, name);
    fldmap = (struct fldmap *)bsearch(&map, s_fldmap, n_fldmap, sizeof(struct fldmap), cmpmap);
    if (fldmap == NULL)
        return;

    pthread_mutex_lock(&lock);
    fldmap->when = 0;
    if (fldmap->chk & FLD_IS_LEFT)
        sprintf(form, "%%-%d.%ds", fldmap->len, fldmap->len);
    else
        sprintf(form, "%%+%d.%ds", fldmap->len, fldmap->len);
    attr = fldmap->att & ~(FC_COLOR | FA_BLINK);
    attr |= color;
    fldmap->att = attr;
    sprintf(msgb, form, data);
    msgb[fldmap->len] = '\0';
    if (real)
    {
        pos = POS(fldmap->row, fldmap->col);
        for (ii = 0; ii < fldmap->len; ii++)
        {
            if (scrn_char[pos + ii] != msgb[ii])
                break;
        }
        if (ii >= fldmap->len)
        {
            pthread_mutex_unlock(&lock);
            return;
        }

        attr |= FA_REVERSE;
        fldmap->att = attr;
        fldmap->when = time(0);
    }
    else
        fldmap->when = 0;
    setattr(attr);
    str2scr(fldmap->row, fldmap->col, msgb);
    pthread_mutex_unlock(&lock);
}

//
// cleaner()
//
static void *cleaner()
{
    char fldb[256];
    time_t clock;
    int ii;

    while (1)
    {
        sleep(1);
        clock = time(0);
        for (ii = 0; ii < n_fldmap; ii++)
        {
            if (!(s_fldmap[ii].att & FA_REVERSE) || (clock - s_fldmap[ii].when) < 1)
                continue;
            pthread_mutex_lock(&lock);
            s_fldmap[ii].att &= ~FA_REVERSE;
            s_fldmap[ii].when = 0;
            getfld(s_fldmap[ii].name, fldb, 0);
            setattr(s_fldmap[ii].att);
            str2scr(s_fldmap[ii].row, s_fldmap[ii].col, fldb);
            pthread_mutex_unlock(&lock);
        }
    }
    pthread_exit(NULL);
}

void putfld(struct field *field, int force)
{
    int row, col;
    char msgb[256];
    int msgl;
    int ii;

    pthread_mutex_lock(&lock);
    if (force)
    {
        n_fldmap = 0;
        field_seq = 1;
        focused = -1;
        setattr(0);
        clrscrn(1);
    }
    for (ii = 0; field[ii].type >= 0; ii++)
    {
        row = field[ii].row;
        col = field[ii].col;
        if (row <= 0 || col <= 0) // dummy field
            continue;
        if (strlen(field[ii].msg))
        {
            strcpy(msgb, field[ii].msg);
            msgl = strlen(msgb);
            if ((col + msgl) > DEF_COL)
                msgl = DEF_COL - col + 1;
            msgb[msgl] = '\0';
            setattr(field[ii].att);
            str2scr(row, col, msgb);
            col += strlen(field[ii].msg);
        }

        if (strlen(field[ii].name) <= 0 || field[ii].len <= 0)
            continue;
        if (n_fldmap >= m_fldmap)
            continue;

        s_fldmap[n_fldmap].type = field[ii].type;
        if (field[ii].seq >= 0)
            sprintf(s_fldmap[n_fldmap].name, "%s%03d", field[ii].name, field[ii].seq);
        else
            strcpy(s_fldmap[n_fldmap].name, field[ii].name);
        s_fldmap[n_fldmap].row = field[ii].row;
        s_fldmap[n_fldmap].col = field[ii].col;
        s_fldmap[n_fldmap].len = field[ii].len;
        s_fldmap[n_fldmap].att = field[ii].att;
        s_fldmap[n_fldmap].chk = field[ii].chk;
        s_fldmap[n_fldmap].when = 0;
        switch (s_fldmap[n_fldmap].type)
        {
        case FT_INPUT:
        case FT_INOUT:
            s_fldmap[n_fldmap].att |= FA_UNDERLINE;
            break;
        default:
            break;
        }
        s_fldmap[n_fldmap].seq = field_seq++;
        setfld(row, col, s_fldmap[n_fldmap].len, s_fldmap[n_fldmap].seq, s_fldmap[n_fldmap].att, s_fldmap[n_fldmap].type);
        n_fldmap++;
    }
    pthread_mutex_unlock(&lock);
}
//
// endfld()
//
int endfld()
{
    int ii;

    pthread_mutex_lock(&lock);
    qsort(s_fldmap, n_fldmap, sizeof(struct fldmap), cmpmap);
    pthread_mutex_unlock(&lock);
    for (ii = 0, no_of_input = 0; ii < n_fldmap; ii++)
    {
        switch (s_fldmap[ii].type)
        {
        case FT_INPUT:
        case FT_INOUT:
            no_of_input++;
            break;
        default:
            break;
        }
    }
    return (n_fldmap);
}

//
// redraw()
// Redraw screen by window size change
//
void redraw(int sig)
{
    struct winsize ws;
    int row, col, pos;

    if (ioctl(0, TIOCGWINSZ, &ws) != 0)
        return;

    pthread_mutex_lock(&lock);
    MAX_ROW = ws.ws_row;
    MAX_COL = ws.ws_col;
    if (MAX_ROW > DEF_ROW)
        MAX_ROW = DEF_ROW;
    if (MAX_COL > DEF_COL)
        MAX_COL = DEF_COL;
    MAX_SIZ = MAX_ROW * MAX_COL;

    sprintf(escbuf, "\033[H\033[2J");
    flush(escbuf);
    for (row = 1; row <= MAX_ROW; row++)
    {
        for (col = 1, pos = POS(row, col); col <= MAX_COL; col++, pos++)
        {
            setattr(scrn_xref[pos]);
            setpos(row, col);
            write(1, &scrn_char[pos], 1);
        }
    }
    pthread_mutex_unlock(&lock);
}

//
// cur2fld()
//
void cur2fld(char *name)
{
    struct fldmap *fldmap, map;

    memset(&map, 0, sizeof(map));
    strcpy(map.name, name);
    fldmap = (struct fldmap *)bsearch(&map, s_fldmap, n_fldmap, sizeof(struct fldmap), cmpmap);
    if (fldmap == NULL)
        return;

    setfocus((int)fldmap->row, (int)fldmap->col);
}
//
// setfld()
// Set field information to scrn_ buffer
//
static void setfld(int row, int col, int len, int seqn, int attr, int type)
{
    char fldb[256];
    int addr;
    int ii;

    memset(fldb, ' ', len);
    fldb[len] = '\0';

    setattr(attr);
    str2scr(row, col, fldb);

    addr = POS(row, col);
    for (ii = 0; ii < len && col <= DEF_COL; ii++, addr++, col++)
    {
        scrn_xpos[addr] = seqn;
        scrn_type[addr] = type;
    }
}

//
// str2fld()
// Dispaly message to a specifield field by name
//
void str2fld(char *name, char *str)
{
    struct fldmap *fldmap, map;
    char msgb[256], form[20];

    memset(&map, 0, sizeof(map));
    strcpy(map.name, name);
    fldmap = (struct fldmap *)bsearch(&map, s_fldmap, n_fldmap, sizeof(struct fldmap), cmpmap);
    if (fldmap == NULL)
        return;

    pthread_mutex_lock(&lock);
    fldmap->when = 0;
    sprintf(form, "%%-%d.%ds", fldmap->len, fldmap->len);
    sprintf(msgb, form, str);
    msgb[fldmap->len] = '\0';
    setattr(fldmap->att);
    str2scr(fldmap->row, fldmap->col, msgb);
    pthread_mutex_unlock(&lock);
}
//
// int2fld()
// Display data value to a specified field by name
//
void int2fld(char *name, int val)
{
    struct fldmap *fldmap, map;
    char msgb[256];
    char form[20];

    memset(&map, 0, sizeof(map));
    strcpy(map.name, name);
    fldmap = (struct fldmap *)bsearch(&map, s_fldmap, n_fldmap, sizeof(struct fldmap), cmpmap);
    if (fldmap == NULL)
        return;
    pthread_mutex_lock(&lock);
    fldmap->when = 0;
    sprintf(form, "%%%dd", fldmap->len);
    sprintf(msgb, form, val);
    msgb[fldmap->len] = '\0';
    setattr(fldmap->att);
    str2scr(fldmap->row, fldmap->col, msgb);
    pthread_mutex_unlock(&lock);
}

void uint2fld(char *name, unsigned int val)
{
    struct fldmap *fldmap, map;
    char msgb[256];
    char form[20];

    memset(&map, 0, sizeof(map));
    strcpy(map.name, name);
    fldmap = (struct fldmap *)bsearch(&map, s_fldmap, n_fldmap, sizeof(struct fldmap), cmpmap);
    if (fldmap == NULL)
        return;
    pthread_mutex_lock(&lock);
    fldmap->when = 0;
    sprintf(form, "%%%du", fldmap->len);
    sprintf(msgb, form, val);
    msgb[fldmap->len] = '\0';
    setattr(fldmap->att);
    str2scr(fldmap->row, fldmap->col, msgb);
    pthread_mutex_unlock(&lock);
}

void ymd2fld(char *name, int ymd)
{
    char buff[80];

    sprintf(buff, "%02d/%02d/%02d", _YEAR(ymd) % 100, _MONTH(ymd), _MDAY(ymd));
    str2fld(name, buff);
}

void hms2fld(char *name, int hms)
{
    char buff[80];

    sprintf(buff, "%02d:%02d:%02d", _HOUR(hms), _MINUTE(hms), _SECOND(hms));
    str2fld(name, buff);
}

//
// attr2fld()
// Change field attribute
//
void attr2fld(char *name, int att)
{
    struct fldmap *fldmap, map;

    memset(&map, 0, sizeof(map));
    strcpy(map.name, name);
    fldmap = (struct fldmap *)bsearch(&map, s_fldmap, n_fldmap, sizeof(struct fldmap), cmpmap);
    if (fldmap == NULL)
        return;
    fldmap->att = att;
}
//
// clear current field
//
void clrfld(char *name)
{
    struct fldmap *fldmap, map;
    char fldb[256];
    int addr;
    int ii;

    memset(&map, 0, sizeof(map));
    strcpy(map.name, name);
    fldmap = (struct fldmap *)bsearch(&map, s_fldmap, n_fldmap, sizeof(struct fldmap), cmpmap);
    if (fldmap == NULL)
        return;
    addr = POS(fldmap->row, fldmap->col);
    for (ii = 0; COL(addr) <= DEF_COL && ii < DEF_SIZ; addr++, ii++)
    {
        if (scrn_xpos[addr] != fldmap->seq)
            break;
        fldb[ii] = ' ';
    }
    fldb[ii] = '\0';
    addr = POS(fldmap->row, fldmap->col);

    setattr(scrn_xref[addr]);
    str2scr(ROW(addr), COL(addr), fldb);
    setpos(ROW(addr), COL(addr));
}

//
// getfld()
// Read specified field
//
int getfld(char *name, char *buff, int trim)
{
    struct fldmap *fldmap, map;
    int addr, ii, jj;

    buff[0] = '\0';
    memset(&map, 0, sizeof(map));
    strcpy(map.name, name);
    fldmap = (struct fldmap *)bsearch(&map, s_fldmap, n_fldmap, sizeof(struct fldmap), cmpmap);
    if (fldmap == NULL)
        return (0);
    addr = POS(fldmap->row, fldmap->col);
    for (ii = 0; COL(addr) <= DEF_COL && addr < DEF_SIZ; addr++)
    {
        if (scrn_xpos[addr] != fldmap->seq)
            break;
        if (trim && ii == 0 && scrn_char[addr] == ' ')
            continue;
        buff[ii++] = scrn_char[addr];
    }
    buff[ii] = '\0';
    if (trim)
    {
        for (jj = ii - 1; jj >= 0; jj--)
        {
            if (buff[jj] != ' ')
                break;
            buff[jj] = '\0';
        }
    }

    return (strlen(buff));
}

//
// curfld()
// Return current field number with field name
//
int curfld(char *name, int row, int col)
{
    int addr, seq;
    int ii;

    name[0] = '\0';
    if (row != -1 && col != -1)
        addr = POS(row, col);
    else
        addr = cursaddr;

    if (addr < 0 || addr >= DEF_SIZ)
        return (-1);
    if ((seq = scrn_xpos[addr]) <= 0)
        return (-1);
    for (ii = 0; ii < n_fldmap; ii++)
    {
        if (s_fldmap[ii].seq == seq)
        {
            strcpy(name, s_fldmap[ii].name);
            return (seq);
        }
    }
    return (-1);
}

//
// fldattr()
// Return fld attribute
//
int fldattr(char *name)
{
    struct fldmap *fldmap, map;

    memset(&map, 0, sizeof(map));
    strcpy(map.name, name);
    fldmap = (struct fldmap *)bsearch(&map, s_fldmap, n_fldmap, sizeof(struct fldmap), cmpmap);
    if (fldmap == NULL)
        return (0);
    return (fldmap->att);
}

//
// clrscrn()
// Clear screen
//
void clrscrn(int line)
{
    int size;

    switch (line)
    {
    case 0:
        sprintf(escbuf, "\033[H\033[2J");
        memset(scrn_char, ' ', DEF_SIZ);
        memset(scrn_xref, 0, DEF_SIZ * sizeof(scrn_xref[0]));
        memset(scrn_xpos, 0, DEF_SIZ * sizeof(scrn_xpos[0]));
        memset(scrn_type, 0, DEF_SIZ);
        cursaddr = 0;
        break;
    default: // clear screen presentation for quotes
        sprintf(escbuf, "\033[%d;0H\033[J", line);
        cursaddr = POS(line, 1);
        size = DEF_SIZ - cursaddr;
        memset(&scrn_char[cursaddr], ' ', size);
        memset(&scrn_type[cursaddr], 0, DEF_SIZ);
        memset(&scrn_xref[cursaddr], 0, size * sizeof(scrn_xref[0]));
        memset(&scrn_xpos[cursaddr], 0, size * sizeof(scrn_xpos[0]));
        break;
    }
    flush(escbuf);
}

//
// clreos()
// Clear to end of screen
//
void clreos(int row)
{
    int ii;

    for (ii = row; ii <= MAX_ROW; ii++)
        clreol(ii, 1);
    cursaddr = -1;
}

//
// clreol()
// Clear to END-OF-LINE
//
void clreol(int row, int col)
{
    int size, addr;

    setpos(row, col);
    sprintf(escbuf, "\033[K");
    flush(escbuf);

    size = DEF_COL - col - 1;
    addr = POS(row, col);
    memset(&scrn_char[addr], ' ', size);
    memset(&scrn_xref[addr], 0, size * sizeof(scrn_xref[0]));
}

//
// insline()
// insert lines
void insline(int r, int n)
{
    char escb[16];
    int fpos, tpos, size;
    int ii;

    setpos(r, 1);
    sprintf(escb, "\033[L");
    fpos = POS(r, 1);
    tpos = POS(r + 1, 1);
    size = DEF_ROW - r - 1;
    for (ii = 0; ii < n; ii++)
    {
        write(1, escb, strlen(escb));
        if (size != 0)
        {
            memmove(&scrn_char[tpos], &scrn_char[fpos], size);
            memmove(&scrn_xref[tpos], &scrn_xref[fpos], size * sizeof(short));
            memmove(&scrn_xpos[tpos], &scrn_xpos[fpos], size * sizeof(short));
        }
        memset(&scrn_char[fpos], 0, DEF_COL);
        memset(&scrn_xref[fpos], 0, DEF_COL * sizeof(short));
        memset(&scrn_xpos[fpos], 0, DEF_COL * sizeof(short));
    }
}

//
// cha2str()
// Set cursor and display a charster
//
void cha2scr(int row, int col, unsigned char cha)
{
    char str[10];

    if (col > MAX_COL)
        return;
    setpos(row, col);
    if (scrn_char[cursaddr] != cha || scrn_xref[cursaddr] != attribute)
    {
        scrn_char[cursaddr] = cha;
        scrn_xref[cursaddr] = attribute;
        if (row <= MAX_ROW && col <= MAX_COL)
        {
            str[0] = cha;
            str[1] = '\0';
            flush(str);
            cursaddr++;
        }
    }
    if (cursaddr >= MAX_SIZ || COL(cursaddr) == 1)
        cursaddr = -1;
}

//
// str2scr()
// Set cursor & display string
//
void str2scr(int row, int col, char *str)
{
    char msgb[512];
    int msgl, cpos;
    int ii;

    cpos = POS(row, col);
    strcpy(msgb, str);
    msgl = strlen(msgb);
    for (ii = 0; ii < msgl; ii++)
    {
        if (scrn_char[cpos + ii] != str[ii] || scrn_xref[cpos + ii] != attribute)
            break;
    }
    if (ii >= msgl)
        return;

    memcpy(&scrn_char[cpos], msgb, msgl);
    for (ii = 0; ii < msgl; ii++)
        scrn_xref[cpos + ii] = attribute;

    if (col > MAX_COL || row > MAX_ROW)
        return;
    if ((col + msgl) > MAX_COL)
        msgl = MAX_COL - col + 1;
    msgb[msgl] = '\0';
    setpos(row, col);
    flush(msgb);
    cursaddr += msgl;
    if (cursaddr >= MAX_SIZ || COL(cursaddr) == 1 || COL(cursaddr) >= MAX_COL)
        cursaddr = -1;
}

//
// setpos()
// Set cursor position
void setpos(int row, int col)
{
    int r, c;

    if (POS(row, col) == cursaddr && COL(cursaddr) != 1)
        return;

    r = ROW(cursaddr);
    c = COL(cursaddr);

    if (cursaddr < 0)
    {
        r = 0;
        c = 0;
    }

    if (row <= MAX_ROW && col <= MAX_COL)
    {
        sprintf(escbuf, "\033[%d;%dH", row, col);
        flush(escbuf);
    }
    cursaddr = POS(row, col);
}

//
// setfoucs()
// Set dummy inout cursor
void setfocus(int row, int col)
{
    int c;

    if (focused >= 0 && focused < MAX_SIZ)
    {
        if (focused == POS(row, col) && scrn_xref[focused] & FA_REVERSE)
            return;

        pthread_mutex_lock(&lock);
        setattr(scrn_xref[focused] & ~FA_REVERSE);
        c = scrn_char[focused];
        cha2scr(ROW(focused), COL(focused), c);
        focused = -1;
        pthread_mutex_unlock(&lock);
    }

    if (!no_of_input || row < 0 || col < 0)
        return;
    pthread_mutex_lock(&lock);
    focused = POS(row, col);
    setattr(scrn_xref[focused] | FA_REVERSE);
    c = scrn_char[focused];
    cha2scr(ROW(focused), COL(focused), c);
    pthread_mutex_unlock(&lock);
}

//
// visible/invisible cursor
//
void cursor(int onoff)
{
    static int visible = 1;

    switch (onoff)
    {
    case 0:
        if (!visible)
            break;
        visible = 0;
        sprintf(escbuf, "\033[?25l");
        flush(escbuf);
        break;
    default:
        if (visible)
            break;
        visible = 1;
        sprintf(escbuf, "\033[?25h");
        flush(escbuf);
        break;
    }
}
//
// savepos()
void savepos()
{
    sprintf(escbuf, "\0337");
    flush(escbuf);
}

//
// restorepos()
//
void restorepos()
{
    sprintf(escbuf, "\0338");
    flush(escbuf);
}

//
// setattr
// Set attribute
//
void setattr(attr)
{
    int setcha[32];
    int esclen, setlen;
    int diff, ii;

#if 1
    attr &= 0x0fff;
#endif
    if (attr == attribute)
        return;

    setlen = 0;
    if (attr & FA_BOLD)
        setcha[setlen++] = 1;
    if (attr & FA_UNDERLINE)
        setcha[setlen++] = 4;
    if (attr & FA_REVERSE)
        setcha[setlen++] = 7;
    if (attr & FA_BLINK)
        setcha[setlen++] = 5;
    switch (COLOR_F(attr))
    {
    case C_BLACK:
        setcha[setlen++] = 30;
        break;
    case C_RED:
        setcha[setlen++] = 31;
        break;
    case C_GREEN:
        setcha[setlen++] = 32;
        break;
    case C_YELLOW:
        setcha[setlen++] = 33;
        break;
    case C_BLUE:
        setcha[setlen++] = 34;
        break;
    case C_MAGENTA:
        setcha[setlen++] = 35;
        break;
    case C_CYAN:
        setcha[setlen++] = 36;
        break;
    case C_WHITE:
        setcha[setlen++] = 37;
        break;
    default:
        setcha[setlen++] = 37;
        break;
    }

    sprintf(escbuf, "\033[0");
    esclen = strlen(escbuf);
    for (ii = 0; ii < setlen; ii++)
    {
        sprintf(&escbuf[esclen], ";%d", setcha[ii]);
        esclen = strlen(escbuf);
    }
    sprintf(&escbuf[esclen], "m");
    esclen = strlen(escbuf);
    diff = attr ^ attribute;
    flush(escbuf);

    attribute = attr;
}

static int cmpmap(struct fldmap *m1, struct fldmap *m2)
{
    return (strcmp(m1->name, m2->name));
}
