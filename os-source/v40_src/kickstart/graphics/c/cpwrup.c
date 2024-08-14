/******************************************************************************
*
*	$Id: cpwrup.c,v 39.75 93/05/17 13:46:53 chrisg Exp $
*
******************************************************************************/

/* graphics  kernel routines */
#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/interrupts.h> 
#include <exec/memory.h>
#include <exec/libraries.h>
#include <exec/semaphores.h>
#include <exec/execbase.h>
#include <hardware/custom.h>
#include <hardware/blit.h>
#include <hardware/dmabits.h>
#include <hardware/intbits.h>
#include "/gfxbase.h"
#include "/macros.h"
#include "/copper.h"
#include "/displayinfo.h"
#include "/cia8520.h"
/*#include "/sane_names.h"*/
#include "/d/d.protos"
#include "/displayinfo_internal.h"
#include "/gfx.h"
#include "/view.h"
#include "/display.h"
#include "/gfxpragmas.h"
#include "/vp_internal.h"
#include "gfxprivate.h"
#include "c.protos"
#include <internal/librarytags.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/exec_pragmas.h>


/*#define GBDEBUG*/		/* sets GfxBase->Debug = 1 */
/*#define DEBUG*/
/*#define RAMDEBUG*/
/*#define BADFMODE*/

#ifdef DEBUG 
#define D(x) 	x
#else
#define D(x) 	;
#endif

#ifdef BADFMODE
#define FMODE io->pad3b[0]
#define FMODE2 io->fmode
#else
#define FMODE io->fmode
#endif

#define ALERTGFXNOMEMMSPC 0x82010001

extern struct Custom custom;
extern struct CIA8520 ciaa;
extern struct CIA8520 ciab;

extern USHORT   vhposr;

extern VERSION, REVISION;
extern char gfx_lib_name[];
#ifdef RAMDEBUG
UBYTE	Debug;
#endif


int BltBitMap(),BltTemplate(),ClearEOL(),ClearScreen();
int TextLength() , SetFont();

int Text();

int OpenFont(),CloseFont(),FontExtent(),WeighTAMatch();
int ExtendFont(),StripFont();
int AskSoftStyle(),SetSoftStyle();

int AddFont(),RemFont(),AskFont();

int AddBob(),AddVSprite(),DoCollision(),DrawGList();
int InitGels(),InitMasks(),RemIBob(),RemVSprite();
int SetCollision(),SortGList(),AddAnimOb(),Animate();
int GetGBuffers(),InitGMasks();
int DrawEllipse();

int LoadRGB4(),InitRastPort(),InitVPort(),MrgCop();
int LoadView(),WaitBlit(),SetRast();
int Move(),Draw();
int AreaMove(),AreaDraw(),AreaEnd(),AreaEllipse();
int WaitTOF(),QBlit();
int InitArea();
int SetRGB4(),QBSBlit();
int BltClear();
int BltPattern();
int ReadPixel(),WritePixel();
int Flood();
int PolyDraw();
int SetAPen(),SetBPen(),SetDrMd(),InitView();
int VBeamPos();
int InitBitMap();
int WaitBOVP();
int GetSprite(),FreeSprite(),ChangeSprite(),MoveSprite();

int LockLayerRom(),UnlockLayerRom();

int OwnBlitter(),DisownBlitter();

int InitTmpRas();

extern int (*vbasm)(),(*bltasm)(),(*timasm)();

int OpenGfx(),CloseGfx(),ExpungeGfx(),ExtFuncGfx();

int AllocRaster(),FreeRaster();
int AndRectRegion(),OrRectRegion(),XorRectRegion();
int NewRegion(),ClearRectRegion(),ClearRegion(),DisposeRegion();

int FreeVPortCopLists(),FreeCopList();
int FreeCprList();

int BltRastPort();
int GetColorMap(),FreeColorMap();
int	GetRGB4();
int ScrollVPort();
int FreeGBuffers();
int BltBitMapRastPort();

int BltMaskRastPort(); /* bart - 1.28.86 */

int OrRegionRegion(),XorRegionRegion(),AndRegionRegion();

int SetRGB4CM();
int GetOPen();

int AttemptLockLayerRom();

int BitMapScale(),ScalerDiv();
int TextExtent(),TextFit();

long VideoControl();

int ReadPixelLine8(),  WritePixelLine8();
int ReadPixelArray8(), WritePixelArray8();
int FindColor();

int ReleasePen();
int ObtainPen();
int AttachPalExtra();
int ObtainBestPenA();
int SetRGB32();
int GetAPen(), GetBPen(), GetDrMd();
int LoadRGB32();
int SetABPenDrMd();
int GetRGB32();
int AlwaysReturnsZero();
int AllocBitMap();
int FreeBitMap();
int GetExtSpriteA();
int ChangeVPBitMap();
int GetBitMapAttr();
int AllocDBufInfo();
int FreeDBufInfo();
int SetOutlinePen(), SetWriteMask(), SetMaxPen();
int SetRGB32CM();
int AllocSpriteDataA();
int ChangeExtSpriteA();
int FreeSpriteData();
int WriteChunkyPixels();

