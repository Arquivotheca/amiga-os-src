/*** strgadclass.c *******************************************************
 *
 * strgadclass.c -- string gadget class
 *
 *  $Id: strgadclass.c,v 38.4 92/11/10 17:11:22 peter Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (C) 1989, Commodore-Amiga, Inc.
 *  All Rights Reserved
 *
 ****************************************************************************/

#include "intuall.h"
#include <exec/memory.h>
#include "classusr.h"
#include "classes.h"
#include "gadgetclass.h"
#include <utility/pack.h>

#define D(x)	;
#define DU(x)	;

/* new instance data for objects of strgadget class */
struct SGIData
{
    UWORD		sgi_Flags;
    struct StringInfo	sgi_SInfo;
    struct StringExtend	sgi_SEx;
};

#define SGIF_MYBUFFER	0x0001	/* I allocated the buffer */
#define SGIF_REFRESH	0x0002	/* I allocated the buffer */

#define sgiSINFO( sgid ) 	(&sgid->sgi_SInfo)
#define sgiSEX( sgid ) 	(&sgid->sgi_SEx)   /* string extend instance data */
#define G(o)		((struct Gadget *) (o))		/* base class */

Class	*
initStrGadgetClass()
{
    ULONG strgadgetDispatch();
    Class	*makePublicClass();
    void	*sharedbuffs;
    Class	*myclass;
    void	*AllocMem();
    void	*AllocVec();
    extern UBYTE	*StrGClassName;
    extern UBYTE	*GadgetClassName;

    /*  allocate some default shared buffers first */
    D( printf("initSGC\n"));
    if ( !( sharedbuffs = AllocMem( 2 * SG_DEFAULTMAXCHARS, MEMF_CLEAR ) ) )
    {
	D( printf("no shared buffs\n"));
	return ( NULL );
    }

    myclass = makePublicClass(StrGClassName, GadgetClassName,
		sizeof(struct SGIData),strgadgetDispatch);

    if ( myclass )
    {
	myclass->cl_UserData = (ULONG) sharedbuffs;
    }
    else
    {
	FreeMem( sharedbuffs, 2 * SG_DEFAULTMAXCHARS );
    }
    D( printf("initStrGadgetClass returning %lx\n", myclass ) );

    return ( myclass );
}

/*
 * The main dispatcher (or hook) for this class of custom gadgets
 */
