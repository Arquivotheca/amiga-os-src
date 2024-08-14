
/*** display.c ************************************************************
*
*   display.c	- 	Display routines for Overscan Editor
*
*   Copyright 1989, 1990, Commodore-Amiga, Inc.
*
*   $Id: display.c,v 36.11 91/02/19 11:29:10 eric Exp $
*
*   $Log:	display.c,v $
*   Revision 36.11  91/02/19  11:29:10  eric
*   Changed menuitem from Quit... to Quit
*   
*   Revision 36.10  91/01/15  13:35:40  eric
*   Now includes internal/asl_obsolete.h instead of libraries/asl.h
*   Changed obsolete W_ tags to WA_ variants.
*   
*   Revision 36.9  91/01/15  12:10:26  eric
*   Release 2.03
*   
*   Revision 36.8  90/08/01  13:06:17  peter
*   The code which was conditional on "#define MODECHOICE" (not defined)
*   has been removed.  This code would have allowed the mode-within-monitor
*   selection that we used to have when Overscan had two scrolling lists.
*   The (compiled-out) code to display the video-overscan dimensions has
*   been pulled.
*   
*   Revision 36.7  90/07/26  14:53:31  peter
*   Now include asl.h instead of aslbase.h.
*   
*   Revision 36.6  90/06/08  23:26:59  peter
*   Uses new PRESET_DIR and PRESET_NAME for initial contents of file requester.
*   
*   Revision 36.5  90/05/17  18:05:05  peter
*   Rearranged order in which things are done inside InitModeForDisplay()
*   to permit better-looking refresh.
*   
*   Revision 36.4  90/05/16  01:27:10  peter
*   No longer provide initial blank strings to TEXT_KIND gadgets.
*   
*   Revision 36.3  90/05/15  00:35:49  peter
*   Conditionally compiled out code to handle editing of arbitrary modes,
*   and removed second scrolling list.
*   Changed DE_XXX labels to GAD_XXX labels.
*   Now uses text-gadgets for the size display.
*   Removed code for "Show Overscans".
*   Rearranged window to account for the removed gadgets.
*   
*   Revision 36.2  90/04/19  23:43:14  peter
*   Not including <stdio.h> anymore.
*   
*   Revision 36.1  90/04/18  13:48:43  peter
*   Now opens up on the same screen it thinks it will (!).
*   
*   Revision 36.0  90/04/09  17:39:36  peter
*   Initial RCS check-in.
*   
*
**************************************************************************/

/*------------------------------------------------------------------------*/

#include <exec/types.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>

#include <libraries/gadtools.h>
#include <libraries/asl.h>

#include <string.h>

#include "overscan.h"
#include "display.h"

extern UBYTE *VerTitle;
extern BOOL machineispal;

#define ASL_LOADFLAGS	(FILF_DOMSGFUNC)
#define ASL_SAVEFLAGS	(ASL_LOADFLAGS | FILF_SAVE)

/*------------------------------------------------------------------------*/

/*  Functions defined elsewhere: */

BOOL EditOScans(WORD oscantype, struct Window *wbwin);
WORD WriteOScanPrefs(char *name);
WORD ReadOScanPrefs(char *name, BOOL Quiet);
void AdoptPrefsEntries(struct MinList *list);
WORD rectWidth( struct Rectangle *rect );
WORD rectHeight( struct Rectangle *rect );



ULONG handleReqRefresh( ULONG mask, struct IntuiMessage *imsg,
    struct FileRequester *freq );
struct Window *InitDisplay(void);
long HandleDisplayMessage(struct Window *win,
    struct IntuiMessage *imsg, BOOL inedit);
VOID FreeDisplay(struct Window *win);
VOID UpdateDisplay(struct Window *win);

BOOL CreatePrefsGadgets(struct Window *win, void *vi, UBYTE topborder);
VOID FreePrefsGadgets(struct Window *win);
UBYTE *SPrintfRectSize(UBYTE *buffer, struct Rectangle *rect);
void establishMonitor(struct Window *win);
struct FileRequester *GetFileRequest(struct Window *win,
    struct FileRequester *frequest);
void busyWindow( struct Window *win );
void readyWindow( struct Window *win );

void WriteIcon(UBYTE *name, BYTE unit);

