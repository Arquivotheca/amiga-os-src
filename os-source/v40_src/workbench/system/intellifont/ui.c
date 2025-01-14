/*
**	$Id: ui.c,v 38.6 92/03/25 15:52:11 davidj Exp $
**
**	Fountain/ui.c -- user interface management
**
**	(C) Copyright 1990 Robert R. Burns
**	    All Rights Reserved
*/

#include  "fountain.h"

extern struct Library *SysBase;
extern struct Library *GadToolsBase;
extern struct Library *GfxBase;
extern struct Library *IntuitionBase;

extern char SDirName[];
extern struct FontsEntry *CurrDDir;
extern struct FaceDisplay *CurrFaceDisplay;
extern WORD CurrHeight;

extern char *GDDirNames[];

extern BOOL ValidateOnly;

BOOL ModifyActive;


struct TextAttr T8TA = {
    FONT_TOPAZ, 8, 0, 0
};
struct TextFont *Topaz8 = 0;
struct Screen *LockedScreen = 0;
struct Screen *Screen = 0;
void *VisualInfo = 0;
struct DrawInfo *DrawInfo = 0;
UWORD TopOrigin, InsideHeight, InsideWidth;
UWORD ZoomSize[4] = {
    0, 0,			/* origin */
    200, 0			/* width, height (filled in later) */
};

#if 0
struct NewWindow NW = {         /* 1.3 compatable part of window definition */
    0, 0,			/* LeftEdge, TopEdge */
    INSIDEWIDTH, 0,		/* Width, Height (filled in later) */
    -1, -1,			/* DetailPen, BlockPen */
    CLOSEWINDOW|REFRESHWINDOW|	/* IDCMPFlags */
    RAWKEY|VANILLAKEY|BUTTONIDCMP|LISTVIEWIDCMP|MXIDCMP|PALETTEIDCMP,
    ACTIVATE|SMART_REFRESH|	/* Flags */
    WINDOWCLOSE|WINDOWDEPTH|WINDOWDRAG,
    0, 0,			/* FirstGadget, CheckMark */
    0,				/* Title */
    0, 0,			/* Screen, BitMap */
    1, 1,			/* MinWidth, MinHeight */
				/*   (small enough so zoom works) */
    0, 0,			/* MaxWidth, MaxHeight */
				/*   (same as initial dimension) */
    WBENCHSCREEN		/* Type */
};
#endif

#define	IDCMP_FLAGS		CLOSEWINDOW|REFRESHWINDOW|RAWKEY|VANILLAKEY|BUTTONIDCMP|LISTVIEWIDCMP|MXIDCMP|PALETTEIDCMP
#define	WA_FLAGS		ACTIVATE|SMART_REFRESH|WINDOWCLOSE|WINDOWDEPTH|WINDOWDRAG

UWORD *ClockPtr[9] = { 0 };
UWORD ClockPtrActive = 0;
struct Window *Window = 0;
struct RastPort *RastPort = 0;
struct Gadget *GadgetList = 0;

UWORD DrawerVectors[] = {
    /* file folder */
    15,5, 15,10, 4,10, 4,4, 8,4, 10,2, 13,2, 15,4,
    15,5, 14,6, 10,6, 9,5, 5,5, 5,9
};

struct Border DrawerImage = {
    0, 0, 1, 0, JAM1, 14, DrawerVectors, 0
};

struct Gadget GSDirReq = {
    0,				/* NextGadget */
    16, 0,			/* LeftEdge, TopEdge (filled in later) */
    20, 14,			/* Width, Height */
    GADGHCOMP, RELVERIFY,	/* Flags, Activation */
    BOOLGADGET, &DrawerImage,	/* GadgetType, GadgetRender */
    0, 0, 0, 0, G_SDIRREQ, 0	/* SelectRender, GadgetText,,, GadgetID, UserData */
};

struct Gadget GDDirReq = {
    0,				/* NextGadget */
    314, 0,			/* LeftEdge, TopEdge (filled in later) */
    20, 14,			/* Width, Height */
    GADGHCOMP, RELVERIFY,	/* Flags, Activation */
    BOOLGADGET, &DrawerImage,	/* GadgetType, GadgetRender */
    0, 0, 0, 0, G_DDIRREQ, 0	/* SelectRender, GadgetText,,, GadgetID, UserData */
};

