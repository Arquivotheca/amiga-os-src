/* dispatch.c
 *
 */

#include "classbase.h"
#include "classdata.h"

#include <hardware/custom.h>
#include <hardware/dmabits.h>
#include <hardware/intbits.h>
#include <gadgets/tapedeck.h>

/*****************************************************************************/

#define	DB(x)	;

/*****************************************************************************/

/* Trigger methods we support */
struct DTMethod tm[] =
{
    {"Rewind",		"REWIND",	STM_REWIND},
    {"Play",		"PLAY",		STM_PLAY},
    {"Fast Forward",	"FF",		STM_FASTFORWARD},
    {"Stop",		"STOP",		STM_STOP},
    {"Pause",		"PAUSE",	STM_PAUSE},
    {NULL,},
};

/*****************************************************************************/

/* Methods we support */
ULONG m[] =
{
    OM_NEW,
    OM_GET,
    OM_SET,
    OM_UPDATE,
    OM_DISPOSE,
    GM_LAYOUT,
    GM_HITTEST,
    GM_GOACTIVE,
    GM_HANDLEINPUT,
    GM_RENDER,

    DTM_FRAMEBOX,
    DTM_TRIGGER,

    DTM_CLEARSELECTED,
    DTM_COPY,
    DTM_PRINT,
    DTM_WRITE,

    ~0,
};

/*****************************************************************************/

void SetTapeDeckAttrs (struct ClassBase *cb, struct localData *lod, ULONG tags, ...)
{
    if (lod->lod_Window && (lod->lod_Flags & LODF_CONTROLS))
	SetGadgetAttrsA ((struct Gadget *) lod->lod_TapeDeck, lod->lod_Window, NULL, (struct TagItem *) &tags);
}

/*****************************************************************************/

void freeExtraInfo (struct ClassBase * cb, struct localData * lod)
{
    LONG i;

    if (lod->lod_NumAlloc)
    {
	for (i = (LONG) (lod->lod_NumAlloc - 1); i >= 0; i--)
	    ReleasePen (lod->lod_ColorMap, lod->lod_Allocated[i]);
	lod->lod_NumAlloc = 0L;
    }
}

/*****************************************************************************/

ULONG handleMethod (struct ClassBase * cb, Class * cl, Object * o, struct gpInput * gpi)
{
    struct localData *lod = INST_DATA (cl, o);
    struct InputEvent *ie = gpi->gpi_IEvent;
    ULONG retval = GMR_MEACTIVE;
    LONG clock;
    UWORD term;

    /* Mouse down */
    if ((ie->ie_Code == SELECTDOWN) && (lod->lod_Flags & LODF_CONTROLS))
    {
	/* Don't come back */
	retval = GMR_NOREUSE;

	/* See if they hit the control panel */
	if ((gpi->gpi_Mouse.Y >= lod->lod_TapeY) && (gpi->gpi_Mouse.Y <= lod->lod_TapeY + 16))
	{
	    /* The tapedeck is active */
	    lod->lod_Active = lod->lod_TapeDeck;
	}
    }
    else if ((ie->ie_Code == SELECTUP) && !(lod->lod_Flags & LODF_CONTROLS))
    {
	/* Locate the frame and start playing */
	SetConductorState (lod->lod_Player, CONDSTATE_LOCATE, lod->lod_ClockTime);

	retval = GMR_NOREUSE | GMR_VERIFY;
    }

    /* Make sure we have an active gadget */
    if (lod->lod_Active)
    {
	/* Adjust the mouse (never the X because it is the same) */
	gpi->gpi_Mouse.Y -= (lod->lod_TapeY + 1);

	/* Pass the message to the active gadget */
	retval = DoMethodA ((Object *) lod->lod_Active, (Msg) gpi);

	/* Get the code value */
	term = *gpi->gpi_Termination;

	/* Pause if we are playing */
	if (lod->lod_Flags & LODF_PLAYING)
	    SetConductorState (lod->lod_Player, CONDSTATE_PAUSED, 0);

	if (term & 0x8000)
	{
	    clock = (LONG) (term - 0x8000);
	    if (clock != lod->lod_Frame)
	    {
		SetConductorState (lod->lod_Player, CONDSTATE_SHUTTLE, (ULONG) clock * lod->lod_TicksPerFrame);
	    }
	}
	else if ((ie->ie_Class == IECLASS_TIMER) && ((term == 0) || (term == 2)))
	{
	    if (((struct Gadget *)lod->lod_Active)->Flags & GFLG_SELECTED)
	    {
	    clock = (LONG) lod->lod_Frame;
	    if (term == 0)
		clock -= lod->lod_Adjust;
	    else
		clock += lod->lod_Adjust;
	    if (clock < 0)
		clock = 0;
	    else if (clock > lod->lod_NumFrames)
		clock = lod->lod_NumFrames;

	    if (clock != lod->lod_Frame)
	    {
		SetConductorState (lod->lod_Player, CONDSTATE_SHUTTLE, (ULONG) clock * lod->lod_TicksPerFrame);
	    }
	    }
	}

	/* Handle termination */
	if ((retval == (GMR_NOREUSE | GMR_VERIFY)) || ((ie->ie_Code == SELECTDOWN) && (term & 0x8000)))
	{
	    /* See if they released the slider */
	    if ((retval == (GMR_NOREUSE | GMR_VERIFY)) && (term & 0x8000))
	    {
		/* Shuttle to the frame */
		clock = (LONG) (term - 0x8000);
		SetConductorState (lod->lod_Player, CONDSTATE_SHUTTLE, (ULONG) clock * lod->lod_TicksPerFrame);
	    }
	    else
	    {
		if (term & 0x1000)
		{
		    /* Pause */
		    SetConductorState (lod->lod_Player, CONDSTATE_PAUSED, 0);
		}
		else if (term & 0x8000)
		{
		    /* Tell it to pause */
		    SetConductorState (lod->lod_Player, CONDSTATE_PAUSED, 0);
		}
		else
		{
		    /* Un-pause */
		    lod->lod_Flags &= ~LODF_PAUSE;

		    /* For some reason I couldn't do this with an array!!! */
		    switch (term)
		    {
			case 1:
			    /* Start playing */
			    clock = (LONG) lod->lod_Frame;
			    SetConductorState (lod->lod_Player, CONDSTATE_LOCATE, (ULONG) clock * lod->lod_TicksPerFrame);
			    break;

			case 3:
			    /* Even though the controller gadget sends stop, we use pause. */
			    SetConductorState (lod->lod_Player, CONDSTATE_PAUSED, 0);
			    break;
		    }
		}
	    }
	}
    }
    return (retval);
}

/*****************************************************************************/

