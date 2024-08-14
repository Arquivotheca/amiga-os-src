/***********************************************************************/
/**		EGZIP.H                                                       **/
/**			EGZIP ver1.0 Find Address and Zip module header.          **/
/**                                                                   **/
/**      	Copyright  1990 ERGOSOFT Corp.                           **/
/**	        All Rights Reserved.                                      **/
/**                                                                   **/
/**	        Coded by  T.Koide	1992.09.11                            **/
/**                                                                   **/
/**    THIS PROGRAM BELONGS TO ERGO SOFT CORP.  IT IS CONSIDERED A    **/
/**	   TRADE SECRET AND IS NOT TO BE DIVULGED OR USED BY PARTIES      **/
/**	   WHO HAVE NOT RECEIVED WRITTEN AUTHORIZATION FROM THE OWNER.    **/
/***********************************************************************/
typedef unsigned char	UBYTE;
typedef unsigned short	UWORD;
typedef unsigned long	ULONG;
typedef signed char		BYTE;
typedef signed short	WORD;
typedef signed long		LONG;
typedef void			VOID;

/****************************
**	initZipm()	error number
*****************************/
#define	ERROR01		-1				/* ZipDic open error */
#define ERROR02		-2				/* ZipDic read error */
#define ERROR03		-3				/* Memory allocate error */
#define NORMAL		0				/* Initialize Succsess */


#define NOFIND		0				/* Zip no find */
#define FIND		1				/* Zip find */
#define FINDNEXT	2				/* Zip find and next is alive */


#define BLOCKSIZE	1024			

#define ZLCASH		15				/* ZL block cash count */
#define VLCASH		20				/* VL block cash count */


/******************************
***** structure definiton.
*******************************/
typedef struct {
	UBYTE	block[BLOCKSIZE];		/* Block */
} BLOCK;

typedef struct {
	UWORD	cashNbr;				/* cashing number */
	UWORD	blokNbr;				/* block number */
} TBLCTL;

typedef struct {
	UWORD	zlCount;				/* ZL block count */
	UWORD	vlCount;				/* VL block count */

	UBYTE	*zlIndex;				/* ZL index */
	UBYTE	*vlIndex;				/* VL index */

	BLOCK	*zlTable;				/* ZL table */
	BLOCK	*vlTable;				/* VL table */

	UWORD	zlCashCnt;				/* ZLCASH count */
	UWORD	vlCashCnt;				/* VLCASH count */

	TBLCTL	*zlCash;				/* ZL table cashing control */
	TBLCTL	*vlCash;				/* VL table cashing control */

	ULONG	zlFPos;					/* ZL file position */
	ULONG	vlFPos;					/* VL file position */

	FILE	*fp;					/* EGZIPM.DIC File pointer */

	UWORD	repCount;				/* Repeat count */
} ZIPM, *PZIPM;


/********************************
**** FUNCTION PROTOTYPE
*********************************/
#ifndef	UNIX
WORD	initZipm(PZIPM , UBYTE *);
VOID 	termZipm(PZIPM );
WORD	cnvAddrToZip(PZIPM, UBYTE *, UBYTE *, ULONG *);
WORD 	cnvZipToAddr(PZIPM, ULONG, UBYTE *, UWORD *);
#else
WORD	initZipm();
VOID 	termZipm();
WORD	cnvAddrToZip();
WORD 	cnvZipToAddr();
#endif