struct Image HelpImage = {
    0, 0,			/* LeftEdge, TopEdge */
    INSIDEWIDTH-4, 0,		/* Width, Height (filled in later) */
    0,				/* Depth (filled in later) */
    0,				/* ImageData (filled in later) */
    0xff, 0, 0			/* PlanePick, PlaneOnOff, NextImage */
};

struct Gadget GHelp = {
    0,				/* NextGadget */
    0, 0,			/* LeftEdge, TopEdge (filled in later) */
    INSIDEWIDTH-4, 0,		/* Width, Height (filled in later) */
    GADGHNONE|GADGIMAGE,	/* Flags */
    GADGIMMEDIATE, BOOLGADGET,	/* Activation, GadgetType */
    &HelpImage, 0,		/* GadgetRender, SelectRender */
    0, 0, 0, G_HELP, 0		/* GadgetText,,, GadgetID, */
};

#define  CLOCKWORDS	((16*2)+6)
UWORD ClockPtrSrc[CLOCKWORDS*9] = {
    0x0000, 0x0000,	/* standard wait */
    0x0400, 0x07C0,
    0x0000, 0x07C0,
    0x0100, 0x0380,
    0x0000, 0x07E0,
    0x07C0, 0x1FF8,
    0x1FF0, 0x3FEC,
    0x3FF8, 0x7FDE,
    0x3FF8, 0x7FBE,
    0x7FFC, 0xFF7F,
    0x7EFC, 0xFFFF,
    0x7FFC, 0xFFFF,
    0x3FF8, 0x7FFE,
    0x3FF8, 0x7FFE,
    0x1FF0, 0x3FFC,
    0x07C0, 0x1FF8,
    0x0000, 0x07F0,
    0x0000, 0x0000,
    0x0000, 0x0000,

    0x0000, 0x0000,	/* wait 1/8 */
    0x0400, 0x07C0,
    0x0000, 0x07C0,
    0x0100, 0x0380,
    0x0000, 0x07E0,
    0x0600, 0x1FF8,
    0x1E10, 0x3FEC,
    0x3E38, 0x7FDE,
    0x3E78, 0x7FBE,
    0x7EFC, 0xFF7F,
    0x7EFC, 0xFFFF,
    0x7FFC, 0xFFFF,
    0x3FF8, 0x7FFE,
    0x3FF8, 0x7FFE,
    0x1FF0, 0x3FFC,
    0x07C0, 0x1FF8,
    0x0000, 0x07F0,
    0x0000, 0x0000,
    0x0000, 0x0000,

    0x0000, 0x0000,	/* wait 2/8 */
    0x0400, 0x07C0,
    0x0000, 0x07C0,
    0x0100, 0x0380,
    0x0000, 0x07E0,
    0x0600, 0x1FF8,
    0x1E00, 0x3FFC,
    0x3E00, 0x7FFE,
    0x3E00, 0x7FFE,
    0x7E00, 0xFFFF,
    0x7EFC, 0xFF03,
    0x7FFC, 0xFFFF,
    0x3FF8, 0x7FFE,
    0x3FF8, 0x7FFE,
    0x1FF0, 0x3FFC,
    0x07C0, 0x1FF8,
    0x0000, 0x07F0,
    0x0000, 0x0000,
    0x0000, 0x0000,

    0x0000, 0x0000,	/* wait 3/8 */
    0x0400, 0x07C0,
    0x0000, 0x07C0,
    0x0100, 0x0380,
    0x0000, 0x07E0,
    0x0600, 0x1FF8,
    0x1E00, 0x3FFC,
    0x3E00, 0x7FFE,
    0x3E00, 0x7FFE,
    0x7E00, 0xFFFF,
    0x7E00, 0xFFFF,
    0x7F80, 0xFF7F,
    0x3FC0, 0x7FBE,
    0x3FE0, 0x7FDE,
    0x1FF0, 0x3FEC,
    0x07C0, 0x1FF8,
    0x0000, 0x07F0,
    0x0000, 0x0000,
    0x0000, 0x0000,

    0x0000, 0x0000,	/* wait 4/8 */
    0x0400, 0x07C0,
    0x0000, 0x07C0,
    0x0100, 0x0380,
    0x0000, 0x07E0,
    0x0600, 0x1FF8,
    0x1E00, 0x3FFC,
    0x3E00, 0x7FFE,
    0x3E00, 0x7FFE,
    0x7E00, 0xFFFF,
    0x7E00, 0xFFFF,
    0x7F00, 0xFEFF,
    0x3F00, 0x7EFE,
    0x3F00, 0x7EFE,
    0x1F00, 0x3EFC,
    0x0700, 0x1EF8,
    0x0000, 0x07F0,
    0x0000, 0x0000,
    0x0000, 0x0000,

    0x0000, 0x0000,	/* wait 5/8 */
    0x0400, 0x07C0,
    0x0000, 0x07C0,
    0x0100, 0x0380,
    0x0000, 0x07E0,
    0x0600, 0x1FF8,
    0x1E00, 0x3FFC,
    0x3E00, 0x7FFE,
    0x3E00, 0x7FFE,
    0x7E00, 0xFFFF,
    0x7E00, 0xFFFF,
    0x7E00, 0xFDFF,
    0x3C00, 0x7BFE,
    0x3800, 0x77FE,
    0x1000, 0x2FFC,
    0x0000, 0x1FF8,
    0x0000, 0x07F0,
    0x0000, 0x0000,
    0x0000, 0x0000,

    0x0000, 0x0000,	/* wait 6/8 */
    0x0400, 0x07C0,
    0x0000, 0x07C0,
    0x0100, 0x0380,
    0x0000, 0x07E0,
    0x0600, 0x1FF8,
    0x1E00, 0x3FFC,
    0x3E00, 0x7FFE,
    0x3E00, 0x7FFE,
    0x7E00, 0xFFFF,
    0x7E00, 0x81FF,
    0x0000, 0xFFFF,
    0x0000, 0x7FFE,
    0x0000, 0x7FFE,
    0x0000, 0x3FFC,
    0x0000, 0x1FF8,
    0x0000, 0x07F0,
    0x0000, 0x0000,
    0x0000, 0x0000,

    0x0000, 0x0000,	/* wait 7/8 */
    0x0400, 0x07C0,
    0x0000, 0x07C0,
    0x0100, 0x0380,
    0x0000, 0x07E0,
    0x0600, 0x1FF8,
    0x1E00, 0x2FFC,
    0x0E00, 0x77FE,
    0x0600, 0x7BFE,
    0x0200, 0xFDFF,
    0x0000, 0xFFFF,
    0x0000, 0xFFFF,
    0x0000, 0x7FFE,
    0x0000, 0x7FFE,
    0x0000, 0x3FFC,
    0x0000, 0x1FF8,
    0x0000, 0x07F0,
    0x0000, 0x0000,
    0x0000, 0x0000,

    0x0000, 0x0000,	/* wait 8/8 */
    0x0400, 0x07C0,
    0x0000, 0x07C0,
    0x0100, 0x0380,
    0x0000, 0x07E0,
    0x0100, 0x1EF8,
    0x0100, 0x3EFC,
    0x0100, 0x7EFE,
    0x0100, 0x7EFE,
    0x0100, 0xFEFF,
    0x0000, 0xFFFF,
    0x0000, 0xFFFF,
    0x0000, 0x7FFE,
    0x0000, 0x7FFE,
    0x0000, 0x3FFC,
    0x0000, 0x1FF8,
    0x0000, 0x07F0,
    0x0000, 0x0000,
    0x0000, 0x0000,
};


