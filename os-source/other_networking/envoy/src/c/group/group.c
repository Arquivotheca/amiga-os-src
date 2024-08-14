/*
** group.c -- Allows setting of the GroupID of a file or directory
**
** Copyright 1992, Commodore-Amiga, Inc.
**
** $id$
**
**/

#include        <exec/types.h>
#include        <exec/memory.h>
#include        <dos/dos.h>
#include        <clib/dos_protos.h>
#include        <clib/exec_protos.h>
#include        <pragmas/dos_pragmas.h>
#include        <pragmas/exec_pragmas.h>

#include        "group_rev.h"

STRPTR vs=VERSTAG;

/* temp, or until Randell finally gives me #'s for them ... */
#define ACTION_Dummy                20000
#define ACTION_USERNAME_TO_UID      (ACTION_Dummy+0)
#define ACTION_GROUPNAME_TO_GID     (ACTION_Dummy+1)
#define ACTION_UID_TO_USERINFO      (ACTION_Dummy+2)
#define ACTION_GID_TO_GROUPINFO     (ACTION_Dummy+3)



#define TEMPLATE    "FILE/A,GROUP/A,ALL/S,QUIET/S"
#define OPT_FILE    0
#define OPT_GROUP   1
#define OPT_ALL     2
#define OPT_QUIET   3
#define OPT_COUNT   4

void strcpy(char *, char *);
int strlen(char *);

ULONG owner(void)
{
    struct Library *SysBase = (*((struct Library **) 4));
    struct Library *DOSBase;
    ULONG opts[OPT_COUNT];
    ULONG err, y;
    struct RDArgs *rdargs;

    DOSBase = (struct Library *) OpenLibrary("dos.library",37);
    if (DOSBase)
    {
        for (y = 0; y < OPT_COUNT; y++)
            opts[y]=0L;

        if (!(rdargs = ReadArgs(TEMPLATE, &opts[0], NULL)))
        {
            PrintFault(IoErr(),NULL);
        }
        else
        {
            struct AnchorPath *ap;
            ULONG newowner=0;
            ap = (struct AnchorPath *)  AllocMem(sizeof(struct AnchorPath) + 255,MEMF_CLEAR);
            if (ap)
            {
                int indent=1;
                ap->ap_Strlen = 255;
                ap->ap_Flags = APF_DOWILD;
                ap->ap_BreakBits = SIGBREAKF_CTRL_C;
                err = MatchFirst((UBYTE *)opts[OPT_FILE],ap);
                if (err)
                {
                    err = IoErr();
                    if (err != ERROR_NO_MORE_ENTRIES)
                        PrintFault(err,NULL);
                }
                else
                {
                    if ( (!(ap->ap_Flags & APF_ITSWILD)) && (!opts[OPT_ALL]) )
                        opts[OPT_QUIET] = TRUE;

                    do
                    {
                        ULONG res1;
                        struct MsgPort *m;
                        ULONG ownercode;
                        int x;
                        BPTR  l;
                        STRPTR fn=(STRPTR) (&(ap->ap_Buf[0]));

                        if (ap->ap_Flags & APF_DIDDIR)
                        {
                            ap->ap_Flags &= ~APF_DIDDIR;
                            indent--;
                            continue;
                        }

                        if ((ap->ap_Info.fib_DirEntryType >= 0) && opts[OPT_ALL])
                        {
                            ap->ap_Flags |= APF_DODIR;
                            indent++;
                        }

                        if (CheckSignal(SIGBREAKF_CTRL_C))
                        {
                            PrintFault(ERROR_BREAK,NULL);
                            break;
                        }
                        if (ap->ap_Current->an_Lock)
                        {
                            m = ((struct FileLock *) BADDR(ap->ap_Current->an_Lock))->fl_Task;
                            l = ap->ap_Current->an_Lock;
                        }
                        else
                        {
                            m = DeviceProc(fn);
                            l = 0L;
                        }

                        if (!opts[OPT_QUIET])
                        {
                            for (x = 0; x < indent; x++)
                                PutStr("   ");

                            PutStr(ap->ap_Info.fib_FileName);
                            if (ap->ap_Info.fib_DirEntryType >= 0)
                                PutStr(" (dir)");
                        }

                        if (m)
                        {
                            STRPTR b;

                            if (!newowner)
                            {
                                newowner = DoPkt(m,ACTION_GROUPNAME_TO_GID,opts[OPT_GROUP],0,0,0,0);
                                if ((IoErr() != ERROR_ACTION_NOT_KNOWN) && (!newowner))
                                {
                                    PutStr("\nUnknown group\n");
                                    break;
                                }
                                ownercode = (ap->ap_Info.fib_OwnerUID << 16) | (newowner);
                            }

                            b = AllocMem(128,0);
                            if (b)
                            {
                                strcpy(&b[1],ap->ap_Info.fib_FileName);
                                b[0]=strlen(&b[1]);
                                if (newowner)
                                {
                                    res1 = DoPkt(m,ACTION_SET_OWNER,0,(LONG)l,(LONG)MKBADDR(b),ownercode,0);
                                    if (!res1)
                                    {
                                        PutStr("...");
                                        PrintFault(IoErr(),NULL);
//                                        break;
                                    }
                                }
                                FreeMem(b,128);
                            }
                        }
                        if (!opts[OPT_QUIET])
                            PutStr("...Done\n");
                    }
                    while (!MatchNext(ap));

                    err = IoErr();
                    if ((err) && (err != ERROR_NO_MORE_ENTRIES))
                        PrintFault(err,NULL);
                    MatchEnd(ap);
                }
                FreeMem(ap,sizeof(struct AnchorPath) + 255);
            }
            FreeArgs(rdargs);
        }
        CloseLibrary(DOSBase);
    }
    return(0);
}


