
#include <stdio.h>
#include <string.h>

#include <exec/types.h>
#include <intuition/intuition.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/nipc_pragmas.h>
#include <pragmas/iffparse_pragmas.h>
#include <pragmas/accounts_pragmas.h>
#include <exec/memory.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/utility_protos.h>
#include <clib/nipc_protos.h>
#include <clib/intuition_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/accounts_protos.h>
#include <envoy/services.h>
#include <envoy/nipc.h>
#include <envoy/accounts.h>
#include <envoy/errors.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/exall.h>
#include <libraries/iffparse.h>
#include "/fs.h"
#include "fs_rev.h"
#include "fsdbase.h"

struct ExportVolume
{
    struct Node     ev_Link;            /* Link node */
    struct List     ev_Users;           /* List of users that can access this volume */
    UBYTE           ev_VolumeName[64];  /* Name of this volume for remote users */
    ULONG           ev_Flags;           /* Flag bits for this export */
};

#define EVF_SnapshotOK          1
#define EVF_LeftoutOK           2

struct UserOrGroup
{
    struct Node     ug_Link;            /* Link node */
    UWORD           ug_ID;
    UBYTE           ug_Type;            /* see below */
};

#define UGTYPE_USER             0
#define UGTYPE_GROUP            1

 char *revision=VERSTAG;

 extern STRPTR  FSDUser;
 extern STRPTR  FSDPassword;
 extern STRPTR  FSDEntityName;
 extern ULONG   FSDSignalMask;
 extern ULONG   FSDError;
 extern struct Task *FSDSMProc;

 extern __far FSDServer(void);

void RemExall(BPTR lock, ULONG done);
void ASM CleanupDeadMount(REG(a0) struct MountedFSInfo *m);
BOOL CheckV37(BPTR lock);

ULONG ASM StartService(REG(a0) struct TagItem *taglist)
{
    ULONG cptags[5];
    UBYTE *entityName;
    STRPTR userName;
    STRPTR password;
    struct Process *fsdproc;
//    BOOL status = FALSE;
    UBYTE  sigbit;
    struct TagItem *tstate;
    struct TagItem *tag;

    tstate = taglist;
    while (tag = NextTagItem(&tstate))
    {
        switch(tag->ti_Tag)
        {
            case SSVC_EntityName:
                entityName = (UBYTE *) tag->ti_Data;
                break;
            case SSVC_UserName:
                userName = (STRPTR) tag->ti_Data;
                break;
            case SSVC_Password:
                password = (STRPTR) tag->ti_Data;
                break;
        }
    }

    strcpy(entityName,"Filesystem");

    if(!FSDBase->FSD_Entity)
    {
        LoadConfig(&Mounts);
        if(sigbit = AllocSignal(-1L))
        {
            FSDSMProc = (struct Task *) FindTask(0L);
            FSDSignalMask = (1L<<sigbit);

            cptags[0] = NP_Entry;
            cptags[1] = (ULONG) FSDServer;
            cptags[2] = NP_Name;
            cptags[3] = (ULONG) "Filesystem";
            cptags[4] = TAG_DONE;

            FSDUser = userName;
            FSDPassword = password;
            FSDEntityName = entityName;

            if(fsdproc = (struct Process *) CreateNewProc((struct TagItem *)cptags))
            {
                Wait(1L<<sigbit);
//                status = TRUE;
            }
            FreeSignal(sigbit);
        }
    }
    else
    {
        FSDError = 0;
    }
    return FSDError;
}

VOID ASM GetServiceAttrsA(REG(a0) struct TagItem *tagList)
{
    struct TagItem *ti;

    if(ti=(struct TagItem *)FindTagItem( SVCAttrs_Name, tagList))
    {
        strcpy((STRPTR)ti->ti_Data,"Filesystem");
    }
}

