/* getdisks.c - Grab all available disk devices and return them to you in a
 * 		simple exec list. The list is made up of named nodes--the "names"
 *	 	being the device name. 
 *
 * 	 Phillip Lindsay (c) 1987 Commodore-Amiga Inc. 
 * You may use this source as long as the copyright notice is left intact.
 *
 * modified by Don Meyer 9-Jan-89 to use short ints.
 *
 */

#include "standard.h"
#include <exec/nodes.h>
#include <exec/lists.h>
#include <libraries/filehandler.h>
#include "getdisks.h"
#include "dbug.h"

extern struct DosLibrary *DOSBase;

static void btoc PROTO((char *));
static struct Node *GetNode PROTO((char *, UBYTE, UBYTE));
static void FreeNode PROTO((struct Node *));

/*
 * btoc() takes a pointer to a string in BCPL format and converts it to a
 * C string.
 */

static void btoc (bstring)
char *bstring;
{
    register UBYTE len;
    register UBYTE count;
    register UBYTE *cstring;

    DBUG_ENTER ("btoc");
    cstring = (UBYTE *) bstring;
    len = cstring[0];
    for (count = 0; count < len; count++) {
	cstring[count] = cstring[count + 1];
    }
    cstring[count] = '\0';
    DBUG_VOID_RETURN;
}

/*
 * GetNode() will build a node structure for you. It will append memory to the
 * 	node structure for the node name passed.
 */

static struct Node *GetNode (name, type, pri)
char *name;
UBYTE type;
UBYTE pri;
{
    register struct Node *mynode;
    register char *myname;
    register UBYTE *mymemory;
    register ULONG mynamesize;

    DBUG_ENTER ("GetNode");
    mynamesize = (((ULONG) strlen (name)) ? (ULONG) strlen (name) + 1 : 0L);
    mymemory = (UBYTE *) AllocMem ((ULONG) sizeof (struct Node) + mynamesize,
		MEMF_PUBLIC | MEMF_CLEAR);
    if (!mymemory) {
	DBUG_RETURN ((struct Node *) NULL);
    }
    mynode = (struct Node *) mymemory;
    if (mynamesize) {
	myname = (char *) mymemory + (ULONG) sizeof (*mynode);
	strcpy (myname, name);
	mynode -> ln_Name = myname;
    }
    mynode -> ln_Type = type;
    mynode -> ln_Pri = pri;
    DBUG_RETURN (mynode);
}

/*
 * This function assumes you used GetNode() for node initialization. Will
 * 	free all memory used by node. Make sure you remove node from any list. 
 */

static void FreeNode (mynode)
struct Node *mynode;
{
    register ULONG mymemsize;

    DBUG_ENTER ("FreeNode");
    mymemsize = (ULONG) sizeof (*mynode);
    mymemsize += ((mynode -> ln_Name) ?
		 (ULONG) strlen (mynode -> ln_Name) + 1 : 0L);
    FreeMem (mynode, mymemsize);
    DBUG_VOID_RETURN;
}

/*
 *  getdisks() will grab all disk device names in the system device list and
 *  append an exec node to a given list. The node being named in respect to the
 *  device.  
 */

void getdisks (dlist)
struct List *dlist;	/* passed a pointer to a initialize exec list */
{
    extern struct DosLibrary *DOSBase;
    struct RootNode *rnode;
    struct DosInfo *dinfo;
    register struct DeviceNode *dnode;
    register struct Node *adisk;
    char *bname;
    char name[32];


    DBUG_ENTER ("getdisks");
    rnode = (struct RootNode *) DOSBase -> dl_Root;
    dinfo = (struct DosInfo *) BADDR (rnode -> rn_Info);


    Forbid ();

    for(  dnode = (struct DeviceNode *)BADDR( dinfo->di_DevInfo );
	    BADDR( dnode );
	    dnode = (struct DeviceNode *)BADDR( dnode->dn_Next )  )
	{
		if(  ( ! dnode->dn_Type ) &&
			( dnode->dn_Task ) &&
			BADDR( dnode->dn_Name )  )
		{
		    bname = (char *)BADDR( dnode->dn_Name );
	    	movmem( bname, name, (int)(bname[0] + 1L) );
		    btoc( name );
		    if( (adisk = GetNode(name, 0, 0)) )
			{
				AddTail(dlist, adisk);
		    }
		}
    }
    Permit ();


    DBUG_VOID_RETURN;
}

/*
 *  freedisks() will free all nodes in a given list. Function assumes nodes where
 *    initialized with GetNode().
 */

void freedisks (dlist)
struct List *dlist;
{
    register struct Node *disk;
    extern struct Node *RemTail ();

    DBUG_ENTER ("freedisks");
    while ((disk = RemTail (dlist))) {
	FreeNode (disk);
    }
    DBUG_VOID_RETURN;
}
