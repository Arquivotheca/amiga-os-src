/*
	SetCPU V1.6
	by Copyright 1989 by Dave Haynie

	MEMORY.C MODULE

	This module is responsible for ROM image allocation, MMU table
	allocation and creation, and other functions based on memory.
*/

#include "setcpu.h"

#define min(a,b)        ((a<b)?a:b)
#define max(a,b)        ((a>b)?a:b)

#define NODEBUG

/* ====================================================================== */

#ifndef KICKFILE
/* This function fills the basic MMU table and other standard tag itemd from
   an allocated image.	Some tables may need additional information in the
   table.  The basic table consists of one table level and uses direct page
   translation on a grain of 128K per entry.  Everything's directly mapped
   except for the last two entries, which are for the $FC0000-$FFFFFF area. */

static void FillBasicTable(tag)
struct systag *tag;
{
   register ULONG i, image;
   ULONG page,size;
   struct ConfigDev *cd;

   tag->tagsize = SizeOf(struct systag);
   tag->progver = PROGRAM_VERSION;
   tag->patches = 0;
   tag->patchlist = tag->devs = NULL;
   tag->config = 1;
   tag->BerrSize = 0L;
   tag->OldBerr = tag->BerrHandler = NULL;

   for (i = 0; i < tag->tablesize/sizeof(ULONG); i++)
      tag->maintable[i] = PD_ADDR(i<<17)|PD_DT_PAGE;

   image = (ULONG)tag->romimage;
   for (i = 128 - tag->romsize/PAGESIZE; i < 128; i++,image += PAGESIZE)
      tag->maintable[i] = PD_ADDR(image)|PD_WP|PD_DT_PAGE;

    /* :TODO: Fails to deal with 512K images */
   image = ((ULONG)tag->romimage)/ROMROUND;
   tag->maintable[image] = IV_ADDR(tag)|PD_DT_INVALID;
   tag->maintable[image+1] = IV_ADDR(tag)|PD_DT_INVALID;


   /* Here I look for Bridge Cards, which are known to have problems with
      the data cache enabled, so I always disable it, reguardless of
      whether caching is actually enabled in the CACR. */

   if (ExpansionBase && ((cd = FindConfigDev(NULL,0x201L,0x01L)) ||
			 (cd = FindConfigDev(NULL,0x201L,0x02L)))) {
      page = (ULONG)cd->cd_BoardAddr/PAGESIZE;
      size = max((ULONG)cd->cd_BoardSize/PAGESIZE,1);
      while (size-- && page < tag->tablesize)
	 if ((tag->maintable[page] & PD_DT_TYPE) == PD_DT_PAGE)
	    tag->maintable[page++] |= PD_CI;
   }

#ifdef DEBUG
   kprintf("\n");
   for (i = 0; i < tag->tablesize/sizeof(ULONG); i++)
      kprintf("[%3ld]=%8lx  ",i,tag->maintable[i]);
   kprintf("\n");
#endif
}

/* This routine sets up the MMU registers (shadowed in tag fields) in the
   standard SetCPU fashion.  The CPU Root Pointer tells the MMU about the
   table I've set up, and all that's left to do is bang the Translation
   Control Register to turn the thing on.  Note that the first half of the
   CRP is control data, the second the address of my table.  */

void SetMMURegs(tag)
struct systag *tag;
{
   tag->CRP[0] = CRP_LIMIT(tag->tablesize/sizeof(ULONG)-1)|CRP_SG|CRP_DT_V4BYTE;
   tag->CRP[1] = (ULONG)(tag->maintable);

   tag->TC = TC_PS(0x0c)|TC_IS(tag->wrapup)|
	     TC_TIA(0x0f-tag->wrapup)|TC_TIB(0x03)|TC_TIC(0x02)|TC_TID(0);
}

/* ====================================================================== */

/* This section contains the memory allocation code.  There are two
   problems here.  First of all, we'd like to use 32 bit FAST memory if
   at all possible.  Next, block translation and page tables must be on at
   least a page sized boundary, if not a block boundary.  */