ULONG
strgadgetDispatch( cl, o, msg )
Class   *cl;
Object  *o;
Msg     msg;
{
    struct IntuitionBase	*IBase = fetchIBase();
    Object  		*newobj;
    struct SGIData	*sgi;
    struct Gadget	*g;
    LONG		oldlongint;
    ULONG		retval;

    sgi = INST_DATA( cl, o );

    switch ( msg->MethodID )
    {
    case OM_NEW:
	D( printf("strgclass got OM_NEW\n") );
	if ( newobj = (Object *) SSM( cl, o, msg ) )
	{
	    sgi = INST_DATA( cl, newobj );

	    SetSuperAttrs( cl, newobj,
		GA_SPECIALINFO,	sgiSINFO(sgi),
		TAG_END );

	    G(newobj)->Activation |= STRINGEXTEND;

	    /* ---- init the instance data of this class ---- */
	    sgiSINFO(sgi)->Extension = sgiSEX( sgi );

	    /* set up pointers to buffers, which can be
	     * overridden if the user wants longer ones.
	     */
	    sgiSINFO(sgi)->MaxChars = GetUserTagData0( STRINGA_MaxChars,
		((struct opSet *)msg)->ops_AttrList );

	    if ( sgiSINFO(sgi)->MaxChars < 1 )
	    {
		sgiSINFO(sgi)->MaxChars = 1;	/* lame default */
	    }

	    sgiSINFO(sgi)->Buffer = (UBYTE *) GetUserTagData0( STRINGA_Buffer,
		((struct opSet *)msg)->ops_AttrList );

	    if ( ! sgiSINFO(sgi)->Buffer )
	    {
		/* NOTE VERY CAREFULLY:  For LongInt gadgets,
		 * setupSGWork() trims MaxChars to be no bigger
		 * than LONGBSIZE.  We used to use AllocMem() here,
		 * and later FreeMem() according to MaxChars.  This
		 * could result in a mismatched FreeMem().  Using
		 * AllocVec()/FreeVec() protects us from that.
		 */
		if ( !(sgiSINFO(sgi)->Buffer = 
		    AllocVec( sgiSINFO(sgi)->MaxChars, MEMF_CLEAR)))
		{
		    D( printf( "couldn't get stringo buffer\n") );
		    SendSuperMessage( cl, newobj, OM_DISPOSE );
		    return ( NULL );
		}
		SETFLAG( sgi->sgi_Flags, SGIF_MYBUFFER );
	    }

	    /* if I've got enough space in my per-class default buffers,
	     * provide them as defaults.
	     */
	    if ( sgiSINFO(sgi)->MaxChars <= SG_DEFAULTMAXCHARS )
	    {
		D( printf("undobuffer shared at %lx\n", cl->cl_UserData));
		sgiSINFO(sgi)->UndoBuffer = (UBYTE *) cl->cl_UserData;

		/* second half of allocated space is workbuffer */
		D( printf("workbuffer shared at %lx\n",
			(UBYTE *) cl->cl_UserData + SG_DEFAULTMAXCHARS ));

		sgiSEX(sgi)->WorkBuffer =
			(UBYTE *) cl->cl_UserData + SG_DEFAULTMAXCHARS;
	    }

	    sgiSEX(sgi)->Pens[0] = 1;
	    sgiSEX(sgi)->Pens[1] = 0;
	    sgiSEX(sgi)->ActivePens[0] = 1;
	    sgiSEX(sgi)->ActivePens[1] = 0;

	    /* set initial attributes, for THIS class, not 'true' class */
	    setStrGadgetAttrs( cl, newobj, msg );
	}
	D( printf("strgadclass om_new returning %lx\n", newobj ) );
	return ( (ULONG) newobj );

    case OM_SET:
	D( printf("sgc gets OM_SET\n"));
    case OM_UPDATE:
	{
	    ULONG		refresh;
	    struct RastPort	*rp;

	    refresh =  SSM( cl, o, msg );
	    refresh +=  setStrGadgetAttrs( cl, o, msg );

	    DU( printf("sgclass, update/set, refresh is %lx\n", refresh));

	    /* if I'm the true class, then handle refreshing of visuals */
	    if ( refresh && ( OCLASS(o) == cl ) )
	    {
		rp = ObtainGIRPort( ((struct opSet *)msg)->ops_GInfo );
		if ( rp ) 
		{
		    /* come back through myself */
		    CoerceMessage( cl, o, GM_RENDER,
			((struct opSet *)msg)->ops_GInfo, rp, GREDRAW_UPDATE );
		}
		ReleaseGIRPort( rp );
		return ( 0 );		/* don't need refresh any more */
	    }
	    /* else */
	    return ( (ULONG) refresh );	/* return to subclass perhaps	*/
	}

    case OM_GET:
	return ( getStrGadgetAttrs( cl, o, msg ) );

    case GM_HITTEST:
    case GM_RENDER:
    case GM_GOINACTIVE:
	/* This is what it's all about: here you "demote" one of
	 * these fancy class objects into a plain old Intuition
	 * boolean gadget by using the stripped down boolean hooks.
	 *
	 * This is why objects have their secret header instance
	 * data before the gadget/object address.
	 *
	 * This is why Hooks have a MinNode so that a Hook
	 * pointer and a class pointer are the same.
	 */
	D( printf("strgadclass pass to phooks\n") );
	return ( callHookPkt( &IBase->IIHooks[ IHOOK_STRINGG ], o, msg ) );

	/* ZZZ: separate GADGETUP notification of change? */

    case GM_GOACTIVE:	/* stash original top value */
    case GM_HANDLEINPUT:
	/* save "before" values */
	/* ZZZ: how am I going to know if string changed,
	 * in an efficient way?  ??  Maybe only do it on
	 * endgadget condition.
	 *
	 * might have to stash original value in GoActive
	 */
	retval = callHookPkt( &IBase->IIHooks[ IHOOK_STRINGG ], o, msg );

	/* only notify changes when I'm done	*/
	if (  retval != GMR_MEACTIVE )
	{
	    DU( printf("sgclass HI, not active\n"));
	    if ( TESTFLAG( G(o)->Activation, LONGINT ) )
	    {
		NotifyAttrChanges( o, ((struct gpInput *)msg)->gpi_GInfo, 0,
		    GA_ID, G(o)->GadgetID,
		    STRINGA_LongVal, sgiSINFO(sgi)->LongInt,
		    TAG_END );
	    }
	    else
	    {
		/* don't worry about detecting change */
		NotifyAttrChanges( o, ((struct gpInput *)msg)->gpi_GInfo, 0,
		    GA_ID, G(o)->GadgetID,
		    STRINGA_TextVal, sgiSINFO(sgi)->Buffer,
		    TAG_END );
	    }
	}

	return ( retval );
    
    case OM_DISPOSE:
	if ( TESTFLAG( sgi->sgi_Flags, SGIF_MYBUFFER ) )
	{
	    /* NOTE VERY CAREFULLY:  For LongInt gadgets,
	     * setupSGWork() trims MaxChars to be no bigger
	     * than LONGBSIZE.  We used to AllocMem() the buffer
	     * and later FreeMem() it here according to MaxChars.
	     * This could result in a mismatched FreeMem().  Using
	     * AllocVec()/FreeVec() protects us from that.
	     */
	    FreeVec( sgiSINFO(sgi)->Buffer );
	}
	/* fall through */

    default:
	return ( SSM( cl, o, msg ) );	/* let the superclass handle it */
    }
}