/* Set attributes of an object */
ULONG setAttrsMethod (struct ClassBase * cb, Class * cl, Object * o, struct opSet * msg, BOOL init)
{
    struct DTSpecialInfo *si = (struct DTSpecialInfo *) G (o)->SpecialInfo;
    struct localData *lod = INST_DATA (cl, o);
    struct TagItem *tags = msg->ops_AttrList;
    struct TagItem *tstate;
    BOOL notifynom = FALSE;
    struct FrameInfo *fri;
    UBYTE *sample = NULL;
    struct TagItem *tag;
    struct BitMap *obm;
    ULONG refresh;
    ULONG tidata;
    WORD ncolors;
    ULONG modeid;

    if (init)
    {
	/* We need to remap and have controls by default */
	lod->lod_Flags = LODF_REMAP | LODF_CONTROLS;

	/* Set default audio settings */
	lod->lod_Volume = 64;
	lod->lod_Period = 360;

	/* Compute default frame rate */
	lod->lod_FPS = 10;
	lod->lod_Frame = lod->lod_Which = 0;
	lod->lod_TicksPerFrame = (ULONG) RealTimeBase->rtb_Reserved1 / lod->lod_FPS;

	/* Initialize the semaphore */
	InitSemaphore (&lod->lod_FLock);
	InitSemaphore (&lod->lod_DLock);

	/* Initialize the frame list */
	NewList (&lod->lod_FrameList);
	NewList (&lod->lod_DiscardList);

	/* Prepare the hook */
	lod->lod_CB = cb;
    }
    else
    {
	/* Let the superclass handle it first */
	refresh = (ULONG) DoSuperMethodA (cl, o, (Msg) msg);
    }

    /* Get a pointer to the frame information */
    GetAttr (DTA_FrameInfo, o, (ULONG *) & fri);

    /* Track changes */
    ncolors = lod->lod_NumColors;
    modeid = lod->lod_ModeID;
    obm = lod->lod_KeyFrame;

    /* process rest */
    tstate = tags;
    while (tag = NextTagItem (&tstate))
    {
	tidata = tag->ti_Data;
	switch (tag->ti_Tag)
	{
	    case DTA_ControlPanel:
		if (tidata)
		    lod->lod_Flags |= LODF_CONTROLS;
		else
		    lod->lod_Flags &= ~LODF_CONTROLS;
		break;

	    case DTA_Immediate:
		if (tidata)
		    lod->lod_Flags |= LODF_AUTOPLAY;
		else
		    lod->lod_Flags &= ~LODF_AUTOPLAY;
		break;

	    case ADTA_Remap:
		if (tidata)
		    lod->lod_Flags |= LODF_REMAP;
		else
		    lod->lod_Flags &= ~LODF_REMAP;
		break;

	    case ADTA_ModeID:
		lod->lod_ModeID = tidata;
		break;

	    case ADTA_Width:
		lod->lod_Width = tidata;
		lod->lod_BMHD.bmh_Width = lod->lod_BMHD.bmh_PageWidth = (UWORD) lod->lod_Width;
		notifynom = TRUE;
		break;

	    case ADTA_Height:
		lod->lod_Height = lod->lod_BltHeight = tidata;
		lod->lod_BMHD.bmh_Height = lod->lod_BMHD.bmh_PageHeight = (UWORD) lod->lod_Height;
		notifynom = TRUE;
		break;

	    case ADTA_Depth:
		lod->lod_BMHD.bmh_Depth = lod->lod_Depth = (UBYTE) tidata;
		break;

	    case ADTA_Frames:
		lod->lod_NumFrames = tidata;
		lod->lod_Adjust = 10;
		if (tidata < 50)
		    lod->lod_Adjust = (tidata + 4) / 5;
		setattrs (cb, lod->lod_TapeDeck, TDECK_Frames, tidata, TAG_DONE);
		break;

	    case ADTA_KeyFrame:
		lod->lod_KeyFrame = (struct BitMap *) tidata;
		break;

	    case ADTA_NumColors:
		lod->lod_NumColors = (WORD) tidata;
		break;

	    case ADTA_FramesPerSecond:
		if (tidata > 60)
		    tidata = 60;
		lod->lod_FPS = tidata;
		lod->lod_TicksPerFrame = (ULONG) RealTimeBase->rtb_Reserved1 / lod->lod_FPS;
		break;

	    case DTA_TopVert:
		si->si_TopVert = tidata;
		refresh = 1L;
		break;

	    case DTA_VisibleVert:
		si->si_VisVert = tidata;
		break;

	    case DTA_TotalVert:
		si->si_TotVert = tidata;
		break;

	    case DTA_TopHoriz:
		si->si_TopHoriz = (LONG) tidata;
		if (si->si_TopHoriz < 0)
		    si->si_TopHoriz = 0;
		refresh = 1L;
		break;

	    case DTA_VisibleHoriz:
		si->si_VisHoriz = tidata;
		break;

	    case DTA_TotalHoriz:
		si->si_TotHoriz = tidata;
		break;

		/*********/
		/* SOUND */
		/*********/
	    case SDTA_Sample:
		sample = (UBYTE *) tidata;
		break;

	    case SDTA_SampleLength:
		lod->lod_SampleLength = tidata;
		break;

	    case SDTA_Period:
		lod->lod_Period = tidata;
		break;

	    case SDTA_Volume:
		lod->lod_Volume = tidata;
		break;
	}
    }

    if (notifynom)
    {
	ULONG width, height;

	width = lod->lod_Width;
	height = lod->lod_Height;
	if (lod->lod_Flags & LODF_CONTROLS)
	{
#if 0
	    width = MAX (202, lod->lod_Width);
#endif
	    height += 17;
	}

	/* Tell the super class about our attributes */
	setdtattrs (cb, o,
		    DTA_NominalHoriz, width,
		    DTA_NominalVert, height,
		    TAG_DONE);
    }

    /* Create the sound object if it doesn't already exist */
    if ((lod->lod_SndObj == NULL) && sample)
    {
	lod->lod_SndObj = newdtobject (cb, (APTR) "movie.snd",
				       DTA_SourceType, DTST_RAM,
				       DTA_GroupID, GID_SOUND,
				       SDTA_Continuous, TRUE,
				       SDTA_SampleLength, lod->lod_SampleLength,
				       SDTA_Period, lod->lod_Period,
				       SDTA_Volume, lod->lod_Volume,
				       SDTA_Cycles, 1,
				       TAG_DONE);
    }

    if (lod->lod_NumColors != ncolors)
    {
	ULONG size1;
	ULONG size2;
	ULONG size3;
	ULONG size4;
	ULONG size5;
	ULONG size6;
	ULONG size7;
	ULONG msize;

	ncolors = lod->lod_NumColors;

	if (lod->lod_Colors)
	    FreeVec (lod->lod_Colors);

	/* How big is the histogram table */
	lod->lod_HistoSize = sizeof (ULONG) * ncolors;

	/* Compute amount of memory needed for colors */
	size1 = sizeof (struct ColorRegister) * ncolors + 2;	/* lod_Colors */
	size2 = size1 + (sizeof (LONG) * 3 * ncolors) + 2;	/* lod_CRegs */
	size3 = size2 + (sizeof (LONG) * 3 * ncolors * 2) + 2;	/* lod_GRegs */
	size4 = size3 + (sizeof (UBYTE) * ncolors) + 2;		/* lod_ColorTable */
	size5 = size4 + (sizeof (UBYTE) * ncolors) + 2;		/* lod_ColorTable2 */
	size6 = size5 + (sizeof (UBYTE) * ncolors * 2) + 2;	/* lod_Allocated */
	size7 = size6 + lod->lod_HistoSize + 2;			/* lod_Histogram */
	msize = size7 + lod->lod_HistoSize + 2;			/* lod_Histogram2 */

	if (lod->lod_Colors = AllocVec (msize, MEMF_CLEAR))
	{
	    lod->lod_CRegs = MEMORY_N_FOLLOWING (lod->lod_Colors, size1);
	    lod->lod_GRegs = MEMORY_N_FOLLOWING (lod->lod_Colors, size2);
	    lod->lod_ColorTable = MEMORY_N_FOLLOWING (lod->lod_Colors, size3);
	    lod->lod_ColorTable2 = MEMORY_N_FOLLOWING (lod->lod_Colors, size4);
	    lod->lod_Allocated = MEMORY_N_FOLLOWING (lod->lod_Colors, size5);
	    lod->lod_Histogram = MEMORY_N_FOLLOWING (lod->lod_Colors, size6);
	    lod->lod_Histogram2 = MEMORY_N_FOLLOWING (lod->lod_Colors, size7);
	}

	fri->fri_ColorMap = (struct ColorMap *) lod->lod_CRegs;
    }

    if (obm != lod->lod_KeyFrame)
    {
	fri->fri_Dimensions.Width = GetBitMapAttr  (lod->lod_KeyFrame, BMA_WIDTH);
	fri->fri_Dimensions.Height = GetBitMapAttr (lod->lod_KeyFrame, BMA_HEIGHT) + 17;
	fri->fri_Dimensions.Depth = GetBitMapAttr  (lod->lod_KeyFrame, BMA_DEPTH);
    }

    if (modeid != lod->lod_ModeID)
    {
	struct DimensionInfo dim;
	struct DisplayInfo di;
	struct FrameInfo *fri;
	ULONG flags = NULL;

	/* Get a pointer to the frame information */
	GetAttr (DTA_FrameInfo, o, (ULONG *) & fri);

	/* Start out as not available */
	di.NotAvailable = TRUE;

	/* Try to get the mode information */
	if (!GetDisplayInfoData (NULL, (APTR) & di, sizeof (struct DisplayInfo), DTAG_DISP, lod->lod_ModeID) ||
	    di.NotAvailable)
	{
	    /* Set up the flags */
	    if (lod->lod_ModeID & 0x800)
		flags |= DIPF_IS_HAM;
	    else if (lod->lod_ModeID & 0x80)
		flags |= DIPF_IS_EXTRAHALFBRITE;

	    /* Get the mode ID */
	    if ((modeid = bestmodeid (cb,
				      BIDTAG_SourceID, lod->lod_ModeID,
				      BIDTAG_DIPFMustHave, flags,
				      BIDTAG_NominalWidth, lod->lod_BMHD.bmh_PageWidth,
				      BIDTAG_NominalHeight, lod->lod_BMHD.bmh_PageHeight,
				      BIDTAG_DesiredWidth, lod->lod_BMHD.bmh_Width,
				      BIDTAG_DesiredHeight, lod->lod_BMHD.bmh_Height,
				      BIDTAG_Depth, lod->lod_BMHD.bmh_Depth,
				      TAG_DONE)) != INVALID_ID)
	    {
		lod->lod_ModeID = modeid;
		GetDisplayInfoData (NULL, (APTR) & di, sizeof (struct DisplayInfo), DTAG_DISP, lod->lod_ModeID);
	    }
	}

	if (!di.NotAvailable)
	{
	    GetDisplayInfoData (NULL, (APTR) & dim, sizeof (struct DimensionInfo), DTAG_DIMS, lod->lod_ModeID);
	    fri->fri_Dimensions.Depth = MIN (fri->fri_Dimensions.Depth, dim.MaxDepth);
	    fri->fri_PropertyFlags = di.PropertyFlags;
	    fri->fri_Resolution = *(&di.Resolution);
	    fri->fri_RedBits = di.RedBits;
	    fri->fri_GreenBits = di.GreenBits;
	    fri->fri_BlueBits = di.BlueBits;
	}
    }

#if 0
    if (depth != lod->lod_Depth)
    {
	FreeBitMap (lod->lod_BMap1);
	lod->lod_BMap1 = AllocBitMap ((ULONG) lod->lod_Width, (ULONG) lod->lod_Height, (ULONG) lod->lod_Depth,
				      BMF_INTERLEAVED | BMF_DISPLAYABLE, NULL);
	lod->lod_BMap = lod->lod_BMap1;
    }
#endif

    return (refresh);
}

