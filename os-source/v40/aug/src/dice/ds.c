/** dropshadow.c **/

/* this program copyright 1987, james mackraz.  may not be distributed
 * for profit.  copies of the source may be made for not-for-profit
 * distribution, but must include this notice.
 *
 * james mackraz, 4021 Second Street, Palo Alto, CA, 94306
 */

#include "ds.h"
#include <tools/debug.h>

#define printf  kprintf


/* hacked up for testing cliprect dicing	*/
#define DICE	1



#if DICE
UBYTE	wtitle[128];
#else
UBYTE	*wtitle	= (UBYTE *) "  DropShadow -- ver 2, rev 0  ";
#endif
struct  Window      *getNewWind();

struct RastPort	*rport2 = NULL;	/* for rendering into bmap2	*/
struct Window   *window = NULL;

struct	Task	*mytask;
UBYTE			mysig = 0;

struct  IntuitionBase   *IntuitionBase;
struct  GfxBase         *GfxBase;
struct	Library		*LayersBase;

ULONG   flg = ACTIVATE | WINDOWCLOSE | NOCAREREFRESH | WINDOWDRAG
	    | WINDOWDEPTH | SIMPLE_REFRESH;

ULONG   iflg = MOUSEMOVE | CLOSEWINDOW | GADGETDOWN | GADGETUP;

/* for each layer, front to back, not incl. backdrops	*/
#define FORLAYERS(l, linfo)	for (l = (linfo)->top_layer; \
	 l && !(l->Flags & LAYERBACKDROP);l=l->back)

#define BACKTOFRONT(l)	 for (l = window->WLayer->LayerInfo->top_layer;	\
l && l->back; l = l->back) ; \
for (; l; l = l->front)

#define PREDLAYERS(pred, l) for (pred = l->front; pred; pred = pred->front)

#define MYPRI			 7L	/* a little advantage						*/
#define INPUTPRI_PLUS	25L	/* i guess i could findtask("input.device")	*/

USHORT	hdrop = 7;
USHORT	vdrop = 3;