/*------------------------------------------------------------------------*/

extern struct MinList restorelist;
extern struct MinList defaultlist;
extern struct List monitors;

extern struct MsgPort *DummyPort;

struct MonitorEntry *current;

Point ViewPos, dViewPos;
BOOL noadjust;
struct Gadget *edittxtgad, *editstdgad, *monitor_gad, *nominal_gad,
    *text_gad, *standard_gad, *maximum_gad;

UBYTE topborder;

BOOL SaveIcon = TRUE;

struct Menu *menu = NULL;
struct Screen *mysc = NULL;
struct DrawInfo *mydri = NULL;
struct Gadget *firstgad = NULL;
struct TextFont *displayfont = NULL;
void *vi = NULL;
struct FileRequester *freq = NULL;

UBYTE namebuffer[NAMEBUFSIZE];

UBYTE TextPen, HighTextPen;

#define SIZEBUFFER	16	/* 14 is needed */

/*  To hold text-gadget displayed values: */
UBYTE NominalBuffer[SIZEBUFFER];
UBYTE TextBuffer[SIZEBUFFER];
UBYTE StandardBuffer[SIZEBUFFER];
UBYTE MaximumBuffer[SIZEBUFFER];

/*------------------------------------------------------------------------*/

struct NewWindow mynewwin =
    {
    WINLEFT, WINTOP,		/*  LeftEdge, TopEdge */
    WINWIDTH, WINHEIGHT,	/*  Width, Height */
    -1, -1,             /*  DetailPen, BlockPen */
    MENUPICK | REFRESHWINDOW | GADGETUP | /* CLOSEWINDOW |*/
	LISTVIEWIDCMP, /*  IDCMPFlags */
    ACTIVATE | WINDOWDRAG | WINDOWDEPTH | /*WINDOWCLOSE |*/
    SIMPLE_REFRESH,	/* Flags */
    NULL,		/*  FirstGadget */
    NULL,		/*  CheckMark */
#ifdef RELEASE
    (UBYTE *) "Overscan Preferences", /*  Title */
#else
    NULL,
#endif
    NULL,		/*  Screen */
    NULL,		/*  BitMap */    
    MINWINWIDTH, MINWINHEIGHT,	/*  MinWidth, MinHeight */
    MAXWINWIDTH, MAXWINHEIGHT,	/*  MaxWidth, MaxHeight */
    WBENCHSCREEN,	/*  Type */
    };


WORD ZoomSize[4] =
	{
	ZOOMLEFT, ZOOMTOP,
	ZOOMWIDTH, ZOOMHEIGHT,
	};

struct TextAttr Topaz80 =
    {
    "topaz.font",	/*  Name */
    8,			/*  YSize */
    0,			/*  Style */
    0,			/*  Flags */
    };

#define OPEN_MENU	1
#define	SAVEAS_MENU	2
#define QUIT_MENU	3
#define RESETALL_MENU	4
#define LASTSAVED_MENU	5
#define RESTORE_MENU	6
#define SAVEICONS_MENU	7

struct NewMenu mynewmenu[] =
    {
	{ NM_TITLE, "Project",	  0 , 0, 0, 0,},
	{  NM_ITEM, "Open...",	 "O", 0, 0, ((void *)OPEN_MENU),},
	{  NM_ITEM, "Save As...", 0 , 0, 0, ((void *)SAVEAS_MENU),},
	{  NM_ITEM, NM_BARLABEL,  0 , 0, 0, 0,},
	{  NM_ITEM, "Quit",	 "Q", 0, 0, ((void *)QUIT_MENU),},

	{ NM_TITLE, "Edit",	  0 , 0, 0, 0,},
	{  NM_ITEM, "Reset To Defaults",  0, 0, 0, ((void *)RESETALL_MENU),},
	{  NM_ITEM, "Last Saved", 0 , 0, 0, ((void *)LASTSAVED_MENU),},
	{  NM_ITEM, "Restore",	  0 , 0, 0, ((void *)RESTORE_MENU),},

	{ NM_TITLE, "Options",	  0 , 0, 0, 0,},
	{  NM_ITEM, "Save Icons?",0 , CHECKIT|CHECKED|MENUTOGGLE, 0, ((void *)SAVEICONS_MENU),},

	{   NM_END, 0,		  0 , 0, 0, 0},
    };


