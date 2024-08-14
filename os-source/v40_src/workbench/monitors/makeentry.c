/******************************************************************************
*
*	$Id: makeentry.c,v 40.2 93/04/28 16:18:17 spence Exp $
*
******************************************************************************/

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <graphics/gfxbase.h>
#include <graphics/displayinfo.h>
#include <graphics/modeid.h>
#include <internal/displayinfo_internal.h>
#include <internal/vp_internal.h>
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <stdio.h>

#include "monitorstuff.h"

#if (DO_MULTISCAN_MONITOR)
#include "multiscan.h"
#endif

#if (DO_DOUBLE_NTSC_MONITOR)
#include "DblNTSC.h"
#endif

#if (DO_DOUBLE_PAL_MONITOR)
#include "DblPAL.h"
#endif

#if (DO_SUPER72_MONITOR)
#include "Super72.h"
#endif

#if (DO_EURO72_MONITOR)
#include "Euro72.h"
#endif

#if (DO_EURO36_MONITOR)
#include "Euro36.h"
#endif

#if (DO_A2024_MONITOR)
#include "A2024.h"
#endif

#if (DO_MOTIVATOR_MONITOR)
#include "motivator.h"
#endif

#if (DO_MOTIVATIM_MONITOR)
#include "motivatim.h"
#endif

#pragma libcall GfxBase AddDisplayInfoData 2e8 2109805
struct DisplayInfoRecord * AddDisplayInfoData(struct DisplayInfoRecord * handle,
                                              UBYTE * buf,
                                              ULONG size,
                                              ULONG tagID,
                                              ULONG ID);

extern struct GfxBase *GfxBase;
extern struct ExecBase *SysBase;

/*#define _DEBUG*/

#ifdef _DEBUG
#define D(x) {x}
#else
#define D(x)
#endif

struct DisplayInfoRecord *add_record(struct RecordNode *root, struct RecordNode *record)
{
    D(kprintf("add_record(root = 0x%lx, record = 0x%lx\n", root, record);)

    if( root && record )
    {
	struct RecordNode **sibling;

	/* does this root already have children? */
	for( sibling = &root->rcn_Child;
	   (*sibling && (*sibling)->rcn_Succ );
	     sibling = &(*sibling)->rcn_Succ );

	D(kprintf("sibling = 0x%lx, *sibling = 0x%lx\n", sibling, (*sibling ? *sibling : NULL));)

	Forbid();

	record->rcn_Pred   = NULL;
	record->rcn_Parent = root;

	/* long identifier */
	((struct DisplayInfoRecord*)record)->rec_MajorKey =
	((struct DisplayInfoRecord*)  root)->rec_MinorKey;

	if( *sibling )  /* one child among many */
	{
	    (*sibling)->rcn_Succ = record;
	    record->rcn_Pred = (*sibling);
	}
	else            /* only child */
	{
	    root->rcn_Child = record;
	}

	Permit();
    }
    else
    {
	record = NULL;
    }

    return((struct DisplayInfoRecord *)record);
}

#define RAWMNTR_SKIP 9
#define RAWDIMS_SKIP 8
#define RAWDISP_SKIP 4
#define RAWVEC_SKIP 2

