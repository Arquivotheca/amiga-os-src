/****************************************************************/
/*	MAKEZIPM.C :	ERGOSOFT Make EGZIPM.DIC from ZIPMAIN.TXT 	*/
/*																*/
/*		designed by M.Yamashita    90.07.03 					*/
/*		coded	 by   Harada       90.07.04 Version 1.0 		*/
/*		modified by T.KOIDE		   93.02.01 					*/
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

typedef unsigned char	UBYTE;
typedef unsigned short	UWORD;
typedef unsigned long	ULONG;


#define BLOCK	1024				/* Block size */
#define ZLMAX	62					/* ZL max block count */
#define VLMAX	32					/* VL max block count */


/* global valuable definition. */

UBYTE	gxTable[BLOCK];				/* GX table */

UBYTE	zlIndex[BLOCK];				/* ZL index */
UBYTE	zlTable[ZLMAX][BLOCK];		/* ZL table */

UWORD	zlbCount = 0;				/* ZL block count */
UWORD	zlpCount = 0;				/* ZL point count */

UBYTE	vlIndex[BLOCK];				/* VL index */
UBYTE	vlTable[VLMAX][BLOCK];		/* VL table */

UWORD	vlbCount = 0;				/* VL block count */
UWORD	vlpCount = 0;				/* VL point count */
UWORD	vidCount = 0;				/* vid count */


/* function prototype definiton. */

void initTbl();
short int makeVlTbl();
short int makeZlTbl();
short int sortVlTbl();
void makeVlIdx();
void makeZlIdx();
short int	addVl();
short int addZl();
UWORD schVl();
UWORD schZl();
UBYTE *getVid();
UBYTE *setVid();
UBYTE *getZipNo();
UBYTE *setZipNo();
void getVlName();


/* main procedure. */

main(argc, argv)
int argc;
char *argv[];
{
	char	ziptext[64];
	FILE	*fp;
	UWORD	i;

	printf("ERGOSOFT Make EGZIPM.DIC from ZIPMAIN.TXT Version 1.0\n");
	printf("(C)Copyright 1990 ERGOSOFT Corp. All rights reserved.\n\n");

	if (argc < 2) {
		printf("usage: MAKEZIPM zipmain.txt \n");
		return (0);
	}
	else {
		strcpy(ziptext, argv[1]);
	}

	initTbl();

	if ((fp = fopen(ziptext, "r")) == NULL) {
		printf("%séâIü[âvâôéé½ééé±.\n", ziptext);
		exit(1);
	}

	printf("éuék(Value List)éìÉ¼ééé.   \n");
	if (makeVlTbl(fp)) {
		exit(1);
	}

	printf("éuék(Value List)éâ\\ü[âgééé.\n");
	if (sortVlTbl()) {
		exit(1);
	} 

	fseek(fp, 0L, SEEK_SET);

	printf("éyék(Zip List)éìÉ¼ééé.     \n");
	if (makeZlTbl(fp)) {
		exit(1);
	}

	fclose(fp);

	printf("îïëéEGZIPM.DICéÅæé½ìééé. \n");
	for (i = 0; i < BLOCK; i++) {
		gxTable[i] = 0;
	}
	gxTable[0] = (UBYTE)zlbCount;
	gxTable[1] = (UBYTE)vlbCount;
	memcpy(&gxTable[  2], zlIndex, 300);
	memcpy(&gxTable[302], vlIndex, 300);

	if ((fp = fopen("EGZIPM.DIC", "wb")) == NULL) {
		printf("EGZIPM.DICéÅæé½ìééÄöséééé.\n");
		exit(1);
	}
	fwrite(gxTable, BLOCK, 1, fp);
	for (i = 0; i < zlbCount; i++) {
		fwrite(zlTable[i], BLOCK, 1, fp);
	}
	for (i = 0; i < vlbCount; i++) {
		fwrite(vlTable[i], BLOCK, 1, fp);
	}
	fclose(fp);

	return (0);
}

