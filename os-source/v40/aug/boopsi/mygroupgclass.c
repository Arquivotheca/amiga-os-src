/* mygroupgclass.c -- :ts=8
 * Example of a "gadget group subclass"
 * which creates composite gadgets for demo 5.
 */

/*
Copyright (c) 1989, 1990 Commodore-Amiga, Inc.

Executables based on this information may be used in software
for Commodore Amiga computers. All other rights reserved.
This information is provided "as is"; no warranties are made.
All use is at your own risk, and no liability or responsibility
is assumed.
*/

#include "sysall.h"
#include <intuition/classes.h>
#include "mymodel.h"		/* attributes are defined there	*/

#define D(x)	;

#ifdef printf
#undef printf
#endif
#define printf kprintf

#define G(o) 	((struct Gadget *) (o))

/* private class	*/
#define MYCLASSID	(NULL)
extern struct Library	*IntuitionBase;
#define SUPERCLASSID	GROUPGCLASS

struct MyGroupData	{
    Object		*mgd_MyModel;
    struct Image	*mgd_UpImage; 
    struct Image	*mgd_DownImage;
};

Class	*
initMyGroupClass()
{
    ULONG __saveds	dispatchMyGroup();
    ULONG	hookEntry();
    Class	*cl;
    Class	*MakeClass();

    void	*modclass;
    void	*initMyModClass();

    D( printf("INITMYGROUPGCLASS\n"));

    if ( ! ( modclass = initMyModClass() ) )
    {
	D( printf("mgg: couldn't get modelclass\n" ));
	return ( NULL );
    }

    D( printf("initMC got modclass at %lx\n", modclass ) );

    if ( cl =  MakeClass( MYCLASSID, 
		SUPERCLASSID, NULL,		/* superclass is public      */
 		sizeof ( struct MyGroupData ),
		0 ))
    {
	/* initialize the cl_Dispatcher Hook	*/
	cl->cl_Dispatcher.h_Entry = hookEntry;
	cl->cl_Dispatcher.h_SubEntry = dispatchMyGroup;
	cl->cl_Dispatcher.h_Data = (VOID *) 0xFACE;	/* unused */

	/* keep track of this class handle for use in OM_NEW
	 * and freeMyGroupClass()
	 */
	cl->cl_UserData = (ULONG) modclass;
    }
    return ( cl );
}

freeMyGroupClass( cl )
Class	*cl;
{
    int success = TRUE;

    if ( cl )
    {
	success = freeMyModClass( cl->cl_UserData );
    }
    /* don't try to free if the model class didn't free	*/
    return ( success? FreeClass( cl ): FALSE  );
}

ULONG __saveds
dispatchMyGroup( cl, o, msg )
Class   *cl;
Object  *o;
Msg     msg;
{
    Object  		*newobj;
    struct MyGroupData	*mgd;

    mgd = INST_DATA( cl, o );

    switch ( msg->MethodID )
    {
    case OM_NEW:
	D( printf("myg: om_new\n"));

	if ( newobj = (Object *) DSM( cl, o, msg ) )
	{
	    if ( ! initMyGroupObject( cl, newobj, msg ) )
	    {
		D( printf("om_new: iMGO failed\n") );
		/* free what's there if failure	*/
	    	CoerceMethod( cl, newobj, OM_DISPOSE );
		newobj = NULL;
	    }
	    else
	    {
		D( printf("om_new: iMGO succeeded, set atts\n"));
		/* superclass already initialized, don't
		 * propagate attributes up to group gadget
		 * again now.
		 */
		setMyGroupAttrs( cl, newobj, msg );
	    }
	}
	D( printf("mygroup: OM_NEW returning %lx\n", newobj ) );
	return ( (ULONG) newobj );

    case OM_GET:	
	return ( (ULONG) getMyGroupAttrs( cl, o, msg ) );

    case OM_UPDATE:	
	/* delegated to the model and superclass within in setMyGroupAttrs() */
    case OM_SET:	

	/* this time, we want to let our superclass know about
	 * certain things.
	 */
	return ( (ULONG) setMyGroupAttrs( cl, o, msg ) );

    case OM_DISPOSE:
	/* dispose of components	*/
	D( printf("mygroup: dispose model\n"));
    	DM( mgd->mgd_MyModel, msg );	/* dispose model (and IC's)	*/
	D( printf("mygroup: dispose self and components\n"));
	DSM( cl, o, msg );		/* dispose self and component gadgets */
	D( printf("mygroup: dispose upimage\n"));
	DM( mgd->mgd_UpImage, msg );
	D( printf("mygroup: dispose downimage\n"));
	DM( mgd->mgd_DownImage, msg );
	break;

    default:	/* let the superclass handle it */
	return ( (ULONG) DSM( cl, o, msg ) );
    }
    return ( 1 );
}