VOID ASM Server(REG(a0) STRPTR IUserName,
                REG(a1) STRPTR IPassword,
                REG(a2) STRPTR IEntityName)
{
    struct MountedFSInfo *mi;
    struct MinList *mounts = &FSDBase->FSD_Current;
    struct Entity *re;
    ULONG sigbit;
    ULONG cetags[7]={ENT_Name,0L,ENT_Public,0,ENT_AllocSignal,0L,TAG_DONE};
    ULONG setags[4]={ENT_TimeoutLinks,1800,TAG_DONE,0};
    register struct SignalSemaphore *ss = &FSDBase->FSD_CurrentLock;

    NewList((struct List *) &ExAllList);
    NewList((struct List *) mounts);
    InitSemaphore(ss);

    cetags[1]=(ULONG) IEntityName;
    cetags[5]=(ULONG) &sigbit;

    re = (struct Entity *) CreateEntityA((struct TagItem *)&cetags);
    if (!re)
        FSDError = 999;

    SetEntityAttrsA(re,(struct TagItem *) &setags);

    FSDBase->FSD_Entity = re;

    Signal(FSDSMProc,FSDSignalMask);

    if (re)
    {
        struct Transaction *t;

        while (TRUE)
        {
            WaitEntity(re);

            t = GetTransaction(re);
            if (t)
            {
                switch (t->trans_Command)
                {
                    case (FSCMD_SHOWMOUNTS):
                    {
                        struct RequestMounts *rm;
                        STRPTR out;
                        struct ExportVolume *ev;
                        struct UserInfo *ui;
                        struct GroupInfo *gi;

                        out = t->trans_ResponseData;
                        rm = t->trans_RequestData;
                        gi = AllocGroupInfo();
                        if (gi)
                        {
                            ui = AllocUserInfo();
                            if (ui)
                            {
                            /* ARgh!  This sucks -- by allowing some mounts to be "non-security", I have to conditionally give the user
                             * less error feedback -- if given a bad username & pw, I don't know if they're talking to a non-secure
                             * mount, or just typed in a pw wrong.  I'm settling for the point that if NO mounts are
                             * found, then I check username/pw & return "bad u/pw" in that case.  Not a good solution, but
                             * better than nothing.
                             */
                                int verd;
                                /* If password is encrypted */
                                if (rm->Password[0] == '$')
                                    verd = VerifyUserCrypt(rm->User,&rm->Password[1],ui);
                                else
                                    verd = VerifyUser(rm->User,rm->Password,ui);

                                ev = (struct ExportVolume *) Mounts.lh_Head;
                                while (ev->ev_Link.ln_Succ)
                                {
                                    struct UserOrGroup *q;
                                    /* check to see if this user/pw fits for this mount */
                                    q = (struct UserOrGroup *) ev->ev_Users.lh_Head;
                                    if (!(ev->ev_Flags & EVF_NoSecurity))  /* Everyone! */
                                    {
                                        while (q->ug_Link.ln_Succ)
                                        {
                                            if (q->ug_Type == UGTYPE_USER)
                                            {
                                                if (!verd)
                                                    if (ui->ui_UserID == q->ug_ID)
                                                    {
                                                        break;
                                                    }
                                            }
                                            else
                                            {
                                                if (!IDToGroup(q->ug_ID,gi))
                                                {
                                                    if (!verd)
                                                        if (!MemberOf(gi,ui))
                                                        {
                                                            break;
                                                        }
                                                }
                                            }
                                            q = (struct UserOrGroup *) q->ug_Link.ln_Succ;
                                        }
                                    }
                                    if (FSDBase->FSD_Mode == FSDMODE_NORMAL)
                                    {
                                        if ( (q->ug_Link.ln_Succ) || (ev->ev_Flags & EVF_NoSecurity) )
                                        {
					    // NOTE!  Greg uses modification of out as a flag!
                                            if (ev->ev_VolumeName[0])
                                                strcpy(out,ev->ev_VolumeName);      // Give the volume name rather than the device name
                                            else
                                                strcpy(out,ev->ev_Link.ln_Name);    // If no vol name exists, go dev name
                                            out = &out[64];
                                        }
                                    }
                                    if (((ULONG) out - (ULONG) t->trans_ResponseData) >= t->trans_RespDataLength)
                                        break;
                                    ev = (struct ExportVolume *) ev->ev_Link.ln_Succ;
                                }
                                if ((ULONG)out == (ULONG) t->trans_ResponseData)
                                {
                                    if (!VerifyUser(rm->User,rm->Password,ui))
                                        t->trans_Error = FSERR_REJECTED_USER;
                                }
                                FreeUserInfo(ui);
                            }
                            FreeGroupInfo(gi);
                        }
                        t->trans_RespDataActual = ((ULONG)out - (ULONG) t->trans_ResponseData);

                        ReplyTransaction(t);
                        break;
                    }
                    case (FSCMD_DOSPACKET):
                    {
                        struct TPacket *tp;

                        tp = (struct TPacket *) t->trans_RequestData;
                        mi = (struct MountedFSInfo *) tp->tp_ServerMFSI;

                        if (FSDBase->FSD_Mode == FSDMODE_NORMAL)
                        {
                            struct MountedFSInfo *mm;

                            mm = (struct MountedFSInfo *) mounts->mlh_Head;
                            while (mm->mfsi_Link.mln_Succ)
                            {
                                if (mm == mi)
                                {
                                    if (t->trans_SourceEntity == mi->mfsi_SourceEntity)
                                        break;
                                }
                                mm = (struct MountedFSInfo *) mm->mfsi_Link.mln_Succ;
                            }
                            if (!mm->mfsi_Link.mln_Succ)
                            {
                                t->trans_Error = FSERR_REJECTED_USER;
                                ReplyTransaction(t);
                                break;
                            }
                            Remove((struct Node *)mm);
                            AddHead((struct List *)mounts,(struct Node *)mm);   /* Keep recent mfsi's at the top of the list */
                            ObtainSemaphore(ss);
                            DoDosPacket(t,mi);
                            ReleaseSemaphore(ss);
                        }
                        else
                            t->trans_Error = FSERR_REJECTED_USER;
                        ReplyTransaction(t);
                        break;
                    }
                    case (FSCMD_MOUNT):
                    {
                        APTR oldwindow;
                        struct Process *p;
                        struct MountedFSInfo *z;
                        ULONG denial= -1L;

                        t->trans_RespDataActual = 4L;
                        p = (struct Process *) FindTask(0L);
                        oldwindow = (APTR) p->pr_WindowPtr;
                        mi = (struct MountedFSInfo *) AllocMem(sizeof(struct MountedFSInfo),MEMF_PUBLIC|MEMF_CLEAR);
                        if (mi)
                        {
                            UBYTE *lockname;
                            UBYTE *ch;
                            BOOL aup;
                            struct ExportVolume *ev;
                            struct UserInfo *ui;
                            struct GroupInfo *gi;
                            STRPTR Uname, Pword;

                            Uname = &mi->mfsi_UserName[0];
                            Pword = &mi->mfsi_Password[0];

                            mi->mfsi_BaseLock = 0L;
                            mi->mfsi_Flags = 0L;
                            NewList((struct List *)&mi->mfsi_Locks);
                            NewList((struct List *)&mi->mfsi_FileHandles);
                            NewList((struct List *)&mi->mfsi_Etc);
                            mi->mfsi_SourceEntity = t->trans_SourceEntity;
                            p->pr_WindowPtr = (struct Window *) -1L;
                            lockname = (UBYTE *) t->trans_RequestData;
                            /* Match the volume name they've asked for up with a real name that's lockable */
                            ev = (struct ExportVolume *) Mounts.lh_Head;
                            while (ev->ev_Link.ln_Succ)
                            {
                                if (!Stricmp(t->trans_RequestData,ev->ev_VolumeName))
                                {
                                    lockname = (UBYTE *) ev->ev_Link.ln_Name;
                                    break;
                                }
                                ev = (struct ExportVolume *) ev->ev_Link.ln_Succ;
                            }

                            mi->mfsi_BaseLock = (struct FileLock *) Lock(lockname,ACCESS_READ);
                            GetHostName(t->trans_SourceEntity,(STRPTR)&mi->mfsi_HostName,80);
                            /* Extract the client's name for the mount, username, and password from the mount request packet */
                            ch = (UBYTE *) t->trans_RequestData;
                            while (*ch)
                                ch++;
                            ch++;

                            strcpy((char *)&mi->mfsi_ClientMount,(char *)ch);
                            while (*ch)
                                ch++;
                            ch++;

                            strcpy((char *)&mi->mfsi_UserName,(char *)ch);
                            while (*ch)
                                ch++;
                            ch++;

                            strcpy((char *)&mi->mfsi_Password,(char *)ch);
                            //while (*ch)	not needed, we're done with ch
                            //    ch++;
                            //ch++;

                            /* Attempt to Authenticate a user by the name and password
                             * given.
                             */
                            /* Verify that the mount they're requesting is possible */
                            aup = FALSE;
                            gi = AllocGroupInfo();
                            if (gi)
                            {
                                ui = AllocUserInfo();
                                if (ui)
                                {
                                    /* This user/pw is real.  Try to find the mount they want. */
                                    ev = (struct ExportVolume *) Mounts.lh_Head;
                                    while (ev->ev_Link.ln_Succ)
                                    {
                                        /* If they ask for either the raw device name or the
                                         * permitted volume name, match on it.
                                         */
                                        if ( (!Stricmp(ev->ev_Link.ln_Name,t->trans_RequestData)) ||
                                             (!Stricmp(ev->ev_VolumeName,t->trans_RequestData)) )
                                        {
                                            /* Okay ... that mount exists ... are they permitted to use it? */
                                            struct UserOrGroup *ug;

                                            ug = (struct UserOrGroup *) ev->ev_Users.lh_Head;

                                            if (ev->ev_Flags & EVF_NoSecurity)  /* Everyone! */
                                            {
                                                aup = TRUE;
                                                mi->mfsi_UID = (UWORD) -1;
                                                mi->mfsi_GID = (UWORD) -1;
                                                mi->mfsi_UserFlags = 0;
                                                break;
                                            }
                                            else
                                            {
                                                while (ug->ug_Link.ln_Succ)
                                                {
                                                    BOOL gooduser=FALSE;

                                                    if (ug->ug_Type == UGTYPE_USER)
                                                    {
                                                        ULONG stat;
                                                        if (Pword[0] == '$')
                                                            stat = VerifyUserCrypt(Uname,&Pword[1],ui);
                                                        else
                                                            stat = VerifyUser(Uname,Pword,ui);

                                                        if (!stat)
                                                        {
                                                            if (ui->ui_UserID == ug->ug_ID)
                                                                if (FSDBase->FSD_Mode == FSDMODE_NORMAL)
                                                                    gooduser = TRUE;
                                                        }
                                                        else
                                                        {
                                                            denial = FSERR_REJECTED_USER;
                                                            break;
                                                        }
                                                    }
                                                    else
                                                    {
                                                        ULONG stat;
                                                        if (Pword[0] == '$')
                                                            stat = VerifyUserCrypt(Uname,&Pword[1],ui);
                                                        else
                                                            stat = VerifyUser(Uname,Pword,ui);

                                                        if (!stat)
                                                        {
                                                            if (!IDToGroup(ug->ug_ID,gi))
                                                            {
                                                                if (!MemberOf(gi,ui))
                                                                    if (FSDBase->FSD_Mode == FSDMODE_NORMAL)
                                                                        gooduser = TRUE;
                                                            }
                                                        }
                                                        else
                                                        {
                                                            denial = FSERR_REJECTED_USER;
                                                            break;
                                                        }

                                                    }
                                                    if (gooduser)
                                                    {
                                                        aup = TRUE;
                                                        mi->mfsi_UID = ui->ui_UserID;
                                                        mi->mfsi_GID = ui->ui_PrimaryGroupID;
                                                        mi->mfsi_UserFlags = ui->ui_Flags;
                                                        break;
                                                    }
                                                    ug = (struct UserOrGroup *) ug->ug_Link.ln_Succ;
                                                }
                                                if (!ug->ug_Link.ln_Succ)
                                                    denial = FSERR_REJECTED_USER;
                                                else
                                                    break;
                                            }
                                        }
                                        ev = (struct ExportVolume *) ev->ev_Link.ln_Succ;
                                    }
                                    if (!ev->ev_Link.ln_Succ)
                                        denial = FSERR_REJECTED_NOMOUNT;
                                    FreeUserInfo(ui);
                                }
                                FreeGroupInfo(gi);

                                if (!aup)
                                {
                                    FreeMem(mi,sizeof(struct MountedFSInfo));
                                    mi=0L;
                                }
                                else
                                {
                                    if (TRUE) /* This user/group isn't priv'd. */
                                    {
                                        p->pr_WindowPtr = (struct Window *) oldwindow;
                                        if (mi->mfsi_BaseLock)
                                        {
                                            STRPTR xp;
                                            ULONG notag[2]={TAG_DONE,0};
                                            t->trans_Error = 0L;
                                            ((ULONG *)t->trans_ResponseData)[0] = (ULONG) mi;
                                            xp = (STRPTR) (t->trans_ResponseData);
                                            xp = &xp[4];
                                            strcpy((UBYTE *)xp,(UBYTE *)&ev->ev_VolumeName);
                                            mi->mfsi_Flags |= MFSIF_NOBACKDROP|MFSIF_NODISKINFO|MFSIF_SECURITYON;
                                            if (ev->ev_Flags & EVF_SnapshotOK)
                                                mi->mfsi_Flags &= ~MFSIF_NODISKINFO;
                                            if (ev->ev_Flags & EVF_LeftoutOK)
                                                mi->mfsi_Flags &= ~MFSIF_NOBACKDROP;
                                            if (!(ev->ev_Flags & EVF_Full))
                                                mi->mfsi_Flags &= ~MFSIF_SECURITYON;

					    // we must know if this is a V37 filesystem
					    if (CheckV37(mi->mfsi_BaseLock))
						mi->mfsi_Flags |= MFSIF_V37_FS | MFSIF_COMMENT_BSTR;

                                            mi->mfsi_VolumeName = &ev->ev_VolumeName[0];

                                            mi->mfsi_FIB = (APTR) AllocDosObject(DOS_FIB, (struct TagItem *) &notag);

                                            t->trans_RespDataActual = (strlen(xp))+1+4;
                                            ReplyTransaction(t);
                                        }
                                        else
                                        {
                                            FreeMem(mi,sizeof(struct MountedFSInfo));
                                            mi = 0L;
                                        }
                                        if (mi)
                                        {
                                            /* Search for duplicates */
                                            /* Same source name, equiv. locks, same foreign lock name constitutes a match */
                                            z = (struct MountedFSInfo *) mounts->mlh_Head;
                                            while (z->mfsi_Link.mln_Succ)
                                            {
                                                if ( (!strcmp((char *)&z->mfsi_HostName,(char *)&mi->mfsi_HostName)) &&
                                                     (!strcmp((char *)&z->mfsi_ClientMount,(char *)&mi->mfsi_ClientMount)) &&
                                                     (SameLock((BPTR)z->mfsi_BaseLock,(BPTR)mi->mfsi_BaseLock) == LOCK_SAME) )
                                                {
                                                    struct MountedFSInfo *q;
                                                    q = z;
                                                    z = (struct MountedFSInfo *)z->mfsi_Link.mln_Pred;
                                                    ObtainSemaphore(ss);
                                                    Remove((struct Node *)q);
                                                    ReleaseSemaphore(ss);
                                                    CleanupDeadMount(q);
                                                }
                                                z = (struct MountedFSInfo *) z->mfsi_Link.mln_Succ;
                                            }
                                            ObtainSemaphore(ss);
                                            AddTail((struct List *)mounts,(struct Node *)mi);
                                            ReleaseSemaphore(ss);
                                        }
                                    }
                                    else /* User not accepted */
                                    {
                                        FreeMem(mi,sizeof(struct MountedFSInfo));
                                        mi=0L;
                                    }
                                }
                            }
                            else
                            {
                                FreeMem(mi,sizeof(struct MountedFSInfo));
                                mi = 0L;
                            }
                        }
                        if (!mi)
                        {
                            t->trans_Error = denial;
                            /* Institute use count */
                            ((ULONG *)t->trans_ResponseData)[0]=0L;
                            t->trans_RespDataActual = 4;
                            ReplyTransaction(t);
                            /* Close all open files, free locks */
                        }
                        break;
                    }
                    default:
                        break;
                }
            }
        }
    }
}

