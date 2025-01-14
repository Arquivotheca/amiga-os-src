/*
 * $Id: gels.c,v 38.2 92/02/03 13:14:17 mks Exp $
 *
 * $Log:	gels.c,v $
 * Revision 38.2  92/02/03  13:14:17  mks
 * Fixed the way BOBs look on higher depth screens.  If the screen
 * is deeper than the icon it will now correctly only invert the
 * planes as needed...
 * 
 * Revision 38.1  91/06/24  11:35:37  mks
 * Initial V38 tree checkin - Log file stripped
 *
 */

#include "proto.h"
#include "string.h"
#include "exec/types.h"
#include "exec/memory.h"
#include "exec/alerts.h"
#include "libraries/dos.h"
#include "libraries/dosextens.h"
#include "graphics/rastport.h"
#include "graphics/gels.h"
#include "graphics/regions.h"
#include "graphics/gfxmacros.h"
#include "intuition/intuition.h"
#include "workbench.h"
#include "workbenchbase.h"
#include "global.h"
#include "support.h"

#define	MIN_BOX_SIZE	8

/* blitter modes */
#define COPY_MODE	0xC0
#define MASKCOPY_MODE	0xE0
#define INVERT_MODE	0x30
#define MASKINVERT_MODE	0x00


struct GV {
    struct GelsInfo gv_GelsInfo;
    struct VSprite gv_vs0;
    struct VSprite gv_vs1;
};

int initWbGels()
{
    struct WorkbenchBase *wb = getWbBase();
    struct RastPort *rp;
    struct GV *gv;

    rp = &wb->wb_Screen->RastPort;

    if (gv = ALLOCVEC(sizeof(struct GV), MEMF_CLEAR|MEMF_PUBLIC))
    {
	InitGels(&gv->gv_vs0, &gv->gv_vs1, &gv->gv_GelsInfo);
	rp->GelsInfo = &gv->gv_GelsInfo;
	wb->wb_GelsInfo = &gv->gv_GelsInfo;
    }
    return((int)gv);
}

void uninitWbGels()
{
    struct WorkbenchBase *wb = getWbBase();

    MP(("uninitWbGels: enter\n"));
    MP(("\tfreeing %ld bytes @$%lx\n",
	sizeof(struct GV), wb->wb_GelsInfo));
    FREEVEC(wb->wb_GelsInfo);
    MP(("uninitWbGels: exit\n"));
}

