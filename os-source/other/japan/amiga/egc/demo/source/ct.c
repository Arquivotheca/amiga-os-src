#ifndef lint
static char     rcsid[] = "$Id: egctest.c,v 3.4 91/06/20 21:45:27 katogi Exp Locker: katogi $";
#endif
/************************************************************************/
/*                                                                      */
/*    egctest : EGConvert  Single & Multi User Monitor test program     */
/*                                                                      */
/*              Designed    by  Y.Katogi        09/03/1990              */
/*              Coded       by  Y.Katogi        09/03/1990              */
/*              Modefied    by  H.Yanata        02/06/1993              */
/*                                                                      */
/*      (C) Copyright 1990-1993 ERGOSOFT Corp.                          */
/*      All Rights Reserved                                             */
/*                                                                      */
/*                         --- NOTE ---                                 */
/*                                                                      */
/*      THIS PROGRAM BELONGS TO ERGO SOFT CORP.  IT IS CONSIDERED A     */
/*      TRADE SECRET AND IS NOT TO BE DIVULGED OR USED BY PARTIES       */
/*      WHO HAVE NOT RECEIVED WRITTEN AUTHORIZATION FROM THE OWNER.     */
/*                                                                      */
/************************************************************************/
/*      < #ifdef definitions >
 * -DUNIX     : for UNIX
 * -DDOS_EGC  : for MS-DOS
 * -DEXP      : for Expert dictionary
 * -DALLSRC   : if you have all of egconvert source, yes
 * -DSIN      : for on UNIX single user mode  and on MS-DOS ignore
 */
/*----------------------------------------------------------------------*/

#include    <stdio.h>
#ifdef UNIX
#include    <signal.h>
#include    <strings.h>
#else                                  /* UNIX */
#include    <stdlib.h>
#include    <string.h>
#include    <process.h>
#endif                                 /* UNIX */

#include    <setjmp.h>

#include    "edef.h"
#include    "egcerr.h"
#include    "egcenv.h"

#ifdef ALLSRC
#include    "egcdef.h"
#include    "egckcnv.h"
#ifdef SIN
#include    "egcexp.h"
#include    "egcudep.h"
#endif
#else
#include    "egcldml.h"
#endif

#include    "egcdict.h"
#include    "egcldpro.h"
#include    "egcproto.h"

#define MAXBUF 3

UBYTE           buf[BUFSIZ * MAXBUF + 1];
UBYTE           src[BUFSIZ * MAXBUF + 1];
UBYTE           dst[BUFSIZ * MAXBUF + 1];
UBYTE           obuf[BUFSIZ * MAXBUF + 1];
UBYTE           srcstr[BUFSIZ * MAXBUF + 1];
UWORD           bufl;
UWORD           srcl;
UWORD           dstl;
UBYTE           dcep[512];
UBYTE           mode;
UBYTE           defcom;                /* default command */
UBYTE           mainp[64];
WORDID          dcip[512];
UWORD           sep[256];
UWORD           sepn;
UWORD           dnbr;
UWORD           fixl;
UWORD           totl;
int             ret_st;
DICR            dicinfo;
typedef struct _cont {
    UBYTE          *string;
    UWORD           length;
}               Context, *PtrCont;

#ifdef  UNIX                           /*----- for UNIX -----*/
UBYTE           inputbuf[1024];
UWORD           inputlen;
UBYTE           outputbuf[1024];
UWORD          *outputlen;
extern PTRWFUNC egcttype();
extern PTRWFUNC inputcvt;
extern PTRWFUNC outputcvt;
extern int      EGCDictID;
extern int      EGCUID;
#ifndef SIN                            /* MUM */
extern BYTE    *convert_hostname;
BYTE            hostname[16];
#endif
#endif                                 /* end of UNIX */

jmp_buf         jmp_start;

#ifdef ALLSRC
# ifdef  DEBUG
FILE           *db;
#define DB_FILE "cvt.db"
# endif                                /* DEBUG */
#endif

extern int      egcerrno;
extern int      egcverck;
extern int      egcsysck;
extern int      egcplock;


int             test_rmcvt();
int             test_txcvt();
int             test_clcvt();
int             test_wdadd();
int             test_wddel();
int             test_learn();
int             test_cll();
int             test_find();
int             test_read();
VOID            test_quit();
VOID            err_message();
VOID            outstr();
VOID            outdic();
VOID            test_version();
VOID            get_version();
VOID            set_version();
VOID            man_version();
VOID            str_version();
VOID            homdisp();
VOID            test_ud();
VOID            test_reinit();
VOID            test_slt_dump();
LONG            menu_version();
extern WORD     getinfo();
extern WORD     getidnum();
BOOL            get_string();
UWORD           get_cvt_mode();
PTRWFUNC        kccvt;

#ifdef DOS_EGC
int             test_openexp();
int             test_closexp();
#endif

#ifdef  DEBUG
FILE           *db;
#define DB_FILE "cvt.db"
#endif                                 /* DEBUG */

/*PAGE*/

