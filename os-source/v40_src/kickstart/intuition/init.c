
/*** init.c *****************************************************************
 *
 *  A compendium of initialization procedures
 *
 *  $Id: init.c,v 38.22 93/02/15 18:59:50 peter Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (c) Commodore-Amiga, Inc.
 *
 ****************************************************************************/

#include "intuall.h"

#include "imageclass.h"

#ifndef GRAPHICS_DISPLAYINFO_H
#include <graphics/displayinfo.h>
#endif

#define D(x)		;
#define DG(x)		;
#define DINIT(x)	;

struct Image	*makeImage();
void		*AllocMem();

#include "preferences.h"

#ifndef EXEC_MEMORY_H
#include <exec/memory.h>
#endif

#ifndef EXEC_ALERTS_H
#include <exec/alerts.h>
#endif

#ifndef GRAPHICS_SPRITE_H
#include <graphics/sprite.h>
#endif

#ifndef GRAPHICS_GFXNODES_H
#include <graphics/gfxnodes.h>
#endif

#ifndef GRAPHICS_GFXMACROS_H
#include <graphics/gfxmacros.h>
#endif

#ifndef HARDWARE_CUSTOM_H
#include <hardware/custom.h>
#endif

#ifndef HARDWARE_DMABITS_H
#include <hardware/dmabits.h>
#endif

#ifndef INTERNAL_LIBRARYTAGS_H
#include <internal/librarytags.h>
#endif

extern struct Custom custom;

#define TEXTPAD 4

extern IntuiVecs;
extern struct Preferences DefaultPreferences;

extern VERSION;

/* = declarations ======================================================== */

extern struct Image RomCheckMarks[RESCOUNT];
extern struct Image RomAmigaIcon[RESCOUNT];

/** initial colormap values. **
 * Some are old/new values for  0-3 and 17-19,
 * the rest are copies of graphic's default init
 * from _dflt_clrs, graphics/c/cpwrup.c
 */
UWORD InitDefaultColors[] = {
    COLOR0, COLOR1, COLOR2, COLOR3,
    COLORN3, COLORN2, COLORN1, COLORN0,
    COLOR17, COLOR18, COLOR19,
};

extern UWORD	twoColorPens[ NUMDRIPENS + 1 ];
extern UWORD	fourColorPens[ NUMDRIPENS + 1 ];