/*****************************************************************************/

ULONG updateMethod (struct ClassBase * cb, Class * cl, Object * o, struct opSet * msg)
{

    if (setAttrsMethod (cb, cl, o, (struct opSet *) msg, FALSE))
    {
	struct RastPort *rp;

	/* Get a pointer to the rastport */
	if (rp = ObtainGIRPort (((struct opSet *) msg)->ops_GInfo))
	{
	    struct gpRender gpr;

	    /* Force a redraw */
	    gpr.MethodID = GM_RENDER;
	    gpr.gpr_GInfo = ((struct opSet *) msg)->ops_GInfo;
	    gpr.gpr_RPort = rp;
	    gpr.gpr_Redraw = GREDRAW_UPDATE;
	    DoMethodA (o, (Msg) & gpr);

	    /* Release the temporary rastport */
	    ReleaseGIRPort (rp);
	}
    }
    return (0);
}

/*****************************************************************************/

/* Inquire attribute of an object */
ULONG getAttrMethod (struct ClassBase * cb, Class * cl, Object * o, struct opGet * msg)
{
    struct localData *lod = INST_DATA (cl, o);

    switch (msg->opg_AttrID)
    {
	/* Method information */
	case DTA_TriggerMethods:
	    *msg->opg_Storage = (ULONG) tm;
	    break;
	case DTA_Methods:
	    *msg->opg_Storage = (ULONG) m;
	    break;

	/* Display information */
	case ADTA_KeyFrame:
	    *msg->opg_Storage = (ULONG) lod->lod_KeyFrame;
	    break;
	case ADTA_ModeID:
	    *msg->opg_Storage = lod->lod_ModeID;
	    break;
	case PDTA_BitMapHeader:
	    *msg->opg_Storage = (ULONG) & lod->lod_BMHD;
	    break;
	case ADTA_Width:
	    *msg->opg_Storage = lod->lod_Width;
	    break;
	case ADTA_Height:
	    *msg->opg_Storage = lod->lod_Height;
	    break;

	/* Color information */
	case ADTA_Depth:
	    *msg->opg_Storage = (ULONG) lod->lod_Depth;
	    break;
	case ADTA_ColorRegisters:
	    *msg->opg_Storage = (ULONG) lod->lod_Colors;
	    break;
	case ADTA_CRegs:
	    *msg->opg_Storage = (ULONG) lod->lod_CRegs;
	    break;
	case ADTA_GRegs:
	    *msg->opg_Storage = (ULONG) lod->lod_GRegs;
	    break;
	case ADTA_ColorTable:
	    *msg->opg_Storage = (ULONG) lod->lod_ColorTable;
	    break;
	case ADTA_ColorTable2:
	    *msg->opg_Storage = (ULONG) lod->lod_ColorTable2;
	    break;
	case ADTA_Allocated:
	    *msg->opg_Storage = (ULONG) lod->lod_Allocated;
	    break;
	case ADTA_NumColors:
	    *msg->opg_Storage = (ULONG) lod->lod_NumColors;
	    break;
	case ADTA_NumAlloc:
	    *msg->opg_Storage = (ULONG) lod->lod_NumAlloc;
	    break;

	/* Number of frames */
	case ADTA_Frames:
	    *msg->opg_Storage = lod->lod_NumFrames;
	    break;
	/* Current frame */
	case ADTA_Frame:
	    *msg->opg_Storage = lod->lod_Frame;
	    break;
	/* Frames per second */
	case ADTA_FramesPerSecond:
	    *msg->opg_Storage = lod->lod_FPS;
	    break;
	/* Amount to change frame when fast forwarding */
	case ADTA_FrameIncrement:
	    *msg->opg_Storage = lod->lod_Adjust;
	    break;

	/* Sound information */
	case ADTA_Sample:
	    *msg->opg_Storage = (ULONG) lod->lod_Sample;
	    break;
	case ADTA_SampleLength:
	    *msg->opg_Storage = lod->lod_SampleLength;
	    break;
	case ADTA_Period:
	    *msg->opg_Storage = lod->lod_Period;
	    break;
	case ADTA_Volume:
	    *msg->opg_Storage = lod->lod_Volume;
	    break;
	case ADTA_Cycles:
	    *msg->opg_Storage = 1;
	    break;

	default:
	    return (DoSuperMethodA (cl, o, (Msg) msg));
    }

    return (1L);
}

