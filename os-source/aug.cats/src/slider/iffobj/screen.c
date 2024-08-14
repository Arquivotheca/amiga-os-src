/* screen.c - 2.0 screen module for Display
 * based on scdemo, oscandemo, looki
 * 10/91 - added public screen support by checking user screen tags
 *         for SA_PubScreen and setting screen public if found.
 * 09/92 - added V39 BestFit call for modefallback, and usermodeid support
 * 39.10 - cast Detail and Block pen to UBYTE, cast penarray value to UWORD
 * 39.11 - Add NoCenter arg to clipit()
 */

/*
Copyright (c) 1989-1993 Commodore-Amiga, Inc.

Executables based on this information may be used in software
for Commodore Amiga computers. All other rights reserved.
This information is provided "as is"; no warranties are made.
All use is at your own risk, and no liability or responsibility
is assumed.
*/

#define INTUI_V36_NAMES_ONLY

#include "iffp/ilbmapp.h"

BOOL   VideoControlTags(struct ColorMap *,ULONG tags, ...);

extern struct Library *GfxBase;
extern struct Library *IntuitionBase;

struct TextAttr SafeFont = { (UBYTE *) "topaz.font", 8, 0, 0, };
UWORD  penarray[] = {(UWORD)~0};

/* default new window if none supplied in ilbm->nw */
struct   NewWindow      defnw = {
   0, 0,                                  /* LeftEdge and TopEdge */
   0, 0,                          	  /* Width and Height */
   (UBYTE)-1, (UBYTE)-1,                  /* DetailPen and BlockPen */
   IDCMP_VANILLAKEY | IDCMP_MOUSEBUTTONS, /* IDCMP Flags with Flags below */
   WFLG_BACKDROP | WFLG_BORDERLESS |
   WFLG_SMART_REFRESH | WFLG_NOCAREREFRESH |
   WFLG_ACTIVATE | WFLG_RMBTRAP,
   NULL, NULL,                            /* Gadget and Image pointers */
   NULL,                                  /* Title string */
   NULL,                                  /* Screen ptr null till opened */
   NULL,                                  /* BitMap pointer */
   50, 20,                                /* MinWidth and MinHeight */
   0 , 0,                                 /* MaxWidth and MaxHeight */
   CUSTOMSCREEN                           /* Type of window */
   };


/* opendisplay - passed ILBMInfo, dimensions, modeID
 *
 *    Attempts to open correct 2.0 modeID screen and window,
 *    else an old 1.3 mode screen and window.
 *
 * Returns *window or NULL.
 */

struct Window *opendisplay(struct ILBMInfo *ilbm,
			   SHORT wide, SHORT high, SHORT deep,
			   ULONG mode)
    {
    struct NewWindow newwin, *nw;

    closedisplay(ilbm);
    if(ilbm->scr = openidscreen(ilbm, wide, high, deep, mode))
	{
	nw = &newwin;
	if(ilbm->windef) *nw = *(ilbm->windef);
	else *nw = *(&defnw);
	nw->Screen	= ilbm->scr;

	D(bug("sizes: scr= %ld x %ld  passed= %ld x %ld\n",
		ilbm->scr->Width,ilbm->scr->Height,wide,high)); 

	nw->Width	= wide;
	nw->Height	= high;
	if (!(ilbm->win = OpenWindow(nw)))
	    {
	    closedisplay(ilbm);
	    D(bug("Failed to open window."));
	    }
	else
	    {
	    if(ilbm->win->Flags & WFLG_BACKDROP)
		{
		ShowTitle(ilbm->scr, FALSE);
		ilbm->TBState = FALSE;
		}
	    }
	}

    if(ilbm->scr)	/* nulled out by closedisplay if OpenWindow failed */
	{
	ilbm->vp  = &ilbm->scr->ViewPort;
	ilbm->srp = &ilbm->scr->RastPort;
	ilbm->wrp = ilbm->win->RPort;
	}
    return(ilbm->win);
    }


