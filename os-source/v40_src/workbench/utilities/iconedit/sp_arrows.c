
/* prototypes */
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>

/* direct ROM interface */
#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>

/* application includes */
#include "sp_arrows.h"
#include "iemain.h"


/*****************************************************************************/


extern struct Library *IntuitionBase;
extern struct Library *GfxBase;


/*****************************************************************************/


/* Arrow Data --- single bit-plane.  Use for mask & image */
USHORT __chip ArrowUp[] =
{				/* Up Arrow */
    0x000E, 0x0000, 0x003F, 0x8000, 0x00FF, 0xE000, 0x03FF, 0xF800,
    0x0FFF, 0xFE00, 0x3FFF, 0xFF80, 0xFFFF, 0xFFE0, 0x00FF, 0xE000,
    0x00FF, 0xE000, 0x00FF, 0xE000, 0x00FF, 0xE000, 0x00FF, 0xE000,
    0x00FF, 0xE000
};

USHORT __chip ArrowRight[] =
{				/* Right Arrow */
    0x0018, 0x0000, 0x001E, 0x0000, 0x001F, 0x8000, 0xFFFF, 0xE000,
    0xFFFF, 0xF800, 0xFFFF, 0xFE00, 0xFFFF, 0xF800, 0xFFFF, 0xE000,
    0x001F, 0x8000, 0x001E, 0x0000, 0x0018, 0x0000
};

USHORT __chip ArrowDown[] =
{				/* Down Arrow */
    0x00FF, 0xE000, 0x00FF, 0xE000, 0x00FF, 0xE000, 0x00FF, 0xE000,
    0x00FF, 0xE000, 0x00FF, 0xE000, 0xFFFF, 0xFFE0, 0x3FFF, 0xFF80,
    0x0FFF, 0xFE00, 0x03FF, 0xF800, 0x00FF, 0xE000, 0x003F, 0x8000,
    0x000E, 0x0000
};

USHORT __chip ArrowLeft[] =
{				/* Left Arrow */
    0x0030, 0x0000, 0x00F0, 0x0000, 0x03F0, 0x0000, 0x0FFF, 0xFE00,
    0x3FFF, 0xFE00, 0xFFFF, 0xFE00, 0x3FFF, 0xFE00, 0x0FFF, 0xFE00,
    0x03F0, 0x0000, 0x00F0, 0x0000, 0x0030, 0x0000
};

struct Image Arrows[] =
{
    {0, 0, 27, 13, 1, ArrowUp, 0x0001, 0x0000, NULL},
    {0, 0, 23, 11, 1, ArrowRight, 0x0001, 0x0000, NULL},
    {0, 0, 27, 13, 1, ArrowDown, 0x0001, 0x0000, NULL},
    {0, 0, 23, 11, 1, ArrowLeft, 0x0001, 0x0000, NULL}
};

struct BoolInfo AMask[] =
{
    {BOOLMASK, ArrowUp, 0},
    {BOOLMASK, ArrowRight, 0},
    {BOOLMASK, ArrowDown, 0},
    {BOOLMASK, ArrowLeft, 0}
};

struct Gadget MGad[] =
{
    {NULL, 19, 0, 27, 13, NULL, NULL, BOOLGADGET, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
    {&MGad[0], 42, 12, 23, 11, NULL, NULL, BOOLGADGET, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
    {&MGad[1], 19, 22, 27, 13, NULL, NULL, BOOLGADGET, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
    {&MGad[2], 0, 12, 23, 11, NULL, NULL, BOOLGADGET, NULL, NULL, NULL, NULL, NULL, NULL, NULL}
};


/*****************************************************************************/


VOID AddMoveBox(struct SketchPad * sp, SHORT LeftEdge, SHORT TopEdge)
{
struct Window *win = sp->Window;
SHORT i;
struct Image *im;

    for (i = 0; i < 4; i++)
    {
	im = (struct Image *) & Arrows[i];
	MGad[i].LeftEdge    += (LeftEdge + 11) + 16;
	MGad[i].TopEdge     += (TopEdge + 5);
	MGad[i].Width        = im->Width;
	MGad[i].Height       = im->Height;
	MGad[i].Flags        = GADGIMAGE | GADGHCOMP;
	MGad[i].Activation   = GADGIMMEDIATE | RELVERIFY | BOOLEXTEND;
	MGad[i].GadgetRender = (APTR) im;
	MGad[i].SpecialInfo  = (APTR) & AMask[i];
	MGad[i].GadgetID     = SPSCROLL_ID;
	MGad[i].UserData     = (APTR) (i + 1);
    }
    AddGList (win, &MGad[3], ((USHORT) - 1), ((USHORT) - 1), NULL);
}


/*****************************************************************************/


VOID MoveImage(USHORT id, DynamicImagePtr di)
{
SHORT mx, my;

    switch (id)
    {
	case 1:
	    mx = 0;
	    my = 1;
	    break;

	case 2:
	    mx = (-1);
	    my = 0;
	    break;

	case 3:
	    mx = 0;
	    my = (-1);
	    break;

	case 4:
	    mx = 1;
	    my = 0;
	    break;
    }
    ScrollRaster(&di->di_RPort,mx,my,0,0,di->di_Image.Width,di->di_Image.Height);
}


/*****************************************************************************/


VOID SPScroll(struct SketchPad * sp, struct IntuiMessage * msg)
{
ULONG class = msg->Class;
struct Gadget *gad = (struct Gadget *) msg->IAddress;
USHORT gid = (USHORT) gad->UserData;

    if (class != GADGETUP)
    {
	SPSaveToUndo(sp);
	MoveImage(gid,sp->Adi);
	SPRefresh(sp);
    }
}
