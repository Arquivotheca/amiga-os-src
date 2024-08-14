
/*** sinput.c ****************************************************************
 *
 *  String Gadget input
 *
 *  $Id: sinput.c,v 38.5 93/01/12 16:21:03 peter Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (C) 1985, Commodore-Amiga, Inc.
 *  All Rights Reserved
 *
 ****************************************************************************/

#include "intuition.h"
#include "intuall.h"
#include "sghooks.h"
#include "gadgetclass.h"

BOOL LONGTest();
BOOL LONGInRange();

#define D(x)	;
#define DC(x)	;
#define DNA(x)	;	/* NEXTACTIVE/PREVACTIVE support */
#define REPLACETEST	(0)

struct StringExtend	*getSEx();

/* handle string gadget state input events.
 * return 0 if gadget not terminated
 *	1 if non-verify termination (select outside, menudown)
 *	2 if verified (return key)
 */
stringInput(ie, g, gi, redisplay, gadgetupcode ) 
struct InputEvent	*ie;
struct Gadget		*g;
struct GadgetInfo	*gi;
BOOL			*redisplay;
LONG			*gadgetupcode;	/* what to send in GADGETUP message */
{
    struct IntuitionBase	*IBase = fetchIBase();
    struct StringInfo		*si;
    struct StringExtend		*sex;
    UBYTE			character;
    struct SGWork		sgwork;
    int				siret = 0;

    *redisplay = FALSE;
    sex = IBase->ActiveSEx;

    setupSGWork( &sgwork, g, gi, sex, ie );
    si = sgwork.StringInfo;

    switch ( ie->ie_Class ) 
    {
    case IECLASS_RAWKEY:

	D( printf("sI: rawkey\n") );
	/* i'm only into downers, man jimm: 4/16/86 */
	if (ie->ie_Code & IECODE_UP_PREFIX) 
	{
	    D( printf("discard upcode\n") );
	    return ( GMR_MEACTIVE );
	}

	/* copy to workbuffer	*/
	/* we only do this overhead when we intend to use it	*/
	if (sgwork.PrevBuffer != sgwork.WorkBuffer)
	{
	    CopyMem( sgwork.PrevBuffer, sgwork.WorkBuffer, sgwork.NumChars+1);
	}

	/* sgwork.Code = '\0'; * in setupSGW	*/
	/* Peter 30-Nov-90:  Some people use AltKeyMaps to filter out
	 * everything but 0 to 9 and ".".  V34 grabbed Amiga-Q and X
	 * before the keymap.  For the sake of these people, we must
	 * not use the AltKeyMap if the AMIGARIGHT key is held.
	 */
	if (MapRawKey(ie, &character, 1, 
	    ( TESTFLAG( ie->ie_Qualifier, AMIGARIGHT ) ? NULL : IBase->KeyMap )) == 1)
	{
	    sgwork.Code = character;
	}

	/* call iEdit through hook system	*/
	callHook( IBase->EditHook, &sgwork, SGH_KEY );

	if ( sex && sex->EditHook )
	{
	    D( printf("calling user hook at %lx\n", sex->EditHook) );
	    D( printf("passing sgwork at %lx\n", &sgwork) );
	    D( printf("callHook: %lx sgw: %lx\n", sex->EditHook, &sgwork ));

	    callHook( sex->EditHook, &sgwork, SGH_KEY );
	}

	/* Take resultant actions		*/
	D( printf("sI: actions: %lx\n", sgwork.Actions ) );
	siret = sgAction( &sgwork, redisplay, gadgetupcode );
	break;

    case IECLASS_RAWMOUSE:
	siret = iHandleMouse( &sgwork, redisplay, &IBase->ActiveGBox );
    }

    /* if full, don't let replace mode cursor go off to the right */
    if ( TESTFLAG( sgwork.Modes, SGM_REPLACE ) 
	&& (si->NumChars >= si->MaxChars - 1) )
    {
	if ( si->BufferPos > si->NumChars - 1 )
	{
	    si->BufferPos = si->NumChars - 1;
	    *redisplay = TRUE;
	}
    }

    return ( siret );
}