/* Under 2.0, returns result of CloseScreen (TRUE for successful close)
 * Under 1.3, always returns TRUE
 */
BOOL closedisplay(struct ILBMInfo *ilbm)
    {
    extern struct Library *IntuitionBase;
    BOOL temp, result = TRUE;

    if(ilbm)
	{
	if(ilbm->scr)	ScreenToBack(ilbm->scr);
    	if(ilbm->win)	CloseWindow(ilbm->win), ilbm->win=NULL, ilbm->wrp=NULL;
	if(ilbm->scr)
	    {
	    if(ilbm->scr->Flags & PUBLICSCREEN)
		{
		if(((struct Library *)IntuitionBase)->lib_Version >= 36)
			PubScreenStatus(ilbm->scr, PSNF_PRIVATE);
		}
	    temp = CloseScreen(ilbm->scr);
	    if(IntuitionBase->lib_Version >= 36)  result = temp;
	    if(result)
		{
		ilbm->scr = NULL;	
	    	ilbm->vp  = NULL;
    		ilbm->srp = NULL;
		}
	    }
	}
    return(result);
    }



/* openidscreen - ILBMInfo, dimensions, modeID
 *
 *    Attempts to open correct 2.0 modeID screen with centered
 *    overscan based on user's prefs,
 *    else old 1.3 mode screen.
 *
 * If ilbm->stype includes CUSTOMBITMAP, ilbm->brbitmap will be
 *   	used as the screen's bitmap.
 * If ilbm->stags is non-NULL, these tags will be added to the
 *	end of the taglist.
 *
 * If ilbm->IFFPFlags IFFPF_USERMODE, use ilbm->usermodeid
 * If ilbm->IFFPFlags IFFPF_BESTFIT, call modefallback()
 *
 * Returns *screen or NULL.
 */

