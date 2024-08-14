/* myclassinit.c -- :ts=8
 * Init, Open, Close, Expunge for myclass.library.
 * Note: although this seems to work well, this library
 * needs to be tested more before it is used as a
 * base for other libraries.
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
#include <exec/libraries.h>
#include <intuition/classes.h>
#include "myclassbase.h"

#ifdef printf
#undef printf
#endif
#define printf kprintf

#define D(x)	x

/* These globals are for library calls from the classes.
 * This library is not rommable as is.
 */
struct  IntuitionBase   *IntuitionBase = NULL;
struct  GfxBase         *GfxBase = NULL;
struct  Library         *UtilityBase = NULL;
struct  Library         *SysBase = NULL;

/* gets my public class	*/
struct IClass	*initEmbBClass();


/*
 * The basic protocol here is that if the class pointer
 * MyLibBase.mlb_MyClass is NULL, then this library
 * should be expungable as is.
 *
 * So, myOpen() must clean up if it can't get the library,
 * and myClose() must be completely expungable iff it frees the 
 * class.
 */

/*
 * This routine gets called after the library has been allocated.
 * The library pointer is in D0.  The segment list is in A0.
 * If it returns non-zero then the library will be linked into
 * the library list.
 */
struct Library * __saveds
__asm myLibInit(
	register __d0 struct MyLibBase *mlb,
	register __a0 void	*seglist,
	register __a6 void 	*sysbase )
{
	D( printf("mylibinit, mlb %lx, seglist %lx, sysbase %lx\n",
		mlb, seglist, sysbase ) );

	SysBase = sysbase;	/* set up as global for my class to use */

	mlb->mlb_SegList = seglist;

	return ( (struct Library *) mlb );
}


struct Library * __saveds
__asm myLibOpen( register __a6 struct MyLibBase *mlb )
{
   D( printf("openlib: mlb %lx\n", mlb ) );

   mlb->mlb_Flags &= ~LIBF_DELEXP;
   if ( ++mlb->mlb_Library.lib_OpenCnt == 1 )
   {
	D( printf("we are the first opener\n") );

	/* first open	*/
	if ( mlb->mlb_MyClass )
	{
	    D( printf("openlib: class is hanging around at %lx\n", 
	    	mlb->mlb_MyClass ) );

	    /* class still hanging around, needs only to be re-added */
	    /* Also, I've still got the libraries open */
	    AddClass( mlb->mlb_MyClass );
	}
	else
	{
	    if ( ! openAll( mlb ) ||
	    	(mlb->mlb_MyClass = initEmbBClass()) == NULL )
	    {
		/*** FAILURE TO OPEN ***/
		D( printf("failure to open/initialize\n") );
		closeAll();	/* make self expungeable	*/

		mlb->mlb_Library.lib_OpenCnt = 0;
		return ( NULL );	/* failed OpenLibrary() */
	    }

	    D(printf("successfully open, class at %lx\n", mlb->mlb_MyClass));
	}
   }

   return ( (struct Library *) mlb );
}

/*
 * Returns seglist if it is to be freed.
 * Won't expunge if library is open or if class is hanging around.
 */
void * __saveds
__asm myLibExpunge( register __a6 struct MyLibBase *mlb )
{
    void *seglist;

    D( printf("myLibExpunge\n") );
    if ( mlb->mlb_Library.lib_OpenCnt || mlb->mlb_MyClass )
    {
	D( printf("can't expunge, set LIBF_DELEXP\n") );
	mlb->mlb_Flags |= LIBF_DELEXP;
    	return ( NULL );
    }

    D( printf("expunging now.\n") );

    /* remove library from lib list */
    Remove( mlb );

    /* cache seglist before freeing library 	*/
    seglist = mlb->mlb_SegList;
    D( printf("seglist at %lx\n", seglist ) );

    /* free library node */
    D( printf("freeing library node at %lx, size (hex) %lx\n", 
     mlb,mlb->mlb_Library.lib_NegSize+mlb->mlb_Library.lib_PosSize) );

    /* free library base data structure, including the part
     * *before* the Library pointer.
     */
    FreeMem( ((UBYTE *)mlb) - mlb->mlb_Library.lib_NegSize, 
	    mlb->mlb_Library.lib_NegSize+mlb->mlb_Library.lib_PosSize);

    return ( seglist );
}

/*
 * called directly.
 * returns NULL if it isn't expunging
 */
void * __saveds
__asm myLibClose( register __a6 struct MyLibBase *mlb )
{
    struct IClass	*cl;

    D( printf("closelib, base at %lx, opencnt %lx\n", mlb,
    	mlb->mlb_Library.lib_OpenCnt ) );
    D( printf("object count %lx, subclass count %lx\n",
	mlb->mlb_MyClass->cl_ObjectCount,mlb->mlb_MyClass->cl_SubclassCount));

    if ( --mlb->mlb_Library.lib_OpenCnt <= 0 )
    {
	D( printf("we're the last closer\n") );

	/* Last user is closing.  Try to free up the class
	 * so we can be expunged.
	 */
	if ( (cl = mlb->mlb_MyClass) == NULL )
	{
	    /* this case shouldn't happen, but you never know */
	    D( printf("surprised to find my class == NULL!\n") );
	    closeAll();
	    goto OUT;
	}

	RemoveClass( cl );	/* make class unavailable */
	D( printf("class removed\n") );

	/* now that nobody can get at the class, see if anybody
	 * still depends on it.  There's a slight hitch here,
	 * in that we're assuming that if the two 'counts' are
	 * zero, that we'll be able to successfully free our
	 * class, OR, that it will be usable after we try.
	 */
	if ( cl->cl_ObjectCount == 0 &&
	     cl->cl_SubclassCount == 0 &&
	     freeEmbBClass( cl ) )
	{
	    D( printf("successfully freed class\n") );
	    closeAll();
	    mlb->mlb_MyClass = NULL;	/* means we're expungable */
	}

	/* else, we're all closed, our class is removed, but
	 * we're not expungable.
	 */
    }
OUT:
    /* delayed expunge stuff */
    if ( mlb->mlb_Library.lib_OpenCnt == 0 &&
    	(mlb->mlb_Flags & LIBF_DELEXP ) )
    {
	D( printf("closelib: delayed expunge\n") );
	return ( myLibExpunge( mlb ) );
    }

    D( printf("closelib: not expunged, open count %lx, flags %lx\n",
    	mlb->mlb_Library.lib_OpenCnt, mlb->mlb_Flags ) );
    return ( NULL );	/* not expunged */
}

/* returns 0 if failure */
openAll( mlb )
struct MyLibBase	*mlb;
{
    if (!(UtilityBase=(struct Library *)OpenLibrary("utility.library",36L)))
    	return ( 0 );

    if (!(IntuitionBase = 
	(struct IntuitionBase *) OpenLibrary("intuition.library", 36L)))
    	return ( 0 );

    if (!(GfxBase = (struct GfxBase *) OpenLibrary("graphics.library", 36L)))
    	return ( 0 );

    return ( 1 );
}

closeAll()
{
    if (IntuitionBase) CloseLibrary(IntuitionBase);
    if (GfxBase) CloseLibrary(GfxBase);
    if (UtilityBase) CloseLibrary(UtilityBase);
}