#define PWIDTH		(120)	/* width of horizontal propslider	*/
#define SWIDTH		(50)	/* width of string gadget		*/

enum gadgetids {
    gUp = 1,
    gDown,
    gSlider,
    gString,
    gGroup,
};

/* a macro for the "adjacent" position to the right of a gadget,
 * but safe, if the reference gadget is NULL
 */
#define RIGHTBY( g )	( ((g)==NULL)? 20: ((g)->LeftEdge + (g)->Width ) )

/****************************************************************/
/*  mapping tag lists						*/
/****************************************************************/

struct TagItem	slider2model[] = {
    {PGA_TOP, MYMODA_CURRVAL},
    {TAG_END, }
};

struct TagItem	model2slider[] = {
    {MYMODA_CURRVAL, PGA_TOP},
    {MYMODA_RANGE, PGA_TOTAL },
    {TAG_END, }
};

struct TagItem	string2model[] = {
    {STRINGA_LongVal, MYMODA_CURRVAL},
    {TAG_END, }
};

struct TagItem	model2string[] = {
    {MYMODA_CURRVAL, STRINGA_LongVal},
    {TAG_END, }
};

struct TagItem	uparrow2model[] = {
    {GA_ID, MYMODA_INCRSTROBE},
    {TAG_END, }
};

struct TagItem	downarrow2model[] = {
    {GA_ID, MYMODA_DECRSTROBE},
    {TAG_END, }
};

/****************************************************************/
/* tag lists for creating objects				*/
/****************************************************************/


struct TagItem	proptags[] = {
    {GA_WIDTH,		PWIDTH},	/* height to be specified	*/
    /* {ICA_TARGET,	XXX }, * will be model object	*/
    {ICA_MAP,		(ULONG) &slider2model[0]},
    {PGA_FREEDOM,	FREEHORIZ},
    {PGA_VISIBLE,	1},		/* want an integer value slider	*/
    {TAG_END ,}
};

struct TagItem	stringtags[] = {
    {GA_WIDTH,		SWIDTH},
    {GA_HEIGHT,		12},		/* fix this	*/
    /* {ICA_TARGET,	XXXX },		will be model object	*/
    {ICA_MAP,		(ULONG) &string2model[0]},
    {STRINGA_MaxChars,	10},
    {STRINGA_LongVal,	0},		/* make it an integer gadget */
    {STRINGA_Justification,
    			STRINGRIGHT},
    {TAG_END, }
};


/*
 * Return TRUE if I could do everything needed
 * to initialize one of my objects.
 */
initMyGroupObject( cl, o, msg )
Class   *cl;
Object  *o;
struct opSet	*msg;
{
    struct DrawInfo	*drinfo;
    struct Gadget	*g;
    Object		*ic;
    WORD		nextleft;

    struct MyGroupData	*mgd = INST_DATA( cl, o );
    struct TagItem	*tags = msg->ops_AttrList;

    D( printf("iMGO in\n"));

    /* DrawInfo is required	*/
    if ( ! (drinfo = (struct DrawInfo *) GetTagData( GA_DRAWINFO, 0,  tags ) ) )
    {
	D( printf("iMGO didn't get the needed drawinfo\n"));
	return ( FALSE );
    }

    D( printf("iMGO got drawinfo at %lx\n", drinfo));

