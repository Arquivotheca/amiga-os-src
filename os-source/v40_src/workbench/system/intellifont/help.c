/*
**	$Id: help.c,v 37.6 91/03/11 14:22:31 kodiak Exp $
**
**	Fountain/help.c -- help subsystem
**
**	(C) Copyright 1990 Robert R. Burns
**	    All Rights Reserved
*/

#include "fountain.h"

extern struct Library *SysBase;
extern struct Library *GfxBase;
extern struct Library *IntuitionBase;

extern struct DrawInfo *DrawInfo;
extern UWORD TopOrigin, InsideHeight;
extern struct TextFont *Topaz8;
extern void *VisualInfo;
extern struct Window *Window;
extern struct RastPort *RastPort;
extern struct Gadget *GadgetList;
extern struct Image HelpImage;
extern struct Gadget GHelp;

extern BOOL ModifyActive;

int HelpActive = 0;			/* current help display + 1 */
int X0, Y0, X1, Y1;			/* current help display bounds */
struct BitMap SaveBM;
struct BitMap HelpBM;
struct RastPort SaveRP;
struct RastPort HelpRP;

/*
 *	The window is divided into seven areas for the purpose of selecting
 *	help on a specific topic:
 *	    upper left, middle left, lower left, upper right, middle right,
 *	    lower right, and the current help display
 *	There are two screens.
 */

BoxTextFill(septant, index)
int septant, index;
{
    char *text, *next;
    int cx, cy, x, y, w, h, i;

    /* size the text box */
    text = LzS[index];
    w = h = 0;
    while (*text) {
	h++;
	i = 0;
	while (*text) {
	    if (*text++ == '\n')
		break;
	    i++;
	}
	if (i > w)
	    w = i;
	DBG3("Row %ld len %lx MaxCol %ld\n", h, i, w);
    }
    w = w*8+20;
    h = h*10+20;
    DBG2("Box %ld x %ld\n", w, h);

    /* determine the origin from the centering terms */
    if (septant == 0) {
	cx = cy = 0;
    }
    else {
	cx = (septant>3)?-1:1;
	cy = 1-((septant-1)%3);
    }
    X0 = Y0 = 0;
    if (cx >= 0) {
	i = HelpImage.Width-w;
	if (cx == 0)
	    X0 += i/2;
	else
	    X0 += i;
    }
    if (cy >= 0) {
	i = HelpImage.Height-h;
	if (cy == 0)
	    Y0 += i/2;
	else
	    Y0 += i;
    }
    X1 = X0+w-1;
    Y1 = Y0+h-1;
    DBG4("Box (%ld, %ld)  (%ld, %ld)\n", X0, Y0, X1, Y1);

    /* render the text box */
    SetAPen(&HelpRP, DrawInfo->dri_Pens[backgroundPen]);
    SetBPen(&HelpRP, DrawInfo->dri_Pens[backgroundPen]);
    RectFill(&HelpRP, X0, Y0, X1, Y1);
    DrawBevelBox(&HelpRP, X0+2, Y0+2, X1-X0-4, Y1-Y0-4,
	    GT_VisualInfo, VisualInfo, TAG_DONE);
    DrawBevelBox(&HelpRP, X0+3, Y0+3, X1-X0-6, Y1-Y0-6,
	    GT_VisualInfo, VisualInfo, TAG_DONE);
    DrawBevelBox(&HelpRP, X0+4, Y0+4, X1-X0-8, Y1-Y0-8,
	    GT_VisualInfo, VisualInfo, TAG_DONE);
    SetAPen(&HelpRP, DrawInfo->dri_Pens[textPen]);

    /* render the text */
    x = X0+10;
    y = Y0+6;
    next = text = LzS[index];
    while (*next) {
	i = 0;
	while (*next) {
	    if (*next++ == '\n')
		break;
	    i++;
	}
	y += 10;
	Move(&HelpRP, x, y);
	Text(&HelpRP, text, i);
	text = next;
    }
}


