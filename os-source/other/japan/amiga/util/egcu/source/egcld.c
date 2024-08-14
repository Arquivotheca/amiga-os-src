/************************************************************************/
/*                                                                      */
/*      EGCLD : EGConvert  dictionary LoaD utility                      */
/*                                                                      */
/*              Designed    by  I.Iwata         01/13/1987              */
/*              Coded       by  I.Iwata         01/13/1987              */
/*                                                                      */
/*      (C) Copyright 1987-1991 ERGOSOFT Corp.                          */
/*      All Rights Reserved                                             */
/*                                                                      */
/*                         --- NOTE ---                                 */
/*                                                                      */
/*      THIS PROGRAM BELONGS TO ERGO SOFT CORP.  IT IS CONSIDERED A     */
/*      TRADE SECRET AND IS NOT TO BE DIVULGED OR USED BY PARTIES       */
/*      WHO HAVE NOT RECEIVED WRITTEN AUTHORIZATION FROM THE OWNER.     */
/*                                                                      */
/************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#ifdef  UNIX
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

#define memncmp(S,T,C)  strncmp(S,T,C)
#define GFILE    "dict88.dic"

extern int  egcerrno;
EGCFILE     gref;
UBYTE*      grtp;

UBYTE*  egcopengrmdic();
VOID    egcclosegrmdic();
WORD    egcgetopt();
WORD    egcload();
VOID    loadusage();

/*********************************************************/
NONREC
REG main(argc, argv)
    REG             argc;
    UBYTE          *argv[];
/*********************************************************/
{
    EGCFILE         loadfp = (EGCFILE)0;
    UBYTE           loadfile[DICNAMELEN];
    UBYTE           ct;
    UBYTE         **hoptext;
    DICM            loaddic;
    BOOL            nonload;
    WORD            retval;            /* return value */
    LONG            totalrec = 0;
    UWORD           norec = 0;
    EGCPART         partopt;
    unsigned int    major, minor, micro;
    LONG            verl;
    UBYTE           vers[EGCSYSVERLEN + 1];
    UBYTE           versbuf[DICNAMELEN];

#ifdef UNIX
    signal(SIGKILL, egcabort);
    signal(SIGTERM, egcabort);
    signal(SIGINT, egcabort);
#endif

    vers_ltoc(egcgetsysverno(), vers);
    sprintf(versbuf, "%d.%d.%d", vers[0], vers[1], vers[2]);
    fprintf(stdout, "\nERGOSOFT egcld version %s\n", versbuf);
    if (egcgetopt(argc, argv, &partopt, DIRECTORY) == ERROR)
        loadusage();
    if (egccreate(partopt.lfile, &loaddic) == ERCRE01) {
        fprintf(stderr, "egccreate error %d\n", egcerrno);
        goto err_exit;
    }
    if (egcopengrmdic() == NULL) {
        fprintf(stderr, "grm error %d\n", egcerrno);
        goto err_exit;
    }
    nonload = TRUE;
    hoptext = partopt.hext;
    /*@@@ Modified 01/01/1992 H.Yanata @@@
    for (ct = 0; ct < 44; ct++) {
    @@@*/
    for (ct = 0; ct < 45; ct++) {
        sprintf(loadfile, "%s%s%s.%s",
                partopt.dirs, DELIMITER, partopt.dirs, *hoptext++);
        if ((loadfp = egcfopen(loadfile, OPENMODE)) == (EGCFILE)NULL) {
            fprintf(stdout, " Cannot open file %s\n", loadfile);
            continue;
        }
        switch (retval = egcload(loadfp, partopt.egcpval.recval,
                                 &norec, &loaddic)) {
        case ERWRI01:               /* main dict write error */
        case ERWRI02:               /* yomi string error     */
        case ERWRI03:               /* yomi length error     */
        case ERWRI04:               /* kanji string error    */
        case ERWRI05:               /* kanji length error    */
        case ERWRI06:               /* hinshi code error     */
        case ERWRI12:               /* over homonyms error   */
        case ERWRI13:               /* over tan-kanji error  */
        case ERWRI14:               /* over segments error   */
        case ERWRI22:               /* main dict full error  */
        case ERWRI31:               /* flush error           */
        case ERWRI32:               /* skip data  error      */
            fprintf(stdout, "Error-No.%4x file %s\n", retval, loadfile);
            fprintf(stdout, "load.lrcnt=%d\n", loaddic.lrcnt);
            fprintf(stdout, "load.homcnt=%d\n", loaddic.homcnt);
            break;
        }
        nonload = FALSE;
        if (loadfp) {
            if (egcfclose(loadfp) == ERROR)
                goto err_exit;
        }
        fprintf(stdout, "Number of records %5d: File %-s\n"
                ,norec, loadfile);
        totalrec += (LONG) norec;
    }
    if (nonload == TRUE) {
        fprintf(stderr, "\nDirectory is empty!!\n");
        goto err_exit;
    }
    if (egceof(&loaddic) != NORMAL) {
        fprintf(stderr, "\negceof error %d\n", egcerrno);
        goto err_exit;
    }
    egcclosegrmdic();
    if (egcndic(partopt.lfile, partopt.ufile, 16 * 1024, 4) != NORMAL) {
        fprintf(stderr, "\negcndic error %d\n", egcerrno);
        goto err_exit;
    }
    fprintf(stdout, "\nTotal of records %6ld\n", totalrec);
    fprintf(stdout, "Input Management version\n");
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
    if (egcsetmaindicver(partopt.lfile, verl) == -1L)
        fprintf(stderr, "Can't Set Maindict-version\n");
    if (egcsetuserdicver(partopt.ufile, verl) == -1L)
        fprintf(stderr, "Can't set Userdict-version\n");

    return (NORMAL);

err_exit:
    if (loadfp)
        egcfclose(loadfp);
    exit(1);
}                                      /* end of main */
/*PAGE*/
/*********************************************************/
NONREC
VOID loadusage()
/*********************************************************/
{
    fprintf(stdout, "Usage: egcld <-1,2,3> [-d -r] input_directory\n");
    fprintf(stdout, " d -> Dest dictionary is egdicm.dic & egdicu.dic.\n");
    fprintf(stdout, " r -> File extentions is romaji.\n");
    fprintf(stdout, " 1 -> File type is 32 byte.\n");
    fprintf(stdout, " 2 -> File type is 48 byte.\n");
    fprintf(stdout, " 3 -> File type is 80 byte.\n");
    exit(1);
}

