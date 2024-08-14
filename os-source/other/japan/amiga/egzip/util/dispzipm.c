/****************************************************************/
/*	DISPZIPM.C :	ERGOSOFT Display EGZIPM.DIC	utility			*/
/*																*/
/*		designed by M.Yamashita    90.07.03 					*/
/*		coded	 by   Harada       90.07.04 Version 1.0 		*/
/*		modified by M.Yamashita    90.12.26	Ver 1.1 small model	*/
/*      Update   by T.KOIDE        93.02.01                     */
/*																*/
/*	(C) Copyright 1993 ERGOSOFT Corp. All Rights Reserved.		*/
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

typedef char			BYTE;
typedef unsigned char	UBYTE;
typedef unsigned short	UWORD;
typedef unsigned long	ULONG;

#define BLOCKSIZE	1024			/* Block size */

#define NOFIND		0				/* Zip no find */
#define FIND		1				/* Zip find */
#define FINDNEXT	2				/* Zip find and next is alive */

#define ZLCASH		10				/* ZL block cash count */
#define VLCASH		15				/* VL block cash count */


/* structure definiton. */

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

	BLOCK	*zlTable;				/* ZL table cashing */
	BLOCK	*vlTable;				/* VL table cashing */

	TBLCTL	*zlCash;				/* ZL table cashing control */
	TBLCTL	*vlCash;				/* VL table cashing control */

	ULONG	zlFPos;					/* ZL file position */
	ULONG	vlFPos;					/* VL file position */

	FILE	*fp;					/* EGZIPM.DIC File pointer */

	UWORD	repCount;				/* Repeat count */
} ZIPM, *PZIPM;


/* function prototype definiton. */

short int initZipm();
void termZipm();
short int getAddress();

void getVlName();
UWORD schZl();
UBYTE *getVid();
UBYTE *getZipNo();
ULONG makeZipNo();
UBYTE *getZlTable();
UBYTE *getVlTable();


/* main procedure. */

main(argc, argv)
	int argc;
	char *argv[];
{
	ZIPM	zipm;
	UBYTE	zipstr[64];
	UBYTE	addr[128];
	UWORD	len;
	ULONG	zipStart;
	ULONG	zipEnd;
	ULONG	zipNo;
	short int		rs;

	printf("ERGOSOFT Display EGZIPM.DIC utility Version 1.0\n");
	printf("(C)Copyright 1990 ERGOSOFT Corp. All rights reserved.\n\n");

	if (argc <= 1) {
		printf("usage: DISPZIPM egzipm.dic [zip code] [end zip code]\n");
		return (0);
	}
	else {
		strcpy(zipstr, argv[1]);
		if (initZipm(&zipm, zipstr)) {
			exit(1);
		}
	}

	zipStart = 0L;
	zipEnd   = 99999L;
	if (argc > 2) {
		strcpy(zipstr, argv[2]);
		zipStart = makeZipNo(zipstr);
		zipEnd = zipStart;
	}
	if (argc > 3) {
		strcpy(zipstr, argv[3]);
		zipEnd = makeZipNo(zipstr);
	}
	if (zipStart >= 100000L || zipEnd >= 100000L) {
		printf("îìééùXòöìåé¬òsÉéé.\n");
		exit(1);
	}

	for (zipNo = zipStart; zipNo <= zipEnd; zipNo++) {
		do {
			rs = getAddress(&zipm, zipNo, addr, &len);
			if (rs != NOFIND) {
				addr[len] = 0;
				printf("üº%03ld-%02ld  %s \n", zipNo / 100, zipNo % 100, addr);
			}
		} while (rs == FINDNEXT);
	}

	termZipm(&zipm);
	return (0);
}