void KeepRecordLock(struct MountedFSInfo *m, BPTR fh, ULONG pos, ULONG len)
{
    struct LockedRecord *l;
    l = (struct LockedRecord *) AllocMem(sizeof(struct LockedRecord),MEMF_CLEAR);
    if (l)
    {
        l->rl_FileHandle = fh;
        l->rl_Position = pos;
        l->rl_Length = len;
        KeepEtc(ETCTYPE_RecordLock,(APTR) l, m);
    }
}

void NukeRecordLock(struct MountedFSInfo *m, BPTR fh, ULONG pos, ULONG len)
{
    struct LockedRecord *l;
    struct ResourcesUsed *r;
    r = (struct ResourcesUsed *) m->mfsi_Etc.mlh_Head;
    while (r->ru_link.ln_Succ)
    {
        l = (struct LockedRecord *) r->ru_Resource;
        if ((l->rl_FileHandle == fh) && (l->rl_Position == pos) && (l->rl_Length == len) &&
            (r->ru_link.ln_Type == ETCTYPE_RecordLock))
        {
            Remove((struct Node *)r);
            FreeMem(r,sizeof(struct ResourcesUsed));
            return;
        }
        r = (struct ResourcesUsed *) r->ru_link.ln_Succ;
    }
}


void KeepEtc(ULONG type, APTR data, struct MountedFSInfo *m)
{
    struct ResourcesUsed *ru;

    ru = (struct ResourcesUsed *) AllocMem(sizeof(struct ResourcesUsed),MEMF_CLEAR|MEMF_PUBLIC);
    if (!ru)
        return;
    ru->ru_link.ln_Type = type;
    ru->ru_Resource = data;
    AddTail((struct List *)&m->mfsi_Etc,(struct Node *)&ru->ru_link);
}

