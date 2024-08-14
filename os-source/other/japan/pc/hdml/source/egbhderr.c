#ifndef lint
static char     rcsid[]="$Header$";
#endif
/************************************************************************/
/*                                                                      */
/*      EGBhderr.c : HDML fatal error handle procedure                  */
/*                                                                      */
/*              Designed    by  I.Iwata         10/10/1986              */
/*              Coded       by  I.Iwata         10/10/1986              */
/*              Modified    by  K.Nakamura      10/18/1988              */
/*                                                                      */
/*      (C) Copyright 1985-88 ERGOSOFT Corp.                            */
/*      All Rights Reserved                                             */
/*                                                                      */
/*                          --- NOTE ---                                */
/*                                                                      */
/*      THIS PROGRAM BELONGS TO ERGO SOFT CORP.  IT IS CONSIDERED A     */
/*      TRADE SECRET AND IS NOT TO BE DIVULGED OR USED BY PARTIES       */
/*      WHO HAVE NOT RECEIVED WRITTEN AUTHORIZATION FROM THE OWNER.     */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*      include subrutins                                               */
/*                                                                      */
/*      1  UWORD  edmerr(error)                                         */
/*                                                                      */
/************************************************************************/
#include  "edef.h"
#include  "egbhdrng.h"
#include  "egbhdfuc.h"
#include  "egckcnv.h"
#include  "hdmlcvt.h"
#include  "egberr.h"

#define THROUGH 0
#define ABORT   1

#ifdef  DOS_EGB

/*
*** Error Message Definitions
*/
#define MSG1    26
#define MSG2    24
#define MSG3    28
#define MSG4    28

static UBYTE    msg1[MSG1] = {"fBXPbg*+ ]V~"};
static UBYTE    msg2[MSG2] = {"+t@C* h\9q"};
static UBYTE    msg3[MSG3] = {" E    Igp5=+Fa"  \7 "};
static UBYTE    msg4[MSG4] = {"[U[+TCY*s3E7"};

/*
*** Function Value Definitions
*/

#define     SMDAPDISP   0x0400
#define     SMDMKO      0x0200
#define     SMDSTX      0x0100

extern UWORD    wMkApDisp;
extern UWORD    wErorrSt;

#endif
/*PAGE*/

/*------------------------------------------------------------------*/
/*  1   hdmerr : HDML error handler                                 */
/*          f/val = 0 : through                                     */
/*                  1 : abort                                       */
/*                 -1 : retry                                       */
/*------------------------------------------------------------------*/
WORD 
hdmerr(error)
    WORD            error;             /* error status         */
{
#ifdef  DOS_EGB

    UBYTE          *msg;               /* -> error message */
    UWORD           msgl;              /* error message length */
    UWORD           row;
    UWORD           col;
    UWORD           cmode;
    UWORD           maxink;
    UWORD           systemline;

    if (wMkApDisp & SMDAPDISP) {
        if (wErorrSt == 0)
            wErorrSt = error;
        if (error >= THROUGH)
            return (error);
        else
            return (ABORT);
    }
    else {
        if (error >= THROUGH)
            return (error);
        bssspc(&row, &col, &cmode, &maxink);
        systemline = row + 1;
        if (error == ERRPR) {
            msg = msg1;
            msgl = MSG1;
        }
        else if ((error == ERRNR) || (error == ERROP)) {
            msg = msg2;
            msgl = MSG2;
        }
        else if (error == ERRDD) {
            msg = msg3;
            msgl = MSG3;
        }
        else if (error == ERRUS) {
            bsssys(1);
            msg = msg4;
            msgl = MSG4;
        }
        srsys(0, systemline);
        erasesys(1, 80, systemline);
        bsspstr(systemline, 40, msgl, msg, 0x0400);
        chrwait();
        srsys(1, systemline);
        if (error == ERRUS)
            erasesys(1, 80, systemline);
        return (ABORT);
    }
#else
    return (THROUGH);
#endif

}
