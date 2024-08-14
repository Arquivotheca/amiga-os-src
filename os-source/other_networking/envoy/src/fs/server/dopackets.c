
#include        <stdio.h>
#include        <string.h>
#include        <exec/types.h>
#include        <exec/memory.h>
#include        <dos/dos.h>
#include        <dos/dosextens.h>
#include        <dos/exall.h>
#include        <pragmas/exec_pragmas.h>
#include        <pragmas/dos_pragmas.h>
#include        <pragmas/utility_pragmas.h>
#include        <pragmas/nipc_pragmas.h>
#include        <clib/exec_protos.h>
#include        <clib/dos_protos.h>
#include        <clib/utility_protos.h>
#include        <clib/nipc_protos.h>
#include        "/fs.h"
#include        "fsdbase.h"

#define ERROR_EXECUTE_PROTECTED ERROR_READ_PROTECTED;
// 0xff00 is all the group and other bits
#define DEFAULTPERMS    (((FIBF_OTR_READ|FIBF_OTR_EXECUTE|FIBF_GRP_READ|FIBF_GRP_EXECUTE)) & 0xFF00)

// NOTE: all of these are inverted - 0 means permission
#define DEFAULT_REQUIRED	(FIBF_READ|FIBF_WRITE|FIBF_EXECUTE|FIBF_DELETE)

#define SAME 0

extern BOOL SafeExamine(BPTR lock, struct FileInfoBlock *f);

BPTR GetALock(BSTR a, BPTR b, struct MountedFSInfo *c, LONG type);
BPTR RealParent(BPTR alock, BSTR path, struct MountedFSInfo *mi);
struct EAC_Node *FindExall (BPTR lock, ULONG flags, struct MountedFSInfo *mi);
void RemExall (BPTR lock, ULONG done);
ULONG BestPermED(struct ExAllData *ed, ULONG type, struct MountedFSInfo *m, ULONG *flags);

static void BtoCstr (UBYTE *str)
{
	UBYTE len = *str;

	while (len--)
	{
		*str = *(str+1);
		str++;
	}
	*str = '\0';
}

