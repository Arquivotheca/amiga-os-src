#ifndef lint
static char     rcsid[] = "$Id: egcmexp.c,v 3.4 91/06/20 21:45:22 katogi Exp Locker: katogi $";
#endif
/************************************************************************/
/*                                                                      */
/*      EGCMEXP : EGConvert Marge for Expert dictionary                 */
/*                                                                      */
/*              Designed    by  T.Harada        02/20/1990              */
/*              Coded       by  I.Harada        02/20/1990              */
/*              modified    by  H.Yanata        19/06/1991              */
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
#include <stdlib.h>
#include "edef.h"
#include "egcdef.h"
#include "egcenv.h"
#include "egcerr.h"
#include "egcpart.h"
#include "egcproto.h"
#include "egcldpro.h"
#include "egcudep.h"
#include "egckcnv.h"

WORD            egcmarge();
VOID            margeusage();
WORD            main();

/******************************************************************/
NONREC
WORD
main(argc, argv)
    REG             argc;
    UBYTE          *argv[];
/******************************************************************/
{
    UBYTE           ownopt[8][64];
    WORD            st;
    EGCPART         partopt;
    UBYTE           vers[EGCSYSVERLEN + 1];
    UBYTE           versbuf[DICNAMELEN];

    vers_ltoc(egcgetsysverno(), vers);
    sprintf(versbuf, "%d.%d.%d", vers[0], vers[1], vers[2]);
    fprintf(stdout, "\nERGOSOFT egcmexp version %s\n", versbuf);
    if (egcgetopt(argc, argv, &partopt, DIRECTORY) == ERROR
        || egcgetownopt(argc, argv, ownopt) == ERROR) {
        margeusage();
        return ERROR;
    }
    switch (st = egcmarge(partopt.lfile, partopt.ufile, ownopt[0])) {
    case ERMAR01:
    case ERMAR02:
    case ERMAR03:
    case ERMAR11:
    case ERMAR12:
    case ERMAR23:
        fprintf(stderr, "error status %04x\n", st);
        exit(1);
    }
    fprintf(stdout, "Completed\n");
    return (0);
}

VOID
margeusage()
{
    fprintf(stderr, "\nUsage: egcmexp [-d] outputfile \n");
    fprintf(stderr, "                 -d -> egdicm.dic & egdicu.dic\n");
    fprintf(stderr, "         outputfile -> Create expert dictionry name.\n");
}
