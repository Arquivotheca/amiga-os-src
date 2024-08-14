
/*** pubclasses.c ************************************************************
 *
 * pubclasses.c -- Intution public class create/manage
 *
 *  $Id: pubclasses.c,v 38.3 92/08/03 15:55:44 peter Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (C) 1989, 1990 Commodore-Amiga, Inc.
 *  All Rights Reserved
 *
 ****************************************************************************/

#if 0
TO DO:
Xadd class list semaphore to IBase
Xinit class list semaphore 
Xadd objectcount, subclasscount to Class
X	bump in rootclass/OM_NEW
X	dec in rootclass/OM_DISPOSE
Xchange MakeClass [tell davidj]
X    change .sfd
X    change intuitionface.asm
Xwrite my own makePublicClass using new MakeClass
Xcreate INLIST Class flag 
add entry points:
X 	RemoveClass[cl]
X 	FreeClass[cl]
X 	lockClassList[]
X 	unlockClassList[]
!!!!!!!! autodocs
Xis FindClass useless without LockClassList/UnlockClassList?
	yep
XNewObject needs to hang onto classlist lock!
Xremove FindClass from sendCompatibleImageMessage [cache imageclass in IBase-]
Xinstall this file.
#endif


#include "intuall.h"
#include <exec/nodes.h>
#include <exec/memory.h>
#include "classusr.h"
#include "classes.h"

#define D(x)	;
#define DMC(x)	;

InitClassList()
{
    NewList( &fetchIBase()->IClassList );
}

struct List	*
lockClassList()
{
    ObtainSemaphore( &fetchIBase()->ISemaphore[ CLASSLOCK ] );
    return ( &fetchIBase()->IClassList );
}

unlockClassList()
{
    ReleaseSemaphore( &fetchIBase()->ISemaphore[ CLASSLOCK ] );
}

/* worthless without lockClassList	*/
Class *
FindClass( classid )
ClassID classid;
{
    Class	*cl, *succ;

    for ( cl = (struct IClass *) lockClassList()->lh_Head;
	succ = (struct IClass *) cl->cl_Dispatcher.h_MinNode.mln_Succ; 
	cl = succ )
    {
	if ( jstreq( cl->cl_ID, classid ) ) 
	{
	    D( printf("found class <%s>, result %lx\n", classid, cl ) ); 
	    unlockClassList();
	    return ( cl );
	}
    }
    D( printf("class <%s> no find\n", classid ));
    unlockClassList();
    return ( NULL );
}
/*** intuition.library/MakeClass ***/

/*
 * Create a class, which might be private (classid = NULL).
 * Pass either ID of public superclass or pointer to private
 * superclass gotten to by a previous call to MakeClass().
 * Caller is responsible for not passing NULL superclass pointer
 * except for my root class.
 * Class is returned "unattached", so you can set up your hook
 * before you take it public.
 *
 * failure conditions:
 *	public class not unique (already exists)
 *	public superclass not found
 *	no mem for class data structure
 */
Class   *
MakeClass( classid, superid, superclass, instsize, flags )
ClassID classid;
Class   *superclass;
UWORD   instsize;
/* flags are for future enhancement	*/
{
    Class   *cl;
    Class   *FindClass();
    ULONG    stubReturn();

    DMC( printf( "MakeClass: <%s> super <%x> instsize %ld \n",
    	classid, superclass, instsize ) );

    /* scream if a public class already exists	*/
    if ( classid && (cl = FindClass( classid )) ) return ( NULL );

    /* get handle on superclass	*/
    if ( superid )	/* public superclass	*/
    {
	lockClassList();
	if ( superclass = FindClass( superid ) )
	{
	    /* bump subclass count, to keep superclass around	*/
	    superclass->cl_SubclassCount++;
	    unlockClassList();
	}
	else	/* public class not found	*/
	{
	    unlockClassList();
	    return ( NULL );
	}
    }
    /* Peter 9-Oct-91: This was being done even for rootclass, which
     * has a superclass of NULL.  This bumped location $28!
     * (Fixed in SetPatch 2.05, and here for ROM 2.05)
     */
    else if (superclass) /* private superclass	*/
    {
	/* bump subclass count, to keep superclass around	*/
	superclass->cl_SubclassCount++;
    }

    /* if I've got a superclass pointer by now, it's safe; not
     * going to go away on me
     */

    if ( cl = (Class *) AllocMem( sizeof *cl, MEMF_PUBLIC | MEMF_CLEAR ) )
    {
	cl->cl_Super      = superclass;
	cl->cl_InstOffset = superclass? 
		superclass->cl_InstOffset + superclass->cl_InstSize:
		0;
	cl->cl_InstSize   = instsize;
	cl->cl_ID	  = classid;	/* ZZZ: copy this in? */

	/* count on 0 init values for:
	 * cl_SubclassCount, cl_ObjectCount, cl_Flags
	 */

	/* initialize Hook to no-op initial value */
	cl->cl_Dispatcher.h_Entry = stubReturn;
	cl->cl_Dispatcher.h_SubEntry = 0;
	cl->cl_Dispatcher.h_Data = (VOID *) 0xDEAD;	/* I don't use it */
    }

    return ( cl );	/* returns unattached class	*/
}


