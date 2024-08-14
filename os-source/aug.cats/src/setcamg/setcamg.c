;/* setcamg.c - Execute me to compile me with SAS C 6.x
SC setcamg.c data=near nominc strmer streq nostkchk saveds ign=73
Slink FROM LIB:c.o,setcamg.o TO setcamg LIBRARY LIB:SC.lib,LIB:Amiga.lib ND
quit
*/

#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <intuition/intuition.h>
#include <iffp/ilbm.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef __SASC
void __regargs __chkabort(void) {}      /* Disable SAS CTRL-C checking. */
#else
#ifdef LATTICE
void chkabort(void) {}                  /* Disable LATTICE CTRL-C checking */
#endif
#endif

#define MINARGS 3

UBYTE *vers = "\0$VER: setcamg 40.1";
UBYTE *Copyright = 
  "setcamg v40.1\n(c) Copyright 1993 Commodore-Amiga, Inc.  All Rights Reserved";
UBYTE *usage = 
"Usage: setcamg ilbmname flag [flag flag flag...]\n"
"Flags: HIRES LORES LACE HAM HALFBRITE SUPERHIRES\n"
"       PAL NTSC PRODUCTIVITY VGA EURO72 DBLPAL DBLNTSC\n"
"Example: setcamg mypicture HIRES HAM LACE\n";

void bye(UBYTE *s, int e);
void cleanup(void);

LONG file;

void main(int argc, char **argv)
    {
    ULONG stuff[4], modeid = 0L;
    LONG rlen, error=0L;
    int k;
    BOOL Done;

    if(((argc)&&(argc<MINARGS))||((argc>1)&&(argv[argc-1][0]=='?')))
	{
	printf("%s\n%s\n",Copyright,usage);
	bye("",RETURN_OK);
	}

    if(!(file = Open(argv[1],MODE_OLDFILE)))
	{
	printf("Can't open file: %s\n",argv[1]);
	bye("",RETURN_WARN);
	}

    Read(file,stuff,12);
    if((stuff[0] != ID_FORM)||(stuff[2] != ID_ILBM))
        {
	bye("Not a FORM ILBM\n",RETURN_WARN);
	}

    for(k=2; k<argc; k++)
	{
	if(!(stricmp(argv[k],"hires")))			modeid |= HIRES;
	else if(!(stricmp(argv[k],"lores")))		modeid |= 0L;
	else if(!(stricmp(argv[k],"lace")))		modeid |= LACE;
	else if(!(stricmp(argv[k],"ham")))		modeid |= HAM;
	else if(!(stricmp(argv[k],"halfbrite")))	modeid |= EXTRA_HALFBRITE;
	else if(!(stricmp(argv[k],"extrahalfbrite")))	modeid |= EXTRA_HALFBRITE;
	else if(!(stricmp(argv[k],"superhires")))	modeid |= SUPERHIRES;
	else if(!(stricmp(argv[k],"productivity")))	modeid |= VGAPRODUCT_KEY;
	else if(!(stricmp(argv[k],"vga")))	modeid |= VGA_MONITOR_ID;
	else if(!(stricmp(argv[k],"euro72")))	modeid |= EURO72_MONITOR_ID;
	else if(!(stricmp(argv[k],"pal")))	modeid |= PAL_MONITOR_ID;
	else if(!(stricmp(argv[k],"ntsc")))	modeid |= NTSC_MONITOR_ID;
	else if(!(stricmp(argv[k],"dblntsc")))	modeid |= DBLNTSC_MONITOR_ID;
	else if(!(stricmp(argv[k],"dblpal")))	modeid |= DBLPAL_MONITOR_ID;
	else printf("unknown flag %s\n",argv[k]);
	}

    Done = FALSE;
    while(!Done)
	{
	if((rlen = Read(file,stuff,8)) == 8)
	    {
	    if(stuff[0] == ID_CAMG)
		{
		if(4 == Write(file,&modeid,4))
		    {
		    printf("Set CAMG to $%08lx\n",modeid);
		    }
		else
		    {
		    printf("Write error: %ld\n",IoErr());
		    error = RETURN_FAIL;
		    }
		Done = TRUE;
		}
	    else	/* not CAMG */
		{
		Seek(file,stuff[1],OFFSET_CURRENT);
		if(stuff[1] & 0x01)  Seek(file,1,OFFSET_CURRENT);
		}
	    }
	else 	/* EOF */
	    {
	    printf("EOF - CAMG not found\n");
	    error = RETURN_FAIL;
	    Done = TRUE;
	    }
	}
    bye("",error);
    }


void bye(UBYTE *s, int e)
    {
    cleanup();
    exit(e);
    }

void cleanup()
    {
    if(file) Close(file);
    }