short int initZipm(pZipm, egzipm)
	PZIPM pZipm;
	UBYTE *egzipm;
{
	FILE	*fp;
	UBYTE	gx[BLOCKSIZE];
	UWORD	i;

	if ((fp = fopen(egzipm, "rb")) == NULL) {
		printf("%séâIü[âvâôéé½ééé±.\n", egzipm);
		return (-1);
	}

	fread(gx, BLOCKSIZE, 1, fp);
	if (gx[5] != 0) {
		fclose(fp);
		printf("%séEGZIPM.DICéâtâ@âCâïî`Ä«éééáéééé±.\n", egzipm);
		return (-1);
	}

	pZipm->zlCount = (UWORD)gx[0];
	pZipm->vlCount = (UWORD)gx[1];
	pZipm->zlIndex = malloc(300);
	pZipm->vlIndex = malloc(300);
	pZipm->zlTable = (BLOCK *)malloc(ZLCASH * BLOCKSIZE);
	pZipm->vlTable = (BLOCK *)malloc(VLCASH * BLOCKSIZE);
	pZipm->zlCash  = (TBLCTL *)malloc(ZLCASH * sizeof(TBLCTL));
	pZipm->vlCash  = (TBLCTL *)malloc(VLCASH * sizeof(TBLCTL));
	pZipm->zlFPos  =  1                   * BLOCKSIZE;
	pZipm->vlFPos  = (1 + pZipm->zlCount) * BLOCKSIZE;
	pZipm->fp      = fp;
	pZipm->repCount = 0;

	if (pZipm->zlIndex == NULL ||
	    pZipm->vlIndex == NULL ||
	    pZipm->zlTable == NULL ||
	    pZipm->vlTable == NULL ||
	    pZipm->zlCash  == NULL ||
	    pZipm->vlCash  == NULL) {
		fclose(fp);
		if (pZipm->zlIndex) free(pZipm->zlIndex);
		if (pZipm->vlIndex) free(pZipm->vlIndex);
		if (pZipm->zlTable) free(pZipm->zlTable);
		if (pZipm->vlTable) free(pZipm->vlTable);
		if (pZipm->zlCash ) free(pZipm->zlCash );
		if (pZipm->vlCash ) free(pZipm->vlCash );
		printf("memory alloc error !!\n");
		return (-1);
	}

	memcpy(pZipm->zlIndex, &gx[  2], 300);
	memcpy(pZipm->vlIndex, &gx[302], 300);

	for (i = 0; i < ZLCASH; i++) {
		pZipm->zlCash[i].cashNbr = 0xffff;
		pZipm->zlCash[i].blokNbr = i;
	}
	for (i = 0; i < VLCASH; i++) {
		pZipm->vlCash[i].cashNbr = 0xffff;
		pZipm->vlCash[i].blokNbr = i;
	}

	return (0);
}


void termZipm(pZipm)
PZIPM pZipm;
{
	fclose(pZipm->fp);
	pZipm->zlCount = 0;
	pZipm->vlCount = 0;
	free(pZipm->zlIndex);
	free(pZipm->vlIndex);
	free(pZipm->zlTable);
	free(pZipm->vlTable);
	free(pZipm->zlCash );
	free(pZipm->vlCash );
	pZipm->zlFPos   = 0;
	pZipm->vlFPos   = 0;
	pZipm->repCount = 0;
}


short getAddress(pZipm, zipNo, addr, len)
	PZIPM pZipm;
	ULONG zipNo; 
	UBYTE *addr; 
	UWORD *len;
{
	static BYTE *prr[15] = { "", "ïÄäÅæöá", "ÄûûâZâôâ^ü[", "ï", "ôs", "ô", 
	                  "ò{", "îº", "Äs", "îS", "ÄxÆí", "ïµ", "Æ¼", "æ", "ïµ" };

	UBYTE	*idx;
	UBYTE	*tbl;
	UWORD	zlc;
	UWORD	zlb;
	UWORD	zlp;
	UBYTE	str[32];
	ULONG	zipn;
	UWORD	rid;
	UWORD	rep;
	UWORD	vid;
	UWORD	rr;
	UWORD	ln;
	UWORD	i, l;
	UBYTE	*p;

	idx = pZipm->zlIndex;
	zlc = pZipm->zlCount;
	zlb = 0;
	for (i = 0; i < zlc; i++) {
		idx = getZipNo(idx, &zipn, &rid);
		if (zipNo < zipn) {
			break;
		}
		zlb = *idx++;
		if (zipNo == zipn) {
			break;
		}
	}

	tbl = getZlTable(pZipm, zlb);
	zlp = schZl(tbl, 0, zipNo);
	rep = pZipm->repCount;
	while (rep--) {
		getZipNo(tbl + zlp, &zipn, &rid);
		zlp += (3 + rid*2);
		zlp = schZl(tbl, zlp, zipNo);
		if (zlp == 0) {
			pZipm->repCount = 0;
			return (NOFIND);
		}
	}
	getZipNo(tbl + zlp, &zipn, &rid);
	if (zipNo != zipn) {
		pZipm->repCount = 0;
		return (NOFIND);
	}
	zlp += 3;
	l = 0;
	p = addr;
	for (i = 0; i < rid; i++) {
		getVid(tbl + zlp, &vid, &rr);
		zlp += 2;
		getVlName(pZipm, vid, str, &ln);
		memcpy(p, str, ln);
		p += ln;
		l += ln;
		ln = strlen(prr[rr]);
		memcpy(p, prr[rr], ln);
		p += ln;
		l += ln;
	}
	*len = l;

	if (zlp + 5 < BLOCKSIZE) {
		getZipNo(tbl + zlp, &zipn, &rid);
		if (zipNo == zipn) {
			pZipm->repCount++;
			return (FINDNEXT);
		}
	}

	pZipm->repCount = 0;
	return (FIND);
}