void defras();

struct Resident *FindResident();

/*struct DisplayInfoRecord *find_info();*/

void db_avail(struct DisplayInfoRecord *record)
{
    BOOL HRDenise, Lisa;

    HRDenise = (GBASE->ChipRevBits0 & GFXF_HR_DENISE);
    Lisa = (GBASE->ChipRevBits0 & GFXF_AA_LISA);

    {
		struct RawDisplayInfo *dinfo;

		/* not vanilla */

		if (dinfo  = FindTagItem(DTAG_DISP,&record->rec_Tag))
		{
			struct RawMonitorInfo *minfo;
			struct RawDimensionInfo *dims;

			record->rec_Control = dinfo->ModeID;

			/* We may be coming through here from SetChipRev(). So, if
			 * we are setting to 'A' mode, we may have been here initially
			 * when the machine thought it was ECS. Therefore, set the
			 * DI_AVAIL_NOCHIPS flag first, and only clear it
			 * in the appropriate place.
			 */
			if (dinfo->PropertyFlags & (DIPF_IS_ECS | DIPF_IS_AA))
			{
				dinfo->NotAvailable |= DI_AVAIL_NOCHIPS;
			}

			if (HRDenise && (dinfo->PropertyFlags & DIPF_IS_ECS))
			{
				dinfo->NotAvailable &= ~DI_AVAIL_NOCHIPS;
			}
			if (Lisa && (dinfo->PropertyFlags & DIPF_IS_AA))
			{
				/* Assume we can clear the NOCHIPS flag */
				dinfo->NotAvailable &= ~DI_AVAIL_NOCHIPS;

				/* but if we don't have the badwidth for the HAM or EHB... */
				if (dinfo->ModeID & (HAM | EXTRA_HALFBRITE))
				{
					if (! (((dinfo->ModeID & SUPERHIRES) && (GBASE->MemType == BANDWIDTH_4X))
					    ||
					   ((dinfo->ModeID & HIRES) && GBASE->MemType) 
					    ||
					    ((dinfo->ModeID & (HIRES | SUPERHIRES)) == NULL)))
					{
						dinfo->NotAvailable |= DI_AVAIL_NOCHIPS;
					}
				}
			}

			if((record) && (dims = FindTagItem(DTAG_DIMS,&record->rec_Tag)))
			{
				record->rec_ClipOScan.MinX = 
					dims->VideoOScan.MinX / dinfo->Resolution.x;
				record->rec_ClipOScan.MaxX = 
					dims->VideoOScan.MaxX / dinfo->Resolution.x;
				record->rec_ClipOScan.MinY = 
					dims->VideoOScan.MinY / dinfo->Resolution.y;
				record->rec_ClipOScan.MaxY = 
					dims->VideoOScan.MaxY / dinfo->Resolution.y;
			}

			if(GBASE->system_bplcon0 & GENLOCK_VIDEO)
			{
				if(!(dinfo->PropertyFlags & DIPF_IS_GENLOCK))
				{
					dinfo->NotAvailable |= DI_AVAIL_NOTWITHGENLOCK;
				}
			}

			if((record = (struct DisplayInfoRecord *) record->rec_Node.rcn_Parent)
			&& (minfo  = FindTagItem(DTAG_MNTR,&record->rec_Tag)))
			{
				if(minfo->Mspc) dinfo->NotAvailable &= ~DI_AVAIL_NOMONITOR;
			}
		}
    }
}

void all_db_avail(void)
{
    ULONG ID = INVALID_ID;
    struct DisplayInfoRecord *record = GBASE->DisplayInfoDataBase, *head;

    while ((ID = NextDisplayInfo(ID)) != INVALID_ID)
    {
	if (head = (struct DisplayInfoRecord *)FindDisplayInfo(ID))
	{
		db_avail(head);
	}
    }
}

void activate_mspc( monitor, mspc )
ULONG monitor;
struct MonitorSpec *mspc;
{
    ULONG ID = INVALID_ID;
    struct RawMonitorInfo querymonitor;

    while( ( ID = NextDisplayInfo( ID ) ) != INVALID_ID )
    {
	if( ( ID & ~0xFFFF ) == ( monitor & ~0xFFFF ) )
	{
	    struct DisplayInfoRecord *record = FindDisplayInfo( ID );
	
	    if(record)
	    {
		int numbytes = 0;
		struct RawMonitorInfo *chunk = &querymonitor;

		if(numbytes = 
		   (int)GetDisplayInfoData(record, (UBYTE *)chunk, sizeof(*chunk), DTAG_MNTR, ID))
		{
		    chunk->Mspc = mspc;
		    if(SetDisplayInfoData(record, (UBYTE *)chunk,numbytes,DTAG_MNTR,ID)) break;
		}
	    }
	}
    }
}