/*****************************************************************************/

ULONG frameMethod (struct ClassBase * cb, Class * cl, Object * o, struct dtFrameBox * msg)
{
    struct FrameInfo *fri;

    if (!(msg->dtf_FrameFlags & FRAMEF_SPECIFY))
    {
	/* Tell me about yourself */
	GetAttr (DTA_FrameInfo, o, (ULONG *) & fri);
	CopyMem (fri, msg->dtf_FrameInfo, MIN (msg->dtf_SizeFrameInfo, sizeof (struct FrameInfo)));
    }
    return (1);
}

/*****************************************************************************/

void ASM XLateBitMap (REG (a0) struct BitMap * sbm, REG (a1) struct BitMap * dbm, REG (a2) char *table1, REG (a3) char *table2, REG (d0) ULONG width);

/*****************************************************************************/

/* 6 7 3 */
#define	RED_SCALE	6
#define	GREEN_SCALE	7
#define	BLUE_SCALE	3

/*****************************************************************************/

#define	SQ(x)		((x)*(x))
#define	AVGC(i1,i2,c)	((lod->lod_GRegs[((i1) * 3) + (c)]>>1)+(lod->lod_GRegs[((i2) * 3) + (c)]>>1))

/*****************************************************************************/

static ULONG color_error (ULONG r1, ULONG r2, ULONG g1, ULONG g2, ULONG b1, ULONG b2)
{

    r1 >>= 24;
    g1 >>= 24;
    b1 >>= 24;
    r2 >>= 24;
    g2 >>= 24;
    b2 >>= 24;
    return (SQ (r1 - r2) + SQ (g1 - g2) + SQ (b1 - b2));
}

/*****************************************************************************/