void do_db_startup(struct DisplayInfoRecord *root, UWORD MonitorID)
{
    struct DisplayInfoRecord *record, *subrecord, *child, *origrecord;
    struct ProgInfo *progdata = GfxBase->ProgData, *ThisProgData = NULL;
    APTR entry;
    UWORD YDim, XDim, ID;
    #if ((DO_SUPER72_MONITOR) || (DO_EURO36_MONITOR) || (DO_A2024_MONITOR) || (DO_MOTIVATOR_MONITOR) || (DO_MOTIVATIM_MONITOR))
    UWORD IDs[UNIQUEIDS] =
    {
	0x1000,		/* normal */
	0x1400,		/* dpf */
	0x1440,		/* dpf2 */
	0x1800,		/* ham */
	0x1080,		/* ehb */
    };
    #else
    UWORD IDs[UNIQUEIDS] =
    {
	0x1004,		/* normal */
	0x1404,		/* dpf */
	0x1444,		/* dpf2 */
	0x1804,		/* ham */
	0x1084,		/* ehb */
    };
    #endif
    UWORD BplCon2[UNIQUEIDS] =
    {
	VGABPLCON2,	/* normal */
	VGABPLCON2,	/* dpf */
	VGABPLCON2 | 0x40,	/* dpf2 */
	VGABPLCON2,	/* ham */
	VGAPLFD_PRIO,	/* ehb */
    };
    UWORD ResnSpeeds[] =
    {
	35,
	70,
	140,
    };
    #if ((DO_EURO36_MONITOR) || (DO_A2024_MONITOR))
    UWORD E36VecTypes[][] =	/* array of VecTypes */
    {
	{2, 1, 0},		/* SuperHires, Hires, Lores */
	{9, 8, 7},		/* DPF variants */
	{12, 11, 10},		/* DPF2 variants */
	{5, 4, 3},		/* HAM variants */
	{27, 26, 6},		/* EHB variants */
    };
    #define A2024VecTypes E36VecTypes
    #endif
    UWORD VecTypes[][] =	/* array of VecTypes for progbeamsync modes */
    {
	{13, 16, 15},		/* Productivity, Lores, ExtraLores */
	{22, 21, 20},		/* DPF variants */
	{25, 24, 23},		/* DPF2 variants */
	{14, 18, 17},		/* HAM variants */
	{29, 28, 19},		/* EHB variants */
    };
    UBYTE crb = GfxBase->ChipRevBits0;
    UBYTE memtype = GfxBase->MemType;
    BOOL IsECS = (crb & GFXF_HR_DENISE);
    BOOL IsBigAgnus = (crb & GFXF_HR_AGNUS);
    BOOL IsAA = (crb & GFXF_AA_LISA);
    UWORD Fetch1 = FETCH1;
    #undef FETCH1
    #define FETCH1 Fetch1
    UWORD PixResY;
    UWORD SpriteResY = SPRITE_RES_Y;
    BOOL RomBug = FALSE;
    #if ((DO_DOUBLE_NTSC_MONITOR) || (DO_DOUBLE_PAL_MONITOR))
    /* DblNTSC/PAL screens get clipped on the RHS with kickstart 39.106 and VGAOnly,
     * but all other monitors are OK. (graphics.library = 39.89 in kickstart 39.106)
     */
    RomBug = ((GfxBase->LibNode.lib_Version == 39) && (GfxBase->LibNode.lib_Revision <= 89) && (!(GfxBase->Bugs & BUG_ALICE_LHS_PROG)));
    #endif
    
    D(kprintf("do_db_startup(root = 0x%lx, MonitorID = 0x%lx\n", root, MonitorID);)
    /* This code works for all the VGA-type modes. */

    /* First, the MonitorInfo */
    if ((record = AllocMem(sizeof(struct DisplayInfoRecord), MEMF_PUBLIC | MEMF_CLEAR)) && 
        (subrecord = AllocMem(sizeof(struct DisplayInfoRecord), MEMF_PUBLIC | MEMF_CLEAR)))
    {
	record->rec_MinorKey = MonitorID;
	subrecord->rec_MinorKey = (PREFERRED_MODEID & 0xffff);
	if (entry = AllocMem(sizeof(struct RawMonitorInfo), MEMF_PUBLIC | MEMF_CLEAR))
	{
		struct RawMonitorInfo *mntr;
		mntr = (struct RawMonitorInfo *)entry;

		D(kprintf("FETCH1 = 0x%lx, VPX = 0x%lx, VPY = 0x%lx\n", FETCH1, VPX, VPY);)
		D(kprintf("VPRANGEX = 0x%lx, VPRANGEY = 0x%lx\n", VPRANGEX, VPRANGEY);)
		D(kprintf("(0x%lx, 0x%lx) - (0x%lx, 0x%lx)\n", VPMINX, VPMINY, VPMAXX, VPMAXY);)

    		/* fill in the QueryHeader */
		mntr->Header.StructID = DTAG_MNTR;
    		mntr->Header.DisplayID = ((MonitorID << 16) | EXTENDED_MODE);
		mntr->Header.SkipID = TAG_SKIP;
    		mntr->Header.Length = RAWMNTR_SKIP;
		
		mntr->ViewResolution.x = VRESNX;
		mntr->ViewResolution.y = VRESNY;
		mntr->ViewPositionRange.MinX = VPMINX;
		mntr->ViewPositionRange.MinY = VPMINY;
		mntr->ViewPositionRange.MaxX = VPMAXX;
		mntr->ViewPositionRange.MaxY = VPMAXY;
		mntr->DefaultViewPosition.x = VPX;
		mntr->DefaultViewPosition.y = VPY;
		mntr->ViewPosition.x = mntr->DefaultViewPosition.x;
		mntr->ViewPosition.y = mntr->DefaultViewPosition.y;
		mntr->MouseTicks.x = MOUSETICKS_X;
		mntr->MouseTicks.y = MOUSETICKS_Y;

		mntr->TotalRows = TOTROWS;
		mntr->TotalColorClocks = TOTCLKS;
		mntr->MinRow = MINROW;
		mntr->Compatibility = COMPAT;

		mntr->TxtOScan.MinX = (NOM_MINX * PIX_RES_X);
		mntr->TxtOScan.MinY = (NOM_MINY * PIX_RES_Y);
		mntr->TxtOScan.MaxX = ((NOM_MINX + NOM_WIDTH * PIX_RES_X) - 1);
		mntr->TxtOScan.MaxY = ((NOM_MINY + NOM_HEIGHT * PIX_RES_Y) - 1);

		mntr->StdOScan.MinX = mntr->TxtOScan.MinX;
		mntr->StdOScan.MinY = mntr->TxtOScan.MinY;
		mntr->StdOScan.MaxX = mntr->TxtOScan.MaxX;
		mntr->StdOScan.MaxY = mntr->TxtOScan.MaxY;

		mntr->PreferredModeID = PREFERRED_MODEID;

		if (child = add_record(root, record))
		{
			add_record(child, subrecord);
			AddDisplayInfoData(subrecord, (UBYTE *)mntr, sizeof(struct RawMonitorInfo), DTAG_MNTR, INVALID_ID);
		}

	}
    }
    record = origrecord = subrecord;

    #if ((DO_EURO36_MONITOR) || (DO_A2024_MONITOR) || (DO_MOTIVATOR_MONITOR) || (DO_MOTIVATIM_MONITOR))
    PixResY = PIX_RES_Y;
    #else
    PixResY = (PIX_RES_Y << (IsAA ? 1 : 0));
    #endif

    /* Loops -
     * Lace/Non-lace/SDbled
     *    35ns, 70ns, 140ns,
     *        NORM, HAM, EHB, DPF, DPF2
     */

    for (YDim = LOW_Y_RESN; YDim <= IS_LACE; YDim++)
    {
	UWORD PixResX = PIX_RES_X;
	UWORD SpriteResX = SPRITE_RES_X;
	UWORD MaxDepth;
	D(kprintf("YDimloop = %ld\n", YDim);)

    	for (XDim = HIGH_X_RESN; XDim <= LOW_X_RESN; XDim++)
    	{
		D(kprintf("XDimloop = %ld\n", XDim);)

		for (ID = 0; ID < LASTID; ID++)
		{
		    if (!((YDim == SDBLED) && ((ID == IDDPF) || (ID == IDDPF2))))
		    {

	    		/* The main work: */
			/* First, calculate the ID */
			ULONG ModeID = EXTENDED_MODE;
			ULONG bplcon0 = 1;	/* ECSENA */
			UWORD ThisID = IDs[ID];
			#if (DO_A2024_MONITOR)
			bplcon0 = 0;
			#endif
			D(kprintf("IDLoop = %ld\n", ID);)

			switch (XDim)
			{
				case RESN_35:
				{
					#if ((DO_DOUBLE_NTSC_MONITOR) || (DO_DOUBLE_PAL_MONITOR))
					{
						ThisID |= 0x8000;
					}
					#else
					{
	 					ThisID |= 0x8020;
					}
					#endif
					ModeID |= SUPERHIRES;
					bplcon0 |= 0x40;	/* SHRES */
					MaxDepth = 2;
					break;
				}
				case RESN_70:
				{
					#if ((DO_DOUBLE_NTSC_MONITOR) || (DO_DOUBLE_PAL_MONITOR))
					{
					}
					#else
					{
						ThisID |= 0x8000;
					}
					#endif
					ModeID |= HIRES;
					bplcon0 |= HIRES;
					MaxDepth = 4;
					break;
				}
				case RESN_140:
				{
					#if ((DO_DOUBLE_NTSC_MONITOR) || (DO_DOUBLE_PAL_MONITOR))
					{
						ThisID |= 0x200;
					}
					#endif
					/* Lores usually has 5 bitplanes, but
					 * HAM, EHB and DPF can have 6.
					 */
					MaxDepth = ((ID == IDNORM) ? 5 : 6);
					break;
				}
			}
			if (YDim == IS_LACE)
			{
				#if ((DO_SUPER72_MONITOR) || (DO_EURO36_MONITOR) || (DO_MOTIVATOR_MONITOR) || (DO_MOTIVATIM_MONITOR))
				{
					ThisID |= LACE;
				}
				#else
				{
					ThisID |= 1;
				}
				#endif
				ModeID |= LACE;
			}
			if (YDim == SDBLED)
			{
				ThisID &= ~4;
				ModeID |= DOUBLESCAN;
				#if (DO_SUPER72_MONITOR)
				{
					ThisID |= DOUBLESCAN;
				}
				#endif
			}

			if (IsAA)
			{
				MaxDepth <<= GfxBase->bwshifts[memtype];
			}

			/* rely on the fact that the MonitorInfo is using a ModeID
			 * of normal 35ns non-laced.
			 */
	    		if (record)
    			{
				struct RawDimensionInfo *dims;
				struct RawDisplayInfo *disp;
				struct RawVecInfo *vec;

    				record->rec_MinorKey = ThisID;
    				D(kprintf("ThisID = 0x%lx, ModeID = 0x%lx\n", ThisID, ModeID);)
    				/* First, the DimensionInfo */
    				if (entry = AllocMem(sizeof(struct RawDimensionInfo), MEMF_PUBLIC | MEMF_CLEAR))
    				{
    					dims = (struct RawDimensionInfo *)entry;
    					/* fill in the QueryHeader */
					dims->Header.StructID = DTAG_DIMS;
    					dims->Header.DisplayID = ((MonitorID << 16) | ThisID);
					dims->Header.SkipID = TAG_SKIP;
    					dims->Header.Length = RAWDIMS_SKIP;

					/* The MaxDepth depends on chips, bandwidth and mode */
					if (ID == IDEHB)
					{
						dims->HWMaxDepth = 6;
					}
					else
					{
						dims->HWMaxDepth = min(8, MaxDepth);
					}
					#if (DO_A2024_MONITOR)
					dims->MaxDepth = dims->HWMaxDepth = 2;
					dims->MinRasterWidth = NOM_WIDTH;
					dims->MinRasterHeight = NOM_HEIGHT;
					#else
					dims->MaxDepth = dims->HWMaxDepth;
					dims->MinRasterWidth = 16;
					dims->MinRasterHeight = 1;
					#endif
					dims->MaxRasterWidth = (IsBigAgnus ? 16368 : 1008);
					dims->MaxRasterHeight = (IsBigAgnus ? 16384 : 1024);

					dims->Nominal.MinX = (NOM_MINX * PIX_RES_X);
					dims->Nominal.MinY = (NOM_MINY * PIX_RES_Y);
					dims->Nominal.MaxX = ((NOM_MINX + NOM_WIDTH * PIX_RES_X) - 1);
					dims->Nominal.MaxY = ((NOM_MINY + NOM_HEIGHT * PIX_RES_Y) - 1);

					D(kprintf("MaxOScan = (%ld, %ld) - (%ld, %ld)\n", MAX_MINX, MAX_MINY, MAX_MAXX, MAX_MAXY);)
					dims->MaxOScan.MinX = (MAX_MINX * PIX_RES_X);
					dims->MaxOScan.MinY = (MAX_MINY * PIX_RES_Y);
					dims->MaxOScan.MaxX = ((MAX_MAXX * PIX_RES_X) - 1);
					dims->MaxOScan.MaxY = ((MAX_MAXY * PIX_RES_Y) - 1);

					D(kprintf("VideoOScan = (%ld, %ld)", VIDEO_MINX, VIDEO_MINY);)
					D(kprintf(" - (%ld, %ld)\n",VIDEO_MAXX, VIDEO_MAXY);)
					dims->VideoOScan.MinX = (VIDEO_MINX * PIX_RES_X);
					dims->VideoOScan.MinY = (VIDEO_MINY * PIX_RES_Y);
					dims->VideoOScan.MaxX = ((VIDEO_MAXX * PIX_RES_X) - 1);
					dims->VideoOScan.MaxY = ((VIDEO_MAXY * PIX_RES_Y) - 1);

					if ((record == subrecord) || (add_record(child, record)))
					{
						AddDisplayInfoData(record, (UBYTE *)dims, sizeof(struct RawDimensionInfo), DTAG_DIMS, INVALID_ID);
					}
					FreeMem(entry, sizeof(struct RawDimensionInfo));
				}

    				if (entry = AllocMem(sizeof(struct RawDisplayInfo), MEMF_PUBLIC | MEMF_CLEAR))
    				{
					ULONG property = MIN_DIPF_FLAGS;
					UWORD avail  = (!IsECS ? DI_AVAIL_NOCHIPS : 0);
					UWORD palette = (IsAA ? -1 : (XDim == RESN_35 ? 64 : 4096));
    					disp = (struct RawDisplayInfo *)entry;
    					/* fill in the QueryHeader */
					disp->Header.StructID = DTAG_DISP;
    					disp->Header.DisplayID = ((MonitorID << 16) | ThisID);
					disp->Header.SkipID = TAG_SKIP;
    					disp->Header.Length = RAWDISP_SKIP;
					if ((ID == IDEHB) || (ID == IDHAM))
					{
						/* check we have the bandwidth */
						if (((XDim == RESN_35) && (!(memtype == BANDWIDTH_4X))) ||
						    ((XDim == RESN_70) && (!memtype)))
						{
							avail |= DI_AVAIL_NOCHIPS;
						}
					}
					D(kprintf("avail = 0x%lx\n", avail);)
					disp->NotAvailable = avail;
					if (property & DIPF_IS_SPRITES)
					{
						if ((IsECS) && (!(XDim == RESN_35)))
						{
							/* 35ns modes cannot attach sprites */
							property |= DIPF_IS_SPRITES_ATT;
						}
						if (IsAA)
						{
							/* Add the AA Sprite features */
							#if (DO_A2024_MONITOR)
							property |= (DIPF_IS_SPRITES_ATT | DIPF_IS_SPRITES_CHNG_PRI);
							#else
							property |= (DIPF_IS_SPRITES_ATT | DIPF_IS_SPRITES_CHNG_RES | 
								     DIPF_IS_SPRITES_BORDER | DIPF_IS_SPRITES_CHNG_BASE | DIPF_IS_SPRITES_CHNG_PRI);
							#endif
						}
					}
					switch (ID)
					{
						case (IDNORM) :
						{
							/* Is this a WB mode? */
							if (((XDim == RESN_35) && (WB_35)) ||
							    ((XDim == RESN_70) && (WB_70)) ||
							    ((XDim == RESN_140) && (WB_140)))
							{
								property |= DIPF_IS_WB;
							}
							break;
						}
						case (IDDPF) :
						{
							property |= DIPF_IS_DUALPF;
							ModeID |= DUALPF;
							bplcon0 |= DUALPF;
							break;
						}
						case (IDDPF2) :
						{
							property |= (DIPF_IS_DUALPF | DIPF_IS_PF2PRI);
							ModeID |= (DUALPF | PFBA);
							bplcon0 |= (DUALPF | PFBA);
							break;
						}
						case (IDHAM) :
						{
							property |= DIPF_IS_HAM;
							if (XDim != RESN_140)
							{
								property &= ~DIPF_IS_ECS;
								property |= DIPF_IS_AA;
							}
							ModeID |= HAM;
							bplcon0 |= HAM;
							break;
						}
						case (IDEHB) :
						{
							property |= DIPF_IS_EXTRAHALFBRITE;
							if (XDim != RESN_140)
							{
								property &= ~DIPF_IS_ECS;
								property |= DIPF_IS_AA;
							}
							ModeID |= EXTRA_HALFBRITE;
							bplcon0 |= EXTRA_HALFBRITE;
							break;
						}
						default : break;
					}
					if (YDim == IS_LACE)
					{
						property |= DIPF_IS_LACE;
					}
					if (YDim == SDBLED)
					{
						property |= (DIPF_IS_SCANDBL | DIPF_IS_AA);
						property &= ~DIPF_IS_ECS;
					}
					disp->PropertyFlags = property;
					disp->Resolution.x = PixResX;
					disp->Resolution.y = PixResY;
					disp->PixelSpeed = ResnSpeeds[XDim];
					disp->NumStdSprites = 8;
					disp->PaletteRange = palette;
					disp->SpriteResolution.x = SpriteResX;
					disp->SpriteResolution.y = SpriteResY;
					disp->ModeID = ((MONITOR_NUM << 16) | ModeID);
					disp->RedBits = disp->GreenBits = disp->BlueBits = (IsAA ? 8 : (XDim == RESN_35 ? 2 : 4));
					#if (DO_A2024_MONITOR)
					disp->NotAvailable = 0;
					disp->PaletteRange = 4;
					disp->RedBits = 2; disp->GreenBits = 3; disp->BlueBits = 1;
					#endif


					AddDisplayInfoData(record, (UBYTE *)disp, sizeof(struct RawDisplayInfo), DTAG_DISP, INVALID_ID);
					FreeMem(entry, sizeof(struct RawDisplayInfo));
				}

    				if ((entry = AllocMem(sizeof(struct RawVecInfo), MEMF_PUBLIC | MEMF_CLEAR)) &&
    				    ((!(YDim == SDBLED)) || (ThisProgData = (struct ProgInfo *)AllocMem(sizeof(struct ProgInfo), MEMF_PUBLIC | MEMF_CLEAR))))
    				{
    					vec = (struct RawVecInfo *)entry;
    					/* fill in the QueryHeader */
					vec->Header.StructID = DTAG_VEC;
    					vec->Header.DisplayID = ((MonitorID << 16) | ThisID);
					vec->Header.SkipID = TAG_SKIP;
    					vec->Header.Length = RAWVEC_SKIP;
					vec->Type = VecTypes[ID][XDim];
					#if (DO_EURO36_MONITOR)
					if (IsAA && (GfxBase->Bugs & BUG_ALICE_LHS))
					{
						vec->Type = E36VecTypes[ID][XDim];
					}
					#endif
					#if (DO_A2024_MONITOR)
					vec->Type = A2024VecTypes[ID][XDim];
					#endif
					if (YDim != SDBLED)
					{
						vec->Data = (APTR)(progdata + vec->Type);
					}
					else
					{
						/* have to build the ProgData structure, because it is not in ROM */
						ThisProgData->bplcon0 = bplcon0;
						ThisProgData->bplcon2 = BplCon2[ID];
						ThisProgData->ToViewX = (2 - XDim);
						ThisProgData->Flags = (PROGINFO_NATIVE | PROGINFO_VARBEAM | PROGINFO_SCANDBL | (ID == IDHAM ? PROGINFO_HAM : 0));
						ThisProgData->MakeItType = (ID >= IDDPF ? 1 : 0);
						ThisProgData->ScrollVPCount = 6;
						ThisProgData->DDFSTRTMask = 0xfff8;
						ThisProgData->DDFSTOPMask = 0xfff8;
						ThisProgData->ToDIWResn = XDim;
						ThisProgData->Offset = XDim;
						vec->Data = (APTR)ThisProgData;
					}
					AddDisplayInfoData(record, (UBYTE *)vec, sizeof(struct RawVecInfo), DTAG_VEC, INVALID_ID);
					FreeMem(entry, sizeof(struct RawVecInfo));
				}
			}
			record = AllocMem(sizeof(struct DisplayInfoRecord), MEMF_PUBLIC | MEMF_CLEAR);
		    }
		}
		PixResX <<= 1;
		#if (!(DO_EURO36_MONITOR))
		{
			SpriteResX <<= 1;
		}
		#endif
		#if (DO_A2024_MONITOR)
		PixResX >>= 1;
		SpriteResX >>= 2;
		#endif
	}
	PixResY >>= 1;
	#if (DO_A2024_MONITOR)
	YDim ++;
	#endif
    }
    /* We didn't need that last record */
    FreeMem(record, sizeof(struct DisplayInfoRecord));

    #if (DO_DOUBLE_NTSC_MONITOR)
    /* Map the "ScanDbl" DPF modes to the FFixed modes. */
    {
	struct DisplayInfoRecord *record, *model;
	DisplayInfoHandle handle;
	D(kprintf("ExtraLores DPF\n");)
	if (record = AllocMem(sizeof(struct DisplayInfoRecord), MEMF_PUBLIC | MEMF_CLEAR))
	{
		if (handle = FindDisplayInfo(0x91604))
		{
			model = (struct DisplayInfoRecord *)handle;
			record->rec_MajorKey = model->rec_MajorKey;
			record->rec_MinorKey = (model->rec_MinorKey & ~4);
			record->rec_Tag = model->rec_Tag;
			add_record(child, record);
		}
	}
	D(kprintf("ExtraLores DPF2\n");)
	if (record = AllocMem(sizeof(struct DisplayInfoRecord), MEMF_PUBLIC | MEMF_CLEAR))
	{
		if (handle = FindDisplayInfo(0x91644))
		{
			model = (struct DisplayInfoRecord *)handle;
			record->rec_MajorKey = model->rec_MajorKey;
			record->rec_MinorKey = (model->rec_MinorKey & ~4);
			record->rec_Tag = model->rec_Tag;
			add_record(child, record);
		}
	}
	D(kprintf("Lores DPF\n");)
	if (record = AllocMem(sizeof(struct DisplayInfoRecord), MEMF_PUBLIC | MEMF_CLEAR))
	{
		if (handle = FindDisplayInfo(0x91404))
		{
			model = (struct DisplayInfoRecord *)handle;
			record->rec_MajorKey = model->rec_MajorKey;
			record->rec_MinorKey = (model->rec_MinorKey & ~4);
			record->rec_Tag = model->rec_Tag;
			add_record(child, record);
		}
	}
	D(kprintf("Lores DPF2\n");)
	if (record = AllocMem(sizeof(struct DisplayInfoRecord), MEMF_PUBLIC | MEMF_CLEAR))
	{
		if (handle = FindDisplayInfo(0x91444))
		{
			model = (struct DisplayInfoRecord *)handle;
			record->rec_MajorKey = model->rec_MajorKey;
			record->rec_MinorKey = (model->rec_MinorKey & ~4);
			record->rec_Tag = model->rec_Tag;
			add_record(child, record);
		}
	}
	D(kprintf("Hires DPF\n");)
	if (record = AllocMem(sizeof(struct DisplayInfoRecord), MEMF_PUBLIC | MEMF_CLEAR))
	{
		if (handle = FindDisplayInfo(0x99404))
		{
			model = (struct DisplayInfoRecord *)handle;
			record->rec_MajorKey = model->rec_MajorKey;
			record->rec_MinorKey = (model->rec_MinorKey & ~4);
			record->rec_Tag = model->rec_Tag;
			add_record(child, record);
		}
	}
	D(kprintf("Hires DPF2\n");)
	if (record = AllocMem(sizeof(struct DisplayInfoRecord), MEMF_PUBLIC | MEMF_CLEAR))
	{
		if (handle = FindDisplayInfo(0x99444))
		{
			model = (struct DisplayInfoRecord *)handle;
			record->rec_MajorKey = model->rec_MajorKey;
			record->rec_MinorKey = (model->rec_MinorKey & ~4);
			record->rec_Tag = model->rec_Tag;
			add_record(child, record);
		}
	}
    }
    #endif

    #if (DO_DOUBLE_PAL_MONITOR)
    /* Map the "ScanDbl" DPF modes to the FFixed modes. */
    {
	struct DisplayInfoRecord *record, *model;
	DisplayInfoHandle handle;
	D(kprintf("ExtraLores DPF\n");)
	if (record = AllocMem(sizeof(struct DisplayInfoRecord), MEMF_PUBLIC | MEMF_CLEAR))
	{
		if (handle = FindDisplayInfo(0xa1604))
		{
			model = (struct DisplayInfoRecord *)handle;
			record->rec_MajorKey = model->rec_MajorKey;
			record->rec_MinorKey = (model->rec_MinorKey & ~4);
			record->rec_Tag = model->rec_Tag;
			add_record(child, record);
		}
	}
	D(kprintf("ExtraLores DPF2\n");)
	if (record = AllocMem(sizeof(struct DisplayInfoRecord), MEMF_PUBLIC | MEMF_CLEAR))
	{
		if (handle = FindDisplayInfo(0xa1644))
		{
			model = (struct DisplayInfoRecord *)handle;
			record->rec_MajorKey = model->rec_MajorKey;
			record->rec_MinorKey = (model->rec_MinorKey & ~4);
			record->rec_Tag = model->rec_Tag;
			add_record(child, record);
		}
	}
	D(kprintf("Lores DPF\n");)
	if (record = AllocMem(sizeof(struct DisplayInfoRecord), MEMF_PUBLIC | MEMF_CLEAR))
	{
		if (handle = FindDisplayInfo(0xa1404))
		{
			model = (struct DisplayInfoRecord *)handle;
			record->rec_MajorKey = model->rec_MajorKey;
			record->rec_MinorKey = (model->rec_MinorKey & ~4);
			record->rec_Tag = model->rec_Tag;
			add_record(child, record);
		}
	}
	D(kprintf("Lores DPF2\n");)
	if (record = AllocMem(sizeof(struct DisplayInfoRecord), MEMF_PUBLIC | MEMF_CLEAR))
	{
		if (handle = FindDisplayInfo(0xa1444))
		{
			model = (struct DisplayInfoRecord *)handle;
			record->rec_MajorKey = model->rec_MajorKey;
			record->rec_MinorKey = (model->rec_MinorKey & ~4);
			record->rec_Tag = model->rec_Tag;
			add_record(child, record);
		}
	}
	D(kprintf("Hires DPF\n");)
	if (record = AllocMem(sizeof(struct DisplayInfoRecord), MEMF_PUBLIC | MEMF_CLEAR))
	{
		if (handle = FindDisplayInfo(0xa9404))
		{
			model = (struct DisplayInfoRecord *)handle;
			record->rec_MajorKey = model->rec_MajorKey;
			record->rec_MinorKey = (model->rec_MinorKey & ~4);
			record->rec_Tag = model->rec_Tag;
			add_record(child, record);
		}
	}
	D(kprintf("Hires DPF2\n");)
	if (record = AllocMem(sizeof(struct DisplayInfoRecord), MEMF_PUBLIC | MEMF_CLEAR))
	{
		if (handle = FindDisplayInfo(0xa9444))
		{
			model = (struct DisplayInfoRecord *)handle;
			record->rec_MajorKey = model->rec_MajorKey;
			record->rec_MinorKey = (model->rec_MinorKey & ~4);
			record->rec_Tag = model->rec_Tag;
			add_record(child, record);
		}
	}
    }
    #endif

}