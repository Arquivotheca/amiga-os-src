/*
	SetCPU V1.6
	by Copyright 1989 by Dave Haynie

	MAIN PROGRAM

	This program is a CPU identification and MMU support tool for the
	Amiga operating system.  It will identify various CPU system
	elements, and allow decisions to be made based on those elements
	in script files.  It will also use the MMU on systems so equipped
	to translate system ROM, or alternate system ROMs, into system
	memory, with preference given to any 32 bit memory it might be
	able to detect.
*/

#define MAIN_MODULE

#include "setcpu.h"

/* ====================================================================== */

static BOOL tags[CHECKS] =
   { FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE };

USHORT code = 0L;				/* Program return code */

/* Global options */

BOOL	verbose 	= FALSE,	/* Display lots of info? */
	helpmode	= FALSE,	/* Do we just want help? */
	handler 	= TRUE, 	/* Install trap handler? */
	allochead	= FALSE;	/* Allocate from head of memory */

short	fastrom 	= 0,		/* Set up the FASTROM? */
	wrapbits	= 8;		/* How does the MMU wrap addresses? */
char	*romfile	= NULL; 	/* Where to look for a kick image */

/* Do we want autoconfig?  I think a default level 2 makes more sense. */

short	configlevel	= 2;
BOOL	configset	= FALSE;

/* ====================================================================== */

/* This is the termination routine, which also displays numbered errors. */

void quit(err)
int err;
{
   if (err >= 10) printf("Error: ");
   switch(err) {
      case 1:
	 printf("Usage: SetCPU [INST|DATA] [[NO]CACHE|[NO]BURST] [TRAP n] [HEAD]\n");
	 printf("              [KICKROM path|dfN: [DELAY n]] [VERBOSE] [CONFIG n]\n");
	 printf("              [[NO]FASTROM [path] [KEYPATCH n]] [CARDROM path]\n");
	 printf("              [CHECK 680x0|68851|6888x|MMU|FPU]\n");
	 break;
      case 11: printf("Invalid numeric parameter value\n");     break;
      case 12: printf("Illegal Command Line Option\n");         break;
      case 13: printf("KICKROM translation can't be removed\n");break;
      case 14: printf("Can't get memory for FASTROM\n");        break;
      case 15: printf("Can't locate specified file\n");         break;
      case 16: printf("Invalid device/filename specified\n");   break;
      case 17: printf("KEYPATCH requires FASTROM\n");           break;
      case 18: printf("Can't get memory for KICKROM\n");        break;
      case 19: printf("Option requires a file name argument\n");break;
      case 20: printf("System is already FASTKICKed\n");        break;
      case 21: printf("File/ROM version mismatch, use KICKROM\n"); break;
      default:	break;
   }

   if (ExpansionBase) CloseLibrary(ExpansionBase);
   if (err > 10) exit(10);
   exit(err);
}

/* This routine prints FPU codes and sets things accordingly. */

static void PrintFPU(fpu)
ULONG fpu;
{
   if (fpu == 68881L) {
      printf("68881 ");
      if (tags[CK68881]) code = 0;
   } else if (fpu == 68882L) {
      printf("68882 ");
      if (tags[CK68882]) code = 0;
   }
   if (fpu && tags[CKFPU]) code = 0;
}

/* ====================================================================== */

/* This routine installs the given valid rom image as a fast ROM.  It will
   install any I/O translations that are appropriate, and also apply the
   system patch list to the ROM image, before engaging the MMU. */

