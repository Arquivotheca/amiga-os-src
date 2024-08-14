#include "ed.h"

#if AMIGA
#include "exec/types.h"
#endif

/* default function key definitions */

UBYTE	*fkmdef[35] = {
	"\000",
	"", /* clone line		*/
	"",	/* delete line			*/
	"e", 	/* execute macro		*/
	"",		/* next screen			*/
	"V",	/* previous screen		*/
	"2",	/* split window 		*/
	"1",	/* one window 			*/
	"",		/* roll window up		*/
	"z",	/* roll window down		*/
	"", 	/* save file and exit		*/
	"\012",		/* line feed */
	"0",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"9",
	".",
	"\012",		/* line feed */
	"-",
	"\000",
	"\000",
	"\000",
	"\000",
	"\000",
	"\000",
	"\000",
	"\000",
	"\000",
	"\000"
};   