/*
***************************************************************************
** makeBob -- turn a workbench image into a bob
**
** This routine does the low level hacking necessary to make a Bob
** and a VSprite structure out of a workbench image
**
** It allocates all the memory it needs, and puts this memory on
** the wb-global Bob memory free list
**
** INPUTS:
**	im -- a workbench image structure
**	mode -- either COPY_MODE or INVERT_MODE (corresponding to
**		blitter mode bits
**	maskplane -- non-zero if this image already has a mask
**		computed for it
**
** RESULTS
**	ptr to initialize Bob structure, or NULL on failure
**
***************************************************************************
*/
struct Bob *
makeBob( im, mode, maskplane )
struct Image *im;
ULONG mode;
UWORD *maskplane;
{
    struct BV {
	struct Bob bv_Bob;
	struct VSprite bv_VSprite;
    } *bv;
    struct WorkbenchBase *wb = getWbBase();
    struct Bob *b;
    struct VSprite *vs;
    UWORD *buf;
    ULONG planesize;
    int alloccopy = 0;			/* number of planes to clone data */
    int allocmask = 0;			/* number of planes to hold mask */

    /* get the extra memory we need */
    bv = WBFreeAlloc(&wb->wb_BobMem,sizeof(struct BV),MEMF_PUBLIC|MEMF_CLEAR);
    if( ! bv ) return( NULL );

    /* get local pointers to them */
    b = &bv->bv_Bob;
    vs = &bv->bv_VSprite;

    /* make them point to each other */
    b->BobVSprite = vs;
    vs->VSBob = b;

    /* set the size and data fields */
    vs->Height = im->Height;
    vs->Width = (im->Width +15)>> 4;	/* convert to words */
    vs->Depth = im->Depth;
    vs->PlanePick = (1 << im->Depth)-1;	/*im->PlanePick;*/
    vs->ImageData = im->ImageData;

    DP(("makeBob: PlanePick=$%lx, PlaneOnOff=$%lx, VSDepth=$%lx, Depth=$%lx\n",
	vs->PlanePick, vs->PlaneOnOff, vs->Depth, wb->wb_Screen->RastPort.BitMap->Depth));

    /* allocate a hit field */
    planesize = vs->Width * vs->Height;

    if( mode != COPY_MODE ) alloccopy = vs->Depth;
    if( ! maskplane ) allocmask = 1;

    /* !!! breaks if planeonoff is used ? */
    /* remember that width is in words */
    buf = WBFreeAlloc(&wb->wb_BobMem,(planesize*(allocmask+alloccopy+wb->wb_Screen->RastPort.BitMap->Depth)+vs->Width) * 2,MEMF_CHIP|MEMF_CLEAR);
    if( !buf ) return( NULL );

    vs->BorderLine = buf;
    buf = &buf[vs->Width];


    b->SaveBuffer = buf;
    buf = &buf[planesize*wb->wb_Screen->RastPort.BitMap->Depth];

    if( alloccopy ) {
	struct BitMap SBM, DBM;

	vs->ImageData = buf;
	buf = &buf[planesize*alloccopy];

	ImageToBitMap( im, &SBM, (UBYTE *)im->ImageData );
	ImageToBitMap( im, &DBM, (UBYTE *)vs->ImageData );

	BltBitMap( &SBM, 0, 0, &DBM, 0, 0, im->Width, im->Height, mode,
	    im->PlanePick, 0 );
    }

    if( maskplane )
    {
	/* a mask was provided for us */
	vs->CollMask = b->ImageShadow = maskplane;

    }
    else
    {
	/* generate a mask that is the entire image */
	UWORD *m;
	UWORD endOfLineMask;
	int rows;

	m = vs->CollMask = b->ImageShadow = buf;
	/* buf = &buf[planesize]; */

	/* mask for unused bits at the end of a line */
	if( ! (endOfLineMask = ~(0xffff >> (im->Width & 0xf))) )
	{
	    /* im->Width was divisible by 16 */
	    endOfLineMask = 0xffff;
	}

	rows = vs->Height;
	while( rows-- )
	{
	    int cols = vs->Width;
	    while( --cols )
	    {
		*m++ = ~0;
	    }
	    *m++ = endOfLineMask;
	}
    }


    {
	/* we have not yet initialized BorderLine */
	UWORD *bl;
	UWORD *m;
	UWORD *borderend;
	int i;

	bl = vs->BorderLine;
	borderend = &bl[vs->Width];	/* Width is in words */

	/* zero the buffer */
	for( ; bl < borderend ; *bl++ = 0 );

	/* OR all the lines together */
	for( m = b->ImageShadow, i = vs->Height; i-- > 0; ) {
	    for( bl = vs->BorderLine; bl < borderend; ) {
		*bl++ |= *m++;
	    }
	}
    }

    vs->Flags = SAVEBACK | OVERLAY;

    return( b );
}

int DragSelectIcon(struct WBObject *obj,struct Region *r,struct Window *w,struct NewDD *dd)
{
struct	WorkbenchBase	*wb=getWbBase();

	/* Check if the sibling is in the window */
	if (obj->wo_IconWin==w)
	{
	struct	Rectangle	ibox;
	struct	Region		*myr;

		ibox.MinX=obj->wo_Gadget.LeftEdge+w->LeftEdge;
		ibox.MinY=obj->wo_Gadget.TopEdge+w->TopEdge;
		ibox.MaxX=ibox.MinX+obj->wo_Gadget.Width-1;
		ibox.MaxY=ibox.MinY+obj->wo_Gadget.Height-1;

		if (dd->dd_ViewModes <= DDVM_BYICON)
		{
			ibox.MaxX+=EMBOSEWIDTH;
			ibox.MaxY+=EMBOSEHEIGHT;
		}
		else ibox.MaxX+=VIEWBYTEXTLEFTOFFSET;

		/* Build a region for the icon... */
		if (myr=NewRegion())
		{
			if (OrRectRegion(myr,&ibox))
			{
				/* Check for intersection with visible window */
				if (AndRegionRegion(r,myr))
				{
					if (myr->RegionRectangle) SelectIt(obj);
				}
			}
			DisposeRegion(myr);
		}
	}
	return(0);
}

