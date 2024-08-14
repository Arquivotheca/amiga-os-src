/* ieio.c
 * Written by David N. Junod
 *
 */

/* includes */
#include <exec/types.h>
#include <graphics/gfxmacros.h>
#include <string.h>
#include <stdio.h>

/* prototypes */
#include <clib/macros.h>
#include <clib/exec_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/icon_protos.h>
#include <clib/utility_protos.h>

/* direct ROM interface */
#include <pragmas/exec_pragmas.h>
#include <pragmas/gadtools_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/icon_pragmas.h>
#include <pragmas/utility_pragmas.h>

/* application includes */
#include "ieio.h"
#include "iemain.h"
#include "iemenus.h"
#include "iemisc.h"
#include "iegads.h"
#include "iesrc.h"
#include "texttable.h"

/*****************************************************************************/

#define	DB(x)	;

/*****************************************************************************/

extern struct Library *GadToolsBase;
extern struct Library *IntuitionBase;
extern struct Library *GfxBase;
extern struct Library *IconBase;
extern struct Library *UtilityBase;

/*****************************************************************************/

static int ResizeImage (DynamicImagePtr si, DynamicImagePtr di);
static int CompareSizes (USHORT flags, DynamicImagePtr di1, DynamicImagePtr di2);

/*****************************************************************************/

VOID ClearIcon (struct Image * im, struct RastPort * rp)
{

    SetAPen (rp, 0);
    SetBPen (rp, 0);
    SetAfPt (rp, NULL, 0);
    SetRast (rp, 0);

    if (im)
    {
	DrawImage (rp, im, 0, 0);
    }
}

/*****************************************************************************/

VOID UpdateIconInfo (WindowInfoPtr wi, struct DiskObject * dob)
{
    struct Window *wp;
    USHORT pos;

    wp = wi->win;
    if (wp && dob)
    {
	wi->type = dob->do_Type - 1;
	wi->hilite = dob->do_Gadget.Flags & GADGHIGHBITS;
	CheckMenuItem (wi, 2, wi->type);
	CheckMenuItem (wi, 3, wi->hilite);
	SetType (wi, dob, wi->type);
	SetHilite (wi, dob, wi->hilite);
	pos = RemoveGList (wp, wi->img1, 1);
	wi->img1->Flags = (USHORT) wi->hilite | GADGIMAGE;

	ClearIcon ((struct Image *) dob->do_Gadget.GadgetRender, &wi->images[0].di_RPort);
	ClearIcon ((struct Image *) dob->do_Gadget.SelectRender, &wi->images[1].di_RPort);

	AddGList (wp, wi->img1, (LONG) pos, 1, NULL);
	RefreshImages (wi);
    }
}

/*****************************************************************************/

struct DiskObject *LoadIcon (STRPTR name, BOOL real)
{
    struct DiskObject *dob;
    SHORT i;

    if (real)
    {
	if (!(dob = GetDiskObject (name)))
	{
	    i = strlen (name);
	    if (i > 5)
	    {
		if (Stricmp (&name[i - 5], ".INFO") == 0)
		{
		    name[i - 5] = 0;
		    dob = LoadIcon (name, real);
		}
	    }
	}
	return (dob);
    }

    return (GetDiskObjectNew (name));
}

/*****************************************************************************/

BOOL CheckForClipped (WindowInfoPtr wi)
{
    UBYTE pname[255];

    strcpy (pname, wi->iconname);
    if (strlen (pname) < 1)
	strcpy (pname, GetString (MSG_IE_UNTITLED));

    return (EasyReq (wi->win, VNAM, GetString (MSG_IE_SAVEAFTERCLIP),
		     GetString (MSG_IE_SAVEAFTERCLIP_GADS),
		     pname) != 0);
}

/*****************************************************************************/

VOID SetUpperPlanes (WindowInfoPtr wi, DynamicImagePtr di, int maxDepth)
{
    struct BitMap *bm = &di->di_BMap;
    int i, j, k;
    int planes;

    DB (kprintf ("SetUpperPlanes : "));

    if (maxDepth > 2)
    {
	j = wi->mysc->BitMap.Depth - 1;
	planes = (int) RASSIZE (di->di_Image.Width, di->di_Image.Height);

	DB (kprintf ("j=%ld, depth=%ld\n", (ULONG)j, (ULONG)di->di_Image.Depth));

	for (i = 2; i < (ULONG)di->di_Image.Depth; i++)
	{
	    if (i != j)
		memcpy ((char *)bm->Planes[i], (char *)bm->Planes[j], planes);
	}
    }
    else
    {
	DB (kprintf ("maxdepth=%ld\n", (ULONG) maxDepth));
    }
}