VOID
main(argc, argv)
    REG             argc;
    UBYTE         **argv;
{
    WORD            wret;
    int             scom;
#ifdef  UNIX
    extern          abort();

    signal(SIGKILL, abort);
    signal(SIGTERM, abort);
    signal(SIGINT, abort);

    kccvt = egcttype();
    if (kccvt == (PTRWFUNC) NULL)
        exit(1);                    /* get term type for egcpart() */
#endif

#ifdef  DEBUG
    if ((db = fopen(DB_FILE, "w")) == NULL) {
        fprintf(stderr, "error : Can't open file %s.\n", DB_FILE);
        fprintf(stderr, "aborted\n");
        exit(1);
    }
#endif
    /* get command options */
    while (--argc, ++argv, (argc >= 1) && ((*argv)[0] == '-')) {
        switch ((*argv)[1]) {

        case 'i':
            egcverck = egcsysck = FALSE;
            break;
        case 'd':
            fprintf(stdout, "Main dictionary path & name ->");
            gets(&mainp[0]);
            fprintf(stdout, "User dictionary path & name ->");
            gets(&buf[0]);
            break;
#ifdef UNIX
#ifndef SIN
        case 'c':
            strcpy(hostname, &(*argv)[2]);
            convert_hostname = hostname;
            break;
#endif
#endif
        default:
            break;
        }
    }

#ifndef VER_OLD

    scom = setjmp(jmp_start);
    switch (scom) {
    case 100:
#ifdef SIN
        fprintf(stdout, "Main dictionary path & name ->");
        gets(&mainp[0]);
        fprintf(stdout, "User dictionary path & name ->");
        gets(&buf[0]);
        egcsetmaindic(mainp);
        egcsetuserdic(buf);
        break;
#endif
#ifndef SIN
        fprintf(stdout, "Input New hostname ->");
        gets(&buf[0]);
        strcpy(hostname, buf);
        convert_hostname = hostname;
        break;
#endif
    case 1:
    default:
        break;
    }

    wret = egcinit();
    switch (wret) {
    case NORMAL:                    /* error case */
        break;
    case ERINI03:
        fprintf(stderr, "It's new dictionary, use new EGConvert!!\n");
        goto init_err;

    case ERINI02:                   /* OK */
    case ERINI04:                   /* OK */
        goto init_ok;
    default:
init_err:
        fprintf(stderr, "EGConvert : connect error. %x\n", wret);
        fprintf(stderr, "            egcerr  error. %d\n", egcerrno);
        exit(0);
    }
init_ok:
#else
    egcinit();
    if (egcuson() != NORMAL) {
        fprintf(stderr, "EGConvert : connect error.\n");
        exit(0);
    }
#endif

    for (;;) {
        fprintf(stdout, "\n");
        fprintf(stderr, " EGConvert Test Program\n");
        fprintf(stderr, "        Version 1.20   \n");
        fprintf(stderr, "----< Menu >-----------\n");
        fprintf(stderr, " ASCII to Romaji    : 1\n");
        fprintf(stderr, " Text conversion    : 2\n");
        fprintf(stderr, " Clause conversion  : 3\n");
        fprintf(stderr, " Kanji addition     : 4\n");
        fprintf(stderr, " Kanji deletion     : 5\n");
        fprintf(stderr, " Learning           : 6\n");
        fprintf(stderr, " Clause learning    : 7\n");
        fprintf(stderr, " Find dictionary    : 8\n");
#ifdef EXP
#ifdef DOS_EGC
        fprintf(stderr, " Open Expert dic    : 9\n");
        fprintf(stderr, " Close Expert dic   : 0\n");
#endif
#ifdef UNIX
        fprintf(stderr, " Select expert dict : s\n");
#endif
#endif
        fprintf(stderr, " Version mode       : v\n");
#ifdef SIN
#ifdef ALLSRC
        fprintf(stderr, " User dict dump     : u\n");
        fprintf(stderr, " SLT dump           : l\n");
#endif
#endif
        fprintf(stderr, " Reinit EGConvert   : r\n");
        fprintf(stderr, " Quit               : q\n");
        fprintf(stderr, "-----------------------\n");
#ifdef UNIX
#ifndef SIN
        fprintf(stderr, "DICT[%2d] EGCUID[%2d]   \n", EGCDictID, EGCUID);
        fprintf(stderr, "-----------------------\n");
#endif
#endif
        fprintf(stdout, "Command [%c]--> ", (defcom) ? defcom : '_');
        gets(buf);
        if (*buf == 0)
            *buf = defcom;
        else
            defcom = *buf;
        switch (*buf) {
        case '1':
            ret_st = test_rmcvt();
            break;
        case '2':
            ret_st = test_txcvt();
            break;
        case '3':
            ret_st = test_clcvt();
            break;
        case '4':
            ret_st = test_wdadd();
            break;
        case '5':
            ret_st = test_wddel();
            break;
        case '6':
            ret_st = test_learn();
            break;
        case '7':
            ret_st = test_cll();
            break;
        case '8':
            ret_st = test_find();
            break;
#if defined(EXP) && defined(DOS_EGC)
        case '9':
            ret_st = test_openexp();
            break;
        case '0':
            ret_st = test_closexp();
            break;
#endif
#if defined(EXP) && defined(UNIX)
        case 's':
            ret_st = test_dicselecter();
            break;
#endif
        case 'v':
            test_version();
            break;
#ifdef SIN
#ifdef ALLSRC
        case 'u':
            test_ud();
            break;
        case 'l':
            test_slt_dump();
            break;
#endif
#endif
        case 'r':
            test_reinit();
            break;
        case 'q':
        case 'Q':
            test_quit();
            break;
        default:
            fprintf(stderr, "Illegal command [%c]\n", *buf);
            break;
        }
        if (ret_st != NORMAL) {
            err_message(ret_st);
        }
    }
}                                      /* end of main */

/*
 * ASCII to Romaji
 */
test_rmcvt()
{
    UBYTE           imode;
    UWORD           status;

    fprintf(stdout, "Input romaji string --> ");
    gets(src);
    srcl = (WORD) strlen(src);
    sprintf(obuf, "stat:dstl:totl\n");
    outstr(obuf, strlen(obuf));
    for (imode = 0; imode < 3; imode++) {
        status = egcrmcvt(src, srcl, dst, &dstl, &totl, imode);
        sprintf(obuf, " %2d : %2d : %2d ", status, dstl, totl);
        outstr(obuf, strlen(obuf));
        strncpy(buf, dst, (int) totl);
        *(buf + totl) = '\0';
        sprintf(obuf, "MODE[%d]=%s\n", imode, buf);
        outstr(obuf, strlen(obuf));
    }
    return (NORMAL);
}

/*
 * Hommonym
 */
test_clcvt()
{
    UBYTE          *p;

    fprintf(stdout, "Input romaji string [%s]--> ",
            (UBYTE *) ((*srcstr) ? srcstr : (UBYTE *) "?"));
    gets(src);
    if (*src == 0)
        strcpy(src, srcstr);
    else
        strcpy(srcstr, src);
    do {
        fprintf(stdout, "Input romaji mode 0 : Hankaku\n");
        fprintf(stdout, "                  1 : Zen_kana\n");
        fprintf(stdout, "                  2 : Zen_hera\n");
        fprintf(stdout, "                  3 : Not change [2]--> ");
        gets(dst);
        if (*dst == 'q')
            return (NORMAL);
        if (*dst == 0)
            *dst = 0x32;
        mode = *dst - (UBYTE)0x30;
    } while (mode < 0 || mode > 3);
    srcl = (WORD) strlen(src);
    if (mode == 3) {
        strncpy(dst, src, srcl);
        dstl = srcl;
    }
    else {
        egcrmcvt(src, srcl, dst, &dstl, &sepn, mode);
    }

    strncpy(src, dst, dstl);
    *(src + dstl) = '\0';
    srcl = (WORD) strlen(src);
    do {
        fprintf(stdout, "Convert mode 0 : Bunsetu\n");
        fprintf(stdout, "             1 : Homonym\n");
        fprintf(stdout, "             2 : Special [1]--> \n");
        gets(dst);
        if (*dst == 'q')
            return (NORMAL);
        if (*dst == 0)
            *dst = 0x31;
        mode = *dst - (UBYTE)0x30;
    } while (mode < 0 || mode > 2);

    if (mode == 2) {
        egctxcvt(src, srcl, dst, &dstl, sep, &sepn, &fixl, 0);
        dcep[0] = dstl;
    }
    ret_st = (int) egcclcvt(src, srcl, dst, dcep, dcip, &dnbr, mode);

    sprintf(obuf, "dnbr = %d\n", dnbr);
    outstr(obuf, strlen(obuf));

    p = dst;
    homdisp(p);
    return (ret_st);
}
/*PAGE*/