void DoDosPacket(struct Transaction *t,struct MountedFSInfo *mi)
{
    struct TPacket *tp,*rtp;

    t->trans_RespDataActual = sizeof(struct TPacket);
    rtp = (struct TPacket *) t->trans_RequestData;
    tp = (struct TPacket *) t->trans_ResponseData;
    if (rtp != tp)
    {
	// if we're not reusing the input buffer, copy the packet info here
	// saves from doing it N places.  modifications to tp will get back to the client
	*tp = *rtp;
    }
    tp->tp_Res2 = 0;

//kprintf("Type %ld, %lx %lx %lx %lx %lx %lx\n",
//                tp->tp_Type, tp->tp_Arg1, tp->tp_Arg2, tp->tp_Arg3,
//                             tp->tp_Arg4, tp->tp_Arg5, tp->tp_Arg6);

    switch (tp->tp_Type)
    {
/* FINDOUTPUT needs to set the default r/w/e flags for group, other */
/* FINDOUTPUT ALSO needs to set the UID/GID of the creator of a file! */
        case ACTION_FINDINPUT:
        case ACTION_FINDOUTPUT:
        case ACTION_FINDUPDATE:
        {
            struct FileHandle *fh;
            ULONG perms=DEFAULT_REQUIRED, needed=DEFAULT_REQUIRED;
            tp->tp_Arg3 = MKBADDR( (ULONG) tp->tp_Arg3 + (ULONG) tp );
            if (!tp->tp_Arg2)
                tp->tp_Arg2 = (LONG) mi->mfsi_BaseLock;
            else
            {
                if (!(tp->tp_Arg2+1))
                {
                    tp->tp_Arg2 = 0L;
                }
            }

            if (mi->mfsi_Flags & MFSIF_SECURITYON)
            {
                BPTR templock;
                /* If security is on, I need to (painfully) try to get a lock on
                 * the file they want to open.  If I can't get a lock, they
                 * aren't allowed to open it.
                 */
                if (tp->tp_Type == ACTION_FINDINPUT)
                {
                    templock = GetALock(tp->tp_Arg3,tp->tp_Arg2,mi,ACCESS_READ);
                    if (!templock)
                    {
                        tp->tp_Res1 = DOSFALSE;
                        tp->tp_Res2 = IoErr();
                        break;
                    }
                    perms = BestPermFromLock((struct FileLock *)templock,mi);
                    UnLock(templock);
                    if (!(perms & FIBF_READ))
                    {
                        tp->tp_Res1 = DOSFALSE;
                        tp->tp_Res2 = ERROR_READ_PROTECTED;
                        break;
                    }
                } else if (tp->tp_Type == ACTION_FINDOUTPUT) {

	                templock = RealParent((BPTR) tp->tp_Arg2, (BSTR) tp->tp_Arg3, mi);
        	        if (templock)
                	{
	                    perms = BestPermFromLock((struct FileLock *)templock,mi);
	                    /* If Delete isn't set, I ought to try to lock the item they're trying
	                     * to create -- if it's there, and they're trying FINDOUTPUT,
	                     * fail.  FIXME. */
			}
	                UnLock(templock);
	                needed = FIBF_WRITE;

                } else if (tp->tp_Type == ACTION_FINDUPDATE) {

		    // if file exists, treat like OLDFILE.  If it doesn't, treat like newfile.
                    templock = GetALock(tp->tp_Arg3,tp->tp_Arg2,mi,ACCESS_READ);
                    if (!templock)
                    {
	                templock = RealParent((BPTR) tp->tp_Arg2, (BSTR) tp->tp_Arg3, mi);
        	        if (templock)
                	{
	                    perms = BestPermFromLock((struct FileLock *)templock,mi);
	                    /* If Delete isn't set, I ought to try to lock the item they're trying
	                     * to create -- if it's there, and they're trying FINDOUTPUT,
	                     * fail.  FIXME. */
			}
	                UnLock(templock);
	                needed = FIBF_WRITE;
                    } else {
	                perms = BestPermFromLock((struct FileLock *)templock,mi);
	                UnLock(templock);
	                if (!(perms & FIBF_READ))
	                {
	                    tp->tp_Res1 = DOSFALSE;
	                    tp->tp_Res2 = ERROR_READ_PROTECTED;
	                    break;
	                }
		    }
                }
            }

//kprintf("perms $%lx needed $%lx\n",perms,needed);
            if (perms & needed)
            {
                if ((tp->tp_Arg4) || (mi->mfsi_Flags))
                {
                    if (SameLock((BPTR)tp->tp_Arg2,(BPTR)mi->mfsi_BaseLock) == LOCK_SAME)
                    {
                        char fname[128];
                        char *p= (char *) BADDR(tp->tp_Arg3);
                        if (p[0] < 128)
                        {
                            CopyMem(&p[1],&fname[0],p[0]);
                            fname[p[0]] = 0;
                        }

                        if ((tp->tp_Arg4 & MFSIF_NOBACKDROP) || (mi->mfsi_Flags & MFSIF_NOBACKDROP))
                        {
                            if (!Strnicmp(&fname[0],".backdrop",128))
                            {
                                tp->tp_Res1 = DOSFALSE;
                                if (tp->tp_Type == ACTION_FINDINPUT)
                                    tp->tp_Res2 = ERROR_OBJECT_NOT_FOUND;
                                else
                                    tp->tp_Res2 = ERROR_WRITE_PROTECTED;
                                break;
                            }
                        }
                        if ((tp->tp_Arg4 & MFSIF_NODISKINFO) || (mi->mfsi_Flags & MFSIF_NODISKINFO))
                        {
                            if (!Strnicmp(&fname[0],"Disk.info",128))
                            {
                                tp->tp_Res1 = DOSFALSE;
                                if (tp->tp_Type == ACTION_FINDINPUT)
                                    tp->tp_Res2 = ERROR_OBJECT_NOT_FOUND;
                                else
                                    tp->tp_Res2 = ERROR_WRITE_PROTECTED;
                                break;
                            }

                        }
                    }
                }
                fh = (struct FileHandle *) AllocMem(sizeof(struct SFH),MEMF_CLEAR|MEMF_PUBLIC);  // must be clear
                if (!fh)
                {
		    tp->tp_Arg1 = NULL;		// probably not needed
                    tp->tp_Res1 = DOSFALSE;
                    tp->tp_Res2 = ERROR_NO_FREE_STORE;
                    break;
                }
                else
                {
		    // by default, sfh_Flags are 0 (allocated clear)
		    // perms are always 1 == allowed
		    if (!(perms & FIBF_WRITE))
			((struct SFH *) fh)->sfh_Flags = SFHF_NOWRITE;

		    tp->tp_Arg1 = MKBADDR(fh);
                    fh->fh_Pos = -1L;
                    fh->fh_End = -1L;
                    fh->fh_Type = ((struct FileLock *) BADDR(mi->mfsi_BaseLock))->fl_Task;
                    tp->tp_Res1 =
                        DoPkt((((struct FileLock *)BADDR(mi->mfsi_BaseLock))->fl_Task),tp->tp_Type,tp->tp_Arg1,tp->tp_Arg2,tp->tp_Arg3,tp->tp_Arg4,tp->tp_Arg5);
                    tp->tp_Res2 = IoErr();

                    /* If Full permissions are on, close the file, set the owner and default permissions,
			then reopen it. */
                    /* What a pain to have to work around everything.  This shouldn't be necessary. */
                    if (mi->mfsi_Flags & EVF_Full)
                    {
                        if ((tp->tp_Res1) && (tp->tp_Type == ACTION_FINDOUTPUT))
                        {
                            /* Close it */
                            tp->tp_Res1 =
                                DoPkt(((struct FileLock *)BADDR(mi->mfsi_BaseLock))->fl_Task,ACTION_END,
					fh->fh_Arg1,tp->tp_Arg2,tp->tp_Arg3,tp->tp_Arg4,tp->tp_Arg5);
                            tp->tp_Res2 = IoErr();
                            /* Set Owner */
                            tp->tp_Res1 =
                                DoPkt(((struct FileLock *)BADDR(mi->mfsi_BaseLock))->fl_Task,ACTION_SET_OWNER,0,
					tp->tp_Arg2,tp->tp_Arg3,((mi->mfsi_UID << 16)|(mi->mfsi_GID)),tp->tp_Arg5);
                            tp->tp_Res2 = IoErr();
                            /* Set default protection bits */
                            tp->tp_Res1 =
                                DoPkt(((struct FileLock *)BADDR(mi->mfsi_BaseLock))->fl_Task,ACTION_SET_PROTECT,0,
					tp->tp_Arg2,tp->tp_Arg3,DEFAULTPERMS,tp->tp_Arg5);
                            tp->tp_Res2 = IoErr();
                            /* Re-open the file */
                            tp->tp_Res1 =
                                DoPkt((((struct FileLock *)BADDR(mi->mfsi_BaseLock))->fl_Task),ACTION_FINDUPDATE,
					tp->tp_Arg1,tp->tp_Arg2,tp->tp_Arg3,tp->tp_Arg4,tp->tp_Arg5);
                            tp->tp_Res2 = IoErr();
                        }
                    }

                    if (!tp->tp_Res1)
                        FreeMem(fh,sizeof(struct SFH));
                    else
                    {
                        tp->tp_Res1 = (LONG) fh;
                        KeepFH(fh,mi);
                        TackPathFH(tp->tp_Arg2,fh,t);
                    }
                }
            }
            else
            {
                ULONG g;
                tp->tp_Res1 = 0L;
                g =  ((needed ^ perms) & perms);
                if (g & FIBF_WRITE)
                    tp->tp_Res2 = ERROR_WRITE_PROTECTED;
                if (g & FIBF_READ)
                    tp->tp_Res2 = ERROR_READ_PROTECTED;
                /* If both were needed, in the case of FINDUPDATE,
                 * and neither were supplied, it'll error with a READ_PROT.
                 */
            }
            break;
        }
        case ACTION_WRITE:
        {
            struct FileHandle *fh;

            fh = (struct FileHandle *) tp->tp_Arg1;
	    if (((struct SFH *) fh)->sfh_Flags & SFHF_NOWRITE)
	    {
		tp->tp_Res1 = -1;
		tp->tp_Res2 = ERROR_WRITE_PROTECTED;
		break;
	    }

            tp->tp_Arg2 += (LONG) rtp;			// NOTE! not tp!

            tp->tp_Res1 =
                DoPkt(((struct FileLock *)BADDR(mi->mfsi_BaseLock))->fl_Task,tp->tp_Type,fh->fh_Arg1,tp->tp_Arg2,tp->tp_Arg3,tp->tp_Arg4,tp->tp_Arg5);
            tp->tp_Res2 = IoErr();
            break;
        }
        case ACTION_READ:
        {
            struct FileHandle *fh;
            fh = (struct FileHandle *) tp->tp_Arg1;
            tp->tp_Arg2 = ((LONG) tp) + sizeof(struct TPacket);
            tp->tp_Res1 =
                DoPkt(((struct FileLock *)BADDR(mi->mfsi_BaseLock))->fl_Task,tp->tp_Type,fh->fh_Arg1,tp->tp_Arg2,tp->tp_Arg3,tp->tp_Arg4,tp->tp_Arg5);
            tp->tp_Res2 = IoErr();
            if (tp->tp_Res1 >= 0)	// so -1 (error) doesn't screw it up
                t->trans_RespDataActual = tp->tp_Res1 + sizeof(struct TPacket);
            break;
        }
        case ACTION_SEEK:
        {
            struct FileHandle *fh;
            fh = (struct FileHandle *) tp->tp_Arg1;
            tp->tp_Res1 =
                DoPkt(((struct FileLock *)BADDR(mi->mfsi_BaseLock))->fl_Task,tp->tp_Type,fh->fh_Arg1,tp->tp_Arg2,tp->tp_Arg3,tp->tp_Arg4,tp->tp_Arg5);
            tp->tp_Res2 = IoErr();
            break;
        }
        case ACTION_END:
        {
            struct FileHandle *fh;
            fh = (struct FileHandle *) tp->tp_Arg1;
            tp->tp_Res1 =
                DoPkt(((struct FileLock *)BADDR(mi->mfsi_BaseLock))->fl_Task,tp->tp_Type,fh->fh_Arg1,tp->tp_Arg2,tp->tp_Arg3,tp->tp_Arg4,tp->tp_Arg5);
            tp->tp_Res2 = IoErr();
            NukeFH(fh,mi);
            FreeMem(fh,sizeof(struct SFH));
            break;
        }

        case ACTION_FLUSH:
        {
            tp->tp_Res1 =
                DoPkt(((struct FileLock *)BADDR(mi->mfsi_BaseLock))->fl_Task,tp->tp_Type,tp->tp_Arg1,tp->tp_Arg2,tp->tp_Arg3,tp->tp_Arg4,tp->tp_Arg5);
            tp->tp_Res2 = IoErr();
            break;
        }


        case ACTION_FREE_LOCK:
        case ACTION_PARENT:
        case ACTION_COPY_DIR:
        {

            ULONG needed=DEFAULT_REQUIRED, perms=DEFAULT_REQUIRED;

            if (!tp->tp_Arg1)
            {
                if (tp->tp_Type == ACTION_FREE_LOCK)
                {
                    tp->tp_Res1 = 0L;
                    tp->tp_Res2 = 0L;
                    break;
                }
                tp->tp_Arg1 = (LONG) mi->mfsi_BaseLock;
            }

            if (tp->tp_Type == ACTION_PARENT)
            {
                if (SameLock((BPTR)tp->tp_Arg1,(BPTR)mi->mfsi_BaseLock) == LOCK_SAME)
                {
                    tp->tp_Res1 = 0L;
                    tp->tp_Res2 = 0L;
                    break;
                }
            }


            if (mi->mfsi_Flags & MFSIF_SECURITYON)
            {
                perms = BestPermFromLock((struct FileLock *) tp->tp_Arg1,mi);
                switch (tp->tp_Type)
                {
                    case ACTION_FREE_LOCK:
                        perms = DEFAULT_REQUIRED;
                        break;
                    case ACTION_PARENT:
                    case ACTION_COPY_DIR:
                        needed = FIBF_EXECUTE;
                        break;
                }
            }

            if (needed & perms)
            {
                if (tp->tp_Type == ACTION_FREE_LOCK)
                    NukeLock((APTR)tp->tp_Arg1,mi);
                tp->tp_Res1 =
                    DoPkt((((struct FileLock *)BADDR(mi->mfsi_BaseLock))->fl_Task),tp->tp_Type,tp->tp_Arg1,tp->tp_Arg2,tp->tp_Arg3,tp->tp_Arg4,tp->tp_Arg5);
                tp->tp_Res2 = IoErr();
                if ((tp->tp_Type != ACTION_FREE_LOCK) && (tp->tp_Res1))
                {
                    KeepLock((APTR)tp->tp_Res1,mi);
                    TackPathLock((APTR)tp->tp_Res1,t);
                }
            }
            else
            {
                tp->tp_Res1 = 0L;
                tp->tp_Res2 = ERROR_EXECUTE_PROTECTED;
            }

            break;
        }
        case ACTION_RENAME_OBJECT:
        {
            ULONG needed = DEFAULT_REQUIRED, perms = DEFAULT_REQUIRED;

            tp->tp_Arg2 += (LONG) rtp;
            tp->tp_Arg2 = MKBADDR(tp->tp_Arg2);
            tp->tp_Arg4 += (LONG) rtp;
            tp->tp_Arg4 = MKBADDR(tp->tp_Arg4);

            if (!tp->tp_Arg1)
                tp->tp_Arg1 = (LONG) mi->mfsi_BaseLock;
            if (!tp->tp_Arg3)
                tp->tp_Arg3 = (LONG) mi->mfsi_BaseLock;

            if (mi->mfsi_Flags & MFSIF_SECURITYON)
            {
                BPTR tlock;
                /* Make sure we can get to the original object */
                tlock = GetALock(tp->tp_Arg2,tp->tp_Arg1,mi,ACCESS_READ);
                if (!tlock)
                {
                    tp->tp_Res1 = 0L;
                    tp->tp_Res2 = ERROR_OBJECT_NOT_FOUND;
                    break;
                }
                UnLock(tlock);

                /* Find out if the destination dir allows writes */
                perms = BestPermFromLock((struct FileLock *)tp->tp_Arg3,mi);
                needed = FIBF_WRITE;
            }

            if (needed & perms)
            {
                tp->tp_Res1 =
                    DoPkt((((struct FileLock *)BADDR(mi->mfsi_BaseLock))->fl_Task),tp->tp_Type,tp->tp_Arg1,tp->tp_Arg2,tp->tp_Arg3,tp->tp_Arg4,tp->tp_Arg5);
                tp->tp_Res2 = IoErr();
            }
            else
            {
                tp->tp_Res1 = 0L;
                tp->tp_Res2 = ERROR_WRITE_PROTECTED;
            }
            break;

        }
        case ACTION_CREATE_DIR:
        case ACTION_DELETE_OBJECT:
        case ACTION_LOCATE_OBJECT:
        {
            ULONG needed = DEFAULT_REQUIRED, perms = DEFAULT_REQUIRED;
            BPTR locktocheck;
            BOOL gotalock = FALSE;

            UBYTE xr[128], *y;

            tp->tp_Arg2 += (LONG) rtp;
            y = (UBYTE *) tp->tp_Arg2;
            CopyMem(&y[1],xr,y[0]);
            xr[y[0]]=0;

            tp->tp_Arg2 = MKBADDR(tp->tp_Arg2);
            if (!tp->tp_Arg1)
            {
                tp->tp_Arg1 = (LONG) mi->mfsi_BaseLock;
            }
            else
            {
                if (!(tp->tp_Arg1+1))
                {
                    tp->tp_Arg1 = 0L;
                }
            }

            locktocheck = (BPTR) tp->tp_Arg1;
            if (mi->mfsi_Flags & MFSIF_SECURITYON)
            {

                if (tp->tp_Type == ACTION_DELETE_OBJECT)
                {
                    /* If delete_object, we need to be sure that the object to be deleted actually has delete permissions -- not the directory in which it resides */
                    locktocheck =
                        GetALock(tp->tp_Arg2,tp->tp_Arg1,mi,ACCESS_READ);
                    if (!locktocheck)
                    {
                        tp->tp_Res1 = 0L;
                        tp->tp_Res2 = ERROR_OBJECT_NOT_FOUND;
                        break;
                    }
                    gotalock = TRUE;
                }
                perms = BestPermFromLock((struct FileLock *)locktocheck,mi);

                if (gotalock)
                    DoPkt((((struct FileLock *)BADDR(mi->mfsi_BaseLock))->fl_Task),ACTION_FREE_LOCK,locktocheck,tp->tp_Arg2,tp->tp_Arg3,tp->tp_Arg4,tp->tp_Arg5);

                switch (tp->tp_Type)
                {
                    case ACTION_CREATE_DIR:
                        needed = FIBF_WRITE;
                        break;
                    case ACTION_DELETE_OBJECT:
                        needed = FIBF_DELETE;
                        break;
//                    case ACTION_LOCATE_OBJECT:
//                        needed = FIBF_EXECUTE;
//                        break;
                }
            }

            if (needed & perms)
            {
                if (tp->tp_Type == ACTION_LOCATE_OBJECT)
                {
                    char fname[128];
                    char *p = (char *) BADDR(tp->tp_Arg2);
                    if (p[0] < 128)
                    {
                        CopyMem(&p[1],&fname[0],p[0]);
                        fname[p[0]] = 0;
                    }

                    if ((tp->tp_Arg4 & MFSIF_NOBACKDROP) || (mi->mfsi_Flags & MFSIF_NOBACKDROP))
                    {
                        if (!Strnicmp(&fname[0],".backdrop",128))
                        {
                            tp->tp_Res1 = DOSFALSE;
                            tp->tp_Res2 = ERROR_OBJECT_NOT_FOUND;
                            break;
                        }
                    }
                    if (SameLock((BPTR)tp->tp_Arg1,(BPTR)mi->mfsi_BaseLock) == LOCK_SAME)
                    {
                        if ((tp->tp_Arg4 & MFSIF_NODISKINFO) || (mi->mfsi_Flags & MFSIF_NODISKINFO))
                        {
                            if (!Strnicmp(&fname[0],"Disk.info",128))
                            {
                                tp->tp_Res1 = DOSFALSE;
                                tp->tp_Res2 = ERROR_OBJECT_NOT_FOUND;
                                break;
                            }
                        }
                    }
                }

                if (tp->tp_Type != ACTION_LOCATE_OBJECT)
                {
                    tp->tp_Res1 =
                        DoPkt(((struct FileLock *)BADDR(mi->mfsi_BaseLock))->fl_Task,tp->tp_Type,tp->tp_Arg1,tp->tp_Arg2,tp->tp_Arg3,tp->tp_Arg4,tp->tp_Arg5);
                    tp->tp_Res2 = IoErr();
                    if ((tp->tp_Type == ACTION_CREATE_DIR) && (tp->tp_Res1))
                    {
                        STRPTR empty;

                        ChangeMode(CHANGE_LOCK,tp->tp_Res1,ACTION_READ);        /* Get rid of the exclusive lock stuff */
                        empty = AllocMem(1,MEMF_CLEAR);
                        if (empty)
                        {
                            /* Set Owner */
                            DoPkt(((struct FileLock *)BADDR(mi->mfsi_BaseLock))->fl_Task,ACTION_SET_OWNER,0,tp->tp_Res1,MKBADDR(empty),((mi->mfsi_UID << 16)|(mi->mfsi_GID)),tp->tp_Arg5);
                            /* Set default protection bits */
                            DoPkt(((struct FileLock *)BADDR(mi->mfsi_BaseLock))->fl_Task,ACTION_SET_PROTECT,0,tp->tp_Res1,MKBADDR(empty),DEFAULTPERMS,tp->tp_Arg5);
                            FreeMem(empty,1);
                        }
                    }
                }
                else
                {
                    /* Kludge alert */
                    tp->tp_Arg3 = ACCESS_READ;
                    /* Kludge ends */
                    tp->tp_Res1 = GetALock((BSTR) tp->tp_Arg2, tp->tp_Arg1,mi,tp->tp_Arg3);
                    tp->tp_Res2 = IoErr();
                }

                if ((tp->tp_Res1) && (tp->tp_Type != ACTION_DELETE_OBJECT))
                {
                    KeepLock((APTR)tp->tp_Res1,mi);
                    TackPathLock((APTR)tp->tp_Res1,t);
                }
            }
            else
            {
                tp->tp_Res1 = 0L;
                switch (tp->tp_Type)
                {
                    case ACTION_LOCATE_OBJECT:
                        tp->tp_Res2 = ERROR_EXECUTE_PROTECTED;
                        break;
                    case ACTION_DELETE_OBJECT:
                        tp->tp_Res2 = ERROR_DELETE_PROTECTED;
                        break;
                    case ACTION_CREATE_DIR:
                        tp->tp_Res2 = ERROR_WRITE_PROTECTED;
                        break;
                }
            }
            break;
        }
        case ACTION_SET_OWNER:
        case ACTION_SET_PROTECT:
        {
            ULONG perms = DEFAULT_REQUIRED, needed = DEFAULT_REQUIRED;

            tp->tp_Arg3 += (LONG) rtp;
            tp->tp_Arg3 = MKBADDR(tp->tp_Arg3);

            if (!tp->tp_Arg2)
                tp->tp_Arg2 = (LONG) mi->mfsi_BaseLock;

            if (mi->mfsi_Flags & MFSIF_SECURITYON)
            {
                /* Get a lock on the darned thing, make sure we own it, then unlock */
                /* get the lock */
                tp->tp_Res1 =
                    GetALock(tp->tp_Arg3,tp->tp_Arg2,mi,ACCESS_READ);
                tp->tp_Res2 = IoErr();

                if (!tp->tp_Res1)
                    break;

                /* see if we're the owner, or admin */
                if (mi->mfsi_UserFlags & UFLAGF_AdminAll)   /* If admin, they've got it */
                    perms = DEFAULT_REQUIRED;
                else
                {
                    int u;
                    u = UIDFromLock((struct FileLock *) tp->tp_Res1);
                    if (!u)
                        perms = DEFAULT_REQUIRED;
                    else
                        perms = (ULONG) (mi->mfsi_UID == u);

//                    perms = (ULONG) (mi->mfsi_UID == UIDFromLock((struct FileLock *) tp->tp_Res1));
                }
                needed = 1L;    /* The 1 has no significance ... It should be 'TRUE'. */

                /* unlock */
                tp->tp_Res1 =
                    DoPkt((((struct FileLock *)BADDR(mi->mfsi_BaseLock))->fl_Task),ACTION_FREE_LOCK,tp->tp_Res1,tp->tp_Arg2,tp->tp_Arg3,tp->tp_Arg4,tp->tp_Arg5);
                tp->tp_Res2 = IoErr();

            }

            if (needed & perms)
            {
                tp->tp_Res1 =
                    DoPkt((((struct FileLock *)BADDR(mi->mfsi_BaseLock))->fl_Task),tp->tp_Type,tp->tp_Arg1,tp->tp_Arg2,tp->tp_Arg3,tp->tp_Arg4,tp->tp_Arg5);
                tp->tp_Res2 = IoErr();
            }
            else
            {
                tp->tp_Res1 = 0L;
                tp->tp_Res2 = ERROR_WRITE_PROTECTED;
            }
            break;
        }
        case ACTION_SET_COMMENT:
        {
            ULONG perms = DEFAULT_REQUIRED, needed = DEFAULT_REQUIRED;
            tp->tp_Arg3 += (LONG) rtp;
            tp->tp_Arg3 = MKBADDR(tp->tp_Arg3);
            tp->tp_Arg4 += (LONG) rtp;
            tp->tp_Arg4 = MKBADDR(tp->tp_Arg4);
            if (!tp->tp_Arg2)
                tp->tp_Arg2 = (LONG) mi->mfsi_BaseLock;
            if (mi->mfsi_Flags & MFSIF_SECURITYON)
            {
                BPTR anylock;
                anylock = GetALock(tp->tp_Arg3,tp->tp_Arg2,mi,ACCESS_READ);
                if (!anylock)
                {
                    tp->tp_Res1 = 0L;
                    tp->tp_Res2 = ERROR_OBJECT_NOT_FOUND;
                    break;
                }
                UnLock(anylock);

                perms = BestPermFromLock((struct FileLock *) tp->tp_Arg2,mi);
                needed = FIBF_WRITE;
            }
            if (needed & perms)
            {
                tp->tp_Res1 =
                    DoPkt((((struct FileLock *)BADDR(mi->mfsi_BaseLock))->fl_Task),tp->tp_Type,tp->tp_Arg1,tp->tp_Arg2,tp->tp_Arg3,tp->tp_Arg4,tp->tp_Arg5);
                tp->tp_Res2 = IoErr();
            }
            else
            {
                tp->tp_Res1 = 0L;
                tp->tp_Res2 = ERROR_WRITE_PROTECTED;
            }
            break;
        }

        case ACTION_SET_DATE:
        {
            ULONG needed = DEFAULT_REQUIRED, perms=DEFAULT_REQUIRED;
            tp->tp_Arg3 = MKBADDR((ULONG) tp->tp_Arg3 + (ULONG) rtp); /* Add offset to tp, make into a bptr */
            if (!tp->tp_Arg2)
                tp->tp_Arg2 = (LONG) mi->mfsi_BaseLock;
            tp->tp_Arg4 += (ULONG) rtp;

            if (mi->mfsi_Flags & MFSIF_SECURITYON)
            {
                perms = BestPermFromLock((struct FileLock *)tp->tp_Arg2,mi);
                needed = FIBF_WRITE;
            }
            if (needed & perms)
            {
                tp->tp_Res1 =
                    DoPkt((((struct FileLock *)BADDR(mi->mfsi_BaseLock))->fl_Task),tp->tp_Type,tp->tp_Arg1,tp->tp_Arg2,tp->tp_Arg3,tp->tp_Arg4,tp->tp_Arg5);
                tp->tp_Res2 = IoErr();
                break;
            }
            else
            {
                tp->tp_Arg1 = 0L;
                tp->tp_Arg2 = ERROR_WRITE_PROTECTED;
            }

        }

        case ACTION_SAME_LOCK:
        {
            if (!tp->tp_Arg1)
                tp->tp_Arg1 = (LONG) mi->mfsi_BaseLock;

            if (!tp->tp_Arg2)
                tp->tp_Arg2 = (LONG) mi->mfsi_BaseLock;

            tp->tp_Res1 =
                DoPkt((((struct FileLock *)BADDR(mi->mfsi_BaseLock))->fl_Task),tp->tp_Type,tp->tp_Arg1,tp->tp_Arg2,tp->tp_Arg3,tp->tp_Arg4,tp->tp_Arg5);
            tp->tp_Res2 = IoErr();

            break;
        }

        case ACTION_INFO:
        {
            if (!tp->tp_Arg1)
                tp->tp_Arg1 = (LONG) mi->mfsi_BaseLock;

            tp->tp_Arg2 = MKBADDR((ULONG) tp->tp_Arg2 + (ULONG) tp);
            tp->tp_Res1 =
                DoPkt((((struct FileLock *)BADDR(mi->mfsi_BaseLock))->fl_Task),tp->tp_Type,tp->tp_Arg1,tp->tp_Arg2,tp->tp_Arg3,tp->tp_Arg4,tp->tp_Arg5);
            tp->tp_Res2 = IoErr();
            t->trans_RespDataActual = (sizeof(struct TPacket)+sizeof(struct InfoData));
            break;

        }

        case ACTION_DISK_INFO:
        {
            tp->tp_Arg1 = MKBADDR((ULONG) tp->tp_Arg1 + (ULONG) tp);
            tp->tp_Res1 =
                DoPkt(((struct FileLock *)BADDR(mi->mfsi_BaseLock))->fl_Task,tp->tp_Type,tp->tp_Arg1,tp->tp_Arg2,tp->tp_Arg3,tp->tp_Arg4,tp->tp_Arg5);
            tp->tp_Res2 = IoErr();
            t->trans_RespDataActual = (sizeof(struct TPacket)+sizeof(struct InfoData));
            break;

        }

        case ACTION_EXAMINE_FH:
        case ACTION_EXAMINE_NEXT:
        case ACTION_EXAMINE_OBJECT:
        {
            BOOL rootdir=FALSE;
            BOOL loop=TRUE;
            ULONG perms=DEFAULT_REQUIRED, needed=DEFAULT_REQUIRED;

            if (tp->tp_Type != ACTION_EXAMINE_FH)
	    {
                if (!tp->tp_Arg1)
                    tp->tp_Arg1 = (LONG) mi->mfsi_BaseLock;
		else
		    RemExall(tp->tp_Arg1,FALSE);
		// He might have just stopped ExNexting, and then examined it again.  Clean up.

	    } else {

		// ExamineFH must pass the filesystem fh_Arg1, NOT the filehandle itself!
		tp->tp_Arg1 = ((struct FileHandle *) tp->tp_Arg1)->fh_Arg1;
	    }

            if (mi->mfsi_Flags & MFSIF_SECURITYON)
            {
                if (tp->tp_Type == ACTION_EXAMINE_NEXT)
                {
                    perms = BestPermFromLock((struct FileLock *)tp->tp_Arg1,mi);  /* On dir that the file is in */
                    needed = FIBF_READ;  /* If a dir has no Read bit set, nobody can see into it */
                }
            }

            if (needed & perms)
            {
                if (tp->tp_Type == ACTION_EXAMINE_OBJECT)
                    if (SameLock((BPTR)tp->tp_Arg1,(BPTR)mi->mfsi_BaseLock) == LOCK_SAME)
                        rootdir = TRUE;
                tp->tp_Arg2 = MKBADDR((ULONG) tp->tp_Arg2 + (ULONG) tp); /* Add offset to tp, make into a bptr */

                while(loop)
                {
                    loop = FALSE;
                    CopyMem(BADDR(tp->tp_Arg2), mi->mfsi_FIB, sizeof(struct FileInfoBlock));

                    tp->tp_Res1 =
                        DoPkt((((struct FileLock *)BADDR(mi->mfsi_BaseLock))->fl_Task),tp->tp_Type,tp->tp_Arg1,
				MKBADDR(mi->mfsi_FIB),tp->tp_Arg3,tp->tp_Arg4,tp->tp_Arg5);
                    tp->tp_Res2 = IoErr();

                    if (mi->mfsi_Flags & MFSIF_SECURITYON)
                    {
                        ULONG px, flags=0;
                        struct FileInfoBlock *f=(struct FileInfoBlock *) mi->mfsi_FIB;
                        px = BestPermX(f,mi,&flags);
                        if ( (flags & BPF_GROUP) || (flags & BPF_OTHER) )           /* Promote into user field */
                            f->fib_Protection = (f->fib_Protection & (~OWNERBITS)) | ((~px) & OWNERBITS);
                        if (flags & BPF_OTHER)                                      /* Promote into group field */
                            f->fib_Protection = (f->fib_Protection &(~OTHERBITS)) | (((~px) & OWNERBITS) << 8);
                    }
                    CopyMem(mi->mfsi_FIB, BADDR(tp->tp_Arg2), sizeof(struct FileInfoBlock));
                    if ((tp->tp_Res1) && (rootdir))
                    {
                        struct FileInfoBlock *myfib=(struct FileInfoBlock *) BADDR(tp->tp_Arg2);
                        myfib->fib_DirEntryType = ST_ROOT;
                        strncpy((char *)&myfib->fib_FileName[1],(char *)mi->mfsi_VolumeName,106);
                        myfib->fib_FileName[0]=strlen(&myfib->fib_FileName[1]);
                    }
                    if (tp->tp_Res1)
                    {
                        /* Convert from a BSTR to a CSTR */
                        struct FileInfoBlock *myfib=(struct FileInfoBlock *) BADDR(tp->tp_Arg2);
                        char fname[128];
                        char *p= (char *) &myfib->fib_FileName[0];
                        if (p[0] < 128)
                        {
                            CopyMem(&p[1],&fname[0],p[0]);
                            fname[p[0]] = 0;
                        }

                        if ((tp->tp_Arg4 & MFSIF_NOBACKDROP) || (mi->mfsi_Flags & MFSIF_NOBACKDROP))
                        {
                            if ((!Strnicmp(&fname[0],".backdrop",107)) && (tp->tp_Type == ACTION_EXAMINE_NEXT))
                                loop = TRUE;
                        }
                        if ((tp->tp_Arg4 & MFSIF_NODISKINFO) || (mi->mfsi_Flags & MFSIF_NODISKINFO))
                        {
                            if ((tp->tp_Type == ACTION_EXAMINE_NEXT) &&
				(SameLock((BPTR)tp->tp_Arg1,(BPTR)mi->mfsi_BaseLock) == LOCK_SAME) &&
                                (!Strnicmp(&fname[0],"Disk.info",107)))
			    {
                                    loop = TRUE;
			    }
                        }
                    }
                }
//                if (tp->tp_Type == ACTION_EXAMINE_FH)
//                {
//                    struct FileInfoBlock *mf=BADDR(tp->tp_Arg2);
//                    kprintf("name is '%s'\n",mf->fib_FileName);
//                }

                t->trans_RespDataActual = ((ULONG)(BADDR(tp->tp_Arg2))+sizeof(struct FileInfoBlock))-(ULONG)tp;
                /* ^^^ Only send back what we need! */
            }
            else
            {
                tp->tp_Res1 = 0L;
                tp->tp_Res2 = ERROR_READ_PROTECTED;
            }
            break;
        }

	case ACTION_EXAMINE_ALL_END:
	    // RemExall will clean anything up that needs cleaning up
	    RemExall(tp->tp_Arg1,FALSE);	// exall still under way
	    tp->tp_Res1 = DOSTRUE;
	    tp->tp_Res2 = 0;
	    break;

	case ACTION_EXAMINE_ALL:
	    // all pointers are offsets
            if (tp->tp_Arg6)
		tp->tp_Arg6 += ((ULONG) rtp);		// must make a real pointer here

	     // fall through
	     // the same, since we have no knowlege of the asynchronicity of ENVOY_EXAMINE_ALL

        case ACTION_ENVOY_EXAMINE_ALL:
        {
            ULONG perms=DEFAULT_REQUIRED, needed=DEFAULT_REQUIRED;
	    struct EAC_Node *enode;
	    struct ExAllControl *ec;

	    // client will never send us a NULL lock, those use regular exnext
	    // however, it might just happen with ExAll()
	    if (!tp->tp_Arg1)
	    {
		tp->tp_Res1 = FALSE;
		tp->tp_Res2 = ERROR_INVALID_LOCK;
		break;
	    }

            if (mi->mfsi_Flags & MFSIF_SECURITYON)
            {
        	perms = BestPermFromLock((struct FileLock *)tp->tp_Arg1,mi);  /* On dir that the file is in */
		needed = FIBF_READ;  /* If a dir has no Read bit set, nobody can see into it */
            }

            if (needed & perms)
            {
		// NOTE! arg2 is APTR!
                tp->tp_Arg2 = (ULONG) tp->tp_Arg2 + (ULONG) tp; /* Add offset to tp */

		// we keep an LRU list of active exalls, this finds the right one or
		// creates a new ExAllControl struct.  Note that this means that
		// broken connections, if in the middle of an exall, may restart at
		// the beginning.  Oh well, no biggie.

		enode = FindExall(tp->tp_Arg1, tp->tp_Arg5, mi);
		if (!enode)
		{
			tp->tp_Res1 = FALSE;
			tp->tp_Res2 = IoErr();	// no memory
			break;
		}
		ec = enode->en_EAC;

		// if real examine all, we already made this a real pointer (into reqdata)
		if (tp->tp_Type == ACTION_EXAMINE_ALL)
		{
			if (tp->tp_Arg6)
				ec->eac_MatchString = (UBYTE *) tp->tp_Arg6;

			// if V37 ROM fs, protect them from danger!  V37 ROM fs doesn't handle
			// anything over ED_COMMENT correctly (it doesn't set up the ED_COMMENT
			// field if the type is > ED_COMMENT).
			if ((mi->mfsi_Flags & MFSIF_V37_FS) && (tp->tp_Arg4 > ED_COMMENT))
			{
				tp->tp_Res1 = FALSE;
				tp->tp_Res2 = ERROR_BAD_NUMBER;
			}
		} else {
			// if we've already dropped the ED_xxx level, retain it!
			// if V37 ROM fs, always drop it!
			if (enode->en_Type)
				tp->tp_Arg4 = enode->en_Type;
			else if (mi->mfsi_Flags & MFSIF_V37_FS) {
				tp->tp_Arg4 = enode->en_Type = ED_COMMENT;
			}
		}

		// don't use packets, let dos emulate exall...
exall_loop:	tp->tp_Res1 = ExAll(tp->tp_Arg1,(struct ExAllData *) tp->tp_Arg2,tp->tp_Arg3,
				    tp->tp_Arg4,ec);
		tp->tp_Res2 = IoErr();

//kprintf("exall returned %ld (%ld)\n",tp->tp_Res1,tp->tp_Res2);
		// V37 filesystems might reject ED_OWNER with bad number (mainly RAM:)
		if (!tp->tp_Res1 && tp->tp_Res2 == ERROR_BAD_NUMBER &&
		    tp->tp_Type == ACTION_ENVOY_EXAMINE_ALL &&
		    tp->tp_Arg4 > ED_COMMENT)
		{
			// we MUST remember this, or continuations will fail
			// don't do this if the user submitted the request - his problem
			tp->tp_Arg4 = ED_COMMENT;
			enode->en_Type = ED_COMMENT;

			// not really, but keeps us from sending packets we know will be rejected in the future
			mi->mfsi_Flags |= MFSIF_V37_FS;
			// don't set MFSIF_COMMENT_BSTR!

			// re-init control struct
			ec->eac_LastKey = 0;
			ec->eac_Entries = 0;	// paranoia
			goto exall_loop;
		}

		if (tp->tp_Res1 || tp->tp_Res2 == ERROR_NO_MORE_ENTRIES)
                {
                        ULONG px, i, flags=0;
                        struct ExAllData *ed = (struct ExAllData *) tp->tp_Arg2;

			// scan the entries and make any modifications we need to
//kprintf("returning %ld entries for eac $%lx\n",ec->eac_Entries,ec);
			tp->tp_Arg5 = ec->eac_Entries;		// ugly!! should return a struct...

			for (i = 0; i < ec->eac_Entries; i++)
			{
//kprintf("$%lx:\t%s\n",ed,ed->ed_Name);
			    if (mi->mfsi_Flags & MFSIF_SECURITYON && tp->tp_Arg4 >= ED_PROTECTION)
			    {
                              px = BestPermED(ed,tp->tp_Arg4,mi,&flags);
                              if ( (flags & BPF_GROUP) || (flags & BPF_OTHER)) /* Promote into user field */
                                ed->ed_Prot = (ed->ed_Prot & (~OWNERBITS)) | ((~px) & OWNERBITS);
                              if (flags & BPF_OTHER)			       /* Promote into group field */
                                ed->ed_Prot = (ed->ed_Prot &(~OTHERBITS)) | (((~px) & OWNERBITS) << 8);
			    }

			    // .backdrop and Disk.info are filtered out by eac_MatchString...

			    // make the name/comment/next pointers be offsets from the buffer
			    ed->ed_Name = (void *) (((ULONG) ed->ed_Name) - tp->tp_Arg2);
			    if (ed->ed_Next)
				ed->ed_Next = (void *) (((ULONG) ed->ed_Next) - tp->tp_Arg2);
			    if (tp->tp_Arg4 >= ED_COMMENT && ed->ed_Comment)
			    {
				// the V37 fs screwed up and returns comments as BSTRs!
				if (mi->mfsi_Flags & MFSIF_COMMENT_BSTR)
					BtoCstr(ed->ed_Comment);
				ed->ed_Comment = (void *) (((ULONG) ed->ed_Comment) - tp->tp_Arg2);
			    }
			    ed = (void *) (((ULONG) ed->ed_Next) + tp->tp_Arg2);
			}
                }
		// are we done with the exallcontrol struct?
		if (!tp->tp_Res1)
			RemExall(tp->tp_Arg1,TRUE);

		// send it all back.  We could find the end and limit it to that, maybe.
                t->trans_RespDataActual = sizeof(struct TPacket) + tp->tp_Arg3;
                /* ^^^ Only send back what we need! */
            }
            else
            {
                tp->tp_Res1 = 0L;
                tp->tp_Res2 = ERROR_READ_PROTECTED;
            }
            break;
        }
        case ACTION_USERNAME_TO_UID:
        {
            struct UserInfo *u;
            tp->tp_Arg1 += (ULONG) rtp;
            u = AllocUserInfo();
            tp->tp_Res1 = DOSFALSE;
            if (u)
            {
                if (!NameToUser((STRPTR)tp->tp_Arg1, u))
                    tp->tp_Res1 = u->ui_UserID;
                FreeUserInfo(u);
            }
            else
                tp->tp_Res2 = ERROR_NO_FREE_STORE;
            break;
        }
        case ACTION_GROUPNAME_TO_GID:
        {
            struct GroupInfo *g;
            tp->tp_Arg1 += (ULONG) rtp;
            g = (struct GroupInfo *) AllocGroupInfo();
            tp->tp_Res1 = DOSFALSE;
            if (g)
            {
                if (!NameToGroup((STRPTR)tp->tp_Arg1, g))
                    tp->tp_Res1 = g->gi_GroupID;
                FreeGroupInfo(g);
            }
            else
            {
                tp->tp_Res2 = ERROR_NO_FREE_STORE;
            }
            break;
        }
        case ACTION_UID_TO_USERINFO:
        {
            struct UserInfo *u, *aui;
            ULONG err;
            u = (struct UserInfo *) &((UBYTE *)tp)[tp->tp_Arg2];
            aui = AllocUserInfo();
            if (aui)
            {
                err = IDToUser(tp->tp_Arg1,aui);
                tp->tp_Res1 = DOSTRUE;
                if (!err)
                    CopyMem(aui,u,sizeof(struct UserInfo));
                else
                    tp->tp_Res1 = DOSFALSE;
                tp->tp_Res2 = 0;
                t->trans_RespDataActual = (tp->tp_Arg2 + sizeof(struct UserInfo));
                FreeUserInfo(aui);
            }
            else
            {
                tp->tp_Res1 = DOSFALSE;
                tp->tp_Res2 = ERROR_NO_FREE_STORE;
            }
            break;
        }
        case ACTION_GID_TO_GROUPINFO:
        {
            struct GroupInfo *g, *agi;
            ULONG err;
            g = (struct GroupInfo *) &((UBYTE *)tp)[tp->tp_Arg2];
            agi = (struct GroupInfo *) AllocGroupInfo();
            if (agi)
            {
                err = IDToGroup(tp->tp_Arg1,agi);
                tp->tp_Res1 = DOSTRUE;
                if (!err)
                    CopyMem(agi,g,sizeof(struct GroupInfo));
                else
                    tp->tp_Res1 = DOSFALSE;
                tp->tp_Res2 = 0;
                t->trans_RespDataActual = (tp->tp_Arg2 + sizeof(struct GroupInfo));
                FreeGroupInfo(agi);
            }
            else
            {
                tp->tp_Res1 = DOSFALSE;
                tp->tp_Res2 = ERROR_NO_FREE_STORE;
            }
            break;
        }
        case ACTION_SET_FILE_SIZE:
        {
            struct FileHandle *fh;

            fh = (struct FileHandle *) tp->tp_Arg1;
	    if (((struct SFH *) fh)->sfh_Flags & SFHF_NOWRITE)
	    {
		tp->tp_Res1 = -1;
		tp->tp_Res2 = ERROR_WRITE_PROTECTED;
		break;
	    }

            tp->tp_Res1 =
                DoPkt(((struct FileLock *)BADDR(mi->mfsi_BaseLock))->fl_Task,tp->tp_Type,fh->fh_Arg1,tp->tp_Arg2,tp->tp_Arg3,tp->tp_Arg4,tp->tp_Arg5);
            tp->tp_Res2 = IoErr();

            break;
        }
        case ACTION_CHANGE_MODE:
        {
            tp->tp_Res1 =
                DoPkt(((struct FileLock *)BADDR(mi->mfsi_BaseLock))->fl_Task,tp->tp_Type,tp->tp_Arg1,tp->tp_Arg2,tp->tp_Arg3,tp->tp_Arg4,tp->tp_Arg5);
            tp->tp_Res2 = IoErr();
            break;
        }
        case ACTION_FH_FROM_LOCK:
        {
            struct FileHandle *fh;
            ULONG perms=DEFAULT_REQUIRED, needed=DEFAULT_REQUIRED;

            if (mi->mfsi_Flags & MFSIF_SECURITYON)
            {
                    BPTR templock = tp->tp_Arg2;

                    perms = BestPermFromLock((struct FileLock *)templock,mi);
                    if (!(perms & FIBF_READ))
                    {
                        tp->tp_Res1 = DOSFALSE;
                        tp->tp_Res2 = ERROR_READ_PROTECTED;
                        break;
                    }
	    }

	    // can only fail if it's read protected I think
            if (perms & needed)
            {
              fh = AllocMem(sizeof(struct SFH),MEMF_PUBLIC|MEMF_CLEAR);
              if (!fh)
              {
                tp->tp_Res1 = DOSFALSE;
                tp->tp_Res2 = ERROR_NO_FREE_STORE;
                break;
              }
              else
              {
                UBYTE *p;

		if (!(perms & FIBF_WRITE))
			((struct SFH *) fh)->sfh_Flags = SFHF_NOWRITE;

                fh->fh_Pos = -1L;
                fh->fh_End = -1L;
                fh->fh_Type = ((struct FileLock *) BADDR(mi->mfsi_BaseLock))->fl_Task;

                /* Get the full path before the lock gets stolen . . . */
                p = &(((UBYTE *)t->trans_ResponseData)[sizeof(struct TPacket)]);
                if (GNameFromLock(tp->tp_Arg2,p,t->trans_RespDataLength-sizeof(struct TPacket)))
                    t->trans_RespDataActual = sizeof(struct TPacket) + strlen(p) + 1;

                tp->tp_Res1 =
                    DoPkt(((struct FileLock *)BADDR(mi->mfsi_BaseLock))->fl_Task,tp->tp_Type,tp->tp_Arg1,tp->tp_Arg2,tp->tp_Arg3,tp->tp_Arg4,tp->tp_Arg5);
                tp->tp_Res2 = IoErr();
              }
              if (!tp->tp_Res1)
                FreeMem(fh,sizeof(struct SFH));
              else
              {
                NukeLock((APTR)tp->tp_Arg2,mi);
                tp->tp_Res1 = (LONG) fh;
                KeepFH(fh,mi);
              }

	    } else {
		// not enough perms to open it
                ULONG g;

                tp->tp_Res1 = FALSE;
                g =  ((needed ^ perms) & perms);
                if (g & FIBF_WRITE)
                    tp->tp_Res2 = ERROR_WRITE_PROTECTED;
                if (g & FIBF_READ)
                    tp->tp_Res2 = ERROR_READ_PROTECTED;
	    }
            break;
        }
        case ACTION_PARENT_FH:
        case ACTION_COPY_DIR_FH:
        {
            struct FileHandle *fh;
            fh = (struct FileHandle *) tp->tp_Arg1;

            tp->tp_Res1 =
                DoPkt(((struct FileLock *)BADDR(mi->mfsi_BaseLock))->fl_Task,tp->tp_Type,fh->fh_Arg1,tp->tp_Arg2,tp->tp_Arg3,tp->tp_Arg4,tp->tp_Arg5);
            tp->tp_Res2 = IoErr();

            if (tp->tp_Res1)
            {
                KeepLock((APTR)tp->tp_Res1,mi);
                TackPathLock((APTR)tp->tp_Res1,t);
            }
            break;
        }

        case ACTION_READ_LINK:
        {
            tp->tp_Arg2 = ((ULONG) tp + (ULONG) tp->tp_Arg2);
            tp->tp_Arg3 = ((ULONG) tp + (ULONG) tp->tp_Arg3);
            tp->tp_Res1 =
                DoPkt((((struct FileLock *)BADDR(mi->mfsi_BaseLock))->fl_Task),tp->tp_Type,tp->tp_Arg1,tp->tp_Arg2,tp->tp_Arg3,tp->tp_Arg4,tp->tp_Arg5);
            tp->tp_Res2 = IoErr();
            break;
        }

	default:
	    tp->tp_Res1 = FALSE;
	    tp->tp_Res2 = ERROR_ACTION_NOT_KNOWN;
    }

//    kprintf("     Res1 $%lx Res2 $%lx\n",tp->tp_Res1,tp->tp_Res2);
}