/* This routine finds the memory block for me to use in AllocAligned().
   It takes into account either A2620 or A2630 systems, where I can snoop
   out the memory for that particular board, knowing it's the fastest.  I
   can adjust for 1.4's automatic memory merging in this case too, to be
   sure I have 32 bit RAM.  If I don't have one of my boards, I'll return
   a pointer to the first non-$C00000 memory list marked as FAST. */

static struct range {
   ULONG first;
   ULONG last;
};

static struct range SRange = { 0L, 0L };
static ULONG MaxMem = 0;

static BOOL SmartlyGetRange() {
   struct ExecBase *eb = *((struct ExecBase **)4L);
   struct ConfigDev *cd;
   register struct MemHeader *mem;

   /* First try for either A2620 or A2630 */

   if (ExpansionBase && ((cd = FindConfigDev(NULL,0x202L,0x50L)) ||
			 (cd = FindConfigDev(NULL,0x202L,0x51L)))) {
      SRange.first = (ULONG)cd->cd_BoardAddr;
      SRange.last = ((ULONG)cd->cd_BoardSize) + SRange.first;
   }

   /* It's apparently another critter */

   for (mem = (struct MemHeader *)eb->MemList.lh_Head; mem->mh_Node.ln_Succ;
	mem = (struct MemHeader *)mem->mh_Node.ln_Succ) {
      if ((ULONG)(mem->mh_Upper) > MaxMem) MaxMem = (ULONG)mem->mh_Upper;
      if (mem->mh_Attributes & MEMF_CHIP) continue;
      if (((ULONG)mem >= 0xc00000 && (ULONG)mem <= 0xc80000)) continue;
      if (!SRange.first) {
	 SRange.first = (ULONG)mem->mh_Lower;
	 SRange.last = (ULONG)mem->mh_Upper;
      }
   }
   if (SRange.first)
      return TRUE;
   return FALSE;
}

/* This routine allocates such an aligned block of memory from a specified
   memory list. */

void *AllocAligned(size,bound)
register ULONG size;
register ULONG bound;
{
   register ULONG target;
   void *mem = NULL;

   Forbid();
   if (!allochead) {
      target = (SRange.last-size) & ~(bound-1);
      while (target > SRange.first && !(mem = AllocAbs(size,target)))
	 target -= bound;
      SRange.last = (ULONG)mem;
   } else {
      target = (SRange.first+size+bound-1) & ~(bound-1);
      while (target < SRange.last && !(mem = AllocAbs(size,target)))
	 target += bound;
      SRange.first = (ULONG)mem+size;
   }
   Permit();
   return mem;
}

/* This routine finds the memory wrap and appropriate MMU table size for
   the given configuration. It requires the value of MaxMem to have been
   already calculated. */

void FindWrap(tag)
struct systag *tag;
{
   ULONG test;

   tag->wrapdown = 0;
   for (test = MaxMem; !(test & 0x80000000) && tag->wrapdown < 8; test <<= 1)
      tag->wrapdown++;
   tag->tablesize = (128 << (8 - tag->wrapdown)) * sizeof(ULONG);
}

/* ====================================================================== */

/* This section contains routines that manage different ROM image types. */

/* This function gets an aligned ROM image copied from system ROM. */

struct systag *AllocROMImage(type)
UWORD type;
{
   struct systag *tag = NULL;
   ULONG *rom = NULL, *table = NULL;

  /* Let's make the allocations.  I allocate the ROM first, then the table,
     then the tag; since we're coming from the end of memory, this should
     result in the least fragging. */

   SmartlyGetRange();
   if (!(rom = AllocAligned(ROMSIZE_256,ROMROUND))) goto fail;
   if (!(tag = AllocAligned(SizeOf(struct systag),8L))) goto fail;
   FindWrap(tag);
   if (!(table = AllocAligned(tag->tablesize,TABROUND))) goto fail;

   tag->maintable = table;
   tag->romimage = rom;
   tag->romtype = type;
   tag->romsize = ROMSIZE_256;
   tag->romused = ROMSIZE_256;
   FillBasicTable(tag);
   CopyMem(ROM_END-ROMSIZE_256,tag->romimage,ROMSIZE_256);
   return tag;

fail:
   if (rom)   FreeMem(rom,ROMSIZE_256);
   if (table) FreeMem(table,tag->tablesize);
   if (tag)   FreeMem(tag,SizeOf(struct systag));
   return NULL;
}
#endif

