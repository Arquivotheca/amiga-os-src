/****************************************************************/
/*	DMPVL.C :	ERGOSOFT DUMP PROGRAM VLBLOCK IN EGZIPM.DIC		*/
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
#include	<stdlib.h>

/*
#include	<Types.h>
*/

typedef unsigned char	UBYTE;
typedef unsigned short	UWORD;
typedef unsigned long	ULONG;

/**************************
*** FUNCTION PROTOTYPE
***************************/
short int		openDic();
void	dumpVl();
void	closeDic();
void	nstrcpy();

#define	false		0
#define true		1
#define	BLK_SIZE	1024

UBYTE	defDicName[] = "EGZIPM.DIC";
UBYTE	DicName[256];
FILE	*dicFp;

UWORD	zlCount;
UWORD	vlCount;
UBYTE	zlIndex[300];
UBYTE	vlIndex[300];
/*
UBYTE	zlBuf[BLK_SIZE];
*/
UBYTE	vlBuf[BLK_SIZE];


/**********************************
*** main : Dump VL List utility
***********************************/
void	main(ac, av)
	int ac;
	char *av[];
{
	short int		i;

	strcpy(DicName, defDicName);
	for (i = 1; i < ac; ++i)
		strcpy(DicName, av[i]);
	if (openDic()) {
		dumpVl();
		closeDic();
	}
}


/************************************
*** openDic() : open ZipDictionary
************************************/
short int	openDic()
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
	memcpy(zlIndex, &buf[2], 300);
	memcpy(vlIndex, &buf[302], 300);
	return(true);
}


/*************************************
***	closeDic() : Close ZipDictionary
**************************************/
void closeDic()
{
	if (dicFp)
		fclose(dicFp);
}


/*********************************************
*** dumpVl() : Dump VL List in ZipDictionary
**********************************************/
void dumpVl()
{
	register int	i, len;
	UWORD			blk, vid;
	ULONG			vlPos;
	UBYTE			*ptr;
	UBYTE			str[256];
	
	vlPos = BLK_SIZE * (1 + zlCount);

	for (blk = 0; blk < vlCount; ++blk)
	{
		fseek(dicFp, vlPos + (BLK_SIZE * blk), SEEK_SET);
		fread(vlBuf, BLK_SIZE, 1, dicFp);
		printf("BLOCK %d **********************************************\n", blk);
		for (ptr = vlBuf, i = 0; i < 1022;)
		{
			vid = *ptr++;
			vid <<= 4;
			vid += (*ptr >> 4);
			len = (*ptr++ & 0x0f) * 2;
			if (!vid)
				break;
			nstrcpy(str, ptr, len);
			
			printf("\t%d\t%s\n", vid, str);
			ptr += len;
			i += len;
		}
	}
}


/**********************************
*** nstrcpy() : 
***********************************/
void nstrcpy(dst, src, n)
	UBYTE *dst;
	UBYTE *src;
	short int n;
{
	while (*src && n--)
		*dst++ = *src++;
	*dst = '\0';
}