/*****************************************************************************/

BOOL SaveIcon (WindowInfoPtr wi, STRPTR name, struct DiskObject * dob, WORD mode)
{
    struct Window *wp;
    BOOL retval = FALSE;
    struct DrawerData *tmpdraw = NULL;
    BOOL clipped = FALSE;
    int maxDepth = 0;

    wp = wi->win;
    if (wp && name && dob)
    {
	struct Image *im;
	SHORT defw, defh;
	APTR backup1, backup2;
	char *deftool;
	struct DiskObject *defdob = NULL;
	struct DrawerData *defdraw;
	UBYTE deftype;
	USHORT defflags, i;
	BOOL chop;

	/* Strip the .info of the name of the file */
	i = strlen (name);
	chop = TRUE;
	while (i > 5 && chop)
	{
	    chop = FALSE;
	    if (Stricmp (&name[i - 5], ".INFO") == 0)
	    {
		name[i - 5] = 0;
		chop = TRUE;
	    }
	    i = strlen (name);
	}

#if 0
	dob->do_CurrentX = NO_ICON_POSITION;
	dob->do_CurrentY = NO_ICON_POSITION;
#endif

	defw = dob->do_Gadget.Width;
	defh = dob->do_Gadget.Height;
	backup1 = dob->do_Gadget.GadgetRender;
	backup2 = dob->do_Gadget.SelectRender;
	deftool = dob->do_DefaultTool;
	defdraw = dob->do_DrawerData;
	deftype = dob->do_Type;
	defflags = dob->do_Gadget.Flags;
	dob->do_Gadget.Flags = GADGIMAGE | ((USHORT) wi->hilite);

	/* correct type specific fields */
	if (dob->do_Type != (wi->type + 1))
	{
	    dob->do_Type = wi->type + 1;
	    defdob = GetDefDiskObject (dob->do_Type);
	    if (defdob)
		tmpdraw = defdob->do_DrawerData;
	    switch (dob->do_Type)
	    {
		case WBDISK:
		    if (!dob->do_DrawerData)
		    {
			dob->do_DefaultTool = (char *) "SYS:System/DiskCopy";
			dob->do_DrawerData = tmpdraw;
		    }
		    break;
		case WBDRAWER:
		case WBGARBAGE:
		    if (!dob->do_DrawerData)
		    {
			dob->do_DefaultTool = NULL;
			dob->do_DrawerData = tmpdraw;
		    }
		    break;
		case WBTOOL:
		case WBPROJECT:
		    if (dob->do_DrawerData)
			dob->do_DrawerData = NULL;
		    break;
	    }
	}

	/* Has the image been modified? */
	if (wi->changed & CH_MAJOR)
	{
	    DB (kprintf ("Major Change\n"));

	    /* Initialize the resize images */
	    InitDynamicImage (&wi->images[2], 0, 0, 0);
	    InitDynamicImage (&wi->images[3], 0, 0, 0);

	    /* Resize the normal image */
	    dob->do_Gadget.GadgetRender = (APTR) & wi->images[0].di_Image;
	    maxDepth = ResizeImage (&wi->images[0], &wi->images[2]);

	    DB (kprintf ("maxDepth = %ld\n", (ULONG) maxDepth));

	    /* Do we have an alternate image to resize? */
	    if (dob->do_Gadget.Flags & GADGHIMAGE)
	    {
		dob->do_Gadget.SelectRender = (APTR) & wi->images[1].di_Image;
		maxDepth = MAX (maxDepth, ResizeImage (&wi->images[1], &wi->images[3]));
	    }
	    else
	    {
		dob->do_Gadget.SelectRender = NULL;
	    }

	    /* See who is largest */
	    CompareSizes (dob->do_Gadget.Flags, &wi->images[2], &wi->images[3]);

	    if ((AllocDynamicImage (&wi->images[2])) && (AllocDynamicImage (&wi->images[3])))
	    {
		if (dob->do_Gadget.SelectRender)
		{
		    DrawImage (&wi->images[3].di_RPort, &wi->images[1].di_Image, 0, 0);
		    SetUpperPlanes (wi, &wi->images[3], maxDepth);
		    dob->do_Gadget.SelectRender = (APTR) & wi->images[3].di_Image;
		}

		DrawImage (&wi->images[2].di_RPort, &wi->images[0].di_Image, 0, 0);
		SetUpperPlanes (wi, &wi->images[2], maxDepth);
		dob->do_Gadget.GadgetRender = (APTR) & wi->images[2].di_Image;
	    }
	    else
	    {
		DB (kprintf ("couldn't allocate dynamic images\n"));
	    }

	    im = (struct Image *) dob->do_Gadget.GadgetRender;
	    dob->do_Gadget.Width = im->Width;
	    dob->do_Gadget.Height = im->Height + 1;

	    if (wi->clippable)
	    {
		clipped = TRUE;
		wi->clippable = FALSE;
	    }
	}
	else
	{
	    DB (kprintf ("Minor Change\n"));
#if 0
	    if (dob->do_Gadget.SelectRender)
		SetUpperPlanes (wi, &wi->images[1]);
	    SetUpperPlanes (wi, &wi->images[0]);
#endif
	}

	if ((!clipped) || CheckForClipped (wi))
	{
	    if (mode == 0)
	    {
		if (!(retval = PutDiskObject (name, dob)))
		{
		    NotifyUser (MSG_IE_ERROR_SAVE_ICON, NULL);
		}
	    }
	    else if (mode == 1)
	    {
		if (!(retval = PutDefDiskObject (dob)))
		{
		    NotifyUser (MSG_IE_ERROR_SAVE_DEF_ICON, NULL);
		}
	    }
	    else
	    {
		if (!(IconToC (wi->iconname, name, dob, wi->w_SaveIcons)))
		{
		    NotifyUser (MSG_IE_ERROR_SAVE_SOURCE, NULL);
		}
	    }
	}

	dob->do_Gadget.Flags = defflags;
	dob->do_Gadget.Width = defw;
	dob->do_Gadget.Height = defh;
	dob->do_Gadget.GadgetRender = backup1;
	dob->do_Gadget.SelectRender = backup2;
	dob->do_DefaultTool = deftool;
	dob->do_DrawerData = defdraw;
	dob->do_Type = deftype;

	FreeDynamicImage (&wi->images[2]);
	FreeDynamicImage (&wi->images[3]);

#if 1
	CreateWinTitle (wi, name);
#else
	sprintf (wi->wintitle, "IconEdit: %s", name);
#endif
	SetWindowTitles (wp, wi->wintitle, (UBYTE *) - 1);

	if (defdob)
	{
	    FreeDiskObject (defdob);
	}
    }

    return (retval);
}