void *vectbl[] =
{
/*exec21*/  OpenGfx,    /* openlib */
	    CloseGfx,   /* close lib */
	    ExpungeGfx, /* initlib */
	    ExtFuncGfx, /* expunge lib */

    BltBitMap,BltTemplate,ClearEOL,ClearScreen,
    TextLength,Text,SetFont,
    OpenFont,CloseFont,AskSoftStyle,SetSoftStyle,

	/* 16 slots for gels */
    AddBob,AddVSprite,DoCollision,DrawGList,
    InitGels,InitMasks,RemIBob,RemVSprite,
    SetCollision,SortGList,AddAnimOb,Animate,
    GetGBuffers,InitGMasks,
    DrawEllipse,AreaEllipse,
	/* rest of graphics */
	LoadRGB4,InitRastPort,InitVPort,MrgCop,
	MakeVPort,LoadView,WaitBlit,SetRast,
	Move,Draw,
	AreaMove,AreaDraw,AreaEnd,
	WaitTOF,QBlit,
	InitArea,
	SetRGB4,QBSBlit,
	BltClear,
	RectFill,BltPattern,
	ReadPixel,WritePixel,
	Flood,PolyDraw,
	SetAPen,SetBPen,SetDrMd,InitView,
	CBump,CMove,CWait,VBeamPos,
	InitBitMap,
	ScrollRaster,
	WaitBOVP,
	GetSprite,FreeSprite,ChangeSprite,MoveSprite,
	LockLayerRom,UnlockLayerRom,
	SyncSBitMap,CopySBitMap,
	OwnBlitter,DisownBlitter,
	InitTmpRas,
	AskFont,AddFont,RemFont,
	AllocRaster,FreeRaster,
	AndRectRegion,OrRectRegion,
	NewRegion,ClearRectRegion,ClearRegion,DisposeRegion,
	FreeVPortCopLists,FreeCopList,
	BltRastPort,
	XorRectRegion,
	FreeCprList,
	GetColorMap,FreeColorMap,GetRGB4,
	ScrollVPort,
	UCopperListInit,
    	FreeGBuffers,
    	BltBitMapRastPort,
	OrRegionRegion,
	XorRegionRegion,
	AndRegionRegion,
	SetRGB4CM,
	BltMaskRastPort,
	ReleasePen,
	ObtainPen,
	AttemptLockLayerRom,
	GfxNew,GfxFree,GfxAssociate,
	BitMapScale,ScalerDiv,
	TextExtent,TextFit,
	GfxLookUp,
	VideoControl,
	OpenMonitor,
	CloseMonitor,
	FindDisplayInfo, 
	NextDisplayInfo, 
	AddDisplayInfo,     /* graphics private */
	AddDisplayInfoData, /* graphics private */
	SetDisplayInfoData, /* graphics private */
	GetDisplayInfoData, 
	FontExtent,
	ReadPixelLine8,
	WritePixelLine8,
	ReadPixelArray8,
	WritePixelArray8,
	GetVPModeID, 
	ModeNotAvailable,
	WeighTAMatch,
	EraseRect,
	ExtendFont,
	StripFont,
	CalcIVG,
	AttachPalExtra,
	ObtainBestPenA,
	AlwaysReturnsZero,
	SetRGB32,
	GetAPen,
	GetBPen,
	GetDrMd,
	GetOPen,
	LoadRGB32,
	SetChipRev,
	SetABPenDrMd,
	GetRGB32,
	AlwaysReturnsZero,
	SetDefaultMonitor,
	AllocBitMap,
	FreeBitMap,
	GetExtSpriteA,
	CoerceMode,
	ChangeVPBitMap,
	ReleasePen,
	ObtainPen,
	GetBitMapAttr,
	AllocDBufInfo,
	FreeDBufInfo,
	SetOutlinePen,
	SetWriteMask,
	SetMaxPen,
	SetRGB32CM,
	ScrollRasterBF,
	FindColor,
	AlwaysReturnsZero,
	AllocSpriteDataA,
	ChangeExtSpriteA,
	FreeSpriteData,
	SetRPAttrA,
	GetRPAttrA,
	BestModeIDA,
	WriteChunkyPixels,
	AlwaysReturnsZero,
	AlwaysReturnsZero,
	AlwaysReturnsZero,
	(int *)(-1)
};

UBYTE bwshifts[] = {0, 1, 1, 2};

extern UWORD StrtFetchMasks[][];
extern UWORD StopFetchMasks[][];
extern UWORD Overrun[][];
extern UWORD RealStops[][];
extern APTR  ProgData;

struct MonitorSpec *new_monitorspec(GB,flags,name)
struct GfxBase *GB;
UWORD flags;	
STRPTR name;
{
    struct MonitorSpec *mspc = (struct MonitorSpec *) GfxNew(MONITOR_SPEC_TYPE);
    /* initialize monitorspec and add to list */
    if(mspc)
    {
	mspc->ms_Node.xln_Library = (LONG)GB;
        mspc_init(GB, mspc, flags); /* assembly interface */
	mspc->ms_Node.xln_Name = name;

    }
    return(mspc);
}

