/*** gadclass.c **************************************************************
 *
 * gadclass.c -- gadget class
 *
 *  $Id: gadclass.c,v 38.11 93/01/08 14:44:00 peter Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (C) 1989, Commodore-Amiga, Inc.
 *  All Rights Reserved
 *
 ****************************************************************************/


#include <intuition/intuition.h>
#include "intuall.h"

#include "classusr.h"
#include "classes.h"
#include "gadgetclass.h"
#include "icclass.h"		/* gadgets have own interconnection */


#include <utility/pack.h>

#include "gadclass_protos.h"

/*---------------------------------------------------------------------------*/

static ULONG gadgetDispatch(Class * cl,
                            Object * o,
                            Msg msg);

static int setGadgetAttrs(Class * cl,
                          Object * o,
                          struct opSet * msg);

static int setGHigh(struct Gadget * g,
                    int highlight);

/*---------------------------------------------------------------------------*/


/* Shorthands for packing gadget entries */
#define GA_PACK_ENTRY(tag,field,control) PACK_ENTRY(GA_Dummy,tag,ExtGadget,field,control)
#define GA_PACK_WORDBIT(tag,field,flags) PACK_WORDBIT(GA_Dummy,tag,ExtGadget,field,PKCTRL_BIT,flags)
#define GA_PACK_NEGWORDBIT(tag,field,flags) PACK_WORDBIT(GA_Dummy,tag,ExtGadget,field,PKCTRL_FLIPBIT,flags)

#define GA_PACK_EXWORDBIT(tag,field,flags) PACK_WORDBIT(GA_Dummy,tag,ExtGadget,field,PSTF_EXISTS|PKCTRL_BIT,flags)
#define GA_PACK_EXNEGWORDBIT(tag,field,flags) PACK_WORDBIT(GA_Dummy,tag,ExtGadget,field,PSTF_EXISTS|PKCTRL_FLIPBIT,flags)

#define GA_PACK_LONGBIT(tag,field,flags) PACK_LONGBIT(GA_Dummy,tag,ExtGadget,field,PKCTRL_BIT,flags)


#define D(x)	;
#define DN(x)	;		/* debug notification mechanism	*/
#define D2(x)	;


/* instance data for gadget class */
struct GIData {
    struct ExtGadget	gid_Gadget;
    Object		*gid_Target;
    struct TagItem	*gid_Map;
};

Class	*
initGadgetClass()
{
    ULONG gadgetDispatch();
    Class	*makePublicClass();
    extern UBYTE	*GadgetClassName;
    extern UBYTE	*RootClassName;

    D( printf("initGadgetClass\n") );
    return (makePublicClass(GadgetClassName, RootClassName,
		sizeof(struct GIData),gadgetDispatch));
}