/*****************************************************************************/

static int CompareSizes (USHORT flags, DynamicImagePtr di1, DynamicImagePtr di2)
{
    int fw, fh, fd = 0;

    if (di1 && di2)
    {
	if (!(flags & GADGHIMAGE))
	{
	    di2->di_Image.Width = 1;
	    di2->di_Image.Height = 1;
	    di2->di_Image.Depth = 1;
	}

	fw = MAX (di1->di_Image.Width, di2->di_Image.Width);
	fh = MAX (di1->di_Image.Height, di2->di_Image.Height);
	fd = MAX (di1->di_Image.Depth, di2->di_Image.Depth);
	if (fd > 2)
	    fd = 8;

	/* Reinitialize to the common size */
	InitDynamicImage (di1, fd, fw, fh);
	InitDynamicImage (di2, fd, fw, fh);
    }
    return (fd);
}

/*****************************************************************************/

static int ResizeImage (DynamicImagePtr si, DynamicImagePtr di)
{
    struct RastPort *rp = &(si->di_RPort);
    SHORT w = si->di_Image.Width;
    SHORT h = si->di_Image.Height;
    SHORT x, y, mx = 0, my = 0, mp = 0, p;
    int fd;

    for (x = 0; x < w; x++)
    {
	for (y = 0; y < h; y++)
	{
	    p = ReadPixel (rp, x, y);
	    if (p > 0)
	    {
		mx = MAX (x, mx);
		my = MAX (y, my);
		mp = MAX (p, mp);
	    }
	}
    }

    DB (kprintf ("mp=%ld\n", (LONG) mp));

#if 1
    fd = 8;
    if (mp < 2)
	fd = 1;
    else if (mp < 4)
	fd = 2;
#else
    fd = 0;
    if (mp < 2)
	fd = 1;
    else if (mp < 4)
	fd = 2;
    else if (mp < 8)
	fd = 3;
    else if (mp < 16)
	fd = 4;
    else if (mp < 32)
	fd = 5;

    if (fd > 2)
	fd = 8;
#endif

    InitDynamicImage (di, fd, mx + 1, my + 1);
    di->di_Flags = DI_LAYER;

    return (fd);
}