getStrGadgetAttrs( cl, o, msg )
Class		*cl;
Object		*o;
struct opGet	*msg;
{
    struct SGIData 	*sgi;
    sgi = INST_DATA( cl, o );

    switch ( msg->opg_AttrID )
    {
    case STRINGA_TextVal:
	*msg->opg_Storage = (ULONG) sgiSINFO(sgi)->Buffer;
	break;
    case STRINGA_LongVal:
	*msg->opg_Storage = (ULONG) sgiSINFO(sgi)->LongInt;
	break;

    /* let the superclass try	*/
    default:
	return ( SSM( cl, o, msg ) );
    }
    return ( 1 );	/* found it */
}

ULONG stringPackTable[] =
{
    PACK_STARTTABLE( STRINGA_Dummy ),
    PACK_ENTRY( STRINGA_Dummy, STRINGA_EditModes, SGIData, sgi_SEx.InitialModes, PKCTRL_ULONG ),
    PACK_LONGBIT( STRINGA_Dummy, STRINGA_ReplaceMode, SGIData, sgi_SEx.InitialModes, PKCTRL_BIT, SGM_REPLACE ),
    PACK_LONGBIT( STRINGA_Dummy, STRINGA_FixedFieldMode, SGIData, sgi_SEx.InitialModes, PKCTRL_BIT, SGM_FIXEDFIELD ),
    PACK_LONGBIT( STRINGA_Dummy, STRINGA_NoFilterMode, SGIData, sgi_SEx.InitialModes, PKCTRL_BIT, SGM_NOFILTER ),
    PACK_LONGBIT( STRINGA_Dummy, STRINGA_ExitHelp, SGIData, sgi_SEx.InitialModes, PKCTRL_BIT, SGM_EXITHELP ),
    PACK_ENTRY( STRINGA_Dummy, STRINGA_UndoBuffer, SGIData, sgi_SInfo.UndoBuffer, PKCTRL_ULONG ),
    PACK_ENTRY( STRINGA_Dummy, STRINGA_WorkBuffer, SGIData, sgi_SEx.WorkBuffer, PKCTRL_ULONG ),
    PACK_ENTRY( STRINGA_Dummy, STRINGA_EditHook, SGIData, sgi_SEx.EditHook, PKCTRL_ULONG ),
    PACK_ENTRY( STRINGA_Dummy, STRINGA_AltKeyMap, SGIData, sgi_SInfo.AltKeyMap, PKCTRL_ULONG ),

    PACK_ENTRY( STRINGA_Dummy, STRINGA_BufferPos, SGIData, sgi_SInfo.BufferPos, PKCTRL_WORD ),
    PACK_WORDBIT( STRINGA_Dummy, STRINGA_BufferPos, SGIData, sgi_Flags, PSTF_EXISTS|PKCTRL_BIT, SGIF_REFRESH ),
    PACK_ENTRY( STRINGA_Dummy, STRINGA_DispPos, SGIData, sgi_SInfo.DispPos, PKCTRL_WORD ),
    PACK_WORDBIT( STRINGA_Dummy, STRINGA_DispPos, SGIData, sgi_Flags, PSTF_EXISTS|PKCTRL_BIT, SGIF_REFRESH ),
    PACK_ENTRY( STRINGA_Dummy, STRINGA_Font, SGIData, sgi_SEx.Font, PKCTRL_ULONG ),
    PACK_WORDBIT( STRINGA_Dummy, STRINGA_Font, SGIData, sgi_Flags, PSTF_EXISTS|PKCTRL_BIT, SGIF_REFRESH ),

    PACK_ENDTABLE,    
};