RenderHelpPage(page)
int page;
{
    ClipBlit(&SaveRP, 0, 0, &HelpRP, 0, 0, GHelp.Width, GHelp.Height, 0xc0);

    if (page < 0) {
	if (HelpActive) {
	    RemoveGadget(Window, &GHelp);
	    ClipBlit(&SaveRP, 0, 0, RastPort, GHelp.LeftEdge, GHelp.TopEdge,
		    GHelp.Width, GHelp.Height, 0xc0);
	    FreeMem(SaveBM.Planes[0],
		    SaveBM.Rows * SaveBM.BytesPerRow * SaveBM.Depth * 2);
	    HelpImage.ImageData = 0;
	    HelpActive = 0;
	    if (ModifyActive)
		SetWindowTitles(Window, LzS[TITLE_Modify], (char *) ~0);
	    else
		SetWindowTitles(Window, LzS[TITLE_Main], (char *) ~0);
	}
    }
    else {
	if (ModifyActive)
	    BoxTextFill(page, HELP_Introduction+page+7);
	else
	    BoxTextFill(page, HELP_Introduction+page);
	HelpActive = page+1;
	DBG1("HelpActive now %ld\n", HelpActive);
    }
    if (HelpActive)
	DrawImage(RastPort, &HelpImage, GHelp.LeftEdge, GHelp.TopEdge);
}


Help(onOff)
int onOff;
{
    UBYTE *planes;
    int planeSize, i;

    if ((!Window) || (Window->Flags & ZOOMED))
	/* no window yet or zoom'd small */
	return;

    if (onOff && (!HelpActive)) {
	InitBitMap(&SaveBM, HelpImage.Depth, HelpImage.Width, HelpImage.Height);
	InitBitMap(&HelpBM, HelpImage.Depth, HelpImage.Width, HelpImage.Height);
	planeSize = SaveBM.Rows * SaveBM.BytesPerRow;
	planes = AllocMem(planeSize * SaveBM.Depth * 2, MEMF_CHIP);
	if (planes) {
	    InitRastPort(&SaveRP);
	    InitRastPort(&HelpRP);
	    SaveRP.BitMap = &SaveBM;
	    HelpRP.BitMap = &HelpBM;
	    for (i = 0; i < HelpImage.Depth; i++) {
		SaveBM.Planes[i] = planes;
		planes += planeSize;
	    }
	    HelpImage.ImageData = (UWORD *) planes;
	    SetFont(&HelpRP, Topaz8);
	    for (i = 0; i < HelpImage.Depth; i++) {
		HelpBM.Planes[i] = planes;
		planes += planeSize;
	    }
	    ClipBlit(RastPort, GHelp.LeftEdge, GHelp.TopEdge, &SaveRP, 0, 0,
		    GHelp.Width, GHelp.Height, 0xc0);
	    ClipBlit(&SaveRP, 0, 0, &HelpRP, 0, 0,
		    GHelp.Width, GHelp.Height, 0xc0);
	    AddGadget(Window, &GHelp, 0);
	    SetWindowTitles(Window, LzS[TITLE_Help], (char *) ~0);
	    RenderHelpPage(0);
	}
    }
    if ((!onOff) && HelpActive) {
	RenderHelpPage(-1);
    }
}

SelectHelp()
{
    short mx, my, septant;

    mx = Window->MouseX-Window->BorderLeft-2;
    my = Window->MouseY-TopOrigin-2;

    if ((X0 <= mx) && (mx <= X1) && (Y0 <= my) && (my <= Y1)) {
	septant = HelpActive;
	if (septant > 6)
	    /* no more help, turn off help */
	    septant = -1;
    }
    else {
    if (mx < ((INSIDEWIDTH-4)/2))
	septant = 1;
    else
	septant = 4;

    if (my < (InsideHeight-27)) {
	if (ModifyActive) {
	    if (my > (InsideHeight-60))
		septant++;
	}
	else {
	    if (my > 58)
		septant++;
	}
    }
    else
	septant += 2;
    }
    RenderHelpPage(septant);
}