struct Screen *openidscreen(struct ILBMInfo *ilbm,
			    SHORT wide, SHORT high, SHORT deep,
			    ULONG mode)
    {
    struct NewScreen ns;			/* for old style OpenScreen */
    DisplayInfoHandle displayhandle;
    struct DimensionInfo dimensioninfo;
    struct Rectangle spos, dclip, txto, stdo, maxo, uclip;  /* display rectangles */
    struct Rectangle *uclipp;
    struct Screen   *scr = NULL;
    LONG   error, trynew;
    ULONG  bitmaptag, passedtags;
    BOOL   vctl;

    if(!ilbm)	return(0L);

    if(ilbm->IFFPFlags & IFFPF_USERMODE)	mode = ilbm->usermodeid;
    if(ilbm->IFFPFlags & IFFPF_BESTFIT)
	{
	mode = modefallback(mode,wide,high,deep);
	}

    if (trynew = ((((struct Library *)GfxBase)->lib_Version >= 36)&&
          (((struct Library *)IntuitionBase)->lib_Version >= 36)))
	{
    	/* if >= v36, see if mode is available */
	if(error = ModeNotAvailable(mode))
	    {
	    D(bug("Mode $%08lx not available, error=%ld:\n",mode,error));
	    /* if not available, try fall back mode */
	    mode = modefallback(mode,wide,high,deep);
	    error = ModeNotAvailable(mode);

	    D(bug("$%08lx ModeNotAvailable=%ld:\n",mode,error));
	    }


	if(!error)	/* mode is available - how about depth ? */
	    {
            if(displayhandle=FindDisplayInfo(mode))
		{
            	if(GetDisplayInfoData(displayhandle,(UBYTE *) &dimensioninfo,
        		sizeof(struct DimensionInfo),DTAG_DIMS,NULL))
		    {
		    if(dimensioninfo.MaxDepth < deep)
			{
			message("%s: %ld %s\n",
				ilbm->ParseInfo.filename,
				ilbm->Bmhd.nPlanes,SI(MSG_ILBM_TOODEEP));
			deep = dimensioninfo.MaxDepth;
			}
		    }
		}
	    }

	if(error) trynew = FALSE;
	else trynew=((QueryOverscan(mode,&txto,OSCAN_TEXT))&&
			(QueryOverscan(mode,&stdo,OSCAN_STANDARD))&&
			    (QueryOverscan(mode,&maxo,OSCAN_MAX)));
	}

    D(bug("\nILBM: w=%ld, h=%ld, d=%ld, mode=0x%08lx\n",
		wide,high,deep,mode));	
    D(bug("OPEN: %s.\n",
        trynew	? "Is >= 2.0 and mode available, trying OpenScreenTags"
		: "Not 2.0, doing old OpenScreen"));

    if(trynew)
	{
	/* If user clip type specified and available, use it */
	if(ilbm->Video) ilbm->ucliptype = OSCAN_VIDEO;
	if((ilbm->ucliptype)&&(QueryOverscan(mode,&uclip,ilbm->ucliptype)))
		uclipp = &uclip;
	else uclipp = NULL;

	clipit(wide,high,&spos,&dclip,&txto,&stdo,&maxo,uclipp,
		ilbm->IFFPFlags & IFFPF_NOCENTER ? TRUE : FALSE);

	D(bug("Using dclip  %ld,%ld  to  %ld,%ld... width=%ld height=%ld\n",
			dclip.MinX,dclip.MinY,dclip.MaxX,dclip.MaxY,
			dclip.MaxX-dclip.MinX+1,dclip.MaxY-dclip.MinY+1));
	D(bug("spos->minx = %ld, spos->miny = %ld\n",spos.MinX,spos.MinY));
	D(bug("DEBUG: About to attempt OpenScreenTags\n"));

	bitmaptag = ((ilbm->brbitmap)&&(ilbm->stype & CUSTOMBITMAP)) ?
		SA_BitMap : TAG_IGNORE;
	passedtags = ilbm->stags ? TAG_MORE : TAG_IGNORE;

	scr=(struct Screen *)OpenScreenTags((struct NewScreen *)NULL,
		SA_DisplayID,	mode,
		SA_Type,	ilbm->stype,
		SA_Behind,	TRUE,
		SA_Top,		spos.MinY,
		SA_Left,	spos.MinX,
		SA_Width,	wide,
		SA_Height,	high,
		SA_Depth,	deep,
		SA_DClip,	&dclip,
		SA_AutoScroll,	ilbm->Autoscroll ? TRUE : FALSE,
		SA_Title,	ilbm->stitle,
		SA_Font,	&SafeFont,
		SA_Pens,	penarray,
		SA_ErrorCode,	&error,
		bitmaptag,	ilbm->brbitmap,
		passedtags,	ilbm->stags,
		TAG_DONE
		);

	    D(bug("DEBUG: OpenScreenTags scr at 0x%lx\n",scr));

	    if(scr)
		{
		/* If caller specified a public screen, open for business */
		if(scr->Flags & PUBLICSCREEN)
		    {
		    if(((struct Library *)IntuitionBase)->lib_Version >= 36)
				PubScreenStatus(scr,NULL);
		    }
		if(ilbm->Notransb)
		    {
		    vctl=VideoControlTags(scr->ViewPort.ColorMap,
				VTAG_BORDERNOTRANS_SET, TRUE,
				TAG_DONE);

	D(bug("VideoControl to set bordernotrans, error = %ld\n",vctl));

		    MakeScreen(scr);
		    RethinkDisplay();
		    }
		}
	    else message("%s\n",openScreenErr(error));
        }

    if(!scr)
	{
	/* ns initialization for 1.3 old style OpenScreen only
         */
	ns.LeftEdge = ns.TopEdge = 0;   
	ns.Width 	=	wide;
	ns.Height 	=	high;
	ns.Depth	=	deep;
	ns.ViewModes	= 	modefallback(mode,wide,high,deep);
	ns.DetailPen	=	0;
	ns.BlockPen	=	1;
	ns.Gadgets	=	NULL;
	ns.CustomBitMap	=	((ilbm->brbitmap)&&(ilbm->stype & CUSTOMBITMAP))
					? ilbm->brbitmap : NULL;
	ns.Font		=	&SafeFont;
	ns.DefaultTitle = 	ilbm->stitle;
	ns.Type		=	ilbm->stype & 0x01FF;  /* allow only 1.3 types */

	scr=(struct Screen *)OpenScreen(&ns);

	D(bug("DEBUG: ns.ViewModes=0x%lx, vp->Modes=0x%lx\n",
                 ns.ViewModes,scr->ViewPort.Modes));
	D(bug("DEBUG: non-extended scr at 0x%lx (0=failure)\n",scr));
	}
    return(scr);
    }


