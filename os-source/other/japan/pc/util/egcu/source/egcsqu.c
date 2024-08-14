/*--------------------------------------------------------------------------
#
#	Name : egcsqu.c
#		Create ERGOSOFT Sequential Dictionary Utility. ver1.0
#
#	Copyright (C) 1991 ERGOSOFT Corp.
#	All Rights Reserved.
#
#	Create : T.Harada	1990.02.15
#	Update : T.Harada	1991.04.25
#
#	THIS PROGRAM BELONGS TO ERGO SOFT CORP.  IT IS CONSIDERED A
#	TRADE SECRET AND IS NOT TO BE DIVULGED OR USED BY PARTIES
#	WHO HAVE NOT RECEIVED WRITTEN AUTHORIZATION FROM THE OWNER.
#
--------------------------------------------------------------------------*/

#include	<stdio.h>
#include	"edef.h"

/*
*** Maximum length difinition.
*/
#define		MAXYOMI		12
#define		MAXKANJ		32
#define		MAXHINS		 4

/*
*** Prototypes difinition.
*/
BOOL	aDoConvert(FILE *sfp,FILE *ofp);
BOOL	aGetData(UBYTE *buffer,UBYTE *yomi,UBYTE *kanji,UBYTE *hinshi);
VOID	aSetHinshi(UBYTE *hinshi,UBYTE *buffer);
BOOL	aWriteData(UBYTE *yomi,UBYTE *kanji,UBYTE *hinshi,FILE *ofp);
VOID	aMyError(WORD n);
BOOL	aIsKana(UBYTE theChar);
BOOL	aIsDigit(UBYTE theChar);
BOOL	aIsKanji(UBYTE theChar);
BOOL	aIsComment(UBYTE theChar);
BOOL	aIsDelimita(UBYTE theChar);
VOID	aDispBits(ULONG);
VOID	aDispHelp(VOID);

/*
*** gloval values difinition.
*/
UBYTE	srcname[128];
UBYTE	outname[128];
LONG	serNo;

WORD main(argc, argv)
	WORD argc;
	UBYTE *argv[];
{
	FILE	*sfp;
	FILE	*ofp;
	UBYTE	*ptr;
	WORD	i;
	
	printf("\nCreate ERGOSOFT Sequential Dictionary Utility Version 1.00\n");
	printf("Copyright(C) 1991. ERGOSOFT Corp. All rights reserved.\n");
	fflush(stdout);

	ptr = argv[1];
	if(argc != 3) {
		aDispHelp();
		return 2;
	}
	ptr = argv[1];
	for (i = 0;; ) {
		if (*ptr == '\0') break;
		srcname[i++] = *ptr++;
	}
	srcname[i] = '\0';
	printf("\nSource file : %s\n",srcname);
	fflush(stdout);

	ptr = argv[2];
	for (i = 0;; ) {
		if (*ptr == '\0') break;
		outname[i++] = *ptr++;
	}
	outname[i] = '\0';
	printf("Output file : %s\n",outname);
	fflush(stdout);

	if((sfp = fopen(srcname, "rt")) == NULL) {
		printf("Couldn't open source file !!\n");
		aDispHelp();
		return 2;
	}
	if((ofp = fopen(outname, "wt")) == NULL) {
		printf("Could't open output file\n");
		aDispHelp();
		return 2;
	}
	if (! aDoConvert(sfp, ofp)) {
		fclose(sfp);
		fclose(ofp);
		return 2;
	}
	fclose(sfp);
	fclose(ofp);
	printf("\n	Completed...\n\n");
	return 0;
}

BOOL aDoConvert(sfp, ofp)
	FILE	*sfp;
	FILE	*ofp;
{
	UBYTE	buffer[512];
	UBYTE	yomi[128];
	UBYTE	kanji[128];
	UBYTE	hinshi[4];

	for (serNo = 1;; serNo++) {
		if (fgets(buffer, 512, sfp) == NULL) {
#ifdef DEBUG
			printf ("###### ERROR %d\n", ferror(sfp));
			printf ("###### EOF ERROR %d\n", feof(sfp));
#endif
			break;
		}
		if (! aGetData(buffer, yomi, kanji, hinshi)) continue;
		if (! aWriteData(yomi, kanji, hinshi, ofp)) {
			return (FALSE);
		}
	}
	return (TRUE);
}

BOOL aGetData(buffer, yomi, kanji, hinshi)
	UBYTE	*buffer;
	UBYTE	*yomi;
	UBYTE	*kanji;
	UBYTE	*hinshi;
{
	WORD	i, j, k;
	
	if (aIsComment(buffer[0])) return (FALSE);

	/* yomi set */
	
	for (i = 0, j = 0; ; ) {
		if (j > MAXYOMI) {
			aMyError(4);				/* yomi error !! */
			return (FALSE);
		}
		if (buffer[i] == '\n') {
			aMyError(2);				/* kanji error !! */
			return (FALSE);
		}
		if (aIsDelimita(buffer[i])) {
			i++;
			break;
		}
		if (! aIsKana(buffer[i]) && ! aIsDigit(buffer[i])) {
			aMyError(1);				/* yomi error !! */
			return (FALSE);
		}
		yomi[j++] = buffer[i++];
	}
	if (j <= 0) {
		aMyError(6);					/* yomi error !! */
		return (FALSE);
	}
	for (k = j; k < MAXYOMI; k++) yomi[k] = 0;
	
	/* kanji set */
	
	for (j = 0; ; ) {
		if (j > MAXKANJ) {
			aMyError(5);				/* kanji error !! */
			return (FALSE);
		}
		if (buffer[i] == '\n') {
			aSetHinshi(hinshi, "");
			break;
		}
		if (aIsDelimita(buffer[i])) {
			aSetHinshi(hinshi, &buffer[i+1]);
			break;
		}
		if (! aIsKanji(buffer[i])) {
			aMyError(2);				/* kanji error !! */
			return (FALSE);
		}
		kanji[j++] = buffer[i++];
		kanji[j++] = buffer[i++];
	}
	if (j <= 0) {
		aMyError(7);					/* kanji error !! */
		return (FALSE);
	}
	for (k = j; k < MAXKANJ; k++) kanji[k] = 0;
	return (TRUE);
}