initIntuition()
{
    int i;
    struct IntuitionBase *IBase = fetchIBase();
    struct ExtSprite *extsprite;
    struct Preferences *Pref;
    struct BitMap sprite_bm;
    extern APTR AbsExecBase;
    extern struct IScreenModePrefs	DefaultScreenMode;

    /* jimm: 1/6/86: use to init locks */
    struct MsgPort *m;

    struct Task	*FindTask();

    IBase->SysBase = AbsExecBase;

    DINIT( IBase->DebugFlag = 0 );

    IBase->SysReqTitle = "System Request";
    IBase->WBScreenTitle = "Workbench Screen";

    /* am not waiting on anything yet */
    /* IBase->getOKWindow = NULL; */

    /* Only Wack needs the SemaphoreList.  No user does.  Wack should
     * be modified.  100 bytes of ROM * 1 million machines = 100 MB
     * of ROM, just so I can Wack easier...
     */
    for (i = 0; i < NUMILOCKS+NUMOTHERLOCKS; ++i) 
    {
	InitSemaphore( &IBase->ISemaphore[i] );
    }

    /* jimm: 1/6/86: changed version passed from
     *  VERSION to 31.  (couldn't develop with
     *  1.1 kickstart otherwise).
     */

    /* NOTE: I can't pull an alert now anyway, so let 'er ride */
    IBase->UtilityBase = (struct Library *) TaggedOpenLibrary( OLTAG_UTILITY );
    IBase->GfxBase = (struct GfxBase *) TaggedOpenLibrary( OLTAG_GRAPHICS );
    IBase->LayersBase= (struct LayersBase *)TaggedOpenLibrary( OLTAG_LAYERS );

    /* Will be overridden in OpenIntuition() for PAL machines, but for
     * now let's ensure we have a sane value.
     */
    IBase->MouseScaleX = MOUSESCALEX;
    IBase->MouseScaleY = NTSC_MOUSESCALEY;

    /* Let's make ourselves a blank pointer, in part to have a valid
     * sprite from the moment we obtain the sprite engine, in part
     * because we'll be using this later to blank the sprite
     * around imagery changes.
     */
    InitBitMap( &sprite_bm, 2, 16, 0 );
    extsprite = (struct ExtSprite *)AllocSpriteData( &sprite_bm,
	TAG_DONE );
    IBase->BlankSprite = IBase->TetrisSprite = IBase->SimpleSprite = extsprite;
    GetExtSprite( extsprite,
	GSTAG_SPRITE_NUM, 0,
	GSTAG_SOFTSPRITE, TRUE,
	TAG_DONE );

    IBase->RP = (struct RastPort *) AllocMem(sizeof (struct RastPort), 0);
    InitRastPort(IBase->RP);
    InitRastPort(&IBase->MenuRPort);

    /* MEMF_CLEAR means no need:
     * IBase->FirstScreen = IBase->ActiveWindow = IBase->ActiveScreen = NULL;
     */

    /*** !!! jimm thinks these are not necessary,
     *** since we call SetPrefs() below, which overrides
     ***/
    /* but then, why not be safe, rather than sorry?	*/
    setColorPrefs( InitDefaultColors, 0, WBCOLOR_NUMBER );

    CopyMem( fourColorPens, IBase->ScreenPens4, sizeof( UWORD )*( NUMDRIPENS + 1 ) );
    CopyMem( fourColorPens, IBase->ScreenPens8, sizeof( UWORD )*( NUMDRIPENS + 1 ) );

    IBase->Preferences = (struct Preferences *)AllocMem(
	    sizeof(struct Preferences), MEMF_CLEAR );
    GetDefPrefs(IBase->Preferences, sizeof(struct Preferences));

    InitView(&IBase->ViewLord);
    IBase->TrackViewDx = IBase->ViewLord.DxOffset;
    IBase->TrackViewDy = IBase->ViewLord.DyOffset;

    IBase->ViewLord.Modes = SPRITES;

    /* IBase->ScanRate = SCAN_NORMAL; */
    IBase->ViewExtra = (struct ViewExtra *)GfxNew(VIEW_EXTRA_TYPE);
    GfxAssociate(&IBase->ViewLord,IBase->ViewExtra);

    IBase->ViewInitX = IBase->ViewLord.DxOffset;
    IBase->ViewInitY = IBase->ViewLord.DyOffset;

    /* initialize display	*/
    D( printf("initI: calling modeVerify (needed?)\n") );
    modeVerify(  TRUE );    /* set up modes related stuff */

    /* init distant echo list	*/
    initTokens();	/* ISM: */
    initIEvents();

    NewList( &IBase->PubScreenList );	/* default is NULL */
    NewList( &IBase->PendingPubScreens );

    /* initialize reply port for asynch WB messages	*/
    NewList( &IBase->WBReplyPort.mp_MsgList );
    IBase->WBReplyPort.mp_Flags = PA_IGNORE;

#if 0	/* default for pubscreen global flags will be zero */
    IBase->PubScreenFlags = POPPUBSCREEN;
#endif

    InitIIHooks();
    IBase->EditHook = &IBase->IIHooks[ IHOOK_SEDIT ];

    initIClasses();	/* init the kernel gadget classes */

    /* Initialize imagery and pointer after the classes have been initialized */
    {
	struct DrawInfo dri;
	int im;
	extern UBYTE *SysIClassName;

	/* sysiclass really only needs pens and depth to make the
	 * default menu checkmark and Amiga key.
	 */
	clearWords( &dri, sizeof( struct DrawInfo )/2 );
	dri.dri_Pens = twoColorPens;
	dri.dri_Depth = 1;

	for ( im = MENUCHECK; im <= AMIGAKEY; im++ )
	{
	    for (i = 0; i < RESCOUNT; i++)
	    {
		IBase->MenuImage[ im-MENUCHECK ][ i ] = (struct Image *) NewObject( NULL, SysIClassName,
		    SYSIA_DrawInfo, &dri,
		    SYSIA_Which, im,
		    SYSIA_Size, i,
		    TAG_DONE );
	    }
	}
    }
    initMousePointer();

    IBase->NewIControl =
	IC_COERCE_COLORS | IC_COERCE_LACE | IC_STRINGG_CTRL | IC_DOMAGICMENU;

    IBase->MetaDragQual = IEQUALIFIER_LCOMMAND;

    /* "left amiga" keys */
    IBase->WBtoFCode = 'N';
    IBase->FtoBCode = 'M';
    IBase->ReqTrueCode = 'V';
    IBase->ReqFalseCode = 'B';

    /* Peter 20-Aug-90: IBase now keeps track of the last
     * screen mode that worked, so it can try to fall back
     * on that.  Init it to something reasonable here:
     */
    IBase->LastScreenMode = DefaultScreenMode;

    OFF_SPRITE;

    /* This is normally a do-nothing macro defined in intuall.h.
     * However, when LOCKDEBUG is on, this macro sets a flag
     * which enables the checking.  That allows the init routine
     * to run without any lock-checking.  It doesn't need any,
     * since the semaphores and IntuitionBase aren't really
     * open for business anyways.
     */
    enableLockCheck();
}



/*** intuition.library/InitRequester ***/

InitRequester(req)
struct Requester *req;
{
    BYTE *cptr;
    int i;

    cptr = (BYTE *)req;
    for (i = sizeof(struct Requester); i > 0; i--) *cptr++ = 0;
}



IntuiRomInit()
/* this is the library initialization routine */
{
    struct IntuitionBase *IBase = fetchIBase();

    extern REVISION;
    extern UBYTE  IntuiName[];

    DINIT( printf("\n=============\nINTUITION INIT dbpt: %lx\n",
	    &IBase->debugpt) );

    IBase->LibNode.lib_Node.ln_Name = IntuiName;
    IBase->LibNode.lib_Version = VERSION;
    IBase->LibNode.lib_Revision = REVISION;
    IBase->LibNode.lib_Flags |= (LIBF_SUMUSED | LIBF_CHANGED);

    initIntuition();
}
