
/*** iprefs.c ************************************************************** 
 * 
 *  Intuition's interaction with New Preferences
 * 
 *  $Id: iprefs.c,v 38.21 93/01/08 14:47:33 peter Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc. 
 *  Copyright (C) 1989, Commodore-Amiga, Inc. 
 *  All Rights Reserved 
 * 
 ***************************************************************************/ 
 
#include "intuall.h" 
#include "preferences.h" 

#ifndef EXEC_INITIALIZERS_H
#include <exec/initializers.h>
#endif

#ifndef GRAPHICS_DISPLAYINFO_H
#include <graphics/displayinfo.h>
#endif

#ifndef GRAPHICS_MODEID_H
#include <graphics/modeid.h>
#endif

#include "pointerclass.h"

#include "iprefs_protos.h"

/*---------------------------------------------------------------------------*/

static LONG setIPrefs(CPTR pp,
                      ULONG psize,
                      ULONG ptype,
                      BOOL backsync);

/*---------------------------------------------------------------------------*/

#define D(x)		;
#define DBACK(x)	;	/* backsync */
#define DSYNC(x)	;	/* sync */

/* used several places:
 * in alerts
 * as fallback if string gadget font too big
 * as rom default font.
 */
struct IFontPrefs topazprefs = {
     {{ topazprefs.ifp_TABuff.tab_NameBuff, 8, 0, 0, }, KARLA},
     NULL,
    IPF_SYSFONT,
};

/* is there enough Preferences buffer to hold 'field'?	*/
#define ENUFPREFS( field ) ( ((ULONG) OFFSET(Preferences, field) )+ \
	    sizeof(((struct Preferences *)0)->field) )

/*
 *  calls setIPrefs() with data constructed from old
 *  struct Preferences, for data which is defined in both old
 *  and new Preferences.  The following are always respected:
 *	- palette
 *	- view position
 *	- text overscan (morerows)
 *
 *  The following are respected once only (i.e. from SetPrefs() of
 * 	devs:system-configuration):
 *	- system font (size of topaz)
 *	- workbench interlace
 *	- pointer
 */
