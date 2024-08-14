/*
	SetCPU V1.6
	by Copyright 1989 by Dave Haynie

	PATCHES.C MODULE

	This module is responsible for managing ROM image patchs.
*/

#include "setcpu.h"

/* ====================================================================== */

/* The "JSR address" opcode used for my patches. */

#define JSR_OPCODE	0x4eb9

/* These are for some string patches */

/*
static char DiskS[] = "SALV to recover it! ";
static char DiskS[] = "Use DISKDOCTOR to correct it";
static char DiskS[] = "Use a disk repair utility  "; */
static char DiskS[] = "Use a disk utility to fix it";
/* Trackdisk patch added 10-27-89 -bryce */
static UWORD TrackP[]=0xb0bc; /* Change cmp.l $8000,d0 to cmp.l #$8000,d0 */

/* These are the V1.3 patches */

struct pitem PL_345[] = {
   { PT_KEYBOARD,0,0x2528a,   0L, NULL },	       /* Keyboard... */
   { PT_STRING,  0,0x3fe19-8,28L, (UWORD *)DiskS },   /* "DiskDoctor" name */
   { PT_STRING,  0,0x2af9c  , 2L, (UWORD *)TrackP }, /* Trackdisk RAW fix */
   { PT_END,	 0,	 0  , 0L, NULL }
};

struct pitem PL_33180[] = {
   { PT_KEYBOARD,0,0x2572a,  0L, NULL },		/* Keyboard... */
   { PT_STRING,  0,0x3fe11-8, 28L, (UWORD *)DiskS },  /* "DiskDoctor" name */
   { PT_STRING,  0,0x2b31c  , 2L, (UWORD *)TrackP }, /* Trackdisk RAW fix */
   { PT_END,	 0,	 0,  0L, NULL }
};

/* This is main system patch list. */

struct patch SystemPatch[] = {
   { &SystemPatch[1], &PL_345[0],34,5 },
   { NULL, &PL_33180[0],33,180 }
};

/* This is a pointer to the base of the last applied patch, for making
   patch links.  The structure of any applied allocated patch, such as
   a JSR patch, is a link pointer, the actual patch size, and then the
   patch code.	This lets any version of SetCPU remove allocated patches
   from any other version, providing it can locate the tag. */

struct MemChunk *lastpatch = NULL;

/* This routine applys the patch */

BOOL AddPatch(rom,lst,tag)
ULONG rom;
struct patch *lst;
struct systag *tag;
{
   UWORD *kickver = (UWORD *)(0x100000C - ( * (ULONG *) 0xFFFFEC ));
   UWORD *addr, *base;
   struct pitem *item;
   struct MemChunk *chunk;
   BOOL any = FALSE;

   while (lst) {
      if (lst->Version == kickver[0] && lst->Revision == kickver[1]) {
	 for (item = &lst->list[0]; item->Type != PT_END; ++item) {
	    any = TRUE;
	    switch (item->Type) {
	       case PT_IGNORE :
		  break;
	       case PT_STRING :
		  CopyMem(item->Code,(char *)(rom+item->Offset),item->Length);
		  ++tag->patches;
		  break;
	       case PT_JSR    :
		  chunk = (struct MemChunk *)AllocMem(item->Length+14L,0L);
		  chunk->mc_Next = lastpatch;
		  lastpatch = chunk;
		  chunk->mc_Bytes = item->Length+14L;
		  addr = (UWORD *)(((ULONG)chunk)+8L);
		  base = (UWORD *)(rom+item->Offset);
		  CopyMem(base,addr,6L);
		  base[0] = (UWORD)JSR_OPCODE;
		  base[1] = (UWORD)(((ULONG)addr)>>16L);
		  base[2] = (UWORD)(((ULONG)addr)&0x0ffff);
		  CopyMem(item->Code,&addr[3],item->Length);
		  ++tag->patches;
		  break;
	    }
	 }
      }
      lst = lst->next;
   }
   return any;
}
