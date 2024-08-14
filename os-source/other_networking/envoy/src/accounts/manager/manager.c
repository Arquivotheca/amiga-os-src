#include <exec/types.h>
#include <exec/memory.h>

#include <dos/dos.h>
#include <dos/rdargs.h>
#include <libraries/iffparse.h>
#include <envoy/nipc.h>
#include <envoy/errors.h>
#include <envoy/accounts.h>
#include <workbench/startup.h>
#include "/transactions.h"
#include "/accountsinternal.h"
#include "manager_internal.h"
#include "manager_rev.h"
#include "string.h"

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/nipc_protos.h>
#include <clib/utility_protos.h>
#include <clib/icon_protos.h>

#ifndef PROTOS
#include "manager_protos.h"
#endif

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/iffparse_pragmas.h>
#include <pragmas/nipc_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/icon_pragmas.h>

#define SysBase (*(struct Library **)4L)
#define TEMPLATE	"InitDB/S,UserDB/K,GroupDB/K"

#define getreg __builtin_getreg
extern long getreg __ARGS((int));

UBYTE VersionTag[] = VERSTAG;
UBYTE DefUserDB[] = "PROGDIR:UserData";
UBYTE DefGroupDB[] = "PROGDIR:GroupData";

extern void kprintf(STRPTR fmt, ...);

ULONG __saveds server(void)
{
    ServerDataPtr sd;
    struct RDArgs *rdargs=NULL;
    struct RDArgs *myRDArgs = NULL;
    struct WBStartup *wbmsg = NULL;
    struct Process *myproc;
    struct MsgPort *amport;

    ULONG rc;

    ULONG args[3];
    BOOL error = FALSE;

    myproc = (struct Process *)FindTask(NULL);
    if(!myproc->pr_CLI)
    {
    	WaitPort(&myproc->pr_MsgPort);
    	wbmsg = (struct WBStartup *)GetMsg(&myproc->pr_MsgPort);
    }

    Forbid();
    if(amport = FindPort("Accounts Manager"))
    {
    	Signal(amport->mp_SigTask,SIGBREAKF_CTRL_F);
    	amport = NULL;
    }
    else
    {
    	amport = CreateMsgPort();
    	amport->mp_Node.ln_Name = "Accounts Manager";
    	amport->mp_Node.ln_Pri = -128;
    	AddPort(amport);
    }
    Permit();

    if(amport)
    {
        if(sd = (ServerDataPtr) AllocMem(sizeof(struct ServerData),MEMF_CLEAR|MEMF_PUBLIC))
        {
            sd->sd_SysBase = SysBase;
//          sd->sd_WBStartup = wbmessage;
            stccpy(sd->sd_UserDBName,DefUserDB,256);
            stccpy(sd->sd_GroupDBName,DefGroupDB,256);

            DOSBase = OpenLibrary("dos.library",37L);
            NIPCBase = OpenLibrary("nipc.library",0L);
            UtilityBase = OpenLibrary("utility.library",37L);


            if((DOSBase) && (NIPCBase) && (UtilityBase))
            {
                args[0] = args[1] = args[2] = 0;


                /* Way-K00L Workbench Icon handling routine by Mike Sinz */

                if (wbmsg)
                {
                struct  Library *IconBase;
                struct  WBArg   *wbArg;

                        /*
                         * Started from Workbench so do icon magic...
                         *
                         * What we will do here is try all of the tooltypes
                         * in the icon and keep only those which do not cause
                         * errors in the RDArgs.
                         */
                        if (wbArg=wbmsg->sm_ArgList) if (IconBase=OpenLibrary("icon.library",0))
                        {
                        struct  DiskObject      *diskObj;
                                BPTR            tmplock;

                                /*
                                 * Use project icon if it is there...
                                 */
                                if (wbmsg->sm_NumArgs > 1) wbArg++;

                                tmplock=CurrentDir(wbArg->wa_Lock);
                                if (diskObj=GetDiskObject(wbArg->wa_Name))
                                {
                                char    **ToolTypes;

                                        if (ToolTypes=diskObj->do_ToolTypes)
                                        {
                                        char    *TotalString;
                                        ULONG   totalSize=3;

                                                while (*ToolTypes)
                                                {
                                                        totalSize+=strlen(*ToolTypes)+1;
                                                        ToolTypes++;
                                                }

                                                if (TotalString=AllocVec(totalSize,MEMF_PUBLIC))
                                                {
                                                char    *CurrentPos=TotalString;
                                                ULONG   currentLength;

                                                        ToolTypes=diskObj->do_ToolTypes;
                                                        do
                                                        {
                                                                *CurrentPos='\0';
                                                                if (*ToolTypes) strcpy(CurrentPos,*ToolTypes);
                                                                currentLength=strlen(CurrentPos);
                                                                CurrentPos[currentLength+0]='\n';
                                                                CurrentPos[currentLength+1]='\0';

                                                                if (rdargs) FreeArgs(rdargs);
                                                                rdargs=NULL;
                                                                memset((char *)args, 0, sizeof(args));

                                                                if (myRDArgs) FreeDosObject(DOS_RDARGS,myRDArgs);
                                                                if (myRDArgs=AllocDosObject(DOS_RDARGS,NULL))
                                                                {
                                                                        myRDArgs->RDA_Source.CS_Buffer=TotalString;
                                                                        myRDArgs->RDA_Source.CS_Length=strlen(TotalString);

                                                                        if (rdargs=ReadArgs(TEMPLATE, args, myRDArgs))
                                                                        {
                                                                                CurrentPos[currentLength]=' ';
                                                                                CurrentPos+=currentLength+1;
                                                                        }
                                                                }
                                                        } while (*ToolTypes++);
                                                        FreeVec(TotalString);
                                                }
                                        }
                                        FreeDiskObject(diskObj);
                                }

                                CurrentDir(tmplock);
                                CloseLibrary(IconBase);
                        }
                        rc=RETURN_OK;

                        if(args[1])
                            stccpy(sd->sd_UserDBName,(STRPTR)args[1],256);

                        if(args[2])
                            stccpy(sd->sd_GroupDBName,(STRPTR)args[2],256);

                }
                else if(rdargs = ReadArgs(TEMPLATE,(LONG *)args,NULL))
                {
                    if(args[1])
                    	stccpy(sd->sd_UserDBName,(STRPTR)args[1],256);

                    if(args[2])
                    	stccpy(sd->sd_GroupDBName,(STRPTR)args[2],256);

                    FreeArgs(rdargs);
                }
                NewList((struct List *)&sd->sd_UserList);
                NewList((struct List *)&sd->sd_GroupList);

                ReadUserData(sd);
                ReadGroupData(sd);

                if(args[0])
                {
                    UserData ud;

                    FlushUserData(sd, FALSE);
                    strcpy(sd->sd_UserHeader.AdminPassword,"commodore");
                    strcpy(ud.UserName,"Admin");
                    strcpy(ud.Password,"Admin");
                    sd->sd_GroupHeader.NextGroupID = 1;
                    sd->sd_UserHeader.NextUserID = 1;
                    ud.PrimaryGroupID = 0;
                    ud.Flags = UFLAGF_AdminName | UFLAGF_AdminPassword | UFLAGF_AdminGroups | UFLAGF_AdminAll;

                    if(!InternalAddUser(sd, &ud))
                    {
                        error = TRUE;
                        PutStr("Error! Couldn't initialize database!");
                    }
                    else
                    {
                        WriteUserData(sd);
                        WriteGroupData(sd);
                    }
                }
                if(!error && !args[0])
                {
                    ProcessRequests(sd);
                    FlushUserData(sd, TRUE);
                }
            }
            if (myRDArgs) FreeDosObject(DOS_RDARGS,myRDArgs);
            if(DOSBase) CloseLibrary(DOSBase);
            if(NIPCBase) CloseLibrary(NIPCBase);
            if(UtilityBase) CloseLibrary(UtilityBase);
        }
        RemPort(amport);
        DeleteMsgPort(amport);
    }
    if(wbmsg)
    {
    	Forbid();
    	ReplyMsg((struct Message *)wbmsg);
    }
    return(0L);
}

