/*
 * nlmenu.c - shows use of NewLook menus using Intuition and GadTools
 *
 * (c) Copyright 1992 Commodore-Amiga, Inc.  All rights reserved.
 *
 * This software is provided as-is and is subject to change; no warranties
 * are made.  All use is at your own risk.  No liability or responsibility
 * is assumed.
 * 
 * Demo shows off the new look menu features of V39.
 */

#include <exec/types.h>
#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <intuition/imageclass.h>
#include <libraries/gadtools.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/diskfont_protos.h>
#include <clib/gadtools_protos.h>

void printf(STRPTR,...);
int stcd_l(char *, long *);
void exit(int);

/*------------------------------------------------------------------------*/

void main(int, char *[]);
void bail_out(int);
BOOL HandleMenuEvent(UWORD);

/*------------------------------------------------------------------------*/

/* Here we specify what we want our menus to contain: */

struct NewMenu mynewmenu[] =
{
	{ NM_TITLE, "Project",	  0 , 0, 0, 0,},
	{  NM_ITEM, "Open...",	 "O", 0, 0, 0,},
	{  NM_ITEM, "Save",	  0 , 0, 0, 0,},
	{  NM_ITEM, NM_BARLABEL,  0 , 0, 0, 0,},
	{  NM_ITEM, "Print",	  0 , 0, 0, 0,},
	{   NM_SUB, "Draft",	  0 , CHECKIT|CHECKED, ~1, 0,},
	{   NM_SUB, "NLQ",	  0 , CHECKIT, ~2, 0,},
	{   NM_SUB, "Laser",	  0 , CHECKIT, ~4, 0,},
	{  NM_ITEM, NM_BARLABEL,  0 , 0, 0, 0,},
	{  NM_ITEM, "Quit...",	 "Q", 0, 0, 0,},

	{ NM_TITLE, "Edit",	  0 , 0, 0, 0,},
	{  NM_ITEM, "Cut",	 "X", 0, 0, 0,},
	{  NM_ITEM, "Copy",	 "C", 0, 0, 0,},
	{  NM_ITEM, "Paste",	 "V", 0, 0, 0,},
	{  NM_ITEM, NM_BARLABEL,  0 , 0, 0, 0,},
	{  NM_ITEM, "Undo",	 "Z", 0, 0, 0,},

	{   NM_END, 0,		  0 , 0, 0, 0,},
};

/*------------------------------------------------------------------------*/

struct TextAttr customtattr;
struct TextAttr *tattr;

/*------------------------------------------------------------------------*/

struct GfxBase *GfxBase = NULL;
struct IntuitionBase *IntuitionBase = NULL;
struct Library *GadToolsBase = NULL;
struct Library *DiskfontBase = NULL;
struct Screen *mysc = NULL;
struct Menu *menu = NULL;
struct Window *mywin = NULL;
struct TextFont *customfont = NULL;
void *vi = NULL;
struct DrawInfo *dri = NULL;
struct Image *checkimage = NULL;
struct Image *amigakeyimage = NULL;

/*------------------------------------------------------------------------*/

BOOL terminated;

/*------------------------------------------------------------------------*/

void main(argc, argv)

int argc;
char *argv[];