/*
 * modefallback - passed a mode id, attempts to provide a
 *                suitable old mode to use instead
 */

/* for old 1.3 screens */
#define MODE_ID_MASK (LACE|HIRES|HAM|EXTRA_HALFBRITE)

ULONG modefallback(ULONG oldmode, SHORT wide, SHORT high, SHORT deep)
{
    ULONG newmode, bestmode;
    struct TagItem tags[6];

    if(GfxBase->lib_Version >= 39)
	{
	tags[0].ti_Tag = BIDTAG_DIPFMustHave;
	tags[0].ti_Data = (oldmode & HAM ? DIPF_IS_HAM : 0);
	tags[0].ti_Data |= (oldmode & EXTRA_HALFBRITE ? DIPF_IS_EXTRAHALFBRITE : 0);
	tags[1].ti_Tag = BIDTAG_NominalWidth;
	tags[1].ti_Data = wide; //bmhd->XAspect; //wide;
	tags[2].ti_Tag = BIDTAG_NominalHeight;
	tags[2].ti_Data = high; //bmhd->YAspectHeight; //high;
	tags[3].ti_Tag = BIDTAG_Depth;
	tags[3].ti_Data = deep;
	tags[4].ti_Tag = BIDTAG_SourceID;
	tags[4].ti_Data = (FindDisplayInfo(oldmode) ? oldmode : (oldmode & (~(MONITOR_ID_MASK | SPRITES|GENLOCK_AUDIO|GENLOCK_VIDEO|VP_HIDE))));
	tags[5].ti_Tag = TAG_DONE;

	if((bestmode = BestModeIDA(tags)) != INVALID_ID)
	    {
	    newmode = bestmode;
	    }
	D(bug("Best fit ID = 0x%lx\n", newmode));
	}
    else
	{
    	newmode = oldmode & MODE_ID_MASK;
    	D(bug("Falling back to 0x%08lx instead of 0x%08lx\n",newmode,oldmode));
	}	
    return(newmode);
}


/*
 * clipit - passed width and height of a display, and the text, std, and
 *          max overscan rectangles for the mode, clipit fills in the
 *          spos (screen pos) and dclip rectangles to use in centering.
 *          Centered around smallest containing user-editable oscan pref,
 *          with dclip confined to legal maxoscan limits.
 *          Screens which center such that their top is below text
 *          oscan top, will be moved up.
 *          If a non-null uclip is passed, that clip is used instead.
 *
 * 39.11 - Added NoCenter BOOl arg. If set, image top-left will be
 *         positioned at displayclip top-left (i.e. not centered)
 */