extern STRPTR rev_name;

#define CCLKS(x) ((x)->total_colorclocks)

#define MAX_DENISE(x) (((CCLKS(x)+1)<<1)+1)

#define BPLPT 0xe0
#define BPLCON0 0x100
#define BPLCON3 0x106
#define COLOR 0x180
#define REG_FMODE 0x1fc
#define DIWSTRT 0x8e
#define DIWSTOP 0x90
#define DIWHIGH 0x1e4
#define DDFSTRT 0x92
#define DDFSTOP 0x94
#define BPLCON2 0x104
#define COPJMP2 0x8a
#define NOP	0x1fe

UWORD copinit_header[]= {
	/* first one just a place saver for new chips */
						 COPPER_MOVE|BPLPT,0,
	/* Now we need place for five more moves -- (in diagstrt) for colours */
						 COPPER_MOVE|BPLCON0,0x200,
						 COPPER_MOVE|BPLCON3,0xc00,
						 COPPER_MOVE|COLOR,
};

UWORD copinit_mid_ecs[]={
	COPPER_MOVE|NOP,0xc00,
	/* will become BPLCON3 in update_top_colour() after enlightenment */
	/* Now, set the default bandwidth to 1x */
	COPPER_MOVE|REG_FMODE,0,
	/* Open a display window of one line ... */
	COPPER_MOVE|DIWSTRT,0x0181,
	COPPER_MOVE|DIWSTOP,0x0281,
	COPPER_MOVE|DIWHIGH,0,
	/* And a single fetch to hide the Alice bug. This removes the blank
	   bar at the RHS for the 16 and 32 fetch cycles when I set DDFSTOP to
	   0xd8 */
	COPPER_MOVE|DDFSTRT,0x18,
	COPPER_MOVE|DDFSTOP,0x20,
	/* Now set bplcon2 */
	COPPER_MOVE|BPLCON2,0x24,
};

UWORD copinit_mid_a[]={
	COPPER_MOVE|NOP,0xc00,
	COPPER_MOVE|NOP,0x0,
	COPPER_MOVE|NOP,0x0,
	COPPER_MOVE|NOP,0x0,
	COPPER_MOVE|NOP,0x0,
	COPPER_MOVE|NOP,0x0,
	COPPER_MOVE|NOP,0x0,
	COPPER_MOVE|NOP,0x0,
};


UWORD copinit_end[]={
				COPPER_WAIT | TOPLINE,0xFFFE,  /* wait for first line */
				/* just a place saver here for real hbstop */
				COPPER_WAIT | TOPLINE,0xFFFE,  /* wait for first line */

	/* the above wait will allow the vertical blank routine time to set
	   up cop2ptr to continue the refresh of the display */

				COPPER_MOVE|COPJMP2,0,
				COPPER_MOVE|BPLCON3,0x0c00,
				0xffff,0xfffe /* wait forever */
};