static ULONG
gadgetDispatch( cl, o, msg )
Class   *cl;
Object  *o;
Msg     msg;
{
    Object  		*newobj;
    struct GIData 	*gid;
    ULONG		retval = 0;
    struct Gadget	*g;
    struct Gadget	*prevg;

    /* for iterating the list of IC objects */
    Object  		*NextObject();

    /* won't be valid for OM_NEW */
    gid = INST_DATA( cl, o );
    g = &gid->gid_Gadget;

    D( printf("gadgetDispatch, class %lx, object %lx, msg %lx\n", cl,o,msg));

    switch ( msg->MethodID )
    {

    case OM_NEW:
	/* have "extra size" parameter */
	D( printf("\t\tgD: OM_NEW\n") );
	if ( newobj = (Object *) SSM( cl, o, msg ) )
	{
	    D( printf("gadget new: object at %lx\n,", newobj ) );
	    gid = INST_DATA( cl, newobj );
	    g = &gid->gid_Gadget;

	    D( printf("gadget new: gid at %lx, g at %lx\n", gid, g ) );

	    /* mandatory init */
	     /* random numbers */
#if 0	     /** these should default to zero ***/
	     g->LeftEdge= 50;
	     g->TopEdge = 30;
#endif

	     g->Width = 80;
	     g->Height = 40;

	    /* native class == hook */
	    g->GadgetType = CUSTOMGADGET;
	    /* We use MutualExclude as the gadget hook pointer */
	    g->MutualExclude = (LONG) o;

	    /* link in to previous gadget */
	    if ( prevg = (struct Gadget *) GetUserTagData0( GA_PREVIOUS,
				    ((struct opSet *)msg)->ops_AttrList ) )
	    {
		/* links self in	*/
		g->NextGadget = prevg->NextGadget;
		prevg->NextGadget = g;
	    }

	    setGadgetAttrs( cl, newobj, msg );
	}

	return ( (ULONG) newobj );

    case OM_UPDATE:
    case OM_SET:
	SSM( cl, o, msg );
	setGadgetAttrs( cl, o, msg );
	/* return 0, no refresh	*/
	break;

    case OM_GET:
#if 1
	return ( 0 );	/* see comment for getGadgetAttr */
#else
	return ( getGadgetAttr( cl, o, msg ) );
#endif

    case OM_NOTIFY:
	/* map changed attributes and deliver to target	
	 * in the form of OM_UPDATE.  Use the same message, since
	 * there is some other stuff in there (ginfo).
	 * OK to corrupt the attlist, since I created it
	 * my ownself.
	 */
	DN( printf("gadget notification\n"));
	if ( gid->gid_Target )
	{
	    DN( printf("target at %lx, map at %lx\n",
		gid->gid_Target, gid->gid_Map ) );
	    /* map my changed attributes	*/
	    MapTags( ((struct opSet *)msg)->ops_AttrList, gid->gid_Map,
		MAP_KEEP_NOT_FOUND );

	    /* convert from OM_NOTIFY to OM_UPDATE	*/
	    msg->MethodID = OM_UPDATE;

	    /* deliver to target	*/
	    /* special target value of ~0 means send to window */
	    if ( (ULONG) gid->gid_Target == ~0 )
	    {
		DN( printf("gadclass calls sendNotify\n"));
		sendNotifyIDCMP( msg );
	    }
	    else
	    {
		SM( gid->gid_Target, msg );
	    }
	}
	break;

    case GM_HITTEST:
	/* accept hits, and swallow them: I am not transparent */
        retval = GMR_GADGETHIT;
	break;


    case GM_HELPTEST:
	/* accept GM_HELPTEST */
	retval = GMR_HELPHIT;
	break;

    case GM_RENDER:
	/* This base class does nothing like render */
	retval = 0;
	break;

    case GM_GOACTIVE:
	/* refuse to go active */
	return ( GMR_NOREUSE );

    case GM_GOINACTIVE:
	break;

    case GM_HANDLEINPUT:
	/* swallow whatever he sends me (selectdown)
	 * and get out of here.  I shouldn't see this ever.
	 */
	return ( GMR_NOREUSE );

    case OM_DISPOSE:
	/* do not free target, nor the map list	*/
	return ( SSM( cl, o, msg ) );	/* free super */
	break;

    default:
	D( printf("gD: default case\n") );
	return ( SSM( cl, o, msg ) );
    }

    return ( retval );
}

