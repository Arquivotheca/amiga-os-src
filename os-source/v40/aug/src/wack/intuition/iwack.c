/* intuition wack handler */

#include <exec/execbase.h>
#include <exec/memory.h>
#include <graphics/gfx.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>

#include <string.h>
#include <stdio.h>

#include "/protos/wack_protos.h"
#include "/pragmas/wack_pragmas.h"

#include "iwack_proto.h"

extern UWORD far VERSION;
extern UWORD far REVISION;

struct Library *SysBase;
struct Library *DOSBase;
struct MsgPort *WackBase;

#define NAMESIZE 100
#define ABSSYSBASE ((void *)4)

void ShowIBase( void );
void ShowThing( char *arg );
void ShowThingList( char *arg );
void ShowIClasses( void );
void ShowISems( void );
void ShowXScreen( char *arg );
void ShowXWindow( char *arg );
void ShowWindowFlags( char *arg );
void IDebug( char *arg );

#define TEMPLATE "wackport/a,ibase/s,thing/s,things/s,classes/s,isems/s,screen/s,window/s,wflags/s,idebug/s,version/s,arg/f"
#define OPT_WACKPORT	0
#define OPT_IBASE	1
#define OPT_THING	2
#define OPT_THINGS	3
#define OPT_CLASSES	4
#define OPT_ISEMS	5
#define OPT_SCREEN	6
#define OPT_WINDOW	7
#define OPT_WFLAGS	8
#define OPT_IDEBUG	9
#define OPT_VERSION	10
#define	OPT_ARG		11
#define OPT_COUNT	12

long
mymain( void )
{
    long x;
    long opts[OPT_COUNT];
    struct ReadArgs *rdargs;
    long result = 20;

    SysBase = (*((struct Library **) 4));

    if ( DOSBase = OpenLibrary("dos.library", 37 ) )
    {
	for ( x=0; x<OPT_COUNT; opts[x++]=0 )
	{
	}

	if ( rdargs = ReadArgs( TEMPLATE, opts, NULL ) )
	{
	    result = 5;
	    if ( WackBase = FindPort( (STRPTR)opts[OPT_WACKPORT] ))
	    {
		if ( opts[OPT_IBASE] )
		{
		    ShowIBase();
		    result = 0;
		}
		else if ( opts[ OPT_THING ] )
		{
		    ShowThing( (STRPTR)opts[OPT_ARG] );
		    result = 0;
		}
		else if ( opts[ OPT_THINGS ] )
		{
		    ShowThingList( (STRPTR)opts[OPT_ARG] );
		    result = 0;
		}
		else if ( opts[ OPT_CLASSES ] )
		{
		    ShowIClasses();
		    result = 0;
		}
		else if ( opts[ OPT_ISEMS ] )
		{
		    ShowISems();
		    result = 0;
		}
		else if ( opts[ OPT_SCREEN ] )
		{
		    ShowXScreen( (STRPTR)opts[OPT_ARG] );
		    result = 0;
		}
		else if ( opts[ OPT_WINDOW ] )
		{
		    ShowXWindow( (STRPTR)opts[OPT_ARG] );
		    result = 0;
		}
		else if ( opts[ OPT_WFLAGS ] )
		{
		    ShowWindowFlags( (STRPTR)opts[OPT_ARG] );
		    result = 0;
		}
		else if ( opts[ OPT_IDEBUG ] )
		{
		    IDebug( (STRPTR)opts[OPT_ARG] );
		    result = 0;
		}
		else if (opts[ OPT_VERSION ] )
		{
		    Wack_Printf("  Private Intuition extensions (Version %ld.%ld)\n", VERSION, REVISION );
		    result = 0;
		}
	    }
	    FreeArgs( rdargs );
	}
	CloseLibrary( DOSBase );
    }
    return( result );
}

void
Wack_Printf( STRPTR fmt, ... )
{
    Wack_VPrintf( fmt, ((ULONG *)&fmt)+1 );
}

void
GetName( APTR nameAddr, char *nameStr )
{
    if (nameAddr == 0) {
	nameStr[0] = 0;
    }
    else {
	Wack_ReadBlock(nameAddr, nameStr, NAMESIZE);
	nameStr[NAMESIZE - 1] = 0;
    }
}

/* no newline */
void
ShowAddressN( char *s, APTR x )
{
    Wack_Printf("%s at", s);
    if (strlen(s) < 6) Wack_Printf("\t");
    Wack_Printf("\t%8lx  ", x);
}

void
NewLine( void )
{
    Wack_Printf("\n");
}

void
ShowAddress( char *s, APTR x )
{
    ShowAddressN( s, x );
    NewLine();
}

void
dumpRect( UBYTE *s, struct Rectangle *r )
{
    Wack_Printf("%s\t", s);
    Wack_Printf("\tX: [%ld/%ld]\tY: [%ld/%ld]\n", r->MinX, r->MaxX, r->MinY, r->MaxY);
}