BOOL GNameFromLock(ULONG lock, char * buffer, ULONG len)
{

    return((BOOL) NameFromLock(lock,buffer,len));

//    return(TRUE);
}



void TackPathLock(struct FileLock *f,struct Transaction *t)
{
    UBYTE *p;
    p = &(((UBYTE *)t->trans_ResponseData)[sizeof(struct TPacket)]);

    if (GNameFromLock((ULONG)f,p,t->trans_RespDataLength-sizeof(struct TPacket)))
    {
        t->trans_RespDataActual = sizeof(struct TPacket) + strlen(p) + 1;
    }

}


/*
 * TackPathFH - finds the complete path to a file, and stores that in
 * the free data area at the end of a TPacket.
 *
 * I can't simply use NameFromFH, though.  It's possible to
 * CreateDir(), and Open() a file in that dir.  This causes NameFromFH
 * to fail, because CreateDir() yields an EXCLUSIVE lock on the directory.
 * (Actually, NameFromFH works by using ParentFH to find the directory
 * in which the file exists, NameFromLock to find the name of that, and
 * ExamineFH to find the filename.  ParentFH complains about the
 * exclusive lock.  I get around this by almost duplicating NameFromFH,
 * (what little of it there is), except that since I already have the
 * parent directory locked, I needn't bother with ParentFH.
 *
 */

