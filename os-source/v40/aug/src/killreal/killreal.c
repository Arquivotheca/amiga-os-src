/*
 * $Id$
 *
 * $Log$
 */

/*
 * KillReal is an AppMenuItem that takes the selected icons and deletes
 * just the icon part.
 */

/*
******* KillReal *************************************************************
*
*   NAME
*	KillReal
*
*   SYNOPSIS
*	KillReal - Deletes just the icon part...
*
*   FUNCTION
*	Takes the selected icons and deletes the icon part.
*
*	If there are no icons selected when the KillReal menu is selected,
*	KillReal will exit.
*
*   INPUTS
*	icons -- Selected icons when the menu item is selected
*
*   RESULTS
*	Icons are deleted from disk.
*
*   SEE ALSO
*	icon.library/DeleteDiskObject()
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

#include    "killreal_rev.h"

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

                    if (apkey=AddAppMenuItemA(0,0,"Kill Real",mport,NULL))
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

                                        if (name) DeleteDiskObject(name);

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