/*
 * This routine does the new drag selection method.  It goes on a
 * window by window basis and thus should be much faster.
 */
int DragSelect(struct WBObject *obj,struct Rectangle *box)
{
struct	NewDD		*dd=obj->wo_DrawerData;
struct	Window		*w;
struct	Region		*r;	/* The region we will use...

	/*
	 * We only care about the windows...
	 */
	if (dd) if (w=dd->dd_DrawerWin)
	{
	struct	Rectangle	wbox;
	struct	Layer		*l;
	struct	Layer		*wl=w->WLayer;
		short		flag;

		/*
		 * Lock the layers while we do this window
		 */
		LockLayerInfo(wl->LayerInfo);

		wbox.MinX=w->LeftEdge+w->BorderLeft;
		wbox.MinY=w->TopEdge+w->BorderTop;
		wbox.MaxX=w->LeftEdge+w->Width-w->BorderRight;
		wbox.MaxY=w->TopEdge+w->Height-w->BorderBottom;

		/*
		 * Check if this window is even in the box...
		 */
		if (!((wbox.MinX > box->MaxX) ||
		      (wbox.MinY > box->MaxY) ||
		      (wbox.MaxX < box->MinX) ||
		      (wbox.MaxY < box->MinY)    ))
		{
			if (r=NewRegion())
			{
				/* Make region the window */
				if (OrRectRegion(r,&wbox))
				{
					/* Mask in the select box */
					AndRectRegion(r,box);
					flag=TRUE;
					/* Zap out all layers that are above me... */
					for (l=wl->LayerInfo->top_layer;l!=wl;l=l->back)
					{
						flag&=ClearRectRegion(r,&(l->bounds));
					}
					/* If all is still go */
					if (flag) if (r->RegionRectangle)
					{
						SiblingSuccSearch(dd->dd_Children.lh_Head,DragSelectIcon,r,w,dd);
					}
				}
				DisposeRegion(r);
			}
		}

		/*
		 * Let layers go again...
		 */
		UnlockLayerInfo(wl->LayerInfo);
	}
	CheckDiskIO();
	return(0);
}

RedrawOneBob( bn, dx, dy )
struct BobNode *bn;
LONG dx, dy;
{
    struct VSprite *vs;

    vs = bn->bn_Bob->BobVSprite;

    vs->X += dx;
    vs->Y += dy;

    return( 0 );
}

void RedrawBobs( msg )
struct IntuiMessage *msg;
{
    struct WorkbenchBase *wb = getWbBase();
    struct Screen *screen;
    struct RastPort *rp;
    LONG dx, dy;

    screen = wb->wb_Screen;

    rp = &screen->RastPort;

    if( msg ) {
	dx = msg->MouseX - wb->wb_LastX;
	dy = msg->MouseY - wb->wb_LastY;
	wb->wb_LastX = msg->MouseX;
	wb->wb_LastY = msg->MouseY;

	SearchList( &wb->wb_BobList, RedrawOneBob, dx, dy );
    }
#ifdef MEMDEBUG
    PrintAvailMem("before calling SortGList", MEMF_CHIP);
#endif MEMDEBUG
    SortGList( rp );
#ifdef MEMDEBUG
    PrintAvailMem("after calling SortGList", MEMF_CHIP);
#endif MEMDEBUG
    DrawGList( rp, &screen->ViewPort );
#ifdef MEMDEBUG
    PrintAvailMem("after calling DrawGList", MEMF_CHIP);
#endif MEMDEBUG
}

AddWbBob( obj, screen )
struct WBObject *obj;
struct Screen *screen;
{
    struct WorkbenchBase *wb = getWbBase();
    struct BobNode *bn;
    struct VSprite *vs;
    struct Image *im;
    struct Window *win;
    ULONG mode;
    UWORD *mask;
    struct NewDD *dd = obj->wo_Parent->wo_DrawerData;