void NukeEtc(APTR data, struct MountedFSInfo *m)
{
    struct ResourcesUsed *r;
    r = (struct ResourcesUsed *) m->mfsi_Etc.mlh_Head;
    while (r->ru_link.ln_Succ)
    {
        if (r->ru_Resource == (APTR) data)
        {
            Remove((struct Node *)r);
            FreeMem(r,sizeof(struct ResourcesUsed));
            return;
        }
        r = (struct ResourcesUsed *) r->ru_link.ln_Succ;
    }
}

void KeepLock(APTR thelock,struct MountedFSInfo *m)
{

    struct ResourcesUsed *ru;

    ru = (struct ResourcesUsed *) AllocMem(sizeof(struct ResourcesUsed),MEMF_CLEAR|MEMF_PUBLIC);
    if (!ru)
        return;
    ru->ru_Resource = thelock;
    AddTail((struct List *)&m->mfsi_Locks,(struct Node *)&ru->ru_link);
}


void KeepFH(APTR thefh,struct MountedFSInfo *m)
{

    struct ResourcesUsed *ru;

    ru = (struct ResourcesUsed *) AllocMem(sizeof(struct ResourcesUsed),MEMF_CLEAR|MEMF_PUBLIC);
    if (!ru)
        return;
    ru->ru_Resource = thefh;
    AddTail((struct List *)&m->mfsi_FileHandles,(struct Node *)&ru->ru_link);
}