void clipit(SHORT wide, SHORT high,
	    struct Rectangle *spos, struct Rectangle *dclip,
	    struct Rectangle *txto, struct Rectangle *stdo,
	    struct Rectangle *maxo, struct Rectangle *uclip,
	    BOOL NoCenter)
{
struct  Rectangle *besto;
SHORT	minx, maxx, miny, maxy;
SHORT	txtw, txth, stdw, stdh, maxw, maxh, bestw, besth;

    /* get the txt, std and max widths and heights */
    txtw = txto->MaxX - txto->MinX + 1;
    txth = txto->MaxY - txto->MinY + 1;
    stdw = stdo->MaxX - stdo->MinX + 1;
    stdh = stdo->MaxY - stdo->MinY + 1;
    maxw = maxo->MaxX - maxo->MinX + 1;
    maxh = maxo->MaxY - maxo->MinY + 1;

    if((wide <= txtw)&&(high <= txth))
	{
	besto = txto;
	bestw = txtw;
	besth = txth;

	D(bug("Best clip to center around is txto\n"));
	}
    else
	{
	besto = stdo;
	bestw = stdw;
	besth = stdh;

	D(bug("Best clip to center around is stdo\n"));
	}

    D(bug("TXTO: mnx=%ld mny=%ld mxx=%ld mxy=%ld  stdw=%ld stdh=%ld\n",
		txto->MinX,txto->MinY,txto->MaxX,txto->MaxY,txtw,txth));
    D(bug("STDO: mnx=%ld mny=%ld mxx=%ld mxy=%ld  stdw=%ld stdh=%ld\n",
		stdo->MinX,stdo->MinY,stdo->MaxX,stdo->MaxY,stdw,stdh));
    D(bug("MAXO: mnx=%ld mny=%ld mxx=%ld mxy=%ld  maxw=%ld maxh=%ld\n",
		maxo->MinX,maxo->MinY,maxo->MaxX,maxo->MaxY,maxw,maxh));

    if(uclip)
	{
	*dclip = *uclip;
    	spos->MinX = uclip->MinX;
	spos->MinY = uclip->MinY;

	D(bug("UCLIP: mnx=%ld mny=%ld maxx=%ld maxy=%ld\n",
			dclip->MinX,dclip->MinY,dclip->MaxX,dclip->MaxY));
	}
    else
	{
	/* CENTER the screen based on best oscan prefs
 	* but confine dclip within max oscan limits
 	*
 	* FIX MinX first */
	spos->MinX = minx = besto->MinX - ((wide - bestw) >> 1);
	maxx = wide + minx - 1;
	if(maxx > maxo->MaxX)  maxx = maxo->MaxX;	/* too right */
	if(minx < maxo->MinX)
	    {
	    minx = maxo->MinX;	/* too left  */
	    /* if we want left edge of screen not clipped */
	    if(NoCenter)	spos->MinX = minx;
	    }

	D(bug("DCLIP: minx adjust from %ld to %ld\n",spos->MinX,minx));

	/* FIX MinY */
	spos->MinY = miny = besto->MinY - ((high - besth) >> 1);
	/* if lower than top of txto, move up */
	spos->MinY = miny = MIN(spos->MinY,txto->MinY);
	maxy = high + miny - 1;
	if(maxy > maxo->MaxY)  maxy = maxo->MaxY;	/* too down  */
	if(miny < maxo->MinY)
	    {
	    miny = maxo->MinY;	/* too up    */
	    /* if we want top of screen not clipped */
	    if(NoCenter)	spos->MinY = miny;
	    }
	D(bug("DCLIP: miny adjust from %ld to %ld\n",spos->MinY,miny));

	/* SET up dclip */
	dclip->MinX = minx;
	dclip->MinY = miny;
	dclip->MaxX = maxx;
	dclip->MaxY = maxy;

	D(bug("CENTER: mnx=%ld mny=%ld maxx=%ld maxy=%ld\n",
			dclip->MinX,dclip->MinY,dclip->MaxX,dclip->MaxY));
	}
}

/*----------------------------------------------------------------------*/

BOOL VideoControlTags(struct ColorMap *cm, ULONG tags, ...)
    {
    return (VideoControl(cm, (struct TagItem *)&tags));
    }


/*----------------------------------------------------------------------*/