    /* get images for the up and down arrows, sensitive
     * to depth and pen specs for current screen (we'll be
     * adding resolution/size selection later).
     */
    mgd->mgd_UpImage = (struct Image *) NewObject( NULL, "sysiclass",
	SYSIA_Size,	0,		/* normal "medium-res" for now */
	SYSIA_DrawInfo, drinfo,
	SYSIA_Which,	UPIMAGE,
    	TAG_END );

    D( printf("iMGO 1\n"));

    mgd->mgd_DownImage = (struct Image *) NewObject( NULL, "sysiclass",
	SYSIA_Size,	0,		/* normal "medium-res" for now */
	SYSIA_Which,	DOWNIMAGE,
	SYSIA_DrawInfo, drinfo,
    	TAG_END );

    D( printf("iMGO 2\n"));

    /* Allocate model: we have a pointer to the model class
     * stashed in cl_UserData for just this occasion.
     *
     * Target of model's UPDATE notification is set when
     * user sets the group gadget attribute and map.
     */
    D( printf("get model object, class at %lx\n", cl->cl_UserData ) );
    mgd->mgd_MyModel =  (Object *) NewObject( cl->cl_UserData, NULL, 
			TAG_END );
    D( printf("model at %lx\n", mgd->mgd_MyModel ) );

    if ( ! mgd->mgd_MyModel ) return ( FALSE );	/* I really need this guy */

    /* get the string gadget	*/
    g  = (struct Gadget *) NewObject( NULL, "strgclass",
    	GA_LEFT,	0,
	ICA_TARGET,	mgd->mgd_MyModel,
	TAG_MORE,	stringtags,
	TAG_END );
    D( printf( "string gadget at %lx\n", g ) );
    nextleft = RIGHTBY( g );

    /* add it to the group ... which is myself	*/
    DoMethod( G(o), OM_ADDMEMBER, g );

    /* The string gadget now talks to the model.
     * Add an interconnection from the model to
     * the string gadget.
     */
    ic = (Object *) NewObject( NULL, ICCLASS,
    		ICA_TARGET,	g,
		ICA_MAP,	model2string,
		TAG_END );
    DoMethod( mgd->mgd_MyModel, OM_ADDMEMBER, ic );

    D( printf("iMGO 1\n"));

    /* get the prop gadget, immediately to the right of the string */
    g = (struct Gadget *) NewObject( NULL, "propgclass",
	GA_LEFT,	nextleft,
	GA_HEIGHT,	mgd->mgd_DownImage? mgd->mgd_DownImage->Height: 20,
	ICA_TARGET,	mgd->mgd_MyModel,
	TAG_MORE,	proptags,
	TAG_END );
    D( printf( "prop gadget returned: %lx\n", g ) );
    nextleft = RIGHTBY( g );

    /* add it to the group ...	*/
    DoMethod( G(o), OM_ADDMEMBER, g );

    /* ... and get an ic for the prop gadget	*/
    ic = (Object *) NewObject( NULL, ICCLASS,
    		ICA_TARGET,	g,
		ICA_MAP,	model2slider,
		TAG_END );
    DoMethod( mgd->mgd_MyModel, OM_ADDMEMBER, ic );

    /* down button is immediately to the right ...	*/
    g = (struct Gadget *) NewObject( NULL, "buttongclass",
	GA_IMAGE, 	mgd->mgd_DownImage,
	GA_LEFT,	nextleft,
	GA_ID,		gDown,
	/* interconnections ...	*/
	ICA_TARGET,	mgd->mgd_MyModel,
	ICA_MAP,	&downarrow2model[0],
	TAG_END );
    D( printf("downgadget at %lx\n", g ));
    nextleft = RIGHTBY( g );

    /* ... and add it to the group gadget	*/
    DoMethod( G(o), OM_ADDMEMBER, g );

    /* the buttons don't need to hear from the model, 
     * hence, no ic's for them.
     */

    /* up arrow button	*/
    g = (struct Gadget *) NewObject( NULL, "buttongclass",
	GA_IMAGE, 	mgd->mgd_UpImage,
	GA_LEFT,	nextleft,
	GA_ID,		gUp,
	/* interconnections ...	*/
	ICA_MAP,	&uparrow2model[0],
	ICA_TARGET,	mgd->mgd_MyModel,
	TAG_END );
    D( printf("upgadget at %lx\n", g ));

