/*** object.c *****************************************************************
 *
 *  objects.c -- root class definition
 *
 *  $Id: rootclass.c,v 38.0 91/06/12 14:29:50 peter Exp $
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

#include "rootclass_protos.h"

/*---------------------------------------------------------------------------*/

static int rootDispatch(Class * cl,
                        Object * o,
                        Msg msg);

/*---------------------------------------------------------------------------*/

#define D(x)	;
#define D2(x)	;
#define D3(x)	;

/** --------------  Base Class ---------------------- **/

Class	*
initRootClass()
{
    int	rootDispatch();
    Class	*makePublicClass();
    extern UBYTE	*RootClassName;

    /*
     * we are putting the root Object instance data BEFORE the object
     * handle we hand back.
     */

    D( printf("initRootClass\n"));
    return ( makePublicClass( RootClassName, NULL, 0, rootDispatch ) );
}

static rootDispatch( cl, o, msg )
Class   *cl;
Object  *o;
Msg     msg;
{
    Class   *trueclass;

    D( printf("rootDispatch, class %lx, object %lx, msg %lx (id: %lx)\n", 
	cl,o,msg, msg->MethodID ));

    switch ( msg->MethodID  )
    {

    case OM_NEW:
	trueclass = (struct IClass *) o;

	D( printf("\t\troot OM_NEW: object size (dec) %ld\n",
		SIZEOF_INSTANCE(trueclass) ) );

	/* allocate instance data appropriate for the true class */
	o = (Object *) AllocMem( SIZEOF_INSTANCE(trueclass),
			MEMF_PUBLIC|MEMF_CLEAR );

	/* install true class pointer in the object header */
	if ( o ) 
	{
	    /** My little secret: _Object header is BEFORE object handle **/
	    o = BASEOBJECT( o );    /* advance pointer to base object */
	    OCLASS( o ) = trueclass;
	    trueclass->cl_ObjectCount++;
	}
	return ( (ULONG) o );

    case OM_DISPOSE:

	D2( printf("\t\trootD: OM_DISPOSE %lx. object size (dec) %ld\n",
		o, SIZEOF_INSTANCE( OCLASS( o ) ) ) );

	OCLASS(o)->cl_ObjectCount--;

	/** My little secret: Object header is BEFORE object pointer **/
	FreeMem( _OBJECT( o ),  SIZEOF_INSTANCE( OCLASS( o ) ) );
	return ( NULL );

    case OM_ADDTAIL:
	D3( printf("ROOT handling OM_ADDTAIL, list %lx, node: %lx\n",
		((struct opAddTail *)msg)->opat_List, &_OBJECT(o)->o_Node));

	AddTail( ((struct opAddTail *)msg)->opat_List, (struct Node *)&_OBJECT(o)->o_Node );
	break;

    case OM_REMOVE:
	Remove( (struct Node *)&_OBJECT( o )->o_Node );
	break;
	
	/* fall through */
    default:
	return ( 0 );
    }
    return ( 1 );
}