/* This function allocates a ROM image from a KickStart disk for KICKROM. */


#define ColorShift(x)   (LONG)((((x)&0xf)+1)%15)

#ifdef KICKFILE
struct systag *AllocKSImage(name)
#else
struct systag *AllocKSImage(type,name)
UWORD type;
#endif
char *name;
{
   struct systag *tag = NULL;
   ULONG *rom = NULL, *table = NULL, blocks,size;
   LONG unit, i, r = 0x0004L, g = 0x0008L, b = 0x000cL;
   char *buf = NULL, *ptr, *rombase;
   struct MsgPort *port = NULL;
   struct IOStdReq *req = NULL;
#ifndef KICKFILE
   struct Window *hand = NULL;
#endif
   BOOL devok = FALSE;

 /* Then we expect it to be a kickstart disk, so we check out the device,
    the disk, etc.  If all goes well, we'll be in business. */

   if ((unit = CheckTDDev(name)) == -1L) {
#ifndef KICKFILE
      LoadErr = 16;
#endif
      goto fail;
   }

  /* This is the stuff that need opening... */
   port = (struct MsgPort *) CreatePort("diskreq.port",0L);
   if (port) req = CreateStdIO(port,0L);
   if (req) devok = !OpenDevice("trackdisk.device",unit,(struct IORequest *)req,0L);
   if (devok) buf = (char *)AllocMem(512L,MEMF_CHIP);
   if (!port || !req || !devok || !buf) goto fail;

  /* Make sure we're a kick disk. */
#ifdef KICKFILE
   printf("Place KickStart Disk in %s:\n",name);
#else
   hand = CoolHand();
#endif
   while ((ReadBuf(buf,0L,req) != READOK) || !strniequ("KICK",buf,4))
      Delay(150L);

  /* Is it a magic big kickstart? */

   if (!(tag = AllocMem(SizeOf(struct systag),0L))) goto fail;

   if (strniequ("SIZE",&buf[4],4)) {
      size = tag->romused = *((ULONG *)&buf[8]);
      blocks = (tag->romused+511L)/512L;
      tag->romsize = ((tag->romused-1)/ROMROUND+1)*ROMROUND;
   } else {
      blocks = 512L;
      size = tag->romused = ROMSIZE_256;
      tag->romsize = ROMSIZE_256;
   }
   if (!(rom = AllocMem(tag->romsize,0L))) goto fail;
   tag->romimage = rom;

  /* Looks like we're in shape to get the actual system. */

#ifndef KICKFILE
   SetRGB4(&(hand->WScreen->ViewPort),2L,r,g,b);
#endif
   ptr = ((char *)tag->romimage) + (tag->romsize - tag->romused);
   for (i = 1; i <= blocks; ++i) {
      if (ReadBuf(buf,i,req) != READOK) break;
      CopyMem(buf,ptr,min(size,512L));
      ptr += min(size,512L);
      size -= 512L;
#ifndef KICKFILE
      r = ColorShift(r);
      g = ColorShift(g);
      b = ColorShift(b);

      SetRGB4(&(hand->WScreen->ViewPort),2L,r,g,b);
#endif
   }
#ifndef KICKFILE
   LoadErr = 0;
#endif

done:
   if (buf) FreeMem(buf,512L);

   if (devok) {
      MotorOff(req);
      CloseDevice((struct IORequest *)req);
   }
   if (req)  DeleteStdIO(req);
   if (port) DeletePort(port);
   return tag;

fail:
   if (rom) FreeMem(rom,tag->romsize);
   if (tag) {
      if (table) FreeMem(table,tag->tablesize);
      FreeMem(tag,SizeOf(struct systag));
      tag = NULL;
   }
   goto done;
}