/*------------------------------------------------------------------------*/

/*/ InitDisplay()
 *
 * Function to initialize the display for this preferences editor.
 * Opens the window, creates the gadgets, and displays it all.
 * Returns pointer to the window if successful, NULL otherwise.
 *
 * Created:   6-Sep-89, Peter Cherna
 * Modified:  3-Apr-90, Peter Cherna
 *
 */

struct Window *InitDisplay()
{
    struct Window *mywin = NULL;
    BOOL found;
    ULONG myscreenID;

    /*  Open display font: */

    if (!(displayfont = OpenFont(&Topaz80)))
    {
	FreeDisplay(NULL);
	return(NULL);
    }

    if (!(mysc = LockPubScreen(NULL)))
    {
	FreeDisplay(NULL);
	return(NULL);
    }

    if (!(mydri = GetScreenDrawInfo(mysc)))
    {
	FreeDisplay(NULL);
	return(NULL);
    }

    TextPen = mydri->dri_Pens[textPen];
    HighTextPen = mydri->dri_Pens[hilighttextPen];

    /*  cribbed from intuition/wtags.c, so it had better be right :-) */
    topborder = mysc->WBorTop + (mysc->Font->ta_YSize + 1);
    if (!(vi = GetVisualInfo(mysc,
	TAG_DONE)))
    {
	FreeDisplay(NULL);
	return(NULL);
    }

    /*  Build and layout menus: */
    if (!(menu = CreateMenus(mynewmenu,
	TAG_DONE)))
    {
	FreeDisplay(NULL);
	return(NULL);
    }

    if (!LayoutMenus(menu, vi,
	TAG_DONE))
    {
	FreeDisplay(NULL);
	return(NULL);
    }


    /*  We want to find the monitor of the current (displayed) mode: */

    myscreenID = MONITOR_PART( GetVPModeID(&mysc->ViewPort) );
    /* Since we now deal in explicit NTSC and PAL, we've got to
     * do this transformation in order to find the current monitor
     * in our lists.
     */
    if ( !myscreenID )
	{
	myscreenID = (machineispal) ? PAL_MONITOR_ID : NTSC_MONITOR_ID;
	}

    DP(("ID of screen is $%lx, monitor part = $%lx\n", 
	GetVPModeID(&mysc->ViewPort), myscreenID));

    found = FALSE;
    /*  Now look for the ID in our stuff: */
    current = (struct MonitorEntry *)monitors.lh_Head;
    while ((!found) && (current->me_Node.ln_Succ))
    {
	DP(("Comparing against ID $%lx\n", current->me_ID));
	found = (BOOL) ( MONITOR_PART(current->me_ID) == myscreenID );
	if (!found)
	{
	    current = (struct MonitorEntry *)current->me_Node.ln_Succ;
	}
    }

    if (!found)
    {
	DP(("Workbench's modeID not found in our lists; using first available\n"));
	if (!Empty( &monitors ))
	{
	    current = (struct MonitorEntry *) monitors.lh_Head;
	}
	else
	{   /*  Error on account of no monitors found */
	    FreeDisplay(NULL);
	    return(NULL);
	}
    }

    /*  Create the various gadgets: */
    if (!CreatePrefsGadgets(mywin, vi, topborder))
    {
	FreeDisplay(NULL);
	return(NULL);
    }

    ZoomSize[3] = topborder;
    /*  Open the window: */
    if (!(mywin = OpenWindowTags(&mynewwin,
	WA_Zoom, ZoomSize,
	WA_InnerHeight, WINHEIGHT,
	WA_AutoAdjust, TRUE,
	WA_PubScreen, mysc,
#ifndef RELEASE
	WA_Title, VerTitle,
#endif
	TAG_DONE )))
	{
	FreeDisplay(NULL);
	return(NULL);
	}

    /*  Stuff window pointer for use by another invocation: */
    DummyPort->mp_SigTask = (struct Task *)mywin;

    /*  Set the font to the display font */
    SetFont(mywin->RPort, displayfont);

    AddGList(mywin, firstgad, ((USHORT) -1), ((USHORT) -1), NULL);

    establishMonitor(mywin);