ULONG layoutMethod (struct ClassBase * cb, Class * cl, Object * o, struct gpLayout * msg)
{
    struct DTSpecialInfo *si = (struct DTSpecialInfo *) G (o)->SpecialInfo;
    struct localData *lod = INST_DATA (cl, o);
    ULONG top, left, width, height;
    LONG testr, testg, testb;
    struct TagItem tags[2];
    struct Screen *scr;
    struct IBox *b;
    STRPTR title;
    ULONG retval;
    ULONG odepth;
    ULONG depth;
    BOOL remap;
    LONG i, j;
    LONG err1;
    LONG err2;
    LONG got;

    /* Set the layout flag */
    si->si_Flags |= DTSIF_LAYOUT;

    /* Let the superclass layout */
    retval = (ULONG) DoSuperMethodA (cl, o, (Msg) msg);

    /* Show that we are attached to the window */
    lod->lod_Flags |= LODF_ATTACHED;

    GetAttr (DTA_Domain,  o, (ULONG *) &b);
    GetAttr (DTA_ObjName, o, (ULONG *) &title);

    /* Time to do minor conversion */
    si->si_VisVert = b->Height;
    si->si_TotVert = lod->lod_BMHD.bmh_Height;

    si->si_VisHoriz = b->Width;
    si->si_TotHoriz = lod->lod_BMHD.bmh_Width;

    remap = (lod->lod_Flags & LODF_REMAP) ? TRUE : FALSE;

    if (msg->gpl_Initial)
    {
	/* Start the preload process */
	Signal (lod->lod_LoadProc, SIGBREAKF_CTRL_E);

	lod->lod_Window = msg->gpl_GInfo->gi_Window;
	scr = lod->lod_Window->WScreen;

	/* Time to do major conversion */
	lod->lod_ColorMap = scr->ViewPort.ColorMap;
	depth = MAX (scr->RastPort.BitMap->Depth, (ULONG) lod->lod_BMHD.bmh_Depth);

	/* Free the extra information */
	freeExtraInfo (cb, lod);

	/* Create the bitmap */
	FreeBitMap (lod->lod_BMap1);
#if 0
	lod->lod_BMap1 = AllocBitMap ((ULONG) lod->lod_Width, (ULONG) lod->lod_Height, (ULONG)depth,
				      BMF_CLEAR, &scr->BitMap);
#else
	lod->lod_BMap1 = AllocBitMap ((ULONG) lod->lod_Width, (ULONG) lod->lod_Height, (ULONG) scr->BitMap.Depth,
				      BMF_CLEAR, &scr->BitMap);
#endif
	lod->lod_BMap = lod->lod_BMap1;

	DB (kprintf ("lod_BMap=%08lx : %08x\n", lod->lod_BMap, GetBitMapAttr (lod->lod_KeyFrame, BMA_FLAGS)));

	/* We only do remap if we have color information */
	if (remap && lod->lod_Colors)
	{
	    /* Set up tags */
	    tags[0].ti_Tag = OBP_Precision;
	    tags[0].ti_Data = PRECISION_ICON;
	    tags[1].ti_Tag = TAG_DONE;

	    /* See if we really need to remap */
	    remap = FALSE;
	    for (i = 0; i < lod->lod_NumColors; i++)
	    {
		got = ObtainBestPenA (lod->lod_ColorMap,
				      lod->lod_CRegs[i * 3 + 0],
				      lod->lod_CRegs[i * 3 + 1],
				      lod->lod_CRegs[i * 3 + 2], tags);
		GetRGB32 (lod->lod_ColorMap, got, 1, (lod->lod_GRegs + (lod->lod_NumAlloc * 3)));
		lod->lod_Allocated[lod->lod_NumAlloc++] = got;
		if (got != i)
		    remap = TRUE;
		lod->lod_ColorTable[i] = lod->lod_ColorTable2[i] = got;
	    }

	    if (remap)
	    {
		for (i = 0; i < lod->lod_NumColors; i++)
		{
		    err1 = color_error (lod->lod_CRegs[i * 3 + 0], lod->lod_GRegs[i * 3 + 0],
					lod->lod_CRegs[i * 3 + 1], lod->lod_GRegs[i * 3 + 1],
					lod->lod_CRegs[i * 3 + 2], lod->lod_GRegs[i * 3 + 2]);

		    if (err1 > 2000)
		    {
			for (j = 0; j < lod->lod_NumAlloc; j++)
			{
			    testr = AVGC (i, j, 0);
			    testg = AVGC (i, j, 1);
			    testb = AVGC (i, j, 2);

			    err2 = color_error (lod->lod_CRegs[i * 3 + 0], testr,
						lod->lod_CRegs[i * 3 + 1], testg,
						lod->lod_CRegs[i * 3 + 2], testb);

			    if (err2 < err1)
			    {
				err1 = err2;
				lod->lod_ColorTable2[i] = lod->lod_Allocated[j];
			    }
			}
		    }
		}

		odepth = lod->lod_KeyFrame->Depth;
		lod->lod_KeyFrame->Depth = MIN (odepth, 8);
		XLateBitMap (lod->lod_KeyFrame, lod->lod_BMap, lod->lod_ColorTable, lod->lod_ColorTable2, lod->lod_Width);
		lod->lod_KeyFrame->Depth = odepth;
	    }
	}

	if (!remap)
	{
	    BltBitMap (lod->lod_KeyFrame, 0, 0,
		       lod->lod_BMap, 0, 0, lod->lod_BMHD.bmh_Width, lod->lod_BMHD.bmh_Height,
		       0xC0, 0xFF, NULL);
	}

	/* Start playing if we are autoplay */
	if (lod->lod_Flags & LODF_AUTOPLAY)
	{
	    /* Locate the frame and start playing */
	    SetConductorState (lod->lod_Player, CONDSTATE_LOCATE, lod->lod_ClockTime);
	}
    }

    /* Come up with the size */
    width  = lod->lod_Width;
    height = lod->lod_Height;
    if (lod->lod_Flags & LODF_CONTROLS)
    {
	ULONG gwidth;

	/* Adjust the tape deck */
	left = (ULONG) b->Left;
#if 1
	gwidth = (ULONG) MIN (lod->lod_Width, b->Width) - 1;
#else
	gwidth = (ULONG) MIN (202, b->Width);
#endif
	j = 17;
	top = (ULONG) (b->Top + lod->lod_Height + 1);
	if (top + 14 > b->Top + b->Height - 1)
	{
	    top = b->Top + b->Height - 15;
	    j = 0;
	}
	height += j;

	/* Remember the positioning */
	lod->lod_TapeX = left - b->Left;
	lod->lod_TapeY = top - b->Top;

	/* TEST TEST TEST */
	lod->lod_BltHeight = lod->lod_TapeY - 1;

	DB (kprintf ("GA_Width, %ld : lod_Width=%ld, b->Width=%ld\n", gwidth, (LONG)lod->lod_Width, (LONG)b->Width));
	SetAttrs (lod->lod_TapeDeck, GA_Top, top, GA_Left, left, GA_Width, gwidth, TAG_DONE);
	DoMethodA ((Object *) lod->lod_TapeDeck, (Msg) msg);
    }

    /* Clear the layout flag */
    si->si_Flags &= ~DTSIF_LAYOUT;

    /* Tell everyone about our changes */
    notifyAttrChanges (cb, o, msg->gpl_GInfo, NULL,
		       GA_ID, G (o)->GadgetID,

		       DTA_VisibleVert, (si->si_VisVert = (LONG) b->Height),
		       DTA_TotalVert, (si->si_TotVert = (LONG) height),

		       DTA_VisibleHoriz, (si->si_VisHoriz = (LONG) b->Width),
		       DTA_TotalHoriz, (si->si_TotHoriz = (LONG) width),

		       DTA_Busy, FALSE,
		       DTA_Title, (ULONG) title,
		       DTA_Sync, TRUE,
		       TAG_DONE);

    return (retval);
}

/*****************************************************************************/