main()
{
    struct  IntuiMessage    *msg;

    /* hold data from *msg  */
    ULONG   class;
    UBYTE   code;
	struct Gadget *gaddress;

    struct Screen	*wbscreen;
	struct BitMap	*save_bitmap= NULL;	/* original WB RasInfo bitmap	*/
	struct BitMap	*ribitmap	= NULL;		/* my 3-deep replacement		*/
    struct BitMap  	*bmap2		= NULL;

	int		i;
    int		it_is_done = 0;		/* success flag			*/
    WORD	exitval = 0;
	ULONG	sigmask;
	ULONG	sigtaken;

	extern struct PropInfo twxtinfo;
	USHORT	twxtpot;

    if (!(IntuitionBase = (struct IntuitionBase *)
		OpenLibrary("intuition.library", 0L)))
    {
		D( printf("NO INTUITION LIBRARY\n") );
		exitval = 1;
		goto EXITING;
    }

    if (!(GfxBase = (struct GfxBase *) OpenLibrary("graphics.library", 0L)))
    {
		D( printf("NO GRAPHICS LIBRARY\n") );
		exitval = 2;
		goto EXITING;
    }

    if (!(LayersBase=(struct Library *) OpenLibrary("layers.library", 0L)))
    {
		D( printf("NO LAYERS LIBRARY\n") );
		exitval = 2;
		goto EXITING;
    }

    /* get a window on the workbench	*/
    window = getNewWind(120, 20, 400, 30, flg, iflg);
    if (window == NULL)
    {
		D( printf("test: can't get window.\n") );
		exitval = 1;
		goto EXITING;
    }

    /* Add bitplane to the Workbench, as far as ViewPort is concerned	*/

    wbscreen = window->WScreen;		/* find it	*/

	/* new 3-deep bitmap will replace the one in WB's RInfo	*/
    if (!(ribitmap = (struct BitMap *)
		AllocMem((LONG) sizeof(struct BitMap), (LONG) MEMF_PUBLIC|MEMF_CLEAR)))
    {
		D( printf("alloc bitmap failed\n") );
		goto EXITING;
    }
    InitBitMap(ribitmap, 3L, (LONG) wbscreen->Width, (LONG) wbscreen->Height);

    /* allocate bitmap for my rastport view of single bitplane	*/
    if (!(bmap2 = (struct BitMap *)
		AllocMem((LONG) sizeof(struct BitMap), (LONG) MEMF_PUBLIC|MEMF_CLEAR)))
    {
		D( printf("alloc bitmap failed\n") );
		goto EXITING;
    }

	/* my rendering bit map: depth 1	*/
    InitBitMap(bmap2, 1L, (LONG) wbscreen->Width, (LONG) wbscreen->Height);

    if (!(bmap2->Planes[0] =
		(UBYTE *) AllocRaster((LONG) wbscreen->Width, (LONG) wbscreen->Height)))
    {
		D( printf("alloc raster failed\n") );
		goto EXITING;
    }

    /* get a rastport, and set it up for rendering into bmap2	*/
    if (!(rport2 = (struct RastPort *)
		AllocMem((LONG) sizeof (struct RastPort), (LONG) MEMF_PUBLIC)))
    {
		D( printf("alloc rastport failed\n") );
		goto EXITING;
    }
    InitRastPort(rport2);
    rport2->BitMap = bmap2;

    SetRast(rport2, 0L);

	/* set up new bitmap for RInfo	*/
	save_bitmap =  wbscreen->ViewPort.RasInfo->BitMap;
	ribitmap->Planes[0] = save_bitmap->Planes[0];
	ribitmap->Planes[1] = save_bitmap->Planes[1];
	ribitmap->Planes[2] = bmap2->Planes[0];

    Forbid();

	/* add our extra plane to the bit-map as viewed by
	 * the WB ViewPort (WB should never know the difference)
	 */
	wbscreen->ViewPort.RasInfo->BitMap = ribitmap;

    Permit();

	it_is_done = 1;

	shadowColors(wbscreen);

    /* put viewport changed into effect	*/
    MakeScreen(wbscreen);
    RethinkDisplay();

	twxtpot = twxtinfo.HorizPot;
	hdrop = twxtpot >> TWXTSHIFT;
	vdrop = twxtpot >> (TWXTSHIFT + 1);
    drawShadow();

	/* hook into the library vectors	*/
	mytask = FindTask(0L);
	mysig  = AllocSignal((LONG) -1);
	sigmask = ((LONG) 1 << mysig);
	sigmask |= ((LONG) 1 << window->UserPort->mp_SigBit);
	setup_hooks();

	/* a little help	*/
	SetTaskPri(mytask, MYPRI);

    FOREVER
    {
		if ((msg = (struct IntuiMessage *)GetMsg(window->UserPort)) == NULL)
		{
			sigtaken = Wait(sigmask);
		    if (sigtaken & ((LONG) 1 << mysig))
			{
				drawShadow();
			}
		    continue;
		}

		class   = msg->Class;
		code	= msg->Code;
		gaddress  = (struct Gadget *) msg->IAddress;
		ReplyMsg(msg);

		switch (class)
		{
		case MOUSEMOVE:		/* taken at higher priority	*/
			shadowColors(wbscreen);
			break;

		case GADGETDOWN:
			SetTaskPri(mytask, INPUTPRI_PLUS);
			break;

		case GADGETUP:
			SetTaskPri(mytask, MYPRI);

			switch (gaddress->GadgetID)
			{
			case TWIXTID:
				twxtpot = twxtinfo.HorizPot;
				hdrop = twxtpot >> TWXTSHIFT;
				vdrop = twxtpot >> (TWXTSHIFT + 1);
				drawShadow();
				break;
			case DARKID:
				shadowColors(wbscreen);
				break;
			}
			break;

		case CLOSEWINDOW:
			SetTaskPri(mytask, 0L);
			if (cleanup_hooks()) goto EXITING;
		default:
		    D( printf("unknown event: class %lx\n", class) );
		}
    }

EXITING:
    /* clean up trick	*/
    if (it_is_done)
    {
		Forbid();
		wbscreen->ViewPort.RasInfo->BitMap = save_bitmap;
		Permit();
		MakeScreen(wbscreen);
		RethinkDisplay();
    }

	if (mysig) FreeSignal((LONG) mysig);

    if (rport2) FreeMem(rport2, (LONG) sizeof (struct RastPort));
    if (bmap2) 
    {
		if (bmap2->Planes[0])
		{
		    FreeRaster(bmap2->Planes[0],
		    	(LONG) wbscreen->Width, (LONG) wbscreen->Height);
		}
		FreeMem(bmap2, (LONG) sizeof (struct BitMap));
    }
	if (ribitmap) FreeMem(ribitmap, (LONG) sizeof (struct BitMap));

    if (window) CloseWindow(window);
    if (GfxBase) CloseLibrary(GfxBase);
    if (LayersBase) CloseLibrary(LayersBase);
    if (IntuitionBase) CloseLibrary(IntuitionBase);

    exit (exitval);
}

/* returns true if this layer casts a shadow
 * note that menu layer does not cast a shadow
 */
