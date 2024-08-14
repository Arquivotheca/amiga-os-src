/* mymodelclass.c -- :ts=8
 * Example of a simple subclass of "modelclass".
 * It maintains an integer "current value", which
 * it keeps between 0 and some specified maximum.
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

#ifdef printf
#undef printf
#endif

#define printf kprintf


#define D(x)	;

/* private class	*/
#define MYCLASSID	(NULL)
extern struct Library	*IntuitionBase;
#define SUPERCLASSID	MODELCLASS

struct MyModData	{
    ULONG	mmd_CurrentValue;
    ULONG	mmd_Range;		/* current value <= range-1	*/
};

Class	*
initMyModClass()
{
    ULONG __saveds	dispatchMyMod();
    ULONG	hookEntry();
    Class	*cl;
    Class	*MakeClass();

    if ( cl =  MakeClass( MYCLASSID, 
		SUPERCLASSID, NULL,		/* superclass is public      */
 		sizeof ( struct MyModData ),
		0 ))
    {
	/* initialize the cl_Dispatcher Hook	*/
	cl->cl_Dispatcher.h_Entry = hookEntry;
	cl->cl_Dispatcher.h_SubEntry = dispatchMyMod;
	cl->cl_Dispatcher.h_Data = (VOID *) 0xFACE;	/* unused */
    }
    return ( cl );
}

freeMyModClass( cl )
Class	*cl;
{
    return ( FreeClass( cl )  );
}

ULONG __saveds 
dispatchMyMod( cl, o, msg )
Class   *cl;
Object  *o;
Msg     msg;
{
    Object	*newobj;
    ULONG	oldval;
    ULONG	oldrng;
    ULONG	notify_msg_flags = 0;
    ULONG	interim_flag;

    struct MyModData	*mmd;

    D( printf("mymodel dispatcher, method ID %lx\n", msg->MethodID ) );

    mmd = INST_DATA( cl, o );

    switch ( msg->MethodID )
    {
    /* use superclass defaults for everything else */
    case OM_NEW:
	D( printf("mymodel: OM_NEW\n") );
	if( newobj = (Object *) DSM( cl, o, msg ) )
	{
	    D( printf("new model object at %lx\n", newobj ) );
	    /* initialize instance data (they start life as 0)	*/
	    setMyModAttrs( cl, newobj, msg );
	}
	return ( (ULONG) newobj );

    case OM_GET:
	return ( (ULONG) getMyModAttr( cl, o, msg ) );

    case OM_SET:
    case OM_UPDATE:
	D( printf("mymod update\n" ) );
	if ( ! DoSuperMethod( cl, o, ICM_CHECKLOOP ) ) 
	{
	    /* let the superclass see whatever it wants from OM_SET,
	     * such as ICA_TARGET, ICA_MAP, and so on.  For OM_NOTIFY,
	     * however, we control all traffic and issue notification
	     * specifically for the attributes we're interested in.
	     */
	    if ( msg->MethodID == OM_SET )
	    {
		D( printf("mymod update is actually OM_SET\n"));
		DSM( cl, o, msg );
	    }
	    else
	    {
		/* these flags aren't present in the message of OM_SET	*/
		notify_msg_flags =  ((struct opUpdate *)msg)->opu_Flags;
	    }

	    /*
	     * I'll be wanting to know this is an "interim" message
	     * or a final report (which I always want to send, even
	     * if the value of mmd_CurrentValue hasn't changed).
	     */
	    interim_flag =  notify_msg_flags & OPUF_INTERIM;

	    /* Now set possibly new value of mmd_CurrentVal, and
	     * maybe a range change.
	     * Only send a notification message along for values of
	     * interest that have changed.
	     */

	    /* save 'em	*/
	    oldval = mmd->mmd_CurrentValue;
	    oldrng = mmd->mmd_Range;

	    /* change 'em, only if changed (or if
	     * a "non-interim" message.
	     */
	    if ( setMyModAttrs( cl, o, msg ) || ! interim_flag )
	    {
	    	Tag	rangetag;
	    	Tag	currvaltag;

/* if condition is false, replace tag with TAG_IGNORE	*/
#define XTAG( expr, tagid ) ((expr)? (tagid): TAG_IGNORE)

		rangetag = (oldrng!=mmd->mmd_Range)? MYMODA_RANGE: TAG_IGNORE;
		if ( ! interim_flag || (oldval != mmd->mmd_CurrentValue) )
		{
		    currvaltag = MYMODA_CURRVAL;
		}
		else
		{
		    currvaltag = TAG_IGNORE;
		}


		D( printf("mymod: sending notification\n") );

		/* Pass along GInfo, if any, so gadgets can redraw
		 * themselves.  Pass along opu_Flags, so that the
		 * application will know the difference between
		 * and interim message and a final message
		 */
		notifyAttrChanges( o, ((struct opSet *)msg)->ops_GInfo,
			interim_flag,
			rangetag,	mmd->mmd_Range,
			currvaltag,	mmd->mmd_CurrentValue,
			TAG_END );
	    }
	    D( else printf("setMyModAttrs returnes 'nochange'\n"));
	}
	D( else printf("Loop Check violation!\n"));
	break;

    case OM_NOTIFY:
    	D( printf("mymod: forwarding OM_NOTIFY to superclass modelclass\n"));
    case OM_DISPOSE:
    default:
	D( printf("let superclass handle it\n"));
	return ( (ULONG) DSM( cl, o, msg ) );
    }
    return ( 1 );
}

