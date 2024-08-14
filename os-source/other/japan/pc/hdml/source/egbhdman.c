#ifndef lint
static char     rcsid[]="$Header$";
#endif
/************************************************************************/
/*                                                                      */
/*      egbhdman.c : main procedure for HDML                            */
/*                                                                      */
/*              Designed    by  I.Iwata         07/15/1986              */
/*              Coded       by  I.Iwata         12/02/1986              */
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
/*                                                                      */
/*  include subrutins                                                   */
/*                                                                      */
/*  1 UWORD hdml(cb)                                                    */
/*                                                                      */
/************************************************************************/
#include  "edef.h"
#include  "egbhdrng.h"
#include  "egbhdfuc.h"
#include  "egckcnv.h"
#include  "hdmlcvt.h"
#include  "egberr.h"
#include  "egcerr.h"
#ifdef UNIX
#include  "egbhdml.h"
#endif	/* UNIX */

#define egccep(P)       P->acbebp+(P->acbcpn-1)*3

/*****************************************************************/
/*                                                               */
/*  1  UWORD hdml(cb)                                            */
/*                                                               */
/*****************************************************************/
#ifdef UNIX_EGK
UWORD
hdml(cb, egb_hdml)
	ACB     cb;
	EGB_HDML*	egb_hdml;
#else
UWORD  hdml(cb)
ACB     cb;
#endif	/* UNIX_EGK */
{
    UWORD   st=0;     /* status                  */
    UWORD   cst;      /* status                  */

    cb->acbrsp = 0;
    if (cb->acbcmd == CLEAR) {
#ifdef UNIX_EGK
        st = hdmlclear(cb, egb_hdml);
#else
        st = hdmlclear(cb);
#endif /* UNIX_EGK */
        cb->acbsmd |= CSMCVT;
        cb->acbsmd &= ~CSMCEL;
        cb->acbrsp = st;
        return (NORMAL);
    }
    if (*(egccep(cb)) == 0)
        cb->acbsmd &= ~CSMCEL;
    else
        cb->acbsmd |=  CSMCEL;
    if (cb->acbcmd == CMOVE) {
        if (!(cb->acbsmd & CSMCEL)) {
            st = movchar(cb->acbdir,cb);
                if (st >= ERROR)
                    st = movphr(cb->acbdir,cb);
        }
        else {
            if (cb->acbsmd & CSMCVT) {
                st = movphr(cb->acbdir,cb);
            }
            else {
                st = movhom(cb->acbdir,cb);
            }
        }
    }
    else if (cb->acbcmd == RVCVT) {
        if ((cb->acbdir == DRLEFT) || (cb->acbdir == DRRIGHT)) {
            if (!(cb->acbsmd & CSMCEL))
                st = rvcvt(cb->acbdir,cb);
            else
                st = CVTMODEERR;
        }
        else {
            if (cb->acbsmd & CSMCEL)
                st = rvcvt(cb->acbdir,cb);
            else
                st = CVTMODEERR;
        }
    }
    else if (cb->acbcmd == TXCVT) {
        if (!(cb->acbsmd & CSMCEL)) {
            if ((st = cvttext(cb->acbdir,cb)) == NORMAL)
                cb->acbsmd |= CSMCVT;
        }
        else {
            if (cb->acbsmd & CSMCVT) {
                if ((st = readhom(cb)) == NORMAL)
                    cb->acbsmd &= ~CSMCVT;
            }
            else {
                st = NOCVTMODE;
            }
        }
    }
    else if (cb->acbcmd == CHCVT) {
        st = crrphr(cb->acbdir,cb);
    }
    else if (cb->acbcmd == PHFIX) {
        if ((st=fixhom(cb->acbdir,cb)) == NORMAL)
            cb->acbsmd |= CSMCVT;
        }
        else if (cb->acbcmd == AGEHOM) {
            if (cb->acbsmd&CSMAGE)
                st = agehom(cb);
        }
        else if (cb->acbcmd == CDCVT) {
            st = cdcvt(cb->acbdir,cb);
        }
        else if (cb->acbcmd == CANHOM) {
            if ((st = canhom(cb)) == NORMAL)
                cb->acbsmd |= CSMCVT;
        }
    else if (cb->acbcmd == NEXTHOM) {
        if ((st = readhom(cb)) == NORMAL) {
            cst = curhomchk(cb);
            if (cst)
                st = movhom(DRRIGHT,cb);
            st = fixhom((UBYTE)0,cb);
        }
    }
    else if (cb->acbcmd == PREVHOM) {
        if ((st = readhom(cb)) == NORMAL) {
            st = movhom(DRLEFT,cb);
            st = fixhom((UBYTE)0,cb);
        }
    }
    else if (cb->acbcmd == CLLEARN) {
        st = cllreq(cb);
    }
    else if (cb->acbcmd == ADDDICKANA) {
        st = addkana(cb);
    }
    else {
        st = CMDERR;
    }
    if (*(egccep(cb)) == 0)
        cb->acbsmd &= ~CSMCEL;
    else
        cb->acbsmd |= CSMCEL;
    if (st != 0) {
        cb->acbrsp = st;
        return (ERROR);
    }
    else
        return (NORMAL);
}