ULONG gadgetPackTable[] =
{
    PACK_STARTTABLE( GA_Dummy ),

    /* Gadget Activation flags: */
    GA_PACK_WORDBIT( GA_ENDGADGET, Activation, ENDGADGET ),
    GA_PACK_WORDBIT( GA_IMMEDIATE, Activation, GADGIMMEDIATE ),
    GA_PACK_WORDBIT( GA_RELVERIFY, Activation, RELVERIFY ),
    GA_PACK_WORDBIT( GA_FOLLOWMOUSE, Activation, FOLLOWMOUSE ),
    GA_PACK_WORDBIT( GA_RIGHTBORDER, Activation, RIGHTBORDER ),
    GA_PACK_WORDBIT( GA_LEFTBORDER, Activation, LEFTBORDER ),
    GA_PACK_WORDBIT( GA_TOPBORDER, Activation, TOPBORDER ),
    GA_PACK_WORDBIT( GA_BOTTOMBORDER, Activation, BOTTOMBORDER ),
    GA_PACK_WORDBIT( GA_TOGGLESELECT, Activation, TOGGLESELECT ),

    /* Gadget Flags: */
    GA_PACK_WORDBIT( GA_DISABLED, Flags, GADGDISABLED ),
    GA_PACK_WORDBIT( GA_SELECTED, Flags, SELECTED ),
    GA_PACK_WORDBIT( GA_TabCycle, Flags, GFLG_TABCYCLE ),
    GA_PACK_WORDBIT( GA_RelSpecial, Flags, GFLG_RELSPECIAL ),

    /* Gadget GadgetType flags: */
    GA_PACK_WORDBIT( GA_GZZGADGET, GadgetType, GZZGADGET ),
    GA_PACK_WORDBIT( GA_SYSGADGET, GadgetType, SYSGADGET ),

    /* ExtGadget MoreFlags: */
    GA_PACK_LONGBIT( GA_GadgetHelp, MoreFlags, GMORE_GADGETHELP ),
    GA_PACK_LONGBIT( GA_Bounds, MoreFlags, GMORE_BOUNDS ),

    GA_PACK_ENTRY( GA_ID, GadgetID, PKCTRL_UWORD ),
    GA_PACK_ENTRY( GA_USERDATA, UserData, PKCTRL_ULONG ),
    GA_PACK_ENTRY( GA_SPECIALINFO, SpecialInfo, PKCTRL_ULONG ),
    GA_PACK_ENTRY( GA_NEXT, NextGadget, PKCTRL_ULONG ),
    GA_PACK_ENTRY( GA_IMAGE, GadgetRender, PKCTRL_ULONG ),
    GA_PACK_EXWORDBIT( GA_IMAGE, Flags, GADGIMAGE ),
    GA_PACK_ENTRY( GA_BORDER, GadgetRender, PKCTRL_ULONG ),
    GA_PACK_EXNEGWORDBIT( GA_BORDER, Flags, GADGIMAGE ),

    GA_PACK_ENTRY( GA_INTUITEXT, GadgetText, PKCTRL_ULONG ),
    GA_PACK_EXNEGWORDBIT( GA_INTUITEXT, Flags, GFLG_LABELSTRING ),
    GA_PACK_EXNEGWORDBIT( GA_INTUITEXT, Flags, GFLG_LABELIMAGE ),

    GA_PACK_ENTRY( GA_TEXT, GadgetText, PKCTRL_ULONG ),
    GA_PACK_EXWORDBIT( GA_TEXT, Flags, GFLG_LABELSTRING ),
    GA_PACK_EXNEGWORDBIT( GA_TEXT, Flags, GFLG_LABELIMAGE ),

    GA_PACK_ENTRY( GA_LABELIMAGE, GadgetText, PKCTRL_ULONG ),
    GA_PACK_EXNEGWORDBIT( GA_LABELIMAGE, Flags, GFLG_LABELSTRING ),
    GA_PACK_EXWORDBIT( GA_LABELIMAGE, Flags, GFLG_LABELIMAGE ),

    GA_PACK_ENTRY( GA_LEFT, LeftEdge, PKCTRL_WORD ),
    GA_PACK_EXNEGWORDBIT( GA_LEFT, Flags, GFLG_RELRIGHT ),
    GA_PACK_ENTRY( GA_RELRIGHT, LeftEdge, PKCTRL_WORD ),
    GA_PACK_EXWORDBIT( GA_RELRIGHT, Flags, GFLG_RELRIGHT ),

    GA_PACK_ENTRY( GA_TOP, TopEdge, PKCTRL_WORD ),
    GA_PACK_EXNEGWORDBIT( GA_TOP, Flags, GFLG_RELBOTTOM ),
    GA_PACK_ENTRY( GA_RELBOTTOM, TopEdge, PKCTRL_WORD ),
    GA_PACK_EXWORDBIT( GA_RELBOTTOM, Flags, GFLG_RELBOTTOM ),

    GA_PACK_ENTRY( GA_WIDTH, Width, PKCTRL_WORD ),
    GA_PACK_EXNEGWORDBIT( GA_WIDTH, Flags, GFLG_RELWIDTH ),
    GA_PACK_ENTRY( GA_RELWIDTH, Width, PKCTRL_WORD ),
    GA_PACK_EXWORDBIT( GA_RELWIDTH, Flags, GFLG_RELWIDTH ),

    GA_PACK_ENTRY( GA_HEIGHT, Height, PKCTRL_WORD ),
    GA_PACK_EXNEGWORDBIT( GA_HEIGHT, Flags, GFLG_RELHEIGHT ),
    GA_PACK_ENTRY( GA_RELHEIGHT, Height, PKCTRL_WORD ),
    GA_PACK_EXWORDBIT( GA_RELHEIGHT, Flags, GFLG_RELHEIGHT ),

    PACK_ENDTABLE,
};

