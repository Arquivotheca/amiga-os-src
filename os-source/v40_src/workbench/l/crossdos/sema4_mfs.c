/* Sema4_MFS.c **************************************************************
** Copyright 1991 CONSULTRON
*
*   CrossDOS semaphore for Text Filter/Translator functions.
*
*************************************************************************/

#include "FS:FS.h"

struct trans_table      *trans_tbl=0;

extern char             DeviceName[];
extern struct FS        *fsys;
extern struct ExecBase  *SysBase;

static struct Node *FindNameNC(struct List *list, STRPTR name);
static void AddSemaphore_( struct SignalSemaphore *sigSem );
static struct CrossDOSLock *Make_CDSema4_Node(UBYTE *sema4name);
static struct CrossDOSHandler *Make_CDHandler_Node(struct CrossDOSLock *sema4,UBYTE *dname);

/**********************************************************************
*   FindNameNC()    -- equivalent to FindName() except NOT Case-sensitive
**********************************************************************/
static struct Node *FindNameNC(struct List *list, STRPTR name)
{
    struct Node *node;
    WORD         result;

    node = list->lh_Head;
    while (node->ln_Succ)
    {
        if(0 ==(result = stricmp(name,node->ln_Name))) return(node);
        node = node->ln_Succ;
    }

    return(NULL);
}


/**********************************************************************
*   AddSemaphore_();
**********************************************************************/
static void AddSemaphore_( struct SignalSemaphore *sigSem )
{
    /* Do not use AddSemaphore for code to be used prior to exec V36.
     See autodocs exec/AddSemaphore() for other code */

    if((((struct Library *)(*(ULONG *)0x04))->lib_Version) >= 36) return(AddSemaphore( sigSem ));

    sigSem->ss_Link.ln_Type=NT_SIGNALSEM;
    InitSemaphore(sigSem);
    Forbid();
    Enqueue(&(((struct ExecBase *)(*(ULONG *)0x04))->SemaphoreList),(struct Node *)sigSem);
    Permit();
}


/**********************************************************************
*   Make_CDSema4_Node() -- Make new CrossDOSLock (Sema4) node.
*
*   This function is add just in case the IPrefs that the user
*   is using does not support CrossDOS prefs.  If this is the
*   case, I suggest the user upgrade his IPrefs program.
**********************************************************************/
static struct CrossDOSLock *Make_CDSema4_Node(UBYTE *sema4name)
{
    UBYTE sema4name_sz = strlen(sema4name)+1;
    struct CrossDOSLock *sema4;
    UBYTE *strct;

    if(sema4 = (struct CrossDOSLock *)(strct = 
        AllocMem(sizeof(struct CrossDOSLock)+sema4name_sz, MEMF_PUBLIC|MEMF_CLEAR)))
    {
        CopyMem(sema4name,&strct[sizeof(struct CrossDOSLock)],sema4name_sz);
        sema4->cdl_StructSize = sizeof(struct CrossDOSLock);
        InitSemaphore((struct SignalSemaphore *)sema4);
        sema4->cdl_Lock.ss_Link.ln_Name = &(strct[sizeof(struct CrossDOSLock)]);
        NewList(&sema4->cdl_Handlers);
        NewList(&sema4->cdl_TransTables);
        AddSemaphore_((struct SignalSemaphore *)sema4);
    }
    return(sema4);
}


/**********************************************************************
*   Make_CDHandler_Node() -- Make new CrossDOSHandler node.
**********************************************************************/
static struct CrossDOSHandler *Make_CDHandler_Node(struct CrossDOSLock *sema4,UBYTE *dname)
{
    UBYTE dname_sz = strlen(dname)+1;
    struct CrossDOSHandler *s_cdh;
    UBYTE *strct;

    if(s_cdh = (struct CrossDOSHandler *)(strct = 
        AllocMem(sizeof(struct CrossDOSHandler)+dname_sz, MEMF_PUBLIC|MEMF_CLEAR)))
    {
        CopyMem(dname,&strct[sizeof(struct CrossDOSHandler)],dname_sz);
        s_cdh->cdh_Link.ln_Name = &(strct[sizeof(struct CrossDOSHandler)]);
        AddHead(&(sema4->cdl_Handlers), &(s_cdh->cdh_Link));
    }
    return(s_cdh);
}


/**********************************************************************
*   Check_Sema4() -- Check the CrossDOS semaphore struct
*       for changes to the internal environment variables of the FS.
*       Current environment types supported are text filter and
*       translator flags.
**********************************************************************/
void Check_Sema4()
{
    struct CrossDOSLock *sema4;
    struct CrossDOSHandler *s_cdh;
    struct CrossDOSTrans   *s_cdt;

    if(0 == (sema4 = fsys->f_sema4))
    {   /* CrossDOS sema4 not found yet */
        Forbid();
        if(0 == (sema4 = (struct CrossDOSLock *)FindSemaphore(CROSSDOSNAME)))
        {   /* Could not find sema4.  Make new one */
            sema4 = Make_CDSema4_Node(CROSSDOSNAME);
        }
        Permit();
        fsys->f_sema4 = sema4;
    }
    if(sema4)
    {
        Forbid();
        ObtainSemaphore((struct SignalSemaphore *)sema4);
        Permit();
    }
     else return;

    if(0 == (s_cdh = fsys->f_sema4_hndlr))
    {   /* CrossDOS sema4 handler node not found yet */

        if(0 == (s_cdh = (struct CrossDOSHandler *)FindNameNC(&(sema4->cdl_Handlers), DeviceName)))
        {   /* Could not find handler node for this device.  Make new one */
            s_cdh = Make_CDHandler_Node(sema4,DeviceName);
        }
        fsys->f_sema4_hndlr = s_cdh;
    }

    if(s_cdh)
    {
    /* get filter/translate flags and set global flags in the FS */
        if(s_cdh->cdh_Flags & CDF_FILTER) fsys->f_FSflags |= MF_GLOB_TXFLTR;   /* turn on text filter */
         else fsys->f_FSflags &= ~MF_GLOB_TXFLTR;   /* turn off text filter */
        if(s_cdh->cdh_Flags & CDF_TRANSLATE) fsys->f_FSflags |= MF_GLOB_TXTRANS;   /* turn on text translate */
         else fsys->f_FSflags &= ~MF_GLOB_TXTRANS;   /* turn off text translate */
    /* get pointer to translate table if available */
        if(s_cdt = s_cdh->cdh_TransTable)
        {
            trans_tbl = (struct trans_table *)&(s_cdt->cdt_AToM);
        }
         else trans_tbl = 0;
    /* Reset all non-recognized flag bits to zero */
        s_cdh->cdh_Flags &= (CDF_FILTER|CDF_TRANSLATE);

    /* If a task is available to notify, do so at this time */
        if(s_cdh->cdh_NotifyTask) Signal(s_cdh->cdh_NotifyTask,1<<s_cdh->cdh_NotifySigBit);
    }

    ReleaseSemaphore((struct SignalSemaphore *)sema4);
}
