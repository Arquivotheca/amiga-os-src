/*
	SetCPU V1.6
	by Copyright 1989 by Dave Haynie

	MAIN HEADER FILE
*/

/* #define DEBUG */


#ifdef KICKFILE
#define PROGRAM_VERSION 121
#else
#define PROGRAM_VERSION 171
#endif

#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include <exec/io.h>
#include <devices/trackdisk.h>
#ifndef KICKFILE
#include <libraries/expansionbase.h>
#include <libraries/configregs.h>
#include <libraries/configvars.h>
#include <graphics/gfxbase.h>
#include <graphics/gfxmacros.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#endif
#include <libraries/dosextens.h>
#include <libraries/filehandler.h>
#include <functions.h>
#include <stdio.h>
#include <ctype.h>

/* ====================================================================== */

/* Define all bit components used for manipulation of the Cache Control
   Register. */

#define CACR_INST	(1L<<0)
#define CACR_DATA	(1L<<8)

#define CACR_WALLOC	5
#define CACR_BURST	4
#define CACR_CLEAR	3
#define CACR_ENTRY	2
#define CACR_FREEZE	1
#define CACR_ENABLE	0

/* ====================================================================== */

/* Define important bits used in various MMU registers. */

/* Here are the CRP definitions.  The CRP register is 64 bits long, but
   only the first 32 bits are control bits, the next 32 bits provide the
   base address of the table. */

#define CRP_UPPER	(1L<<31)                /* Upper/lower limit mode */
#define CRP_LIMIT(x)    ((ULONG)((x)&0x7fff)<<16)/* Upper/lower limit value */
#define CRP_SG		(1L<<9)                 /* Indicates shared space */
#define CRP_DT_INVALID	0x00			/* Invalid root descriptor */
#define CRP_DT_PAGE	0x01			/* Fixed offset, auto-genned */
#define CRP_DT_V4BYTE	0x02			/* Short root descriptor */
#define CRP_DT_V8BYTE	0x03			/* Long root descriptor */

/* Here are the TC definitions.  The TC register is 32 bits long. */

#define TC_ENB		(1L<<31)                /* Enable the MMU */
#define TC_SRE		(1L<<25)                /* For separate Supervisor */
#define TC_FCL		(1L<<24)                /* Use function codes? */
#define TC_PS(x)        ((ULONG)((x)&0x0f)<<20) /* Page size */
#define TC_IS(x)        ((ULONG)((x)&0x0f)<<16) /* Logical shift */
#define TC_TIA(x)       ((ULONG)((x)&0x0f)<<12) /* Table indices */
#define TC_TIB(x)       ((ULONG)((x)&0x0f)<<8)
#define TC_TIC(x)       ((ULONG)((x)&0x0f)<<4)
#define TC_TID(x)       ((ULONG)((x)&0x0f)<<0)

/* Here are the page descriptor definitions, for short desctriptors only,
   since that's all I'm using at this point. */

#define PD_ADDR(x)      ((ULONG)(x)&~0x0fL)     /* Translated Address */
#define IV_ADDR(x)      ((ULONG)(x)&~0x03L)     /* Invalid unused field */
#define PD_WP		(1L<<2)                 /* Write protect it! */
#define PD_CI		(1L<<6)                 /* Cache inhibit */
#define PD_DT_TYPE	0x03			/* Page descriptor type */
#define PD_DT_INVALID	0x00			/* Invalid root descriptor */
#define PD_DT_PAGE	0x01			/* Fixed offset, auto-genned */
#define PD_DT_V4BYTE	0x02			/* Short root descriptor */
#define PD_DT_V8BYTE	0x03			/* Long root descriptor */

/* This is needed for identification of bogus systems that test positive
   for MMUs. */

#define BOGUSMMU	0xffffffffL

/* Here's the MMU support stuff. */

#define ROMROUND	0x00020000L
#define ROMSIZE_256	0x00040000L
#define ROMSIZE_512	0x00080000L
#define ROM_END 	0x01000000L
#define DEVROUND	0x00004000L
#define TABROUND	0x00001000L
#define PAGESIZE	0x00020000L
#define MAINTABSIZE	(128L * sizeof(ULONG))
#define SUBTABSIZE	(8L * sizeof(ULONG))

/* ====================================================================== */

/* Some external system declarations. */

extern BPTR Lock(), Open();

/* Checking logic */

#define CK68000 0
#define CK68010 1
#define CK68020 2
#define CK68030 3
#define CK68851 4
#define CK68881 5
#define CK68882 6
#define CKFPU	7
#define CKMMU	8
#define CHECKS	9

#define WARNING 5
#define READOK	0L

#define SizeOf(x)       ((ULONG)sizeof(x))

#ifndef KICKFILE
/* ====================================================================== */

/* From the 030STUFF.A module */

extern void			SetCACR(),
				GetCRP(),
				SetCRP(),
				SetTC(),
				SetMMUTag(),
				SetKeyDelay(),
				ROMBoot();
				SetBootDelay();