test_txcvt()
{
    UWORD           j;
    UWORD          *wp;

    fprintf(stdout, "Input romaji string or ^a \n");
    fprintf(stdout, "if ^a then file input \n");
    fprintf(stdout, "[%s]--> ", (UBYTE *) ((*srcstr) ? srcstr : (UBYTE *) "?"));
    gets(src);
    if (*src == 0x01) {
        FILE           *fpin;

        fprintf(stdout, "Input filename ->");
        gets(src);

        if ((fpin = fopen(src, "r")) == NULL) {
            fprintf(stderr, "Cannot open file is %s!\n", src);
            return (ERROR);
        }
        else {
            register int    i = 0;
            while(fread(&src[i], 1, 1, fpin)) {
                i ++;
            }
            src[i] = 0x00;
        }
    }
    else if (*src == (UBYTE)0)
        strcpy(src, srcstr);
    else
        strcpy(srcstr, src);
    do {
        fprintf(stdout, "Input romaji mode 0 : Hankaku\n");
        fprintf(stdout, "                  1 : Zen_kana\n");
        fprintf(stdout, "                  2 : Zen_hera\n");
        fprintf(stdout, "                  3 : Not change [2]--> ");
        gets(dst);
        if (*dst == 'q')
            return (NORMAL);
        if (*dst == (UBYTE)0)
            *dst = 0x32;
        mode = *dst - (UBYTE)0x30;
    } while (mode < 0 || mode > 3);
    srcl = (WORD) strlen(src);
    if (mode == 3) {
        strcpy(dst, src);
        dstl = srcl;
    }
    else {
        egcrmcvt(src, srcl, dst, &dstl, &sepn, mode);
    }
    strcpy(buf, dst);
    *(buf + dstl) = '\0';
    bufl = (WORD) strlen(buf);
    sprintf(obuf, "Strings  -> %s\n", buf);
    outstr(obuf, strlen(obuf));
    fprintf(stdout, "Convert mode 0 : Batch\n");
    fprintf(stdout, "             1 : Free\n");
    fprintf(stdout, "             2 : Bufferd\n");
    fprintf(stdout, "             3 : Compound [0]--> ");
    gets(src);
    if (*src == 'q')
        return (NORMAL);
    if (*src) {
        mode = (UBYTE)atoi(src);
        if (mode < 0 || mode > 3)
            mode = 0;
    } else {
        mode = 0;
    }
    ret_st = (int) egctxcvt(buf, bufl, dst, &dstl, sep, &sepn, &fixl, mode);
    dst[dstl] = '\0';
    fprintf(stdout, "dst=%s\n", dst);

    sprintf(obuf, "dstl = %d : sepn = %d : fixl = %d\n", dstl, sepn, fixl);
    outstr(obuf, strlen(obuf));

    sprintf(obuf, "sep   -> | ");
    outstr(obuf, strlen(obuf));
    for (j = sepn, wp = sep; j > 0; j--) {
        sprintf(obuf, "0x%02X ", *(wp++));
        outstr(obuf, strlen(obuf));
        sprintf(obuf, "%d ", *(wp++));
        outstr(obuf, strlen(obuf));
        sprintf(obuf, "%d | ", *(wp++));
        outstr(obuf, strlen(obuf));
/*@@@ Test, show wakachi_go @@@*/
#if 0
        egcmemmove(sdst + k, dst + 
#endif
    }
    sprintf(obuf, "\n");
    outstr(obuf, strlen(obuf));

    *(dst + dstl) = '\0';
    sprintf(obuf, "Kanji -> %s\n", dst);
    outstr(obuf, strlen(obuf));
    return (ret_st);
}
/*PAGE*/

/*
 * This program quit.
 */
VOID
test_quit()
{
#ifdef  UNIX
    egcusoff();
#endif                              /* UNIX */

#ifdef  DEBUG
    fclose(db);
#endif
    exit(1);
}
/*PAGE*/

/*
 * Word addition.
 */
test_wdadd()
{
    WORD            i;
    UBYTE           uh[4];

    fprintf(stdout, "Yomi input -> ");
    gets(src);
    if (*src == 'q')
        return (NORMAL);
    srcl = (WORD) strlen(src);
    do {
        fprintf(stdout, "Input romaji mode 0 : Hankaku\n");
        fprintf(stdout, "                  1 : Zen_kana\n");
        fprintf(stdout, "                  2 : Zen_hera\n");
        fprintf(stdout, "                  3 : Not change [2]--> ");
        gets(dst);
        if (*dst == 'q')
            return (NORMAL);
        if (*dst == (UBYTE)0)
            *dst = 0x32;
        mode = *dst - (UBYTE)0x30;
    } while (mode < 0 || mode > 3);
    srcl = (WORD) strlen(src);
    if (mode == 3) {
        srcl = cvtatoj(1, src, srcl, dst);
        dstl = srcl;
    }
    else {
        egcrmcvt(src, srcl, dst, &dstl, &sepn, mode);
    }
    strncpy(buf, dst, dstl);
    *(buf + dstl) = '\0';
    bufl = (WORD) strlen(buf);

    fprintf(stdout, "kanji yomi input -> ");
    gets(src);
    if (*src == 'q')
        return (NORMAL);
    srcl = (WORD) strlen(src);
    egcrmcvt(src, srcl, dst, &dstl, &totl, (WORD) 2);

    *(dst + dstl) = '\0';
    strcpy(src, dst);
    srcl = (WORD) strlen(src);
    *(src + srcl) = '\0';
    fprintf(stdout, "Convert mode (0 or 1) ? ");
    gets(dst);
    if (*dst == 'q')
        return (NORMAL);
    if (*dst == 0)
        mode = 0;
    else
        mode = (BYTE) ((*dst == '0') ? 0 : 1);
    ret_st = (int) egctxcvt(src, srcl, dst, &dstl, sep, &sepn, &fixl, mode);
    if (ret_st == NORMAL) {
        for (i = 0; i < 4; i++) {
            fprintf(stdout, "hinshi[%d] = ", i);
            gets(src);
            if (*src == 'q')
                return (NORMAL);
            uh[i] = (UBYTE) atoi(src);
        }

        *(dst + dstl) = '\0';
        sprintf(obuf, "Yomi  string  [%s]\n", buf);
        outstr(obuf, strlen(obuf));
        sprintf(obuf, "Kanji string  [%s]\n", dst);
        outstr(obuf, strlen(obuf));
        fprintf(stdout, "Is this OK (y / n) ? ");
        gets(src);
        if (*src == 'n' && *src == 'N') {
            return (NORMAL);
        }
        ret_st = (int) egcwdadd(buf, bufl, dst, dstl, uh);
    }
    return (ret_st);
}
/*PAGE*/
test_wddel()
{
    UBYTE          *p;
    register UWORD  i;
    int             size;

    mode = 2;
    fprintf(stdout, "Input romaji string --> ");
    gets(src);
    if (*src == 'q')
        return (NORMAL);
    srcl = (WORD) strlen(src);
    do {
        fprintf(stdout, "Input romaji mode 0 : Hankaku\n");
        fprintf(stdout, "                  1 : Zen_kana\n");
        fprintf(stdout, "                  2 : Zen_hera\n");
        fprintf(stdout, "                  3 : Not change [2]--> ");
        gets(dst);
        if (*dst == 'q')
            return (NORMAL);
        if (*dst == (UBYTE)0)
            *dst = 0x32;
        mode = *dst - (UBYTE)0x30;
    } while (mode < 0 || mode > 3);
    srcl = (WORD) strlen(src);
    if (mode == 3) {
        srcl = cvtatoj(1, src, srcl, dst);
        dstl = srcl;
    }
    else {
        egcrmcvt(src, srcl, dst, &dstl, &sepn, mode);
    }
    strncpy(src, dst, dstl);
    *(src + dstl) = '\0';
    srcl = (WORD) strlen(src);

    mode = (BYTE) 1;
    ret_st = (int) egcclcvt(src, srcl, dst, dcep, dcip, &dnbr, mode);
    if (ret_st == NORMAL) {
        p = dst;
        homdisp(p);
        fprintf(stdout, "Input delete number -> ");
        gets(buf);
        if (*buf == 'q')
            return (NORMAL);
        sepn = (WORD) atoi(buf);
        if (sepn < 0 || sepn >= dnbr) {
            fprintf(stdout, "Number [%d] is not found.\n", sepn);
            return (NORMAL);
        }
        size = 0;
        for (i = 0; i < sepn; i++) {
            size += dcep[i];
        }
        strncpy(buf, dst + size, dcep[i]);
        *(buf + dcep[i]) = '\0';
        dstl = (WORD) strlen(buf);
        sprintf(obuf, "yomi [%d]   = %s\n", srcl, src);
        outstr(obuf, strlen(obuf));
        sprintf(obuf, "Kanji[%d]   = %s\n", dstl, buf);
        outstr(obuf, strlen(obuf));
        fprintf(stdout, "Is this OK (y / n) ? ");
        gets(dst);
        if (*dst == 'n' && *dst == 'N') {
            return (NORMAL);
        }
        ret_st = (int) egcwddel(src, srcl, buf, dstl);
    }
    return (ret_st);
}
/*PAGE*/

test_learn()
{
#ifdef OLDLRN
    WORDID         *ptr;
#endif
    UWORD           nbr;

    ret_st = test_clcvt();
    fprintf(stdout, "Learning number -> ");
    gets(buf);
    if (*buf == 'q')
        return (NORMAL);
    nbr = (WORD) atoi(buf);
    if (nbr < 0 || nbr >= dnbr) {
        fprintf(stdout, "Illegal numbert %d inputed.\n", nbr);
        return (NORMAL);
    }
#ifdef  OLDLRN
    ret_st = (int) egclearn(ptr);
#else
    ret_st = (int) egclearn2(dcip, dnbr, nbr);
#endif                              /* OLDLRN */
    return (ret_st);
}
/*PAGE*/

test_find()
{
    UBYTE           smode;
    UBYTE           dmode;
    WORD            idno;

    fprintf(stdout, "Kind of dictionary\n");
    fprintf(stdout, " 0 or m : Main\n");
    fprintf(stdout, " 1 or u : User\n");
    fprintf(stdout, " 2 or e : Expert\n");
    fprintf(stdout, "Select return->");
    gets(buf);
    if (*buf == 'q')
        return (NORMAL);
    dmode = (*buf == '0' || *buf == 'm') ? (UBYTE) 0 :
        (*buf == '1' || *buf == 'u') ? (UBYTE) 1 :
        (*buf == '2' || *buf == 'e') ? (UBYTE) 2 : (UBYTE) 0;
    if (dmode == 2) {
        if (getinfo() == NORMAL) {
            if ((idno = (WORD) getidnum()) == NORMAL)
                return (NORMAL);
            dmode == (UBYTE) (idno + 1);
        }
    }

    fprintf(stdout, "Search Mode\n");
    fprintf(stdout, " 0 or s : Sequential\n");
    fprintf(stdout, " 1 or d : Direct\n");
    fprintf(stdout, "Select return->");
    gets(buf);
    if (*buf == 'q')
        return (NORMAL);
    smode = (*buf == (UBYTE)'0' || *buf == (UBYTE)'s') ? (UBYTE) 0 :
        (*buf == (UBYTE)'1' || *buf == (UBYTE)'d') ? (UBYTE) 1 : (UBYTE)0;

    fprintf(stdout, "Yomi -> ");
    gets(buf);
    if (*buf == 'q')
        return (NORMAL);

    bufl = (WORD) strlen(buf);
    mode = (BYTE) 0;
    egcrmcvt(buf, bufl, dst, &dstl, &totl, mode);
    *(dst + dstl) = '\0';
    if ((ret_st = (int) egcfind(dst, dstl, smode, dmode, &dicinfo)) != FALSE)
        return (ret_st);
    outdic(&dicinfo, 0);
    for (;;) {
        fprintf(stdout, "1 or n : next  2 or r : read   e or [CR]: Exit ");
        fprintf(stdout, "No ? ");
        gets(buf);
        fprintf(stdout, "\n");
        switch (*buf) {
        case 'n':
        case '1':                   /* egcnext */
            if ((ret_st = (int) egcnext(&dicinfo)) != FALSE) {
                return (ret_st);
            }
            outdic(&dicinfo, 1);
            break;
        case 'r':
        case '2':                   /* egcread */
            if ((ret_st = test_read()) != FALSE) {
                break;
            }
            break;
        case 'e':
        case 'q':                   /* break */
        case  0x00:
            return (NORMAL);
        default:
            fprintf(stdout, "Illegal number %s\n", buf);
            break;
        }
    }
}
/*PAGE*/

test_read()
{
    UBYTE           hinshi[4];

    strncpy(src, dst, (int) dstl);
    srcl = dstl;
    ret_st = (int) egcread(src, &srcl, dst, &dstl, hinshi, &dicinfo);
    if (ret_st != FALSE)
        return (ret_st);
    *(dst + dstl) = 0;
    src[srcl] = '\0';
    sprintf(obuf, "Yomi    = %s\n", src);
    outstr(obuf, strlen(obuf));
    dst[dstl] = '\0';
    sprintf(obuf, "Kanji   = %s\n", dst);
    outstr(obuf, strlen(obuf));
    outdic(&dicinfo, 2);
    return (ret_st);
}
/*PAGE*/

VOID
outdic(dic, cmd)
    DICRP           dic;
    int             cmd;
{
    UBYTE           wcode;
    UWORD           i;

    sprintf(obuf, "Dic Infomation  ");
    outstr(obuf, strlen(obuf));
    switch (cmd) {
    case 0:
        sprintf(obuf, "[FIND]\n");
        break;
    case 1:
        sprintf(obuf, "[NEXT]\n");
        break;
    case 2:
        sprintf(obuf, "[READ]\n");
        break;
    }
    outstr(obuf, strlen(obuf));
    sprintf(obuf, "\tdic->smode  = %d\n", (int) (dic->smode));
    outstr(obuf, strlen(obuf));
    sprintf(obuf, "\tdic->dmode  = %d\n", (int) (dic->dmode));
    outstr(obuf, strlen(obuf));
    for (i = 0; i < dic->yomil; i++)
        *(buf + i) = *(dic->yomi + i);
    *(buf + dic->yomil) = '\0';
    sprintf(obuf, "\tdic->yomi   = %s\n", buf);
    outstr(obuf, strlen(obuf));
    sprintf(obuf, "\tdic->yomil  = %d\n", (int) (dic->yomil));
    outstr(obuf, strlen(obuf));
    sprintf(obuf, "\tdic->hinshi = \n");
    outstr(obuf, strlen(obuf));
    sprintf(obuf, "\t| 0 1 2 3 4 5 6 7 | 8 9 0 1 2 3 4 5 | 6 7 8 9 0 1 2 3 | 4 5 6 7 8 9 0 1\n");
    outstr(obuf, strlen(obuf));
    sprintf(obuf, "\t");
    outstr(obuf, strlen(obuf));
    for (i = 0; i < 4; i++) {
        sprintf(obuf, "| ");
        outstr(obuf, strlen(obuf));
        for (wcode = 0x80; wcode; wcode >>= 1) {
            if (wcode & dic->hinshi[i]) {
                sprintf(obuf, "1 ");
                outstr(obuf, strlen(obuf));
            }
            else {
                sprintf(obuf, "0 ");
                outstr(obuf, strlen(obuf));
            }
        }
    }
    sprintf(obuf, "\n");
    outstr(obuf, strlen(obuf));
    sprintf(obuf, "\tdic->recq   = %d\n", (int) (dic->recq));
    outstr(obuf, strlen(obuf));
    sprintf(obuf, "\tdic->segno  = %d\n", (int) (dic->segno));
    outstr(obuf, strlen(obuf));
    sprintf(obuf, "\tdic->recno  = %d\n", (int) (dic->recno));
    outstr(obuf, strlen(obuf));
    sprintf(obuf, "\tdic->dbks   = %d\n", (int) (dic->dbks));
    outstr(obuf, strlen(obuf));
    sprintf(obuf, "\tdic->dbkq   = %d\n", (int) (dic->dbkq));
    outstr(obuf, strlen(obuf));
    sprintf(obuf, "\tdic->dbws   = %d\n", (int) (dic->dbws));
    outstr(obuf, strlen(obuf));
    sprintf(obuf, "\tdic->dbwq   = %d\n", (int) (dic->dbwq));
    outstr(obuf, strlen(obuf));
}
/*PAGE*/

test_cll()
{
#define CLLSIZ  100
    UBYTE           yomi1[CLLSIZ + 1];
    UBYTE           yomi2[CLLSIZ + 1];
    UBYTE           kanji1[CLLSIZ + 1];
    UBYTE           kanji2[CLLSIZ + 1];
    UWORD           yomil1;
    UWORD           yomil2;
    UWORD           kanjil1;
    UWORD           kanjil2;
    UBYTE           yomibuf[128];

    memset(yomibuf, 0x00, 128);
    printf("yomi(1) -> ");
    gets(yomi1);
    egcrmcvt(yomi1, strlen(yomi1), src, &yomil1, &totl, 2);
    memcpy(yomibuf, src, yomil1);
    egctxcvt(src, yomil1, kanji1, &kanjil1, sep, &sepn, &fixl, 0);
    kanji1[kanjil1] = 0x00;
    printf("yomi(2) -> ");
    gets(yomi2);
    egcrmcvt(yomi2, strlen(yomi2), src, &yomil2, &totl, 2);
    memcpy(yomibuf + yomil1, src, yomil2);
    egctxcvt(src, yomil2, kanji2, &kanjil2, sep, &sepn, &fixl, 0);
    kanji2[kanjil2] = 0x00;

    sprintf(obuf, "Yomi  string  %s/%s\n", yomi1, yomi2);
    outstr(obuf, strlen(obuf));
    sprintf(obuf, "Kanji string  %s/%s\n", kanji1, kanji2);
    outstr(obuf, strlen(obuf));
    fprintf(stdout, "Is this OK (y or n) ? ");
    gets(src);
    if (*src == 'q')
        return (NORMAL);
    if (*src == 'n' && *src == 'N') {
        return (NORMAL);
    }
    strcpy(buf, kanji1);
    strcpy(buf + kanjil1, kanji2);
    ret_st = (int) egccllearn(buf, kanjil1, kanjil2, yomibuf, yomil1, yomil2);
    return (ret_st);
}
/*PAGE*/

VOID            test_version()
{
    fprintf(stdout, "<Version Mode Menu>\n");
    fprintf(stdout, "Version Get or Set (g or s)->");
    gets(src);
    if (*src == 'g')
        get_version();
    else if (*src == 's') {
        fprintf(stdout, "<Version Set Mode Menu>\n");
        fprintf(stdout, "What's type of version \n");
        fprintf(stdout, " Strcuture or Management(s or m)->");
        gets(src);
        if (*src == 'm') {
            man_version(menu_version());
        }
        else if (*src == 's') {
            str_version(menu_version());
        }
        else
            fprintf(stdout, " Not change\n");
    }
    else
        fprintf(stdout, " Not change\n");
}
LONG            menu_version()
{
    unsigned int    major, minor, micro;
    LONG            verl;

    fprintf(stdout, "<Set Version>\n");
    fprintf(stdout, " ----------------------------------\n");
    fprintf(stdout, " |Example version number 3.12.7   |\n");
    fprintf(stdout, " |   major=3: minor=12: micro=7   |\n");
    fprintf(stdout, " |   (0 <= value <= 99)           |\n");
    fprintf(stdout, " |Your input                      |\n");
    fprintf(stdout, " |Input Version ->3.12.7[Return]  |\n");
    fprintf(stdout, " ----------------------------------\n");
    do {
        fprintf(stdout, "Try input version ->");
        fscanf(stdin, "%d.%d.%d", &major, &minor, &micro);
    } while (major >= 99 || minor >= 99 || micro > 99);
    verl = (LONG) major *10000 + (LONG) minor *100 + micro;
    return (verl);
}

VOID    man_version(lv)
    LONG            lv;
{
    if (egcsetmaindicver(NULL, lv) == -1L)
        fprintf(stderr, "Can't Set Maindict-version\n");
    if (egcsetuserdicver(NULL, lv) == -1L)
        fprintf(stderr, "Can't set Userdict-version\n");
}
VOID    str_version(lv)
    LONG            lv;
{
    if (egcsetmainstrver(NULL, lv) == -1L)
        fprintf(stderr, "Can't Set Maindict-version\n");
    if (egcsetuserstrver(NULL, lv) == -1L)
        fprintf(stderr, "Can't set Userdict-version\n");
}

#define v_ltoc(cv, lv) \
        *cv = (UBYTE) (((LONG) lv / 10000) % 100); \
        *(cv + 1) = (UBYTE) ((int) (lv / 100) % 100); \
        *(cv + 2) = (UBYTE) (lv % 100);

VOID            get_version()
{
    LONG            msl, mml, uml, usl;
    UBYTE           vbuf[16];
    UBYTE           ver[4];

    fprintf(stdout, "             Structure| Management \n");
    fprintf(stdout, "MainDict ->");
    if ((msl = egcgetmainstrver(NULL)) == -1L) {
        fprintf(stderr, "  ERROR ");
    }
    else {
        v_ltoc(ver, msl);
        sprintf(vbuf, "  %2d.%2d.%2d", ver[0], ver[1], ver[2]);
        fprintf(stdout, "%s", vbuf);
    }
    fprintf(stdout, " |");
    if ((mml = egcgetmaindicver(NULL)) == -1L) {
        fprintf(stderr, "  ERROR \n");
    }
    else {
        v_ltoc(ver, mml);
        sprintf(vbuf, "%2d.%2d.%2d", ver[0], ver[1], ver[2]);
        fprintf(stdout, "%s\n", vbuf);
    }

    fprintf(stdout, "UserDict ->");
    if ((usl = egcgetuserstrver(NULL)) == -1L) {
        fprintf(stderr, "  ERROR ");
    }
    else {
        v_ltoc(ver, usl);
        sprintf(vbuf, "  %2d.%2d.%2d", ver[0], ver[1], ver[2]);
        fprintf(stdout, "%s", vbuf);
    }
    fprintf(stdout, " |");
    if ((uml = egcgetuserdicver(NULL)) == -1L) {
        fprintf(stderr, "  ERROR \n");
    }
    else {
        v_ltoc(ver, uml);
        sprintf(vbuf, "%2d.%2d.%2d", ver[0], ver[1], ver[2]);
        fprintf(stdout, "%s\n", vbuf);
    }
}


VOID
err_message(error)
    int             error;
{
    fprintf(stderr, "Something error %4x\n", error);
}

VOID
outstr(str, n)
    UBYTE          *str;               /* outbuf string */
    int             n;                 /* outbuf length */
{

#ifdef  UNIX
    UBYTE           strbuf[BUFSIZ + 1];
    UWORD           tlen;

    (VOID) kccvt(str, n, strbuf, &tlen);        /* code conversion */
    *(strbuf + tlen) = NULL;
    fprintf(stdout, "%s", strbuf);

#else
    fprintf(stdout, "%s", str);
#endif                              /* UNIX */
}
/*PAGE*/


 /* homonym display */
VOID            homdisp(p)
    UBYTE          *p;
{
    register UWORD  i;
    for (i = 0; i < dnbr; i++) {

        strncpy(buf, p, dcep[i]);
        *(buf + dcep[i]) = '\0';
        sprintf(obuf, "[%3d] :%-9s :len=%3d", i, buf, dcep[i]);
#ifdef ALLSRC
        {
            UWORD           wiseg = GETWIDSEG(dcip[i].wisegno, dcip[i].wiseqno);
            UBYTE           wiseq = GETWIDSEQ(dcip[i].wiseqno);
            UWORD           wirec = GETWIDREC(dcip[i].wirecno);
            sprintf(obuf + strlen(obuf),
                    ":seg=%3d/%03x:seq=%3d/%02x:rec=%4d/%03x:id=%d/%x",
                    wiseg, wiseg, wiseq, wiseq, wirec, wirec,
                    wirec >> 12, wirec >> 12);

        }
#endif
        sprintf(obuf + strlen(obuf), "\n");
        outstr(obuf, strlen(obuf));
        p += dcep[i];
    }
}                                      /* end of homdisp */
#ifdef SIN
#ifdef ALLSRC
VOID    test_slt_dump()
{
    UDDH            udp;
    UWORD           maxseg;
    UWORD           ssno, esno;
    UWORD           usno;
    UWORD           detail = 0;
    UBYTE           maxslt;
    UBYTE           sltmode = 0;       /* clear flag */

    udp = dicprty[0].pds->hbuhdr;
    maxseg = getint(dicprty[0].pds->hbuhdr.uesegq);
    maxslt = (UBYTE) getint(dicprty[0].pds->hbuhdr.udmaxslt);
    while (1) {
        fprintf(stdout,
                " Input segment number [max:%d] (a:All,d:Detail,c:Clear) ->",
                maxseg);
        gets(buf);
        switch (*buf) {
        case 'q':
            return;
        case 'd':
            fprintf(stdout, " Detail");
            detail = !detail;
            if (detail) {
                fprintf(stdout, "  <on>\n");
                fprintf(stdout, " -------------------------\n");
                fprintf(stdout, " | <Mean>    |Kanji:Hara |\n");
                fprintf(stdout, " | ----------|-----------|\n");
                fprintf(stdout, " | Main dict |  M  :  *  |\n");
                fprintf(stdout, " | User dict |  U  :  &  |\n");
                fprintf(stdout, " -------------------------\n");
            }
            else
                fprintf(stdout, " <off>\n");
            break;
        case 'c':
        case 'C':
            sltmode = (UBYTE)(! sltmode);
            if (sltmode)
                fprintf(stdout, "Clear <on>\n");
            else
                fprintf(stdout, " Clear <off>\n");
            break;
        case 0x00:
        case 'a':
        case 'A':
            ssno = 1, esno = maxseg;
            goto slt_go;
        default:
            usno = (UWORD) atoi(buf);
            if ((usno > 0) && (usno <= maxseg)) {
                ssno = esno = usno;
                goto slt_go;
            }
        }
    }
slt_go:
    for (; ssno <= esno; ssno++) {
        UBYTE           i = maxslt;
        UBYTE          *sltptr;

        if (detail) {
            fprintf(stdout, "[seq(7bit)/rec(9bit)]\n");
        }
        fprintf(stdout, "seg=%3d:slt=", ssno);
#ifdef  PMSLT
        sltptr = GetSltp(dicprty[0].pds, (ssno - 1) * (maxslt * 2));
#else
        sltptr = dicprty[0].pds->hbuhdr.udsltp + ((ssno - 1) * (maxslt * 2));
#endif
        for (; i--; sltptr += 2) {
            UWORD           sltval = getint(sltptr);
            if (sltmode == 1) {
                setint(sltptr, 0x0000);
            }
            else {
                if (sltval == 0x0000) {
                    fprintf(stdout, "non");
                    break;
                }
                if (detail) {
                    UWORD           recval = GETWIDREC(sltval);
                    UBYTE           seqval = (UBYTE)(sltval >> 9);
                    seqval = GETWIDSEQ(seqval);
                    fprintf(stdout, "%02x/", seqval);
                    fprintf(stdout, "%02x", recval);
                    if (seqval <= 62)
                        fprintf(stdout, "(M)");
                    else if ((seqval == 63) && (recval < MAXPLE))
                        fprintf(stdout, "(*)");
                    else if (((seqval == 63) && (recval > MAXPLE)) ||
                             (seqval == 127))
                        fprintf(stdout, "(&)");
                    else if ((recval > MAXPLE) && (seqval >= 64))
                        fprintf(stdout, "(U)");
                }
                else {
                    fprintf(stdout, "%04x", sltval);
                }
                fprintf(stdout, " ");
            }
        }                           /* end of for */
        if (sltmode == 1) {
            putuseg(0, dicprty[0].pds->hbuhdr.udsltp,
                    (UWORD) getint(dicprty[0].pds->hbuhdr.uesltl),
                    (UWORD) getint(dicprty[0].pds->hbuhdr.ueslts));
            sltmode = 0;
            fprintf(stdout, " deleted");
        }
        fprintf(stdout, " \n");
    }
}

VOID            test_ud()
{
    UBYTE           smode, dmode;
    UDDH            udp;
    UWORD           uex;
    UBYTE           in_char;
    UBYTE           hinshi[4];
    FILE           *ofp;
    UBYTE           outflags = FALSE;
    WORD            read_st, find_st;

    fprintf(stdout, "No Check PLO (y or n)->");
    gets(buf);
    if (*buf != 'n')
        egcplock = 0;

    fprintf(stdout, "Select output device -> return : stdout\n");
    fprintf(stdout, "                        f      : file\n");
    fprintf(stdout, "([CR] or f) ->?");
    gets(buf);
    if (*buf == 'f') {
        fprintf(stdout, "Select outputfile\n");
        fprintf(stdout, "     return : [egdicu.txt]\n");
        fprintf(stdout, "     other  : filename (input yours name)\n");
        fprintf(stdout, "([CR] or [u...]) ->?");
        gets(buf);
        if (*buf == (UBYTE)0) {
            strcpy(buf, "egdicu.txt");
        }
        if ((ofp = fopen(buf, "w")) == NULL) {
            fprintf(stdout, "open error %s\n", buf);
        }
        fprintf(stdout, "Writing %s\n", buf);
        outflags = TRUE;
        *buf = 'a';
        bufl = (UWORD) strlen(buf);
        mode = (UBYTE) 0;
        egcrmcvt(buf, bufl, dst, &dstl, &totl, mode);
        in_char = *dst;

    }
    else {                          /* Output <stdout> */
        ofp = stdout;

        udp = dicprty[0].pds->hbuhdr;

        fprintf(stdout, "path : %s\n", dicprty[0].pds->path);
        fprintf(stdout, "          Hex  Oct  Dec\n");
        fprintf(stdout, "         ------------------\n");
        fprintf(stdout, "udidxs :  %4x %4o %4d\n"
                ,udp.udidxs, udp.udidxs, udp.udidxs);
        fprintf(stdout, "udidxl :  %4x %4o %4d\n"
                ,udp.udidxl, udp.udidxl, udp.udidxl);
        fprintf(stdout, "udslts :  %4x %4o %4d\n"
                ,udp.udslts, udp.udslts, udp.udslts);
        fprintf(stdout, "udsltl :  %4x %4o %4d\n"
                ,udp.udsltl, udp.udsltl, udp.udsltl);
        fprintf(stdout, "udplos :  %4x %4o %4d\n"
                ,udp.udplos, udp.udplos, udp.udplos);
        fprintf(stdout, "udplol :  %4x %4o %4d\n"
                ,udp.udplol, udp.udplol, udp.udplol);
        fprintf(stdout, "uddsos :  %4x %4o %4d\n"
                ,udp.uddsos, udp.uddsos, udp.uddsos);
        fprintf(stdout, "udsegq :  %4x %4o %4d\n"
                ,udp.udsegq, udp.udsegq, udp.udsegq);
        uex = getint(udp.udexts);
        fprintf(stdout, "udexts :  %4x %4o %4d\n", uex, uex, uex);

        fprintf(stdout, "ueidxs :  %4x %4o %4d\n",
                udp.ueidxs, udp.ueidxs, udp.ueidxs);
        fprintf(stdout, "ueidxl :  %4x %4o %4d\n",
                udp.ueidxl, udp.ueidxl, udp.ueidxl);
        fprintf(stdout, "ueslts :  %4x %4o %4d\n",
                udp.ueslts, udp.ueslts, udp.ueslts);
        fprintf(stdout, "udsltl :  %4x %4o %4d\n",
                udp.uesltl, udp.uesltl, udp.uesltl);
        fprintf(stdout, "ueplos :  %4x %4o %4d\n",
                udp.ueplos, udp.ueplos, udp.ueplos);
        fprintf(stdout, "ueplol :  %4x %4o %4d\n",
                udp.ueplol, udp.ueplol, udp.ueplol);
        fprintf(stdout, "uedsos :  %4x %4o %4d\n",
                udp.uddsos, udp.uddsos, udp.uddsos);

        fprintf(stdout, "udidxp :  %8x \n", udp.udidxp);
        fprintf(stdout, "udsltp :  %8x \n", udp.udsltp);
        fprintf(stdout, "udplop :  %8x \n", udp.udplop);
        fprintf(stdout, "uddsop :  %8x \n", udp.uddsop);
        fprintf(stdout, "uesegq :  %4x %4o %4d\n",
                udp.uesegq, udp.uesegq, udp.uesegq);
        fprintf(stdout, "udmaxsl:  %4x %4o %4d\n",
                udp.udmaxslt, udp.udmaxslt, udp.udmaxslt);

        fprintf(stdout, "PL off :  %8x\n", dicprty[0].pds->ploff);
        fprintf(stdout, "DS off :  %8x\n", dicprty[0].pds->dsoff);
        fprintf(stdout, "uduidx: ");
        {
            int             i;
            for (i = 0; i < 44; i++) {
                if (!(i % 10))
                    fprintf(stdout, "\n        ");
                fprintf(stdout, "%02x,", (UBYTE) udp.uduidx[i]);
            }
        }
        /* yomi input start */
        fprintf(stdout, "\nYomi -> ");
        gets(buf);
        switch (*buf) {
        case 'q':
            return;
        case 0x00:
            *buf = 'a';
        }
        bufl = (UWORD) strlen(buf);
        buf[bufl] = '0';
        mode = (UBYTE) 0;
        egcrmcvt(buf, bufl, dst, &dstl, &totl, mode);
        in_char = *dst;
        fprintf(stdout, "n or [CR]: next, e or q: Exit ?\n");

    }
    smode = 0;                      /* Sequential */
    dmode = 1;                      /* user dict */
    *dst = in_char;
    dstl = 1;
    *(dst + dstl) = '\0';
    strncpy(src, dst, (int) dstl);
    srcl = dstl;
    find_st = egcfind(&src[0], srcl, smode, dmode, &dicinfo);
    if (find_st == ERFND01)
    	fprintf(stderr,"Nothing\n");
    else {
    while (TRUE) {
        if (find_st == ERNXT25) {
            sprintf(obuf, "error plo segno=%d", dicinfo.segno);
            find_st = 0;
            goto r_out;
        }
        if (egcread(src, &srcl, dst, &dstl, hinshi, &dicinfo)) {
            switch (read_st = egcnextdb(&dicinfo)) {
            case ERNXT25:
                sprintf(obuf, "error plo segno=%d", dicinfo.segno);
                goto r_out;
            case ERNXT01:
                sprintf(obuf, "error next ");
                goto r_exit;
            case FALSE:
            default:
                egcread(src, &srcl, dst, &dstl, hinshi, &dicinfo);
            }
        }

        src[srcl] = '\0';
        dst[dstl] = '\0';
        sprintf(obuf, "Yomi= %-15s ,Kanji= %-30s", src, dst);
r_out:
        if (outflags == TRUE) {
            fputs(obuf, ofp);
            fputs("\n", ofp);
        }
        else {
            fputs(obuf, ofp);
            gets(buf);
            if (*buf == (UBYTE)0) {
                continue;
            }
            else if (*buf == 'q' || *buf == 'e' || *buf == ' ') {
                break;
            }
        }
    }                               /* end of if */
	}
r_exit:
    if ((outflags == TRUE) && (ofp != 0))
        fclose(ofp);
}
#endif                                 /* SINGLE */
#endif

VOID            test_reinit()
{
    int             jcom;
#ifdef SIN
    egcfinish();
    fprintf(stdout, "Disconnect EGConvert success\n");
    fprintf(stdout, "Change dict : y or n\n");
#endif

#ifdef UNIX
#ifndef SIN
    egcusoff();
    fprintf(stdout, "Disconnect EGConvert success\n");
    fprintf(stdout, "Change host : y or n\n");
#endif
#endif

    fprintf(stdout, "Input (y or n)->");
    gets(buf);
    fprintf(stdout, "\n");
    jcom = (*buf != 'y') ? 1 : 100;
    longjmp(jmp_start, jcom);
}

UWORD
get_cvt_mode()
{
    UBYTE           Com[BUFSIZ];
    UWORD           Com_mode;

    do {
        fprintf(stdout, " 0 : Hankaku\n");
        fprintf(stdout, " 1 : Zen_kana\n");
        fprintf(stdout, " 2 : Zen_hira\n");
        fprintf(stdout, " 3 : Ascii\n");
        fprintf(stdout, " 4 : Kanji\n");
        fprintf(stdout, "Input convert mode -> ");
        gets(Com);
        switch (*Com) {
        case 'q':
            return (*Com);
        case 0x00:
            *Com = 0x32;
        }
        Com_mode = *Com - 0x30;
    } while (Com_mode < 0 || Com_mode > 5);
    return (Com_mode);
}

BOOL
get_string(msg, pcon)
    UBYTE          *msg;
    PtrCont         pcon;
{
    UWORD           len = pcon->length;

    if (msg != NULL)
        fprintf(stdout, msg);
    gets(pcon->string);
    if (*(pcon->string) == 'q')
        return (ERROR);
    pcon->length = (WORD) strlen(pcon->string);
    if (pcon->length > len) {
        fprintf(stderr, "Buffer overflow. %d byte\n", (UWORD) (pcon->length - len));
        return (ERROR);
    }
    *(pcon->string + pcon->length) = '\0';
    return (NORMAL);
}
/*--- End of Program -----*/