void initTbl()
{
	int	i, j;

	zlbCount = 0;
	zlpCount = 0;
	for (j = 0; j < BLOCK; j++) {
		zlIndex[j] = 0;
	}
	for (i = 0; i < ZLMAX; i++) {
		for (j = 0; j < BLOCK; j++) {
			zlTable[i][j] = 0;
		}
	}

	vlbCount = 0;
	vlpCount = 0;
	vidCount = 0;
	for (j = 0; j < BLOCK; j++) {
		vlIndex[j] = 0;
	}
	for (i = 0; i < VLMAX; i++) {
		for (j = 0; j < BLOCK; j++) {
			vlTable[i][j] = 0;
		}
	}
}

short int makeVlTbl(fp)
	FILE *fp;
{
	UBYTE	zipNo[7];
	UBYTE	addr[3][64];
	UWORD	addc;
	UWORD	i;
	UWORD	len;

	while (!feof(fp) && !ferror(fp)) {
		fscanf(fp, "%s", zipNo);

		if (zipNo[0] == NULL) {
			break;
		}

		addc = (UWORD)(zipNo[0] - '0');
		switch (addc) {
		case 1:
			fscanf(fp, "%s", addr[0]);
			break;
		case 2:
			fscanf(fp, "%s %s", addr[0], addr[1]);
			break;
		case 3:
			fscanf(fp, "%s %s %s", addr[0], addr[1], addr[2]);
			break;
		default:
			printf("bad address data !!\n");
			return(-1);
			break;
		}

		for (i = 0; i < addc; i++) {
			len = strlen(addr[i]) - 1;
			printf(" %s \033[0K\r", addr[i]);
			if (addVl(addr[i], len)) {
				return (-1);
			}
		}
	}
	if (vlpCount > 0) {
		vlbCount++;
	}
	makeVlIdx();

	return (0);
}

short int makeZlTbl(fp)
	FILE *fp;
{
	UBYTE	zip[7];
	UBYTE	addr[3][32];
	UWORD	rid;
	UWORD	i;
	UWORD	len;
	ULONG	zipNo;

	while (!feof(fp) && !ferror(fp)) {
		fscanf(fp, "%s", zip);

		if (zip[0] == NULL) {
			break;
		}

		rid = (UWORD)(zip[0] - '0');
		switch (rid) {
		case 1:
			fscanf(fp, "%s", addr[0]);
			addr[1][0] = addr[2][0] = 0;

			break;
		case 2:
			fscanf(fp, "%s %s", addr[0], addr[1]);
			addr[2][0] = 0;
			break;
		case 3:
			fscanf(fp, "%s %s %s", addr[0], addr[1], addr[2]);
			break;
		default:
			printf("bad address data !!\n");
			return(-1);
			break;
		}

		len = strlen(&zip[1]);
		if (len < 5) {
			zip[4] = '0';
			zip[5] = '0';
		}
		zipNo = 0L;
		for (i = 1; i <= 5; i++) {
			zipNo = (zipNo * 10) + (ULONG)(zip[i] - '0');
		}

		printf(" %05ld %s %s %s \033[0K\r", zipNo, addr[0], addr[1], addr[2]);
		if (addZl(rid, zipNo, addr[0], addr[1], addr[2])) {
			return (-1);
		}
	}
	if (zlpCount > 0) {
		zlbCount++;
	}
	makeZlIdx();

	return (0);
}

