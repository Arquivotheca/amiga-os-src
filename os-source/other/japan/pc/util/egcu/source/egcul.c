#ifndef lint
static char     rcsid[] = "$Id: egcul.c,v 3.4 91/06/20 21:45:29 katogi Exp Locker: katogi $";
#endif
/************************************************************************/
/*                                                                      */
/*      EGCUL : EGConvert  dictionary UnLoad utility                    */
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
#include "edef.h"
#include "egcenv.h"
#include "egckcnv.h"
#include "egcpart.h"
#include "egcdef.h"
#include "egcproto.h"
#include "egcldpro.h"
#include "egcudep.h"

/* external functions */
#define PATHLEN 128
extern int      egcerrno;
extern int      egcverck;
extern int      egcsysck;

/* internal functions */
WORD            egcgetopt();
WORD            egcunload();
VOID            unloadusage();
/*PAGE*/

/*********************************************************/
/* main                                                  */
/*********************************************************/
REG
main(argc, argv)
    REG             argc;
    UBYTE          *argv[];
{
    EGCFILE         sref;
    UBYTE           unloadfile[PATHLEN];
    UBYTE         **hoptext;
    UBYTE           ct;
    REG             ret;
    WORD            unloaderr;
    LONG            totalrec = 0L;
    LONG            norec = 0L;
    EGCPART         partopt;
    UBYTE           vers[EGCSYSVERLEN + 1];
    UBYTE           versbuf[DICNAMELEN];
    extern UBYTE*   asciiext[];

    vers_ltoc(egcgetsysverno(), vers);
    sprintf(versbuf, "%d.%d.%d", vers[0], vers[1], vers[2]);
    fprintf(stdout, "\nERGOSOFT egcul version %s\n", versbuf);
    if (egcgetopt(argc, argv, &partopt, DIRECTORY) == ERROR)
        unloadusage();
    egcsetmaindic(partopt.lfile);
    egcsetuserdic(partopt.ufile);
    egcverck = egcsysck = FALSE;
    if (egcinit() != NORMAL) {
        fprintf(stderr, "EGConvert : connect error.\n");
        exit(0);
    }
    hoptext = partopt.hext;
    for (ct = 0; ct < 45; ct++) {
        sprintf(unloadfile, "%s%s%s.%s",
                partopt.dirs, DELIMITER, partopt.dirs, hoptext[ct]);
        if ((sref = egcfopen(unloadfile, CREATMODE)) == (EGCFILE)NULL) {
            fprintf(stderr, "Cannot unloadfile open error %s.\n", unloadfile);
            break;
        }
        norec = 0L;
        if ((unloaderr = egcunload(sref, partopt.egcpval.recval,
                        &norec, asciiext[ct], asciiext[ct + 1])) == ERROR) {
            fprintf(stderr, "Unload error : No. = %x", unloaderr);
            exit(1);
        }
        ret = egcfclose(sref);
        if (ret) {
            fprintf(stdout, "File close error : %s\n", unloadfile);
            exit(1);
        }
        fprintf(stdout, "File %s : Number of records %4ld\n"
                ,unloadfile, norec);
        totalrec += norec;
    }
    fprintf(stdout, "Total of records %5ld\n", totalrec);
    egcusoff();
    return (NORMAL);                /* end of main */
}

VOID
unloadusage()
{
    fprintf(stdout, "\nUsage: egcul <-1, 2, 3> [-d -r] output_directory\n");
    fprintf(stdout, " d -> Dictionary is egdicm.dic & egdicu.dic.\n");
    fprintf(stdout, " r -> Extentions is romaji.\n");
    fprintf(stdout, " 1 -> Sequencial file size is 32 byte.\n");
    fprintf(stdout, " 2 -> Sequencial file size is 48 byte\n");
    fprintf(stdout, " 3 -> Sequencial file size is 80 byte\n");
    exit(1);
}