#undef SysBase
#define SysBase	sd->sd_SysBase;

VOID ReadUserData(ServerDataPtr sd)
{
    struct Library *IFFParseBase;
    struct IFFHandle *iff;
    BPTR iffstream;
    struct ContextNode *cn;
    LONG error;
    UserNode *newuser;

    if(IFFParseBase = OpenLibrary("iffparse.library",37L))
    {
    	if(iffstream=Open(sd->sd_UserDBName,MODE_OLDFILE))
	{
	    if(iff = AllocIFF())
	    {
		InitIFFasDOS(iff);

		if(!OpenIFF(iff,IFFF_READ))
		{
		    iff->iff_Stream = iffstream;

		    while(TRUE)
		    {
			error =	ParseIFF(iff, IFFPARSE_STEP);

			if(!error)
			{
			    if(cn = CurrentChunk(iff))
			    {
			    	if((cn->cn_Type == ID_UDAT) && (cn->cn_ID == ID_UHDR))
			    	{
			    	    ReadChunkBytes(iff, &sd->sd_UserHeader, sizeof(struct UserIFFHeader));
			    	}
				if((cn->cn_Type	== ID_UDAT) && (cn->cn_ID == ID_USER))
				{
				    if(newuser = (struct UserNode *) AllocMem(sizeof(struct UserNode),MEMF_PUBLIC|MEMF_CLEAR))
				    {
					if(ReadChunkBytes(iff, &newuser->Data, sizeof(struct UserData))	== sizeof(struct UserData))
					{
					    AddTail((struct List *)&sd->sd_UserList,(struct Node *)newuser);
					}
				    }
				}
			    }
			}
			else if(error == IFFERR_EOC)
			    continue;
			else
			    break;
		    }
		    CloseIFF(iff);
		}
		FreeIFF(iff);
	    }
	    Close(iffstream);
	}
	CloseLibrary(IFFParseBase);
    }
}

VOID WriteUserData(ServerDataPtr sd)
{
    struct Library *IFFParseBase;
    struct IFFHandle *iff;
    struct UserNode *user;
    BPTR iffstream;

    if(IFFParseBase = OpenLibrary("iffparse.library",37L))
    {
    	if(iffstream = Open(sd->sd_UserDBName,MODE_NEWFILE))
    	{
    	    if(iff = AllocIFF())
    	    {
    	    	InitIFFasDOS(iff);

    	    	if(!OpenIFF(iff,IFFF_WRITE))
    	    	{
    	    	    iff->iff_Stream = iffstream;

    	    	    PushChunk(iff, ID_UDAT, ID_FORM, IFFSIZE_UNKNOWN);

		    PushChunk(iff, ID_UDAT, ID_UHDR, sizeof(struct UserIFFHeader));
		    WriteChunkBytes(iff, &sd->sd_UserHeader, sizeof(struct UserIFFHeader));
		    PopChunk(iff);

    	    	    user = (UserNode *) sd->sd_UserList.mlh_Head;

    	    	    while(user->Link.ln_Succ)
    	    	    {
    	    	    	PushChunk(iff, ID_UDAT, ID_USER, sizeof(struct UserIFFChunk));

    	    	    	WriteChunkBytes(iff, &user->Data, sizeof(struct UserIFFChunk));

    	    	    	PopChunk(iff);

    	    	    	user = (UserNode *)user->Link.ln_Succ;
    	    	    }

    	    	    PopChunk(iff);

    	    	    CloseIFF(iff);
    	    	}
    	    	FreeIFF(iff);
    	    }
    	    Close(iffstream);
    	}
    	CloseLibrary(IFFParseBase);
    }
}