extern ULONG			GetCACR(),
				GetTC(),
				GetCPUType(),
				GetMMUType(),
				GetFPUType(),
				*GetVBR(),
				KeyCode,
				KeyCodeSize,
				BootCode,
				BootCodeSize,
				BerrCode,
				BerrCodeSize;

/* ====================================================================== */
#endif

/* From the STRINGS.C module */

extern BOOL			striequ(),
				strniequ();

#ifndef KICKFILE
/* ====================================================================== */

/* From the EXPDEV.C module. */

/* This structure is device information used by the memory mapping routine. */

struct ExpROMData {
   struct ExpROMData *next;
   ULONG ROMbase;
   ULONG ROMsize;
   ULONG imagebase;
   ULONG tablebase;
   char *name;
};

extern BOOL				ReadExpDevs();
extern struct ExpROMData		*GetExpROM();
extern void				SafeConfigDevs();
#endif

/* ====================================================================== */

/* From the DEVIO.C module. */

extern void			MotorOff();
extern BYTE			ReadBuf();
extern LONG			CheckTDDev();

#ifndef KICKFILE
/* ====================================================================== */

/* From the PATCHES.C module. */

/* Patch types */

#define PT_STRING	1
#define PT_JSR		2
#define PT_END		3
#define PT_IGNORE	4
#define PT_KEYBOARD	5

/* This is a item */

struct pitem {
   UWORD Type;		/* The type of patch to apply */
   UWORD Pad;		/* Nothing here yet... */
   ULONG Offset;	/* The offset at which to apply the patch */
   ULONG Length;	/* The length of the patch item */
   UWORD *Code; 	/* The actual patch item */
};

/* This is the patch structure. */

struct patch {
   struct patch *next;	/* Next patchlist */
   struct pitem *list;	/* First item in this patchlist */
   UWORD Version;	/* Which ROM version */
   UWORD Revision;	/*   and revision */
};

/* These are the some patch system variables. */

#define KEYPATCH		0

extern struct patch		SystemPatch[];
extern struct MemChunk		*lastpatch;

/* The actual functions. */

extern BOOL			AddPatch();
#endif

/* ====================================================================== */

/* From the MEMORY.C module. */

/* This section describes the system tag structure, where I stick all the
   information that I like to keep around.  */

#define ROM_NOP 		0x0000	/* No ROM operation called for */
#define ROM_FAST		0x0002	/* Normally installed FASTROM image */
#define ROM_KICK		0x0003	/* Installed as a KICK image */
#define ROM_FKICK		0x0004	/* A KICK image made into a FAST image */

/* This was originally the patchtag structure, which looked like a patch.  I
   decided that was real kludgy.  I hook the systag onto an invalid page
   descriptor which I locate in the physical ROM image, a place that should
   always be safe to have an invalid tag structure. */

struct systag {
   ULONG		tagsize;	/* Size of this tag */
   ULONG		progver;	/* The program version */
   ULONG		*maintable;	/* The main ROM table */
   ULONG		*romimage;	/* The main ROM image */
   UWORD		romtype;	/* Type of MMU ROM */
   UWORD		patches;	/* The number of other patches applied */
   struct MemChunk	*patchlist;	/* List of installed patches */
   struct ExpROMData	*devs;		/* Translated device ROMs */
   ULONG		TC;		/* Precomputed TC used for KICK. */
   ULONG		CRP[2]; 	/* Precomputed CRP used for KICK. */
   UWORD		config; 	/* Configuration status. */
   ULONG		BerrSize;	/* Size of bus error handler. */
   char 		*OldBerr;	/* The old BERR routine. */
   char 		*BerrHandler;	/* My BERR routine. */
   short		wrapup; 	/* Upper address wrap bound. */
   short		wrapdown;	/* Lower address wrap bound. */
   ULONG		tablesize;	/* Main table size. */
   char 		*ResetCode;	/* Actual reset routine */
   ULONG		romsize;	/* Full size of ROM memory */
   ULONG		romused;	/* Actual size of ROM code */
};

/* The actual functions */

#ifndef KICKFILE
extern void			SetMMURegs(),
				*AllocAligned(),
				FreeSAFEImage();
extern struct systag		*AllocROMImage(),
				*AllocDISKImage(),
				*AllocSAFEImage(),
				*GetSysTag();
extern BOOL			allochead;

#else
extern struct systag		*AllocKSImage();
#endif

#ifndef KICKFILE
/* ====================================================================== */

/* From the COOLHAND.C module. */

struct Window *CoolHand();

/* ====================================================================== */

/* Intermodule globals */

#ifdef MAIN_MODULE
struct ExpansionBase *ExpansionBase = NULL;	/* The expansion library */
unsigned short LoadErr = 16;
#else
extern struct ExpansionBase *ExpansionBase;
extern unsigned short LoadErr;
#endif

#endif