/* parses out single hex argument (doesn't advance argument pointer)
 * uses CurrAddr if argument not present.  returns false if 
 * syntax error (scanf fails).
 */
BOOL
hexArgAddr( UBYTE *arg, ULONG *val )
{
    BOOL retval = TRUE;
    ULONG argval;

    if ((arg) && (*arg))
    {
	if (sscanf (arg, " %lx ", &argval) >= 1)
	{
	    *val = argval;
	}
	else 
	{
	    retval = FALSE;
	}
    }
    else 
    {
	*val = (ULONG) Wack_ReadCurrAddr();
    }
    return (retval);
}

/* parses out single hex argument (doesn't advance argument pointer)
 * fails if argument not present.  Also fails if syntax error (scanf fails).
 */
BOOL
ScanArg( UBYTE *arg, ULONG *val )
{
    BOOL retval = FALSE;
    ULONG argval;

    if ((arg) && (*arg))
    {
	if (sscanf (arg, " %lx ", &argval) >= 1)
	{
	    *val = argval;
	    retval = TRUE;
	}
    }
    return (retval);
}

/****************/
/** Exec Lists **/
/****************/

/* walk exec list and call function provided, passing it (Sun) address
 * of list node, and of buffer containing node name ("" if NULL).  if
 *  user function returns non-zero, stop walk and pass return value back.
 */
ULONG
WalkList( struct List *list, int nodesize, ULONG (*nodefunc)(), BOOL newline )
{
    struct Node *node;
    struct Node *succ;
    ULONG  retval;

    /* might want to malloc only once, here, rather than in doNode */

    node = Wack_ReadPointer( list );
    succ = Wack_ReadPointer(node);
    while ( succ )
    {
	retval = doNode(node, nodesize, nodefunc);
	if (retval) goto OUT;
	node = succ;
	succ = Wack_ReadPointer(node);
	if ( ( succ ) && ( newline ) )
	{
	    NewLine();
	}
    }

OUT:
    return (retval);
}

ULONG
doNode( struct Node *node, int nodesize, ULONG (*nodefunc)( struct Node *, APTR, char * ) )
{
    char   *buff;
    char  name[NAMESIZE];
    APTR  nameaddr;	/* node->ln_Name */
    ULONG retval;

    buff = AllocVec(nodesize, MEMF_CLEAR);

    /* fetch node */
    Wack_ReadBlock( node, buff, nodesize );

    /* fetch name */
    nameaddr = (APTR) ((struct Node *)buff)->ln_Name;
    GetName (nameaddr, name);

    /* call user function */
    retval = (*nodefunc)(node, buff, name);

OUT:
    FreeVec(buff);
    return (retval);
}

/***************/
/** Min Lists **/
/***************/

/* same as WalkList, but doesn't fetch node names */
ULONG
WalkMinList( struct MinList *list, int nodesize, ULONG (*nodefunc)(), BOOL newline )
{
    struct MinNode *node;
    struct MinNode *succ;
    ULONG  retval;

    /* might want to malloc only once, here, rather than in doNode */

    node = Wack_ReadPointer( list );
    succ = Wack_ReadPointer( node );
    while ( succ )
    {
	retval = doMinNode(node, nodesize, nodefunc);
	if (retval) goto OUT;
	node = succ;
	succ = Wack_ReadPointer( node );
	if ( ( succ ) && ( newline ) )
	{
	    NewLine();
	}
    }

OUT:
    return (retval);
}

ULONG
doMinNode( struct MinNode *node, int nodesize, ULONG (*nodefunc)( struct MinNode *, APTR ) )
{
    char   *buff;
    ULONG retval;

    buff = AllocVec( nodesize, MEMF_CLEAR );

    /* fetch node */
    Wack_ReadBlock( node, buff, nodesize);

    /* call user function */
    retval = (*nodefunc)(node, buff);

OUT:
    FreeVec(buff);
    return( retval );
}

/************************/
/** Singly linked list **/
/************************/

/* same as WalkList, but doesn't fetch node names */
ULONG
WalkSimpleList( APTR firstnode, int nodesize, ULONG (*nodefunc)(), BOOL newline )
{
    APTR node;
    ULONG  retval;

    /* might want to malloc only once, here, rather than in doNode */

    node = firstnode;
    while ( node )
    {
	/* doMinNode can serve double duty */
	retval = doMinNode(node, nodesize, nodefunc);
	if (retval) goto OUT;
        if ( ( node = Wack_ReadPointer(node) ) && ( newline ) )
        {
	    NewLine();
	}
    }

OUT:
    return (retval);
}

void
BadSyntax( void )
{
    Wack_Printf("Bad syntax: expected hex address argument\n");
}