    RefreshGList(firstgad, mywin, NULL, ((USHORT) -1));
    GT_RefreshWindow(mywin, NULL);

    /*  NOTE WELL:  Once the window opens, we're guaranteed to arrive
	here, so the free-code does a ClearMenuStrip() if the window
	opens.  If something should change, we may have to adapt */
    SetMenuStrip(mywin, menu);

    return(mywin);
    }


/*------------------------------------------------------------------------*/

/*/ CreatePrefsGadgets()
 *
 * Function to allocate and create all the necessary gadgets for
 * the Overscan editor.
 *
 * Created:  15-Sep-89, Peter Cherna
 * Modified: 18-Jan-90, Peter Cherna
 *
 * Bugs: None
 *
 * Returns:  (BOOL) success = TRUE
 */

BOOL CreatePrefsGadgets(win, vi, topborder)

    struct Window *win;
    UBYTE topborder;
    void *vi;

    {
    struct Gadget *gad;
    struct NewGadget ng;

    gad = CreateContext(&firstgad);

    /*  Scrolling List of monitors: */

    ng.ng_LeftEdge = 16;
    ng.ng_Width = 276;
    ng.ng_TopEdge = 4 + topborder;
    ng.ng_Height = 56;
    ng.ng_GadgetText = NULL;
    ng.ng_TextAttr = &Topaz80;
    ng.ng_GadgetID = GAD_MONITORS;
    ng.ng_Flags = NG_HIGHLABEL;
    ng.ng_VisualInfo = vi;

    monitor_gad = gad = CreateGadget(LISTVIEW_KIND, gad, &ng,
	GTLV_Labels, &monitors,
	GTLV_ShowSelected, NULL,
	GTLV_Selected, current->me_Index,
	LAYOUTA_SPACING, 1,
	GTLV_ScrollWidth, 18,
	TAG_DONE);

    /*  "Save" gadget: */
    ng.ng_GadgetID = GAD_SAVE;
    ng.ng_LeftEdge = 16;
    ng.ng_TopEdge = 140+topborder;
    ng.ng_Width = 64;
    ng.ng_Height = 12;
    ng.ng_GadgetText = "Save";
    ng.ng_Flags = 0;
    gad = CreateGadget(BUTTON_KIND, gad, &ng,
	TAG_DONE);

    /*  "Use" gadget */
    ng.ng_GadgetID++;		/*  GAD_USE */
    ng.ng_LeftEdge = 122;
    ng.ng_GadgetText = "Use";
    gad = CreateGadget(BUTTON_KIND, gad, &ng,
	TAG_DONE);

    /*  "Cancel" gadget: */
    ng.ng_GadgetID++;		/*  GAD_CANCEL */
    ng.ng_LeftEdge = 228;
    ng.ng_GadgetText = "Cancel";
    gad = CreateGadget(BUTTON_KIND, gad, &ng,
	TAG_DONE);

    noadjust = FALSE;
    /*  "Edit Text Overscan" gadget: */
    ng.ng_GadgetID++;		/*  GAD_EDITTXT */
    ng.ng_LeftEdge = 36;
    ng.ng_TopEdge = 60 + topborder;
    ng.ng_Width = 236;
    ng.ng_GadgetText = "Edit Text Overscan...";
    edittxtgad = gad = CreateGadget(BUTTON_KIND, gad, &ng,
	TAG_DONE);


    /*  "Edit Standard Overscan" gadget: */
    ng.ng_GadgetID++;		/*  GAD_EDITSTD */
    ng.ng_TopEdge += 16;
    ng.ng_GadgetText = "Edit Standard Overscan...";
    editstdgad = gad = CreateGadget(BUTTON_KIND, gad, &ng,
	TAG_DONE);

    ng.ng_LeftEdge = 186;
    ng.ng_TopEdge = 92 + topborder;
    ng.ng_Height = 8;
    ng.ng_Flags = NG_HIGHLABEL;
    ng.ng_GadgetText = "Regular Size:";
    nominal_gad = gad = CreateGadget(TEXT_KIND, gad, &ng,
	TAG_DONE);

    ng.ng_TopEdge += 12;
    ng.ng_GadgetText = "Text Overscan:";
    text_gad = gad = CreateGadget(TEXT_KIND, gad, &ng,
	TAG_DONE);

    ng.ng_TopEdge += 12;
    ng.ng_GadgetText = "Standard Overscan:";
    standard_gad = gad = CreateGadget(TEXT_KIND, gad, &ng,
	TAG_DONE);

    ng.ng_TopEdge += 12;
    ng.ng_GadgetText = "Maximum Overscan:";
    maximum_gad = gad = CreateGadget(TEXT_KIND, gad, &ng,
	TAG_DONE);

    return((BOOL)(gad != NULL));
    }