shadowCaster(l)
register struct Layer *l;
{
	extern struct PropInfo darkinfo;
	register struct Window		*w;
	register struct RastPort	*brp;

	if (l && (w = (struct Window *) l->Window))
	{
		/* must be border layer for GZZ window	*/
		if (brp = w->BorderRPort)
		{
			/* GZZ	*/
			return (brp->Layer == l);
		}
		else
		{
			/* non-GZZ	*/
			return (l == w->WLayer);
		}
	}
	return (0);
}

/* called by my replacement layers vectors	*/
drawShadowSignal(sysret)
LONG	sysret;
{
	Signal( mytask, ((LONG) 1 << mysig));
	return (sysret);
}



#if DICE		/* hack up this version to show cliprect dicing	*/

drawShadow( sysret )
LONG	sysret;
{
	struct Layer	*l;
	struct ClipRect	*cr;
	int		layerleft;
	int		layertop;

	LockLayerInfo(window->WLayer->LayerInfo);

	SetRast(rport2, 0L);

	SetAPen(rport2, 1L);

	for ( l = window->WLayer->LayerInfo->top_layer; l; l = l->back )
	{
		layerleft = l->bounds.MinX;
		layertop = l->bounds.MinY;

		for ( cr = l->ClipRect; cr; cr = cr->Next )
		{
			/* drawOffsetRect( rport2, &cr->bounds, layerleft, layertop ); */
			if ( cr->lobs ) drawOffsetRect( rport2, &cr->bounds, 0, 0 );
		}
	}

	UnlockLayerInfo(window->WLayer->LayerInfo);
}

drawOffsetRect( rp, rect, x, y )
struct RastPort		*rp;
struct Rectangle	*rect;
{
	Move( rp, (LONG) x + rect->MinX, (LONG) y + rect->MinY );
	Draw( rp, (LONG) x + rect->MaxX, (LONG) y + rect->MinY );
	Draw( rp, (LONG) x + rect->MaxX, (LONG) y + rect->MaxY );
	Draw( rp, (LONG) x + rect->MinX, (LONG) y + rect->MaxY );
	Draw( rp, (LONG) x + rect->MinX, (LONG) y + rect->MinY );
}

#else

/* run on my own task's schedule	*/
drawShadow(sysret)
LONG	sysret;
{
	struct Region	*shadow;

	if ( !(shadow = NewRegion()) ) return (sysret);

	bigShadow(shadow);

	SetRast(rport2, 0L);

	Forbid();
	WaitBOVP(&window->WScreen->ViewPort);

	SetAPen(rport2, 1L);
	fillRegion(rport2, shadow);
	Permit();

	DisposeRegion(shadow);
}

/* you want speed? speed this up.	*/
bigShadow(rgn)
struct	Region	*rgn;
{
	register struct Layer	*pred;	/* pred to 'l'						*/
	register LONG			tmprgn;	/* accumulates shadow on layer 'l'	*/
	register struct Layer	*l;		/* create all shadows on each layer	*/

	int				twixt;		/* number of "spaces" between l, pred	*/
	int				htwixt;
	int				vtwixt;
	struct Rectangle lrect;
	struct Rectangle predrect;

	if ( !(tmprgn = (LONG) NewRegion()) ) return;

	/* protect the integrity of the layer list	*/
	LockLayerInfo(window->WLayer->LayerInfo);

	BACKTOFRONT(l)
	{
		/* could eliminate any layers you don't want a shadow on here.	*/

		/* layer rectangle	*/
		lrect = l->bounds;

#if 0
		notRectRegion(rgn, &lrect);	/* l obscures shadows behind it		*/
#else
		ClearRectRegion(rgn, &lrect);	/* dale's version				*/
#endif

#if OLDTWIXT
		twixt = 0;
#else
		htwixt = hdrop << 1;
		vtwixt = vdrop << 1;
#endif
		ClearRegion(tmprgn);		/* will build shadow on single layer */

		/* walk through shadow casters	*/
		PREDLAYERS(pred, l)
		{
			/* menu bar doesn't cast shadow	*/
			if (!shadowCaster(pred)) 
			{
				continue;
			}

			/* rectangle will cast shadow				*/
			predrect = pred->bounds;	/* layer rect	*/

			/* shadow offset increases with depth		*/
#if OLDTWIXT
			translateRect(&predrect,
				HDROP + twixt * (HDROP >> 1), VDROP + twixt * (VDROP >> 1));

			++twixt;				/* another level up					*/
#else
			translateRect(&predrect, htwixt, vtwixt);
			htwixt += hdrop;
			vtwixt += vdrop;

#endif
			/* add it to accumulated shadow on 'l'		*/
			OrRectRegion(tmprgn, &predrect);
		}

		/* all shadows on 'l' should actually be on 'l'	*/
		AndRectRegion(tmprgn, &lrect);

		/* add shadows on l to shadows on layers behind l	*/
		OrRegionRegion(tmprgn, rgn);
	}