void TackPathFH(ULONG l,struct FileHandle *f,struct Transaction *t)
{
    UBYTE *p;
    struct FileInfoBlock *myfib;

    t->trans_RespDataActual = sizeof(struct TPacket);

    p = &(((UBYTE *)t->trans_ResponseData)[sizeof(struct TPacket)]);
    p[0]=0;

    myfib = (struct FileInfoBlock *) AllocDosObject(DOS_FIB,0L);
    if (myfib)
    {
        if (GNameFromLock(l,p,t->trans_RespDataLength-sizeof(struct TPacket)-1))
        {
            if (ExamineFH(MKBADDR(f),myfib))
            {
                if (AddPart(p,myfib->fib_FileName,t->trans_RespDataLength-sizeof(struct TPacket)-1))
                    t->trans_RespDataActual = sizeof(struct TPacket) + strlen(p) + 1;
            }
        }
        FreeDosObject(DOS_FIB,myfib);
    }

}

BPTR GetALock(BSTR q, BPTR relative, struct MountedFSInfo *mi, LONG type)
{

    STRPTR x;
    STRPTR g = (STRPTR) BADDR(q);
    UBYTE one[256],twox[264], *two;
    UBYTE FullPath[512];
    BPTR CurrentLock, NewLock;
    BOOL firstthrough = TRUE;

    CurrentLock = relative;

    two = (UBYTE *) (((ULONG)&twox[0] & (ULONG) ~7L)+8);

    CopyMem(&g[1], &FullPath[0], g[0]);
    FullPath[g[0]] = 0;

    x = &FullPath[0];
    while (TRUE)
    {
        STRPTR y;
        ULONG l;
        y = x;
        while ((*y != ':') && (*y != '/') && (*y))
            y++;

        if ((*y == ':') || (*y == '/'))
            y++;

        l = (ULONG) y - (ULONG) x;
        if (l > 255)
            return(0);

        strncpy(&one[0],x,l);
        one[l]=0;

        strcpy(&two[1],one);
        two[0] = strlen(one);


        if ((l == 1) && (one[0] == '/'))
        {
            /* Find out if the user is trying to jump back a directory from a mountpoint */
            /* If so, return with a NULL lock and an error of 'obj not found'. */
            if (SameLock((BPTR) CurrentLock,(BPTR) mi->mfsi_BaseLock) == LOCK_SAME)
            {
                if (!firstthrough)
                    UnLock(CurrentLock);
                ((struct Process *) FindTask(0L))->pr_Result2 = ERROR_OBJECT_NOT_FOUND;
                return(0L);
            }
            NewLock = ParentDir(CurrentLock);
        }
        else
            NewLock = DoPkt((((struct FileLock *)BADDR(mi->mfsi_BaseLock))->fl_Task),ACTION_LOCATE_OBJECT,(LONG) CurrentLock,(LONG) MKBADDR(two),ACCESS_READ,0,0);

        if (!firstthrough)
            UnLock(CurrentLock);

        if (!NewLock)
        {
            return(0L);
        }

        CurrentLock = NewLock;

        if (mi->mfsi_Flags & MFSIF_SECURITYON)
        {
            ULONG h;

            h = BestPermFromLock((struct FileLock *)CurrentLock,mi);
            if (h & (1 << 31))      /* If it's a directory */
            {
                if (UIDFromLock((struct FileLock *) CurrentLock) != mi->mfsi_UID)   /* owner can -always- get a lock! */
                {
                    if (!(h & FIBF_EXECUTE))
                    {
                        ((struct Process *) FindTask(0L))->pr_Result2 = ERROR_EXECUTE_PROTECTED;
                        return(0L);
                    }
                }
            }
        }

        firstthrough = FALSE;

        x = y;
        if (!(*x))
            break;
    }

    if ((type == ACCESS_WRITE) && (CurrentLock))            /* If they want an exclusive lock ... */
    {
        struct FileInfoBlock *afib;
        afib = AllocDosObject(DOS_FIB,0L);
        if (afib)
        {
            if (SafeExamine(CurrentLock,afib))
                if (afib->fib_DirEntryType < 0)
                    if (!ChangeMode(CHANGE_LOCK,CurrentLock,ACCESS_WRITE))
                    {
                        UnLock(CurrentLock);
                        CurrentLock = 0L;
                    }
            FreeDosObject(DOS_FIB,afib);
        }
    }

    return(CurrentLock);
}