void getVlName(pZipm, vid, str, len)
	PZIPM pZipm;
	UWORD vid;
	UBYTE *str;
	UWORD *len;
{
	UBYTE	*idx;
	UBYTE	*tbl;
	UWORD	vlc;
	UWORD	vlb;
	UWORD	id;
	UWORD	ln;
	UWORD	i;

	idx = pZipm->vlIndex;
	vlc = pZipm->vlCount;
	for (i = 0; i < vlc; i++) {
		idx = getVid(idx, &id, &ln);
		ln *= 2;
		if (vid < id) {
			break;
		}
		else if (vid == id) {
			memcpy(str, idx, ln);
			*len = ln;
			return;
		}
		idx += ln;
		vlb = *idx++;
	}

	tbl = getVlTable(pZipm, vlb);
	ln = 0;
	do {
		tbl += ln;
		tbl = getVid(tbl, &id, &ln);
		ln *= 2;
	} while (vid != id);

	memcpy(str, tbl, ln);
	*len = ln;
}


UWORD schZl(zlb, zlp, zipNo)
	UBYTE *zlb;
	UWORD zlp;
	ULONG zipNo;
{
	ULONG	zn;
	UWORD	rid;

	while ((zlp + 3) < BLOCKSIZE) {
		getZipNo(zlb + zlp, &zn, &rid);
		if (zn == zipNo) {
			return (zlp);
		}
		else if (zn > zipNo || zn == 0L) {
			break;
		}
		zlp += (3 + rid*2);
	}
	return (0);
}


UBYTE *getVid(str, vid, len)
	UBYTE *str; 
	UWORD *vid; 
	UWORD *len;
{
	UWORD	i, l;

	i = (UWORD)(*str++ << 4);
	l = (UWORD)(*str & 0x0F);
	i += (UWORD)(*str++ >> 4);

	*vid = i;
	*len = l;

	return (str);
}


UBYTE *getZipNo(str, zipNo, rid)
	UBYTE *str; 
	ULONG *zipNo;
	UWORD *rid;
{
	ULONG	zn;

	*rid = (UWORD)(*str >> 4);
	zn =  (ULONG)(*str++ & 0x0F) << 16;
	zn += (ULONG)(*str++) << 8;
	zn += (ULONG)(*str++);
	*zipNo = zn;

	return (str);
}


ULONG makeZipNo(str)
	UBYTE	*str;
{
	ULONG	zipNo;
	UWORD	len;
	UWORD	i;

	zipNo = 0L;
	len = strlen(str);
	for (i = len; i < 5; i++) {
		str[i] = '0';
	}
	for (i = 0; i < 5; i++) {
		zipNo = (zipNo * 10) + (ULONG)(str[i] - '0');
	}
	return (zipNo);
}


UBYTE *getZlTable(pZipm, zlb)
	PZIPM pZipm;
	UWORD zlb;
{
	UWORD	i, j;
	TBLCTL	*tcp;
	TBLCTL	tc;
	UBYTE	*tbl;

	tcp = pZipm->zlCash;
	for (i = 0; i < ZLCASH; i++) {
		if (tcp[i].cashNbr == zlb) {
			tc = tcp[i];
			for (j = i; j > 0; j--)
				tcp[j] = tcp[j - 1];
			tcp[0] = tc;
			return ((pZipm->zlTable + tc.blokNbr)->block);
		}
	}
	tc = tcp[ZLCASH - 1];
	for (j = ZLCASH - 1; j > 0; j--)
		tcp[j] = tcp[j - 1];
	tcp[0].cashNbr = zlb;
	tcp[0].blokNbr = tc.blokNbr;
	tbl = (pZipm->zlTable + tc.blokNbr)->block;

	fseek(pZipm->fp, pZipm->zlFPos + (zlb * BLOCKSIZE), SEEK_SET);
	fread(tbl, BLOCKSIZE, 1, pZipm->fp);

	return (tbl);
}


UBYTE *getVlTable(pZipm, vlb)
	PZIPM pZipm;
	UWORD vlb;
{
	UWORD	i, j;
	TBLCTL	*tcp;
	TBLCTL	tc;
	UBYTE	*tbl;

	tcp = pZipm->vlCash;
	for (i = 0; i < VLCASH; i++) {
		if (tcp[i].cashNbr == vlb) {
			tc = tcp[i];
			for (j = i; j > 0; j--)
				tcp[j] = tcp[j - 1];
			tcp[0] = tc;
			return ((pZipm->vlTable + tc.blokNbr)->block);
		}
	}
	tc = tcp[VLCASH - 1];
	for (j = VLCASH - 1; j > 0; j--)
		tcp[j] = tcp[j - 1];
	tcp[0].cashNbr = vlb;
	tcp[0].blokNbr = tc.blokNbr;
	tbl = (pZipm->vlTable + tc.blokNbr)->block;

	fseek(pZipm->fp, pZipm->vlFPos + (vlb * BLOCKSIZE), SEEK_SET);
	fread(tbl, BLOCKSIZE, 1, pZipm->fp);

	return (tbl);
}