/*****************************************************************************/

VOID SPUpdateColors (WindowInfoPtr wi)
{
    struct Window *wp;
    struct SketchPad *sp;
    struct RastPort *rp;
    UWORD colors;

    wp = wi->win;
    if (wp)
    {
	sp = wi->wi_sketch;
	rp = sp->Drp;

	colors = (1 << wi->w_Depth);
	if (sp->APen >= colors)
	    sp->APen = 0;
	if (sp->BPen >= colors)
	    sp->BPen = 0;

	SetAPen (rp, sp->APen);
	SetBPen (rp, sp->BPen);
	SetDrMd (rp, JAM2);
	SetAfPt (rp, sp->AreaPtrn, sp->AreaPtSz);
	RectFill (rp, (wi->w_LO + 4), (SHORT) (wi->topborder + 4),
		  (wi->w_LO + 70), (SHORT) (wi->topborder + 14));
	SetAfPt (rp, NULL, 0);
	SetBPen (rp, 0);
    }
}

/*****************************************************************************/

void RefreshCustomImagery (WindowInfoPtr wi)
{
    struct Window *wp;
    struct RastPort *rp;
    SketchPadPtr sp;

    wp = wi->win;
    if (wp)
    {
	rp = wp->RPort;
	sp = wi->wi_sketch;

	/* Current Color Box */
	DrawBevelBox (rp, wi->w_LO, (SHORT) (wi->topborder + 2), 75, 15,
		      GT_VisualInfo, wi->vi,
		      GTBB_Recessed, TRUE,
		      TAG_DONE);
	SPUpdateColors (wi);

	/* Tool Box */
	DrawBevelBox (rp, (wi->w_LO), (SHORT) (wi->topborder + 93), 75, 46,
		      GT_VisualInfo, wi->vi,
		      GTBB_Recessed, TRUE,
		      TAG_DONE);

	DrawBevelBox (rp, wi->w_LO + 3, (SHORT) (wi->topborder + 95), 69, 42,
		      GT_VisualInfo, wi->vi,
		      TAG_DONE);

	/* Image Boxes */
	DrawBevelBox (rp,
		      (SHORT) ((wi->w_LO + 77) - 4 + 16), (SHORT) (wi->topborder + IMG1TOP - 2),
		      (SHORT) (ICON_WIDTH + 8), (SHORT) (ICON_HEIGHT + 4),
		      GT_VisualInfo, wi->vi,
		      TAG_DONE);

	DrawBevelBox (rp,
		      (SHORT) ((wi->w_LO + 77) - 4 + 16), (SHORT) (wi->topborder + IMG2TOP - 2),
		      (SHORT) (ICON_WIDTH + 8), (SHORT) (ICON_HEIGHT + 4),
		      GT_VisualInfo, wi->vi,
		      GTBB_Recessed, TRUE,
		      TAG_DONE);

	/* Box Around ARROWS */
	DrawBevelBox (rp,
	    ((wi->w_LO + 77) - 4 + 16), (SHORT) (wi->topborder + 120), 87, 44,
		      GT_VisualInfo, wi->vi,
		      GTBB_Recessed, TRUE,
		      TAG_DONE);

	DrawBevelBox (rp,
		  (wi->w_LO + 77 + 16), (SHORT) (wi->topborder + 122), 79, 40,
		      GT_VisualInfo, wi->vi,
		      TAG_DONE);

	/* Box Around SketchPad ***TEMPORARY*** */
	DrawBevelBox (sp->Drp,
		      (SHORT) (sp->LeftEdge - 2), (SHORT) (sp->TopEdge - 1),
		    (SHORT) (sp->Gad.Width + 4), (SHORT) (sp->Gad.Height + 2),
		      GT_VisualInfo, wi->vi,
		      TAG_DONE);

	SPRefresh (sp);
    }
}