static BOOL CreateFastROM(tag,wrapbits)
struct systag *tag;
short wrapbits;
{
   ULONG i, *SUBTable, iospace, *VBR;
   struct ExpROMData *er;

   if (!tag) return FALSE;

   tag->wrapup = (wrapbits < tag->wrapdown) ? wrapbits : tag->wrapdown;

  /* Here I'll add in subtables for any I/O devices that we've been told
     about, that I can locate. */

   while (er = GetExpROM()) {
      er->next = tag->devs;
      tag->devs = er;

      iospace = ((ULONG)er->ROMbase)/ROMROUND;

     /* If necessary, modify the table */
      if (tag->maintable[iospace] == PD_ADDR(iospace<<17)|PD_DT_PAGE) {
	 if (SUBTable = AllocAligned(SUBTABSIZE,TABROUND));
	 for (i = 0; i < 8; ++i)
	    SUBTable[i] = PD_ADDR((iospace<<17)+(i<<14))|PD_DT_PAGE;
	 tag->maintable[iospace] = PD_ADDR(SUBTable)|PD_DT_V4BYTE;
      } else
	 SUBTable = (ULONG *)PD_ADDR(tag->maintable[iospace]);

      if (SUBTable) {
	 er->imagebase = (ULONG)AllocAligned(er->ROMsize,DEVROUND);
	 er->tablebase = (ULONG)&SUBTable[0];
	 CopyMemQuick(er->ROMbase,er->imagebase,er->ROMsize);
	 iospace = (er->ROMbase - (iospace * ROMROUND))/DEVROUND;
	 for (i = 0; i < er->ROMsize/DEVROUND; ++i)
	    SUBTable[iospace+i] = PD_ADDR(er->imagebase+(DEVROUND*i))|PD_DT_PAGE;
      }
   }

  /* Here I apply the patches to this ROM. */

   AddPatch((UWORD *)tag->romimage,SystemPatch,tag);
   tag->patchlist = lastpatch;

  /* How 'bout that exception handler.  I get the Vector Base Register,
     save the old vector, allocate space for the new one, copy it from my
     SetCPU code, and assign it.  The tag keeps track of all of this so
     that I can easily remove the whole thing if FASTROM is turned off. */

   if (handler) {
      VBR = GetVBR();
      tag->OldBerr = (char *)VBR[2];
      tag->BerrSize = BerrCodeSize;
      tag->BerrHandler = (char *)AllocMem(tag->BerrSize,0L);
      CopyMem(BerrCode,tag->BerrHandler,tag->BerrSize);
      VBR[2] = (ULONG)tag->BerrHandler;
   }

  /* Finally, bang the MMU and get outta here */

   SetMMURegs(tag);
   Disable();
   SetMMUTag(tag);
   Enable();
   return TRUE;
}

/* This routine creates a kickable ROM, based on the image in the systag.
   If the Kick ROM can't be allocated, the routine returns FALSE, otherwise,
   TRUE. */

static BOOL CreateKickROM(temptag,wrapbits)
struct systag *temptag;
short wrapbits;
{
   struct systag *tag;

   if (!temptag) return FALSE;

  /* We disable so that nothing else is running; the RAMBoot() routine
     counts on the instruction cache, so we might as well turn that on
     now as well. */
   Forbid();
   Disable();


  /* Now I need a new tag, based on safe memory.  Note that calling the safe
     allocator will very likely crash the system if we're not disabled. */

   /* kprintf("----About to allocate SAFE image----\n"); */
   if (!(tag = AllocSAFEImage(temptag))) {
      Enable();
      return FALSE;
   }

   tag->wrapup = (wrapbits < tag->wrapdown) ? wrapbits : tag->wrapdown;

   if (configlevel == 1 || configlevel == 0) {
      tag->maintable[0xe80000L/ROMROUND] = PD_ADDR(0x800000)|PD_DT_PAGE;
      tag->config = FALSE;
   }

   SetKickJumpAddress(0x01000000L-tag->romsize+2);
   SetMMURegs(tag);
   /* kprintf("----About to set RAMBoot----\n"); */
   RAMBoot(tag,0x01000000L-tag->romsize+2);
   return FALSE;
}


/* This routine removes the Fast ROM, and re-claims the memory previously
   allocated.  We've already checked to make sure that the MMU was
   switched on. */

static void DeleteFastROM(tag)
struct systag *tag;
{
   ULONG i, *VBR;
   struct MemChunk *mem, *del;
   struct ExpROMData *emem, *edel;

  /* First off, turn off the MMU.  This lets us muck with the table and
     reclaim memory without any trouble. */

   if (tag->romtype != ROM_FAST) quit(13);
   SetTC(0L);

  /* First I free any device data and/or images that were allocated. */
   emem = tag->devs;
   while (emem) {
      edel = emem;
      emem = emem->next;
      FreeMem(edel->imagebase,edel->ROMsize);
      FreeMem(edel->name,(ULONG)strlen(edel->name)+1);
      FreeMem(edel,SizeOf(struct ExpROMData));
   }

  /* Now I delete the patches for any patch lists I've got.  They only get
     deleted if they've been applied. */
   mem = tag->patchlist;
   while (mem) {
      del = mem;
      mem = mem->mc_Next;
      FreeMem(del,del->mc_Bytes);
   }

  /* Remove the bus error handler, if there is one. */
   if (tag->BerrHandler) {
      VBR = GetVBR();
      VBR[2] = (ULONG)tag->OldBerr;
      FreeMem(tag->BerrHandler,tag->BerrSize);
   }

  /* Now I just free up table and ROM image memory, and I'm done!  I can free
     the table I built for any subtable here, and then the main table and
     image. */

