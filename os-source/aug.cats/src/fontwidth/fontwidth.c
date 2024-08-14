;/* fontwidth.c - Execute me to compile me with Lattice 5.04
LC -b1 -cfistq -v -y -j73 fontwidth.c
Blink FROM LIB:c.o,fontwidth.o TO fontwidth LIBRARY LIB:LC.lib,LIB:Amiga.lib
quit
*/

#include <exec/types.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <graphics/text.h>
#include <libraries/dos.h>
#include <libraries/diskfont.h>
#include <intuition/intuition.h>

#ifdef LATTICE
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/diskfont_protos.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
int CXBRK(void) { return(0); }  /* Disable Lattice CTRL/C handling */
int chkabort(void) { return(0); }  /* really */
#endif

char *vers = "\0$VER: fontwidth 37.1";
char *Copyright = 
  "fontwidth v37.1\nCopyright (c) 1990 Commodore-Amiga, Inc.  All Rights Reserved";

int strLen(char *s);
void cleanup(void);
void bye(UBYTE *s,int error);
int getval(char *s);

struct Library *IntuitionBase  = NULL;
struct Library *GfxBase        = NULL;
struct Library *DiskfontBase   = NULL;

UWORD  dpxs[] = { 20, 40, 43, 60, 0 };
  
UBYTE  *fontname = "CGTimes.font";
UWORD  fontsize = 24;
ULONG  fontdpx, fontdpy = 20;
ULONG  devicedpi;

UBYTE  fontstyle = FS_NORMAL | FSF_TAGGED;
UBYTE  fontflags = FPF_DISKFONT;

ULONG  fonttags[] = { TA_DeviceDPI, 0, TAG_DONE };

BOOL   FromWb;

struct TTextAttr tAttr = {0};
struct TextFont *font = NULL;


/* 
 * MAIN 
 */
void main(int argc, char **argv)
   {
   struct Window *win = NULL;
   struct RastPort *rp;
   struct TextFont *oldfont;
   ULONG ypos, yoffs;
   UBYTE buf[80];
   int k;

   FromWb = argc ? FALSE : TRUE;
   if(FromWb) exit(RETURN_WARN);

   if((argc==2)&&(argv[1][0]=='?'))
		printf("usage: fontwidth name.font\n"), exit(0L);

   if(argc==2)
       {
       fontname = argv[1];
       }


   /* Open Libraries */

   if(!(IntuitionBase = OpenLibrary("intuition.library", 0)))
      bye("Can't open intuition library.\n",RETURN_WARN);
      
   if(!(GfxBase = OpenLibrary("graphics.library",0)))
      bye("Can't open graphics library.\n",RETURN_WARN);

   if(!(DiskfontBase = OpenLibrary("diskfont.library",0)))
      bye("Can't open diskfont library.\n",RETURN_WARN);

   if(!(win = OpenWindowTags(NULL,NULL)))
      bye("Can't open window\n",RETURN_FAIL);

   rp = win->RPort;

   printf("%s:\n",fontname); /* forces Lattice's stdio allocation now */


   ypos = 30;
   yoffs = fontsize + 4;

   k = 0;

   for(k=0; fontdpx=dpxs[k]; k++, ypos += yoffs)
	{
   	tAttr.tta_Name  = fontname;
   	tAttr.tta_YSize = fontsize;
   	tAttr.tta_Style = fontstyle;
   	tAttr.tta_Flags = fontflags;

	fonttags[1]	=  devicedpi = ((fontdpx << 16)  | fontdpy);
	tAttr.tta_Tags  = (struct TagItem *)fonttags;

	SetDrMd(rp, JAM2);
	SetAPen(rp,1);
	SetBPen(rp,0);

        if(font=(struct TextFont *)OpenDiskFont(&tAttr))
	    {
	    printf("Asked for %s size %ld, DPI=$%08lx. Got size %ld at $%lx\n",
			fontname, fontsize, devicedpi, font->tf_YSize, font);

	    oldfont = rp->Font;
	    SetFont(rp, font);
	    Move(rp,20,ypos);
	    sprintf(buf,"Is this DPI $%08lx ?",devicedpi);
	    Text(rp,buf,strlen(buf));
	    SetFont(rp,oldfont);				    
	    CloseFont(font);
	    Delay(100);
	    }
	else printf("Can't open %s %d\n",fontname,fontsize);
	}

   Delay(200);
   CloseWindow(win);
   cleanup();
   exit(RETURN_OK);
   }


void bye(UBYTE *s,int error)
   {
   if((*s)&&(!FromWb)) printf(s);
   cleanup();
   exit(error);
   }

void cleanup()
   {
   if(DiskfontBase)  CloseLibrary(DiskfontBase);
   if(GfxBase) CloseLibrary(GfxBase);
   if(IntuitionBase) CloseLibrary(IntuitionBase);
   }

int strLen(char *s)
   {
   int i = 0;
   while(*s++) i++;
   return(i);
   }


int getval(char *s)
   {
   int value, count;

   if((s[1]|0x20) == 'x')  count = stch_i(&s[2],&value);
   else count = stcd_i(&s[0],&value);
   return(value);
   }