#ifndef KICKFILE

/* This function allocates a ROM image from an AmigaDOS file for either
   KICKROM or FASTROM. */

struct systag *AllocFILEImage(type,name)
UWORD type;
char *name;
{
   struct systag *tag = NULL;
   ULONG *rom = NULL, *table = NULL, space[2], size, used;
   BPTR file = NULL;
   struct FileInfoBlock *fib;

  /* First off, let's check out this here file. */

   if (!(fib = AllocMem(SizeOf(struct FileInfoBlock),0L))) goto fail;
   if (!(file = Lock(name,ACCESS_READ))) return NULL;
   Examine(file,fib);
   UnLock(file);
   file = NULL;
   if (fib->fib_DirEntryType > 0L) goto fail;
   if (!(file = Open(name,MODE_OLDFILE))) goto fail;

   Read(file,(char *)space,8L);
   if (space[0]) {
      size = used = ROMSIZE_256;
   } else {
      used = space[1];
      size = ((used-1)/ROMROUND+1)*ROMROUND;
      Read(file,(char *)space,8L);
   }

  /* First, let's allocate the space I need.  KICKROMs don't use a disk
     image directly, so they don't need an MMU table and don't care about
     any special alignment.  FASTROMs read from disk do care, so I will
     align this image. */

   SmartlyGetRange();
   if (!(rom = AllocAligned(size,ROMROUND))) goto fail;
   if (!(tag = AllocAligned(SizeOf(struct systag),8L))) goto fail;

   tag->romimage = rom;
   tag->romsize = size;
   tag->romused = used;
   rom += (size-used)/4;
   rom[0] = space[0]; rom[1] = space[1];

   if ((tag->romtype = type) == ROM_FAST) {
      FindWrap(tag);
      if (!(table = AllocAligned(tag->tablesize,TABROUND))) goto fail;
      tag->maintable = table;
      FillBasicTable(tag);
   }

   Read(file,(char *)(rom+2),used);
   Close(file);
   FreeMem(fib,SizeOf(struct FileInfoBlock));
   return tag;

fail:
   if (file) Close(file);
   if (fib) FreeMem(fib,SizeOf(struct FileInfoBlock));
   if (rom) FreeMem(rom,size);
   if (tag) {
      if (table) FreeMem(table,tag->tablesize);
      FreeMem(tag,SizeOf(struct systag));
   }
   return NULL;
}

/* This function gets a ROM image from disk; in temporary unaligned memory
   for a KICKROM image, permanent aligned memory for a FASTROM image. */

struct systag *AllocDISKImage(type,name)
UWORD type;
char *name;
{

  /* Let's check out this here file.  It's easy to tell; we want a FILE
     image if we aren't passed a KICK_ROM request with a device name. */

  if (type == ROM_FAST || name[strlen(name)-1] != ':')
     return AllocFILEImage(type,name);

  return AllocKSImage(type,name);
}

/* This function gets an aligned, reset-safe image in either $00C00000 or
   CHIP memory, copies the ROM code from the passed temporary image, and
   then sets up it's MMU table such that the memory used for the safe image
   will be missed by the Amiga's memory-sizing logic on reboot. */

struct systag *AllocSAFEImage(temp)
struct systag *temp;
{
   ULONG i;
   struct ExecBase *eb = *((struct ExecBase **)4L);
   struct MemHeader *safe, *safeC000 = NULL, *safeCHIP = NULL, *tmp;
   struct systag *tag;
   ULONG phantom,upper,xindex;