setupSGWork( sgw, g, gi, sex, ie )
struct SGWork		*sgw;
struct Gadget		*g;
struct GadgetInfo	*gi;
struct StringExtend	*sex;
struct InputEvent	*ie;
{
    struct StringInfo	*si = SI(g);
    struct IntuitionBase	*IBase = fetchIBase();

    /* set up SGWork	*/
    sgw->Gadget = g;
    sgw->GadgetInfo = gi;
    sgw->StringInfo = si;
    sgw->WorkBuffer = sgw->PrevBuffer = si->Buffer;

    sgw->Modes = SGM_NOWORKB;
    /* SGM_NOFILTER is also specified by IC_STRINGG_CTRL	*/

    sgw->Code = '\0';
    sgw->IEvent = ie;
    sgw->EditOp = EO_NOOP;	/* have done nothing yet	*/

    /* keystroke changes to SGWork	*/
    sgw->BufferPos = si->BufferPos;
    sgw->NumChars = si->NumChars;
    sgw->LongInt = si->LongInt;

    sgw->Actions = SGA_USE;

    if ( g->Activation & LONGINT )
    {
	/* I&I trim your specification	*/
	D( printf("sI: longint gadget\n") );
	if ( si->MaxChars > LONGBSIZE ) si->MaxChars = LONGBSIZE;
	sgw->Modes |= SGM_LONGINT;
	sgw->WorkBuffer = IBase->LongBuff;   /* use internal buffer */
	sgw->Modes &= ~SGM_NOWORKB;
    }

    /* use extended info, if available	*/
    if ( sex )
    {
	sgw->Modes |= (sex->InitialModes &
	    (SGM_EXITHELP|SGM_REPLACE|SGM_FIXEDFIELD|SGM_NOFILTER));
	if (sex->WorkBuffer)
	{
	    sgw->WorkBuffer = sex->WorkBuffer;
	    sgw->Modes &= ~SGM_NOWORKB;
	}
    }
#if REPLACETEST
    /* DEBUG DEBUG everybody replace	*/
    sgw->Modes |= SGM_REPLACE;
    /* sgw->Modes |= SGM_FIXEDFIELD; */
#endif
}

/*
 * take action based on resulting sgwork.Actions
 * set *redisplay, *gadgetupcode.
 * returns proper string HANDLEINPUT or GOACTIVE hook
 *  return value
 */
sgAction( sgw, redisplay, gadgetupcode )
struct SGWork	*sgw;
BOOL		*redisplay;
LONG		*gadgetupcode;
{
    struct StringInfo	*si = sgw->StringInfo;
    int			siret = 0;
    struct IntuitionBase	*IBase = fetchIBase();

    if ( sgw->Actions & SGA_USE )
    {
	D( printf("sgA: SGA_USE\n"));
	if ( !( sgw->Modes & SGM_NOWORKB ) )
	{
	    CopyMem( sgw->WorkBuffer, SI( sgw->Gadget )->Buffer,
		    sgw->NumChars+1 );
	}
	D( printf("sgA: buffer %s\n", si->Buffer) );
	si->NumChars = sgw->NumChars;
	si->BufferPos = sgw->BufferPos;
	if ( sgw->Modes & SGM_LONGINT )
	{
	    si->LongInt = sgw->LongInt;
	}
	/*ZZZ: handle mode	*/
    }
    if ( sgw->Actions & SGA_END )
    {
	D( printf("sgA: SGA_END, code %lx\n", sgw->Code));
	if ( gadgetupcode )*gadgetupcode = sgw->Code;
	siret = GMR_VERIFY | ((sgw->Actions & SGA_REUSE)? GMR_REUSE: GMR_NOREUSE);
	if (sgw->Actions & SGA_NEXTACTIVE)
	{
	    siret |= GMR_NEXTACTIVE;
	}
	if (sgw->Actions & SGA_PREVACTIVE)
	{
	    siret |= GMR_PREVACTIVE;
	}

    }
    if ( sgw->Actions & SGA_BEEP )
    {
	D( printf("sgA: SGA_BEEP\n") );
	DisplayBeep( IBase->ActiveScreen );
    }
    if ( sgw->Actions & SGA_REDISPLAY )
    {
	*redisplay = TRUE;
    }
    return ( siret );
}