void NukeLock(APTR thelock,struct MountedFSInfo *m)
{
    struct ResourcesUsed *r;
    if (!thelock)
        return;

    // also must clean up any exalls underway
    RemExall((BPTR) thelock,FALSE);

    r = (struct ResourcesUsed *) m->mfsi_Locks.mlh_Head;
    while (r->ru_link.ln_Succ)
    {
        if (r->ru_Resource == (APTR) thelock)
        {
            Remove((struct Node *)r);
            FreeMem(r,sizeof(struct ResourcesUsed));
            return;
        }
        r = (struct ResourcesUsed *) r->ru_link.ln_Succ;
    }
}

void NukeFH(APTR thefh,struct MountedFSInfo *m)
{

    struct ResourcesUsed *r;
    r = (struct ResourcesUsed *) m->mfsi_FileHandles.mlh_Head;
    while (r->ru_link.ln_Succ)
    {
        if (r->ru_Resource == (APTR) thefh)
        {
            Remove((struct Node *)r);
            FreeMem(r,sizeof(struct ResourcesUsed));
            return;
        }
        r = (struct ResourcesUsed *) r->ru_link.ln_Succ;
    }
}

void ASM CleanupDeadMount(REG(a0) struct MountedFSInfo *m)
{

    struct ResourcesUsed *r;

    if (m->mfsi_FIB)
        FreeDosObject(DOS_FIB,m->mfsi_FIB);

    while (r = (struct ResourcesUsed *) RemHead((struct List *)&m->mfsi_Locks))
    {
        DoPkt(((struct FileLock *)BADDR(m->mfsi_BaseLock))->fl_Task,ACTION_FREE_LOCK,(ULONG)r->ru_Resource,0,0,0,0);
        FreeMem(r,sizeof(struct ResourcesUsed));
    }

    while (r = (struct ResourcesUsed *) RemHead((struct List *)&m->mfsi_FileHandles))
    {
        struct FileHandle *f;
        f = (struct FileHandle *) r->ru_Resource;
        DoPkt(((struct FileLock *)BADDR(m->mfsi_BaseLock))->fl_Task,ACTION_END,f->fh_Arg1,0,0,0,0);
        FreeMem(f,sizeof(struct SFH));
        FreeMem(r,sizeof(struct ResourcesUsed));
    }

    while (r = (struct ResourcesUsed *) RemHead((struct List *)&m->mfsi_Etc))
    {
        switch (r->ru_link.ln_Type)
        {
            case ETCTYPE_RecordLock:
            {
                struct LockedRecord *l=r->ru_Resource;
                UnLockRecord(l->rl_FileHandle,l->rl_Position,l->rl_Length);
                FreeMem(l,sizeof(struct LockedRecord));
                break;
            }
        }
        FreeMem(r,sizeof(struct ResourcesUsed));
    }

    FreeMem(m,sizeof(struct MountedFSInfo));
}


BOOL BadStructure(UBYTE *structname)
{
    BOOL returnval = FALSE;
    struct Library *ib;

    ib = OpenLibrary("intuition.library",0L);
    IntuitionBase = ib;
    if (IntuitionBase)
    {
        struct EasyStruct ers={sizeof(struct EasyStruct),0L,"Envoy Network Problem",
                               "FS Server reports a null\nstructure.  Name:\n'%s'","OK"};
        returnval = (BOOL) EasyRequestArgs(0L,&ers,0L,&structname);

        CloseLibrary(ib);
    }

    return(returnval);
}

// fs version code stolen from c:Version

#include <exec/resident.h>

struct Resident *FindRomTag(BPTR segList)
{
struct Resident *tag;
UWORD           *seg;
ULONG            i;
ULONG           *ptr;

    while (segList)
    {
        ptr     = (ULONG *)((ULONG)segList << 2);
        seg     = (UWORD *)((ULONG)ptr);
        segList = *ptr;

        for (i=*(ptr-1)>>1; (i>0) ; i--)
        {
            if (*(seg++) == RTC_MATCHWORD)
            {
                tag = (struct Resident *)(--seg);
                if (tag->rt_MatchTag == tag)
                {
                    return(tag);
                }
            }
        }
    }