{
    struct IntuiMessage *imsg;
    ULONG imsgClass;
    UWORD imsgCode;
    struct TagItem moretags[3];

    terminated = FALSE;

    if (argc == 2)
    {
	printf("Usage:\n\tnlmenu\nor\n\tnlmenu fontname.font fontsize\n");
	printf("Example:\n\tnlmenu courier.font 15\n");
	bail_out(0);
    }
    /* Open all libraries: */

    if (!(GfxBase = (struct GfxBase *)
	OpenLibrary("graphics.library", 39L)))
	bail_out(20);

    if (!(IntuitionBase = (struct IntuitionBase *)
	OpenLibrary("intuition.library", 39L)))
	bail_out(20);

    if (!(GadToolsBase = OpenLibrary("gadtools.library", 39L)))
	bail_out(20);

    if (!(DiskfontBase = OpenLibrary("diskfont.library", 37L)))
	bail_out(20);

    if (!(mysc = LockPubScreen(NULL)))
	bail_out(20);

    if (!(vi = GetVisualInfo(mysc,
	TAG_DONE)))
	bail_out(20);

    if (!(dri = GetScreenDrawInfo(mysc)))
	bail_out(20);

    if (argc < 3)
    {
	/* Default to screen's font */
	tattr = mysc->Font;
    }
    else
    {
    LONG longval;

	customtattr.ta_Style = 0;
	customtattr.ta_Flags = 0;
	/* Attempt to use the font specified on the command line: */
	customtattr.ta_Name = argv[1];
	/* Convert decimal size to long */
	stcd_l(argv[2], &longval);
	customtattr.ta_YSize = longval;
	tattr = &customtattr;
	if (!(customfont = OpenDiskFont(tattr)))
	{
	    printf("Could not open font %s %ld\n", customtattr.ta_Name,
		customtattr.ta_YSize);
	    bail_out(20);
	}

	/* Generate a custom checkmark whose size matches
	 * our custom font
	 */
	if (!( checkimage = NewObject(NULL, "sysiclass",
	    SYSIA_DrawInfo, dri,
	    SYSIA_Which, MENUCHECK,
	    SYSIA_ReferenceFont, customfont, /* If NULL, uses dri_Font */
	    TAG_DONE) ))
	{
	    bail_out(20);
	}

	/* Generate a custom Amiga-key image whose size matches
	 * our custom font
	 */
	if (!( amigakeyimage = NewObject(NULL, "sysiclass",
	    SYSIA_DrawInfo, dri,
	    SYSIA_Which, AMIGAKEY,
	    SYSIA_ReferenceFont, customfont, /* If NULL, uses dri_Font */
	    TAG_DONE) ))
	{
	    bail_out(20);
	}
    }

    /* Build and layout menus using the right font: */
    if (!(menu = CreateMenus(mynewmenu,
	TAG_DONE)))
    {
	bail_out(20);
    }

    /* These are only necessary if a custom font was supplied... */
    moretags[0].ti_Tag = GTMN_Checkmark;
    moretags[0].ti_Data = (ULONG) checkimage;
    moretags[1].ti_Tag = GTMN_AmigaKey;
    moretags[1].ti_Data = (ULONG) amigakeyimage;
    moretags[2].ti_Tag = TAG_DONE;

    if (!LayoutMenus(menu, vi,
	GTMN_TextAttr, tattr,
	GTMN_NewLookMenus, TRUE,
	(customfont ? TAG_MORE : TAG_DONE), moretags))
	bail_out(20);

    /* These are only necessary if a custom font was supplied...
     * Note: we re-use some of the tag-array initializations from above
     */
    moretags[0].ti_Tag = WA_Checkmark;
    moretags[1].ti_Tag = WA_AmigaKey;

    if (!(mywin = OpenWindowTags(NULL,
	WA_Width, 500,
	WA_InnerHeight, 100,
	WA_Top, 50,

	WA_Activate, TRUE,
	WA_DragBar, TRUE,
	WA_DepthGadget, TRUE,
	WA_CloseGadget, TRUE,
	WA_SizeGadget, TRUE,
	WA_SmartRefresh, TRUE,

	/* NOTE: NOCAREREFRESH is not allowed if you use GadTools Gadgets! */
	WA_NoCareRefresh, TRUE,

	WA_IDCMP, CLOSEWINDOW | MENUPICK,

	WA_MinWidth, 50,
	WA_MinHeight, 50,
	WA_Title, "GadTools Menu Demo",
	WA_NewLookMenus, TRUE,
	(customfont ? TAG_MORE : TAG_DONE), moretags)))
	bail_out(20);

    SetMenuStrip(mywin, menu);

    while (!terminated)
    {
	Wait (1 << mywin->UserPort->mp_SigBit);
	/* NOTE:  If you use GadTools gadgets, you must use GT_GetIMsg()
	 * and GT_ReplyIMsg() instead of GetMsg() and ReplyMsg().
	 * Regular GetMsg() and ReplyMsg() are safe if the only part
	 * of GadTools you use are menus...
	 */
	while ((!terminated) &&
	    (imsg = (struct IntuiMessage *)GetMsg(mywin->UserPort)))
	{
	    imsgClass = imsg->Class;
	    imsgCode = imsg->Code;
	    ReplyMsg((struct Message *)imsg);
	    switch (imsgClass)
	    {
		case MENUPICK:
		    terminated = HandleMenuEvent(imsgCode);
		    break;

		case CLOSEWINDOW:
		    printf("CLOSEWINDOW.\n");
		    terminated = TRUE;
		    break;
	    }
	}
    }
    bail_out(0);
}

/*------------------------------------------------------------------------*/

/*/ bail_out()
 *
 * Function to close down or free any opened or allocated stuff, and then
 * exit.
 *
 */

void bail_out(code)

int code;

{
    if (mywin)
    {
	ClearMenuStrip(mywin);
	CloseWindow(mywin);
    }

    /* None of these two calls mind a NULL parameter, so it's not
     * necessary to check for non-NULL before calling.  If we do that,
     * we must be certain that the OpenLibrary() of GadTools succeeded,
     * or else we would be jumping into outer space:
     */
    if (GadToolsBase)
    {
	FreeMenus(menu);
	FreeVisualInfo(vi);
	CloseLibrary(GadToolsBase);
    }

    if (dri)
    {
	FreeScreenDrawInfo( mysc, dri );
    }

    if (customfont)
    {
	DisposeObject( amigakeyimage );
	DisposeObject( checkimage );
	CloseFont(customfont);
    }

    if (mysc)
    {
	UnlockPubScreen(NULL, mysc);
    }

    if (DiskfontBase)
    {
	CloseLibrary(DiskfontBase);
    }

    if (IntuitionBase)
    {
	CloseLibrary(IntuitionBase);
    }

    if (GfxBase)
    {
	CloseLibrary(GfxBase);
    }

    exit(code);
}


/*------------------------------------------------------------------------*/

/*/ HandleMenuEvent()
 *
 * This function handles IntuiMessage events of type MENUPICK.
 *
 */

BOOL HandleMenuEvent(code)

UWORD code;

{
    /* Your code goes here */
    return(FALSE);
}

/*------------------------------------------------------------------------*/