setMyModAttrs( cl, o, msg )
Class		*cl;
Object		*o;
struct opSet	*msg;
{
    struct TagItem	*tags = msg->ops_AttrList;
    struct MyModData	*mmd;
    int			changes = FALSE;
    LONG		newval;
    ULONG		newrng;

    mmd = INST_DATA( cl, o );

    D( printf("setMyModAttrs, object %lx\n", o ) );
    D( dumpTagList( "mymod attr tags", tags ));

    newrng =  GetTagData( MYMODA_RANGE, mmd->mmd_Range, tags );

    if ( mmd->mmd_Range != newrng )
    {
	D( printf( "mymod: range has changed value to %ld\n",
		mmd->mmd_Range ));
	mmd->mmd_Range =  newrng;
	changes = TRUE;
    }

    /* validity check	*/
    if ( mmd->mmd_Range == 0 )
    {
	mmd->mmd_Range = 1;
	changes = TRUE;
    }

    D( printf("range is %ld\n", mmd->mmd_Range ) );

    /* start with original value	*/
    newval =  mmd->mmd_CurrentValue;

    D( printf("original currval %ld\n", mmd->mmd_CurrentValue ) );

    /* increment/decrement in response to strobes	*/
    if ( GetTagData( MYMODA_INCRSTROBE, 0, tags ) > 0 )
    {
	newval++;
	D( printf("strobe increment newval to %ld\n", newval ) );
    }
    if ( GetTagData( MYMODA_DECRSTROBE, FALSE, tags ) > 0 )
    {
	if ( newval > 0 ) newval--;
	D( printf("strobe decrement newval to %ld\n", newval ) );
    }

    /* look at "absolute" setting last	*/
    newval = GetTagData( MYMODA_CURRVAL, newval, tags );

    D( printf("unconstrained newval %ld\n", newval ) );

    /* limit mmd_CurrentValue to mmd_Range-1	*/
    if ( newval < 0 ) newval = 0;
    if ( newval > (mmd->mmd_Range-1) ) newval = mmd->mmd_Range - 1;

    D( printf( "final: new current value %ld\n", newval ));

    if ( mmd->mmd_CurrentValue != newval )
    {
    	mmd->mmd_CurrentValue = newval;
	D( printf( "mymod: value has changed to %ld\n",
		mmd->mmd_CurrentValue ));
	changes = TRUE;
    }

    return ( changes );
}

getMyModAttr( cl, o, msg )
Class		*cl;
Object		*o;
struct opGet	*msg;
{
    struct MyModData	*mmd;

    mmd = INST_DATA( cl, o );

    switch ( msg->opg_AttrID )
    {
    case MYMODA_CURRVAL:
	*msg->opg_Storage = mmd->mmd_CurrentValue;
	break;
    case MYMODA_RANGE:
	*msg->opg_Storage = mmd->mmd_Range;
	break;
    default:
	/* I don't recognize this one, let the superclass try	*/
	return ( DSM( cl, o, msg ) );
    }
    return ( TRUE );
}

/*
 * a convenient way to construct and send an OM_NOTIFY message
 */
notifyAttrChanges( o, ginfo, flags, tag1 )
Object	*o;
void	*ginfo;
ULONG	flags;
ULONG	tag1;
{
    return ( DoMethod( o, OM_NOTIFY, &tag1, ginfo, flags ) );
}