syncIPrefs( size )
int size;
{
    struct IntuitionBase *IBase = fetchIBase();
    struct Preferences *oldprefs = IBase->Preferences;

    extern struct IScreenModePrefs	DefaultScreenMode;
    union {
	struct IScreenModePrefs		screenmode;
	struct IFontPrefs	font;
	struct INewPointerPrefs	pointer;
    } p;

    D( printf("---- syncIPrefs\n") );

    if( ! TESTFLAG( IBase->Flags, SEENIPALETTE ) )
    {
	/* We only listen to colors from SetPrefs until we've received
	 * new colors, since there's more color-resolution through
	 * the new method.
	 */
	/* Here we set colors directly, instead of calling SetIPrefs
	 * with the data!
	 */
	setColorPrefs( &oldprefs->color0, WBCOLOR_FIRST4, 4 );
	setColorPrefs( &oldprefs->color17, WBCOLOR_POINTER, 3 );
	/* set up only the 7 preferences colors */
	setWBColorsGrunt( findWorkBench(), FALSE );
    }

    /* We sync font-height and ScreenMode only once, i.e. the SetPrefs of
     * devs:system-configuration:
     */
    if( ! TESTFLAG( IBase->Flags, SEENSETPREFS ) )
    {
	/* update system font, based on old FontHeight	*/
	copyTAttr( &topazprefs, &p.font );

	/* use the prefs size with topaz attr	*/
	p.font.ifp_TABuff.tab_TAttr.ta_YSize = oldprefs->FontHeight;

	DSYNC( printf("syncIP: font name = ----%s----\n",
	    p.font.ifp_TABuff.tab_NameBuff ) );

	p.font.ifp_SysFontType = IPF_SYSFONT;

	D( printf("syncIP: font prefs buffer at: %lx\n", &p.font ) );
	setIPrefs( &p.font, sizeof p.font, IP_FONT, FALSE );

	/* set up WB font, too	*/
	p.font.ifp_SysFontType = IPF_SCREENFONT;
	setIPrefs( &p.font, sizeof p.font, IP_FONT, FALSE );


	/* initialize CoolScreenMode to be Default, plus LACEWB	*/
	p.screenmode = DefaultScreenMode;
	if ( TESTFLAG( oldprefs->LaceWB, LACEWB ) )
	{
	    p.screenmode.ism_DisplayID = HIRESLACE_KEY;
	}
	DSYNC( printf("syncIP: default wb display ID: %lx\n",
	    p.screenmode.ism_DisplayID ) );
	setIPrefs( &p.screenmode, sizeof p.screenmode, IP_SCREENMODE, FALSE );
    }
    DSYNC( else
    {
	printf("syncIP: Not sync'ing old font or screen-mode\n");
    } )

    /* Were we given new pointer information? */
    if ( ( ! TESTFLAG( IBase->Flags, SEENIPOINTER ) ) &&
	( size >= ENUFPREFS( YOffset ) ) )
    {
	/* The BitMap field of the new pointer stuff holds the
	 * old SpriteData for compatible operation
	 */
	p.pointer.inp_BitMap = (struct BitMap *)oldprefs->PointerMatrix;
	/* NB:  Good old struct Preferences contains the "corrected"
	 * sprite X position, though new prefs contains the intended
	 * position, so we must de-compensate:
	 */
	p.pointer.inp_HotSpotX = oldprefs->XOffset - SPRITEERROR;
	p.pointer.inp_HotSpotY = oldprefs->YOffset;
	p.pointer.inp_XResolution = POINTERXRESN_LORES;
	p.pointer.inp_YResolution = POINTERYRESN_DEFAULT;
	p.pointer.inp_WordWidth = -1;	/* Signifies old-style pointer! */
	p.pointer.inp_Type = 0;

	setIPrefs( &p.pointer, sizeof p.pointer, IP_NEWPOINTER, FALSE );
    }
    DSYNC( else
    {
	printf("syncIP: Not sync'ing old pointer stuff\n");
    } )

    /* View position gets sync'd during RemakeDisplay(), since
     * now we're just another View poker.  All will be caught
     * in trackViewPos, which only allows this if
     * the active monitor is the default monitor:
     */
    IBase->ViewLord.DxOffset = IBase->ViewInitX + oldprefs->ViewXOffset;
    IBase->ViewLord.DyOffset = IBase->ViewInitY + oldprefs->ViewYOffset;
}


/* If we are not tracking people who poke the view and RemakeDisplay,
 * then syncViewPos is used in syncIPrefs(), else we use trackViewPos()
 * instead, which is called by RemakeDisplay().
 */


/*
 * If the active monitor is the default monitor and the View has shifted,
 * then update the database and poke the ViewLord offsets.
 *
 */

trackViewPos()
{
    struct IntuitionBase *IBase = fetchIBase();
    struct IOverscanPrefs ios;

    struct DimensionInfo	diminfo;
    ULONG			drecord;
    ULONG			default_id;

    DSYNC( printf("trackVP: ViewLord.Dx/yOffset: %ld,%ld\n",
	IBase->ViewLord.DxOffset, IBase->ViewLord.DyOffset ) );
    DSYNC( printf("trackVP: TrackViewDx/y      : %ld,%ld\n",
	IBase->TrackViewDx, IBase->TrackViewDy ) );
    D( printf("---- trackViewPos\n") );

    default_id = NTSC_MONITOR_ID|HIRES_KEY;
    if ( TESTFLAG( IBase->Flags, MACHINE_ISPAL ) )
    {
	default_id = PAL_MONITOR_ID|HIRES_KEY;
    }

    /* If the view changed and we can get default-view info,
     * then we update the database
     */
    if ( ( ( IBase->ViewLord.DxOffset != IBase->TrackViewDx ) ||
	( IBase->ViewLord.DyOffset != IBase->TrackViewDy ) )
	&& ( drecord = FindDisplayInfo( default_id ) )
	&& GDID( drecord, &diminfo, sizeof diminfo, DTAG_DIMS ) )
    {
	/* set up an IOverscanPrefs for the NTSC or PAL monitor,
	 * depending on the kind of machine (we used to just
	 * do it for the default monitor, but with promotion,
	 * the default monitor could be anything).
	 */
	ios.ios_DisplayID = default_id;
	ios.ios_ViewPos.x = IBase->ViewLord.DxOffset;
	ios.ios_ViewPos.y = IBase->ViewLord.DyOffset;
	ios.ios_Text.x = rectWidth( &diminfo.TxtOScan );
	ios.ios_Text.y = rectHeight( &diminfo.TxtOScan );
	ios.ios_Standard = diminfo.StdOScan;
	setIPrefs( &ios, sizeof ios, IP_OVERSCAN, FALSE );
	DSYNC( printf("trackVP:  After setIPrefs\n") );

    }
    DSYNC( else
    {
	printf("trackVP:  View unchanged\n");
    } )

    /* setIPrefs() sets up TrackViewDx/y as follows:
     *   - If the active monitor is the Default monitor, TrackViewDx/y
     *     will contain bounds-checked versions of what was poked into
     *     the ViewLord.
     *   - If the active monitor was something else, TrackViewDx will
     *     remain untouched.
     * We must freshen the ViewLord offsets to apply the bounds checking,
     * and to make non-default monitors immune to ViewLord poking.
     */

    IBase->ViewLord.DxOffset = IBase->TrackViewDx;
    IBase->ViewLord.DyOffset = IBase->TrackViewDy;
}


