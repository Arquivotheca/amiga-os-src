;/* zap.c - Execute me to compile me with SAS C 5.10
LC -b1 -cfistq -v -y -j73 zap.c
Blink FROM LIB:c.o,zap.o TO zap LIBRARY LIB:LC.lib,LIB:Amiga.lib
quit
*/

#include <exec/types.h>
#include <exec/memory.h>
#include <libraries/dos.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef LATTICE
int CXBRK(void) { return(0); }  /* Disable Lattice CTRL/C handling */
int chkabort(void) { return(0); }  /* really */
#endif


#define MINARGS 4

UBYTE *vers = "\0$VER: zap 37.1";
UBYTE *Copyright = 
  "zap v37.1\nCopyright (c) 1990 Commodore-Amiga, Inc.  All Rights Reserved";
UBYTE *usage = "Usage: zap filename offset 0xhhhhh... or \"string\"";

void bye(UBYTE *s, int e);
int getval(char *s);

void main(int argc, char **argv)
    {
    ULONG offset, nbytes;
    LONG file;
    UWORD hinib, lonib;
    UBYTE *filename, *changes;
    BOOL  stringarg = TRUE;
    int k1, k2;

    if(((argc)&&(argc<MINARGS))||(argv[argc-1][0]=='?'))
	{
	printf("%s\n%s\n",Copyright,usage);
	bye("",RETURN_OK);
	}


   filename	= argv[1];
   offset	= getval(argv[2]);
   changes 	= argv[3];

   if((changes[0]=='0')&&((changes[1] | 0x20)=='x')) stringarg = FALSE;

   if(stringarg)	nbytes = strlen(changes);
   else
	{
	changes += 2;
	nbytes = strlen(changes);
	if(nbytes & 1)	
		bye("Zap: requires even number of HEX digits\n",RETURN_WARN);

	/* convert the hex ASCII (in place) to hex values */
	for(k1=k2=0; k1 < nbytes; k1++, k2+=2)
		{
		hinib = changes[k2];
		if(hinib > 0x39)  hinib = hinib - 7;
		hinib = hinib & 0x0F;
		lonib = changes[k2+1];
		if(lonib > 0x39)  lonib = lonib - 7;
		lonib = lonib & 0x0F;
		hinib = (hinib << 4) | lonib;
		changes[k1] = hinib;
		}
	nbytes = nbytes >> 1;	/* now half as many bytes */
	}

   if(!(file = Open(filename,MODE_OLDFILE)))
	{
	printf("Zap: can't open file \"%s\"\n",filename);
	bye("",RETURN_WARN);
	}

   if(Seek(file,offset,OFFSET_BEGINNING) >= 0)
	{
	for(k1=0; k1 < nbytes; k1++)	Write(file,&changes[k1],1);
	}
   else printf("Zap: could not seek to desired offset in file\n");

   Close(file);
   bye("",RETURN_OK);
   }

void bye(s,e)
UBYTE *s;
int e;
   {
   if(*s) printf(s);
   exit(e);
   }


int getval(s)
char *s;
   {
   int value, count;

   if((s[1]|0x20) == 'x')  count = stch_i(&s[2],&value);
   else count = stcd_i(&s[0],&value);
   return(value);
   }