VOID ReadGroupData(ServerDataPtr sd)
{
    struct Library *IFFParseBase;
    struct IFFHandle *iff;
    BPTR iffstream;
    struct ContextNode *cn;
    LONG error;
    GroupNode *newgroup;
    MinUserNode *minusernode;
    UserNode *user;

    if(IFFParseBase = OpenLibrary("iffparse.library",37L))
    {
    	if(iffstream=Open(sd->sd_GroupDBName,MODE_OLDFILE))
	{
	    if(iff = AllocIFF())
	    {
		InitIFFasDOS(iff);

		if(!OpenIFF(iff,IFFF_READ))
		{
		    iff->iff_Stream = iffstream;

		    while(TRUE)
		    {
			error =	ParseIFF(iff, IFFPARSE_STEP);

			if(!error)
			{
			    if(cn = CurrentChunk(iff))
			    {
			    	if((cn->cn_Type == ID_GDAT) && (cn->cn_ID == ID_GHDR))
			    	{
			    	    ReadChunkBytes(iff, &sd->sd_GroupHeader, sizeof(struct GroupIFFHeader));
			    	}
				else if((cn->cn_Type == ID_GDAT) && (cn->cn_ID == ID_GRUP))
				{
				    if(newgroup = (struct GroupNode *) AllocMem(sizeof(struct GroupNode),MEMF_PUBLIC|MEMF_CLEAR))
				    {
					if(ReadChunkBytes(iff, &newgroup->Data, sizeof(struct GroupData)) == sizeof(struct GroupData))
					{
				    	    NewList((struct List *)&newgroup->UserList);

					    while(cn->cn_Size - cn->cn_Scan)
					    {
					        UWORD uid;

				    	        ReadChunkBytes(iff, &uid, sizeof(UWORD));

				    	        if(user = FindUserByUID(sd, uid))
				    	        {
					    	    if(minusernode = (MinUserNode *)AllocMem(sizeof(MinUserNode),MEMF_CLEAR|MEMF_PUBLIC))
					    	    {
					    	    	minusernode->User = user;
					    	    	AddTail((struct List *)&newgroup->UserList, (struct Node *)minusernode);
					    	    }
						}
					    }
					    AddTail((struct List *)&sd->sd_GroupList,(struct Node *)newgroup);
					}
				    }
				}
			    }
			}
			else if(error == IFFERR_EOC)
			    continue;
			else
			    break;
		    }
		    CloseIFF(iff);
		}
		FreeIFF(iff);
	    }
	    Close(iffstream);
	}
	CloseLibrary(IFFParseBase);
    }
}

VOID WriteGroupData(ServerDataPtr sd)
{
    struct Library *IFFParseBase;
    struct IFFHandle *iff;
    struct GroupNode *group;
    struct MinUserNode *minuser;
    BPTR iffstream;

    if(IFFParseBase = OpenLibrary("iffparse.library",37L))
    {
    	if(iffstream = Open(sd->sd_GroupDBName,MODE_NEWFILE))
    	{
    	    if(iff = AllocIFF())
    	    {
    	    	InitIFFasDOS(iff);

    	    	if(!OpenIFF(iff,IFFF_WRITE))
    	    	{
    	    	    iff->iff_Stream = iffstream;

    	    	    PushChunk(iff, ID_GDAT, ID_FORM, IFFSIZE_UNKNOWN);

		    PushChunk(iff, ID_GDAT, ID_GHDR, sizeof(struct GroupIFFHeader));
		    WriteChunkBytes(iff, &sd->sd_GroupHeader, sizeof(struct GroupIFFHeader));
		    PopChunk(iff);

    	    	    group = (GroupNode *) sd->sd_GroupList.mlh_Head;

    	    	    while(group->Link.ln_Succ)
    	    	    {
    	    	    	PushChunk(iff, ID_GDAT, ID_GRUP, IFFSIZE_UNKNOWN);

    	    	    	WriteChunkBytes(iff, &group->Data, sizeof(struct GroupIFFChunk));

    	    	    	minuser = (MinUserNode *)group->UserList.mlh_Head;

    	    	    	while(minuser->Link.mln_Succ)
    	    	    	{
    	    	    	    WriteChunkBytes(iff, &minuser->User->Data.UserID, sizeof(UWORD));

    	    	    	    minuser = (MinUserNode *)minuser->Link.mln_Succ;
    	    	    	}

    	    	    	PopChunk(iff);

    	    	    	group = (GroupNode *)group->Link.ln_Succ;
    	    	    }

    	    	    PopChunk(iff);

    	    	    CloseIFF(iff);
    	    	}
    	    	FreeIFF(iff);
    	    }
    	    Close(iffstream);
    	}
    	CloseLibrary(IFFParseBase);
    }
}

VOID FlushUserData(ServerDataPtr sd, BOOL flush)
{
    UserNode *user;
    GroupNode *group;
    MinUserNode *minuser;

    if(flush)
    {
	WriteUserData(sd);
	WriteGroupData(sd);
    }

    while(user = (UserNode *) RemHead((struct List *)&sd->sd_UserList))
    	FreeMem(user,sizeof(UserNode));

    while(group = (GroupNode *) RemHead((struct List *)&sd->sd_GroupList))
    {
    	while(minuser = (MinUserNode *)RemHead((struct List *)&group->UserList))
    	    FreeMem(minuser,sizeof(MinUserNode));

    	FreeMem(group,sizeof(GroupNode));
    }
}

struct Entity *CreateEnt(ServerDataPtr sd, Tag firsttag, ...)
{
    return(CreateEntityA((struct TagItem *)&firsttag));
}