setStrGadgetAttrs( cl, o, msg )
Class		*cl;
Object		*o;
struct opSet	*msg;
{
    struct SGIData 	*sgi = INST_DATA( cl, o );

    struct TagItem	*NextTagItem();
    struct TagItem	*tags = msg->ops_AttrList;
    struct TagItem	*tstate = tags;
    struct TagItem	*tag;
    ULONG		tidata;

    BOOL		refresh;


    CLEARFLAG( sgi->sgi_Flags, SGIF_REFRESH );
    PackStructureTags( sgi, stringPackTable, tags );
    refresh = TESTFLAG( sgi->sgi_Flags, SGIF_REFRESH );

    if ( sgiSINFO(sgi)->AltKeyMap )
    {
	SETFLAG( G(o)->Activation, ALTKEYMAP );
    }
    else
    {
	CLEARFLAG( G(o)->Activation, ALTKEYMAP );
    }

    /* process rest */
    while ( tag = NextTagItem( &tstate ) )
    {
	tidata = tag->ti_Data;
	switch ( tag->ti_Tag )
	{
	case STRINGA_Pens:
	    sgiSEX(sgi)->Pens[0] = tidata & 0xf;
	    sgiSEX(sgi)->Pens[1] = (tidata >> 8) & 0xf;
	    refresh = 1;
	    break;

	case STRINGA_ActivePens:
	    sgiSEX(sgi)->ActivePens[0] = tidata & 0xf;
	    sgiSEX(sgi)->ActivePens[1] = (tidata >> 8) & 0xf;
	    refresh = 1;
	    break;

#define JUSTBITS	( STRINGLEFT | STRINGCENTER | STRINGRIGHT )
	case STRINGA_Justification:
	    G(o)->Activation = 
		( G(o)->Activation & ~JUSTBITS ) |
		( tidata & JUSTBITS );
	    refresh = 1;
	    break;
	    
	/* fancy ones: not only init's value, but sets type */
	case STRINGA_LongVal:
	    SETFLAG( G(o)->Activation, LONGINT );
	    sgiSINFO(sgi)->LongInt = (LONG) tidata;
	    if ( sgiSINFO(sgi)->Buffer )
	    {
		jsprintf( sgiSINFO(sgi)->Buffer, "%ld", tidata );
	    }

	    refresh = 1;
	    break;

	case STRINGA_TextVal:
	    CLEARFLAG( G(o)->Activation, LONGINT );
	    D( printf("sgi at %lx\n", sgi ));

	    if ( sgiSINFO(sgi)->Buffer )
	    {
		D( printf("strg copy  from %lx to %lx\n",
			tidata, sgiSINFO(sgi)->Buffer ));

		jstrncpy( sgiSINFO(sgi)->Buffer, tidata, 
			sgiSINFO(sgi)->MaxChars - 1 );
	    }
	    sgiSINFO(sgi)->LongInt = 0;
		    
	    refresh = 1;
	    break;
	}
    }

    return ( refresh );
}

