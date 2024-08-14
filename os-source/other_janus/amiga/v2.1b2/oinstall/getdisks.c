/* getdisks.c - Grab all available disk devices and return them to you in a
 *       simple exec list. The list is made up of named nodes--the "names"
 *       being the device name. 
 *
 *     Phillip Lindsay (c) 1987 Commodore-Amiga Inc. 
 * You may use this source as long as the copyright notice is left intact.
 */
#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>
#include <libraries/filehandler.h>

#include "protos.h"

extern struct DosLibrary *DOSBase;

/* btoc() takes a pointer to a string in BCPL format and converts it to a
 * C string.
 */
VOID btoc(UBYTE *bstring)
{
   UBYTE len,count,*cstring;
 
   cstring = (UBYTE *) bstring;
   len = cstring[0];              
   for(count=0;count < len;count++)
      cstring[count] = cstring[count+1];
   cstring[count] = '\0';
}

/* GetNode() will build a node structure for you. It will append memory to the
 *    node structure for the node name passed.
 */      
struct Node *GetNode(UBYTE *name,UBYTE type,UBYTE pri)
{
   struct Node *mynode;
   char        *myname;
   UBYTE         *mymemory;
   ULONG         mynamesize;

   mynamesize =( ((ULONG)strlen(name)) ? (ULONG)strlen(name)+1 : 0L );   

   mymemory = (UBYTE *) 
      AllocMem((ULONG)sizeof(*mynode)+mynamesize,MEMF_PUBLIC | MEMF_CLEAR); 

   if(!mymemory) return((struct Node *)NULL);
 
   mynode = (struct Node *) mymemory; 
   if(mynamesize)
   {
      myname = (char *) mymemory+(ULONG)sizeof(*mynode);
      strcpy(myname,name);
      mynode->ln_Name = myname;
   }
   mynode->ln_Type = type;
   mynode->ln_Pri  = pri;

   return(mynode);
}

/* This function assumes you used GetNode() for node initialization. Will
 *    free all memory used by node. Make sure you remove node from any list. 
 */
VOID FreeNode(struct Node *mynode)
{
   ULONG mymemsize;

   mymemsize = (ULONG) sizeof(*mynode);
   mymemsize+= ((mynode->ln_Name) ? (ULONG)strlen(mynode->ln_Name)+1 : 0L);
 
   FreeMem(mynode,mymemsize);
}


/* GetDisks() will grab all disk device names in the system device list and
 *  append an exec node to a given list. The node being named in respect to the
 *  device.  
 */
VOID GetDisks(struct List *dlist)
   /* passed a pointer to a initialize exec list */
{
   extern struct DosLibrary      *DOSBase;
   struct RootNode          *rnode; 
   struct DosInfo             *dinfo;
   struct DeviceNode     *dnode;
   struct Node         *adisk;
   char     *bname,name[32];

   rnode   = (struct RootNode *)  DOSBase->dl_Root;
   dinfo   = (struct DosInfo  *)  BADDR(rnode->rn_Info);
 
   Forbid();
   for(dnode = (struct DeviceNode *) BADDR(dinfo->di_DevInfo);BADDR(dnode);
      dnode = (struct DeviceNode *) BADDR(dnode->dn_Next))
   {
      if(!dnode->dn_Type && dnode->dn_Task && BADDR(dnode->dn_Name)) 
      {
         bname = (char *) BADDR(dnode->dn_Name);    
         movmem(bname,name,(ULONG)bname[0]+1L);
         btoc(name);
         if((adisk=GetNode(name,0,0))) AddTail(dlist,adisk);    
      }
   }
   Permit(); 
}

/*FreeDisks() will free all nodes in a given list. Function assumes nodes where
 *    initialized with GetNode().
 */
VOID FreeDisks(struct List *dlist)
{
   struct Node *disk;

   while((disk=RemTail(dlist)))
      FreeNode(disk);
}