VOID ProcessRequests(ServerDataPtr sd)
{
    struct Entity *entity;
    ULONG mySigBit, waitmask, signals;
    struct Transaction *trans;

    if(entity = CreateEnt(sd, ENT_Name,	"Accounts Server",
    			     ENT_Public, TRUE,
    			     ENT_AllocSignal, &mySigBit,
    			     TAG_DONE))
    {
    	waitmask = (1 << mySigBit) | SIGBREAKF_CTRL_F;

    	while(TRUE)
    	{
    	    signals = Wait(waitmask);

    	    if(signals & SIGBREAKF_CTRL_F)
    	    	break;

    	    while(trans = GetTransaction(entity))
    	    {
    	    	switch(trans->trans_Command)
    	    	{
    	    	    case ACMD_AddUser:		AddUser(sd, trans);
	    	    	    			break;

    	    	    case ACMD_RemUser:		RemUser(sd, trans);
    	    	    				break;

    	    	    case ACMD_AddGroup:		AddGroup(sd, trans);
    	    	    				break;

    	    	    case ACMD_RemGroup:		RemGroup(sd, trans);
    	    	    				break;

		    case ACMD_AddMember:	AddMember(sd, trans);
		    				break;

    	    	    case ACMD_RemoveMember:     RemoveMember(sd, trans);
    	    	    				break;

    	    	    case ACMD_NextUser:		NextUser(sd, trans);
    	    	    				break;

    	    	    case ACMD_NextMember:	NextMember(sd, trans);
    	    	    				break;

    	    	    case ACMD_NextGroup:	NextGroup(sd, trans);
    	    	    				break;

    	    	    case ACMD_IDToUser:		IDToUser(sd, trans);
    	    	    				break;

    	    	    case ACMD_NameToUser:	NameToUser(sd, trans);
    	    	    				break;

    	    	    case ACMD_IDToGroup:	IDToGroup(sd, trans);
    	    	    				break;

    	    	    case ACMD_NameToGroup:	NameToGroup(sd, trans);
    	    	    				break;

    	    	    case ACMD_VerifyUser:	VerifyUser(sd, trans);
    	    	    				break;

		    case ACMD_VerifyUserCrypt:	VerifyUserCrypt(sd, trans);
		    				break;

    	    	    case ACMD_AdminUser:	AdminUser(sd, trans);
    	    	    				break;

    	    	    case ACMD_ModifyGroup:	ModifyGroup(sd, trans);
    	    	    				break;

    	    	    case ACMD_MemberOf:		MemberOf(sd, trans);
						break;

    	    	    default:			trans->trans_Error = ENVOYERR_CMDUNKNOWN;
    	    	    				break;
    	    	}
    	    	trans->trans_RespDataActual = trans->trans_ReqDataActual;
    	    	ReplyTransaction(trans);
    	    }
    	    if(sd->sd_FlushUserDat)
    	    {
    	    	sd->sd_FlushUserDat = FALSE;
    	    	WriteUserData(sd);
    	    }
    	    if(sd->sd_FlushGroupDat)
    	    {
    	    	sd->sd_FlushGroupDat = FALSE;
    	    	WriteGroupData(sd);
    	    }
    	}
    	DeleteEntity(entity);
    }
}

VOID AddUser(ServerDataPtr sd, struct Transaction *trans)
{
    AdminUserReq *aur;
    UserNode *authority;

    aur = (AdminUserReq *)trans->trans_RequestData;

    if(authority = VerifyAuthority(sd, &aur->Authority))
    {
    	if(authority->Data.Flags & UFLAGF_AdminAll)
    	{
    	    if(InternalAddUser(sd,  &aur->User))
    	    {
    	    	trans->trans_Error = 0;
    	    }
    	    else
    	    {
    	    	trans->trans_Error = ENVOYERR_NORESOURCES;
    	    }
    	}
    	else
    	{
    	    trans->trans_Error = ACCERROR_NOPRIVS;
    	}
    }
    else
    {
    	trans->trans_Error = ACCERROR_NOAUTHORITY;
    }
}

BOOL InternalAddUser(ServerDataPtr sd, UserData *user)
{
    UserNode *newuser;
    BOOL result = FALSE;

    if(sd->sd_UserHeader.NextUserID <= 65534)
    {
    	if(newuser = AllocMem(sizeof(UserNode),MEMF_CLEAR|MEMF_PUBLIC))
	{
	    newuser->Link.ln_Name = (STRPTR) &newuser->Data;
    	    user->UserID = newuser->Data.UserID = sd->sd_UserHeader.NextUserID++;
    	    newuser->Data.PrimaryGroupID = user->PrimaryGroupID;
    	    newuser->Data.Flags = user->Flags;
    	    stccpy(newuser->Data.UserName,user->UserName,32);
    	    stccpy(newuser->Data.Password,user->Password,32);
	    AddUserSorted(sd, &sd->sd_UserList, newuser);
	    sd->sd_FlushUserDat = result = TRUE;
	}
    }
    return(result);
}

VOID AddUserSorted(ServerDataPtr sd, struct MinList *list, UserNode *user)
{
    UserNode *current;

    current = (UserNode *)list->mlh_TailPred;

    while(current->Link.ln_Pred)
    {
    	if(user->Data.UserID > current->Data.UserID)
    	{
    	    Insert((struct List *)list, (struct Node *)user, (struct Node *)current);
    	    break;
    	}
    	current = (UserNode *)current->Link.ln_Pred;
    }
    if(!current->Link.ln_Pred)
    {
    	AddHead((struct List *)list, (struct Node *)user);
    }
}

VOID RemUser(ServerDataPtr sd, struct Transaction *trans)
{
    AdminUserReq *aur;
    UserNode *authority;

    aur = (AdminUserReq *)trans->trans_RequestData;

    if(authority = VerifyAuthority(sd, &aur->Authority))
    {
    	if(authority->Data.Flags & UFLAGF_AdminAll)
    	{
    	    if(InternalRemUser(sd, &aur->User))
    	    {
    	    	trans->trans_Error = 0;
    	    }
    	    else
    	    {
    	    	trans->trans_Error = ENVOYERR_UNKNOWNUSER;
    	    }
    	}
    	else
    	{
    	    trans->trans_Error = ACCERROR_NOPRIVS;
    	}
    }
    else
    {
    	trans->trans_Error = ACCERROR_NOAUTHORITY;
    }
}