    MP(("AddWbBob: enter, obj=$%lx (%s), screen=$%lx (%s)\n",
	obj, obj->wo_Name, screen, screen->Title));
#ifdef MEMDEBUG
    PrintAvailMem("AddWbBob", MEMF_CHIP);
#endif MEMDEBUG
    bn = WBFreeAlloc(&wb->wb_BobMem,sizeof(struct BobNode),MEMF_PUBLIC|MEMF_CLEAR);

    if( ! bn ) {
	/* stop the list search */
	return( 1 );
    }

    GetImageMode( obj, (APTR *)&im, &mode, (APTR *)&mask );
    DP(("AddWbBob: obj=$%lx (%s), after GetImageMode:\n", obj, obj->wo_Name));
    DP(("\tim=$%lx, mode=$%lx, mask=$%lx\n", im, mode, mask));
    if( ! (bn->bn_Bob = makeBob( im, mode, mask )) ) {
	/* stop the list search */
	return( 1 );
    }

    /* set the initial position of the bob */
    vs = bn->bn_Bob->BobVSprite;
    win = obj->wo_IconWin;

#ifdef DEBUGGING
    if (!win) {
	DP(("AddWbBob: NULL wo_IconWin, obj 0x%lx/%s\n", obj, obj->wo_Name));
	Debug(0L);
    }
#endif DEBUGGING

    vs->X = win->LeftEdge + obj->wo_Gadget.LeftEdge;
    vs->Y = win->TopEdge + obj->wo_Gadget.TopEdge;
    if (dd->dd_ViewModes <= DDVM_BYICON) {
	vs->X += wb->wb_EmboseBorderLeft;
	vs->Y += wb->wb_EmboseBorderTop;
    }
    else {
	vs->X += VIEWBYTEXTLEFTOFFSET;
    }

    bn->bn_Obj = obj;
    bn->bn_Node.ln_Name = obj->wo_Name;

    ADDTAIL( &wb->wb_BobList, (struct Node *)bn );

    AddBob( bn->bn_Bob, &screen->RastPort );

    DP(("AddWbBob: exit\n"));
#ifdef MEMDEBUG
    PrintAvailMem("AddWbBob", MEMF_CHIP);
#endif MEMDEBUG
    return( 0 );
}

RemWbBob( bn, screen )
struct BobNode *bn;
struct Screen *screen;
{
    RemIBob( bn->bn_Bob, &screen->RastPort, &screen->ViewPort );
    return( 0 );
}

void RedrawBox(msg)
struct IntuiMessage *msg;
{
struct WorkbenchBase *wb = getWbBase();

    if (msg)
    {
	/* erase old */
	DrawBox();

	/* update mouse last posn */
	wb->wb_LastX = msg->IDCMPWindow->LeftEdge + msg->MouseX;
	wb->wb_LastY = msg->IDCMPWindow->TopEdge + msg->MouseY;

	/* Check if we are still on the screen! */
	if (wb->wb_LastX < 0) wb->wb_LastX=0;
	else if ((wb->wb_LastX) > (wb->wb_Screen->Width-1)) wb->wb_LastX=wb->wb_Screen->Width-1;

	if (wb->wb_LastY < 0) wb->wb_LastY=0;
	else if ((wb->wb_LastY) > (wb->wb_Screen->Height-1)) wb->wb_LastY=wb->wb_Screen->Height-1;

	/* draw new */
	DrawBox();
    }
}

void SetDragSelect(struct IntuiMessage *msg)
{
struct WorkbenchBase *wb = getWbBase();

	DP(("SetDragSelect: enter\n"));
	wb->wb_Dragging = 1;
	wb->wb_DragSelect = 1;
	SetDrMd(&wb->wb_Screen->RastPort, COMPLEMENT);
	SetAPen(&wb->wb_Screen->RastPort, ~0);
	SetDrPt(&wb->wb_Screen->RastPort, 0xff00);
	wb->wb_XOffset=wb->wb_LastX=msg->IDCMPWindow->LeftEdge + msg->MouseX;
	wb->wb_YOffset=wb->wb_LastY=msg->IDCMPWindow->TopEdge + msg->MouseY;
	DrawBox();
	DP(("SetDragSelect: exit\n"));
}