/*
 * private way to set Intuition's new preferences
 * I expect this sucker to be RIGHT on the money: no
 * validity or size checking for now.
 * Called by in-the-know surrogate process ONLY.
 *
 * setIBasePrefs knows how to call it for data in old struct
 * Preferences, using syncIPrefs().
 *
 * returns TRUE if I know what you're talking about.
 */

/* storage for a 'pheight' tall sprite, two uwords (4 bytes) per row,
 * plus two rows for top and bottom.  (Taken from init.c)
 */
#define SPR_STORAGE( pheight ) 	(( (pheight) + 2 ) << 2)

/* external entry point (from assembler interface file)	*/

SetIPrefs( p, psize, ptype  )
CPTR	p;		/* whatever data	*/
ULONG	psize;		/* how much data	*/
ULONG	ptype;		/* preferences type	*/
{
    struct IntuitionBase *IBase = fetchIBase();

    D( printf("SetIPrefs enter\n") );
    /* An external caller setting the Pointer or colors will
     * disable future SetPrefs() from causing sync for that item:
     */
    if ( ptype == IP_NEWPALETTE )
    {
	SETFLAG( IBase->Flags, SEENIPALETTE );
    }

    return ( setIPrefs( p, psize, ptype, TRUE ) );
}


#define ISCREENMODE( p ) 	(( struct IScreenModePrefs *) (p))
#define IFONT( p )	 	(( struct IFontPrefs *) (p))
#define IOVERSCAN( p ) 		(( struct IOverscanPrefs *) (p))
#define ICONTROL( p ) 		(( struct IIControlPrefs *) (p))
#define	INEWPOINTER(p)		((struct INewPointerPrefs *) (p))
#define ICOLORSPEC(p)		((struct ColorSpec *)(p))
#define IPENPREFS(p)		((struct IPenPrefs *) (p))