short int sortVlTbl()
{
	UWORD	*idx;
	UBYTE	*tbl, *p;
	UWORD	i;
	short	n;
	UBYTE	str1[32];
	UBYTE	str2[32];
	UWORD	len1;
	UWORD	len2;
	UWORD	vid;
	UWORD	id;
	UWORD	dlt;
	short	rs;

	idx = (UWORD *)malloc(vidCount * sizeof(UWORD));
	if (idx == NULL) {
		printf("memory alloc error !!\n");
		return (-1);
	}
	for (i = 0; i < vidCount; i++) {
		*(idx + i) = 0;
	}

	for (i = 0; i < vidCount; i++) {
		vid = i + 1;
		getVlName(vid, str1, &len1);
		str1[len1] = 0;
		printf(" %s \033[0K\r", str1);

		dlt = i / 2;
		n = dlt;
		while (dlt > 0) {
			id = *(idx + n);
			getVlName(id, str2, &len2);
			str2[len2] = 0;
			rs = memcmp(str1, str2, (len1 > len2)? len1: len2);
			dlt /= 2;
			if (rs < 0) {
				n -= dlt;
			}
			else {
				n += dlt;
			}
		}
		if (n > 0) {
			if (rs > 0) {
				while (rs > 0 && n < i) {
					n++;
					id = *(idx + n);
					getVlName(id, str2, &len2);
					str2[len2] = 0;
					rs = memcmp(str1, str2, (len1 > len2)? len1: len2);
				}
			}
			else {
				while (rs < 0 && n > 0) {
					n--;
					id = *(idx + n);
					getVlName(id, str2, &len2);
					str2[len2] = 0;
					rs = memcmp(str1, str2, (len1 > len2)? len1: len2);
				}
				if (rs > 0) {
					n++;
				}
			}
		}
		if ((i - n) > 0) {
			memmove(idx + n + 1, idx + n, (i - n) * sizeof(UWORD));
		}
		*(idx + n) = vid;
	}

	printf(" **** â\\ü[âgééîïëéôoÿ^ééé.\r");

	tbl = malloc(vlbCount * BLOCK);
	if (tbl == NULL) {
		free(idx);
		printf("memory alloc error !!\n");
		return (-1);
	}

	p = tbl;
	for (i = 0; i < vidCount; i++) {
		vid = *(idx + i);
		getVlName(vid, str1, &len1);

		*p++ = (UBYTE)len1;
		memcpy(p, str1, len1);
		p += len1;
		printf(" %4d\r", i);
	}
	n = vidCount;

	initTbl();

	p = tbl;
	for (i = 0; i < n; i++) {
		len1 = (UWORD)*p++;
		addVl(p, len1);
		p += len1;
		printf(" %4d\r", n - i);
	}
	if (vlpCount > 0) {
		vlbCount++;
	}
	makeVlIdx();

	free(tbl);
	free(idx);

	printf(" \033[0K\r");
	return (0);
}

void makeVlIdx()
{
	UWORD	i;
	UBYTE	*idx;
	UBYTE	*tbl;
	UWORD	vid;
	UWORD	len;

	idx = vlIndex;
	for (i = 0; i < vlbCount; i++) {
		tbl = vlTable[i];
		tbl = getVid(tbl, &vid, &len);
		idx = setVid(idx, vid, len);
		memcpy(idx, tbl, len);
		idx += len;
		*idx++ = (UBYTE)i;
	}
}

void makeZlIdx()
{
	UWORD	i;
	UBYTE	*idx;
	UBYTE	*tbl;
	ULONG	zipNo;
	UWORD	rid;

	idx = zlIndex;
	for (i = 0; i < zlbCount; i++) {
		tbl = zlTable[i];
		tbl = getZipNo(tbl, &zipNo, &rid);
		idx = setZipNo(idx, zipNo, 0);
		*idx++ = (UBYTE)i;
	}
}

short int	addVl(str, len)
	UBYTE *str;
	UWORD len;
{
	UBYTE	*tbl;

	if (schVl(str, len) != 0) {
		return (0);
	}
	if ((vlpCount + len + 2) >= BLOCK) {
		vlbCount++;
		vlpCount = 0;
		if (vlbCount == VLMAX) {
			printf("Value List Table overflow !!\n");
			return(-1);
		}
	}
	vidCount++;
	tbl = vlTable[vlbCount];
	tbl = setVid(tbl + vlpCount, vidCount, len);
	memcpy(tbl, str, len);
	vlpCount += (len + 2);

	return(0);
}

