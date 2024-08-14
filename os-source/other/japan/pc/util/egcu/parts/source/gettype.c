#ifndef lint
static char     rcsid[] = "$Id: gettype.c,v 3.4 91/06/20 20:33:46 katogi Exp Locker: katogi $";
#endif
/************************************************************************/
/*                                                                      */
/*      EGCTTYPE : EGConvert get TTYPE                                  */
/*                                                                      */
/*              Designed    by  Y.Katogi        09/03/1990              */
/*              Coded       by  Y.Katogi        09/03/1990              */
/*                                                                      */
/*      (C) Copyright 1990-1991 ERGOSOFT Corp.                          */
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
#include "edef.h"
#include "egcenv.h"
#include "egcerr.h"
#include "egcdef.h"
#include "egckcnv.h"
#include "egcexp.h"
#include "egcudep.h"
#include "egcdict.h"
#include "egcpart.h"

extern WORD     egcsecvt();
extern WORD     egcsjcvt();
extern WORD     nullcvt();

PTRWFUNC
egcttype()
{
    UBYTE          *env, *getenv();

    env = getenv("TTYPE");
    if (env == NULL) {
        fprintf(stderr, "TTYPE is unknow, set TTYPE\n");
        return ((PTRWFUNC) NULL);
    }
    if (!strcmp(env, "euc")) {
        fprintf(stdout, "Terminal is EUC\n");
        (VOID) ecodeset((WORD) CVT_EUC);
        return ((PTRWFUNC) egcsecvt);
    }
    else if (!strcmp(env, "jis")) {
        fprintf(stdout, "Terminal is JIS\n");
        (VOID) ecodeset((WORD) CVT_JIS);
        return ((PTRWFUNC) egcsjcvt);
    }
    else if (!strcmp(env, "mskanji")) {
        fprintf(stdout, "Terminal is Shift-JIS\n");
        (VOID) ecodeset((WORD) CVT_SJIS);
        return ((PTRWFUNC) nullcvt);
    }
    else if (!strcmp(env, "sjis")) {
        fprintf(stdout, "Terminal is Shift-JIS\n");
        (VOID) ecodeset((WORD) CVT_SJIS);
        return ((PTRWFUNC) nullcvt);
    }
    return ((PTRWFUNC) NULL);
}