void cpwrup()
{
    register int i;
    register short *a;
    register struct Custom *io = &custom;
	long WC_ZERO=0;
	UBYTE	agnus_chip_id;
	UBYTE	denise_chip_id;


    /* rectify the library node */

    ((struct Library *)GBASE)->lib_Node.ln_Type = NT_LIBRARY;
    ((struct Library *)GBASE)->lib_IdString = &rev_name;

	agnus_chip_id = (io->vposr>>8) & 0x7f;

/********************************************************************************
*
* This code was sometimes failing to recognise a non-ECS denise properly on
* an A1000. It has been replaced with an assembly function (in misc.asm)
* - For 2.05 A300 build, spence Wed Oct 23 10:33:35 1991
*
*	denise_chip_id= (io->deniseid) & 0xff;
*	for (i = 0; i < 16 ; i++)
*		if ( denise_chip_id != (io->deniseid & 0xff) )
*		{
*			denise_chip_id = -1;
*			break;
*		}
*********************************************************************************/

	denise_chip_id = get_denise_id();

#ifdef RAMDEBUG
	kprintf("Debug at %lx\n",&Debug);
	Debug = 0;
#endif

    /* initialize parallel port directions */
    /* set bit plane data registers to zero */
	for(i=0;i<6;i++)    io->bpldat[i] = 0;

    /*
    c = (struct GfxBase *)MakeLibrary(vectbl
				    ,0,0,sizeof(struct GfxBase),NULL);
    */

    /* initialize utility library for hooks and tags */

    GBASE->UtilBase=TaggedOpenLibrary(OLTAG_UTILITY);

/* What type of chips and memory do we have? 
 * This determines what FMode we use as the
 * system default, and the display modes that are available
 */

    if (agnus_chip_id & 0x20)	/* agnus hr bit set ? */
	GBASE->ChipRevBits0 |= GFXF_HR_AGNUS;
    else
	GBASE->DisplayFlags |= LPEN_SWAP_FRAMES;

    if (!(denise_chip_id & 0x02))
	GBASE->ChipRevBits0 |= GFXF_HR_DENISE;

    if (!(denise_chip_id & 0x04))
    {
    	/* we have a Lisa. Set it's 'mirror' bit to show we have one,
    	 * but it's features aren't enabled until GFXF_AA_LISA is set
    	 * (by SetChipRev()).
    	 * This is so that we can come up in ECS-Denise mode, but not
    	 * have to bit-swizzle the colours for SuperHires and VGA modes.
    	 */
    	GBASE->ChipRevBits0 |= GFXF_AA_MLISA;

	/* Set up bplcon3 and bplcon4, in case we are running as A or ECS */
	io->bplcon3 = 0x0c00;
	io->bplcon4 = 0x0011;

	/* Also, set the real MemType. AllocBitMap() uses this, as does
	 * Intuition when setting the default sprites.
	 */
    	GBASE->BoardMemType = (((~(io->deniseid)) & 0x0300) >> 8);

	/* Show which bugs are in the Chips. These bits may be cleared by
	 * a SetPatch that comes with a new chip set.
	 */
	 GBASE->Bugs = (BUG_ALICE_LHS | BUG_VBLANK | BUG_ALICE_LHS_PROG);
    }

    /* Hang some data off GfxBase (used by MakeVP() - V39) */
    GBASE->bwshifts = (UBYTE *)&bwshifts;
    GBASE->StrtFetchMasks = (UWORD *)&StrtFetchMasks;
    GBASE->StopFetchMasks = (UWORD *)&StopFetchMasks;
    GBASE->Overrun = (UWORD *)&Overrun;
    GBASE->RealStops = (WORD *)&RealStops;
    GBASE->arraywidth = 2;
    GBASE->ProgData = &ProgData;

    /* Set this up for SetChipRevs() and BootMenu */
    GBASE->WantChips = SETCHIPREV_BEST;

    GBASE->BlitLock = -1;       /* unlocked */
    GBASE->BlitNest = -1;
    NewList(&GBASE->BlitWaitQ);
    NewList(&GBASE->TOF_WaitQ);

    GBASE->LibNode.lib_Node.ln_Name = gfx_lib_name;
	GBASE->LibNode.lib_Version = &VERSION;
	GBASE->LibNode.lib_Revision = &REVISION;
	GBASE->LibNode.lib_Flags |= LIBF_SUMUSED|LIBF_CHANGED;
	GBASE->LibNode.lib_OpenCnt = 1;	/* never expunge */

#ifdef DEBUG
	kprintf("call add library\n");
#endif
	AddLibrary(GBASE);

	GBASE->ColorMask=0xf0000000;	/* will be reset by SetChipRevBits if AA */
	GBASE->SpriteWidth=1;	/* cg - initalize sprite width for AA */
	GBASE->DefaultSpriteWidth=1;
	GBASE->SpriteFMode=0;
	a=(short *)AllocMem(sizeof(struct copinit),MEMF_PUBLIC|MEMF_CHIP|MEMF_CLEAR);
	GBASE->SimpleSprites = (struct SimpleSprite **) AllocMem(sizeof(int *)*8,MEMF_CLEAR);
	/*GBASE->SimpleSpriteViewPorts = AllocMem(sizeof(int *)*8,MEMF_CLEAR);*/
	GBASE->copinit = (struct copinit *) a;

	a = GBASE->copinit->vsync_hblank;
	io->dmacon = BITCLR|DMAF_COPPER;  /* turn copper off */
	io->cop1lc = (UWORD *)a;

/* initial copper start up for sprites */
/* and set dwstart to end of display */
/* 1 move.w to dwstart */
/* 8 move.l to ptrs */
/* wait for vbsrvc */
/* jmp to next copper list  ( 1 moves )*/
/* dummy copper instruction */


/* each sprite is initially pointed to this sprite information */
/* it tells the sprite to wait till the end of the screen */
    /* initialize sprite machine */
    /* set up copper list to initialize all sprites to wait till end of */
    /* screen, then I can play with the control registers and ptrs */

	a = GBASE->copinit->vsync_hblank;

	a=CopyWords(copinit_header,a,7);
	*a++ = ((struct ExecBase *)(SysBase))->ex_Pad0;   /* lets show grey for default on ECS too! */
//	*a++=COPPER_MOVE|(0x1FE&(int)(&io->bplcon3)); *a++= 0xe00;
//	*a++ = COPPER_MOVE|(0x1FE&(int)(&io->color[0]));

	/* Put two NOPs here for now. These will get changed to a BPLCON3 and COLOR0
	 *  the first time update_top_color() is called in AA mode.
	 */
	*a++ = COPPER_MOVE|0x1FE; *a++= 0xe00;
	*a++ = COPPER_MOVE|0x1FE;
	*a++ = ((struct ExecBase *)(SysBase))->ex_Pad0;		/* exec sets boot color now! */

	{
		UWORD *src = copinit_mid_a;
		if (GBASE->ChipRevBits0 & GFXF_HR_DENISE)
		{
			src = copinit_mid_ecs;
		}
		a=CopyWords(src, a, 16);
	}

/*	a = GBASE->copinit->sprstrtup;*/

	for(i=0;i<8;i++)
	{
		*a++=COPPER_MOVE|(REGNUM(spr)+(i<<3));
		*a++=0;
	}

	for(i=0;i<8;i++)
	{
	    /* initialize the comparators for the sprites */
	    io->spr[i].pos = 0xFE00;
	    io->spr[i].ctl = 0xFF00;
	    *a++ = COPPER_MOVE|(0x1FE&(int)(&io->sprpt[i]));    /* upper bits */
	    *a++ = TOBB((long)(GBASE->copinit->sprstop) >> 16);
	    *a++ = COPPER_MOVE|(0x1FE&(int)(&io->sprpt[i]))+2;  /* low bits */
	    *a++ = TOBB((long)(GBASE->copinit->sprstop));
	};

	GBASE->TopLine = TOPLINE;
	a=CopyWords(copinit_end,a,10);


	io->cop2lc =GBASE->copinit->wait_forever;		/* point at wait forever */


    /* the sprite processors begin to pick up the control information at
       line 20 (0x14) of the vertical beam ctr */
    /* after that time the ptrs and control registers may be modified to
       position sprites on the screen, by passing the normal control stuff
       in the hardware sprite data structure */


    /* power up blitter safely */
/*      abo_blit();*/
/*      abo_blit();*/
    io->bltcon0 = WC_ZERO;
    io->bltcon1 = WC_ZERO;
    io->bltsize = (1<<6)|1;
    io->dmacon = BITSET|DMAF_BLITTER;
    io->bltsize = (1<<6)|1;

    if(GBASE->ChipRevBits0 & GFXF_HR_AGNUS)
    {
	io->bltsizv = 0x0000; /* hygiene -- bart */
    }

    /* add task to kernel interrupt server */
    GBASE->vbsrv.is_Code = (void *) &vbasm;
    GBASE->vbsrv.is_Data = (APTR) GBASE;
    GBASE->vbsrv.is_Node.ln_Pri = 10;
    GBASE->vbsrv.is_Node.ln_Name = gfx_lib_name;

#ifdef DEBUG
	kprintf("call addintsrv\n");
	/*Debug();*/
#endif

    /* GBASE->LOFlist = GBASE->copinit->sprstrtup+2*(32+6) */;
    GBASE->SHFlist = GBASE->LOFlist = GBASE->copinit->wait_forever;
    AddIntServer(INTB_VERTB,&GBASE->vbsrv);

#ifdef DEBUG
	kprintf("return from addintsrv\n");
#endif

    GBASE->bltsrv.is_Code = (void *) &bltasm;
    GBASE->bltsrv.is_Data = (APTR)(GBASE);
    GBASE->bltsrv.is_Node.ln_Name = gfx_lib_name;
#ifdef DEBUG
	kprintf("call setintvector\n");
#endif
    SetIntVector(INTB_BLIT,&GBASE->bltsrv);
#ifdef DEBUG
	kprintf("return from setintvector\n");
#endif

    GBASE->timsrv.is_Code = (void *)&timasm;
    GBASE->timsrv.is_Data = (APTR) GBASE;
    GBASE->timsrv.is_Node.ln_Pri = 10;
    GBASE->timsrv.is_Node.ln_Name = gfx_lib_name;

    /* initialize ciaa for me */
	/* the Open Resource must NOT fail */

    GBASE->cia = (long *)OpenResource("ciab.resource");

#ifdef DEBUG
    kprintf("now call EnaICRB cia=%lx\n",GBASE->cia);
#endif
    ADDICRVECTOR(GBASE->cia,2,&GBASE->timsrv);
    ciab.crb |= CIA8520CRBF_ALARM;
    ciab.todhi = 0; /* set hi byte of alarm to 0 */

#ifdef DEBUG
    kprintf("enable interupts\n");
#endif

    Disable();
    GBASE->system_bplcon0 = get_genloc(io) | COLORON;
    GBASE->copinit->diagstrt[1] |= GBASE->system_bplcon0;	/* CG: Poke copinit. Genlock fix for V40. */

    /* initialize to standard defaults */
    GBASE->DisplayFlags |= ((GBASE->system_bplcon0 & GENLOC)|get_pal(agnus_chip_id));
    Enable();

    /* get some memory for association hash table */
    GBASE->hash_table = (void *) AllocMem((HASHTABLE_NUMENTRIES * 4),MEMF_CLEAR);
    /* Initialise the semaphore around GfxAsociate/GfxLookUp/GfxFree.
     * Why did this not happen until 3.01???
     */
    GBASE->HashTableSemaphore = (struct SignalSemaphore *)AllocMem(sizeof(struct SignalSemaphore),0);
    InitSemaphore(GBASE->HashTableSemaphore);


    {
	    struct ExecBase *eb;
	    eb = (struct ExecBase *) GBASE->ExecBase;

	    if (GBASE->DisplayFlags & PAL)	eb->VBlankFrequency = 50;
	    else			eb->VBlankFrequency = 60;
    }
    GBASE->MinDisplayColumn = STANDARD_DENISE_MIN; /* bart -- min left clip */
    GBASE->MaxDisplayColumn = STANDARD_DENISE_MAX; /* bart -- max right clip */

    GBASE->NormalDisplayColumns = 640;
    /* MicrosPerLine time 256 */
    GBASE->MicrosPerLine = 16285;		/* approximately */

	/* single thread access to ActiView hardware copper list */

	GBASE->ActiViewCprSemaphore = (struct SignalSemaphore *) 
				  AllocMem(sizeof(struct SignalSemaphore),0);

	InitSemaphore(GBASE->ActiViewCprSemaphore);

	/* display database initialization relys on last chance memory */
	/* to prevent memory fragmenation in early stages of boot      */

	GBASE->LCMptr = (void *) AllocMem(MAXBYTESPERROW,MEMF_CHIP);
	GBASE->LastChanceMemory = (struct SignalSemaphore *)
				AllocMem(sizeof(struct SignalSemaphore),0);
	InitSemaphore(GBASE->LastChanceMemory);

	/* ok. now open the display database */ 
	GBASE->DisplayInfoDataBase = db_startup();

	/* bart -- mspc */
	NewList(&GBASE->MonitorList);

	GBASE->MonitorListSemaphore = (struct SignalSemaphore *) 
				  AllocMem(sizeof(struct SignalSemaphore),0);

	InitSemaphore(GBASE->MonitorListSemaphore);

	/* initialize default monitorspec and add to list */

	gfx_SetDefaultMonitor(((GBASE->DisplayFlags & PAL) ? (PAL_MONITOR_ID >> 16) : (NTSC_MONITOR_ID >> 16)));

    defras();
#ifdef DEBUG
    kprintf("set dma on\n");
#endif

    defras();

    io->dmacon = BITSET|DMAF_COPPER|DMAF_RASTER|DMAF_SPRITE;

#ifdef DEBUG
    kprintf("call SFInit(%lx)\n",c);
#endif
    SFInit(GBASE);      /* initialize fonts */
#ifdef DEBUG
    kprintf("now return\n");
    Debug();
#endif

    /* open the default monitor */ 
    {
	struct MonitorSpec *mspc;

	mspc = OpenMonitor(NULL,NULL); /* new interface */
	
	activate_mspc( ( DEFAULT_MONITOR_ID ), mspc );

	/* guarantee that the default monitor is available before returning */ 
	all_db_avail(); 
    }
	TestForChunkyHardware();
    io->intena = INTF_BLIT;	/* turn this cycle-sucking bitch off */
#ifdef GBDEBUG
    GBASE->Debug = 1;
#endif
}