    /* ... and add it to the group gadget	*/
    DoMethod( G(o), OM_ADDMEMBER, g );

    D(printf("all components initialized\n"));

    return ( TRUE );
}










/** Filter and Delegate Slept Here **/

/* I'll pass along these to the model, and
 * withhold them from my ancestors
 */
Tag	model_attrs[] = {
    MYMODA_CURRVAL,
    MYMODA_RANGE,
    ICA_TARGET,
    ICA_MAP,
    TAG_END,
};


getMyGroupAttrs( cl, o, msg )
Class		*cl;
Object		*o;
struct opGet	*msg;
{
    struct MyGroupData	*mgd = INST_DATA( cl, o );

    /* delegate attribute responsibility to model,
     * for attributes it understands.  Others
     * are handled by group gadget (superclass)
     */

    if ( TagInArray( msg->opg_AttrID, model_attrs ) )
    {
	return ( DM( mgd->mgd_MyModel, msg ) );
    }
    /* else */
	return ( DSM( cl, o, msg ) );
}

/*
 * This function is called for OM_NEW, OM_SET, and OM_UPDATE.
 * What we do is delegate the processing of all the attributes
 * that our model will understand to it, including the
 * target/map.  These last means that the model will be
 * directly connected to the outside world instead of
 * directly our gadget itself.
 *
 * In the case of OM_NEW, I've already created the model with
 * no attributes passed, now I have to convert the OM_NEW
 * message into an OM_SET message so we don't go create
 * another model object.
 *
 * Also, since the superclass has already seen the OM_NEW
 * message and attributes, we don't pass it along again here,
 * in that case.
 */
setMyGroupAttrs( cl, o, msg )
Class		*cl;
Object		*o;
struct opSet	*msg;
{
    /* we're going to duplicate our tagitems and
     * filter the cloned list.
     */
    struct TagItem	*origtags;
    struct TagItem	*CloneTagItems();
    ULONG		FilterTagItems();
    void		RefreshTagItemClones();

    int			retval = 0;
    struct MyGroupData	*mgd;

    mgd = INST_DATA( cl, o );

    /* going to delegate different pieces of the list
     * to different objects, so I need to substitute
     * a clone for the list passed to me.
     */
    origtags = msg->ops_AttrList;
    msg->ops_AttrList = CloneTagItems( origtags );

    /* set attributes for model	*/
    D( dumpTagList( "debug 1", msg->ops_AttrList ) );
    if ( FilterTagItems( msg->ops_AttrList, model_attrs, TAGFILTER_AND ) )
    {
	D( dumpTagList( "after filtering", msg->ops_AttrList ) );

	/* Pass along same message, with filtered attrlist
	 * This message will be mapped and propagated to all
	 * component gadgets, who will refresh themselves
	 * if needed.  There's no additional need to refresh
	 * the group gadget.
	 *
	 * For the case of OM_NEW, we have to convert to OM_SET,
	 * since we've previously created the thing.
	 */
	D( printf("send filtered tags to my model (at %lx)\n", mgd->mgd_MyModel));

	if ( msg->MethodID == OM_NEW )
	{
	    DoMethod(mgd->mgd_MyModel, OM_SET, msg->ops_AttrList, msg->ops_GInfo);
	}
	else
	{
	    /* pass along OM_SET or OM_UPDATE message as is (with mapped tags). */
	    DM( mgd->mgd_MyModel, msg );
	}
    }

    if ( msg->MethodID != OM_NEW )
    {
	/* re-clone, without re-allocating	*/
	RefreshTagItemClones( msg->ops_AttrList, origtags );

	if ( FilterTagItems( msg->ops_AttrList, model_attrs, TAGFILTER_NOT ) )
	{
	    /* Perhaps there is a need to refresh the group gadget now. */
	    D( printf("send to superclass\n"));
	    retval = DSM( cl, o, msg );
	}
	D( else printf("filter failed 2\n"));
    }

    /* free clone and restore original */
    FreeTagItems( msg->ops_AttrList );
    msg->ops_AttrList = origtags ;
    return ( retval );
}