BOOL InternalRemUser(ServerDataPtr sd, UserData *user)
{
    UserNode *olduser;
    GroupNode *group;
    BOOL result = FALSE;

    olduser = FindUserByName(sd, user->UserName);

    if(!olduser)
    	FindUserByUID(sd, user->UserID);

    if(olduser && olduser->Data.UserID)
    {
    	Remove((struct Node *)olduser);

    	group = (GroupNode *)sd->sd_GroupList.mlh_Head;

    	while(group->Link.ln_Succ)
    	{
    	    InternalRemUserFromGroup(sd, group, olduser);

    	    group = (GroupNode *)group->Link.ln_Succ;
    	}
    	sd->sd_FlushUserDat = result = TRUE;
    }
    return(result);
}

VOID InternalRemUserFromGroup(ServerDataPtr sd, GroupNode *group, UserNode *user)
{
    MinUserNode *minuser;

    minuser = (MinUserNode *)group->UserList.mlh_Head;

    while(minuser->Link.mln_Succ)
    {
    	if(minuser->User == user)
    	{
    	    Remove((struct Node *)minuser);
    	    FreeMem(minuser,sizeof(MinUserNode));
    	    break;
    	}
    	minuser = (MinUserNode *)minuser->Link.mln_Succ;
    }
}

UserNode *FindUserByName(ServerDataPtr sd, STRPTR name)
{
    UserNode *user;

    user = (UserNode *)sd->sd_UserList.mlh_Head;

    while(user->Link.ln_Succ)
    {
    	if(!Stricmp(name,user->Data.UserName))
    	    return(user);

    	user = (UserNode *) user->Link.ln_Succ;
    }
    return(NULL);
}

UserNode *FindUserByUID(ServerDataPtr sd, UWORD uid)
{
    UserNode *user;

    user = (UserNode *)sd->sd_UserList.mlh_Head;

    while(user->Link.ln_Succ)
    {
    	if(uid == user->Data.UserID)
    	    return(user);

    	user = (UserNode *) user->Link.ln_Succ;
    }
    return(NULL);
}

GroupNode *FindGroupByName(ServerDataPtr sd, STRPTR name)
{
    GroupNode *group;

    group = (GroupNode *)sd->sd_GroupList.mlh_Head;

    while(group->Link.ln_Succ)
    {
    	if(!Stricmp(name,group->Data.GroupName))
    	    break;

    	group = (GroupNode *) group->Link.ln_Succ;
    }
    if(!group->Link.ln_Succ)
    	group = NULL;

    return(group);
}

GroupNode *FindGroupByGID(ServerDataPtr sd, UWORD gid)
{
    GroupNode *group;

    group = (GroupNode *)sd->sd_GroupList.mlh_Head;

    while(group->Link.ln_Succ)
    {
    	if(gid == group->Data.GroupID)
    	    break;

    	group = (GroupNode *) group->Link.ln_Succ;
    }
    if(!group->Link.ln_Succ)
    	group = NULL;

    return(group);
}


VOID NextUser(ServerDataPtr sd, struct Transaction *trans)
{
    UserNode *user;

    UserData *userdata;

    userdata = (UserData *)trans->trans_RequestData;

    user = (UserNode *)sd->sd_UserList.mlh_Head;

    while(user->Link.ln_Succ)
    {
	if(user->Data.UserID > userdata->UserID)
	{
	    CopyMem(&user->Data,userdata,sizeof(UserData));
	    trans->trans_Error = 0;
	    break;
	}
	user = (UserNode *)user->Link.ln_Succ;
    }
    if(!user->Link.ln_Succ)
	trans->trans_Error = ENVOYERR_LASTUSER;
}

VOID IDToUser(ServerDataPtr sd, struct Transaction *trans)
{
    UserNode *user;
    UserData *userdata;

    userdata = (UserData *)trans->trans_RequestData;

    if(user = FindUserByUID(sd, userdata->UserID))
    {
    	CopyMem(&user->Data,userdata,sizeof(UserData));
    	trans->trans_Error = 0;
    }
    else
    	trans->trans_Error = ENVOYERR_UNKNOWNUSER;
}

VOID NameToUser(ServerDataPtr sd, struct Transaction *trans)
{
    UserNode *user;
    UserData *userdata;

    userdata = (UserData *)trans->trans_RequestData;

    if(user = FindUserByName(sd, userdata->UserName))
    {
    	CopyMem(&user->Data,userdata,sizeof(UserData));
    	trans->trans_Error = 0;
    }
    else
    	trans->trans_Error = ENVOYERR_UNKNOWNUSER;
}

VOID IDToGroup(ServerDataPtr sd, struct Transaction *trans)
{
    GroupNode *group;
    GroupData *groupdata;

    groupdata = (GroupData *)trans->trans_RequestData;

    if(group = FindGroupByGID(sd, groupdata->GroupID))
    {
    	CopyMem(&group->Data,groupdata,sizeof(GroupData));
    	trans->trans_Error = 0;
    }
    else
    	trans->trans_Error = ENVOYERR_UNKNOWNGROUP;
}

VOID NameToGroup(ServerDataPtr sd, struct Transaction *trans)
{
    GroupNode *group;
    GroupData *groupdata;

    groupdata = (GroupData *)trans->trans_RequestData;

    if(group = FindGroupByName(sd, groupdata->GroupName))
    {
    	CopyMem(&group->Data,groupdata,sizeof(GroupData));
    	trans->trans_Error = 0;
    }
    else
    	trans->trans_Error = ENVOYERR_UNKNOWNGROUP;
}