   for (i = 0; i < 128; ++i)
      if ((tag->maintable[i] & PD_DT_TYPE) == PD_DT_V4BYTE)
	 FreeMem(PD_ADDR(tag->maintable[i]),SUBTABSIZE);

   FreeMem(tag->maintable,tag->tablesize);
   if (tag->tagsize <= 66L)
      FreeMem(tag->romimage,ROMSIZE_256);
   else
      FreeMem(tag->romimage,tag->romsize);
   FreeMem(tag,tag->tagsize);
}

/* This routine displays the ROM image information. */

static void PrintFastROM() {
   struct systag *tag;
   struct ExpROMData *dev;

   if (!(tag = GetSysTag())) return;

   printf("KERNEL: (IMAGE: $%6lx) (SIZE: %ldK) (TABLE: $%6lx) (SIZE: %ld) (WRAP: %d)\n",
	  tag->romimage,tag->romsize/1024,tag->maintable,tag->tablesize/4,tag->wrapup);

  /* Now we'll explain any device stuff that's here. */
   for (dev = tag->devs; dev; dev = dev->next) {
      printf("DEVICE: (IMAGE: $%6lx) (TABLE: $%6lx) (NAME: %s)\n",
	     dev->imagebase, dev->tablebase, dev->name);
   }
}

/* Basic "Do I do configuration" logic */

void SetupConfig(tag)
struct systag *tag;
{
   if (configlevel > 0) {
      tag->maintable[0xe80000L/ROMROUND] = PD_ADDR(0xe80000L)|PD_DT_PAGE;
      if (configlevel > 1) SafeConfigDevs();
   }
}

/* ====================================================================== */

/* Codes for the FASTROM action. */

#define FR_NO_ACTION	0
#define FR_DELETE	1
#define FR_MKFAST	ROM_FAST
#define FR_MKKICK	ROM_KICK

/* Command line tokens */

static char *CLITokens[] =
   { "68000","68010","68020","68030","68851","68881","68882","FPU","MMU",
     "CHECK", "FASTROM", "NOFASTROM", "NOPATCH", "KEYPATCH","TRAP", "DATA",
     "INST", "CACHE", "NOCACHE", "BURST", "NOBURST", "VERBOSE", "KICKROM",
     "CARDROM", "CONFIG", "HEAD", "DELAY", "?" };

#define CT_CHECK	CHECKS+0
#define CT_FASTROM	CHECKS+1
#define CT_NOFROM	CHECKS+2
#define CT_NOPATCH	CHECKS+3
#define CT_KEYPAT	CHECKS+4
#define CT_TRAP 	CHECKS+5
#define CT_DATA 	CHECKS+6
#define CT_INST 	CHECKS+7
#define CT_CACHE	CHECKS+8
#define CT_NOCACHE	CHECKS+9
#define CT_BURST	CHECKS+10
#define CT_NOBURST	CHECKS+11
#define CT_VERBOSE	CHECKS+12
#define CT_KICKROM	CHECKS+13
#define CT_CARDROM	CHECKS+14
#define CT_CONFIG	CHECKS+15
#define CT_HEAD 	CHECKS+16
#define CT_DELAY	CHECKS+17
#define CT_HELP 	CHECKS+18

/* This function fetches a bounded numeric value, or takes the error trap
   for numeric arguments if things aren't right. */

short getnumber(arg,low,high)
char *arg;
short low, high;
{
   short num;

   if (!arg || !isdigit(*arg) || (num = atoi(arg)) < low || num > high)
      quit(11);
   return num;
}

/* This is basically the DiskSalv command-line parser re-written for the
   SetCPU command set. */