/*------------------------------------------------------------------------*/

/*/ FreeDisplay()
 *
 * Function to free up the display associated stuff.
 *
 * Created:   6-Sep-89, Peter Cherna
 * Modified:  1-Mar-90, Peter Cherna
 *
 */

VOID FreeDisplay(win)

    struct Window *win;

    {
    if (freq)
	{
	FreeFileRequest(freq);
	}

    if (win)
	{
	DummyPort->mp_SigTask = NULL;
	ClearMenuStrip(win);
	CloseWindow(win);
	}

    /*  It is safe to call both FreeMenus() and FreeVisualInfo() with
	NULL parameters, so no checking is needed: */

    FreeMenus(menu);
    FreeVisualInfo(vi);

    if (mysc)
	{
	if (mydri)
	    {
	    FreeScreenDrawInfo(mysc, mydri);
	    }
	UnlockPubScreen(NULL, mysc);
	}

    FreeGadgets(firstgad);

    if (displayfont)
	CloseFont(displayfont);
    }


/*------------------------------------------------------------------------*/

ULONG handleReqRefresh( mask, imsg, freq )

ULONG mask;
struct IntuiMessage *imsg;
struct FileRequester *freq;
{
    if ( imsg->Class == REFRESHWINDOW )
    {
	HandleDisplayMessage( imsg->IDCMPWindow, imsg, FALSE );
    }
    return( (ULONG) imsg );
}

/*------------------------------------------------------------------------*/

/*/ HandleDisplayMessage()
 *
 * Function to handle the different IntuiMessages.  Returns
 * TRUE if a terminating action occurred.
 *
 * Created:  29-Aug-89, Peter Cherna
 * Modified:  1-Mar-90, Peter Cherna
 *
 * Bugs:  none
 *
 * Returns:  (BOOL) TRUE if the action was a terminating action.
 *
 */