    return(NULL);
}


// return TRUE if this is a V37 ROM filesystem, which doesn't (really) support ED_OWNER safely

BOOL CheckV37(BPTR lock)
{
	struct FileLock *l = BADDR(lock);
	struct Resident *resident;
	struct DosList *dl;
	BOOL result = FALSE;

	dl = LockDosList(LDF_READ|LDF_DEVICES);

	// if the export has a volume and no device, we won't find it, so we assume >= 37
	do {
		dl = NextDosEntry(dl,LDF_READ|LDF_DEVICES);
		if (dl && (dl->dol_Task == l->fl_Task))
		{
			// found the filesystem we're exporting!
			if (dl->dol_misc.dol_handler.dol_Startup)
			{
				// try to make sure it's the ROM fs or l:FastFileSystem
				if (resident = FindRomTag(dl->dol_misc.dol_handler.dol_SegList))
				{
					if (resident->rt_Version < 39 &&
					    (strncmp(resident->rt_IdString,"fs 37.",
						     strlen("fs 37.")) == 0 ||
					     strncmp(resident->rt_Name,"ffs 37.",
						     strlen("ffs IdString.")) == 0))
					{
						result = TRUE;
					}
				}
			}
			break;
		}
	} while (dl);
	UnLockDosList(LDF_READ|LDF_DEVICES);

	return result;
}


/*********************/

struct ConfigStruct
{
    UBYTE   MountName[64];
    UBYTE   VolumeName[64];
    ULONG   Flags;
};

struct ConfigUser
{
    UWORD   ID;
    UBYTE   Flags;
    UBYTE   Filler;
};


#define ID_EFSC     MAKE_ID('E','F','S','C')
#define ID_VOLM     MAKE_ID('V','O','L','M')

#define IFFPrefChunkCnt     1

static LONG far IffPrefChunks[]={ID_EFSC,ID_VOLM};

BOOL LoadConfig(struct List *ExportList)
{
    struct ConfigStruct *cs;
    struct ConfigUser *cu;
    struct Library *i;

    cs = (struct ConfigStruct *) AllocMem(sizeof(struct ConfigStruct),MEMF_PUBLIC);
    if (cs)
    {
        cu = (struct ConfigUser *) AllocMem(sizeof(struct ConfigUser),MEMF_PUBLIC);
        if (cu)
        {
            i = (struct Library *) OpenLibrary("iffparse.library",37L);
            IFFParseBase = i;
            if (IFFParseBase)
            {
                struct IFFHandle *iff;
                iff = (struct IFFHandle *) AllocIFF();
                if (iff)
                {
                    iff->iff_Stream = (ULONG) Open("env:envoy/efs.config",MODE_OLDFILE);
                    if (!iff->iff_Stream)
                        iff->iff_Stream = (ULONG) Open("envarc:envoy/efs.config",MODE_OLDFILE);
                    if (iff->iff_Stream)
                    {
                        InitIFFasDOS(iff);
                        if (!OpenIFF(iff,IFFF_READ))
                        {
                            if (!StopChunk(iff,ID_EFSC,ID_VOLM))
                            {
                                if (!ParseIFF(iff,IFFPARSE_SCAN))
                                {
                                    while (TRUE)
                                    {
                                        struct ContextNode *cn;
                                        cn = (struct ContextNode *) CurrentChunk(iff);
                                        if (cn)
                                        {
                                            ULONG size;
                                            size = cn->cn_Size;
                                            if (ReadChunkBytes(iff,cs,sizeof(struct ConfigStruct)) == sizeof(struct ConfigStruct))
                                            {
                                                struct ExportVolume *ev;
                                                ev = (struct ExportVolume *) AllocMem(sizeof(struct ExportVolume),MEMF_PUBLIC);
                                                if (ev)
                                                {
                                                    UBYTE *namex;
                                                    namex = (UBYTE *) AllocMem(strlen(cs->MountName)+1,MEMF_PUBLIC);
                                                    if (namex)
                                                    {
                                                        strcpy (namex,cs->MountName);
                                                        strcpy (ev->ev_VolumeName,cs->VolumeName);
                                                        ev->ev_Flags = cs->Flags;
                                                        ev->ev_Link.ln_Name = namex;
                                                        NewList(&ev->ev_Users);
                                                        AddTail(ExportList,&ev->ev_Link);
                                                    }
                                                }
                                                size -= sizeof(struct ConfigStruct);
                                                while(size)
                                                {
                                                    if (ReadChunkBytes(iff,cu,sizeof(struct ConfigUser)) == sizeof(struct ConfigUser))
                                                    {
                                                        struct UserOrGroup *ugx;
                                                        ugx = (struct UserOrGroup *) AllocMem(sizeof(struct UserOrGroup),MEMF_PUBLIC);
                                                        if (ugx)
                                                        {
                                                            UBYTE *namex;
                                                            ugx->ug_ID = cu->ID;
                                                            ugx->ug_Type = cu->Flags;
                                                            if (!ugx->ug_Type)
                                                            {
                                                                struct UserInfo *ui;
                                                                ui = AllocUserInfo();
                                                                if (ui)
                                                                {
                                                                    if (!IDToUser(cu->ID,ui))
                                                                    {
                                                                        namex = (UBYTE *) AllocMem(strlen(ui->ui_UserName)+1,MEMF_PUBLIC);
                                                                        if (namex)
                                                                        {
                                                                            strcpy(namex,ui->ui_UserName);
                                                                            ugx->ug_Link.ln_Name = namex;
                                                                        }
                                                                        else
                                                                            break;
                                                                    }
                                                                    else
                                                                    {
                                                                        namex = (UBYTE *) AllocMem(8,MEMF_PUBLIC);
                                                                        if (namex)
                                                                        {
                                                                            strcpy(namex,"Old UID");
                                                                            ugx->ug_Link.ln_Name = namex;
                                                                        }
                                                                        else
                                                                            break;
                                                                    }
                                                                    AddTail(&ev->ev_Users,(struct Node *)ugx);
                                                                    FreeUserInfo(ui);
                                                                }
                                                            }
                                                            else
                                                            {
                                                                struct GroupInfo *gi;
                                                                gi = AllocGroupInfo();
                                                                if (gi)
                                                                {
                                                                    if (!IDToGroup(cu->ID,gi))
                                                                    {
                                                                        namex = (UBYTE *) AllocMem(strlen(gi->gi_GroupName)+1,MEMF_PUBLIC);
                                                                        if (namex)
                                                                        {
                                                                            strcpy(namex,gi->gi_GroupName);
                                                                            ugx->ug_Link.ln_Name = namex;
                                                                        }
                                                                        else
                                                                            break;
                                                                    }
                                                                    else
                                                                    {
                                                                        namex = (UBYTE *) AllocMem(8,MEMF_PUBLIC);
                                                                        if (namex)
                                                                        {
                                                                            strcpy(namex,"Old GID");
                                                                            ugx->ug_Link.ln_Name = namex;
                                                                        }
                                                                        else
                                                                            break;
                                                                    }
                                                                    AddTail(&ev->ev_Users,(struct Node *)ugx);
                                                                    FreeGroupInfo(gi);
                                                                }

                                                            }
                                                        }
                                                    }
                                                    else
                                                        break;
                                                    size -= sizeof(struct ConfigUser);
                                                }
                                            }

                                            if (ParseIFF(iff,IFFPARSE_SCAN))
                                                break;
                                        }
                                    }
                                }
                            }
                            CloseIFF(iff);
                        }
                        Close(iff->iff_Stream);
                    }
                    FreeIFF(iff);
                }
                CloseLibrary(i);
            }
            FreeMem(cu,sizeof(struct ConfigUser));
        }
        FreeMem(cs,sizeof(struct ConfigStruct));
    }
    return(TRUE);
}