/* handles RAWKEY downstroke events
 * to be courteous to our custom hook people, let's always
 * leave SGA_USE set; so if you see something you don't like,
 * copy back from Buffer to WorkBuffer so WB is valid
 */
iEdit( hook, sgw )
CPTR		hook;	/* passed via hook interface	*/
struct SGWork	*sgw;
{
    UWORD 		icode;
    UWORD		iqual;

    struct StringInfo	*si = SI( sgw->Gadget );
    BOOL		handle_char = FALSE;	/* treat as normal char */
    struct IntuitionBase	*IBase = fetchIBase();
    UWORD		fixedfield_adjust;	/* 1 if fixedfield, else 0*/
    int			shifty;

    fixedfield_adjust = (sgw->Modes & SGM_FIXEDFIELD)? 1: 0;

    /* First handle scancoded editing keys	*/
    sgw->Actions |= SGA_REDISPLAY;

    D( printf("iE: iecode: %lx\n", sgw->IEvent->ie_Code ) );

    /* Some space-saving short-cuts */
    icode = sgw->IEvent->ie_Code;
    iqual = sgw->IEvent->ie_Qualifier;
    shifty = TESTFLAG( iqual, SHIFTY );

    /* check (unmapped) editing keys	*/
    /* Gather all unmapped codes that can end the gadget to save space.
     * <ENTER> and <RETURN> end the gadget, with code = 0.
     * <HELP> ends the gadget with code = 0x5F (rawkey code for help)
     * if this gadget is SGM_EXITHELP.
     * If a tab (without left-Amiga) was pressed, and this gadget
     * is GFLG_TABCYCLE , then return SGA_NEXTACTIVE or SGA_PREVACTIVE
     */
    if ( ( ( ( icode == 0x43 ) || ( icode == 0x44 ) ) && /* Enter or Return */
	!TESTFLAG( iqual, IEQUALIFIER_REPEAT ) ) ||
	( ( icode == KEYCODE_HELP ) && TESTFLAG( sgw->Modes, SGM_EXITHELP ) ) ||
	( ( icode == 0x42 ) && TESTFLAG( sgw->Gadget->Flags, GFLG_TABCYCLE ) &&
	    !TESTFLAG( iqual, AMIGALEFT ) ) )
    {
	D( printf( "Enter/Return/Help/Tab\n") );
	sgw->EditOp = EO_ENTER;
	sgw->Code = 0;		/* compatible with V1.2	*/
	sgw->Actions |= SGA_END;
	if ( icode == KEYCODE_HELP )
	{
	    sgw->Code = KEYCODE_HELP;
	    D( printf( "Help returning code %lx\n", sgw->Code) );
	}
	else if ( icode == 0x42 )
	{
	    sgw->Code = 0x09;
	    /* These will be converted to GMR.  There, GMR_PREVACTIVE 
	     * and GMR_NEXTACTIVE imply GMR_REUSE
	     */
	    sgw->Actions |= ( shifty ? SGA_PREVACTIVE : SGA_NEXTACTIVE );
	    D( printf( "Tab returning code %lx\n", sgw->Code) );
	}
    }
    else
    {
	switch ( icode )
	{
	case 0x41:	/* Backspace */
	    stringBackspace( sgw, shifty ? sgw->BufferPos: 1);
	    break;

	case 0x46: /* Delete */
	    stringDelete( sgw, shifty );
	    break;

	case 0x4E: /* Cursor Right */
	    /* next character or end of string */
	    sgw->EditOp = EO_MOVECURSOR;
	    if ( shifty )
	    {
		/* with shift key set, skip to end of string */
		sgw->BufferPos = sgw->NumChars - fixedfield_adjust;
	    }
	    else
	    {
		/* just advance one character */
		if (sgw->BufferPos < (sgw->NumChars - fixedfield_adjust) )
		{
		    sgw->BufferPos++;
		}
	    }
	    break;

	case 0x4F: /* Cursor Left */
	    /* previous character or beginning of string */
	    sgw->EditOp = EO_MOVECURSOR;
	    if ( shifty )
	    {
		/* with shift key set, skip to start of string */
		sgw->BufferPos = 0;
	    }
	    else
	    {
		/* just advance one character */
		if (sgw->BufferPos) sgw->BufferPos--;
		D( printf("new BP: %ld\n", sgw->BufferPos) );
	    }
	    break;

	default:
	    /* check amiga-Q, amiga-X through keymap now
	     * jimm: 5/13/90
	     */
	    if ( iqual & AMIGARIGHT )
	    {
		UBYTE *from;
		UBYTE ucode;

		if ( (ucode = ToUpper( sgw->Code )) == 'Q' )
		{

		    sgw->EditOp = EO_RESET;

		    /* is there an undo buffer? */
		    if (from = si->UndoBuffer)
		    {
			CopyMem(from, sgw->WorkBuffer, si->MaxChars );
			sgw->BufferPos = si->UndoPos;
			sgw->NumChars = strlen(sgw->WorkBuffer);
		    }
		    break;
		}
		else if ( ucode == 'X' )
		{
		    clearSGW( sgw );
		    /*  handle_char = FALSE (already)*/
		    break;
		}
	    }

	    handle_char = TRUE;

	    if ( !  (   TESTFLAG( sgw->Modes, SGM_NOFILTER )
		    || !TESTFLAG( IBase->NewIControl, IC_STRINGG_CTRL )
		    ||  TESTFLAG( sgw->Code, 0x60 )   /* is not control char */
		    ))
	    {
		handle_char = FALSE;	/* swallow all	*/

		/* filter control chars here	*/
		/* if AMIGALEFT is pressed, allow then through	*/
		if (  TESTFLAG( iqual, AMIGALEFT ) )
		{
		    handle_char = TRUE;
		}
		else
		{
		    switch ( sgw->Code )
		    {
		    case 0x08:		/* ^H=backspace	*/
			stringBackspace( sgw, 1 );
			break;

		    case 0x17:		/* ^W=del_word	*/
			delWord( sgw );
			break;

		    case 0x15:		/* ^U=del_begin	*/
			stringBackspace( sgw, sgw->BufferPos );
			break;

		    case 0x0B:		/* ^K=del_end	*/
			stringDelete( sgw, TRUE );	/* to end	*/
			break;

		    case 0x01:		/* ^A=jump_begin*/
			sgw->EditOp = EO_MOVECURSOR;
			sgw->BufferPos = 0;
			break;

		    case 0x1A:		/* ^Z=jump_end	*/
			sgw->EditOp = EO_MOVECURSOR;
			sgw->BufferPos = sgw->NumChars - fixedfield_adjust;
			break;

		    case 0x18:		/* ^X=clear	*/
			clearSGW( sgw );
			break;

		    case 0x0D:		/* ^M=return 	*/
			sgw->EditOp = EO_ENTER;
			sgw->Code = 0;		/* compatible with V1.2	*/
			sgw->Actions |= SGA_END;
			break;
		    }
		}
	    }
	}
    }