void MakeLongFrame(void)
{
	register struct Custom *io;
	WORD vpos;

	/* This code ensures that the frame is Long. When switching from
	 * Lace to non-laced modes, the signal to enable toggling of
	 * LongFrame/ShortFrame is turned off, but the current frame could be
	 * short, thus leaving the system in a permanent ShortFrame mode.
	 */

	io = (struct Custom *)&custom;
	Disable();
	vpos = io->vhposr;
	while ((vpos & 0xff) >= 0x30)	/* a fairly unsafe area */
	{
		vpos = io->vhposr; /* avoid critical region */
	}
	/* the Vertical counts should not change between reading them and
	 * writing them back.
	 */
	io->vposw = 0x8000|io->vposr;
	Enable();
};

unsigned short beamconpal()
{
	if (GBASE->DisplayFlags & PAL)	return (0x20);
	else				return (0);
}

void newdefras(c,hr,vr)
struct GfxBase *c;
int hr,vr;
{
	register struct Custom *io;
	int x;
	int y;

	io = (struct Custom *)&custom;

	io->bplcon0 = 0;
	io->bplcon1 = 0;
	io->bplcon2 = 0x24;
#ifdef BADFMODE
	io->copcon = 2;	/* turn on CDANG bit - what a fuck up */
	FMODE2 = 0;
#endif
	FMODE = 0;

	x =  (hr*0x81)>>4;
	y =  (vr*0x2c)>>4;
	io->diwstrt = (y<<8) | (x & 0xff);

	x = (hr * 0xc1)>>4;
	y = (vr * 0xf4)>>4;
	io->diwstop = (y<<8)||(x & 0xff);

	io->ddfstrt = (0x38*hr)>>4;
	io->ddfstop = (0xd0*hr)>>4;

	MakeLongFrame();
}