BOOL SafeExamine(BPTR alock, struct FileInfoBlock *f)
{

    BPTR newlock;
    BOOL status;

    newlock = DupLock(alock);
    if ((newlock) || ((!newlock) && (!IoErr())))
    {
        status = Examine(newlock,f);
        UnLock(newlock);
        return(status);
    }
    else
    {
        return(FALSE);
    }

}

ULONG PermFromLock(struct FileLock *l)
{
    struct FileInfoBlock *myfib;
    ULONG notag[2]={TAG_DONE,0};
    ULONG permissions=-1L;

    myfib = (struct FileInfoBlock *) AllocDosObject(DOS_FIB,(struct TagItem *) &notag);
    if (myfib)
    {
        if (SafeExamine((BPTR)l, myfib))
            permissions = myfib->fib_Protection;
        FreeDosObject(DOS_FIB,myfib);
    }
    return(permissions);
}


UWORD UIDFromLock(struct FileLock *l)
{
    struct FileInfoBlock *myfib;
    ULONG notag[2]={TAG_DONE,0};
    UWORD userid;

    myfib = (struct FileInfoBlock *) AllocDosObject(DOS_FIB,(struct TagItem *) &notag);
    if (myfib)
    {
        if (SafeExamine((BPTR)l, myfib))
        {
            userid = myfib->fib_OwnerUID;
        }
        FreeDosObject(DOS_FIB,myfib);
    }
    return (userid);
}

UWORD GIDFromLock(struct FileLock *l)
{
    struct FileInfoBlock *myfib;
    UWORD groupid;
    ULONG notag[2]={TAG_DONE,0};

    myfib = (struct FileInfoBlock *) AllocDosObject(DOS_FIB,(struct TagItem *) &notag);
    if (myfib)
    {
        if (SafeExamine((BPTR)l, myfib))
            groupid = myfib->fib_OwnerGID;
        FreeDosObject(DOS_FIB,myfib);
    }
    return (groupid);
}


ULONG BestPermFromLock(struct FileLock *l, struct MountedFSInfo *m)
{
    return(BestPermFromLockX(l,m,0,0));
}