    /* handle translation codes	*/
    D( printf("iE: handle_char: %lx, code: %lx\n", handle_char, sgw->Code));

    if ( handle_char && sgw->Code )
    {
	/* there's some character to insert at current pos */

	/* are we out of room?	*/
	if ((sgw->NumChars >= si->MaxChars - 1)
	    && ( !TESTFLAG( sgw->Modes, SGM_REPLACE) 
		|| sgw->NumChars == sgw->BufferPos ) )
	{
	    D( printf("no room at inn: numc: %ld, mc %ld\n",
		sgw->NumChars, si->MaxChars) );
	    /* oops, no room left at the inn! */
	    /* sgw->EditOp = EO_NOOP; already set to this */
	    sgw->Actions |= SGA_BEEP;
	    sgw->Actions &= ~SGA_REDISPLAY;
	}
	else	/* can handle a new character	*/
	{
	    int	cursor_bump, ii;
	    cursor_bump = 1;	/* I'll hold still at end of fixedfield */

	    if ( ! TESTFLAG( sgw->Modes, SGM_REPLACE ) )
	    {
		/* insert a new character	*/
		sgw->EditOp = EO_INSERTCHAR;

		for (ii = sgw->NumChars; ii >= sgw->BufferPos; ii--)
		{
		    sgw->WorkBuffer[ii + 1] = sgw->WorkBuffer[ii];
		}
		sgw->NumChars++;
	    }
	    else if  ((sgw->BufferPos == sgw->NumChars ) &&
		!(sgw->Modes & SGM_FIXEDFIELD) )
	    {
		/*  extend with new chars at end	*/
		sgw->EditOp = EO_INSERTCHAR;

		sgw->NumChars++;
		sgw->WorkBuffer[ sgw->NumChars ] = '\0';
	    }
	    else if ( (sgw->Modes & SGM_FIXEDFIELD) && 
			(sgw->BufferPos == sgw->NumChars - 1 ) )
	    {
		sgw->EditOp = EO_REPLACECHAR;
		/* replace last character, but don't extend */
		cursor_bump = 0;
	    }
	    else
	    {
		sgw->EditOp = EO_REPLACECHAR;
	    }

	    sgw->WorkBuffer[sgw->BufferPos] = sgw->Code;
	    sgw->BufferPos += cursor_bump;
	}

    }

