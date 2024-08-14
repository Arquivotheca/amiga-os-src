/*
 * Amiga Grand Wack
 *
 * special.h - Wack extensions common include.
 *
 * $Id: special.h,v 39.4 93/05/21 17:35:55 peter Exp $
 *
 * Contains macros to help show structures.  Based on a dumpXXX() function
 * you write, SHOWONE() builds a ShowXXX() function.  SHOWSIMPLE() builds
 * both ShowXXX() and ShowXXXList().
 *
 */

/* amiga core address for field in sun-buffered structure */
#define CADDR(base, buffer, field) 	\
    ((APTR)( (ULONG) (base) + ((ULONG) (field) - (ULONG) (buffer)) ))

#define REFRESH (flags = -1)
#define NOREFRESH (flags = 0)

#define IBADDR(a) ((APTR) ((ULONG) (a) - (ULONG)(&IBase) + (ULONG)(ibase)))

/* builds callers to dumpT(addr, ptr) where T is not a node	*/
#define SHOWONE(T) \
	STRPTR Show ## T( char *arg ) \
	{ struct T *w;ULONG dump ## T(); if (parseAddress(arg, (ULONG *)&w)) { \
	doMinNode((struct MinNode *)w, sizeof (struct T), dump ## T); }\
	else BadSyntax(); return( NULL ); }

/* builds callers to dumpT(addr, ptr) where T is a node in a linked list */
#define SHOWSIMPLE(T,newline) \
	STRPTR Show ## T( char *arg ) \
	{ struct T *w;ULONG dump ## T(); if (parseAddress(arg, (ULONG *)&w)) { \
	doMinNode((struct MinNode *)w, sizeof (struct T), dump ## T); }\
	else BadSyntax(); return( NULL ); }\
	\
	STRPTR Show ## T ## List( char *arg ) \
	{ struct T *w;ULONG dump ## T(); if (parseAddress(arg, (ULONG *)&w)) { \
	WalkSimpleList(w, sizeof (struct T), dump ## T, newline); }\
	else BadSyntax(); return( NULL ); }

#if 0 	/* Here's what an example looks like	*/

SHOWSIMPLE( Window, newline ) yields:

STRPTR ShowWindow( char *arg )
{
    struct Window	*w;

    ULONG	dumpWindow();
    if (parseAddress(arg, (ULONG *)&w))
    {
	doMinNode( (struct MinNode *)w, sizeof (struct Window), dumpWindow);
    }
    else BadSyntax();

    return( NULL );
}

STRPTR ShowWindowList( char *arg )
{
    struct Window	*w;

    ULONG	dumpWindow();
    if (parseAddress(arg, (ULONG *)&w))
    {
	WalkSimpleList( w, sizeof (struct Window), dumpWindow, newline );
    }
    else BadSyntax();

    return( NULL );
}

these call dumpWindow(APTR arg, struct Window *window);

#endif
