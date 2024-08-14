#ifndef lint
static char     rcsid[] = "$Id: dicsel.c,v 3.4 91/06/20 20:33:44 katogi Exp Locker: katogi $";
#endif
/************************************************************************/
/*                                                                      */
/*      DICSEL : EGConvert DICtionary select                            */
/*                                                                      */
/*              Designed    by  Y.Katogi        09/03/1990              */
/*              Coded       by  Y.Katogi        09/03/1990              */
/*                                                                      */
/*      (C) Copyright 1991 ERGOSOFT Corp.                               */
/*      All Rights Reserved                                             */
/*                                                                      */
/*                         --- NOTE ---                                 */
/*                                                                      */
/*      THIS PROGRAM BELONGS TO ERGO SOFT CORP.  IT IS CONSIDERED A     */
/*      TRADE SECRET AND IS NOT TO BE DIVULGED OR USED BY PARTIES       */
/*      WHO HAVE NOT RECEIVED WRITTEN AUTHORIZATION FROM THE OWNER.     */
/*                                                                      */
/************************************************************************/

#ifdef UNIX
#include    <stdio.h>
#include    <signal.h>
#include    <string.h>
#else                                  /* UNIX */
#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>
#include    <process.h>
#endif                                 /* UNIX */

#include    "edef.h"
#include    "egcenv.h"
#include    "egcerr.h"
#include    "egcdef.h"
#include    "egckcnv.h"
#include    "egcexp.h"
#include    "egcudep.h"
#include    "egcdict.h"
#include    "egcpart.h"
#include    "egcproto.h"
#include    "egcldpro.h"

#ifdef UNIX
#include    "ux.h"
#endif                                 /* UNIX */

extern WORD     egcopenexdic();
extern WORD     egccloseexdic();
extern WORD     egcexpdicinfo();
extern WORD     egcopenexpdic();
WORD            getidnum();

PDICS           pdics;
char           *romeext[] = {
    "!", "a", "i", "u", "e", "o",
    "ka", "ki", "ku", "ke", "ko",
    "sa", "si", "su", "se", "so",
    "ta", "ti", "tu", "te", "to",
    "na", "ni", "nu", "ne", "no",
    "ha", "hi", "hu", "he", "ho",
    "ma", "mi", "mu", "me", "mo",
    "ya", "yu", "yo",
    "ra", "ri", "ru", "re", "ro",
    "wa"
};
char           *asciiext[] = {
    "!",
    "", "", "", "", "",
    "", "", "", "", "",
    "", "", "", "", "",
    "", "", "", "", "",
    "", "", "", "", "",
    "", "", "", "", "",
    "", "", "", "", "",
    "", "", "",
    "", "", "", "", "",
    "", ""
};

/*---------------------------------------------------------------------*/
/*  egcgetopt() : get option command   modified by H.Yanata 19/06/1991 */
/*---------------------------------------------------------------------*/
WORD
egcgetopt(argc, argv, ptrpart)
    REG             argc;
    UBYTE          *argv[];
    EGCPART        *ptrpart;
{
#define    FORMAT   "%s%s%s.%s"
    UBYTE           opt;
    REG             index;
    BOOL            selected = TRUE;

    ptrpart->hext = (UBYTE **) asciiext;
    ptrpart->egcpval.recval = SEQVER_1;
    strcpy(ptrpart->lfile, "kjdictm.dic");
    strcpy(ptrpart->ufile, "kjdictu.dic");
    if (argc < 3) {
        return ERROR;
    }
    else {
        for (index = 1; index < argc; index++) {
            if (argv[index][0] == '-') {
                switch (opt = argv[index][1]) {
                case 'r':
                    ptrpart->hext = (UBYTE **) romeext;
                    break;
                case 'd':
                    strcpy(ptrpart->lfile, "egdicm.dic");
                    strcpy(ptrpart->ufile, "egdicu.dic");
                    break;
                case '1':
                    ptrpart->egcpval.recval = SEQVER_1;
                    selected = FALSE;
                    break;
                case '2':
                    ptrpart->egcpval.recval = SEQVER_2;
                    selected = FALSE;
                    break;
                case '3':
                    ptrpart->egcpval.recval = SEQVER_3;
                    selected = FALSE;
                    break;
                case 'i':
                    ptrpart->egcpval.recval = STOM;
                    selected = FALSE;
                    break;
                case 'j':
                    ptrpart->egcpval.recval = STOL;
                    selected = FALSE;
                    break;
                case 'k':
                    ptrpart->egcpval.recval = MTOL;
                    selected = FALSE;
                    break;
                case 'l':
                    ptrpart->egcpval.recval = MTOS;
                    selected = FALSE;
                    break;
                case 'm':
                    ptrpart->egcpval.recval = LTOS;
                    selected = FALSE;
                    break;
                case 'n':
                    ptrpart->egcpval.recval = LTOM;
                    selected = FALSE;
                    break;
                default:
                    fprintf(stderr, "unknown option %c.\n", opt);
                    return ERROR;
                }
            }
        }
        strcpy(ptrpart->dirs, argv[argc - 1]);
    }
    return NORMAL;
}                                      /* end of egcgetopt */