    /* handle longint	*/
    if ( TESTFLAG( sgw->Modes, SGM_LONGINT ) )
    {
	ULONG		tmplong;		/* for LONGINT	*/

	if ( LONGTest( sgw->WorkBuffer, &tmplong ) )
	{
	    /* legal int in there, use it	*/
	    sgw->LongInt  = tmplong;
	}
	else
	{
	    sgw->EditOp = EO_BADFORMAT;
	    /* buffer isn't valid.
	     * since we always want to pass SGA_USE to custom
	     * editing hook, copy back valid previous value and set
	     * beep.
	     */
	    sgw->Actions |= SGA_BEEP;

	    /* note well that it is guaranteed that separater workbuffer
	     * exists and long enough for NumChars+1
	     */
	    CopyMem( sgw->PrevBuffer, sgw->WorkBuffer, sgw->NumChars+1 );
	    sgw->NumChars = sgw->StringInfo->NumChars;
	    sgw->BufferPos = sgw->StringInfo->BufferPos;
	}
    }
}

/*
 * delete back from current position to
 * beginning of word
 */
delWord( sgw )
struct SGWork	*sgw;
{
    int	numkill = 0;
    UBYTE	*cptr;

    if ( sgw->BufferPos == 0 )
    {
	sgw->Actions &= ~SGA_REDISPLAY;
    }
    else
    {
	/* first char which I'll surely delete	*/
	cptr = sgw->WorkBuffer + sgw->BufferPos - 1;

	/* kill all immediately preceding spaces	*/
	while ( cptr >= sgw->WorkBuffer && *cptr == ' ' )
	{
	    numkill++; cptr--;
	}

	/* kill all non-space chars up to earlier space  */
	while ( cptr >= sgw->WorkBuffer && *cptr != ' ' )
	{
	    numkill++; cptr--;
	}
	stringBackspace( sgw, numkill );
    }
}

clearSGW( sgw )
struct SGWork	*sgw;
{
    if ( TESTFLAG( sgw->Modes, SGM_FIXEDFIELD)  )
    {
	sgw->BufferPos = 0;
    }
    else
    {
	sgw->EditOp = EO_CLEAR;
	/* forget the undo buffer */
	sgw->WorkBuffer[0] = '\0';
	sgw->BufferPos = 0;
	sgw->NumChars = 0;
    }
}

stringDelete( sgw, del_to_end )
struct SGWork	*sgw;
BOOL		del_to_end;
{
    int	ii;

    /* a no-op for fixed field mode	*/
    if (sgw->BufferPos < sgw->NumChars &&
	!TESTFLAG( sgw->Modes, SGM_FIXEDFIELD ))
    {
	sgw->EditOp = EO_DELFORWARD;

	/* shift delete, ^K: delete forward to end of string */
	if ( del_to_end )
	{
	    sgw->WorkBuffer[ sgw->BufferPos ] = 0;
	    sgw->NumChars = sgw->BufferPos;
	}
	else
	{
	    for (ii = sgw->BufferPos; ii < sgw->NumChars; ii++)
	    {
		sgw->WorkBuffer[ii] = sgw->WorkBuffer[ii + 1];
	    }
	    sgw->NumChars--;
	}
    }
    else
    {
	sgw->Actions &= ~SGA_REDISPLAY;
    }
}