   for (safe = (struct MemHeader *)eb->MemList.lh_Head; safe->mh_Node.ln_Succ;
	safe = (struct MemHeader *)safe->mh_Node.ln_Succ) {
      tmp = (struct MemHeader *)safe;
      if ((ULONG)(tmp->mh_Upper) > MaxMem) MaxMem = (ULONG)tmp->mh_Upper;
      if (tmp->mh_Attributes & MEMF_CHIP) {
	 if (!safeCHIP || safeCHIP->mh_Upper < tmp->mh_Upper)
	    safeCHIP = tmp;
      } else if ((ULONG)safe >= 0xc00000 && (ULONG)safe <= 0xc80000) {
	 if (!safeC000 || safeC000->mh_Upper < tmp->mh_Upper)
	    safeC000 = tmp;
      }
   }

   /* We do pretty much the same thing whether we're using $C00000 or
      CHIP to hold the ROM code.  The main difference is that we want
      to make the $C00000 phantom image reference chip registers, while
      the CHIP phantom image needs to reference empty space -- I'll
      use expansion space for that. */

   if (safeC000) {
      upper = ((ULONG)safeC000->mh_Upper+ROMROUND-1L) & ~(ROMROUND-1L);
      phantom = 0x00ce0000;
   } else if (safeCHIP) {
      if (safeCHIP->mh_Upper < 0x0c0000L) return NULL;
      upper = ((ULONG)safeCHIP->mh_Upper+ROMROUND-1L) & ~(ROMROUND-1L);
      phantom = 0x00800000;
   }

   FindWrap(temp);

   tag = (struct systag *)((upper - ROMROUND + temp->tablesize + 7) & ~7L);
   tag->tablesize = temp->tablesize;
   tag->wrapdown = temp->wrapdown;
   tag->maintable = (ULONG *)(upper - ROMROUND);
   tag->romimage = (ULONG *)(upper - temp->romsize - ROMROUND);
   tag->romtype = ROM_KICK;
   tag->romsize = temp->romsize;
   tag->romused = temp->romused;


   FillBasicTable(tag);
   CopyMem(temp->romimage,tag->romimage,temp->romsize);
   tag->ResetCode = (char *)((((ULONG)tag)+sizeof(struct systag)+15) & ~7L);
   CopyMem(BootCode,tag->ResetCode,BootCodeSize);
   xindex = (ULONG)(tag->romimage)/ROMROUND;
   tag->maintable[xindex] = PD_ADDR(phantom)|PD_DT_PAGE;

#ifdef DEBUG
   kprintf("\n");
   for (i = 0; i < tag->tablesize/sizeof(ULONG); i++)
      kprintf("[%3ld]=%8lx  ",i,tag->maintable[i]);
   kprintf("\n");
#endif

   return tag;
}

/* This function can be used after rebooting to remove the specially allocated
   SAFE image.	This is normally used after copying the safe image into a
   standard FASTROM image and adjusting the MMU accordingly.  The SAFE RAM
   at this point isn't in any memory list, so we aren't really freeing it,
   just linking it into a list. */

void FreeSAFEImage(kick)
struct systag *kick;
{
   if ((ULONG)kick->romimage >= 0xc00000 && (ULONG)kick->romimage <= 0xc80000)
      AddMemList(ROMROUND+kick->romsize,MEMF_PUBLIC,-15L,kick->romimage,NULL);
   else
      AddMemList(ROMROUND+kick->romsize,MEMF_CHIP|MEMF_PUBLIC,-15L,kick->romimage,NULL);
}

/* ====================================================================== */

/* This routine gets the system tag from the patched system, if it's
   there.  This tag is stored in an invalid table entry that's within the
   physical ROM image.	If the MMU isn't on, I assume there's no system
   tag. */

struct systag *GetSysTag() {
   ULONG i, myCRP[2], *table = NULL;

   if (!(GetTC() & TC_ENB)) return NULL;

   GetCRP(myCRP);
   table = (ULONG *)myCRP[1];
   for (i = 0; i < 4096; ++i)
      if ((table[i] & PD_DT_TYPE) == PD_DT_INVALID && IV_ADDR(table[i]))
	 return (struct systag *)IV_ADDR(table[i]);

   return NULL;
}

#endif
