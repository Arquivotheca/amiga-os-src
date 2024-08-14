/**********************************************************************/
/***																***/
/***	EGZipZtoA.c : Convert Zip Code to Address string module		***/
/***																***/
/**********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "egzip.h"

/***************************************/
/*** FUNCTION PROTO-TYPE DECLARATION ***/
/***************************************/

/* Local */
#ifndef	UNIX
VOID 	z2a_getVlName(PZIPM, UWORD, UBYTE *, UWORD *);
UWORD 	z2a_schZl(UBYTE *, UWORD, ULONG );
UBYTE 	*z2a_getVid(UBYTE *, UWORD *, UWORD *);
UBYTE 	*z2a_getZipNo(UBYTE *, ULONG *, UWORD *);
extern	UBYTE *getZlTable(PZIPM, UWORD);
extern	UBYTE *getVlTable(PZIPM, UWORD);
#else
VOID 	z2a_getVlName();
UWORD 	z2a_schZl);
UBYTE 	*z2a_getVid();
UBYTE 	*z2a_getZipNo();
extern	UBYTE *getZlTable();
extern	UBYTE *getVlTable();
#endif


/* page */
/********************************************************/
/*** cnvZipToAddr() : ZipCode to Address MAIN entry *****/
/********************************************************/
WORD cnvZipToAddr(pzipm, zipNo, addr, len)
	PZIPM	pzipm;
	ULONG	zipNo;
	UBYTE*	addr;
	UWORD*	len;
{
	static BYTE *prr[15] = { "", "ïÄäÅæöá", "ÄûûâZâôâ^ü[", "ï", "ôs", "ô",
							 "ò{", "îº", "Äs", "îS", "ÄxÆí", "ïµ", "Æ¼", "æ",
							 "ïµ" };

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

	idx = pzipm->zlIndex;
	zlc = pzipm->zlCount;
	zlb = 0;
	for (i = 0; i < zlc; i++) {
		idx = z2a_getZipNo(idx, &zipn, &rid);
		if (zipNo < zipn) {
			break;
		}
		zlb = *idx++;
		if (zipNo == zipn) {
			break;
		}
	}

	tbl = getZlTable(pzipm, zlb);
	zlp = z2a_schZl(tbl, 0, zipNo);
	rep = pzipm->repCount;
	while (rep--) {
		z2a_getZipNo(tbl + zlp, &zipn, &rid);
		zlp += (3 + rid*2);
		zlp = z2a_schZl(tbl, zlp, zipNo);
		if (zlp == 0) {
			pzipm->repCount = 0;
			return (NOFIND);
		}
	}
	z2a_getZipNo(tbl + zlp, &zipn, &rid);
	if (zipNo != zipn) {
		pzipm->repCount = 0;
		return (NOFIND);
	}
	zlp += 3;
	l = 0;
	p = addr;
	for (i = 0; i < rid; i++) {
		z2a_getVid(tbl + zlp, &vid, &rr);
		zlp += 2;
		z2a_getVlName(pzipm, vid, str, &ln);
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
		z2a_getZipNo(tbl + zlp, &zipn, &rid);
		if (zipNo == zipn) {
			pzipm->repCount++;
			return (FINDNEXT);
		}
	}

	pzipm->repCount = 0;
	return (FIND);
}

/* page */
/**************************************************************/
/*** z2a_getVlName() : Search VID and Make Address String *****/
/**************************************************************/
VOID z2a_getVlName(pzipm, vid, str, len)
	PZIPM	pzipm;
	UWORD	vid;
	UBYTE*	str;
	UWORD*	len;
{
	UBYTE	*idx;
	UBYTE	*tbl;
	UWORD	vlc;
	UWORD	vlb;
	UWORD	id;
	UWORD	ln;
	UWORD	i;

	idx = pzipm->vlIndex;
	vlc = pzipm->vlCount;
	for (i = 0; i < vlc; i++) {
		idx = z2a_getVid(idx, &id, &ln);
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

	tbl = getVlTable(pzipm, vlb);
	ln = 0;
	do {
		tbl += ln;
		tbl = z2a_getVid(tbl, &id, &ln);
		ln *= 2;
	} while (vid != id);

	memcpy(str, tbl, ln);
	*len = ln;
}

/* page */
/******************************************************/
/*** z2a_schZl() : Search ZipCode From Zlblock ********/
/******************************************************/
UWORD z2a_schZl(zlb, zlp, zipNo)
	UBYTE*	zlb;
	UWORD	zlp;
	ULONG	zipNo;
{
	ULONG	zn;
	UWORD	rid;

	while ((zlp + 3) < BLOCKSIZE) {
		z2a_getZipNo(zlb + zlp, &zn, &rid);
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

/* page */
/**********************************************************/
/*** z2a_getVid() : Make Vid and length From VlTable ******/
/**********************************************************/
UBYTE *z2a_getVid(str, vid, len)
	UBYTE*	str;
	UWORD*	vid;
	UWORD*	len;
{
	UWORD	i, l;

	i = (UWORD)(*str++) << 4;
	l = (UWORD)(*str & 0x0F);
	i += (UWORD)(*str++ >> 4);

	*vid = i;
	*len = l;

	return (str);
}

/************************************************************/
/*** z2a_getZipNo() : Make ZipNo and Rid From ZlTable *******/
/************************************************************/
UBYTE *z2a_getZipNo(str, zipNo, rid)
	UBYTE*	str;
	ULONG*	zipNo;
	UWORD*	rid;
{
	ULONG	zn;

	*rid = (UWORD)(*str >> 4);
	zn =  (ULONG)(*str++ & 0x0F) << 16;
	zn += (ULONG)(*str++) << 8;
	zn += (ULONG)(*str++);
	*zipNo = zn;

	return (str);
}