/*
 * pass this thing only good values of 'numkill'
 */
stringBackspace( sgw, numkill )
struct SGWork	*sgw;
{
    int 	i;

    sgw->EditOp = EO_DELBACKWARD;

    if (sgw->BufferPos <= 0)
    {
	sgw->Actions &= ~SGA_REDISPLAY;
	return;
    }

    if ( TESTFLAG( sgw->Modes, SGM_FIXEDFIELD)  )
    {
	sgw->BufferPos -= numkill;
    }
    else
    {
	for (i = sgw->BufferPos; i <= sgw->NumChars; i++)
	{
	    sgw->WorkBuffer[i - numkill] = sgw->WorkBuffer[i];
	}
	sgw->BufferPos -= numkill;
	sgw->NumChars -= numkill;
    }
}

/* interface to iHandleMouse used by stringGoActive	*/
iMouse( ie, g, gi, sex, redisplay, gbox )
struct InputEvent	*ie;
struct Gadget 		*g;
struct GadgetInfo	*gi;
struct StringExtend	*sex;
BOOL			*redisplay;
struct IBox		*gbox;
{
    struct SGWork	sgwork;

    setupSGWork( &sgwork, g, gi, getSEx( g ), ie );
    return ( iHandleMouse( &sgwork, redisplay, gbox ) );
}

/* returns same values as stringInput	*/
iHandleMouse( sgw, redisplay, gbox )
struct SGWork		*sgw;
BOOL			*redisplay;
struct IBox		*gbox;
{
    struct InputEvent	*ie = sgw->IEvent;
    struct Gadget 	*g = sgw->Gadget;
    struct Point	mouse;
    int			siret =  GMR_MEACTIVE;
    WORD		bpos;
    struct StringExtend	*sex;

    switch ( ie->ie_Code )
    {
    case SELECTDOWN:
	/* make sure he clicked in the gadget	*/
	if ( hitGGrunt( g, sgw->GadgetInfo, gbox, GM_HITTEST ) )
	{
	    /* copy string to workbuffer, in case he wants to change it	*/
	    /* we only do this overhead when we intend to use it	*/
	    if (sgw->PrevBuffer != sgw->WorkBuffer)
	    {
		CopyMem( sgw->PrevBuffer, sgw->WorkBuffer,
		    sgw->NumChars+1);
	    }

	    /* get Intuition's idea of the SGWork	*/
	    SETFLAG( sgw->Actions, SGA_USE+SGA_REDISPLAY );

	    /* mouse is relative to select box */
	    gadgetMouse( g, sgw->GadgetInfo, &mouse, gbox );
	    bpos = sgw->BufferPos = stringClick( g, sgw->GadgetInfo, mouse.X);

	    /* now give custom hook the opportunity to mess with it */
	    if ( (sex = getSEx(g))  && sex->EditHook &&
		callHook( sex->EditHook, sgw, SGH_CLICK ) )
	    {
		siret = sgAction( sgw, redisplay, NULL );
	    }
	    else	/* don't trust his sgwork	*/
	    {
		/* my action is simpler	*/
		SI( g )->BufferPos = bpos;	/* use new pos that I calc  */
		/* always redisplay	*/
		*redisplay = TRUE;
	    }

	    return ( siret );
	}

	/* no break; */
	/** fall through if select is outside	**/
    case MENUDOWN:
	return ( GMR_REUSE );		/* terminate without verification */

    default:
	/* ignore everything else */
	return ( GMR_MEACTIVE );
    }
}

/* return buffer position for new cursor position
 * note new use of CLeft as keeping "leftedge" of text,
 * set up in displayString().
 */
