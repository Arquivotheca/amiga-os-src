/****************************************************************/
/*	EGZIPIO.C :	EGZIP ver1.0  io and allcation module.			*/
/*																*/
/*		designed by 			     .  .    					*/
/*		coded	 by   Koide        93.02.05 Version 1.0 		*/
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

#include "egzip.h"

#ifndef	UNIX
UBYTE *getZlTable(PZIPM pzipm, UWORD zlb);
UBYTE *getVlTable(PZIPM pzipm, UWORD vlb);
#else
UBYTE *getZlTable();
UBYTE *getVlTable();
#endif
/*************************************************/
/*** getZlTable() : Search Zlblock in ZlTable ****/
/*************************************************/
UBYTE *getZlTable(pzipm, zlb)
	PZIPM	pzipm;
	UWORD	zlb;
{
	UWORD	i, j;
	TBLCTL	*tcp;
	TBLCTL	tc;
	UBYTE	*tbl;

	tcp = pzipm->zlCash;
	for (i = 0; i < pzipm->zlCashCnt; i++) {
		if (tcp[i].cashNbr == zlb) {
			tc = tcp[i];
			for (j = i; j > 0; j--)
				tcp[j] = tcp[j - 1];
			tcp[0] = tc;
			return ((pzipm->zlTable + tc.blokNbr)->block);
		}
	}
	tc = tcp[pzipm->zlCashCnt - 1];
	for (j = pzipm->zlCashCnt - 1; j > 0; j--)
		tcp[j] = tcp[j - 1];
	tcp[0].cashNbr = zlb;
	tcp[0].blokNbr = tc.blokNbr;
	tbl = (pzipm->zlTable + tc.blokNbr)->block;

	fseek(pzipm->fp, pzipm->zlFPos + (zlb * BLOCKSIZE), SEEK_SET);
	fread(tbl, BLOCKSIZE, 1, pzipm->fp);

	return (tbl);
}

/* page */
/**************************************************/
/*** getVlTable() : Search Vlblock in VlTable *****/
/**************************************************/
UBYTE *getVlTable(pzipm, vlb)
	PZIPM	pzipm;
	UWORD	vlb;
{
	UWORD	i, j;
	TBLCTL	*tcp;
	TBLCTL	tc;
	UBYTE	*tbl;

	tcp = pzipm->vlCash;
	for (i = 0; i < pzipm->vlCashCnt; i++) {
		if (tcp[i].cashNbr == vlb) {
			tc = tcp[i];
			for (j = i; j > 0; j--)
				tcp[j] = tcp[j - 1];
			tcp[0] = tc;
			return ((pzipm->vlTable + tc.blokNbr)->block);
		}
	}
	tc = tcp[pzipm->vlCashCnt - 1];
	for (j = pzipm->vlCashCnt - 1; j > 0; j--)
		tcp[j] = tcp[j - 1];
	tcp[0].cashNbr = vlb;
	tcp[0].blokNbr = tc.blokNbr;
	tbl = (pzipm->vlTable + tc.blokNbr)->block;

	fseek(pzipm->fp, pzipm->vlFPos + (vlb * BLOCKSIZE), SEEK_SET);
	fread(tbl, BLOCKSIZE, 1, pzipm->fp);

	return (tbl);
}