	UnlockLayerInfo(window->WLayer->LayerInfo);
	DisposeRegion(tmprgn);
}

#endif

/* can use dale's ClearRectRegion	*/
#if 0
notRectRegion(rgn, rect)
struct Region		*rgn;
struct Rectangle	*rect;
{
	OrRectRegion(rgn, rect);
	XorRectRegion(rgn, rect);
}
#endif

/* this doesn't get use anymore	*/
#define GOODRECT(r)	(((r)->MinX < (r)->MaxX) && ((r)->MinY < (r)->MaxY))

fillRegion(rp, rgn)
struct RastPort	*rp;
struct Region	*rgn;
{
	register struct	RegionRectangle	*rgnrect;
	register struct	Rectangle	*bds;

	register int		minx;		/* offsets for all region rectangles	*/
	register int		miny;

	minx = rgn->bounds.MinX;
	miny = rgn->bounds.MinY;
	rgnrect = rgn->RegionRectangle;

	while (rgnrect)
	{
		bds = &rgnrect->bounds;
		RectFill(rp,
			(LONG)minx + bds->MinX,
			(LONG) miny + bds->MinY, 
			(LONG) minx + bds->MaxX,
			(LONG) miny + bds->MaxY);

		rgnrect = rgnrect->Next;
	}
}

translateRect(r, dx, dy)
register struct	Rectangle *r;
register int		dx;
register int		dy;
{
	r->MinX	+= dx;
	r->MaxX	+= dx;
	r->MinY	+= dy;
	r->MaxY	+= dy;
}

screenRect(s, r)
register struct Screen		*s;
register struct Rectangle	*r;
{
	r->MaxX = (r->MinX = s->LeftEdge) + s->Width - 1;
	r->MaxY = (r->MinY = s->TopEdge) + s->Height - 1;
}

windowRect(w, r)
register struct Window		*w;
register struct Rectangle	*r;
{
	r->MaxX = (r->MinX = w->LeftEdge) + w->Width - 1;
	r->MaxY = (r->MinY = w->TopEdge) + w->Height - 1;
}

struct  Window * getNewWind(left, top, width, height, flg, iflg)
SHORT   left, top, width, height;
ULONG   flg, iflg;
{
	extern struct Gadget darkness;

    struct  Window  *OpenWindow();
    struct  NewWindow   nw;

    nw.LeftEdge =   (SHORT) left;
    nw.TopEdge  =   (SHORT) top;
    nw.Width    =   (SHORT) width;
    nw.Height   =   (SHORT) height;
    nw.DetailPen    =   (UBYTE) -1;
    nw.BlockPen =   (UBYTE) -1;
    nw.IDCMPFlags   =   (ULONG) iflg;

    nw.Flags    =   (ULONG) flg;

    nw.FirstGadget  =   (struct Gadget *)   &darkness;
    nw.CheckMark    =   (struct Image *)    NULL;
#if DICE
    sprintf( wtitle, "  Dice Display (LAYERS V%d) ", LayersBase->lib_Version );
    nw.Title    =   (UBYTE *)   wtitle;
#else
    nw.Title    =   (UBYTE *)   wtitle;
#endif
    nw.Screen   =   (struct Screen *)   NULL;
    nw.BitMap   =   (struct BitMap *)   NULL;
    nw.MinWidth =   (SHORT) 50;
    nw.MinHeight=   (SHORT) 30;
    /* work around bug  */
    nw.MaxWidth =   (SHORT) nw.Width;
    nw.MaxHeight    =   (SHORT) nw.Height;
    nw.Type     =   (USHORT) WBENCHSCREEN;

    return ((struct Window *) OpenWindow(&nw));
}


#define DARKER(c)  (((long) (((c) & 0xF) * (darkpot)) ) >> 16)

shadowColors(s)
struct Screen *s;
{
	register int	i;
	register ULONG	rgb;
	ULONG	red, green, blue;
	struct	ColorMap	*cm;
	USHORT	darkpot;

	darkpot = darkinfo.HorizPot;

	cm = s->ViewPort.ColorMap;

	for (i = 0; i < 4; ++i)
	{
		/* get three colors	*/
		rgb		= GetRGB4(cm, (LONG) i);

#if	0
		rgb = ~rgb;	/* wild colors, not wild enough	*/
#endif

		blue	= DARKER(rgb);
		green	= DARKER(rgb >>= 4);
		red		= DARKER(rgb >> 4);
	    SetRGB4(&s->ViewPort, (LONG) i + 4,(LONG) red,(LONG) green,(LONG) blue);
	}
}
