/*****************************************************************/
/*	DMPZL.C :	ERGOSOFT DUMP PROGRAM VZBLOCK IN EGZIPM.DIC		*/
/*																*/
/*		modified by T.KOIDE    93.02.01	                        */
/*																*/
/*	(C) Copyright 1993 ERGOSOFT Corp. All Rights Reserved.		*/
/*																*/
/*							--- NOTE ---						*/
/*																*/
/*	THIS PROGRAM BELONGS TO ERGOSOFT CORP.	IT IS CONSIDERED A	*/
/*	TRADE SECRET AND IS NOT TO BE DIVULGED OR USED BY PARTIES	*/
/*	WHO HAVE NOT RECEIVED WRITTEN AUTHORIZATION FROM THE OWNER. */
/****************************************************************/

#include	<stdio.h>
#include	<string.h>

typedef char			BYTE;
typedef unsigned char	UBYTE;
typedef unsigned short	UWORD;
typedef unsigned long	ULONG;

/******************************
*** FUNCTION PROTOTYPE
*******************************/
int		openDic();
void	dumpZl();
void	closeDic();
void	nstrcpy();

#define false		0
#define true		1
#define	BLK_SIZE	1024

BYTE	defDicName[] = "EGZIPM.DIC";
UBYTE	DicName[256];
FILE	*dicFp;
BYTE	*rrStr[] = {
			"ïÄäÅæöá",
			"ÄûûâZâôâ^ü[",
			"ï",
			"ôs",
			"ô",
			"ò{",
			"îº",
			"Äs",
			"îS",
			"ÄxÆí",
			"ïµ",
			"Æ¼",
			"æ",
			"<ïµ>"
		};

UWORD		zlCount;
UWORD	vlCount;
int		crrBlk = -1;
UBYTE	zlIndex[300];
UBYTE	vlIndex[300];
UBYTE	zlBuf[BLK_SIZE];
UBYTE	vlBuf[BLK_SIZE];

/*********************************
*** MAIN() : Dump ZL List utility
**********************************/
void	main(ac, av)
	int ac;
	char *av[];
{
	short int		i;

	strcpy(DicName, defDicName);
	for (i = 1; i < ac; ++i)
		strcpy(DicName, av[i]);
	if (openDic()) {
		dumpZl();
		closeDic();
	}
}

/**********************************
*** openDic() : Open ZipDictionary
***********************************/
int openDic()
{
	UBYTE			buf[BLK_SIZE];
	register int	ret;
	
	dicFp = fopen(DicName, "rb");
	if (!dicFp)
		return(false);
	ret = fread(buf, BLK_SIZE, 1, dicFp);
	if (ret != 1)
	{
		fclose(dicFp);
		printf("Record Error (openDic)!\n");
		return(false);
	}
	zlCount = (UWORD)buf[0];
	vlCount = (UWORD)buf[1];
	memmove(zlIndex, &buf[2], 300);
	memmove(vlIndex, &buf[302], 300);
	return(true);
}

/**************************************
*** closeDic() : Close ZipDictionary
**************************************/
void closeDic()
{
	if (dicFp)
		fclose(dicFp);
}


/*******************************************
*** getStrFromVid() : Make Strings From VID
********************************************/
void getStrFromVid(str, tvid)
	UBYTE *str;
	UWORD tvid;
{
	short int		blk, len, i;
	UWORD	 vid;
	ULONG	vlPos;
	UBYTE	*ptr;
	
	blk = 0;
	for (ptr = vlIndex, i = 0; i < 295;)
	{
		vid = (*ptr++ << 4);
		vid += (*ptr >> 4);
		if (!vid)
			break;
		len = (*ptr++ & 0x0f) * 2;
		if (vid == tvid)
		{
			nstrcpy(str, ptr, len);
			return;
		}
		if (tvid < vid)
			break;
		ptr += len;
		blk = (UWORD)*ptr++;
		i += (len + 3);
	}
	if (blk != crrBlk)
	{
	
		vlPos = BLK_SIZE * (1 + zlCount);

		fseek(dicFp, vlPos + (BLK_SIZE * blk), SEEK_SET);
		fread(vlBuf, BLK_SIZE, 1, dicFp);
		crrBlk = blk;
	}
	for (ptr = vlBuf, i = 0; i < 1020;)
	{
		vid = *ptr++ << 4;
		vid += *ptr >> 4;
		len = (*ptr++ & 0x0f) * 2;
		if (tvid == vid)
		{
			nstrcpy(str, ptr, len);
			return;
		}
		ptr += len;
		i += (len + 2);
	}
	*str = '\0';
}

/********************************************
*** dumpZl() : Dump ZL List in ZipDictionary
*********************************************/
void dumpZl()
{
	register int	i, ii, rid, rr;
	ULONG			zip;
	UWORD			blk, vid;
	UBYTE			*ptr;
	UBYTE			str[256], tmp[32];
	
	for (blk = 0; blk < zlCount; ++blk)
	{
		fseek(dicFp, BLK_SIZE * (1 + blk), SEEK_SET);
		fread(zlBuf, BLK_SIZE, 1, dicFp);
		printf("BLOCK %d **********************************************\n", blk);
		for (ptr = zlBuf, i = 0; i < 1020;)
		{
			rid = *ptr >> 4;
			if (!rid)
				break;
			zip = (ULONG)(*ptr++ & 0x0f) << 16;
			zip += (ULONG)(*ptr++) << 8;
			zip += (ULONG)(*ptr++);
			*str = '\0';
			for (ii = 0; ii < rid; ++ii)
			{
				vid = *ptr++;
				vid <<= 4;
				vid += (*ptr >> 4);
				rr = (*ptr++ & 0x0f) ;
				getStrFromVid(tmp, vid);
				strcat(str, tmp);
				strcat(str, rrStr[rr - 1]);
				strcat(str, " ");
			}
			printf("%05ld %s\n", zip, str);
		}
	}
}

/************************************
*** nstrcpy() :
*************************************/
void nstrcpy(dst, src, n)
	UBYTE *dst;
	UBYTE *src;
	short int n;
{
	while (*src && n--)
		*dst++ = *src++;
	*dst = '\0';

}