struct MinList SFaceList = {
    (struct MinNode *) &SFaceList.mlh_Tail, 0,
    (struct MinNode *) &SFaceList.mlh_Head
};
struct MinList DFaceList = {
    (struct MinNode *) &DFaceList.mlh_Tail, 0,
    (struct MinNode *) &DFaceList.mlh_Head
};
struct MinList MFaceList = {
    (struct MinNode *) &MFaceList.mlh_Tail, 0,
    (struct MinNode *) &MFaceList.mlh_Head
};

struct Gadget *GPSDir;
struct Gadget *GPSFace;
struct Gadget *GPDDirNext;
struct Gadget *GPDDir;
struct Gadget *GPDFace;
struct Gadget *GPInstall;
struct Gadget *GPModify;
struct Gadget *GPMFace;
struct Gadget *GPMFaceActive;
struct Gadget *GPMSize;
struct Gadget *GPMSizeNum;
struct Gadget *GPMPerform;
struct Gadget *GPMCancel;

void FountLabel(x, y, labelIndex)
int x, y;
int labelIndex;
{
    Move(RastPort, x, y);
    Text(RastPort, LzS[labelIndex], strlen(LzS[labelIndex]));
}


void RefreshUI()
{
    if (HelpImage.ImageData) {
	if (!(Window->Flags & ZOOMED))
	    /* not zoom'd small */
	    DrawImage(RastPort, &HelpImage, GHelp.LeftEdge, GHelp.TopEdge);
    }
    else if (!ModifyActive) {
	SetAPen(RastPort, DrawInfo->dri_Pens[TEXTPEN]);
	SetBPen(RastPort, 0);
	SetDrMd(RastPort, JAM2);
	FountLabel(62, TopOrigin+9, TITLE_OutlineSource);
	FountLabel(344, TopOrigin+9, TITLE_DestinationDrawer);
	DrawBevelBox(RastPort, 16, TopOrigin+15, 20, 14,
		GT_VisualInfo, VisualInfo, TAG_DONE);
	DrawBevelBox(RastPort, 314, TopOrigin+15, 20, 14,
		GT_VisualInfo, VisualInfo, TAG_DONE);
    }
}


