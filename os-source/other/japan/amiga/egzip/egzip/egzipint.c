/****************************************************************/
/*	EGZIPIO.C :	EGZIP ver1.0  io and mamory allcation module.	*/
/*																*/
/*		designed by 			     .  .    					*/
/*		coded	 by   Koide        92.09.10 Version 1.0 		*/
/*		modified by                  .  .	                	*/
/*																*/
/*	(C) Copyright 1990 ERGOSOFT Corp. All Rights Reserved.		*/
/*																*/
/*							--- NOTE ---						*/
/*																*/
/*	THIS PROGRAM BELONGS TO ERGOSOFT CORP.	IT IS CONSIDERED A	*/
/*	TRADE SECRET AND IS NOT TO BE DIVULGED OR USED BY PARTIES	*/
/*	WHO HAVE NOT RECEIVED WRITTEN AUTHORIZATION FROM THE OWNER. */
/****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "egzip.h"

#ifndef	UNIX
WORD		initZipm(PZIPM , UBYTE *);
VOID 	termZipm(PZIPM );
#else
WORD		initZipm();
VOID 	termZipm();
#endif

/***********************************/
/*** initZipm() : Intialize ********/
/***********************************/
WORD		initZipm(pzipm, egzipm)
	PZIPM	pzipm;
	UBYTE*	egzipm;
{
	FILE	*fp;
	UBYTE	gx[BLOCKSIZE];
	UWORD	i;

	if ((fp = fopen(egzipm, "rb")) == NULL) {
		return (ERROR01);
	}

	fread(gx, BLOCKSIZE, 1, fp);
	if (gx[5] != 0) {
		fclose(fp);
		return (ERROR02);
	}

	pzipm->zlCount = (WORD)gx[0];
	pzipm->vlCount = (WORD)gx[1];
	pzipm->zlIndex = malloc(300);
	pzipm->vlIndex = malloc(300);
	pzipm->zlTable = (BLOCK *)malloc(ZLCASH * BLOCKSIZE);
	pzipm->vlTable = (BLOCK *)malloc(VLCASH * BLOCKSIZE);
	pzipm->zlCashCnt = ZLCASH;
	pzipm->vlCashCnt = VLCASH;
	pzipm->zlCash  = (TBLCTL *)malloc(ZLCASH * sizeof(TBLCTL));
	pzipm->vlCash  = (TBLCTL *)malloc(VLCASH * sizeof(TBLCTL));
	pzipm->zlFPos  =  1                   * BLOCKSIZE;
	pzipm->vlFPos  = (1 + pzipm->zlCount) * BLOCKSIZE;
	pzipm->fp      = fp;
	pzipm->repCount = 0;

	if (pzipm->zlIndex == NULL ||
	    pzipm->vlIndex == NULL ||
	    pzipm->zlTable == NULL ||
	    pzipm->vlTable == NULL ||
	    pzipm->zlCash  == NULL ||
	    pzipm->vlCash  == NULL) {
		fclose(fp);
		if (pzipm->zlIndex) free(pzipm->zlIndex);
		if (pzipm->vlIndex) free(pzipm->vlIndex);
		if (pzipm->zlTable) free(pzipm->zlTable);
		if (pzipm->vlTable) free(pzipm->vlTable);
		if (pzipm->zlCash ) free(pzipm->zlCash );
		if (pzipm->vlCash ) free(pzipm->vlCash );
		return (ERROR03);
	}

	memcpy(pzipm->zlIndex, &gx[  2], 300);
	memcpy(pzipm->vlIndex, &gx[302], 300);

	for (i = 0; i < ZLCASH; i++) {
		pzipm->zlCash[i].cashNbr = 0xffff;
		pzipm->zlCash[i].blokNbr = i;
	}
	for (i = 0; i < VLCASH; i++) {
		pzipm->vlCash[i].cashNbr = 0xffff;
		pzipm->vlCash[i].blokNbr = i;
	}
	

	return (NORMAL);
}

/* page */
/****************************************************/
/*** termZipm() : Free WorkMemory and Close File ****/
/****************************************************/
VOID	termZipm(pzipm)
	PZIPM	pzipm;
{
	fclose(pzipm->fp);
	pzipm->zlCount = 0;
	pzipm->vlCount = 0;
	free(pzipm->zlIndex);
	free(pzipm->vlIndex);
	free(pzipm->zlTable);
	free(pzipm->vlTable);
	free(pzipm->zlCash );
	free(pzipm->vlCash );
	pzipm->zlFPos   = 0;
	pzipm->vlFPos   = 0;
	pzipm->repCount = 0;
}

/* page */