long HandleDisplayMessage(win, imsg, inedit)

    struct Window *win;
    struct IntuiMessage *imsg;
    BOOL inedit;

    {
    struct Gadget *gad;
    struct Node *node, *succ;
    long retval;
    UWORD code;
    struct MenuItem *item;
    BOOL newmonitor = FALSE;

    code = imsg->Code;

    retval = 0;

    /*  We always listen for REFRESHWINDOW and CLOSEWINDOW events: */
    switch (imsg->Class)
	{
	case REFRESHWINDOW:
	    GT_BeginRefresh(win);
	    GT_EndRefresh(win, TRUE);
	    break;

/*
	case CLOSEWINDOW:
	    retval = GAD_CANCEL;
	    break;
*/
	}

    /*  The rest are ignored if we are in the edit screen: */
    if (!inedit)
	{
	switch (imsg->Class)
	    {
	    case GADGETUP:
		gad = (struct Gadget *) imsg->IAddress;

		switch (gad->GadgetID)
		    {
		    case GAD_MONITORS:
			node = monitors.lh_Head;
			while ((succ = node->ln_Succ) && (code--))
			    {
			    node = succ;
			    }
			current = (struct MonitorEntry *)node;
			newmonitor = TRUE;
			break;

		    case GAD_SAVE:
		    case GAD_USE:
		    case GAD_CANCEL:
			retval = gad->GadgetID;
			break;

		    case GAD_EDITTXT:
		    case GAD_EDITSTD:
			busyWindow( win );
			retval = EditOScans(gad->GadgetID, win);
			readyWindow( win );
			newmonitor = TRUE;
			break;

		    }
		break;

	    case MENUPICK:
		while ((code != MENUNULL) && (!retval))
		    {
		    item = ItemAddress(menu, code);
		    /*  Note that in C, switch demands an "int": */
		    switch ( (int)MENU_USERDATA(item) )
			{
			case OPEN_MENU:
			    if (freq = GetFileRequest(win, freq))
			    {
				busyWindow( win );
				if (AslRequestTags(freq,
				    ASL_Hail, "Load Overscan Preferences",
				    ASL_FuncFlags, ASL_LOADFLAGS,
				    TAG_DONE))
				{
				    stccpy(namebuffer, freq->rf_Dir, NAMEBUFSIZE);
				    if (AddPart(namebuffer, freq->rf_File, NAMEBUFSIZE))
				    {
					ReadOScanPrefs(namebuffer, FALSE);
					newmonitor = TRUE;
				    }
				}
				readyWindow( win );
			    }
			    break;
			case SAVEAS_MENU:
			    if (freq = GetFileRequest(win, freq))
			    {
				busyWindow( win );
				if (AslRequestTags(freq,
				    ASL_Hail, "Save Overscan Preferences",
				    ASL_FuncFlags, ASL_SAVEFLAGS,
				    TAG_DONE))
				{
				    stccpy(namebuffer, freq->rf_Dir, NAMEBUFSIZE);
				    if (AddPart(namebuffer, freq->rf_File, NAMEBUFSIZE))
				    {
					if ((WriteOScanPrefs(namebuffer))
					  == ST_OK)
					{
					    if (SaveIcon == TRUE)
						WriteIcon(namebuffer, 0);
					}
				    }
				}
				readyWindow( win );
			    }
			    break;
			case QUIT_MENU:
			    retval = GAD_CANCEL;
			    break;
			case RESETALL_MENU:
			    AdoptPrefsEntries(&defaultlist);
			    newmonitor = TRUE;
			    break;
			case LASTSAVED_MENU:
			    ReadOScanPrefs(ARC_NAME, FALSE);
			    newmonitor = TRUE;
			    break;
			case RESTORE_MENU:
			    AdoptPrefsEntries(&restorelist);
			    newmonitor = TRUE;
			    break;
			case SAVEICONS_MENU:
			    if (item->Flags & CHECKED)
				SaveIcon = TRUE;
			    else
				SaveIcon = FALSE;
			    break;
			}
		    /*  There may be more menu selections to process */
		    code = item->NextSelect;
		    }
		break;
	    }
	if (newmonitor)
	    {
	    establishMonitor(win);
	    }
	}
    return(retval);
    }


/*------------------------------------------------------------------------*/

/*/ SPrintfRectSize()
 *
 * Prints the size of a Rectangle into a buffer, in the form #### x ####
 *
 * Created:  15-Sep-89, Peter Cherna
 * Modified: 14-May-90, Peter Cherna
 *
 */

UBYTE *SPrintfRectSize(buffer, rect)

    UBYTE *buffer;
    struct Rectangle *rect;

    {
    /*  Print the rectangle's size: */
    sprintf(buffer,"%4ld x %4ld", rectWidth( rect ), rectHeight( rect ));
    return(buffer);
    }

/*------------------------------------------------------------------------*/

/*/ establishMonitor()
 *
 * Establish 'current' as the current monitor.  Update the gadgets
 * in the preferences window, and cache some view-position information.
 *
 */

