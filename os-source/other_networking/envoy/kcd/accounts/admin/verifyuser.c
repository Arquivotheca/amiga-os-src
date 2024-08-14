#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/rdargs.h>
#include "/accounts.h"
#include <string.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/accounts_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/accounts_pragmas.h>

struct Library *DOSBase;
struct Library *AccountsBase;

ULONG __saveds Verify(VOID)
{
    struct Library *SysBase;
    struct RDArgs *rdargs;
    struct UserInfo *ui;

    LONG args[2];
    UWORD i;

    SysBase = (*(struct Library **)4L);

    if(DOSBase = OpenLibrary("dos.library",37L))
    {
    	for(i=0;i<2;i++)
    	    args[i]=0L;

    	if(rdargs = ReadArgs("User/A,Password/A",args,NULL))
    	{
    	    if(AccountsBase = OpenLibrary("accounts.library",37L))
    	    {
    	    	Printf("AccountsBase: %lx\n",AccountsBase);

    	    	if(ui = AllocUserInfo())
    	    	{
    	    	    if(!VerifyUser((STRPTR)args[0],(STRPTR)args[1],ui))
    	    	    {
    	    	    	PutStr("Vaild Username/Password.");
    	    	    	PutStr("------------------------");
    	    	    	Printf("UserName: %s\n",ui->ui_UserName);
    	    	    	Printf("User ID:  %ld\n",ui->ui_UserID);
    	    	    	Printf("Primary Group ID: %ld\n",ui->ui_PrimaryGroupID);
    	    	    }
    	    	    else
    	    	    	Printf("Username/Password not valid!\n");

    	    	    FreeUserInfo(ui);
    	    	}
    	    	CloseLibrary(AccountsBase);
    	     }
    	     FreeArgs(rdargs);
    	 }
  	 CloseLibrary(DOSBase);
    }
    return(0L);
}

