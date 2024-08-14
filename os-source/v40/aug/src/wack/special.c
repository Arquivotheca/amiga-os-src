/*
 * Amiga Grand Wack
 *
 * special.c -- Odd routines, used by the Graphics and Intuition displayers
 *
 * $Id: special.c,v 39.5 93/06/02 15:51:45 peter Exp $
 *
 * Contains some odd functions, including a library base finder, and
 * support routines to help walk lists and singly-linked lists and
 * retrieve the structures on those lists for further decoding.
 *
 */

#include "exec/types.h"
#include "exec/nodes.h"
#include "exec/lists.h"
#include "exec/interrupts.h"
#include "exec/memory.h"
#include "exec/ports.h"
#include "exec/libraries.h"
#include "exec/devices.h"
#include "exec/tasks.h"
#include "exec/resident.h"
#include "exec/execbase.h"

#include "libraries/dos.h"
#include "libraries/dosextens.h"

#include "symbols.h"
#include "std.h"

#include "sys_proto.h"
#include "show_proto.h"
#include "find_proto.h"
#include "main_proto.h"
#include "special_proto.h"
#include "link_proto.h"
#include "io_proto.h"

#define NAMESIZE 100

extern APTR CurrAddr;

/*
 * find and return library base address
 */
APTR
FindBase( char *name )
{
    struct Library dev; 
    char libname[NAMESIZE];
    struct ExecBase eb;
    struct Node *node;
    struct Node *succ;
    APTR   base = NULL;

    ReadBlock (ReadPointer( ABSSYSBASE ), &eb, sizeof (eb));
    for (node = (struct Node *) eb.LibList.lh_Head; succ = ReadPointer(node);
				    node = succ )
    {
/*PPrintf("Node at %lx\n", node);*/
	ReadBlock (node, &dev, sizeof (dev));
	GetName (dev.lib_Node.ln_Name, libname);
/*PPrintf("Name: %s\n", libname);*/
	if (!strcmp(name, libname))
	{
/*PPrintf("Got it\n");*/
	    base = (APTR) node;
	    goto OUT;
	}
    }
OUT:
    return (base);
}

void
ShowAddress( char *s, APTR x )
{
    PPrintf("%s at", s);
    if (strlen(s) < 6) PPrintf("\t");
    PPrintf("\t%8lx\n", x);
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

    node = ReadPointer(list);
    succ = ReadPointer(node);

    while ( succ )
    {
	retval = doNode(node, nodesize, nodefunc);
	if (retval) goto OUT;
	node = succ;
	succ = ReadPointer(node);
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

    buff = malloc(nodesize);

    /* fetch node */
    ReadBlock( node, buff, nodesize);

    /* fetch name */
    nameaddr = (APTR) ((struct Node *)buff)->ln_Name;
    GetName (nameaddr, name);

    /* call user function */
    retval = (*nodefunc)(node, buff, name);

OUT:
    free(buff);
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

    node = ReadPointer(list);
    succ = ReadPointer(node);

    while ( succ )
    {
	retval = doMinNode(node, nodesize, nodefunc);
	if (retval) goto OUT;
	node = succ;
	succ = ReadPointer(node);
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

    buff = malloc(nodesize);

    /* fetch node */
    ReadBlock( node, buff, nodesize);

    /* call user function */
    retval = (*nodefunc)(node, buff);

OUT:
    free(buff);
    return (retval);
}

/************************/
/** Singly linked list **/
/************************/

/* same as WalkList, but doesn't fetch node names */
ULONG
WalkSimpleList( APTR firstnode, int nodesize, ULONG (*nodefunc)(), BOOL newline )
{
    APTR node;
    ULONG retval;

    /* might want to malloc only once, here, rather than in doNode */

    node = firstnode;
    while ( node )
    {
	/* doMinNode can serve double duty */
	retval = doMinNode(node, nodesize, nodefunc);
	if (retval) goto OUT;
        if ( ( node = ReadPointer(node) ) && ( newline ) )
        {
	    NewLine();
	}
    }
OUT:
    return (retval);
}