static LONG
setIPrefs( pp, psize, ptype, backsync  )
CPTR	pp; /* whatever data	*/
ULONG	psize;		/* how much data	*/
ULONG	ptype;		/* preferences type	*/
BOOL	backsync;	/* Should we backsync?	*/
{
    struct IntuitionBase *IBase = fetchIBase();
    struct GfxBase       *GfxBase = IBase->GfxBase;
    struct IFontPrefs	*sysfontp;
    struct TextFont *OpenFont();

    ULONG			drecord;
    union {
	struct DimensionInfo	dim;
	struct MonitorInfo	mon;
    } info;
    int				cindex;
    struct ColorSpec		*cspec;
    struct Preferences		*oldprefs = IBase->Preferences;

    D( printf(" setIPrefs: prefs chunk type: %ld size: %ld\n", ptype, psize ) );

    switch ( ptype )
    {
    case IP_NEWPALETTE:
	if ( !pp ) return ( 0 );
	for (   cspec = ICOLORSPEC( pp );
		(cindex = cspec->ColorIndex) != -1;
		cspec++ )
	{
	    /* Only eleven colors are now passed */
	    if ( cindex < WBCOLOR_NUMBER )
	    {
		cindex = 3*cindex;
		IBase->WBColors[ cindex++ ] = cspec->Red >> 8;
		IBase->WBColors[ cindex++ ] = cspec->Green >> 8;
		IBase->WBColors[ cindex ] = cspec->Blue >> 8;
	    }
	}
	setWBColorsGrunt( findWorkBench(), TRUE );

#if 0
/* This code won't work any more... */
	if ( backsync )
	{
	    /* backsync colors to keep GetPrefs customers happy: */
	    CopyMem( &IBase->WBColors[0], &oldprefs->color0, 4 * sizeof (UWORD) );
	    CopyMem( &IBase->WBColors[17], &oldprefs->color17, 3 * sizeof (UWORD) );
	    DBACK( printf( "Backsync colors 0-3: %lx %lx %lx %lx, 17-19: %lx %lx %lx\n",
		oldprefs->color0, oldprefs->color1, oldprefs->color2,
		oldprefs->color3, oldprefs->color17, oldprefs->color18,
		oldprefs->color19 ) );
	}
#endif
	break;

    case IP_NEWPOINTER:
	/* setDefaultMousePointer() returns NULL for failure.
	 *
	 * For the busy pointer, it returns a pointer to the 
	 * busy-pointer bitmap, to be freed by IPrefs, or ~0
	 * if this was the first busy-pointer to be sent to us.
	 *
	 * For the default pointer, it returns a pointer to the
	 * bitmap that was passed in.
	 */
	return( setDefaultMousePointer(
		INEWPOINTER(pp)->inp_BitMap, 
		/* V39 fixes the MoveSprite() for ExtSprites.  Since we're
		 * only using ExtSprites, we no longer have to account
		 * for the classic off-by-one bug in MoveSprite()
		 * YEAH!
		 */
		INEWPOINTER(pp)->inp_HotSpotX,
		INEWPOINTER(pp)->inp_HotSpotY,
		INEWPOINTER(pp)->inp_XResolution,
		INEWPOINTER(pp)->inp_YResolution,
		INEWPOINTER(pp)->inp_WordWidth,
		INEWPOINTER(pp)->inp_Type ) );

	/* Peter 1-Apr-92: We no longer backsync the Pointer part because
	 * it's getting too hairy, and there's not much benefit.
	 */

	break;

    case IP_SCREENMODE:
	/* dimension validity checking here? or in openwb? */
	/* must be >= 640x200	*/
	IBase->CoolScreenMode = *ISCREENMODE( pp );
	D( printf("setiprefs: control: %lx\n", ISCREENMODE(pp)->ism_Control));

	/* Even though LACEWB isn't very relevant to new screen modes,
	 * people do seem to depend on it.
	 */
	if (backsync)
	{
	    struct Rectangle txto;

	    oldprefs->LaceWB &= ~LACEWB;
	    /* Same lying test as GetScreenData() uses, namely:
	     * does this mode have a text-overscan >= 400 lines?
	     */
	    if ( QueryOverscan( ISCREENMODE(pp)->ism_DisplayID, &txto,
		OSCAN_TEXT ) && ( rectHeight( &txto ) >= 400 ) )
	    {
		oldprefs->LaceWB |= LACEWB;
		DBACK( printf("LaceWB: %ld\n", oldprefs->LaceWB) );
	    }
	}
	break;

    case IP_FONT:
	 D( printf("SIP: new sys font numbered: %ld\n",
	    IFONT(pp)->ifp_SysFontType) );

	/* find which system font is changing	*/
	sysfontp = &IBase->SysFontPrefs[ IFONT(pp)->ifp_SysFontType ];
	D( printf("p.font %lx type: %ld\n", IFONT(pp),
		IFONT(pp)->ifp_SysFontType ) );

	D( printf("new font name %s, size %ld\n",
	    IFONT( pp )->ifp_TABuff.tab_NameBuff,
	    IFONT( pp )->ifp_TABuff.tab_TAttr.ta_YSize ) );

	/* try to open the sucker	*/
	if ( IFONT( pp )->ifp_Font =
		OpenFont( &IFONT( pp )->ifp_TABuff.tab_TAttr ) )
	{
	    /* report the actual size opened	*/
	    IFONT( pp )->ifp_TABuff.tab_TAttr.ta_YSize =
		IFONT(pp)->ifp_Font->tf_YSize;

	    /* close previous system font	*/
	    D( printf("-- sIP: closing font: %lx -\n", sysfontp->ifp_Font ));

	    /* ZZZ: Much as it hurts, we cannot close any font that has
	     * been used as GfxBase->DefaultFont, since InitRastPort()
	     * copies this font into your RastPort, with no way for
	     * you to relinquish this font.
	     */
	    if (( IFONT(pp)->ifp_SysFontType != IPF_SYSFONT ) &&
	        ( sysfontp->ifp_Font ) )
	    {
		CloseFont( sysfontp->ifp_Font );
	    }

	    /* ZZZ: any other protection so that OpenFont
	     * gets data with integrity?
	     */
	    Forbid();
	    copyTAttr( IFONT( pp ), sysfontp );
	    sysfontp->ifp_Font = IFONT( pp )->ifp_Font;

	    if ( IFONT(pp)->ifp_SysFontType == IPF_SYSFONT )
	    {
		D( printf("---new prefs font is system font\n") );

		/* update graphics base	*/
		GfxBase->DefaultFont = sysfontp->ifp_Font;

#if 0
		/* update Intuition (perhaps critical section here)	*/
		IBase->SysFont = sysfontp->ifp_TABuff.tab_TAttr;
#endif
	    }
	    Permit();

	    /* Peter 5-Apr-91: Now backsync Prefs->FontHeight to 8 as
	     * soon as the "System Default Text" font is changed at all.
	     * (We could have made it go to 9 if SysFont was Topaz9, and
	     * to 8 otherwise, but it wasn't worth the ROM space.
	     */
	    if ( (backsync) && ( IFONT(pp)->ifp_SysFontType == IPF_SYSFONT ) )
	    {
		oldprefs->FontHeight = 8;
	    }
	    break;
	}
	return ( 0 );		/* return success on open font	*/
	
    case IP_OVERSCAN:
	/* Starting with 3.01, we ensure legality of the Overscan Prefs
	 * values, because the legal ranges can change if the user
	 * installs or removes VGAOnly, or if he has an A2024 and
	 * uses BootMenu to switch between PAL and NTSC.
	 */
	D( printf("Overscan prefs\n" ) );

	if ( ( drecord = FindDisplayInfo( IOVERSCAN(pp)->ios_DisplayID ) )

	/* To be formally correct, we should get at least a little bit of
	 * the DimensionInfo to prove that the call will succeed when we
	 * really want it all, but Bart assures me that currently if
	 * GDID( DTAG_MNTR )works, so will GDID( DTAG_DIMS ) unless
	 * somebody has trashed the database:
	 * && GDID( drecord, &info.dim, sizeof( struct QueryHeader ), DTAG_DIMS )
	 */

	    && GDID( drecord, &info.mon, sizeof info.mon, DTAG_MNTR ) )
	{
	    /* set up view position */
	    info.mon.ViewPosition = IOVERSCAN(pp)->ios_ViewPos;
	    D( dumpPt( "viewpos before limit", info.mon.ViewPosition ) );

	    limitPoint( &info.mon.ViewPosition, &info.mon.ViewPositionRange );

	    /* If this monitor is the active monitor, then poke ViewLord. */
	    if ( info.mon.Mspc == IBase->ActiveMonitorSpec )
	    {
		IBase->TrackViewDx = IBase->ViewLord.DxOffset =
		    info.mon.ViewPosition.x;
		IBase->TrackViewDy = IBase->ViewLord.DyOffset =
		    info.mon.ViewPosition.y;
	    }
	    SetDisplayInfoData( drecord, &info.mon, sizeof info.mon, DTAG_MNTR );
	    /* check what I get back	*/
	    D( GDID( drecord, &info.mon, sizeof info.mon, DTAG_MNTR);
	       dumpPt( "viewpos reprise", info.mon.ViewPosition); )

	    GDID( drecord, &info.dim, sizeof info.dim, DTAG_DIMS );
	    /* set up std/text oscan	*/
	    info.dim.StdOScan = IOVERSCAN(pp)->ios_Standard;
	    D( dumpRect( "StdOScan before rectHull", &info.dim.StdOScan ) );
	    limitRect( &info.dim.StdOScan, &info.dim.MaxOScan );
	    rectHull( &info.dim.StdOScan, &info.dim.Nominal );
	    D( dumpRect( "StdOScan after rectHull", &info.dim.StdOScan ) );

	    info.dim.TxtOScan.MinX = info.dim.TxtOScan.MinY = 0;
	    info.dim.TxtOScan.MaxX = IOVERSCAN(pp)->ios_Text.x - 1;
	    info.dim.TxtOScan.MaxY = IOVERSCAN(pp)->ios_Text.y - 1;

	    D( dumpRect( "TxtOScan before rectHull/limitRect", &info.dim.TxtOScan ) );
	    limitRect( &info.dim.TxtOScan, &info.dim.MaxOScan );
	    rectHull( &info.dim.TxtOScan, &info.dim.Nominal );
	    D( dumpRect( "TxtOScan after rectHull/limitRect", &info.dim.TxtOScan ) );

	    SetDisplayInfoData( drecord, &info.dim, sizeof info.dim, DTAG_DIMS );
	    {
		ULONG default_id;

		/* jimm: 3/15/90: fix up gb->NormalDisplayRows/Cols
		 * to hires coords of text oscan on default monitor
		 */

		default_id = NTSC_MONITOR_ID|HIRES_KEY;
		if ( TESTFLAG( IBase->Flags, MACHINE_ISPAL ) )
		{
		    default_id = PAL_MONITOR_ID|HIRES_KEY;
		}
    
		if ( ( drecord = FindDisplayInfo( default_id ) )
		/*
		    && GDID( drecord, &info.mon, sizeof( struct QueryHeader ), DTAG_MNTR) )
		*/
		    && GDID( drecord, &info.dim, sizeof info.dim, DTAG_DIMS ) )

		{
		    /*
		     * Peter 30-Jan-91:  Maintain V34 private IntuitionBase fields
		     * MaxDisplayHeight and MaxDisplayWidth for TV*Text.
		     */
		    IBase->MaxDisplayWidth = GfxBase->NormalDisplayColumns = 
			rectWidth( &info.dim.TxtOScan );
		    IBase->MaxDisplayHeight = ( GfxBase->NormalDisplayRows = 
			rectHeight( &info.dim.TxtOScan ) ) << 1;
		    if ( backsync )
		    {
			GDID( drecord, &info.mon, sizeof info.mon, DTAG_MNTR);
			/* backsync oldprefs Row/ColumnSizeChange and ViewX/YOffset: */
			oldprefs->RowSizeChange =
			    GfxBase->NormalDisplayRows - IBase->OrigNDRows;
			oldprefs->ColumnSizeChange =
			    GfxBase->NormalDisplayColumns - IBase->OrigNDCols;
			oldprefs->ViewXOffset = info.mon.ViewPosition.x - IBase->ViewInitX;
			oldprefs->ViewYOffset = info.mon.ViewPosition.y - IBase->ViewInitY;
			DBACK( printf("Overscan backsync: R/CSizeChange: (%ld,%ld)\n",
			    oldprefs->RowSizeChange, oldprefs->ColumnSizeChange ) );
			DBACK( printf("\tViewOffset: (%ld,%ld)\n",
			    oldprefs->ViewXOffset, oldprefs->ViewYOffset ) );
		    }
		}
	    }

	    /* If backsync is false, then we are coming from SetPrefs()
	     * which will do the RemakeDisplay inside SetIBasePrefs, so
	     * we don't need one here:
	     */
	    if ( backsync )
	    {
		/* use new view position */
		RemakeDisplay();
	    }
	    break;
	}
	return(0);

    case IP_ICONTROL:
	{
	    int mon;

	    D( printf("IControl:\n" ) );

	    D( printf("metadrag: %lx\n", ICONTROL(pp)->iic_MetaDrag ) );
	    /* jimm: 6/8/90, always copy iic_MetaDrag */
	    /* if ( ICONTROL(pp)->iic_MetaDrag )  */
		IBase->MetaDragQual = ICONTROL(pp)->iic_MetaDrag;

	    /* blindly set all bits */
	    D( printf("new icontrol flags: %lx\n", ICONTROL(pp)->iic_ICFlags ));
	    IBase->NewIControl = ICONTROL(pp)->iic_ICFlags;
	    
	    /* Use SetDefaultMonitor() to change the mapping of
	     * the default monitor.
	     */
	    if ( TESTFLAG( GfxBase->ChipRevBits0, GFXF_AA_LISA ) &&
		TESTFLAG( IBase->NewIControl, IC_MODEPROMOTION ) )
	    {
		mon = DBLNTSC_MONITOR_ID >> 16;
		if ( TESTFLAG( IBase->Flags, MACHINE_ISPAL ) )
		{
		    mon = DBLPAL_MONITOR_ID >> 16;
		}
	    }
	    else
	    {
		mon = NTSC_MONITOR_ID >> 16;
		if ( TESTFLAG( IBase->Flags, MACHINE_ISPAL ) )
		{
		    mon = PAL_MONITOR_ID >> 16;
		}
	    }
	    SetDefaultMonitor( mon );

	    D( printf("keys %lc %lc %lc %lc\n",
		    ICONTROL(pp)->iic_WBtoFront,
		    ICONTROL(pp)->iic_FrontToBack,
		    ICONTROL(pp)->iic_ReqTrue,
		    ICONTROL(pp)->iic_ReqFalse ) );

	    IBase->WBtoFCode = ToUpper( ICONTROL(pp)->iic_WBtoFront );
	    IBase->FtoBCode = ToUpper( ICONTROL(pp)->iic_FrontToBack );
	    IBase->ReqTrueCode = ToUpper( ICONTROL(pp)->iic_ReqTrue );
	    IBase->ReqFalseCode = ToUpper( ICONTROL(pp)->iic_ReqFalse );

	    /* Immediately show the results of changing any coercion
	     * or promotion flags
	     */
	    RemakeDisplay();
	}
    	break;

    case IP_PENS:
	/* Copy all the pens that are sent to us, but no more than
	 * we know about.
	 */
	{
	    UWORD *pens = IBase->ScreenPens4;
	    if ( IPENPREFS(pp)->ipn_PenType )
	    {
		pens = IBase->ScreenPens8;
	    }
	    CopyMem( &IPENPREFS(pp)->ipn_FirstPen, pens,
		sizeof(UWORD) * imax( NUMDRIPENS+1, IPENPREFS(pp)->ipn_NumPens ) );
	}
	break;

    default:
	D( printf("setIPrefs: unknown type %ld, buff: %lx, size: %ld\n",
		ptype, pp, psize ) );
	return ( 0 );
    }
    return ( 1 );
}

/* Given an array of UWORD 0x0RGB colors, Set the IBase->WBColors[]
 * array in 8 bits-per-gun.
 */
setColorPrefs( prefcolor, firstcolor, numcolors )
UWORD *prefcolor;
UWORD firstcolor;
UWORD numcolors;
{
    struct IntuitionBase *IBase = fetchIBase();
    /* Start with the BLUE component of the firstcolor: */
    UBYTE *ibcolor = &IBase->WBColors[ firstcolor * 3 + 2];
    UWORD temp, i;

    while ( numcolors-- )
    {
	temp = *prefcolor++;

	for ( i = 0; i < 3; i++ )
	{
	    /* Extract the next nybble, and replicate into
	     * both nybbles:
	     */
	    *ibcolor-- = ( temp & 0x000F ) * 0x11;
	    temp >>= 4;
	}
	/* Skip to the BLUE component of the next color */
	ibcolor += 6;
    }
}
