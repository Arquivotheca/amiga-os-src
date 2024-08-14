;/* fontlist.c - Execute me to compile me with Lattice 5.04
LC -b1 -cfistq -v -y -j73 fontlist.c
Blink FROM LIB:c.o,fontlist.o TO fontlist LIBRARY LIB:LC.lib,LIB:Amiga.lib
quit
*/

#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/libraries.h>
#include <graphics/gfxbase.h>
#include <graphics/text.h>

#include <clib/exec_protos.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef LATTICE
int CXBRK(void) { return(0); }		/* Disable Lattice CTRL/C handling */
int chkabort(void) { return(0); }	/* really */
#endif

char *vers = "\0$VER: fontlist 36.10";
char *Copyright = 
  "fontlist v36.10\nCopyright (c) 1990 Commodore-Amiga, Inc.  All Rights Reserved";

#define ARRAYSIZE 256L

void main(int argc,char **argv)
    {
    struct Library *lib; 
    struct GfxBase *gbase;
    struct TextFont *font, *fonts[ARRAYSIZE];   
    ULONG count = 0L, k;

    
    if(argc > 1) printf("%s\n",Copyright), exit(0L);

    if(!(lib = OpenLibrary("graphics.library",0)))  exit(20);
    gbase = (struct GfxBase *)lib;

    Forbid();
    /* Note - printing within Forbid() would break the forbidden state */
    for ( font = (struct TextFont *)gbase->TextFonts.lh_Head;
          NULL != font->tf_Message.mn_Node.ln_Succ;
          font = (struct TextFont *)font->tf_Message.mn_Node.ln_Succ)
        {
        if (count < ARRAYSIZE) fonts[count++] = font;
        }
    Permit();

    printf("Fonts currently in system:\n");

    for (k=0; k<count; k++)
	{
	font = fonts[k];
	printf(" at $%08lx  Name:%s  YSize=%ld Flags=$%02x Style=$%02x\n",
	    font, font->tf_Message.mn_Node.ln_Name,
		  font->tf_YSize, font->tf_Flags, font->tf_Style);
	}

    if (count == ARRAYSIZE) printf("Error: array overflow\n");
    }
