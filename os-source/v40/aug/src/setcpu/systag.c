/*
	SetCPU V1.5
	by Copyright 1989 by Dave Haynie

	SYSTAG.C MODULE
	
	This module is responsible for maintaining the SetCPU system tag
	structure, which is a global structure I keep in the MMU table to
	help describe the actual ROM image being used. */
*/

#include "setcpu.h"

/* ====================================================================== */

/* This routine gets the system tag from the patched system, if it's
   there.  This tag is stored in an invalid table entry that's within the
   physical ROM image.  If the MMU isn't on, I assume there's no system
   tag. */
   
struct systag *GetSysTag() {
   ULONG i, myCRP[2], *MMUTable = NULL;
   
   if (!(GetTC() & TC_ENB)) return NULL;

   GetCRP(myCRP);
   MMUTable = (ULONG *)myCRP[1];
   for (i = 0; i < 128; ++i)
      if ((MMUTable[i] & PD_DT_TYPE) == PD_DT_INVALID)
         return (struct systag *)IV_ADDR(MMUTable[i]);

   return NULL;
}

/* This allocates a system tag structure, including the fields for MMU table
   and ROM image, and the default MMU table structure. */

struct systag *AllocSystem(type)
UWORD type;
{
   struct systag *tag;
   ULONG i, image;
   
 /* First I want the simple stuff. */
  if (!(tag = AllocMem(SizeOf(struct systag),MEMF_CLEAR))) return NULL;
  tag->tagsize = SizeOf(struct systag);
  tag->progver = PROGRAM_VERSION;
  tag->romtype = type;

  /* Next, get the memory for the 32 bit ROM and the MMU table. */
   tag->romimage = AllocAligned(ROMSIZE,ROMROUND);
   tag->maintable = AllocAligned(MAINTABSIZE,TABROUND);
   if (!tag->romimage || !tag->maintable) {
      if (tag->maintable) FreeMem(tag->maintable,MAINTABSIZE);
      if (tag->romimage)  FreeMem(tag->romimage,ROMSIZE);
      FreeMem(tag,tag->tagsize);
      return NULL;
   }

  /* Here I initialize the MMU table.  This translation is really 
     basic.  I set up one table level and use direct page translation
     on a grain of 128K per entry.  Everything's directly mapped except
     for the last two entries, which is for the $FC0000-$FFFFFF area.
     This I translate to my fastram ROM, and write protect it too. */

   for (i = 0; i < 126; i++) tag->maintable[i] = PD_ADDR(i<<17)|PD_DT_PAGE;
   tag->maintable[126] = PD_ADDR(tag->romimage)|PD_WP|PD_DT_PAGE;
   tag->maintable[127] = PD_ADDR(((ULONG)tag->romimage)+ROMROUND)|PD_WP|PD_DT_PAGE;
   image = ((ULONG)tag->romimage)/ROMROUND;
   tag->maintable[image] = IV_ADDR(tag)|PD_DT_INVALID;
   tag->maintable[image+1] |= PD_WP;

   return tag;
}