void establishMonitor(win)

    struct Window *win;

    {
    DP(("establishMonitor\n"));
    if ((rectWidth(&current->me_DimensionInfo.MaxOScan) ==
	rectWidth(&current->me_DimensionInfo.Nominal)) &&
	(rectHeight(&current->me_DimensionInfo.MaxOScan) ==
	rectHeight(&current->me_DimensionInfo.Nominal)))
	{
	/*  No adjustment is possible */
	if (!noadjust)
	    {
	    noadjust = TRUE;
	    GT_SetGadgetAttrs(editstdgad, win, NULL,
		GA_DISABLED, TRUE,
		TAG_DONE);
	    GT_SetGadgetAttrs(edittxtgad, win, NULL,
		GA_DISABLED, TRUE,
		TAG_DONE);
	    }
	}
    else
	{
	/*  Adjustment is possible: */
	if (noadjust)
	    {
	    noadjust = FALSE;
	    GT_SetGadgetAttrs(editstdgad, win, NULL,
		GA_DISABLED, FALSE,
		TAG_DONE);
	    GT_SetGadgetAttrs(edittxtgad, win, NULL,
		GA_DISABLED, FALSE,
		TAG_DONE);
	    }
	}
    /*  We'll need to track how much the view position changed:
	ViewPos.x and .y are the current view position: */
    ViewPos.x = current->me_MonitorInfo.ViewPosition.x;
    ViewPos.y = current->me_MonitorInfo.ViewPosition.y;
    DP(("ViewPos: [%ld,%ld]\n", (LONG)ViewPos.x, (LONG)ViewPos.y));
    /*  dViewPos.x and dViewPos.y measure (in ViewResolution units)
	the relationship between the MaxOScan upper left limit
	and the ViewPosition (their sum is always constant,
	and dViewPos is that sum): */
    dViewPos.x = ViewPos.x + (((LONG)current->me_DimensionInfo.MaxOScan.MinX) *
	current->me_DisplayInfo.Resolution.x) / current->me_MonitorInfo.ViewResolution.x;
    dViewPos.y = ViewPos.y + (((LONG)current->me_DimensionInfo.MaxOScan.MinY) *
	current->me_DisplayInfo.Resolution.y) / current->me_MonitorInfo.ViewResolution.y;
    DP(("dViewPos: [%ld,%ld]\n", (LONG)dViewPos.x, (LONG)dViewPos.y));

    SPrintfRectSize(NominalBuffer, &current->me_DimensionInfo.Nominal);
    SPrintfRectSize(TextBuffer, &current->me_DimensionInfo.TxtOScan),
    SPrintfRectSize(StandardBuffer, &current->me_DimensionInfo.StdOScan),
    SPrintfRectSize(MaximumBuffer, &current->me_DimensionInfo.MaxOScan),

    GT_SetGadgetAttrs(monitor_gad, win, NULL,
	GTLV_Selected, current->me_Index,
	TAG_DONE);

    GT_SetGadgetAttrs(nominal_gad, win, NULL,
	GTTX_Text, NominalBuffer,
	TAG_DONE);

    GT_SetGadgetAttrs(text_gad, win, NULL,
	GTTX_Text, TextBuffer,
	TAG_DONE);

    GT_SetGadgetAttrs(standard_gad, win, NULL,
	GTTX_Text, StandardBuffer,
	TAG_DONE);

    GT_SetGadgetAttrs(maximum_gad, win, NULL,
	GTTX_Text, MaximumBuffer,
	TAG_DONE);
    }


/*------------------------------------------------------------------------*/

/*/ GetFileRequest()
 *
 * If passed a NULL pointer to filerequester, GetFileRequest() will attempt
 * to allocate and initialize one.
 *
 * Created:   6-Mar-90, Peter Cherna
 * Modified:  6-Mar-90, Peter Cherna
 *
 * Returns:  pointer to file request or NULL if failure.
 *
 */

struct FileRequester *GetFileRequest(win, frequest)

    struct Window *win;
    struct FileRequester *frequest;

    {
    if (!frequest)
	{
	/*  If we don't have one, try to get one: */
	frequest = AllocAslRequestTags(ASL_FileRequest,
	    ASL_Window, win,
	    ASL_HookFunc, handleReqRefresh,
	    ASL_File, PRESET_NAME,
	    ASL_Dir, PRESET_DIR,
	    TAG_DONE);
	}
    return(frequest);
    }

/*------------------------------------------------------------------------*/

BOOL reqSuccess;
struct Requester dummyRequest;

/* Mark window as busy.  We clear the menustrip, put up a dummy
 * requester (to block input to the window), and maybe put up
 * a busy pointer (not implemented).
 */

void busyWindow( win )

struct Window *win;

{
    InitRequester( &dummyRequest );

    ClearMenuStrip( win );
    reqSuccess = Request( &dummyRequest, win );
}

/*------------------------------------------------------------------------*/

/* Reverses busyWindow.  We clear the pointer (not implemented)
 * remove the dummy requester used to block input, and
 * reset the menu strip.
 */


void readyWindow( win )

struct Window *win;

{
    if ( reqSuccess )
    {
	EndRequest( &dummyRequest, win );
    }
    ResetMenuStrip( win, menu );
}

/*------------------------------------------------------------------------*/
