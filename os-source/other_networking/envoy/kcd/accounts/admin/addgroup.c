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

ULONG __saveds AddGroup(VOID)
{
    struct Library *SysBase;
    struct Transaction *trans;
    struct Entity *client;
    struct Entity *server;
    struct RDArgs *rdargs;

    LONG args[5];
    ULONG signal;
    AdminGroupReq *agr;
    UWORD i;

    SysBase = (*(struct Library **)4L);

    if(DOSBase = OpenLibrary("dos.library",37L))
    {
    	for(i=0;i<5;i++)
    	    args[i]=0L;

    	if(rdargs = ReadArgs("Authority/A,AuthPW/A,AdminPW/A,GroupName/A,AdminID/A/N",args,NULL))
    	{
    	    if(NIPCBase = OpenLibrary("nipc.library",37L))
    	    {
    	    	if(client = CreateEntity(ENT_AllocSignal, &signal,
    	    				 TAG_DONE))
    	    	{
    	    	    if(server = FindEntity(NULL,"Accounts Server",client,NULL))
    	    	    {
    	    	    	if(trans = AllocTransaction(TRN_AllocReqBuffer,sizeof(AdminGroupReq),
    	    	    				    TAG_DONE))
    	    	    	{
    	    	    	    trans->trans_ResponseData = trans->trans_RequestData;
    	    	    	    trans->trans_Command = ACMD_AddGroup;

    	    	    	    agr = (AdminGroupReq *)trans->trans_RequestData;

    	    	    	    stccpy(agr->Authority.UserName,(STRPTR)args[0],32);
    	    	    	    stccpy(agr->Authority.Password,(STRPTR)args[1],32);
    	    	    	    stccpy(agr->Authority.AuthPW,(STRPTR)args[2],32);
    	    	    	    stccpy(agr->Group.GroupName,(STRPTR)args[3],32);
    	    	    	    agr->Group.GroupID = 0;
    	    	    	    agr->Group.Flags = 0;
    	    	    	    agr->Group.AdminID = (*((ULONG *)args[4]));

    	    	    	    DoTransaction(server,client,trans);

    	    	    	    if(!trans->trans_Error)
    	    	    	    {
    	    	    	    	Printf("Group Added.  GroupID = %ld\n",(ULONG)agr->Group.GroupID);
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

    	    			    default :	Printf("Unknown Error: %ld\n",(ULONG)trans->trans_Error);
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
	    PutStr("Required Parameter(s) missing.\n");
    }
    return(0);
}