void defras()
{
	struct Custom *io;

#ifdef DEBUG
    kprintf("defras(%lx)\n",c);
#endif
	io = (struct Custom *)&custom;
#ifdef BADFMODE
	io->copcon = 2;	/* turn on CDANG bit - what a fuck up */
	FMODE2 = 0;
#endif
	FMODE = 0;
	io->bplcon0 = (GBASE->system_bplcon0 & ~LACE); /* changed to zero bitplanes */
	io->bplcon1 = 0;
	io->bplcon2 = 0x24;
	io->bpl2mod = 0;
	io->bpl1mod = 0;
	io->diwstrt = 0x2c81;
	io->diwstop = 0xf4c1;
	io->ddfstrt = 0x38;
	io->ddfstop = 0xd0;

	/* stuff for new chips */
	io->beamcon0 = beamconpal();	/* disable most all new stuff */

	MakeLongFrame();
}

get_genloc(io)
struct Custom *io;
{
    int count;
    int pos;
    long	WC_ZERO=0;
    /* must be run while disabled */

    /*
	Mike's Genloc test:
	It is important that the test for GenLock is done in the
	active display area.  We need to make sure that no matter
	how slow the machine, the tests below will work.
    */
    while ((vbeampos() > 160) || (vbeampos() < 20));

