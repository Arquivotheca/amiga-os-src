/************************************************************************/
/*                                                                      */
/*      egberr.h : error definition table for HDML                      */
/*                                                                      */
/*                      Version 1.1                                     */
/*                                                                      */
/*              Designed    by  I.Iwata         01/01/1987              */
/*              Coded       by  I.Iwata         01/01/1987              */
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

/*   Disk Err                     */
/*                                */
/* - 1 to - 9 : phisical error    */
/*  -1 : write protected          */
/*  -2 : drive not ready          */
/*  -3 : open error               */
/* -10 to -99 : logical  error    */
/*  -20 : function code error     */
/*  -21 : parm length error       */
/*  -22 : diffrence dict error    */
/*  -23 : user dict size error    */
/*  -24 : user dict full error    */


#define   ERRPR          -1     /*  -1 : write protected      */ 
#define   ERRNR          -2     /*  -2 : drive not ready      */  
#define   ERROP          -3     /*  -3 : open error           */ 
#define   ERRFC          -20    /*  -20 : function code error     */ 
#define   ERRSL          -21    /*  -21 : parm length error       */
#define   ERRDD          -22    /*  -22 : diffrence dict error    */
#define   ERRUS          -23    /*  -23 : user dict size error    */
#define   ERRDFUL        -24    /*  -24 : user dict full error    */

/*   HDML Err                 */

#define     LIMITOVER           10
#define     NOTCVTCHRTYPE       11
#define     NOTROMAJICVT        12
#define     NOTCURRENTPHRASE    13
#define     NOCHARCTER          14
#define     SUBCMDERR           15
#define     MOVERANGEOVER       16
#define     DISKERR             17
#define     NOHOMONYM           18
#define     NOAGEHOMONYM        19
#define     NOCHCVT             20
#define     NOELMAGE            21
#define     CVTMODEERR          22
#define     NOCVTMODE           23
#define     CMDERR              24
#define     NOADDWORD           25
#define     DICFULLERR          26

