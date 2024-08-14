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

ULONG __saveds AddUser(VOID)
{
    struct Library *SysBase;
    struct Transaction *trans;
    struct Entity *client;
    struct Entity *server;
    struct RDArgs *rdargs;

    LONG args[5];
    ULONG signal;
    AdminUserReq *aur;
    UWORD i;

    SysBase = (*(struct Library **)4L);

    if(DOSBase = OpenLibrary("dos.library",37L))
    {
    	for(i=0;i<5;i++)
    	    args[i]=0L;

    	if(rdargs = ReadArgs("Authority/A,AuthPW/A,AdminPW/A,UserName/A,Password/A",args,NULL))
    	{
    	    if(NIPCBase = OpenLibrary("nipc.library",37L))
    	    {
    	    	if(client = CreateEntity(ENT_AllocSignal, &signal,
    	    				 TAG_DONE))
    	    	{
    	    	    if(server = FindEntity(NULL,"Accounts Server",client,NULL))
    	    	    {
    	    	    	if(trans = AllocTransaction(TRN_AllocReqBuffer,sizeof(AdminUserReq),
    	    	    				    TAG_DONE))
    	    	    	{
    	    	    	    trans->trans_ResponseData = trans->trans_RequestData;
    	    	    	    trans->trans_Command = ACMD_AddUser;

    	    	    	    aur = (AdminUserReq *)trans->trans_RequestData;

    	    	    	    stccpy(aur->Authority.UserName,(STRPTR)args[0],32);
    	    	    	    stccpy(aur->Authority.Password,(STRPTR)args[1],32);
    	    	    	    stccpy(aur->Authority.AuthPW,(STRPTR)args[2],32);
    	    	    	    stccpy(aur->User.UserName,(STRPTR)args[3],32);
    	    	    	    stccpy(aur->User.Password,(STRPTR)args[4],32);
    	    	    	    aur->User.UserID = 0;
    	    	    	    aur->User.PrimaryGroupID = 0;
    	    	    	    aur->User.Flags = 0;

    	    	    	    DoTransaction(server,client,trans);

    	    	    	    if(!trans->trans_Error)
    	    	    	    {
    	    	    	    	Printf("User Added.  UserID = %ld\n",(ULONG)aur->User.UserID);
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

    	    			    default :	Printf("Unknown Error: %ld",&trans->trans_Error);
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