    /*
	MKS:  The following is my new test for GENLOCK
	What needs to be done is as follows:  We need to
	turn on GenLock mode, wait until it takes effect,
	then check the beam position register to see if
	we are still clocking the custom chips.  The trick
	here is that with a genlock connected, the chips
	will still be clocked from outside but without a
	genlock connected, there will not be any clocking
	and the beam will seem to stop.

	Note that we wait for the beam to be within the
	nominal display area before we try this.  If we
	do not, we will end up getting bad/wrong readings.
	This was already done by the code above...
    */

    /*	First:  We need to turn on GENLOCK modes... */
    io->bplcon0 = (GENLOCK_AUDIO | GENLOCK_VIDEO);

    /*	Ok, we need to wait at least 2 horizontal scan lines in order */
    /*	for the genlock to take effect or not.  Since in worst case, */
    /*	a colour clock is 283ns and there are 160 on a line, we come */
    /*	out to 45280ns for a line.  Add in 15% for genlock timing error */
    /*	and multiply by 2 and we get 104144ns required wait. */
    /*  So, at about 280ns (or so) per chip read, we need 372 chip reads */
    pos=0xFF & DelayAndRead(&(vhposr),372);

    /*	So, in the above, we read the vhposr register a few times */
    /*	We know that this will take 104144ns  In addition, since */
    /*	this is part of the same chip, we know it will remain the */
    /*	same timing. */

    /*	Next, we need to wait for a minimum of 283ns plus the maximum */
    /*	legal error time on the genlock timing.  This means that */
    /*	we must wait 326ns which comes out to 1 chip read. */
    /*	Note that this is an extra CHIP read just to make sure we */
    /*	don't come in too early.. */
    count = vhposr;
    count = vhposr;	/* we need an *extra* read - the 1st read might have
    			 * happened on an edge */

    /*	Now the test below can check if the old position is the same */
    /*	as the new position or not... */

    if (pos == (vhposr & 0xFF))
    {
	/* test to see whether ciaa tod counter is safe for ciaa.device */
	/* if a2000 and jumper j300 tied to power supply, should be running */
	/* if a2000 and jumper j300 tied to vsync, will no longer be running */

	ciaa.todlow = ciaa.todmid = ciaa.todhi = 0;

	ciaa.cra  = CIA8520CRAF_RUNMODE;
	ciaa.talo = 0xFF;
	ciaa.tahi = 0xFF;
	
	while( (BYTE)ciaa.tahi < 0 )
	{
	    if( ciaa.todlow ) GBASE->DisplayFlags |= TODA_SAFE;
	}

	ciaa.cra  = NULL;

	io->bplcon0 = WC_ZERO;

	return(0);
    }

    return(GENLOCK_AUDIO | GENLOCK_VIDEO);
}

get_pal(agnus_chip_id)
UBYTE agnus_chip_id;
{
	if(agnus_chip_id & 0x20) /* new hires agnus */
	{
	    /* an ntsc hires agnus has pin 41 grounded */
	    /* a pal hires agnus has pin 41 open */
	    if(!(agnus_chip_id & 0x10)) return(PAL | REALLY_PAL);
	}
	else
	{
	    /* an ntsc display has 262 lines, counts to 261 */
	    /* a pal display has 312 lines, counts to 311 */
	    /* must be run while disabled */
	    register int i;
	    if ((i = vbeampos()) > 270)  return(PAL);
	    else
	    {
		    /* wait till vbeampos >= 256 */
		    while ( (i = vbeampos()) < 256);
		    do
		    {
			    if (i > 270) return(PAL | REALLY_PAL);
			    i = vbeampos();
		    } while (i > 50);	/* if it falls back then no pal */
		    /* I use 50 figuring the genloc wont reset me higher than */
		    /* that every frame time */
	    }
	}
	return (NTSC);
}

