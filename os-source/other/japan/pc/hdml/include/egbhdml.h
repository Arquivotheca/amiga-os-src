/*
 * $Header$
 */

/*****************************************************************/
/*                                                               */
/*     egbhdml.h                                                 */
/*                                                               */
/*      HDML buffer                                              */
/*                                                               */
/*     (C) Copyright 1985,1986 Ergo Soft Corp.                   */
/*     All Rights Reserved					 */
/*								 */
/*                    --- NOTE ---				 */
/*								 */
/*  THIS PROGRAM BELONGS TO ERGO SOFT CORP.  IT IS CONSIDERED A  */
/*  TRADE SECRET AND IS NOT TO BE DIVULGED OR USED BY PARTIES    */
/*  WHO HAVE NOT RECEIVED WRITTEN AUTHORIZATION FROM THE OWNER.  */
/*                                                               */
/*****************************************************************/

#ifndef	Egbhdml_Inc_Defined

typedef
struct tagEGB_HDML {
	UWORD    elbff[3*(MAXCLN+20)];   /* element buffer       */
	UBYTE    scbff[MAXCHN];          /* screen buffer        */
	UBYTE    orbff[MAXCHN];          /* original buffer      */
	UBYTE    hpbff[MAXHMP];          /* homonym page buffer  */
	UBYTE    hebff[MAXHOM];          /* homonym element buf. */
	UBYTE    hmbff[MAXHML];          /* homonym buffer       */
	/*@@@ modify 03/18/87 @@@*/
	UBYTE    hymbff[MAXCL*2+1];      /* homonym yomi buffer  */
	WORDID   widbff[MAXHOM];         /* homonym word id. buf.*/
} EGB_HDML;

#define	Egbhdml_Inc_Defined
#endif	Egbhdml_Inc_Defined

