//
// map.c
// Open map for quote
//
#include "quot.h"
#include "config.h"

struct field field[MAX_FLDS];
struct field txtfld[2];
int nfld = 0;
int insert_mode = 0;

//
// mapopen()
//
int mapopen(char *scrn)
{
    char xmlpath[128];
    struct xmltag xmltag[500];
    int n_conf, c_flow;
    char vargs[16][512];
    int vargv[16], nrow = 0, many, indx = 0;
    int loop;
    int ii, jj, kk;

    sprintf(xmlpath, "%s/fox/%s.cfg", ETC_DIR, scrn);
    if ((n_conf = getxmlcfg(xmlpath, xmltag)) <= 0)
        return (-1);

    nfld = 0;
    loop = 0;
    for (ii = 0, c_flow = 0; ii < n_conf; ii++)
    {
        switch (c_flow)
        {
        case 0:
            if (strcasecmp(xmltag[ii].tags, "scrn") != 0)
                continue;
            getargs(&xmltag[ii], "no", vargs[0]);
            getargs(&xmltag[ii], "title", vargs[1]);
            if (strcmp(vargs[0], scrn) != 0)
                continue;
            getargv(&xmltag[ii], "forwarding", &forwarding);
            header4scrn(scrn, vargs[1]);
            c_flow = 1;
            break;
        case 1:
            if (strcasecmp(xmltag[ii].tags, "/scrn") == 0)
            {
                field[nfld].type = -1;
                field[nfld].row = -1;
                field[nfld].col = -1;
                putfld(field, 0);
                endfld();
                return (0);
            }
            if (strcasecmp(xmltag[ii].tags, "text") == 0)
            {
                memset(&txtfld[0], 0, sizeof(struct field) * 2);
                txtfld[1].type = -1;
                txtfld[1].row = -1;
                txtfld[1].col = -1;
                txtfld[1].seq = -1;
                txtfld[0].type = FT_OUTPUT;
                txtfld[0].att = FC_CYAN;

                getargx(&xmltag[ii], "at", vargv, &many, 2);
                txtfld[0].row = vargv[0];
                txtfld[0].col = vargv[1];
                getargs(&xmltag[ii], "attr", vargs[0]);
                getargs(&xmltag[ii], "fc", vargs[1]);
                getargs(&xmltag[ii], "msg", vargs[2]);
                if (strstr(vargs[0], "reverse") != NULL)
                    txtfld[0].att |= FA_REVERSE;
                if (strstr(vargs[0], "bold") != NULL)
                    txtfld[0].att |= FA_BOLD;
                if (strstr(vargs[1], "green") != NULL)
                    txtfld[0].att |= FC_GREEN;
                else if (strstr(vargs[1], "red") != NULL)
                    txtfld[0].att |= FC_RED;
                strcpy(txtfld[0].msg, vargs[2]);
                putfld(txtfld, 0);
                break;
            }
            if (strcasecmp(xmltag[ii].tags, "tfield") == 0)
            {
                memset(&txtfld[0], 0, sizeof(struct field) * 2);
                txtfld[1].type = -1;
                txtfld[1].row = -1;
                txtfld[1].col = -1;
                txtfld[1].seq = -1;

                txtfld[0].seq = -1;
                txtfld[0].type = FT_OUTPUT;

                getargx(&xmltag[ii], "at", vargv, &many, 2);
                txtfld[0].row = vargv[0];
                txtfld[0].col = vargv[1];
                getargs(&xmltag[ii], "attr", vargs[0]);
                getargs(&xmltag[ii], "fc", vargs[1]);
                getargs(&xmltag[ii], "msg", vargs[2]);
                txtfld[0].att = FC_CYAN;
                strcpy(txtfld[0].msg, vargs[2]);
                putfld(txtfld, 0);

                getargs(&xmltag[ii], "name", vargs[0]);
                getargv(&xmltag[ii], "len", &vargv[0]);
                getargs(&xmltag[ii], "alignment", vargs[1]);
                if (strlen(vargs[0]) <= 0 || vargv[0] <= 0)
                    break;
                memset(&field[nfld], 0, sizeof(struct field));
                if (strcasecmp(vargs[0], "symb") == 0)
                    field[nfld].chk |= FLD_IS_SYMB;
                field[nfld].type = FT_OUTPUT;
                field[nfld].att = FC_WHITE;
                strcpy(field[nfld].name, vargs[0]);
                field[nfld].seq = -1;
                field[nfld].row = txtfld[0].row;
                field[nfld].col = txtfld[0].col + strlen(txtfld[0].msg);
                field[nfld].len = vargv[0];
                if (strcasecmp(vargs[1], "left") == 0)
                    field[nfld].chk |= FLD_IS_LEFT;
                nfld++;
                break;
            }

            if (strcasecmp(xmltag[ii].tags, "while") == 0)
            {
                getargs(&xmltag[ii], "mode", vargs[0]);
                getargv(&xmltag[ii], "howmany", vargv);
                if (strcasecmp(vargs[0], "insert") == 0)
                    insert_mode = 1;
                indx = nfld;
                nrow = vargv[0];
                loop = 1;
                break;
            }
            if (strcasecmp(xmltag[ii].tags, "field") == 0)
            {
                getargs(&xmltag[ii], "name", vargs[0]);
                getargx(&xmltag[ii], "at", &vargv[0], &many, 2);
                getargv(&xmltag[ii], "len", &vargv[2]);
                getargs(&xmltag[ii], "type", vargs[1]);
                getargs(&xmltag[ii], "alignment", vargs[2]);
                if (strlen(vargs[0]) <= 0 || vargv[0] <= 0 || vargv[1] <= 0 || vargv[2] <= 0)
                    break;
                memset(&field[nfld], 0, sizeof(struct field));
                if (strcasecmp(vargs[0], "symb") == 0)
                    field[nfld].chk |= FLD_IS_SYMB;
                field[nfld].type = FT_OUTPUT;
                strcpy(field[nfld].name, vargs[0]);
                if (loop)
                    field[nfld].seq = 0;
                else
                    field[nfld].seq = -1;
                field[nfld].row = vargv[0];
                field[nfld].col = vargv[1];
                field[nfld].len = vargv[2];
                if (strcasecmp(vargs[1], "input") == 0)
                    field[nfld].type = FT_INPUT;
                else if (strcasecmp(vargs[1], "inout") == 0)
                    field[nfld].type = FT_INOUT;
                if (strcasecmp(vargs[2], "left") == 0)
                    field[nfld].chk |= FLD_IS_LEFT;
                nfld++;
                break;
            }
            if (strcasecmp(xmltag[ii].tags, "/while") == 0 && loop)
            {
                kk = nfld;
                for (loop = 1; loop < nrow; loop++)
                {
                    for (jj = indx; jj < kk; jj++)
                    {
                        memcpy(&field[nfld], &field[jj], sizeof(struct field));
                        field[nfld].row += loop;
                        field[nfld].seq = loop;
                        nfld++;
                    }
                }
                loop = 0;
                break;
            }
        }
    }
    sprintf(vargs[0], "No such screen number '%s' !!!", scrn);
    setguide(vargs[0]);
    cur2fld("iSCRN");
    return (0);
}

void clearall()
{
    int ii;

    for (ii = 0; ii < nfld; ii++)
    {
        if (field[ii].type != FT_OUTPUT)
            continue;
        pushfld(field[ii].name, " ", FC_WHITE, 0);
    }
}