UserNode *VerifyAuthority(ServerDataPtr sd, AuthorityData *ad)
{
    UserNode *user;

    if(user = FindUserByName(sd, ad->UserName))
    {
    	if(strcmp(user->Data.Password, ad->Password) && (user->Data.Flags & UFLAGF_NeedsPassword))
    	{
    	    user = NULL;
    	}
    }
    return(user);
}

UserNode *InternalVerifyUser(ServerDataPtr sd, UserData *userdata)
{
    UserNode *user;

    if(user = FindUserByName(sd, userdata->UserName))
    {
    	if(strcmp(user->Data.Password, userdata->Password) && (user->Data.Flags & UFLAGF_NeedsPassword))
    	    user = NULL;
    }
    return(user);
}

VOID VerifyUser(ServerDataPtr sd, struct Transaction *trans)
{
    UserNode *user;
    UserData *userdata;

    userdata = (UserData *)trans->trans_RequestData;

    if(user = InternalVerifyUser(sd, userdata))
    {
    	CopyMem(&user->Data,userdata,sizeof(UserData));
    	trans->trans_Error = 0;
    }
    else
    	trans->trans_Error = ENVOYERR_UNKNOWNUSER;
}

UserNode *IntVerifyUserCrypt(ServerDataPtr sd, UserData *userdata)
{
    UserNode *user;
    UBYTE pwbuff[16];

    if(user = FindUserByName(sd, userdata->UserName))
    {
    	eCrypt(sd,pwbuff,user->Data.Password,userdata->UserName);

    	if(strcmp(pwbuff, userdata->Password) && (user->Data.Flags & UFLAGF_NeedsPassword))
    	    user = NULL;
    }
    return(user);
}

VOID VerifyUserCrypt(ServerDataPtr sd, struct Transaction *trans)
{
    UserNode *user;
    UserData *userdata;

    userdata = (UserData *)trans->trans_RequestData;

    if(user = IntVerifyUserCrypt(sd, userdata))
    {
    	CopyMem(&user->Data,userdata,sizeof(UserData));
    	trans->trans_Error = 0;
    }
    else
    	trans->trans_Error = ENVOYERR_UNKNOWNUSER;
}

VOID AdminUser(ServerDataPtr sd, struct Transaction *trans)
{
    AdminUserReq *adminreq;
    UserNode *auth;
    UserNode *user;

    adminreq = (AdminUserReq *)trans->trans_RequestData;

    if(auth = VerifyAuthority(sd, &adminreq->Authority))
    {
    	if(user = FindUserByUID(sd, adminreq->User.UserID))
    	{
    	    trans->trans_Error = ACCERROR_NOPRIVS;
    	    if((auth == user) && (user->Data.Flags & UFLAGF_AdminName))
    	    {
    	        stccpy(user->Data.UserName,adminreq->User.UserName,32);
    	    	sd->sd_FlushUserDat = TRUE;
    	    	trans->trans_Error = 0;
    	    }
    	    if((auth == user) && (user->Data.Flags & UFLAGF_AdminPassword))
    	    {
    	        stccpy(user->Data.Password,adminreq->User.Password,32);
    	    	sd->sd_FlushUserDat = TRUE;
    	    	trans->trans_Error = 0;
    	    }
	    if(auth->Data.Flags & UFLAGF_AdminAll)
	    {
	    	stccpy(user->Data.UserName,adminreq->User.UserName,32);
	    	stccpy(user->Data.Password,adminreq->User.Password,32);
	    	user->Data.UserID = adminreq->User.UserID;
	    	user->Data.PrimaryGroupID = adminreq->User.PrimaryGroupID;
    	        user->Data.Flags = adminreq->User.Flags;
	    	if(user->Data.UserID == 1)
	    	{
       	           user->Data.Flags |= UFLAGF_AdminAll;
    	           adminreq->User.Flags |= UFLAGF_AdminAll;
    	        }
	    	sd->sd_FlushUserDat = TRUE;
	    	trans->trans_Error = 0;
	    }
	}
	else
	    trans->trans_Error = ENVOYERR_UNKNOWNUSER;
    }
    else
    	trans->trans_Error = ACCERROR_NOAUTHORITY;
}

VOID ModifyGroup(ServerDataPtr sd, struct Transaction *trans)
{
    ModifyGroupReq *modreq;
    UserNode *auth;
    GroupNode *group;

    modreq = (ModifyGroupReq *)trans->trans_RequestData;

    if(auth = VerifyAuthority(sd, &modreq->Authority))
    {
    	if(group = FindGroupByGID(sd, modreq->Group.GroupID))
    	{
     	    if((auth->Data.Flags & UFLAGF_AdminAll) || (auth->Data.UserID = group->Data.AdminID))
     	    {
     	    	strcpy(group->Data.GroupName,modreq->Group.GroupName);
     	    	group->Data.AdminID=modreq->Group.AdminID;
     	    	sd->sd_FlushGroupDat = TRUE;
     	    	trans->trans_Error = 0;
     	    }
     	    else
     	    	trans->trans_Error = ACCERROR_NOPRIVS;
     	}
     	else
     	    trans->trans_Error = ENVOYERR_UNKNOWNGROUP;
     }
     else
     	trans->trans_Error = ACCERROR_NOAUTHORITY;
}