/*---------------------------------------------------------------------*/
/*      egcgetownopt() : get own option    modified by H.Yanata 19/06/1991 */
/*---------------------------------------------------------------------*/
WORD
egcgetownopt(argc, argv, optbuf)
    REG             argc;
    UBYTE          *argv[];
    UBYTE           optbuf[8][64];
{
    REG             index, i = 0;
    WORD            ret = ERROR;

    for (index = 1; index < argc; index++) {
        if (index > 8)
            break;
        if (argv[index][0] != '-') {
            ret = NORMAL;
            strcpy(optbuf[i], argv[index]);
            i++;
        }
    }
    return ret;
}

/*
 *  expert dictionary selecter
 */
#ifdef UNIX
test_dicselecter()
{
    int             idno, tmpid, ret, comsw;
    UBYTE           tmpbuf[64];

    for (;;) {
        fprintf(stdout, "Dictionary Info,Open,Close,Quit (i,o,c,q)");
        gets(tmpbuf);
        fprintf(stdout, "\n");
        comsw = (int) tmpbuf[0];
        switch (comsw) {
        case 'q':
            return (NORMAL);
        case 'i':
            getinfo();
            return (NORMAL);
        case 'c':
            getinfo();
            if ((idno = getidnum()) == NORMAL)
                break;
            ret = egccloseexpdic(idno);
            if (ret == NORMAL) {
                fprintf(stdout, "-->Close success\n");
                return (NORMAL);
            }
            else {
                fprintf(stdout, "-->Close error !!\n");
            }                       /* input continue */
            break;
        default:
            getinfo();
            if ((idno = getidnum()) == NORMAL)
                break;
            ret = egcopenexpdic(idno, (UBYTE *) "");
            if (ret == NORMAL) {
                fprintf(stdout, "-->Open success\n");
                return (NORMAL);
            }
            else if (ret == EROED01) {
                fprintf(stdout, "-->Open error !!\n");
            }
            else {                  /* Case EROED02 */
                fprintf(stdout, "-->Opened already !\n");
            }
            break;
        }                           /* switch */
    }                               /* for */
}
#endif

getinfo()
{
#ifdef UNIX
    UBYTE          *hdics;
    PDICS           tmpdics;

    egcexpdicinfo(&hdics);
    pdics = (PDICS) hdics;
    for (tmpdics = pdics; tmpdics->id != 0 && *tmpdics->name != 0; tmpdics++) {
        fprintf(stdout, "ID[%d] : %s\n", tmpdics->id, tmpdics->name);
    }
    free(hdics);
    return (NORMAL);
#endif

#ifdef DOS_EGC
    fprintf(stdout, "Not impliment\n");
    return (ERROR);
#endif
}

/* Get ID number */
/* return value : 0 -> quit mode */
WORD
getidnum()
{
#ifdef UNIX
    int             idno;
    UBYTE           tmpbuf[64];
    PDICS           tmpdics;

    for (;;) {                      /* Select dictionary */
        fprintf(stdout, "Select Dictionary ID number (e:Exit)\n>");
        (void) gets(tmpbuf);
#if 0                               /* Removed by hasimoto, because can't
                                     * redirect input 1991/02/03 */
        fflush(stdin);
#endif
        if ((tmpbuf[0] == NULL) || (tmpbuf[0] == '\n'))
            continue;
        idno = tmpbuf[0] - '0';
        if (tmpbuf[0] == 'e')
            return (NORMAL);
        else if ((idno >= MAXOPENDIC) || idno <= 0) {
            fprintf(stdout, "Illegal ID number\n>");
            continue;
        }
        break;
    }
    return (idno);
#endif

#ifdef DOS_EGC
    return (NORMAL);
#endif
}
/*PAGE*/
#ifdef DOS_EGC
/*
 *  open expert dictionary
 */
test_openexp()
{
    WORD            ret;
    WORD            idno;
    UBYTE           fullpath[128];

    idno = 1;
    fprintf(stdout, "Input Dictionary Name:\n");
    gets(&fullpath[0]);
    ret = egcopenexpdic(idno, fullpath);
    if (ret == NORMAL)
        fprintf(stdout, "-->Open success\n");
    else if (ret == EROED01)
        fprintf(stdout, "-->Open error!!\n");
    else                            /* Case EROED02 */
        fprintf(stdout, "-->Opened already!\n");
    return (NORMAL);
}

/*
 *  close expert dictionary
 */
test_closexp()
{
    WORD            ret;

    fprintf(stdout, "Expert dictionary\n");
    ret = egccloseexpdic(1);
    if (ret == NORMAL)
        fprintf(stdout, "-->close success\n");
    else
        fprintf(stdout, "-->close error!!\n");
    return (NORMAL);
}
#endif

/* end of programe */