short int addZl(rid, zipNo, str1, str2, str3)
	UWORD rid;
	ULONG zipNo;
	UBYTE *str1;
	UBYTE *str2;
	UBYTE *str3;
{
	static UWORD rrs[3][4] = {4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 0};

	UBYTE	*tbl;
	UBYTE	*addr[3];
	UWORD	vid;
	UWORD	rr;
	UWORD	len;
	UWORD	i;
	short	n;
	UBYTE	*zlb;
	UWORD	zlp;
	ULONG	zn;
	UWORD	ri;

	addr[0] = str1;
	addr[1] = str2;
	addr[2] = str3;

	tbl = zlTable[zlbCount];

	if ((zlpCount + (3 + rid*2)) >= BLOCK) {
		zlb = tbl;
		zlp = 0;
		zlbCount++;
		zlpCount = 0;
		if (zlbCount == ZLMAX) {
			printf("Zip List Table overflow !!\n");
			return(-1);
		}
		tbl = zlTable[zlbCount];

		while ((zlp = schZl(zlb, zlp, zipNo)) != 0) {
			getZipNo(zlb + zlp, &zn, &ri);
			memset(zlb + zlp, 0, 3);
			zlp += 3;
			setZipNo(tbl + zlpCount, zn, ri);
			zlpCount += 3;
			for (i = 0; i < ri; i++) {
				getVid(zlb + zlp, &vid, &rr);
				memset(zlb + zlp, 0, 2);
				setVid(tbl + zlpCount, vid, rr);
				zlp += 2;
				zlpCount += 2;
			}
		}
	}

	tbl = setZipNo(tbl + zlpCount, zipNo, rid);

	for (i = 0; i < rid; i++) {
		len = strlen(addr[i]) - 1;
		n = (short)(addr[i][len] - '1');
		if (n < 0 || n > 3) {
			printf("bad address attribute error !!\n");
			return (-1);
		}
		rr = rrs[i][n];
		if (rid == 1) {
			rr = n + 1;
		}
		vid = schVl(addr[i], len);
		tbl = setVid(tbl, vid, rr * 2);
	}
	zlpCount += (3 + rid*2);

	return(0);
}

UWORD schVl(str, len)
	UBYTE *str; 
	UWORD len;
{
	UWORD	vid = 0;
	UWORD	vlb = 0;
	UWORD	vlp = 0;
	UWORD	id = 0;
	UWORD	ln;
	UBYTE	*tbl;

	while (vid < vidCount) {
		if (id == 0 || vlp >= BLOCK) {
			if (vlb > vlbCount) {
				return (0);
			}
			tbl = vlTable[vlb++];
			vlp = 0;
		}
		getVid(tbl + vlp, &id, &ln);
		vlp += 2;
		vid = id;
		if (len == ln && memcmp(str, tbl + vlp, len) == 0) {
			return (vid);
		}
		vlp += ln;
	}
	return (0);
}

UWORD schZl(zlb, zlp, zipNo)
	UBYTE *zlb;
	UWORD zlp;
	ULONG zipNo;
{
	ULONG	zn;
	UWORD	rid;

	while ((zlp + 3) < BLOCK) {
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
	*len = l * 2;

	return (str);
}

UBYTE *setVid(str, vid, len)
	UBYTE *str;
	UWORD vid;
	UWORD len;
{
	*str++ = (UBYTE)(vid >> 4);
	*str++ = (UBYTE)((vid << 4) + (len / 2));

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

UBYTE *setZipNo(str, zipNo, rid)
	UBYTE *str;
	ULONG zipNo;
	UWORD rid;
{
	*str++ = (UBYTE)((zipNo >> 16) & 0x0F) + (UBYTE)(rid << 4);
	*str++ = (UBYTE)(zipNo >> 8);
	*str++ = (UBYTE)zipNo;

	return (str);
}

void getVlName(vid, str, len)
	UWORD vid;
	UBYTE *str;
	UWORD *len;
{
	UBYTE	*idx;
	UWORD	vlb;
	UWORD	id;
	UWORD	ln;
	UWORD	i;
	UBYTE	*tbl;

	idx = vlIndex;
	for (i = 0; i < vlbCount; i++) {
		idx = getVid(idx, &id, &ln);
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

	tbl = vlTable[vlb];
	ln = 0;
	do {
		tbl += ln;
		tbl = getVid(tbl, &id, &ln);
	} while (vid != id);

	memcpy(str, tbl, ln);
	*len = ln;
}

