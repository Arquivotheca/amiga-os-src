/************************************************************************/
/*                                                                      */
/*      egbhdfuc.h : functions of egconvert for HDML                    */
/*                                                                      */
/*                      Version 1.1                                     */
/*                                                                      */
/*              Designed    by  I.Iwata         01/29/1987              */
/*              Coded       by  I.Iwata         01/29/1987              */
/*              Modified    by  xxxxx           xx/xx/xxxx              */
/*                                                                      */
/*      (C) Copyright 1985-87 ERGOSOFT Corp.                            */
/*      All Rights Reserved                                             */
/*                                                                      */
/*                          --- NOTE ---                                */
/*                                                                      */
/*      THIS PROGRAM BELONGS TO ERGO SOFT CORP.  IT IS CONSIDERED A     */
/*      TRADE SECRET AND IS NOT TO BE DIVULGED OR USED BY PARTIES       */
/*      WHO HAVE NOT RECEIVED WRITTEN AUTHORIZATION FROM THE OWNER.     */
/*                                                                      */
/************************************************************************/
/*------------------------------------------------------------------*****/
/*  included functions                                                  */
/*                                                                      */
/*      1  st=egcinit();                                                */
/*      2  st=egctxcvt(src,srcl,dst,dstl,sep,sepl,fixl,cmode);          */
/*      3  st=egcclcvt(src,srcl,dstr,dcep,dlid,dnbr,cmode);             */
/*      4  st=egclearn(dcid);                                           */
/*      5  st=egcwdadd(ystr,yl,kstr,kl,uh);                             */
/*      6  st=egcwddel(ystr,yl,kstr,kl);                                */
/*      7  st=egcrmcvt(src,srcl,dst,dstl,totl,cmode);                   */
/*      8  st=egcjacvt(src,srcl,dst,dstl);                              */
/*      9  st=egcajcvt(src,srcl,dst,dstl,cmode);                        */
/*     10  st=egcsjcvt(src,srcl,dst,dstl);                              */
/*     11  st=egcjscvt(src,srcl,dst,dstl);                              */
/*     12  st=egccllearn(kstr,kl1,kl2,ystr,yl1,yl2)                     */
/*     13  st=egcdhchk(pt,mode,check)                                   */
/*     14  st=egcmemmove(dst,src,count)                                 */
/*     15  st=egcmemncmp(pt1,pt2,count,cmp)                             */
/*                                                                      */
/************************************************************************/

/*
WORD   egcinit();
WORD   egctxcvt();
WORD   egcclcvt();
WORD   egclearn();
WORD   egcwdadd();
WORD   egcwddel();
WORD   egcrmcvt();
WORD   egcjacvt();
WORD   egcajcvt();
WORD   egcsjcvt();
WORD   egcjscvt();
WORD   egccllearn();
WORD   egcjscvt();
WORD   egcjscvt();
WORD   egcjscvt();
WORD   egcdhchk();
WORD   egcmemmove();
WORD   egcmemncmp();
*/

#include <stdio.h>
#include "egcdef.h"
#include "egcproto.h"

#ifdef BSD42
#define	memcpy(d,s,c)	bcopy((s),(d),(c))
#endif	/* BSD42 */