ULONG BestPermFromLockX(struct FileLock *l, struct MountedFSInfo *m, ULONG *flags, ULONG *map)
{
    struct FileInfoBlock *myfib;
    UWORD groupid, ownerid;
    ULONG notag[2]={TAG_DONE,0};
    ULONG pbits;

    myfib = (struct FileInfoBlock *) AllocDosObject(DOS_FIB,(struct TagItem *) &notag);
    if (myfib)
    {
        BPTR oneback;
        oneback = ParentDir((BPTR)l);
        if (oneback)
            UnLock(oneback);
        if (SafeExamine((BPTR)l, myfib))
        {
            groupid = myfib->fib_OwnerGID;
            ownerid = myfib->fib_OwnerUID;
            pbits = myfib->fib_Protection;
	    pbits &= ALLBITS;			// mask off bits other than protection bits
            if (map)
                *map = pbits;

            if (!oneback)
            {
                FreeDosObject(DOS_FIB,myfib);
                return(15);
            }

            /* If they're the administrator, they're the owner */
	// If it has no group or ownerid, they're NOT the owner!
            if ((m->mfsi_UserFlags & UFLAGF_AdminAll) /*|| (!ownerid) || (!groupid)*/)
            {
                ULONG pc;

                pc = (~pbits & OWNERBITS);
                FreeDosObject(DOS_FIB,myfib);
                if (flags)
                    *flags |= BPF_USER;
                return(pc);
            }

            /* If they're the owner */
            if (ownerid == m->mfsi_UID)
            {
                pbits = ((~pbits) & OWNERBITS);
                if (flags)
                    *flags |= BPF_USER;
            }
            else if (groupid == m->mfsi_GID)
            {
                pbits = ((pbits & GROUPBITS) >> FIBB_GRP_DELETE);
                if (flags)
                    *flags |= BPF_GROUP;
            }
            else
            {
                struct UserInfo *ui;
                struct GroupInfo *gi;
                BOOL good=FALSE;
                ui = AllocUserInfo();
                if (ui)
                {
                    gi = AllocGroupInfo();
                    if (gi)
                    {
                        if (!IDToUser(m->mfsi_UID,ui))
                        {
                            if (!IDToGroup(groupid,gi))
                            {
                                if (!MemberOf(gi,ui))
                                {
                                    good=TRUE;
                                    if (flags)
                                        *flags |= BPF_GROUP;
                                }
                            }
                        }
                        FreeGroupInfo(gi);
                    }
                    FreeUserInfo(ui);
                }
                if (!good)
                {
                    pbits = ((pbits & OTHERBITS) >> FIBB_OTR_DELETE);
                    if (flags)
                        *flags |= BPF_OTHER;

                }
                else
                {
                    pbits = ((pbits & GROUPBITS) >> FIBB_GRP_DELETE);
                }

            }
// huh?????!!!!! FIX????!!!!
            if (myfib->fib_DirEntryType > 0)        /* If this lock references a DIRECTORY, set the high bit */
                pbits |= (1 << 31);
        }
        FreeDosObject(DOS_FIB,myfib);
    }

    return (pbits);
}

ULONG BestPermShared(ULONG groupid, ULONG ownerid, ULONG pbits,ULONG type,
		     struct MountedFSInfo *m, ULONG *flags);

ULONG BestPerm(struct FileInfoBlock *myfib, struct MountedFSInfo *m)
{
    return(BestPermX(myfib,m,0));
}

ULONG BestPermX(struct FileInfoBlock *myfib, struct MountedFSInfo *m, ULONG *flags)
{
    return BestPermShared(myfib->fib_OwnerGID,myfib->fib_OwnerUID,
			  myfib->fib_Protection,myfib->fib_DirEntryType,m,flags);
}

ULONG BestPermED(struct ExAllData *ed, ULONG type, struct MountedFSInfo *m, ULONG *flags)
{
    ULONG gid = 0, uid = 0;

    if (type >= ED_OWNER)
    {
	gid = ed->ed_OwnerGID;
	uid = ed->ed_OwnerUID;
    }

    return BestPermShared(gid,uid,ed->ed_Prot,ed->ed_Type,m,flags);
}

ULONG BestPermShared(ULONG groupid, ULONG ownerid, ULONG pbits,ULONG type,
		     struct MountedFSInfo *m, ULONG *flags)
{
    /* If they're the administrator, they're the same as the owner */
    /* Ditto if the FIB is NO OWNER or NO GROUP */
    // If it has no group or ownerid, they're NOT the owner!
    if ((m->mfsi_UserFlags & UFLAGF_AdminAll) /*|| (!ownerid) || (!groupid)*/)
    {
        ULONG pc;
// FIX!!!
        pc = (pbits & ~OWNERBITS) | ((~pbits) & OWNERBITS);
        if (flags)
            *flags |= BPF_USER;
        return(pc);
    }

    /* If they're the owner */
    if (ownerid == m->mfsi_UID)
    {
        pbits = ((~pbits) & OWNERBITS);
        if (flags)
            *flags |= BPF_USER;
    }
    else if (groupid == m->mfsi_GID)
    {
        pbits = (((pbits) & GROUPBITS) >> FIBB_GRP_DELETE);
        if (flags)
        {
            *flags |= BPF_GROUP;
        }
    }
    else
    {
        struct UserInfo *ui;
        struct GroupInfo *gi;
        BOOL good=FALSE;
        ui = AllocUserInfo();
        if (ui)
        {
            gi = AllocGroupInfo();
            if (gi)
            {
                if (!IDToUser(m->mfsi_UID,ui))
                {
                    if (!IDToGroup(groupid,gi))
                    {
                        if (!MemberOf(gi,ui))
                        {
                            good=TRUE;
                        }
                    }
                }
                FreeGroupInfo(gi);
            }
            FreeUserInfo(ui);
        }
        if (!good)
        {
            pbits = (((pbits) & OTHERBITS) >> FIBB_OTR_DELETE);
            if (flags)
                *flags |= BPF_OTHER;
        }
        else
        {
            pbits = (((pbits) & GROUPBITS) >> FIBB_GRP_DELETE);
            if (flags)
                *flags |= BPF_GROUP;
        }
    }
    if (type > 0)        /* If this lock references a DIRECTORY, set the high bit */
        pbits |= (1 << 31);

    return(pbits);
}

void __asm SetFSMode(register __d0 ULONG mode)
{
    struct MountedFSInfo *m;
    struct List *l=(struct List *)&FSDBase->FSD_Current;
    struct SignalSemaphore *ss=&FSDBase->FSD_CurrentLock;
    ObtainSemaphore(ss);
    FSDBase->FSD_Mode = mode;
    while (m= (struct MountedFSInfo *) RemHead(l))
        CleanupDeadMount(m);
    ReleaseSemaphore(ss);

}