void ClearDragSelect(void)
{
struct WorkbenchBase *wb = getWbBase();

	wb->wb_Dragging = 0;
	wb->wb_DragSelect = 0;
	DrawBox();
	/* restore draw mode */
	SetDrMd(&wb->wb_Screen->RastPort, wb->wb_DrawMode);
	SetDrPt(&wb->wb_Screen->RastPort, ~0); /* reset to solid pattern */
	SetAPen(&wb->wb_Screen->RastPort, wb->wb_APen); /* reset APen */
}

void SetMoving(void)
{
struct WorkbenchBase *wb = getWbBase();

    wb->wb_Dragging = 1;

    NewList( &wb->wb_BobList );
    wb->wb_BobMem.fl_NumFree = 0;
    NewList( &wb->wb_BobMem.fl_MemList );

    wb->wb_LastX = wb->wb_XOffset;
    wb->wb_LastY = wb->wb_YOffset;
    SelectSearch( AddWbBob, wb->wb_Screen );

    RedrawBobs( 0 );
}

void ClearMoving(void)
{
struct WorkbenchBase *wb = getWbBase();

	wb->wb_Dragging = 0;
	SearchList(&wb->wb_BobList, RemWbBob, wb->wb_Screen);
	RedrawBobs(0);
	FreeFreeList(&wb->wb_BobMem);
}

void Redraw(struct IntuiMessage *msg)
{
struct WorkbenchBase *wb = getWbBase();

	if (wb->wb_DragSelect) RedrawBox(msg);
	else if (wb->wb_Dragging) RedrawBobs(msg);
}

BOOL CheckSortRect(struct Rectangle *);

void ClearDragging(msg, flag)
struct IntuiMessage *msg;
int flag; /* 0-just cancelling, 1-ending, do operation */
{
struct WorkbenchBase *wb = getWbBase();
struct Window *window=msg->IDCMPWindow;

	DP(("ClearDragging: enter, msg=$%lx, flags=$%lx\n", msg, flag));
	DP(("\tDragging=%ld, DragSelect=%ld\n",wb->wb_Dragging, wb->wb_DragSelect));

	if (flag)
	{
		flag=0;
		if (wb->wb_DragSelect) flag=1;
		else if (wb->wb_Dragging) flag=2;
	}

	if (wb->wb_Dragging)
	{
		AbortLayerDemon();

		if (wb->wb_DragSelect) ClearDragSelect();
		else ClearMoving();

		UnlockLayers(&wb->wb_Screen->LayerInfo);
	}

	/*
	 * undo the current window...
	 */
	wb->wb_CurrentWindow=NULL;

	ModifyIDCMP(window, window->IDCMPFlags & ~(MOUSEMOVE|MENUVERIFY|INTUITICKS));

	if (flag==1)
	{
	struct	Rectangle	br;

		br.MinX=wb->wb_XOffset;
		br.MinY=wb->wb_YOffset;
		br.MaxX=wb->wb_LastX;
		br.MaxY=wb->wb_LastY;

		/*
		 * Sort and check the rectangle for minimum size
		 */
		if (CheckSortRect(&br))
		{
			/*
			 * prevent the click which started the drag selecting
			 * from being the first click of a double-click
			 */
			wb->wb_Tick.tv_secs = 0;
			/* select anybody who is in the box */
			BeginDiskIO();
			MasterSearch(DragSelect,&br);
			EndDiskIO();
		}
	}
	else if (flag==2) Matrix(msg);

	DP(("ClearDragging: exit\n"));
}

void SetDragging(msg, flag)
struct IntuiMessage *msg;
int flag; /* 0-icon drag, 1-drag select */
{
struct WorkbenchBase *wb = getWbBase();
struct Window *window=msg->IDCMPWindow;

	ModifyIDCMP(window,window->IDCMPFlags|MOUSEMOVE|MENUVERIFY|INTUITICKS);
	/*
	 * Set up the current window for dragging
	 */
	wb->wb_CurrentWindow=window;

	LockLayers(&wb->wb_Screen->LayerInfo);

	if (flag) SetDragSelect(msg);
	else SetMoving();

	PostLayerDemon();
}