VOID AddGroup(ServerDataPtr sd, struct Transaction *trans)
{
    AdminGroupReq *adminreq;
    UserNode *auth;
    GroupNode *group;

    adminreq = (AdminGroupReq *)trans->trans_RequestData;

    if(auth = VerifyAuthority(sd, &adminreq->Authority))
    {
    	if(group = FindGroupByName(sd, adminreq->Group.GroupName))
    	{
    	    trans->trans_Error = ACCERROR_GROUPEXISTS;
    	}
	else if(auth->Data.Flags & (UFLAGF_AdminGroups | UFLAGF_AdminAll))
	{
	    if(sd->sd_GroupHeader.NextGroupID <= 65534)
	    {
    	    	if(group = AllocMem(sizeof(GroupNode),MEMF_CLEAR|MEMF_PUBLIC))
    	    	{
    	    	    NewList((struct List *)&group->UserList);

    	    	    adminreq->Group.GroupID = sd->sd_GroupHeader.NextGroupID++;
    	    	    stccpy(group->Data.GroupName,adminreq->Group.GroupName,32);
    	    	    group->Data.Flags = adminreq->Group.Flags;
    	    	    group->Data.AdminID = adminreq->Group.AdminID;
              	    group->Data.GroupID = adminreq->Group.GroupID;
    	    	    AddGroupSorted(sd, &sd->sd_GroupList, group);
		    sd->sd_FlushGroupDat = TRUE;
		    trans->trans_Error = 0;
		}
		else
		    trans->trans_Error = ENVOYERR_NORESOURCES;
	    }
	    else
	    	trans->trans_Error = ACCERROR_NOFREEGROUPS;
	}
	else
	    trans->trans_Error = ACCERROR_NOPRIVS;
    }
    else
    	trans->trans_Error = ACCERROR_NOAUTHORITY;
}

VOID AddGroupSorted(ServerDataPtr sd, struct MinList *list, GroupNode *group)
{
    GroupNode *current;

    current = (GroupNode *)list->mlh_TailPred;

    while(current->Link.ln_Pred)
    {
    	if(group->Data.GroupID > current->Data.GroupID)
    	{
    	    Insert((struct List *)list, (struct Node *)group, (struct Node *)current);
    	    break;
    	}
    	current = (GroupNode *)current->Link.ln_Pred;
    }
    if(!current->Link.ln_Pred)
    {
    	AddHead((struct List *)list, (struct Node *)group);
    }
}

VOID RemGroup(ServerDataPtr sd, struct Transaction *trans)
{
    AdminGroupReq *adminreq;
    GroupNode *group;
    MinUserNode *minuser;
    UserNode *auth;

    adminreq = (AdminGroupReq *)trans->trans_RequestData;

    if(auth = VerifyAuthority(sd, &adminreq->Authority))
    {
    	group = FindGroupByGID(sd, adminreq->Group.GroupID);
    	if(!group)
    	    group = FindGroupByName(sd, adminreq->Group.GroupName);

    	if(group)
    	{
    	    if((auth->Data.UserID == group->Data.AdminID) || (auth->Data.Flags & UFLAGF_AdminAll))
    	    {
    	    	while(minuser = (MinUserNode *)RemHead((struct List *)&group->UserList))
    	    	    FreeMem(minuser,sizeof(MinUserNode));

    	    	Remove((struct Node *)group);
    	    	FreeMem(group,sizeof(GroupNode));

    	    	sd->sd_FlushGroupDat = TRUE;

    	    	trans->trans_Error = 0;
    	    }
    	    else
    	    	trans->trans_Error = ACCERROR_NOPRIVS;
    	}
    	else
    	    trans->trans_Error = ENVOYERR_UNKNOWNGROUP;
    }
    else
    	trans->trans_Error = ACCERROR_NOAUTHORITY;
}

VOID NextGroup(ServerDataPtr sd, struct Transaction *trans)
{
    GroupData *groupdat;
    GroupNode *group;

    groupdat = (GroupData *)trans->trans_RequestData;

    group = (GroupNode *)sd->sd_GroupList.mlh_Head;

    while(group->Link.ln_Succ)
    {
	if(group->Data.GroupID > groupdat->GroupID)
	{
    	    CopyMem(&group->Data,groupdat,sizeof(GroupData));
	    trans->trans_Error = 0;
	    break;
	}
	group =	(GroupNode *)group->Link.ln_Succ;
    }

    if(!group->Link.ln_Succ)
    	trans->trans_Error = ENVOYERR_LASTGROUP;
}

VOID NextMember(ServerDataPtr sd, struct Transaction *trans)
{
    NextMemberReq *nextmember;
    MinUserNode *minuser;
    GroupNode *group;

    nextmember = (NextMemberReq *)trans->trans_RequestData;

    group = FindGroupByGID(sd, nextmember->Group.GroupID);

    if(group)
    {
	minuser	= (MinUserNode *)group->UserList.mlh_Head;

	while(minuser->Link.mln_Succ)
	{
	    if(minuser->User->Data.UserID > nextmember->User.UserID)
	    {
	        CopyMem(&minuser->User->Data,&nextmember->User,sizeof(UserData));
    	        trans->trans_Error = 0;
		break;
	    }
	    minuser = (MinUserNode *)minuser->Link.mln_Succ;
	}
    	if(!minuser->Link.mln_Succ)
    	    trans->trans_Error = ENVOYERR_LASTMEMBER;
    }
    else
    	trans->trans_Error = ENVOYERR_UNKNOWNGROUP;
}

MinUserNode *FindMember(ServerDataPtr sd, GroupNode *group, UserNode *user)
{
    MinUserNode *current;

    current = (MinUserNode *)group->UserList.mlh_Head;

    while(current->Link.mln_Succ)
    {
    	if(current->User == user)
    	    return(current);

    	current = (MinUserNode *)current->Link.mln_Succ;
    }
    return(NULL);
}

VOID MemberOf(ServerDataPtr sd, struct Transaction *trans)
{
    GroupNode *group;
    UserNode *user;
    NextMemberReq *nextmember;

    nextmember = (NextMemberReq *)trans->trans_RequestData;

    group = FindGroupByGID(sd, nextmember->Group.GroupID);

    if(!group)
    {
    	group=FindGroupByName(sd, nextmember->Group.GroupName);
    }
    if(group)
    {
    	user = FindUserByUID(sd, nextmember->User.UserID);
    	if(!user)
    	    user = FindUserByName(sd, nextmember->User.UserName);

	if(user)
	{
	    if(FindMember(sd, group, user))
	    {
	    	trans->trans_Error = 0;
	    }
	    else
	    {
	    	trans->trans_Error = ENVOYERR_UNKNOWNMEMBER;
	    }
	}
	else
	    trans->trans_Error = ENVOYERR_UNKNOWNUSER;
    }
    else
    	trans->trans_Error = ENVOYERR_UNKNOWNGROUP;
}