struct Gadget *FountGadget(gid, prevGadget, left, top, width, height,
	labelIndex, flags, kind, tags)
int gid;
struct Gadget *prevGadget;
int left, top, width, height;
int labelIndex;
ULONG flags;
int kind;
ULONG tags;
{
    struct NewGadget ng;
    struct Gadget *gp;

    ng.ng_LeftEdge = left;
    ng.ng_TopEdge = top;
    ng.ng_Width = width;
    ng.ng_Height = height;
    if (labelIndex)
	ng.ng_GadgetText = LzS[labelIndex];
    else
	ng.ng_GadgetText = 0;
    ng.ng_TextAttr = &T8TA;
    ng.ng_GadgetID = gid;
    ng.ng_Flags = flags;
    ng.ng_VisualInfo = VisualInfo;

    if (!(gp = CreateGadgetA(kind, prevGadget, &ng, (struct TagItem *) &tags)))
	EndGame(RETURN_ERROR, ENDGAME_InternalNArg, "CreateGadgetA", gid);
    return(gp);
}

void OpenFountainWindow()
{
    struct Rectangle rect;
    STRPTR openTitle;
    ULONG heightTag;
    UWORD height;
    ULONG mode;
    short i;

    if (!(ClockPtr[0] = (UWORD *) AllocMem(sizeof(ClockPtrSrc), MEMF_CHIP)))
	EndGame(RETURN_ERROR, ENDGAME_NoMemory, sizeof(struct FontsEntry));

    memcpy(ClockPtr[0], ClockPtrSrc, sizeof(ClockPtrSrc));
    for (i = 1; i <= 8; i++) {
	ClockPtr[i] = ClockPtr[i-1] + CLOCKWORDS;
    }

    /* get topaz.font/8 that will be used for all text */
    if ((!(Topaz8 = OpenFont(&T8TA))) || (Topaz8->tf_YSize != 8))
	EndGame(RETURN_ERROR, ENDGAME_Internal, "OpenFont");

    /* get the destination screen */
    LockedScreen = Screen = LockPubScreen(0);
    if (!(VisualInfo = GetVisualInfo(Screen, TAG_DONE)))
	EndGame(RETURN_ERROR, ENDGAME_Internal, "GetVisualInfo");
    if (!(DrawInfo = GetScreenDrawInfo(Screen)))
	EndGame(RETURN_ERROR, ENDGAME_Internal, "GetScreenDrawInfo");

    if ((mode = GetVPModeID (&Screen->ViewPort)) == INVALID_ID)
	EndGame(RETURN_ERROR, ENDGAME_Internal, "GetVPModeID");
    QueryOverscan (mode, &rect, OSCAN_TEXT);
    height = rect.MaxY - rect.MinY + 1;
    if (height > Screen->Height)
	height = Screen->Height;

    /* determine the window y dimensions */
    ZoomSize[1] = Screen->BarHeight + 1;
    ZoomSize[2] = MAX (TextLength (&Screen->RastPort,  LzS[TITLE_Main], strlen (LzS[TITLE_Main])),
		       TextLength (&Screen->RastPort,  LzS[TITLE_Initializing], strlen (LzS[TITLE_Initializing])));
    ZoomSize[2] += 80;
    ZoomSize[3] = TopOrigin = Screen->BarHeight + 1;

    InsideHeight = height - (Screen->BarHeight * 2);
    if (InsideHeight > 200)
	InsideHeight -= (InsideHeight - 200)/2;
    if (InsideHeight < 180)
	EndGame(RETURN_ERROR, ENDGAME_FontMetricError);

    heightTag = WA_InnerHeight;
    InsideWidth = INSIDEWIDTH;
    openTitle = LzS[TITLE_Initializing];
    if (ValidateOnly)
    {
	openTitle = LzS[TITLE_Validating];
	InsideHeight = Screen->BarHeight + 1;
	InsideWidth = TextLength (&Screen->RastPort,  LzS[TITLE_Validating], strlen (LzS[TITLE_Validating])) + 80;
	heightTag = WA_Height;
    }

    /* open the window */
    if (!(Window = OpenWindowTags(NULL,
				  WA_Title, openTitle,
				  WA_Zoom, ZoomSize,
				  WA_InnerWidth, InsideWidth,
				  heightTag, InsideHeight,
				  WA_AutoAdjust, TRUE,
				  WA_PubScreen, Screen,
				  WA_IDCMP, IDCMP_FLAGS,
				  WA_Flags, WA_FLAGS,
				  TAG_DONE)))
	EndGame(RETURN_ERROR, ENDGAME_Internal, "OpenWindow");

    WaitPointer(0);

    /* now that the window is open, the screen lock can be let go */
    UnlockPubScreen(0, LockedScreen);
    LockedScreen = 0;

    /* set the window font */
    RastPort = Window->RPort;

    SetFont(RastPort, Topaz8);
}

