/*
 * $Id: makereal.c,v 1.2 91/02/12 20:23:54 mks Exp $
 *
 * $Log:	makereal.c,v $
 * Revision 1.2  91/02/12  20:23:54  mks
 * Did a work-around for a SAS/C compiler bug.
 * 
 * Revision 1.1  91/02/11  14:52:31  mks
 * Initial revision
 *
 */

/*
 * MakeReal is an AppMenuItem that takes the selected icons and makes
 * them Real.
 */

/*
******* MakeReal *************************************************************
*
*   NAME
*	MakeReal
*
*   SYNOPSIS
*	MakeReal - Takes a fake icon and make it real
*
*   FUNCTION
*	Takes the selected icons and makes them into real icons.
*
*	If there are no icons selected when the MakeReal menu is selected,
*	MakeReal will exit.
*
*   INPUTS
*	icons -- Selected icons when the menu item is selected
*
*   RESULTS
*	An icon written to disk.
*
*   SEE ALSO
*	None
*
*   BUGS
*
******************************************************************************
*/

#include    <exec/types.h>
#include    <exec/memory.h>
#include    <intuition/intuition.h>
#include    <workbench/workbench.h>
#include    <workbench/startup.h>
#include    <utility/tagitem.h>

#include    <clib/exec_protos.h>
#include    <clib/dos_protos.h>
#include    <clib/icon_protos.h>
#include    <clib/wb_protos.h>

#include    <pragmas/exec_pragmas.h>
#include    <pragmas/dos_pragmas.h>
#include    <pragmas/icon_pragmas.h>
#include    <pragmas/wb_pragmas.h>

#include    <string.h>

#include    "makereal_rev.h"

long make_real(void)
{
struct Library *SysBase=(*((struct Library **) 4));
struct Library *DOSBase;
struct Library *IconBase;
struct Library *WorkbenchBase;
struct MsgPort *mport;
struct Message *msg=NULL;
struct Process *proc;

    proc=(struct Process *)FindTask(NULL);
    if (!(proc->pr_CLI))
    {
        WaitPort(&(proc->pr_MsgPort));
        msg=GetMsg(&(proc->pr_MsgPort));
    }

    if (DOSBase=OpenLibrary("dos.library",37))
    {
        if (IconBase=OpenLibrary("icon.library",37))
        {
            if (WorkbenchBase=OpenLibrary("workbench.library",37))
            {
                if (mport=CreateMsgPort())
                {
                void *apkey;

                    if (apkey=AddAppMenuItemA(0,0,"Make Real",mport,NULL))
                    {
                    struct AppMessage *amsg;
                    short quit=FALSE;

                        while (!quit)
                        {
                            WaitPort(mport);
                            while (amsg=(struct AppMessage *)GetMsg(mport))
                            {
                            struct WBArg *warg=amsg->am_ArgList;
                            long args=amsg->am_NumArgs;

                                if (!args) quit=TRUE;

                                while (args--)
                                {
                                struct DiskObject *obj;
                                BPTR oldlock;
                                BPTR newlock=NULL;
                                struct FileInfoBlock *fib=NULL;
                                char *name;

                                    if (warg->wa_Lock)
                                    {
                                        oldlock=CurrentDir(warg->wa_Lock);

                                        if (name=warg->wa_Name) if (!*name) name=NULL;
                                        if (!name)
                                        {
                                            if (newlock=ParentDir(warg->wa_Lock))
                                            {

                                                CurrentDir(newlock);
                                                if (fib=AllocVec(sizeof(struct FileInfoBlock),MEMF_PUBLIC))
                                                {
                                                    if (Examine(warg->wa_Lock,fib)) name=fib->fib_FileName;
                                                }
                                            }
                                            else name="disk" VERSTAG;
                                        }

                                        if (name) if (!(obj=GetDiskObject(name)))
                                        {
                                            if (obj=GetDiskObjectNew(name))
                                            {
                                                PutDiskObject(name,obj);
                                            }
                                        }
                                        if (obj) FreeDiskObject(obj);

                                        CurrentDir(oldlock);
                                        UnLock(newlock);
                                        FreeVec(fib);
                                    }
                                    warg++;
                                }
                                ReplyMsg(amsg);
                            }
                        }
                        RemoveAppMenuItem(apkey);
                    }
                    DeleteMsgPort(mport);
                }
                CloseLibrary(WorkbenchBase);
            }
            CloseLibrary(IconBase);
        }
        CloseLibrary(DOSBase);
    }

    if (msg)
    {
	Forbid();
	ReplyMsg(msg);
    }

    return(0);
}
