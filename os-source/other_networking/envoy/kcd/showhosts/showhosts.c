

/* Kludge-Central.  It may be an ugly hack, but it works!
 */



#include <stdio.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <envoy/nipc.h>
#include <utility/tagitem.h>
#include <utility/hooks.h>
#include <dos/dos.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/nipc_pragmas.h>
#include <clib/exec_protos.h>
#include <clib/nipc_protos.h>
#include <clib/utility_protos.h>

typedef ULONG (*HOOK_FUNC)();

BOOL __asm __saveds CallBack(register __a0 struct Hook *hook,
                    register __a1 struct TagItem *taglist,
                    register __a2 struct Task *task);

extern struct Library *SysBase;

 struct Library *NIPCBase;
 struct Library *UtilityBase;
 struct List GlobalList;

 struct XNode
 {
    struct Node  xn_Node;
    ULONG        xn_IP;
    ULONG	 xn_AvailFast;
    ULONG	 xn_Owner[32];
 };


main(int argc, char **argv)
{

    struct Hook myhook;
    ULONG search[8]={MATCH_REALM,0,QUERY_HOSTNAME,0,QUERY_OWNER,0,TAG_DONE,0};
    UBYTE yank[64], *find;
    BOOL wasone=FALSE;
    BOOL doit=FALSE;

    NewList(&GlobalList);

    if (argc == 2)
        if (argv[1][1] == '?')
        {
            printf("Syntax: showhosts <optional realm>\n");
            exit(0);
        }

    doit=FALSE;
    if (argc == 2)
        if (!argv[1])
            doit=TRUE;

    if (argc == 1)
        doit = TRUE;


    if (doit)
    {
        int x,y;
        NIPCBase = (struct Library *) OpenLibrary("nipc.library",37L);
        if (!NIPCBase)
            exit(1);
        GetHostName(0L,yank,64);
        CloseLibrary(NIPCBase);
        y = strlen(yank);
        for (x = 0; x < y; x++)
        {
            if (yank[x] == ':')
            {
                yank[x] = 0;
                wasone = TRUE;
            }
        }
        if (wasone)
            find = &yank[0];
    }
    else
    {
        find = argv[1];
        wasone = TRUE;
    }

    if (TRUE)
    {
        if ((strlen(find)) || (!wasone))
        {
            UtilityBase = (struct Library *) OpenLibrary("utility.library",37L);
            if (UtilityBase)
            {
                NIPCBase = (struct Library *) OpenLibrary("nipc.library",37L);
                if (NIPCBase)
                {
                    struct XNode *n;
                      search[1]= (ULONG) find;
                    if (!wasone)
                        search[0]=TAG_IGNORE;

                    myhook.h_Entry = (HOOK_FUNC) &CallBack;

                    if (NIPCInquiryA(&myhook,2,500,&search[0]))
                        Wait(SIGBREAKF_CTRL_F);

                    /* Weed out copies */
                    n = (struct XNode *) GlobalList.lh_Head;
                    while (n->xn_Node.ln_Succ)
                    {
                        struct XNode *m;
                        m = n;
                        while (m->xn_Node.ln_Succ)
                        {
                            if ((!strcmp(m->xn_Node.ln_Name,n->xn_Node.ln_Name)) && (m != n))
                            {
                                struct XNode *o;
                                o = m;
                                m = (struct XNode *) m->xn_Node.ln_Pred;
                                Remove(o);
                                FreeMem(o->xn_Node.ln_Name,strlen(o->xn_Node.ln_Name)+1);
                                FreeMem(o,sizeof(struct XNode));
                            }
                            m = (struct XNode *) m->xn_Node.ln_Succ;
                        }
                        n = (struct XNode *) n->xn_Node.ln_Succ;
                    }

                    while (n = (struct XNode *) RemHead(&GlobalList))
                    {
                        printf("Response: %32s\t%32s\n",n->xn_Node.ln_Name,n->xn_Owner);
                        FreeMem(n->xn_Node.ln_Name,strlen(n->xn_Node.ln_Name)+1);
                        FreeMem(n,sizeof(struct XNode));
                    }
                    CloseLibrary(NIPCBase);
                }
                CloseLibrary(UtilityBase);
            }
        }
    }
}



BOOL __asm __saveds CallBack(register __a0 struct Hook *hook,
                             register __a1 struct TagItem *taglist,
                             register __a2 struct Task *task)
{
    struct TagItem *tag;
    struct TagItem *tstate = taglist;
    struct XNode *xn=0L;

    if (taglist)
    {
        while (tag = (struct TagItem *) NextTagItem(&tstate))
        {
            switch (tag->ti_Tag)
            {
                case QUERY_IPADDR:
                {
                    if (xn)
                        xn->xn_IP = tag->ti_Data;
                    break;
                }
                case QUERY_AVAILFASTMEM:
                {
                    if (xn)
                    	xn->xn_AvailFast = tag->ti_Data;
                    break;
                }
                case QUERY_HOSTNAME:
                case QUERY_ENTITY:
                case QUERY_SERVICE:
                case QUERY_REALMS:
                {
                    STRPTR name;
                    struct XNode *newnode;
                    int x;
                    x = strlen(tag->ti_Data);
                    name = (STRPTR) AllocMem(x+1,MEMF_PUBLIC);
                    newnode = (struct XNode *) AllocMem(sizeof(struct XNode),MEMF_PUBLIC);
                    if ((name) && (newnode))
                    {
                        newnode->xn_Node.ln_Name = name;
                        xn = newnode;
                        strcpy(name,tag->ti_Data);
                        AddTail(&GlobalList,newnode);
                    }
                    else
                    {
                        xn = 0;
                        if (name)
                            FreeMem(name,x+1);
                        if (newnode)
                            FreeMem(newnode,sizeof(struct XNode));
                    }
                    break;
                }
                case QUERY_OWNER:
                {
                    if(xn)
                    	strcpy(xn->xn_Owner,(STRPTR)tag->ti_Data);
                }
            }
        }
    }
    else
        Signal(task,SIGBREAKF_CTRL_F);

    return(TRUE);

}