void CloseFountainWindow()
{
    struct Message *im;

    if (Window) {
	while (im = GetMsg(Window->UserPort))
	    ReplyMsg(im);
	CloseWindow(Window);
    }
    if (ClockPtr[0])
	FreeMem(ClockPtr[0], sizeof(ClockPtrSrc));
    if (Topaz8)
	CloseFont(Topaz8);
    if (DrawInfo)
	FreeScreenDrawInfo(Screen, DrawInfo);
    if (VisualInfo)
	FreeVisualInfo(VisualInfo);
    if (LockedScreen)
	UnlockPubScreen(0, LockedScreen);
}

void CloseUI()
{
    if (GadgetList) {
	RemoveGList(Window, GadgetList, -1L);
	FreeGadgets(GadgetList);
    }
    GadgetList = 0;
    if (RastPort) {
	SetAPen(RastPort, 0);
	RectFill(RastPort,
		6, TopOrigin, INSIDEWIDTH-6, TopOrigin+InsideHeight-1);
    }
}


OpenMainUI()
{
    struct Gadget *gp;

    /* get out of the old window */
    CloseUI();
    ModifyActive = FALSE;

    /* set the new window title */
    SetWindowTitles(Window, LzS[TITLE_Main], (char *) ~0);
    WaitPointer (-1);
    if (Window->Flags & ZOOMED)
	ZipWindow (Window);

    /* get a gadtools context */
    if (!(gp = CreateContext(&GadgetList)))
	EndGame(RETURN_ERROR, ENDGAME_Internal, "CreateContext");

    /* create gadgets */
    /*   left side */
    gp->NextGadget = &GSDirReq;
    GSDirReq.TopEdge = TopOrigin+15;
    GPSDir = FountGadget(G_SDIR, &GSDirReq,
	    36, TopOrigin+15, 224, 14, 0, 0,
	    STRING_KIND, STRINGA_ExitHelp, TRUE,
	    GTST_MaxChars, 255, GTST_String, SDirName,TAG_DONE);
    GPSFace = FountGadget(G_SFACE, GPSDir,
	    16, TopOrigin+67, 244, InsideHeight-95,
	    GADGET_SourceTypefaces, PLACETEXT_ABOVE, LISTVIEW_KIND,
		GTLV_Labels, &SFaceList,
		LAYOUTA_SPACING,   1,
		GTLV_ScrollWidth,  18,
		TAG_DONE);
    /*   right side */
    GPDDirNext = FountGadget(G_DDIRNEXT, GPSFace,
	    314, TopOrigin+36, 244, 14, 0, 0,
	    CYCLE_KIND,
	    GTCY_Labels, GDDirNames, GTCY_Active, 0, TAG_DONE);
    GPDDirNext->NextGadget = &GDDirReq;
    GDDirReq.TopEdge = TopOrigin+15;
    DBG1("CurrDDir assignPath \"%s\"\n", CurrDDir->assignPath);
    GPDDir = FountGadget(G_DDIR, &GDDirReq,
	    334, TopOrigin+15, 224, 14, 0, 0,
	    STRING_KIND, STRINGA_ExitHelp, TRUE,
	    GTST_MaxChars, 255, GTST_String, CurrDDir->assignPath, TAG_DONE);
    GPDFace = FountGadget(G_DFACE, GPDDir,
	    314, TopOrigin+67, 244, InsideHeight-95,
	    GADGET_ExistingFT, PLACETEXT_ABOVE, LISTVIEW_KIND,
		GTLV_Labels, &DFaceList,
		GTLV_ReadOnly, TRUE,
		LAYOUTA_SPACING,   1,
		GTLV_ScrollWidth,  18,
		TAG_DONE);
    /*   action buttons */
    GPInstall = FountGadget(G_INSTALL, GPDFace,
	    16, TopOrigin+InsideHeight-18, 244, 14,
	    GADGET_Install, 0, BUTTON_KIND, GA_Disabled, TRUE, TAG_DONE);
    GPModify = FountGadget(G_MODIFY, GPInstall,
	    314, TopOrigin+InsideHeight-18, 244, 14,
	    GADGET_Modify, 0, BUTTON_KIND, TAG_DONE);

    /* fill in part of help gadget structure */
    HelpImage.Height = InsideHeight-4;
    HelpImage.Depth = RastPort->BitMap->Depth;
    GHelp.LeftEdge = Window->BorderLeft+2;
    GHelp.TopEdge = TopOrigin+2;
    GHelp.Height = InsideHeight-4;

    /* Add gadgets, refresh them, gadtools refresh them, refresh non-gadgets */
    AddGList(Window, GadgetList, ((UWORD) -1), ((UWORD) -1), NULL);
    RefreshGList(Window->FirstGadget, Window, NULL, ((UWORD) -1));
    GT_RefreshWindow(Window, NULL);
    RefreshUI();
    SelectDDir();
}


