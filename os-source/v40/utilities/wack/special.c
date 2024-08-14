
/* special.c -- special wack commands for intuition,
 * graphics, semaphores, ...
 * $Id: special.c,v 1.8 91/05/29 08:48:15 peter Exp $
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

#define NAMESIZE 100

extern APTR CurrAddr;

/*
 * find and return library base address
 */
APTR
FindBase(name)
char *name;
{
    struct Library dev; 
    char libname[NAMESIZE];
    struct ExecBase eb;
    struct Node *node;
    APTR   base = NULL;

    GetBlock (GetMemL (4), &eb, sizeof (eb));

    for (node = (struct Node *) eb.LibList.lh_Head; GetMemL (node);
				    node = (struct Node *) GetMemL (node))
    {
	GetBlock (node, &dev, sizeof (dev));
	GetName (dev.lib_Node.ln_Name, libname);
	if (!strcmp(name, libname))
	{
	    base = (APTR) node;
	    goto OUT;
	}
    }
OUT:
    return (base);
}

SpecialHelp()
{

    puts("Extended Command Symbols:");
    puts("    * -- indirect whereis");
    NewLine();
    puts("    execbase -- show ExecBase");
    puts("    dosbase -- show DosBase");
    NewLine();

    puts("    ibase -- show IntuitionBase");
    puts("    ascreen (=asc) -- show address of active screen");
    puts("    awindow (=awin) -- show address of active window");
    puts("    screen[s] (=sc) -- show Screen [list] at current address");
    puts("    window[s] (=win) -- show window [list] at current address");
    puts("    gadget[s] (=gad[s]) -- show gadget [list] at current address");
    puts("    fgadget (=fgad) -- show address of first gadget of last window seen");
    puts("    genv -- gadget environment");
    puts("    menu[s] -- show menu [strip] at current address");
    puts("    item[s] -- show menu item[s] at current address");
    puts("    requester[s] -- show requester [list] at current address");
    puts("    thing[s] -- show thing [list] at current address");
    puts("    pinfo -- show PropInfo structure at current address");
    puts("    image -- show Image structure at current address");
    puts("    itext -- show IntuiText structure at current address");
    NewLine();

    puts("    gfxbase -- show GraphicsBase");
    puts("    linfo -- show Layer_Info at current address");
    puts("    layer -- show layer");
    puts("    view -- show view");
    puts("    vext -- show view extra");
    puts("    viewport [viewports] (=vp [vps]) -- show viewport [list]");
    puts("    cop -- active copper list (no color table)");
    puts("    Cop -- active copper list (inc. color table)");
    puts("    copinit -- graphics copper header");
    NewLine();

    puts("    sem -- show SignalSemaphore at current address");
    puts("    sems -- show  SS's in list whose head is at current address");
    puts("    isems -- show intuition semaphore list");
    puts("    lisems -- show all layerinfo semaphores for intuition screens");
    NewLine();

    puts("  The following can also be invoked as (symbol <arg>)");
    puts("    limit -- set find limit to current address");
    puts("    find  (or \'/\') -- find current pattern between curr. addr. and limit");
    puts("    stacklimit (=sl)  -- set limit and curr. addr. for stack of <proc>");
    NewLine();
    FindHelp();	/* dump current find/limit setting */
}

ShowAddress(s, x)
char *s;
APTR x;
{
    printf("%s at", s);
    if (strlen(s) < 6) printf("\t");
    printf("\t%8lx\n", x);
}

/****************/
/** Exec Lists **/
/****************/

/* walk exec list and call function provided, passing it (Sun) address
 * of list node, and of buffer containing node name ("" if NULL).  if
 *  user function returns non-zero, stop walk and pass return value back.
 */
ULONG
WalkList(list, nodesize, nodefunc)
struct List *list;
int	nodesize;
ULONG (*nodefunc)();
{
    struct Node *node;
    ULONG  retval;

    /* might want to malloc only once, here, rather than in doNode */

    for (node =  (struct Node *) GetMemL(list); GetMemL (node);
				    node = (struct Node *) GetMemL (node))
    {
	retval = doNode(node, nodesize, nodefunc);
	if (retval) goto OUT;
    }

OUT:
    return (retval);
}

char *malloc();

doNode(node, nodesize, nodefunc)
struct Node *node;
int    nodesize;
ULONG (*nodefunc)();
{
    char   *buff;
    char  name[NAMESIZE];
    APTR  nameaddr;	/* node->ln_Name */
    ULONG retval;

    buff = malloc(nodesize);

    /* fetch node */
    GetBlock( node, buff, nodesize);

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
WalkMinList(list, nodesize, nodefunc)
struct List *list;
int	nodesize;
ULONG (*nodefunc)();
{
    struct Node *node;
    ULONG  retval;

    /* might want to malloc only once, here, rather than in doNode */

    for (node =  (struct Node *) GetMemL(list); GetMemL (node);
				    node = (struct Node *) GetMemL (node))
    {
	retval = doMinNode(node, nodesize, nodefunc);
	if (retval) goto OUT;
    }

OUT:
    return (retval);
}


doMinNode(node, nodesize, nodefunc)
struct Node *node;
int    nodesize;
ULONG (*nodefunc)();
{
    char   *buff;
    ULONG retval;

    buff = malloc(nodesize);

    /* fetch node */
    GetBlock( node, buff, nodesize);

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
WalkSimpleList(firstnode, nodesize, nodefunc)
APTR	firstnode;
int	nodesize;
ULONG (*nodefunc)();
{
    APTR node;
    ULONG  retval;

    /* might want to malloc only once, here, rather than in doNode */

    for (node = firstnode; node; node = (APTR) GetMemL(node))
    {
	/* doMinNode can serve double duty */
	retval = doMinNode(node, nodesize, nodefunc);
	if (retval) goto OUT;
    }

OUT:
    return (retval);
}

