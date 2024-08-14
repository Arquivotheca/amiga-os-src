;/* charset.c - Execute me to compile me with SAS C 5.10
LC -b1 -cfistq -v -y -j73 charset.c
Blink FROM LIB:c.o,charset.o TO charset LIBRARY LIB:LC.lib,LIB:Amiga.lib
quit
*/

#include <exec/types.h>
#include <exec/memory.h>
#include <libraries/dos.h>
#include <intuition/intuition.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef LATTICE
int CXBRK(void) { return(0); }  /* Disable Lattice CTRL/C handling */
int chkabort(void) { return(0); }  /* really */
#endif


#define MINARGS 1

UBYTE *vers = "\0$VER: charset 37.2";
UBYTE *Copyright = 
  "charset v37.2\nCopyright (c) 1990 Commodore-Amiga, Inc.  All Rights Reserved";
UBYTE *usage = "Usage: charset [allbox]";

UBYTE *header[] = {
"          +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+",
"          | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 |",
"          | 0 | 0 | 0 | 0 | 1 | 1 | 1 | 1 | 0 | 0 | 0 | 0 | 1 | 1 | 1 | 1 |",
"          | 0 | 0 | 1 | 1 | 0 | 0 | 1 | 1 | 0 | 0 | 1 | 1 | 0 | 0 | 1 | 1 |",
"          | 0 | 1 | 0 | 1 | 0 | 1 | 0 | 1 | 0 | 1 | 0 | 1 | 0 | 1 | 0 | 1 |",
"          +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+",
"          | 00| 01| 02| 03| 04| 05| 06| 07| 08| 09| 0a| 0b| 0c| 0d| 0e| 0f|",
"+---------+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+",
NULL
};


UBYTE *sides[] = {
"0000 00","0001 10","0010 20","0011 30",
"0100 40","0101 50","0110 60","0111 70",
"1000 80","1001 90","1010 a0","1011 b0",
"1100 c0","1101 d0","1110 e0","1111 f0" };

UBYTE UC[] = { 0x20, 0x7f, 0x20, 0x00 }; /* Unprintable Character */

#define NU NULL
UBYTE *special[] =
{  NU,   NU,   NU,   NU,   NU,   NU,   NU,   NU,
 " BS","TAB"," LF"," VT"," FF"," CR"," SO"," SI",
   NU,   NU,   NU,   NU,   NU,   NU,   NU,   NU,
   NU,   NU,   NU, "ESC",  NU,   NU,   NU,   NU
};

UBYTE *special2[] =
{ NU,   NU,   NU,   NU,   NU,"NEL",   NU,   NU,
  NU,   NU,   NU,   NU,   NU," RI",   NU,   NU,
  NU,   NU,   NU,   NU,   NU,  NU,    NU,   NU,
  NU,   NU,   NU,"CSI",   NU,  NU,    NU,   NU
};


void main(int argc, char **argv)
    {
    int k,j;
    BOOL allbox = FALSE;
    UBYTE a;

    if(((argc)&&(argc<MINARGS))||(argv[argc-1][0]=='?'))
	{
	printf("%s\n%s\n",Copyright,usage);
	exit(RETURN_OK);
	}

    if((argc==2)&&(!(stricmp(argv[1],"allbox"))))	allbox = TRUE;

    for(k=0; header[k]; k++)	printf("%s\n",header[k]);

    for(a=0,k=0; k<16; k++)
	{
	printf("| %s |",sides[k]);
	for(j=0; j<16; j++, a++)
	    {
	    if(a <= 0x1F)
		{
		if(special[a])	printf("%s|",special[a]);
		else printf("%s|",UC);
		}
	    else if((a >= 0x80)&&(a < 0xa0))
		{
		if(special2[a-0x80])	printf("%s|",special2[a-0x80]);
		else printf("%s|",UC);
		}
	    else printf(" %lc |",a);
	    }
	if(allbox)	printf("\n%s\n",header[7]);
	else		printf("\n");
        }

    if(!allbox)		printf("%s\n",header[7]);
    exit(RETURN_OK);
    }