ULONG renderMethod (struct ClassBase * cb, Class * cl, Object * o, struct gpRender * msg)
{
    struct DTSpecialInfo *si = (struct DTSpecialInfo *) G (o)->SpecialInfo;
    struct localData *lod = INST_DATA (cl, o);
    struct IBox *domain;
    LONG width, height;

    /* Are we attached to a window? */
    if (lod->lod_Flags & LODF_ATTACHED)
    {
	GetAttr (DTA_Domain, o, (ULONG *) & domain);
	width = (ULONG) MIN (lod->lod_Width, domain->Width);
	height = (ULONG) MIN (lod->lod_BltHeight, domain->Height);

	/* If this is a major redraw, then erase the old image */
	if ((msg->gpr_Redraw == GREDRAW_REDRAW) &&
	    (si->si_OTopVert != si->si_TopVert) && (si->si_OTopVert != si->si_TopVert))
	{
	    EraseRect (msg->gpr_RPort,
		       domain->Left, domain->Top,
		       domain->Left + domain->Width - 1,
		       domain->Top + domain->Height - 1);
	}

	/* Make sure we have a bitmap to blit */
	if (lod->lod_BMap)
	{
	    BltBitMapRastPort (lod->lod_BMap, (ULONG) si->si_TopHoriz, (ULONG) si->si_TopVert,
			       msg->gpr_RPort, domain->Left, domain->Top, width, height, 0xC0);
	}

	if (lod->lod_Flags & LODF_CONTROLS)
	{
	    /* Draw the tape deck */
	    msg->gpr_Redraw = GREDRAW_REDRAW;
	    DoMethodA ((Object *) lod->lod_TapeDeck, (Msg) msg);
	}

	si->si_OTopVert = si->si_TopVert;
	si->si_OTopHoriz = si->si_TopHoriz;
    }

    return (1L);
}

/*****************************************************************************/

ULONG newMethod (struct ClassBase * cb, Class * cl, Object * o, struct opSet * ops)
{
    struct localData *lod;
    ULONG retval;

    /* Let the superclass create the object */
    if (retval = DoSuperMethodA (cl, o, (Msg) ops))
    {
	/* Get a pointer to the object data */
	lod = INST_DATA (cl, (Object *) retval);

	/* Set the attributes */
	setAttrsMethod (cb, cl, (Object *) retval, ops, TRUE);

	    /* Create the control gadget */
	    if (lod->lod_TapeDeck = NewObject (NULL, "tapedeck.gadget",
					       GA_Width, 202,
					       GA_Height, 16,
					       GA_RelVerify, TRUE,
					       TAG_DONE))
	    {
		/* Create the main process for this object */
		if (lod->lod_DispProc = createnewproc (cb,
						       NP_Output, Output (),
						       NP_CloseOutput, FALSE,
						       NP_Input, Input (),
						       NP_CloseInput, FALSE,
						       NP_Name, DISP_PROC_NAME,
						       NP_StackSize, 8096L,
						       NP_Entry, DispHandler,
						       NP_Priority, 0L,
						       TAG_DONE))
		{
		    /* Initialize the message */
		    lod->lod_DispMsg.sm_Message.mn_Node.ln_Type = NT_MESSAGE;
		    lod->lod_DispMsg.sm_Message.mn_Length = sizeof (struct StartupMsg);
		    lod->lod_DispMsg.sm_CB = cb;
		    lod->lod_DispMsg.sm_Class = cl;
		    lod->lod_DispMsg.sm_Object = (Object *) retval;
		    lod->lod_LoadMsg = *(&lod->lod_DispMsg);

		    /* Show that we are operational */
		    lod->lod_Flags |= LODF_DGOING;

		    /* Send the information to the process */
		    PutMsg (&(lod->lod_DispProc->pr_MsgPort), (struct Message *) & lod->lod_DispMsg);

		    /* Create a process to asynchronously load data */
		    if (lod->lod_LoadProc = createnewproc (cb,
							   NP_Output, Output (),
							   NP_CloseOutput, FALSE,
							   NP_Input, Input (),
							   NP_CloseInput, FALSE,
							   NP_Name, LOAD_PROC_NAME,
							   NP_StackSize, 8096L,
							   NP_Entry, LoadHandler,
							   NP_Priority, -1L,
							   TAG_DONE))
		    {
			/* Show that we are operational */
			lod->lod_Flags |= LODF_LGOING;

			/* Send the information to the process */
			PutMsg (&(lod->lod_LoadProc->pr_MsgPort), (struct Message *) & lod->lod_LoadMsg);

			/* Wait until the new process starts */
			while (!(lod->lod_Flags & LODF_LRUNNING))
			    Delay (2);

			return (retval);
		    }
		}
	    }

	SetIoErr (ERROR_NO_FREE_STORE);
	CoerceMethod (cl, (Object *) retval, OM_DISPOSE);
	return (NULL);
    }
}

/*****************************************************************************/

ULONG startMethod (struct ClassBase * cb, Class * cl, Object * o, struct adtStart * asa)
{
    struct localData *lod = INST_DATA (cl, o);
    struct dtTrigger dtt;
    struct adtFrame *alf;
    struct FrameNode *fn;
    ULONG cnt;

    /* Start the load process */
    Signal ((struct Task *) lod->lod_LoadProc, SIGBREAKF_CTRL_D);

    /* Get a pointer to the first sample so that we can start double-buffering the sound. */
    cnt = 0;
    fn = NULL;
    while (fn == NULL)
    {
	ObtainSemaphore (&lod->lod_FLock);
	if (lod->lod_FrameList.lh_TailPred != (struct Node *) & lod->lod_FrameList)
	    fn = (struct FrameNode *) lod->lod_FrameList.lh_Head;
	ReleaseSemaphore (&lod->lod_FLock);

	/* If we don't have a frame yet, then give it a chance */
	if (fn == NULL)
	{
	    /* See if we are in a loop waiting for it to start! */
	    cnt++;
	    if (cnt > 10)
	    {
		/* Start the load process */
		Signal ((struct Task *) lod->lod_LoadProc, SIGBREAKF_CTRL_D);
		cnt = 0;
	    }

	    Delay (5);
	}
    }

    /* Set the first sample */
    alf = &fn->fn_Frame;
    if (lod->lod_SndObj)
	setattrs (cb, (struct Gadget *) lod->lod_SndObj, SDTA_Sample, alf->alf_Sample, TAG_DONE);

    /* Set the status flags */
    lod->lod_Flags &= ~LODF_ALL;
    lod->lod_Flags |= LODF_PLAYING;

    /* Start the timing */
    lod->lod_Ticks = 0;
    lod->lod_Which = lod->lod_Frame = asa->asa_Frame;
    lod->lod_Player->pl_MetricTime = lod->lod_Frame * lod->lod_TicksPerFrame;
    lod->lod_ClockTime = lod->lod_Player->pl_MetricTime;
    SetConductorState (lod->lod_Player, CONDSTATE_RUNNING, lod->lod_ClockTime);

    /* Start the sound */
    if (lod->lod_SndObj)
    {
	dtt.MethodID = DTM_TRIGGER;
	dtt.dtt_Function = STM_PLAY;
	DoMethodA (lod->lod_SndObj, (Msg) & dtt);
    }

    /* Set the start control */
    SetTapeDeckAttrs (cb, lod, TDECK_Mode, BUT_PLAY, TAG_DONE);

    return (1);
}