stringClick( g, gi, clickx)
struct Gadget 		*g;
struct GadgetInfo	*gi;
int			clickx;		/* xcoord, rel. to select box	*/
{
    /* note: i don't render through this rastport, so
     * i don't care that the layer hasn't been set up yet.
     * i also assume that i can set its font without
     * saving the old one.
     */
    extern struct IFontPrefs topazprefs;
    struct IntuitionBase	*IBase = fetchIBase();
    struct StringInfo *si = (struct StringInfo *) g->SpecialInfo;
    int		index;
    struct RastPort clonerp;
    /* first visible char in string */
    UBYTE	*dpos = &si->Buffer[si->DispPos];
    struct	TextFont *fp = NULL;
    struct TextExtent dummy_te;

    /* get a copy of the (screen) rastport for TextLength
     * purposes.  Note that I don't need to worry about the
     * layer.
     */
    cloneRP( &clonerp, gi->gi_RastPort );

    if ( IBase->ActiveSEx && IBase->ActiveSEx->Font )
    {
	SetFont( &clonerp, IBase->ActiveSEx->Font );
    }

    if ( IBase->ActiveGBox.Height < clonerp.TxHeight )
    {
	DC( printf("gadget height %ld smaller than TxHeight %ld\n",
		IBase->ActiveGBox.Height, clonerp.TxHeight ) );

	/* Slam down to topaz 8	*/
	fp = ISetFont( &clonerp, &topazprefs.ifp_TABuff.tab_TAttr );
    }

    DC( printf("sC: disppos: %ld\n", si->DispPos) );
    /* ZZZ: SET FONT */

    /* get clickx relative to left edge of text */
    clickx -= si->CLeft;

    DC( printf("clickx = %ld\n", clickx) );

    /* We are to return the index within the buffer of the character
     * clicked on.  If the click was to the right of the rightmost
     * displayed character, we usually return the index of that
     * rightmost character.  But if that character is the last
     * one in the string, we put the cursor one step after the
     * last character.  Why?  Why not?  I didn't worry about it,
     * I just did it because it's always been that way in the old code.
     */
    index = 0;
    if ( clickx > 0 )
    {
	/* How many characters fit entirely to the left of the cursor? */
	index = TextFit( &clonerp, dpos, si->NumChars - si->DispPos,
	    &dummy_te, NULL, 1, clickx, 32767 );
	if ( index >= si->DispCount )
	{
	    index = si->DispCount - 1;
	    if ( ( si->DispPos + index ) == ( si->NumChars - 1 ) )
	    {
		index++;
	    }
	}
    }
    ICloseFont( fp );	/* no-op if unused (NULL)	*/

    return( index + si->DispPos );
}


/* 
 * gets as much long word out of the buffer as makes
 * LONG sense.  returns true if no problems, buffer contains
 * legal value.
 */
BOOL LONGTest(buffer, longvar)
UBYTE *buffer;
LONG *longvar;
{
    int sign;
    BOOL inrange;
    LONG total;
    BYTE digit;

    D( printf("LONGTest %ld\n", *longvar) );
    
    sign = -1;		/* gather negative number	*/

    if (*buffer == '-')
    {
	sign = 1;
	buffer++;
    }
    else if (*buffer == '+')
    {
	buffer++;
    }

    inrange = TRUE;
    total = 0;
    while ( *buffer )
    {
	/* must be a digit	*/
	if ( (digit = *buffer - '0') < 0 || digit > 9 )
	{
	    inrange = FALSE;
	    break;
	}

	if ((total < -214748364) || ((total == -214748364) && (digit > 8)))
	{
	    inrange = FALSE;
	    break;
	}
	else
	{
	    total = (total * 10) - digit;
	    buffer++;
	}
    }

    *longvar = total * sign;
    return ( inrange );
}

#if 0
copybuf(src, size, dest)
BYTE *src;
BYTE *dest;
int size;
{
    for ( ; size; size--) *dest++ = *src++;
}
#endif
/*** intuition.library/SetEditHook ***/

struct Hook	*
SetEditHook( edithook )
struct Hook	*edithook;
{
    struct IntuitionBase	*IBase = fetchIBase();
    struct Hook	*oldhook;

    D( printf("SEH: edithook: %lx\n", edithook ) );

    LOCKIBASE();
    oldhook = IBase->EditHook;
    IBase->EditHook =  edithook;
    UNLOCKIBASE();

    return ( oldhook );
}