static USHORT ParseCommandLine(argc,argv)
int argc;
char **argv;
{
   short i,j, trapmode = 0;
   struct patch *p;
   BPTR lock;
   ULONG mode = CACR_INST | CACR_DATA, cacr = 0;
   char *file;
   BOOL f_FASTROM = FALSE, f_KEYPAT = FALSE;

   cacr = GetCACR();
   for (i = 1; i < argc; i++) {
      for (j = 0; CLITokens[j] && !striequ(argv[i],CLITokens[j]); ++j);
      if (j < CHECKS) {
	 if (code != WARNING) return 12;
	 tags[j] = TRUE;
      } else switch (j) {
	 case CT_CHECK	: code = WARNING;			break;
	 case CT_NOFROM : fastrom = FR_DELETE;			break;
	 case CT_DATA	: mode = CACR_DATA;			break;
	 case CT_INST	: mode = CACR_INST;			break;
	 case CT_CACHE	: cacr |=   mode << CACR_ENABLE;	break;
	 case CT_NOCACHE: cacr &= ~(mode << CACR_ENABLE);       break;
	 case CT_BURST	: cacr |=   mode << CACR_BURST; 	break;
	 case CT_NOBURST: cacr &= ~(mode << CACR_BURST);        break;
	 case CT_VERBOSE: verbose = TRUE;			break;
	 case CT_HEAD	: allochead = TRUE;			break;
	 case CT_HELP	: helpmode = TRUE;			break;
	 case CT_TRAP	:
	    if (argv[i+1] && isdigit(argv[i+1][0]))
	       trapmode = getnumber(argv[++i],0,2);
	    if (trapmode == 0) wrapbits = 0;
	    handler = (trapmode == 2);
	    break;
	 case CT_CONFIG :
	    configset = TRUE;
	    configlevel = getnumber(argv[++i],0,2);
	    break;
	 case CT_DELAY:
	    SetBootDelay(0x60000L*(ULONG)getnumber(argv[++i],0,100));
	    break;
	 case CT_KEYPAT :
	     SetKeyDelay(100L * (ULONG)getnumber(argv[++i],1,100));
	     for (p = SystemPatch; p; p = p->next)
		if (p->list[KEYPATCH].Type == PT_KEYBOARD) {
		   p->list[KEYPATCH].Length = KeyCodeSize;
		   p->list[KEYPATCH].Code = (UWORD *)KeyCode;
		   p->list[KEYPATCH].Type = PT_JSR;
		}
	    f_KEYPAT = TRUE;
	    break;
	 case CT_NOPATCH:
	    for (p = SystemPatch; p; p = p->next) {
	       j = 0;
	       while (p->list[j].Type != PT_END) {
		  if (j != KEYPATCH && p->list[j].Type < PT_END)
		     p->list[j++].Type = PT_IGNORE;
		  else
		     ++j;
	       }
	    }
	    break;
	 case CT_FASTROM:
	    fastrom = FR_MKFAST;
	    if (i < argc && (lock = Lock(argv[i+1],ACCESS_READ))) {
	       UnLock(lock);
	       romfile = argv[++i];
	    }
	    f_FASTROM = TRUE;
	    break;
	 case CT_KICKROM:
	    fastrom = FR_MKKICK;
	    if (!(romfile = argv[++i])) return 19;
	    break;
	 case CT_CARDROM:
	    if (!(file = argv[++i])) return 19;
	    if (!ReadExpDevs(file)) return 15;
	    break;

	 default: return 12;
      }
      SetCACR(cacr | CACR_DATA << CACR_WALLOC);
   }
   if (f_KEYPAT && !f_FASTROM) return 17;

   return 0;
}


/* This function returns TRUE is the tag ROM passed is the same version
   as the actual ROM in use, FALSE otherwise. */

BOOL CheckVersion(tag)
struct systag *tag;
{
   UWORD *tagver,*kickver = (UWORD *)(0x100000C - (*(ULONG *)0xFFFFEC));
   ULONG *rom = (ULONG *)((ULONG)tag->romimage + tag->romsize - ROMSIZE_256);

   tagver = (UWORD *)(((ULONG)rom + ROMSIZE_256 + 0x0C) - rom[0xfffb]);
   return (BOOL) (kickver[0] == tagver[0] && kickver[1] == tagver[1]);
}


/* This be the main program. */

