#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/rdargs.h>
#include <envoy/nipc.h>
#include "/accounts.h"
#include "/transactions.h"
#include <string.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/nipc_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/nipc_pragmas.h>

struct Library *DOSBase;
struct Library *NIPCBase;

ULONG __saveds AddMember(VOID)
{
    struct Library *SysBase;
    struct Transaction *trans;
    struct Entity *client;
    struct Entity *server;
    struct RDArgs *rdargs;

    LONG args[6];
    ULONG signal;
    ModifyGroupReq *mgr;
    UWORD i;

    SysBase = (*(struct Library **)4L);

    if(DOSBase = OpenLibrary("dos.library",37L))
    {
    	for(i=0;i<6;i++)
    	    args[i]=0L;

    	if(rdargs = ReadArgs("Authority/A,AuthPW/A,Group=GroupName/K,GID=GroupID/K/N,User=UserName/K,UID=UserID/K/N",args,NULL))
    	{
    	    if((args[2] || args[3]) && (args[4] || args[5]))
    	    {
                if(NIPCBase = OpenLibrary("nipc.library",37L))
                {
                    if(client = CreateEntity(ENT_AllocSignal, &signal,
                                             TAG_DONE))
                    {
                        if(server = FindEntity(NULL,"Accounts Server",client,NULL))
                        {
                            if(trans = AllocTransaction(TRN_AllocReqBuffer,sizeof(ModifyGroupReq),
                                                        TAG_DONE))
                            {
                                trans->trans_ResponseData = trans->trans_RequestData;
                                trans->trans_Command = ACMD_AddMember;

                                mgr = (ModifyGroupReq *)trans->trans_RequestData;

                                stccpy(mgr->Authority.UserName,(STRPTR)args[0],32);
                                stccpy(mgr->Authority.Password,(STRPTR)args[1],32);

                                if(args[2])
                                    stccpy(mgr->Group.GroupName,(STRPTR)args[2],32);

                                if(args[3])
                                    mgr->Group.GroupID = (UWORD)(*(ULONG *)args[3]);

                                if(args[4])
                                    stccpy(mgr->User.UserName,(STRPTR)args[4],32);

                                if(args[5])
                                    mgr->User.UserID = (UWORD)(*(ULONG *)args[5]);

                                DoTransaction(server,client,trans);

                                if(!trans->trans_Error)
                                {
                                    PutStr("User Added to group.\n");
                                }
                                else
                                {
                                    switch(trans->trans_Error)
                                    {
                                        case ACCERROR_NOPRIVS : PutStr("Error: Insufficient Priviledges.\n");
                                                                break;

                                        case ACCERROR_NOAUTHORITY : PutStr("Error: No authority.\n");
                                                                    break;

                                        case ACCERROR_NORESOURCES : PutStr("Error: No Resources on server.\n");
                                                                    break;

                                        default :   Printf("Unknown Error: %ld",&trans->trans_Error);
                                                    break;
                                    }
                                }
                                FreeTransaction(trans);
                            }
                            else
                                PutStr("Error: Couldn't Allocate Transaction.\n");

                            LoseEntity(server);
                        }
                        else
                            PutStr("Error: Couldn't Connect to Server.\n");

                        DeleteEntity(client);
                    }
                    else
                        PutStr("Error: Couldn't Create Entity.\n");
                }
                else
                    PutStr("Couldn't open nipc.library.\n");
            }
            else
            	PutStr("Must specify groupname/groupid and username/userid.\n");
	}
	else
	    PutStr("Required Parameter(s) missing.\n");
    }
    return(0);
}