void OpenModifyUI()
{
    struct Gadget *gp;
    int listBodyHeight;

    /* get out of the old window */
    CloseUI();
    ModifyActive = TRUE;

    /* set the new window title */
    SetWindowTitles(Window, LzS[TITLE_Modify], (char *) ~0);

    /* get a gadtools context */
    if (!(gp = CreateContext(&GadgetList)))
	EndGame(RETURN_ERROR, ENDGAME_Internal, "CreateContext");

    /* create gadgets */
    listBodyHeight = (((InsideHeight-110)/8)*8)+4;

    /*   left side */
    gp = FountGadget(G_MDIR, gp,
	    16, TopOrigin+15, 244, 12,
	    GADGET_ExistingT, PLACETEXT_ABOVE,
	    TEXT_KIND, GTTX_Text, CurrDDir->assignPath,
	    GTTX_Border, TRUE, TAG_DONE);
    GPMFace = FountGadget(G_MFACE, gp,
	    16, TopOrigin+36, 244, listBodyHeight, 0, 0,
	    LISTVIEW_KIND,
		GTLV_Labels, &MFaceList,
		LAYOUTA_SPACING,   1,
		GTLV_ScrollWidth,  18,
		TAG_DONE);
    GPMFaceActive = FountGadget(G_MFACEACTIVE, GPMFace,
	    16, TopOrigin+listBodyHeight+36, 244, 12, 0, 0,
	    TEXT_KIND, GTTX_Text, CurrFaceDisplay->name,
	    GTTX_Border, TRUE, TAG_DONE);
    /*   right side */
    GPMSize = FountGadget(G_SIZE, GPMFaceActive,
	    358, TopOrigin+36, 156, listBodyHeight,
	    GADGET_Sizes, 0,
	    LISTVIEW_KIND,
		GTLV_Labels, &CurrFaceDisplay->sizes,
		LAYOUTA_SPACING,   1,
		GTLV_ScrollWidth,  18,
		TAG_DONE);
    GPMSizeNum = FountGadget(G_SIZENUM, GPMSize,
	    358, TopOrigin+36+listBodyHeight, 156, 14,
	    GADGET_Size, 0,
	    INTEGER_KIND,  STRINGA_ExitHelp, TRUE,
	    GTIN_Number, CurrHeight, GTIN_MaxChars, 3,
	    GT_Underscore, '_', TAG_DONE);
    /*   size action buttons */
    gp = FountGadget(G_ADDSIZE, GPMSizeNum,
	    314, TopOrigin+listBodyHeight+52, 112, 14,
	    GADGET_AddSize, 0, BUTTON_KIND, GT_Underscore, '_', TAG_DONE);
    gp = FountGadget(G_DELSIZE, gp,
	    314, TopOrigin+listBodyHeight+68, 112, 14,
	    GADGET_DeleteSize, 0, BUTTON_KIND, GT_Underscore, '_', TAG_DONE);
    gp = FountGadget(G_CREATEBM, gp,
	    446, TopOrigin+listBodyHeight+52, 112, 14,
	    GADGET_CreateBitmap, 0, BUTTON_KIND, GT_Underscore, '_', TAG_DONE);
    gp = FountGadget(G_DELETEBM, gp,
	    446, TopOrigin+listBodyHeight+68, 112, 14,
	    GADGET_DeleteBitmap, 0, BUTTON_KIND, GT_Underscore, '_', TAG_DONE);

    /*   action buttons */
    gp = FountGadget(G_DELETEFACE, gp,
	    16, TopOrigin+listBodyHeight+60, 244, 14,
	    GADGET_DeleteFace, 0, BUTTON_KIND, TAG_DONE);
    GPMPerform = FountGadget(G_PERFORM, gp,
	    16, TopOrigin+InsideHeight-18, 244, 14,
	    GADGET_PerformChanges, 0, BUTTON_KIND, GA_Disabled, TRUE, TAG_DONE);
    GPMCancel = FountGadget(G_CANCEL, GPMPerform,
	    314, TopOrigin+InsideHeight-18, 244, 14,
	    GADGET_Cancel, 0, BUTTON_KIND, TAG_DONE);

    /*  Add gadgets, refresh them, gadtools refresh them, refresh non-gadgets */
    AddGList(Window, GadgetList, ((UWORD) -1), ((UWORD) -1), NULL);
    RefreshGList(Window->FirstGadget, Window, NULL, ((UWORD) -1));
    GT_RefreshWindow(Window, NULL);
}


WaitPointer(int tick)
{
    if (tick >= 0) {
	SetPointer(Window, ClockPtr[tick], 16, 16, -6, 0);
    }
    else {
	ClearPointer(Window);
    }
}