VOID NewList(struct List *list)
{
    list->lh_Head = (struct Node *)&list->lh_Tail;
    list->lh_Tail = NULL;
    list->lh_TailPred = (struct Node *)list;
}

VOID AddMember(ServerDataPtr sd, struct Transaction *trans)
{
    GroupNode *group;
    MinUserNode *minuser,*new;
    UserNode *auth,*user;
    ModifyGroupReq *mgr;

    mgr = (ModifyGroupReq *)trans->trans_RequestData;

    group = FindGroupByName(sd,mgr->Group.GroupName);

    if(!group)
    	group = FindGroupByGID(sd,mgr->Group.GroupID);

    if(group)
    {
    	if(auth = VerifyAuthority(sd, &mgr->Authority))
    	{
    	    if((auth->Data.Flags & UFLAGF_AdminAll) || (auth->Data.UserID == group->Data.AdminID))
    	    {
    	    	user = FindUserByUID(sd, mgr->User.UserID);
    	    	if(!user)
    	    	    user = FindUserByName(sd, mgr->User.UserName);

    	    	if(user)
    	    	{
    	    	    if(new = (MinUserNode *) AllocMem(sizeof(MinUserNode),MEMF_CLEAR|MEMF_PUBLIC))
    	    	    {
    	    	    	new->User = user;

    	    	    	minuser = (MinUserNode *)group->UserList.mlh_TailPred;

    	    	    	while(minuser->Link.mln_Pred)
    	    	    	{
    	    	    	    if(user->Data.UserID > minuser->User->Data.UserID)
    	    	    	    {
    	    	    	    	Insert((struct List *)&group->UserList, (struct Node *)new, (struct Node *)minuser);
    	    	    	    	break;
    	    	    	    }
    	    	    	    minuser = (MinUserNode *)minuser->Link.mln_Pred;
    	    	    	}
    	    	    	if(!minuser->Link.mln_Pred)
    	    	    	{
    	                    AddHead((struct List *)&group->UserList, (struct Node *)new);
    	                }
    	                sd->sd_FlushGroupDat = TRUE;

    	            }
    	            else
    	            	trans->trans_Error = ENVOYERR_NORESOURCES;
		}
		else
		    trans->trans_Error = ENVOYERR_UNKNOWNUSER;
	    }
	    else
	    	trans->trans_Error = ACCERROR_NOPRIVS;
	}
	else
	    trans->trans_Error = ACCERROR_NOAUTHORITY;
    }
    else
    	trans->trans_Error = ENVOYERR_UNKNOWNGROUP;

    return;
}

VOID RemoveMember(ServerDataPtr sd, struct Transaction *trans)
{
    GroupNode *group;
    MinUserNode *minuser;
    UserNode *auth,*user;
    ModifyGroupReq *mgr;

    mgr = (ModifyGroupReq *)trans->trans_RequestData;

    group = FindGroupByName(sd,mgr->Group.GroupName);

    if(!group)
    	group = FindGroupByGID(sd,mgr->Group.GroupID);

    if(group)
    {
    	if(auth = VerifyAuthority(sd, &mgr->Authority))
    	{
    	    if((auth->Data.Flags & UFLAGF_AdminAll) || (auth->Data.UserID == group->Data.AdminID))
    	    {
    	    	user = FindUserByUID(sd, mgr->User.UserID);
    	    	if(!user)
    	    	    user = FindUserByName(sd, mgr->User.UserName);

    	    	if(user)
    	    	{
                    minuser = (MinUserNode *)group->UserList.mlh_Head;

                    while(minuser->Link.mln_Succ)
                    {
                        if(user->Data.UserID == minuser->User->Data.UserID)
                        {
                            Remove((struct Node *)minuser);
                            FreeMem(minuser,sizeof(MinUserNode));
                            sd->sd_FlushGroupDat = TRUE;
                            break;
                        }
                        minuser = (MinUserNode *)minuser->Link.mln_Succ;
                    }
                    if(!minuser->Link.mln_Succ)
                    {
                        trans->trans_Error = ENVOYERR_UNKNOWNMEMBER;
                    }
		}
		else
		    trans->trans_Error = ENVOYERR_UNKNOWNUSER;
	    }
	    else
	    	trans->trans_Error = ACCERROR_NOPRIVS;
	}
	else
	    trans->trans_Error = ACCERROR_NOAUTHORITY;
    }
    else
    	trans->trans_Error = ENVOYERR_UNKNOWNGROUP;

    return;
}

/*--------------------------------------------------------------------------*/

#define OSIZE 12

void eCrypt(ServerDataPtr sd, STRPTR buffer, STRPTR password, STRPTR user)
{
	int i ;
	int k ;
	long d1 ;
	unsigned int buf[OSIZE];
        UBYTE username[32],*uptr;
	uptr = username;

        for(i=0; i<32; i++)
        {
            username[i] = ToLower(user[i]);
        }

        for(i = 0; i < OSIZE; i++)
        {
                buf[i] = 'A' + (*password? *password++:i) + (*uptr? *uptr++:i);
        }

        for(i = 0; i < OSIZE; i++)
        {
                for(k = 0; k < OSIZE; k++)
                {
                        buf[i] += buf[OSIZE-k-1];
                        UDivMod32((long)buf[i], 53L) ;
                        d1 = getreg(1) ;
                        buf[i] = (unsigned int)d1 ;
                }
                buffer[i] = buf[i] + 'A' ;
        }
        buffer[OSIZE-1] = 0;
}