VOID aSetHinshi(hinshi, buffer)
	UBYTE *hinshi;
	UBYTE *buffer;
{
	UBYTE	temp[128];
	UWORD	htype;
	LONG 	hbit;
	WORD	i, j;
	
	for (hbit = 0L, i = 0; ; ) {
		if (buffer[i] == '\n' || buffer[i] == '\0'
				|| buffer[i] == '\t') break;
		for (j = 0;; ) {
			if (buffer[i] == '\n' || buffer[i] == '\0'
					|| buffer[i] == '\t') break;
			if (buffer[i] == '+') {
				i++;
				break;
			}
			if (! aIsDigit(buffer[i])) {
				aMyError(3);				/* hinshi error !! */
				printf ("--> Ignored !! '%c' is bad.\n",buffer[i]);
				i++;
				break;
			}
			temp[j++] = buffer[i++];
		}
		temp[j] = 0;

		if (temp[0] == 0) {
			hbit = 0;
			break;
		}
		htype = atoi(temp);

		if (htype > 31) {
			aMyError(3);				/* hinshi error !! */
			printf ("--> Ignored !! '%s' is bad.\n",temp);
		}
		else if (htype < 14 && htype > 10) {
			aMyError(3);				/* hinshi error !! */
			printf ("--> Ignored !! '%s' is reserved.\n",temp);
		}
		else 
			hbit = hbit | (1L << (31 - htype));
	}
#ifdef DEBUG
	aDispBits(hbit);
#endif
	hinshi[3] = (UWORD)(hbit & 0x000000ff);
	hinshi[2] = (UWORD)((hbit >> 8) & 0x000000ff);
	hinshi[1] = (UWORD)((hbit >> 16) & 0x000000ff);
	hinshi[0] = (UWORD)((hbit >> 24) & 0x000000ff);
}

BOOL aWriteData(yomi, kanji, hinshi, ofp)
	UBYTE	*yomi;
	UBYTE	*kanji;
	UBYTE	*hinshi;
	FILE	*ofp;
{
	if (fwrite(yomi, MAXYOMI, 1, ofp) < 1) {
		printf("could not write file\n");
		return (FALSE);
	}
	if (fwrite(hinshi, MAXHINS, 1, ofp) < 1) {
		printf("could not write file\n");
		return (FALSE);
	}
	if (fwrite(kanji, MAXKANJ, 1, ofp) < 1) {
		printf("could not write file\n");
		return (FALSE);
	}
	return (TRUE);
}

VOID aMyError(n)
	WORD	n;
{
	switch (n) {
		case 1:	printf("File %s; Line %d # Yomi  error --> Skip !! Invalid data.\n", srcname, serNo);
				break;
		case 2:	printf("File %s; Line %d # Kanji error --> Skip !! Invalid data.\n", srcname, serNo);
				break;
		case 3:	printf("File %s; Line %d # Hinshi error ", srcname, serNo);
				break;
		case 4:	printf("File %s; Line %d # Yomi  error --> Skip !! Too long.\n", srcname, serNo);
				break;
		case 5:	printf("File %s; Line %d # Kanji error --> Skip !! Too long.\n", srcname, serNo);
				break;
		case 6:	printf("File %s; Line %d # Yomi  error --> Skip !! No data.\n", srcname, serNo);
				break;
		case 7:	printf("File %s; Line %d # Kanji error --> Skip !! No data.\n", srcname, serNo);
				break;
	}
}

BOOL aIsKana(theChar)
	UBYTE	theChar;		/* Char code */
{
	return((theChar >= (UBYTE) 0xa6) && (theChar <= (UBYTE) 0xdf));
}

BOOL aIsDigit(theChar)
	UBYTE	theChar;		// Char code
{
	return((theChar >= (UBYTE) 0x30) && (theChar <= (UBYTE) 0x39));
}

BOOL aIsKanji(theChar)
	UBYTE	theChar;		/* Char code */
{
	return(((theChar >= (UBYTE) 0x81) && (theChar <= (UBYTE) 0x9f)) ||
		   ((theChar >= (UBYTE) 0xe0) && (theChar <= (UBYTE) 0xfc)));
}

BOOL aIsComment(theChar)
	UBYTE	theChar;		/* Char code */
{
	return(theChar == (UBYTE) '/');
}

BOOL aIsDelimita(theChar)
	UBYTE	theChar;		/* Char code */
{
	return(theChar == (UBYTE) '\t');
}

VOID aDispHelp(VOID)
{
	printf("\n");
	printf("Usage: egcsqu <source file> <output file>");
	printf("\n\n");
}

VOID aDispBits(bits)
	ULONG	bits;
{
	WORD	i, b;
	
	printf(" Hinshi bit :  ");
	for (i = 31; i >= 0; i--) {
		b = (bits >> i) & 0x00000001;
		printf ("%d",b);
	}
	printf("\n");
}
