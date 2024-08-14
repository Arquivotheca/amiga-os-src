#include	"exec/types.h"
#include	"prtbase.h"

#include "printer_iprotos.h"

UBYTE *Default8BitChars[] = {
	" ", "!", "c", "L", "o", "Y", "|", "S",
	"\"", "c", "a", "`", "~", "-", "r", "-",
	"*", "+", "2", "3", "'", "u", "P", ".",
	",", "1", "o", "'", "/", "/", "/", "?",
	"A", "A", "A", "A", "A", "A", "A", "C",
	"E", "E", "E", "E", "I", "I", "I", "I",
	"D", "N", "O", "O", "O", "O", "O", "x",
	"O", "U", "U", "U", "U", "Y", "T", "3",
	"a", "a", "a", "a", "a", "a", "a", "c",
	"e", "e", "e", "e", "i", "i", "i", "i",
	"d", "n", "o", "o", "o", "o", "o", "/",
	"o", "u", "u", "u", "u", "y", "t", "y"
};

DoNothing()
{
	return(0);
}

DefaultDoSpecial()
{
	return(-1);
}

struct PrinterExtendedData PEDDefault = {
	"Generic",			/* PrinterName */
	(VOID(*)())DoNothing,		/* Init */
	(VOID(*)())DoNothing,			/* Expunge */
	DoNothing,			/* Open */
	(VOID(*)())DoNothing,			/* Close */
	PPC_BWALPHA,			/* PrinterClass */
	PCC_BW,				/* ColorClass */
	80,					/* MaxColumns */
	1,					/* NumCharSets */
	0,					/* NumRows */
	0,					/* MaxXDots */
	0,					/* MaxYDots */
	0,					/* XDotsInch */
	0,					/* YDotsInch */
	0,					/* Commands */
	DefaultDoSpecial,	/* DoSpecial */
	DoNothing,			/* Render */
	0,					/* Delay */
	Default8BitChars,	/* 8BitChars */
	0,					/* PrintMode */
	0,					/* ConvFunc */
};