/*****************************************************************************/

ULONG pauseMethod (struct ClassBase * cb, Class * cl, Object * o, Msg msg)
{
    struct localData *lod = INST_DATA (cl, o);

    /* Only pause if we are playing */
    if (lod->lod_Flags & LODF_PLAYING)
    {
	/* Stop	the timing */
	SetConductorState (lod->lod_Player, CONDSTATE_PAUSED, lod->lod_ClockTime);

	/* Stop	the sounds */
	if (lod->lod_SndObj)
	{
	    setdtattrs (cb, lod->lod_SndObj, SDTA_Sample, NULL, TAG_DONE);
	}

	/* Stop	the control */
	if (!(lod->lod_Flags & LODF_PAUSE))
	    SetTapeDeckAttrs (cb, lod, TDECK_Mode, BUT_STOP, TAG_DONE);
    }

    /* Set the status flags */
    lod->lod_Flags &= ~LODF_ALL;
    lod->lod_Flags |= LODF_PAUSE;

    return (1);
}

/*****************************************************************************/

ULONG stopMethod (struct ClassBase * cb, Class * cl, Object * o, Msg msg)
{
    struct localData *lod = INST_DATA (cl, o);

    /* Stop the timing */
    SetConductorState (lod->lod_Player, CONDSTATE_STOPPED, lod->lod_ClockTime);

    /* Stop the sounds */
    if (lod->lod_SndObj)
	setdtattrs (cb, lod->lod_SndObj, SDTA_Sample, NULL, TAG_DONE);

    /* Stop the control */
    if (!(lod->lod_Flags & LODF_PAUSE))
	SetTapeDeckAttrs (cb, lod, TDECK_Mode, BUT_STOP, TAG_DONE);

    /* Set the status flags */
    lod->lod_Flags &= ~LODF_ALL;
    lod->lod_Flags |= LODF_STOPPED;

    return (1);
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

struct ColorMap *GetObjectColorMap (struct ClassBase * cb, Class * cl, Object * o)
{
    struct localData *lod = INST_DATA (cl, o);
    struct ColorMap *cm = NULL;
    ULONG i, r, g, b;

    if (lod->lod_Colors)
    {
	/* Get a color map */
	if (cm = GetColorMap (lod->lod_NumColors))
	{
	    /* Set the colors */
	    for (i = 0; i < lod->lod_NumColors; i++)
	    {
		r = lod->lod_CRegs[i * 3 + 0];
		g = lod->lod_CRegs[i * 3 + 1];
		b = lod->lod_CRegs[i * 3 + 2];
		SetRGB32CM (cm, i, r, g, b);
	    }
	}
    }
    return (cm);
}

/*****************************************************************************/

ULONG copyMethod (struct ClassBase * cb, Class * cl, Object * o, struct dtGeneral * msg)
{
    struct localData *lod = INST_DATA (cl, o);
    struct IFFHandle *iff;
    BOOL success = FALSE;

    if (iff = AllocIFF ())
    {
	if (iff->iff_Stream = (ULONG) OpenClipboard (0L))
	{
	    InitIFFasClip (iff);
	    success = writeObject (cb, iff, o, lod);
	    CloseClipboard ((struct ClipboardHandle *) iff->iff_Stream);
	}
	FreeIFF (iff);
    }
    return ((ULONG) success);
}

/*****************************************************************************/

ULONG writeMethod (struct ClassBase * cb, Class * cl, Object * o, struct dtWrite * msg)
{
    struct localData *lod = INST_DATA (cl, o);
    struct IFFHandle *iff;
    BOOL success = FALSE;

    if (iff = AllocIFF ())
    {
	if (iff->iff_Stream = msg->dtw_FileHandle)
	{
	    InitIFFasDOS (iff);
	    success = writeObject (cb, iff, o, lod);
	}
	FreeIFF (iff);
    }
    return ((ULONG) success);
}

/*****************************************************************************/

ULONG printMethod (struct ClassBase * cb, Class * cl, Object * o, struct dtPrint * msg)
{
    struct localData *lod = INST_DATA (cl, o);
    struct GadgetInfo *gi = msg->dtp_GInfo;
    struct RastPort *prp;
    union printerIO *pio;
    UWORD width, height;
    struct ColorMap *cm;
    struct RastPort rp;
    ULONG retval = 0L;
    BOOL going = TRUE;
    struct IBox *sel;
    UWORD left, top;
    UWORD iospecial;
    LONG destcols;
    LONG destrows;
    ULONG sigr;
    ULONG psig;
    ULONG tmpl;

    if (pio = msg->dtp_PIO)
    {
	/* What bit are we going to wait on? */
	psig = 1L << pio->ios.io_Message.mn_ReplyPort->mp_SigBit;

	if (GetAttr (DTA_SelectDomain, o, (ULONG *) & sel) && sel)
	{
	    left   = sel->Left;
	    top    = sel->Top;
	    width  = sel->Width;
	    height = sel->Height;
	}
	else
	{
	    left   = top    = 0;
	    width  = lod->lod_BMHD.bmh_Width;
	    height = lod->lod_BMHD.bmh_Height;
	}

	/* Get a color map */
	if (cm = GetObjectColorMap (cb, cl, o))
	{
	    /* Prepare the RastPort */
	    InitRastPort (&rp);
	    rp.BitMap = lod->lod_BMap;	// lod->lod_SourceBMap;
	    prp = (struct RastPort *) GetTagData (DTA_RastPort, (ULONG)&rp, msg->dtp_AttrList);

	    destcols = destrows = 0;

	    /* Compute the flags & size */
	    if ((lod->lod_BMHD.bmh_Width == width) && (lod->lod_BMHD.bmh_Height == height))
	    {
		iospecial = SPECIAL_FULLCOLS | SPECIAL_ASPECT;
	    }
	    else
	    {
		iospecial = SPECIAL_FRACCOLS | SPECIAL_ASPECT;
		tmpl = lod->lod_BMHD.bmh_Width;
		tmpl = tmpl << 16;
		destcols = (tmpl / width) << 16;
	    }

	    destcols  = (LONG)  GetTagData (DTA_DestCols, (ULONG)destcols,  msg->dtp_AttrList);
	    destrows  = (LONG)  GetTagData (DTA_DestRows, (ULONG)destrows,  msg->dtp_AttrList);
	    iospecial = (UWORD) GetTagData (DTA_Special,  (ULONG)iospecial, msg->dtp_AttrList);

	    /* Prepare to dump the RastPort */
	    pio->iodrp.io_Command = PRD_DUMPRPORT;
	    pio->iodrp.io_RastPort = prp;
	    pio->iodrp.io_ColorMap = cm;
	    pio->iodrp.io_Modes = lod->lod_ModeID;
	    pio->iodrp.io_SrcX = left;
	    pio->iodrp.io_SrcY = top;
	    pio->iodrp.io_SrcWidth = width;
	    pio->iodrp.io_SrcHeight = height;
	    pio->iodrp.io_DestCols = destcols;
	    pio->iodrp.io_DestRows = destrows;
	    pio->iodrp.io_Special = iospecial;
	    SendIO ((struct IORequest *) pio);

	    while (going)
	    {
		/* Wait for something to happen */
		sigr = Wait (psig | SIGBREAKF_CTRL_C);

		/* Did they try to abort us? */
		if (sigr & SIGBREAKF_CTRL_C)
		{
		    /* Abort the IO */
		    AbortIO ((struct IORequest *) pio);
		    WaitIO ((struct IORequest *) pio);

		    /* Reset the printer (otherwise they get garbage the next time they print) */
		    pio->ios.io_Command = CMD_WRITE;
		    pio->ios.io_Data = "\033c";
		    pio->ios.io_Length = -1L;
		    DoIO ((struct IORequest *) pio);
		    going = FALSE;
		}

		/* Is the print complete? */
		if (sigr & psig)
		{
		    while (GetMsg (pio->ios.io_Message.mn_ReplyPort)) ;
		    going = FALSE;
		}
	    }

	    /* Return the return value from the print request */
	    if ((retval = (ULONG) pio->ios.io_Error) == 0)
	    {
		DoMethod (o, DTM_CLEARSELECTED, gi);
	    }

	    /* Free the temporary color map */
	    FreeColorMap (cm);
	}
    }

    return retval;
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

ULONG ASM Dispatch (REG (a0) Class * cl, REG (a2) Object * o, REG (a1) Msg msg)
{
    struct ClassBase *cb = (struct ClassBase *) cl->cl_UserData;
    struct localData *lod = INST_DATA (cl, o);
    ULONG retval;

    switch (msg->MethodID)
    {
	case OM_NEW:
	    return (newMethod (cb, cl, o, (struct opSet *) msg));

	case OM_GET:
	    return (getAttrMethod (cb, cl, o, (struct opGet *) msg));

	case OM_UPDATE:
	case OM_SET:
	    return (updateMethod (cb, cl, o, (struct opSet *) msg));

	case DTM_FRAMEBOX:
	    return (frameMethod (cb, cl, o, (struct dtFrameBox *) msg));

	case GM_LAYOUT:
	    return (layoutMethod (cb, cl, o, (struct gpLayout *) msg));

	case GM_HITTEST:
	    return (GMR_GADGETHIT);
	    break;

	case GM_GOACTIVE:
	case GM_HANDLEINPUT:
	    return (handleMethod (cb, cl, o, (struct gpInput *) msg));

	case GM_GOINACTIVE:
	    if (lod->lod_Active)
		DoMethodA ((Object *) lod->lod_Active, msg);
	    lod->lod_Active = NULL;
	    return (DoSuperMethodA (cl, o, msg));

	case GM_RENDER:
	    return (renderMethod (cb, cl, o, (struct gpRender *) msg));

	case DTM_TRIGGER:
	    retval = (ULONG) DoSuperMethodA (cl, o, msg);
	    {
		struct dtTrigger *dtt = (struct dtTrigger *) msg;
		struct TriggerMsg *tm;

		if (tm = AllocVec (sizeof (struct TriggerMsg), MEMF_CLEAR))
		{
		    tm->tm_Function = dtt->dtt_Function;
		    tm->tm_Data = dtt->dtt_Data;
		    PutMsg (lod->lod_MP, (struct Message *) tm);
		}
	    }
	    break;

	case ADTM_START:
	    return (startMethod (cb, cl, o, (struct adtStart *) msg));

	case ADTM_PAUSE:
	    return (pauseMethod (cb, cl, o, msg));

	case ADTM_STOP:
	    return (stopMethod (cb, cl, o, msg));

	case DTM_PRINT:
	    retval = printMethod (cb, cl, o, (struct dtPrint *) msg);
	    break;

	case DTM_COPY:
	    retval = copyMethod (cb, cl, o, (struct dtGeneral *) msg);
	    break;

	case DTM_WRITE:
	    retval = writeMethod (cb, cl, o, (struct dtWrite *) msg);
	    break;

	case DTM_REMOVEDTOBJECT:
	    /* Force it to stop playing */
	    DoMethod (o, ADTM_STOP);

	    /* Tell our load process to stop loading */
	    if (lod->lod_LoadProc)
		Signal ((struct Task *) lod->lod_LoadProc, SIGBREAKF_CTRL_F);

	    /* Wait until they stop loading */
	    while (lod->lod_Flags & LODF_LOADING)
		Delay (2);

	    /* Tell the display process to detach */
	    if (lod->lod_DispProc)
		Signal ((struct Task *) lod->lod_DispProc, SIGBREAKF_CTRL_E);

	    /* Wait until they detach */
	    while (lod->lod_Flags & LODF_ATTACHED)
		Delay (2);

	    Delay (2);
	    break;

	case OM_DISPOSE:
	    /* Do we have a control process running */
	    if ((lod->lod_Flags & LODF_DRUNNING) && lod->lod_DispProc)
	    {
		/* Tell the control process to go away */
		Signal ((struct Task *) lod->lod_DispProc, SIGBREAKF_CTRL_C);
		Signal ((struct Task *) lod->lod_LoadProc, SIGBREAKF_CTRL_C);

		/* Wait until they go away */
		while ((lod->lod_Flags & LODF_DRUNNING) || (lod->lod_Flags & LODF_LRUNNING))
		    Delay (2);
		Delay (2);
	    }

	    freeExtraInfo (cb, lod);
	    FreeVec (lod->lod_Colors);

	    if (lod->lod_TapeDeck)
		DisposeObject (lod->lod_TapeDeck);

	    WaitBlit ();
	    FreeBitMap (lod->lod_BMap1);

	    /* Get rid of the sound object */
	    if (lod->lod_SndObj)
		DisposeDTObject (lod->lod_SndObj);
	    lod->lod_SndObj = NULL;

	    /* Let the superclass handle everything else */
	default:
	    return (DoSuperMethodA (cl, o, msg));
    }

    return (retval);
}
