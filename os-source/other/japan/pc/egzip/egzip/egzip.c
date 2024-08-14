/****************************************************************/
/*	EGZIP.C :	EGZIP ver1.0 sample program	                	*/
/*																*/
/*		designed by   Koide		   92.10.15    					*/
/*		coded	 by   Koide        92.10.15 Version 1.0 		*/
/*		modified by                  .  .	                	*/
/*																*/
/*	(C) Copyright 1992 ERGOSOFT Corp. All Rights Reserved.		*/
/*																*/
/*							--- NOTE ---						*/
/*																*/
/*	THIS PROGRAM BELONGS TO ERGOSOFT CORP.	IT IS CONSIDERED A	*/
/*	TRADE SECRET AND IS NOT TO BE DIVULGED OR USED BY PARTIES	*/
/*	WHO HAVE NOT RECEIVED WRITTEN AUTHORIZATION FROM THE OWNER. */
/****************************************************************/
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>

#include	"egzip.h"

/***************************************/
/*** FUNCTION PROTO-TYPE DECLARATION ***/
/***************************************/
#ifndef	UNIX
VOID 	ZipZtoA(PZIPM);
VOID 	ZipAtoZ(PZIPM);
VOID	ZtoALoop(PZIPM);
VOID	AtoZLoop(PZIPM);
ULONG 	makeZipNo(UBYTE *);
VOID	ZipUsage(VOID);
UBYTE 	*getLine( UBYTE *str, WORD n, FILE *stream);
ZIPM		zipm;
UBYTE		ZipStr[256];
UBYTE		AddrStr[256];
ULONG		ZipCode;
#else
VOID 	ZipZtoA();
VOID 	ZipAtoZ();
VOID	ZtoALoop();
VOID	AtoZLoop();
ULONG 	makeZipNo(
VOID	ZipUsage();
UBYTE 	*getLine();
ZIPM		zipm;
UBYTE		ZipStr[256];
UBYTE		AddrStr[256];
ULONG		ZipCode;
#endif

/* page */
/**********************************/
/*** main() : EGZIP MAIN **********/
/**********************************/
main(argc, argv)
	WORD	argc;
	char*	argv[];
{
	UBYTE		zipdict[64];
	WORD			result;

	printf("\nERGOSOFT EGZIP Sample Program Version 1.0\n");
	printf("(C)Copyright 1992 ERGOSOFT Corp. All rights reserved.\n");

	if (argc <= 2) {
		ZipUsage();
		return (0);
	}
	else {
		strcpy(zipdict, argv[1]);
		result = initZipm(&zipm, zipdict);	/* intialize zipm and open dict */
	}

	/*--------- intialize error message ----------*/
	if (result != NORMAL ) {
		if (result == ERROR01)
			printf("\n%séâIü[âvâôéé½ééé±.\n", zipdict);
		else if (result == ERROR02) 
			printf("\n%séEGZIPM.DICéâtâ@âCâïî`Ä«éééáéééé±.\n", zipdict);
		else
			printf("\nMemory Allocation Error !!\n");
		exit(1);
		
	}

	if(!strcmp(argv[2],"-a")){
		ZtoALoop(&zipm);
	}
	else if(!strcmp(argv[2],"-z")){
		AtoZLoop(&zipm);
	}
	else {
		ZipUsage();
		return (0);
	}

	termZipm(&zipm); /* memory free and close dict */
	return (0);
}

/* page */
/**********************************************/
/*** ZtoALoop() : *****************************/
/**********************************************/
VOID ZtoALoop(pzipm)
	PZIPM	pzipm;
{

	do {

		printf("\nîìéééóùXòöìåéôùééé¡éééó <E:END> :\n>");

		if ( getLine(ZipStr, sizeof ZipStr, stdin) == NULL ) {
			printf("\n\nEND OF FILE é¬ôùééééóééééÅIùééé.\n");
			return;
		}

		ZipCode = 0L;
		ZipCode = makeZipNo(ZipStr);

		if (ZipStr[0] == 'E' || ZipStr[0] == 'e' ) {
			printf("\n\t******** END ********\n");
		}
		else if (ZipCode >= 100000L || ZipCode == 0L) {
			printf("îìééùXòöìåé¬òsÉéé.\n");
		}
		else {
			ZipZtoA(pzipm);
		}

	} while (ZipStr[0] != 'E' && ZipStr[0] != 'e' );

}
/******************************************/
/*** ZipZtoA() : Zipcode ==> Address ******/
/******************************************/
VOID	ZipZtoA(pzipm)
	PZIPM	pzipm;
{
	UWORD	len;
	WORD		result;

	do {
		result = cnvZipToAddr(pzipm, ZipCode, AddrStr, &len);
		if (result != NOFIND) {
			AddrStr[len] = 0;
			printf("üº%03ld-%02ld  %s \n", 
							ZipCode / 100, ZipCode % 100, AddrStr);
		}
		else {
			printf("èYôûâfü[â^ééáéééé±.\n");
		}
	} while (result == FINDNEXT);
}

/* page */
/**********************************************/
/*** AtoZLoop(): ******************************/
/**********************************************/
VOID	AtoZLoop(pzipm)
	PZIPM	pzipm;
{

	do {
		printf("\nîìéééóÅZÅèéôùééé¡éééó <E:END> :\n>");
		if ( getLine(AddrStr, sizeof AddrStr, stdin) == NULL ) {
			printf("\n\nEND OF FILE é¬ôùééééóééééÅIùééé.\n");
			return;
		}

		if (AddrStr[0] == 'E' || AddrStr[0] == 'e' ) {
			printf("\n\t******** END ********\n");
		}
		else {
			ZipAtoZ(pzipm);
		}

	} while (AddrStr[0] != 'E' && AddrStr[0] != 'e' && AddrStr[0] != EOF);
}

/*******************************************/
/*** ZipAtoZ() : Address ==> Zipcode *******/
/*******************************************/
VOID	ZipAtoZ(pzipm)
	PZIPM	pzipm;
{
	WORD			result;


	do {
		/*------ Address => ZipCode ----------*/
		result = cnvAddrToZip(pzipm, (UBYTE *)AddrStr,
								(UBYTE *)ZipStr, &ZipCode);
		if (result != NOFIND) {
				printf("üº%03ld-%02ld  %s \n", ZipCode / 100,
										 ZipCode % 100, ZipStr);
		}
	} while (result == FINDNEXT);

}

/* page */
/*********************************************/
/*** makeZipNo() : Make zipno from string ****/
/*********************************************/
ULONG makeZipNo(str)
	UBYTE*	str;
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
		if (str[i] < '0' || str[i] > '9') 
			return(0L);
	}
		
	for (i = 0; i < 5; i++) {
			zipNo = (zipNo * 10) + (ULONG)(str[i] - '0');
	}

	return (zipNo);
}

/****************************************************/
/*** GetLine() : Get Strings and not get returnCode */
/****************************************************/
UBYTE *getLine(str, n, stream)
	UBYTE*	str;
	WORD	n;
	FILE*	stream;
{
	UBYTE 	*st = str;
	WORD		c;

	if(n <= 0)	return NULL;

	while ( --n && (c = fgetc(stream)) != EOF  && c != '\n') {
		if (c != ' ') {
			*st = (UBYTE)c;
			st++;
		}
	}
	*st = '\0';

	if ( c == EOF && st == str) {
		return NULL;
	}
	return	st;
}

/*****************************/
/*** ZipUsage() : ************/
/*****************************/
VOID	ZipUsage()
{
	printf("\nusage: EGZIP egzipm.dic <option>\n\n");
	printf("        <option> -a : zipcode ==> address\n");
	printf("                 -z : address ==> zipcode\n");
}