BPTR RealParent(BPTR alock, BSTR path, struct MountedFSInfo *mi)
{

    UBYTE *a, *b;
    UBYTE *c;
    BPTR locktoreturn;


    a = (UBYTE *) BADDR(path);
    c = (UBYTE *) AllocMem(a[0]+1,0);
    if (c)
    {
        CopyMem(&a[1],&c[1],a[0]);
        c[0]=a[0];
        b = &c[a[0]];
        while ((b != &c[1]) && (*b != '/'))
            b--;
        *b=0;
        c[0]=strlen(&c[1]);

        locktoreturn = GetALock(MKBADDR(c),alock,mi,ACCESS_READ);

        FreeMem(c,a[0]+1);
        return(locktoreturn);
    }
    else
        return(0L);
}


UBYTE *matchstrings[] = {
	NULL,
	"~(.backdrop)",
	"~(Disk.info)",
	"~(.backdrop|Disk.info)",
};

#define MIN_PARSE_LEN	(25*2)

// Find the ExAllControl structure being used for lock...

struct EAC_Node *
FindExall (BPTR lock, ULONG flags, struct MountedFSInfo *mi)
{
	struct ExAllControl *ec;
	struct EAC_Node *node;
	struct MinList *exlist = &(ExAllList);
	ULONG stringnum = 0;

	for (node = (struct EAC_Node *) exlist->mlh_Head;
	     node->en_Node.mln_Succ;
	     node = (struct EAC_Node *) node->en_Node.mln_Succ)
	{
		if (lock == node->en_Lock)
		{
//kprintf("found ex $%lx for lock $%lx!\n",node->en_EAC,lock);
			// if not at head of list, put it there
			if (node != (struct EAC_Node *) exlist->mlh_Head)
			{
				Remove((struct Node *) &(node->en_Node));
				AddHead((struct List *) exlist,
					(struct Node *) &(node->en_Node));
			}

			return node;
		}
	}

	// didn't find it.  First time Exall, or a lost lock/reconnect

	node = AllocVec(sizeof(*node) + MIN_PARSE_LEN,0L);
	if (!node)
		return NULL;

	node->en_EAC = ec = AllocDosObject(DOS_EXALLCONTROL,0L);
	if (!ec)
	{
		FreeVec(node);
		return NULL;
	}

	node->en_Lock = lock;
	node->en_Type = 0;
	AddHead((struct List *) exlist,(struct Node *) &(node->en_Node));

	// figure out the string to use...
	if ((flags & MFSIF_NOBACKDROP) || (mi->mfsi_Flags & MFSIF_NOBACKDROP))
		stringnum = 1;
	if ((flags & MFSIF_NODISKINFO) || (mi->mfsi_Flags & MFSIF_NODISKINFO))
		stringnum += 2;

	// try to avoid doing the SameLock if we don't need to
	if (stringnum && SameLock(lock,(BPTR)mi->mfsi_BaseLock) != LOCK_SAME)
		stringnum = 0;

	if (stringnum)
	{
		ec->eac_MatchString = (UBYTE *) (((ULONG) node) + sizeof(*node));
		if (ParsePatternNoCase(matchstrings[stringnum],ec->eac_MatchString,MIN_PARSE_LEN)
		    == -1)
		{
			ec->eac_MatchString = NULL;	// Punt!
		}
	}

//kprintf("made ec $%lx, string %s\n",ec,ec->eac_MatchString);
	return node;
}

