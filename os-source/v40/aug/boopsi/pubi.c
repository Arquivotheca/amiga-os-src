
/* pubi.c -- :ts=8
 * installation of public image class by
 * program (probably run in background)
 * you type control-C to get this one to
 * try to free its class and exit.
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
#include <dos/dos.h>

#define D(x)	x

struct  IntuitionBase   *IntuitionBase;
struct  GfxBase         *GfxBase;
struct  Library         *UtilityBase;

/* class pointer is an abstract handle, which
 * one never uses for "public" access to a class,
 * but we as class owner can use for private access,
 * or to free the class.
 */
void	*initEmbBClass();
void	*EmbBClass;

Object	*NewObject();

struct EasyStruct myez = { sizeof (struct EasyStruct), 0,
	"Emboxclass Installer",
	"Cannot remove 'emboxclass'.",
	"Retry|Private|Public",
};

main()
{
    openAll();	/* get libraries open	*/

    /* init the public class	*/
    EmbBClass = initEmbBClass();

    D( kprintf("class is initialized\n") );

    printf("Class initialized, break ^C to terminate.\n");

    /* take a shot at removing it when we get a Break signal */
    for( Wait( SIGBREAKF_CTRL_C );; )
    {
    	D( kprintf("try to free class\n") );

	/* quit if class can be freed	*/
	if (  freeEmbBClass( EmbBClass ) )
	{
	    D( kprintf("FreeClass succeeded\n") );
	    break;
	}

	switch ( EasyRequest( NULL, &myez, 0 ) )
	{
	case 1:	/* retry without waiting	*/
	    D( kprintf("retry free\n") );
	    continue;

	default:
	    D( kprintf("default request return\n") );
	case 0:		/* public: add, and wait	*/
	    D( kprintf("add class back in\n") );
	    AddClass( EmbBClass );
	case 2:		/* private: just go wait some more	*/
	    break;
	}

	D( kprintf("wait for break again\n") );
	Wait( SIGBREAKF_CTRL_C );
   }

    cleanup( "all done" );
}

cleanup( str )
char	*str;
{
    if (str) printf("%s\n", str);

    if (IntuitionBase) CloseLibrary(IntuitionBase);
    if (GfxBase) CloseLibrary(GfxBase);
    if (UtilityBase) CloseLibrary(UtilityBase);

    exit (0);
}

/* exits via cleanup() if failure	*/
openAll()
{
    if (!(UtilityBase=(struct Library *)OpenLibrary("utility.library",36L)))
    { cleanup("no V36 utility library\n"); }

    if (!(IntuitionBase = 
	(struct IntuitionBase *) OpenLibrary("intuition.library", 36L)))
    { cleanup("no V36 intuition library\n"); }

    if (!(GfxBase = (struct GfxBase *) OpenLibrary("graphics.library", 36L)))
    { cleanup("no V36 graphics library\n"); }
}
