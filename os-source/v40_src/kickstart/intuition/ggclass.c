
/*** ggclass.c *******************************************************
 *
 * ggclass.c -- gadget group class
 *
 *  $Id: ggclass.c,v 38.0 91/06/12 14:18:52 peter Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (C) 1989, Commodore-Amiga, Inc.
 *  All Rights Reserved
 *
 ****************************************************************************/

#include "intuall.h"
#include "classusr.h"
#include "classes.h"
#include "gadgetclass.h"


#define D(x)	;


struct GGData {
	struct MinList	gg_List;	/* gadget group list		*/
	Object		*gg_Active;	/* active member, if any	*/

	/* some basic layout paramters, for default rules	*/
	WORD		gg_Spacing;
	WORD		gg_Orientation;	/* horiz/vert/freeform		*/
	Object		*gg_Layout;	/* layout operation delegate	*/
};

/* transparent base class	*/
#define G(o)	((struct Gadget *)(o))

Class	*
initGGClass()
{
    Class	*makePublicClass();
    ULONG 	ggDispatch();
    extern UBYTE	*GroupGClassName;
    extern UBYTE	*GadgetClassName;

    return (  makePublicClass( GroupGClassName, GadgetClassName,
    	sizeof (struct GGData), ggDispatch));
}


ULONG
ggDispatch( cl, o, msg )
Class   *cl;
Object  *o;
Msg     msg;
{
    Object  		*newobj;
    struct GGData	*gg;
    USHORT		ghreturn;

    /* for iterations	*/
    Object	*member;
    Object	*ostate;
    Object	*NextObject();

    gg = INST_DATA( cl, o );

    switch ( msg->MethodID )
    {
    case OM_NEW:
	if ( newobj = (Object *) SSM( cl, o, msg ) )
	{
	    gg = INST_DATA( cl, newobj );

	    /* init instance */
	    NewList( &gg->gg_List );
#if 0	/* 0 is now the default */
	    G(newobj)->LeftEdge = G(newobj)->TopEdge = 0;
#endif
	    G(newobj)->Width = G(newobj)->Height = 0;

	    setGGAttrs( cl, newobj, msg );
	}
	D( printf("ggclass returning %lx\n", newobj ));

	return ( (ULONG) newobj );

    case OM_SET:	
	return ( (ULONG) 
		SSM( cl, o, msg ) +
		setGGAttrs( cl, o, msg ) );

#if 0	/* use default: */
    case OM_GET:
	return ( SSM( cl, o, msg ) );
#endif

    case OM_DISPOSE:
	/* dispose members	*/
	ostate = gg->gg_List.mlh_Head;
	while ( member = NextObject( &ostate ) )
	{
	    SendMessage( member, OM_REMOVE );
	    SM( member, msg );			/* and dispose	*/
	}

	/* no dispose layout	*/

	/* dispose me	*/
	return ( (ULONG) SSM( cl, o, msg ) );

    case GM_HITTEST:
	return ( propagateHit( o, gg, msg ) );

    case GM_RENDER:
	/* ZZZ: propagate back to front? */
	D( printf("ggclass GM_RENDER\n"));
	ostate = gg->gg_List.mlh_Head;
	while ( member = NextObject( &ostate ) )
	{
	    D( printf("forward to %lx\n", member ) );
	    SM( member, msg );
	}
	break;

    case GM_GOACTIVE:
	/* refuse to active if I don't have a contestant	*/
	if ( !gg->gg_Active )
	{
	    return ( GMR_REUSE );
	}
	/* else */

	/* make mouse coordinates member-relative */
	translateCGPInput( o, gg->gg_Active, msg );
	if ( (ghreturn = SM( gg->gg_Active, msg )) == GMR_MEACTIVE )
	{
	    SETFLAG( G(gg->gg_Active)->Activation, ACTIVEGADGET );
	}

	return( ghreturn );

    case GM_GOINACTIVE:
	/* we can count on this being called if we returned GMR_MEACTIVE
	 * for GM_GOACTIVE
	 */
	if ( gg->gg_Active )
	{
	    SM( gg->gg_Active, msg );
	    CLEARFLAG( G(gg->gg_Active)->Activation, ACTIVEGADGET );
	    gg->gg_Active = NULL;
	}
	break;

    case GM_HANDLEINPUT:
	if ( gg->gg_Active )
	{
	    /* make mouse coordinates member-relative */
	    translateCGPInput( o, gg->gg_Active, msg );
	    return ( SM( gg->gg_Active, msg ) );
	}
	break;

    case OM_ADDMEMBER:
	/* (we'll remove using OM_SUBSTITUTE) */
	member = ((struct opAddMember *)msg)->opam_Object;
	D( printf("ggclass: OM_ADDMEMBER %lx\n", member ) );

	SendMessage( member, OM_ADDTAIL, &gg->gg_List );

	/* convert member relative coordinates to absolutes	*/
	D( printf("convert coords old %ld/%ld, new %ld/%ld\n",
		G(member)->LeftEdge,
		G(member)->TopEdge,
		G(o)->LeftEdge + G(member)->LeftEdge,
		G(o)->TopEdge + G(member)->TopEdge ) );

	/* expand my select box */
	/* doesn't handle negative positions	*/
	G(o)->Width = imax(G(o)->Width, G(member)->LeftEdge+G(member)->Width);
	G(o)->Height = imax(G(o)->Height,G(member)->TopEdge+G(member)->Height);

	/* convert member relative coordinates to absolutes	*/
	/* ZZZ: I don't have window/requester here ...		*/
	SetGadgetAttrs( member, NULL, NULL,
		GA_LEFT, G(member)->LeftEdge + G(o)->LeftEdge,
		GA_TOP, G(member)->TopEdge + G(o)->TopEdge,
		TAG_END );
	break;

    case OM_REMMEMBER:
	member = ((struct opAddMember *)msg)->opam_Object;
	SendMessage( member, OM_REMOVE, &gg->gg_List );
	/* * ZZZ: not recalculating gadget box	*/
	break;

#if 0
    /* Let the superclass handle these cases	*/
    case OM_UPDATE:
	/* DELEGATE this to specific object, probably in subclass.  */
    case OM_NOTIFY:
	/* let superclass update the gadget default target.  */
#endif
    default:
	return ( (ULONG) SSM( cl, o, msg ) );
    }
    return ( 1 );
}