void
RemExall (BPTR lock, ULONG done)
{
	struct EAC_Node *node;

	for (node = (struct EAC_Node *) ExAllList.mlh_Head;
	     node->en_Node.mln_Succ;
	     node = (struct EAC_Node *) node->en_Node.mln_Succ)
	{
		if (lock == node->en_Lock)
		{
//kprintf("removing ec $%lx (lock $%lx)\n",node->en_EAC,lock);
			Remove((struct Node *) &(node->en_Node));
			if (!done)
			{
			    UBYTE *buf = ExallBuffer;	// minimal buffer for exall...

//kprintf("removing active exall for lock $%lx\n",lock);
			    // must ExAll() until we're done
			    if (((struct DosLibrary *) DOSBase)->dl_lib.lib_Version >= 39)
			    {
				ExAllEnd(lock,(struct ExAllData *) buf,sizeof(ExallBuffer),
					 ED_NAME,node->en_EAC);
//kprintf("called exallend($%lx,$%lx,%ld,%ld,$%lx\n",
//lock,(struct ExAllData *) buf,sizeof(ExallBuffer),ED_NAME,node->en_EAC);
			    } else {
//kprintf("exall($%lx,$%lx,%ld,%ld,$%lx\n",lock,(struct ExAllData *) buf,sizeof(ExallBuffer),ED_NAME,node->en_EAC);
				while (ExAll(lock,(struct ExAllData *) buf,sizeof(ExallBuffer),
					     ED_NAME,node->en_EAC))
				{
					/* nothing */ ;
//kprintf("called exall, got %ld...\n",node->en_EAC->eac_Entries);
				}
//kprintf("done with exall, got %ld\n",node->en_EAC->eac_Entries);
			    }
			}
			FreeDosObject(DOS_EXALLCONTROL,node->en_EAC);
			FreeVec(node);

			return;
		}
	}
//kprintf("couldn't find ec for lock $%lx to remove!\n",lock);
}