int main(argc,argv)
int argc;
char *argv[];
{
   ULONG cacr,op,test,cpu,fpu,mmu = 0;
   struct systag *tag, *newtag;
   USHORT i,j,err;

  /* First we parse the command line.  The default cache operation acts
     on both data and instruction caches.  The way all the cache control
     functions are defined, they're just NOPs on machines without the
     appropriate caches. */

   if (argc > 1 && (err = ParseCommandLine(argc,argv))) quit(err);

   /* If they're just asking for help */

   if (verbose || helpmode)
      printf("\23333mSetCPU V%1.2f(BETA 1) Copyright 1989 by Dave Haynie\2330m\n",
	     ((float)PROGRAM_VERSION)/100.0);

   if (helpmode) quit(1);

  /* The FastROM routine uses the expansion library.  It's possible to have
     an old version of the OS without this, so I'll be careful to avoid
     using any ExpansionBase function without checking for the base being
     valid. */

   ExpansionBase = (struct ExpansionBase *)OpenLibrary("expansion.library",0L);

  /* Let's find out what we have, and perform the ROM translation, if it's
     requested and hasn't been done already. */

   cpu = GetCPUType();
   fpu = GetFPUType();
   if ((mmu = GetMMUType()) && mmu != BOGUSMMU) {
      tag = GetSysTag();
      switch (fastrom) {
	 case FR_MKFAST:
	    if (tag) switch (tag->romtype) {
	       case ROM_FAST:
		  DeleteFastROM(tag);
		  if (romfile) {
		     if (!(newtag = AllocDISKImage(ROM_FAST,romfile))) quit(14);
		     if (!CheckVersion(newtag)) {
			DeleteFastROM(newtag);
			quit(21);
		     }
		  } else
		     newtag = AllocROMImage(ROM_FAST);
		  if (!CreateFastROM(newtag,wrapbits)) quit(14);
		  break;
	       case ROM_KICK:
		  SetupConfig(tag);
		  if (!CreateFastROM(AllocROMImage(ROM_FKICK),wrapbits)) quit(14);
		  FreeSAFEImage(tag);
		  break;
	       case ROM_FKICK:
		  quit(20);
	    } else {
	       if (romfile) {
		  if (!(newtag = AllocDISKImage(ROM_FAST,romfile))) quit(14);
		  if (!CheckVersion(newtag)) {
		     DeleteFastROM(newtag);
		     quit(21);
		  }
	       } else
		  newtag = AllocROMImage(ROM_FAST);
	       if (!CreateFastROM(newtag,wrapbits)) quit(14);
	    }
	    break;
	 case FR_MKKICK:
	    if (tag) switch (tag->romtype) {
	       case ROM_FAST:
		  DeleteFastROM(tag);
	       case ROM_FKICK:
		  if (!CreateKickROM(AllocDISKImage(ROM_KICK,romfile),wrapbits))
		     if (LoadErr) quit(LoadErr); else quit(18);
	       case ROM_KICK:
		  SetupConfig(tag);
		  if (!CreateFastROM(AllocROMImage(ROM_FKICK),wrapbits)) quit(14);
		  FreeSAFEImage(tag);
		  break;
	    } else
	       if (!CreateKickROM(AllocDISKImage(ROM_KICK,romfile),wrapbits))
		  if (LoadErr) quit(LoadErr); else quit(18);
	    break;
	 case FR_DELETE:
	    if (fastrom == FR_DELETE && tag) {
	       if (tag->romtype != ROM_FAST) quit(13);
	       DeleteFastROM(tag);
	    }
	    break;
	 case FR_NO_ACTION:
	    if (configset) SetupConfig(tag);
      }
   }
   printf("SYSTEM: ");

   /* If they're not on a 68020/68030, we can't set anything.  For
      compatibility across systems, I don't consider a cache setting
      request an error, just ignore it. */

   if (cpu <= 68010L) {
      if (cpu == 68010L) {
	 printf("68010 ");
	 if (tags[CK68010]) code = 0;
      } else {
	 printf("68000 ");
	 if (tags[CK68000]) code = 0;
      }
      PrintFPU(fpu);
      printf("\n");
      quit(code);
   }

   /* Now we're on a 32 bit system.  But EXEC doesn't know which.  If you
      run SetCPU on a 68030 system once, the '030 flag's set, otherwise,
      we'll test for it. */

   if (cpu == 68030L) {
      printf("68030 ");
      if (tags[CK68030]) code = 0;
   } else {
      printf("68020 ");
      if (tags[CK68020]) code = 0;
   }

   PrintFPU(fpu);

   if (mmu == 68851L) {
      printf("68851 ");
      if (tags[CK68851]) code = 0;
   }
   if (mmu && mmu != BOGUSMMU) {
      if (tags[CKMMU]) code = 0;
      if (tag = GetSysTag()) switch (tag->romtype) {
	 case ROM_FAST:
	    printf("FASTROM "); break;
	 case ROM_KICK:
	    printf("SLOWKICK "); break;
	 case ROM_FKICK:
	    printf("FASTKICK "); break;
	 default:
	    break;
      }
   } else if (mmu == BOGUSMMU)
      printf("(FPU LOGIC ERROR) ");

   /* We always print the results, even if nothing has changed. */

   cacr = GetCACR();
   printf("(INST: ");
   if (!(cacr & (CACR_INST << CACR_ENABLE))) printf("NO");
   printf("CACHE");

   if (cpu == 68030L) {
      printf(" ");
      if (!(cacr & (CACR_INST << CACR_BURST))) printf("NO");
      printf("BURST) (DATA: ");
      if (!(cacr & (CACR_DATA << CACR_ENABLE)))
	 printf("NOCACHE ");
      else
	 printf("CACHE ");

      if (!(cacr & (CACR_DATA << CACR_BURST))) printf("NO");
      printf("BURST");
   }
   printf(")\n");

   if (verbose && (GetTC() & TC_ENB)) PrintFastROM();
   quit(code);
}