/*
 * Make shared, public, named subclass of public superclass.
 * Be sure class doesn't already exist,
 * also be sure that superclass *does* exist
 * 
 * This is what Intuition uses, and isn't too general.
 *
 * Makes a class for C dispatcher
 */
Class   *
makePublicClass( classid, superid, instsize, dispatch )
ClassID classid;
Class   *superid;
UWORD   instsize;
ULONG	(*dispatch)();
{
    Class	*cl;
    Class	*MakeClass();

    /* get the class created	*/
    if ( cl = MakeClass( classid, superid, NULL, instsize, 0 ) )
    {
	initHook( &cl->cl_Dispatcher, dispatch );
	AddClass( cl );
    }
    return ( cl );
}

#if 0	/* not used by Intuition	*/
/*
 * Make public or private subclass of either private or public class.
 * If myclassid is provided, my class is public.
 * If superid is NULL, then use superclass pointer (to private superclass).
 */
Class   *
makeMyClass( myclassid, superid, superclass, instsize, dispatch )
ClassID   *myclass;
ClassID   *superid;
Class	*superclass;
UWORD   instsize;
ULONG	(*dispatch)();
{
    Class	*cl;
    Class	*MakeClass();

    if ( cl = MakeClass( myclassid, superid, superclass, instsize ) )
    {
	initHook( &cl->cl_Dispatcher, dispatch );
	AddClass( cl );
    }
    return ( cl );
}
#endif

/*
 * Makes public class available for public consumption
 * won't add a class with no name
 */

/*** intuition.library/AddClass ***/

AddClass( cl )
Class	*cl;
{
    if ( cl && cl->cl_ID && ! (cl->cl_Flags & CLF_INLIST) )
    {
	AddHead( lockClassList(), (struct Node *) cl );
	cl->cl_Flags |= CLF_INLIST;
	unlockClassList();
    }
}
/*** intuition.library/RemoveClass ***/


/* 
 * makes a public class unavailble for public consumption.
 * OK to call for class not in list
 * Note that this won't call lockClassList if the class isn't
 * in the list.
 */
RemoveClass( cl )
Class	*cl;
{
    if ( cl && (cl->cl_Flags & CLF_INLIST) )
    {
	lockClassList();
	Remove( (struct Node *)cl );
	cl->cl_Flags &= ~CLF_INLIST;
	unlockClassList();
    }
}
/*** intuition.library/FreeClass ***/


/*
 * Frees a class if no objects or subclasses
 * removes if in class list, whether succeeds or not.
 * Returns non-zero (TRUE) if it succeeds.  Otherwise,
 * you can't unload the code which implements the class.
 * Returns TRUE if you pass it a NULL class pointer.
 * Note that this won't call lockClassList if the class isn't
 * in the list.
 *
 * Freeing some subclass of root must, if it's going to
 * fail, must provide for some way to retry, or add the
 * class to the list.
 */
FreeClass( cl )
Class	*cl;
{
    D( printf("FreeClass %lx\n", cl ));
    if ( ! cl ) return ( TRUE );

    RemoveClass( cl );

    D( printf("subclasses: %ld, objects: %ld\n", 
	cl->cl_SubclassCount, cl->cl_ObjectCount ) );

    if ( !( cl->cl_SubclassCount || cl->cl_ObjectCount ) )
    {
	/* one fewer subclasses for my superclass	*/
	if ( cl->cl_Super ) cl->cl_Super->cl_SubclassCount--;
	FreeMem( cl, sizeof *cl );	/*  bye-bye	*/
	return ( TRUE );
    }
    return ( FALSE );
}

