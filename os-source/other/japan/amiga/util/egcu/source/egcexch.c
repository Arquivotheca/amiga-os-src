#ifndef lint
static char     rcsid[] = "$Id: egcexch.c,v 3.4 91/06/20 21:45:19 katogi Exp Locker: katogi $";
#endif
/************************************************************************/
/*                                                                      */
/*      EGCEXCH : EGConvert sequencial file EXCHanger                   */
/*                                                                      */
/*              Designed    by  Y.Katogi        07/24/1990              */
/*              Coded       by  Y.Katogi        07/24/1990              */
/*                                                                      */
/*      (C) Copyright 1990-1991 ERGOSOFT Corp.                          */
/*      All Rights Reserved                                             */
/*                                                                      */
/*                          --- NOTE ---                                */
/*                                                                      */
/*      THIS PROGRAM BELONGS TO ERGO SOFT CORP.  IT IS CONSIDERED A     */
/*      TRADE SECRET AND IS NOT TO BE DIVULGED OR USED BY PARTIES       */
/*      WHO HAVE NOT RECEIVED WRITTEN AUTHORIZATION FROM THE OWNER.     */
/*                                                                      */
/************************************************************************/
#include <stdio.h>
#include <string.h>
#ifdef UNIX
#include <signal.h>
#endif
#include "edef.h"
#include "egcenv.h"
#include "egcerr.h"
#include "egcdef.h"
#include "egckcnv.h"
#include "egcdict.h"
#include "egcpart.h"
#include "egcproto.h"
#include "egcldpro.h"
#include "egcudep.h"

/*
**  upper index constant, error value macros
*/
#define UIVAL 44
#define EREXC01 0x1E01                 /* work file open error  */
#define EREXC02 0x1E02                 /* load file open error  */
#define EREXC03 0x1E03                 /* load file read error  */
#define EREXC04 0x1E04                 /* work file write error */
#define EREXC05 0x1E05                 /* rename error          */
#define EREXC06 0x1E06                 /* seek error            */
#define EREXC07 0x1E07                 /* file close error      */
#define EREXC08 0x1E08                 /* illegal mode select   */

/*
**  function prototypes
*/
REG             main();
static VOID     abort();
static VOID     exchusage(
#ifdef  PROTOTYPE
VOID
#endif
);
static WORD     egcexch(
#ifdef  PROTOTYPE
UBYTE*, UBYTE*, UBYTE
#endif
);
static WORD     egcrename(
#ifdef  PROTOTYPE
UBYTE*, UBYTE*
#endif
);
static BOOL     bfClose(
#ifdef  PROTOTYPE
EGCFILE, EGCFILE
#endif
);

REG
main(argc, argv)
    REG             argc;
    UBYTE          *argv[];
{
    UBYTE           loadfile[64], workfile[64];
    UBYTE         **hoptext;
    REG             ct;
    WORD            retval;
    EGCPART         partopt;
    UBYTE           vers[EGCSYSVERLEN + 1];
    UBYTE           versbuf[DICNAMELEN];

    vers_ltoc(egcgetsysverno(), vers);
    sprintf(versbuf, "%d.%d.%d", vers[0], vers[1], vers[2]);

#ifdef UNIX
    signal(SIGKILL, abort);
    signal(SIGTERM, abort);
    signal(SIGINT, abort);
#endif

    fprintf(stdout, "\nERGOSOFT egcexch version %s\n", versbuf);

    if (egcgetopt(argc, argv, &partopt, DIRECTORY) == ERROR) {
        exchusage();
        return ERROR;
    }

    sprintf(workfile, "%s%s%s.wrk",
            partopt.dirs, DELIMITER, partopt.dirs);

    hoptext = partopt.hext;
    for (ct = 0; ct < UIVAL + 1; ct++) {
        sprintf(loadfile, "%s%s%s.%s",
                partopt.dirs, DELIMITER, partopt.dirs, *hoptext++);
        switch (retval = egcexch(loadfile, workfile, partopt.egcpval.recval)) {
            case NORMAL:
                fprintf(stdout, "File : %s exchanged.\n", loadfile);
                break;
            case EREXC02:
                fprintf(stdout, "No file %s", loadfile);
                break;
            case EREXC08:
                fprintf(stderr, "illegal mode select. abort.\n");
                return 1;
            default:
                fprintf(stdout, "Exchange error file %s", loadfile);
                fprintf(stdout, " error No.%4x\n", retval);
                break;
        }
    }
}
/*************************************************************/
/*  egcexch                                                  */
/*************************************************************/
static WORD
egcexch(loadfile, workfile, Mode)
    UBYTE          *workfile;
    UBYTE          *loadfile;
    UBYTE           Mode;
{
    UBYTE           newrec[MAXRECORD];
    UBYTE           oldrec[MAXRECORD];
    EGCFILE         workfp, loadfp;
    UBYTE           readlen, writelen;

    if ((workfp = egcfopen(workfile, CREATMODE)) == (EGCFILE)0) {
        return (EREXC01);
    }
    if ((loadfp = egcfopen(loadfile, OPENMODE)) == (EGCFILE)0) {
        egcfclose(workfp);
        return (EREXC02);
    }

    switch(Mode) {
        case    STOM: readlen = 32; writelen = 48; break;
        case    STOL: readlen = 32; writelen = 80; break;
        case    MTOL: readlen = 48; writelen = 80; break;
        case    MTOS: readlen = 48; writelen = 32; break;
        case    LTOS: readlen = 80; writelen = 32; break;
        case    LTOM: readlen = 80; writelen = 48; break;
        default     : readlen = 32; writelen = 80; break;
    }

    while (egcfread(oldrec, readlen, 1, loadfp) == NORMAL) {
        memset(newrec, 0, MAXRECORD);
        memcpy(newrec, oldrec, readlen);
        if (egcfwrite(newrec, writelen, 1, workfp) == ERROR) {
            bfClose(workfp, loadfp);
            return  EREXC04;
        }
    }
    if (bfClose(workfp, loadfp))
        return  EREXC07;
    if (egcrename(workfile, loadfile))
        return  EREXC05;

    return (NORMAL);
}
/*PAGE*/
static
WORD
egcrename(workfile, loadfile)
    UBYTE          *workfile, *loadfile;
{
    return (unlink(loadfile) || rename(workfile, loadfile));
}

static
VOID
exchusage()
{
    fprintf(stdout, "Usage: egcexch <-i,j,k,l,m,n> [-r] directory\n");
    fprintf(stdout, " r -> File extentions is romaji.\n");
    fprintf(stdout, " i -> 32 to 48\n");
    fprintf(stdout, " j -> 32 to 80\n");
    fprintf(stdout, " k -> 48 to 80\n");
    fprintf(stdout, " l -> 48 to 32\n");
    fprintf(stdout, " m -> 80 to 32\n");
    fprintf(stdout, " n -> 80 to 48\n");
}

#ifdef  UNIX
static
VOID
abort()
{
    fprintf(stdout, "abort\n");
    exit(1);
}
#endif

/*---------------------------------------------------------------------*/
/* bfClose() : file close           add by H.Yanata 11/06/1991         */
/*---------------------------------------------------------------------*/
static
BOOL    bfClose(workfp, loadfp)
    EGCFILE    workfp;
    EGCFILE    loadfp;
{
    if (egcfclose(workfp) || egcfclose(loadfp)) {
        fprintf(stderr, "file close error.\n");
        return TRUE;
    }
    else
        return FALSE;
}