/*
 * translate mouse position from my coords to member
 * also works for GM_GOACTIVE
 */
translateCGPInput( o, g, msg )
struct Gadget		*o;
struct Gadget		*g;
struct gpInput	*msg;
{
    /* offset by *relative* position of member within group,
     * and remember that member positions are in window-absolute coords
     */
    msg->gpi_Mouse.X -= g->LeftEdge - o->LeftEdge;
    msg->gpi_Mouse.Y -= g->TopEdge - o->TopEdge;
}

propagateHit( o, gg, msg )
Object			*o;
struct GGData		*gg;
struct gpHitTest	*msg;
{
    Object	*member;
    Object	*ostate;
    Object	*NextObject();

    struct IBox	gbox;

    int	origx;
    int origy;

    D( printf("ggclass hittest\n") );
    D( printf("mouse %ld/%ld\n",
	msg->gpht_Mouse.X, msg->gpht_Mouse.Y ) );

    /* stash the mouse original values which are
     * relative to group origin
     */
    origx = msg->gpht_Mouse.X + G(o)->LeftEdge;
    origy = msg->gpht_Mouse.Y + G(o)->TopEdge;

    ostate = gg->gg_List.mlh_Head;
    while ( member = NextObject( &ostate ) )
    {
	D( printf("hittest member %lx\n", member ) );

	/* get a box which is the dims of the member */
	gadgetBox( G(member), msg->gpht_GInfo, &gbox );
	gbox.Left = gbox.Top = 0;

	/* make mouse coordinates member-relative */
	msg->gpht_Mouse.X = origx - G(member)->LeftEdge;
	msg->gpht_Mouse.Y = origy - G(member)->TopEdge;

	D( printf("member-relative mouse %ld/%ld\n",
	    msg->gpht_Mouse.X, msg->gpht_Mouse.Y ) );

	if ( ( ptInBox( msg->gpht_Mouse, &gbox ) ) 
	      && ( SM( member, msg ) ) )
	{
	    D( printf("HIT member %lx\n", member ) );

	    /* if hit gadget is DISABLED, you done */
	    if ( G(member)->Flags & GADGDISABLED )
	    {
		return ( 0 );
	    }

	    gg->gg_Active = member;
	    return ( GADGETON );
	}
    }
    return ( 0 );
}

setGGAttrs( cl, o, msg )
Class		*cl;
Object		*o;
struct opSet	*msg;
{
    struct TagItem	*NextTagItem();
    struct TagItem	*tags = msg->ops_AttrList;
    struct TagItem	*tag;
    ULONG		tidata;
    int			deltaleft = 0;
    int			deltatop = 0;

    Object	*member;
    Object	*ostate;
    Object	*NextObject();

    struct GGData	*gg = INST_DATA( cl, o );

    /* delegate/propagate attributes	*/
    /* ZZZ: not sending to superclass.
     * need to cut in there and block position/dims
     */

    while ( tag = NextTagItem( &tags ) )
    {
	tidata = tag->ti_Data;
	switch ( tag->ti_Tag )
	{
	/* delegate to (optional) layout	*/
	case LAYOUTA_ORIENTATION:
	case LAYOUTA_SPACING:
	case LAYOUTA_LAYOUTOBJ:
	    /* ZZZ: delegate or do simple thing yourself */
	    break;

	/* propagate to members			*/
	case GA_LEFT:
	    deltaleft = ((int) tidata) - G(o)->LeftEdge;
	    break;
	case GA_TOP:
	    deltatop = ((int) tidata) - G(o)->TopEdge;
	    break;
	}
    }

    /* propagate position changes	*/
    if ( deltaleft || deltatop )
    {
	ostate = gg->gg_List.mlh_Head;
	while ( member = NextObject( &ostate ) )
	{
	    /* ZZZ: violation of transparent base class (LeftEdge, etc)?
	     * can't help it: I've got no OM_GET yet
	     * ZZZ: I think there should be a better way of
	     * propagating the GInfo to child members
	     */
	    SetGadgetAttrs( member,
		msg->ops_GInfo->gi_Window, msg->ops_GInfo->gi_Requester,
		GA_LEFT, G(member)->LeftEdge + deltaleft,
		GA_TOP, G(member)->TopEdge + deltatop,
		TAG_END
		);
	}
	G(o)->LeftEdge += deltaleft;
	G(o)->TopEdge += deltatop;
    }

    return ( 0 );	/* means "don't refresh" to gadgets */
}

