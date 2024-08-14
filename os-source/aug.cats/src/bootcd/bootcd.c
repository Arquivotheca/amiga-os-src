;/* bootcd.c - Execute me to compile me with SAS C 5.10
LC -b1 -cfistq -v -y -j73 bootcd.c
Blink FROM LIB:c.o,bootcd.o,coldreboot.o TO bootcd LIBRARY LIB:LC.lib,LIB:Amiga.lib DEFINE __main=__tinymain
quit

bootcd.c - Linked with coldreboot.o
      Puts up a requester that a reboot is required, asks for
      user confirmation.
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
int CXBRK(void)  { return(0); }  /* Disable Lattice CTRL/C handling */
void chkabort(void) { return; }  /* really */
#endif

extern void ColdReboot(void); /* in coldreboot.asm */

void bye(UBYTE *s, long e);

#define MINARGS 1

UBYTE *vers = "$VER: bootcd 37.2";
UBYTE *Copyright = 
  "bootcd v37.2\nCopyright (c) 1992 Commodore-Amiga, Inc.  All Rights Reserved";

struct IntuitionBase *IntuitionBase = NULL;
BOOL FromWb;
struct IntuiText oktext =     { 0,1,JAM2,6,4,NULL,"  OK  ",NULL };
struct IntuiText notext =     { 0,1,JAM2,6,4,NULL,"CANCEL", NULL };

struct IntuiText body3 =
    { 0,1,JAM2,20,32,NULL,"Select OK to Reboot.",NULL };
struct IntuiText body2 =
    { 0,1,JAM2,20,20,NULL,"Remove Bootable Floppy Disks.",&body3 };
struct IntuiText body1 =
    { 0,1,JAM2,20,8,NULL,"Reboot Required.",&body2 };

void main(int argc, char **argv)
    {
    FromWb = argc ? FALSE : TRUE;

    /* Open the Intuition Library */
    IntuitionBase = (struct IntuitionBase *)
                    OpenLibrary( "intuition.library", 0 );

    if (IntuitionBase == NULL)
        bye("can't open Intuition", RETURN_WARN);

    /* Call the autorequester */
    if(AutoRequest( NULL,&body1,&oktext,&notext,NULL,0,320,80 ))
        {
        ColdReboot();  /* User said OK, so reboot the system */
        }

    /* else just cleanup and exit */
    bye("", RETURN_OK);
}


VOID bye( UBYTE *s, LONG e )
{
    if((*s)&&(!FromWb)) printf("%s\n",s);

    if (IntuitionBase) CloseLibrary( IntuitionBase );
    exit(e);
}

