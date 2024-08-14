
#ifndef lint
static char     rcsid[] = "$Id: egctbit.c,v 3.4 91/06/20 21:45:25 katogi Exp Locker: katogi $";
#endif
/*****************************************************************/
/*                                                               */
/*      EGCTBIT : EGConvert  Tankanji BIT on                     */
/*                                                               */
/*         Coded    by T.NOSAKA 05/09/1988                       */
/*         Designed by T.NOSAKA 05/09/1988                       */
/*                                                               */
/*         (C) Copyright 1988-1991 ERGOSOFT Corp.                */
/*         All Rights Reserved                                   */
/*                                                               */
/*                         --- NOTE ---                          */
/*                                                               */
/*  THIS PROGRAM BELONGS TO ERGO SOFT CORP.  IT IS CONSIDERED A  */
/*  TRADE SECRET AND IS NOT TO BE DIVULGED OR USED BY PARTIES    */
/*  WHO HAVE NOT RECEIVED WRITTEN AUTHORIZATION FROM THE OWNER.  */
/*                                                               */
/*****************************************************************/
#include <stdio.h>
#ifdef DOS_EGC
#include <process.h>
#include <malloc.h>
#endif
#include <string.h>
#include "edef.h"
#include "egcenv.h"
#include "egcerr.h"
#include "egckcnv.h"
#include "egcpart.h"
#include "egcdef.h"
#include "egcproto.h"
#include "egcldpro.h"
#include "egcudep.h"

#define  MAXD       63                 /* douongo MAX */
#define  ON         TRUE
#define  OFF        FALSE

WORD            egctbit();
VOID            egct_usage();
WORD            egcrename();
VOID            illegalOrder();
BOOL            tkcheck();

int             main(argc, argv)
    int             argc;
    char          **argv;
{
    UBYTE           ct;
    UBYTE           bfile[64];
    UBYTE           ownopt[8][64];
    UBYTE           bitflag = OFF;
    UBYTE         **hoptext;
    REG             index;
    WORD            ret;
    EGCPART         partopt;
    UBYTE           vers[EGCSYSVERLEN + 1];
    UBYTE           versbuf[DICNAMELEN];

    vers_ltoc(egcgetsysverno(), vers);
    sprintf(versbuf, "%d.%d.%d", vers[0], vers[1], vers[2]);
    fprintf(stdout, "\nERGOSOFT egctbit version %s\n", versbuf);

    if (egcgetopt(argc, argv, &partopt, DIRECTORY) == ERROR
        || egcgetownopt(argc, argv, ownopt) == ERROR) {
        egct_usage();
        return ERROR;
    }
    for (index = 0; index < 8; index++) {
        if (!strcmp(ownopt[index], "on")) {
            bitflag = ON;
            break;
        }
        else if (!strcmp(ownopt[index], "off")) {
            bitflag = OFF;
            break;
        }
    }
    if (index == 8) {
        egct_usage();
        return ERROR;
    }
    else {
        while (--index >= 0)
            fprintf(stderr, "unknown option %s\n", ownopt[index]);
        printf("\nbit operation : %s\n\n", (bitflag) ? "ON" : "OFF");
    }

    hoptext = partopt.hext;
    for (ct = 0; ct < 45; ct++) {
        sprintf(bfile, "%s%s%s.%s",
                partopt.dirs, DELIMITER, partopt.dirs, *hoptext++);
        switch (ret = egctbit(bitflag, partopt.egcpval.recval, bfile)) {
        case NORMAL:
            fprintf(stdout, "Bit %s success : file %s\n",
                    (UBYTE *) ((bitflag) ? "on" : "off"), bfile);
            break;
        case ERROR:
            fprintf(stdout, "Bit %s failure : file %s\n",
                    (UBYTE *) ((bitflag) ? "on" : "off"), bfile);
            break;
        }
    }
    fprintf(stdout, "\nComplete.\n");
    return (NORMAL);
}

/************************************************************************/
NONREC
WORD egctbit(sw, sqsize, readfile)
    REG             sw;                /* switch on or off */
    WORD            sqsize;            /* record size      */
    UBYTE          *readfile;
/************************************************************************/
{
    struct {
        UBYTE           yomi[12];
        UBYTE           hinshi[4];
        UBYTE           kanji[64];
    }               buf;
    UBYTE           yomi[12];          /* old yomi */
    WORD            same_yomi;         /* homonym counter */
    EGCFILE         rfp, wfp;
    BOOL            ret;
    UBYTE*          wfile = "wfile";

    if ((rfp = egcfopen(readfile, OPENMODE)) == (EGCFILE)NULL) {
        fprintf(stderr, "Cannot open file %s\n", readfile);
        return ERROR;
    }
    if ((wfp = egcfopen(wfile, CREATMODE)) == (EGCFILE)NULL) {
        fprintf(stderr, "Cannot creat work file %s\n");
        return ERROR;
    }

    same_yomi = 1;
    memset(yomi, (UBYTE) 0x00, 12);
    while ((ret = egcfread((UBYTE *)&buf, sqsize, 1, rfp)) == NORMAL) {
        if (same_yomi < MAXD) {
            if (sw) {
                if (tkcheck(buf.hinshi))
                    buf.hinshi[1] = 0x04;
            }
            else {
                buf.hinshi[1] &= ~0x04;
            }
        }
        if ((ret = egcfwrite((UBYTE *)&buf, sqsize, 1, wfp)) == ERROR) {
            egcfclose(rfp);
            egcfclose(wfp);
            return ERROR;
        }
        if (!memcmp(yomi, buf.yomi, 12))
            same_yomi++;
        else
            same_yomi = 1;
        if ((same_yomi > MAXD) && (buf.hinshi[0] || buf.hinshi[1] 
                                || buf.hinshi[2] || buf.hinshi[3])) {
            illegalOrder(yomi);
            egcfclose(rfp);
            egcfclose(wfp);
            return    ERROR;
        }
        memcpy(yomi, buf.yomi, 12);
    }
    egcfclose(rfp);
    egcfclose(wfp);
    egcrename(wfile, readfile);

    return NORMAL;
}

BOOL            tkcheck(hinshi)
    UBYTE          *hinshi;
{
    return ((hinshi[0] | hinshi[1] | hinshi[2] | hinshi[3]) == 0x00);
}

/*PAGE*/
/***********************************************************************/
NONREC
VOID egct_usage()
/***********************************************************************/
{
    fprintf(stderr, "Usage: egctbit <-1,2,3> [on or off] directory \n");
    fprintf(stderr, "       on  -> bit on\n");
    fprintf(stderr, "       off -> bit off\n");
    fprintf(stderr, "       1 -> 32 byte\n");
    fprintf(stderr, "       2 -> 48 byte\n");
    fprintf(stderr, "       3 -> 80 byte\n");

}
WORD
egcrename(workfile, loadfile)
    UBYTE          *workfile, *loadfile;
{
    return (unlink(loadfile) || rename(workfile, loadfile));
}

VOID
illegalOrder(yomi)
    UBYTE   *yomi;
{
    fprintf(stderr, "illegal data order. yomi = %s\n", yomi);
    fprintf(stderr, "set order normal.\n\n");
}