#if 0
/* ZZZ: not implemented
 * For now, use transparent baseclass to get these.
 * I really should code this up, though, so people
 * can make legitimate inquiries which the descendants
 * can override.
 */
getGadgetAttr( cl, o, msg )
{
    return ( 0 );
}
#endif

static
setGadgetAttrs( cl, o, msg )
Class		*cl;
Object		*o;
struct opSet	*msg;
{
    struct GIData 	*gid;
    struct ExtGadget	*g;
    struct TagItem	*NextTagItem();

    struct TagItem	*tags = msg->ops_AttrList;
    struct TagItem	*tstate = tags;
    struct TagItem	*tag;
    ULONG		tidata;

    BOOL		refresh = 0; /* I don't really need it */

    D( printf("setGadgetAttrs, o %lx, msg %lx, tags %lx g %lx\n",
			    o, msg, tags, g ) );

    gid = (struct GIData *) ((void *) (((ULONG)(o)) + (cl)->cl_InstOffset));
    g = &gid->gid_Gadget;

    /* process tags */
    PackStructureTags( g, gadgetPackTable, tags );
    /* All gadgetclass objects are extended gadgets, always: */
    g->Flags |= GFLG_EXTENDED;

    /* process rest */
    while ( tag = NextTagItem( &tstate ) )
    {
	tidata = tag->ti_Data;
	D( printf("set tag %lx\n", tag->ti_Tag ) );
	switch ( tag->ti_Tag )
	{
	case ICA_TARGET:
	    D2( printf("setting gad ICA target: %lx\n", tidata ) );
	    gid->gid_Target = (Object *) tidata;
	    break;

	case ICA_MAP:
	    /* don't clone, just keep pointer	*/
	    gid->gid_Map = (struct TagItem *) tidata;
	    break;

	case GA_SELECTRENDER:
	    g->SelectRender = (APTR) tidata;
	    if ( tidata ) setGHigh( g, GADGHIMAGE );
	    else setGHigh( g, GADGHNONE );
	    break;

	case GA_HIGHLIGHT:
	    setGHigh( g, tidata );
	    break;

	/* set the sysgadget type bits to tidata */
	case GA_SYSGTYPE:
	    g->GadgetType = (g->GadgetType & ~GTYP_SYSTYPEMASK) |
		(tidata & GTYP_SYSTYPEMASK);
	    break;

	case GA_PREVIOUS:
	    ((struct Gadget *)tidata)->NextGadget = g;
	    break;

	case GA_Bounds:
	    *((struct IBox *)&g->BoundsLeftEdge) = *((struct IBox *)tidata);
	    break;
	}
    }
    return ( refresh );	/* right now, I don't detect refresh cases */
}

/*
 * set up the proper highlight bits
 */
static
setGHigh( g, highlight )
struct Gadget	*g;
{
    g->Flags = ((g->Flags) & ~GADGHIGHBITS) | highlight;
}

