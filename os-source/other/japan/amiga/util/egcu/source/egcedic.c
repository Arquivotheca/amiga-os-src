#ifndef lint
static char     rcsid[] = "$Id: egcedic.c,v 3.4 91/06/20 21:45:09 katogi Exp Locker: katogi $";
#endif
/************************************************************************/
/*                                                                      */
/*      EGCEDIC : EGConvert Extend user dictionary utility              */
/*                                                                      */
/*              Designed    by  Y.Katogi        06/20/1990              */
/*              Coded       by  Y.Katogi        06/20/1990              */
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
#include "edef.h"
#include "egcenv.h"
#include "egcerr.h"
#include "egcdef.h"
#include "egckcnv.h"
#include "egcdict.h"
#include "egcproto.h"
#include "egcpart.h"
#include "egcldpro.h"
#include "egcudep.h"

#ifdef UNIX
#include <signal.h>
#endif
#ifdef DOS_EGC
#include <stdlib.h>
#endif

VOID            edicusage();

WORD
main(argc, argv)
    REG             argc;
    UBYTE         **argv;
{
    UBYTE           ownopt[8][64];
    UWORD           plo;
    UBYTE           slt;
    WORD            ret;
    EGCPART         partopt;
    UBYTE           vers[EGCSYSVERLEN + 1];
    UBYTE           versbuf[DICNAMELEN];

    vers_ltoc(egcgetsysverno(), vers);
    sprintf(versbuf, "%d.%d.%d", vers[0], vers[1], vers[2]);

#ifdef UNIX
    signal(SIGKILL, egcabort);
    signal(SIGTERM, egcabort);
    signal(SIGINT, egcabort);
#endif

    fprintf(stdout, "\nERGOSOFT egcedic version %s\n", versbuf);

    if (egcgetopt(argc, argv, &partopt, NODIRECTORY) == ERROR
        || egcgetownopt(argc, argv, ownopt) == ERROR) {
        edicusage();
        return ERROR;
    }

    plo = (UWORD)atoi(ownopt[0]);
    slt = (UBYTE)atoi(ownopt[1]);
    printf("Maximum additionary words : %d\n", plo * 250);
    printf("Maximum self learning no. : %d\n", slt);

    if ((plo == 4 || plo == 8 || plo == 12 || plo == 16 || plo == 32)
            && (4 <= slt && slt <= 64)) {
        plo *= 1024;
        if ((ret = egcndic(partopt.lfile, partopt.ufile, plo, slt)) == ERROR)
            fprintf(stderr, "Error !\n");
        else
            fprintf(stdout, "Complete !\n");
    }
    else {
        fprintf(stderr, "illegal parameter.\n");
        edicusage();
    }
    return NORMAL;
}

VOID
edicusage()
{
    fprintf(stdout, "\nUsage: egcedic [-d] plosize sltvalue\n");
    fprintf(stdout, "              -d  -> egdicmdic & egdicu.dic\n");
    fprintf(stdout, "         plosize  -> 4, 8, 12, 16, 32\n");
    fprintf(stdout, "         sltvalue -> 4...64 \n");
}
