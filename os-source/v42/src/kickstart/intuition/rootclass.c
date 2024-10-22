/*** object.c *****************************************************************
 *
 *  objects.c -- root class definition
 *
 *  $Id: rootclass.c,v 40.0 94/02/15 17:29:42 davidj Exp $
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

/*****************************************************************************/

#if 0
/* I want to change this! */
struct _Object
{
    struct MinNode	o_Node;
    struct IClass	*o_Class;
};
#endif

/*****************************************************************************/

static ULONG rootDispatch (Class *cl, Object *o, Msg msg)
{
    Class *trueclass;

    switch (msg->MethodID)
    {
	/* Allocate the object */
	case OM_NEW:
	    trueclass = (struct IClass *) o;

	    /* allocate instance data appropriate for the true class */
	    o = (Object *) AllocMem (SIZEOF_INSTANCE (trueclass), MEMF_PUBLIC | MEMF_CLEAR);

	    /* install true class pointer in the object header */
	    if (o)
	    {
		/** My little secret: _Object header is BEFORE object handle **/
		o = BASEOBJECT (o);	/* advance pointer to base object */
		OCLASS (o) = trueclass;
		trueclass->cl_ObjectCount++;
	    }
	    return ((ULONG) o);

	/* Deallocate the object */
	case OM_DISPOSE:
	    OCLASS (o)->cl_ObjectCount--;

	    /** My little secret: Object header is BEFORE object pointer **/
	    FreeMem (_OBJECT (o), SIZEOF_INSTANCE (OCLASS (o)));
	    return (NULL);

	/* Add the object to a list */
	case OM_ADDTAIL:
	    AddTail (((struct opAddTail *) msg)->opat_List, (struct Node *) &_OBJECT (o)->o_Node);
	    break;

	/* Remove the object from a list */
	case OM_REMOVE:
	    Remove ((struct Node *) &_OBJECT (o)->o_Node);
	    break;

	/* fall through */
	default:
	    return (0);
    }
    return (1);
}

/*****************************************************************************/

Class *initRootClass (void)
{
    extern UBYTE *RootClassName;

    /* we are putting the root Object instance data BEFORE the object handle we hand back. */
    return (makePublicClass (RootClassName, NULL, 0, rootDispatch));
}
