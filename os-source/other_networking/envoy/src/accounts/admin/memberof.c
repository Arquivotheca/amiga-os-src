#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/rdargs.h>
#include <envoy/accounts.h>
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
    struct GroupInfo *gi;
    ULONG returncode;

    LONG args[2];
    UWORD i;

    SysBase = (*(struct Library **)4L);

    if(DOSBase = OpenLibrary("dos.library",37L))
    {
    	for(i=0;i<2;i++)
    	    args[i]=0L;

    	if(rdargs = ReadArgs("Group/A,User/A",args,NULL))
    	{
    	    if(AccountsBase = OpenLibrary("accounts.library",37L))
    	    {
    	    	Printf("AccountsBase: %lx\n",AccountsBase);

    	    	if(ui = AllocUserInfo())
    	    	{
    	    	    if(gi = (struct GroupInfo *)AllocGroupInfo())
    	    	    {
    	    	    	stccpy(ui->ui_UserName,(STRPTR)args[1],32);
    	    	    	stccpy(gi->gi_GroupName,(STRPTR)args[0],32);

			returncode = MemberOf(gi,ui);

                        if(!returncode)
                        {
                            PutStr("Vaild Group Member.\n");
                        }
                        else
                        {
                            switch(returncode)
                            {
                            	case ACCERROR_UNKNOWNGROUP : PutStr("Invalid Group.\n");
                            				     break;

                            	case ACCERROR_UNKNOWNUSER : PutStr("
                            Printf("Invalid Group Member! Error: %ld\n",(LONG)returncode);

                        FreeGroupInfo((struct UserInfo *)gi);
		    }
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

