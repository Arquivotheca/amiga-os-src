/*
**  erc.c  -- Improved Envoy Remote Command example program, client.  
**	      Requires NIPC, Services.
**
**  Copyright 1992, Commodore-Amiga, Inc.
**  Permission to use granted this notice remains intact.
**
**	To-do:  "Can't find server program" needs detailed error text.
**
*/
#include <exec/exec.h>
#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>
#include <dos/dos.h>
#include <dos/dostags.h>
#include <clib/dos_protos.h>
#include <pragmas/dos_pragmas.h>
#include <envoy/nipc.h>
#include <clib/nipc_protos.h>
#include <pragmas/nipc_pragmas.h>
#include <envoy/services.h>
#include <clib/services_protos.h>
#include <pragmas/services_pragmas.h>
/*#include "erc_rev.h"*/
#include <string.h>
/*  To set the secondary return code */
#define EXECBASE (*(struct ExecBase **)4)
#define THISPROC    ((struct Process *)(EXECBASE->ThisTask))
#define Result2(x)  THISPROC->pr_Result2 = x

/*  For ReadArgs():  (Make sure to memset() if an optional arg is introduced) */
#define TEMPLATE "HOST/A,COMMAND/F/A" /* "\0$VER: " VSTRING */
#define OPT_HOSTNAME	0
#define OPT_COMMAND	1
#define OPT_COUNT	2


LONG __saveds erc_command(void)
{
struct Library *SysBase = (*((struct Library **) 4)),
	       *DOSBase,
	       *NIPCBase = NULL,
	       *ServicesBase = NULL;
struct Entity *myentity = NULL,
              *hisentity = NULL;
struct Transaction *trans = NULL;
LONG rc = RETURN_FAIL,			/* return code */
     opts[OPT_COUNT], 			/* for ReadArgs() */
     entsig, mask;
struct RDargs *rdargs;
/*
**  The C-stub versions of CreateEntity() and AllocTransaction() are very nice,
**  but are rather difficult to use in a program with no globals...
*/
struct TagItem create[] =
{
	{ENT_AllocSignal, NULL},
	{TAG_DONE}
};
struct TagItem alloc[] =
{
	{TRN_AllocRespBuffer, 5000},
	{TAG_DONE}
};
struct TagItem find[] =
{
//	{FSVC_UserName, NULL},
//	{FSVC_PassWord, NULL},
//	{FSVC_Error, },
	{TAG_DONE}
};

	create[0].ti_Data = (LONG)&entsig;
	if( !(DOSBase = OpenLibrary("dos.library", 37L)) )
	{
		Result2(ERROR_INVALID_RESIDENT_LIBRARY);
		return(rc);
	}
//	memset(opts, 0, sizeof(opts));  don't have to because all are required!
	if(rdargs = ReadArgs(TEMPLATE, opts, NULL))
	{
		if(NIPCBase = OpenLibrary("nipc.library", 0))
		{
			if(ServicesBase = OpenLibrary("services.library", 0))
			{
				if(myentity = CreateEntityA(create))
				{
					hisentity = FindServiceA((STRPTR)opts[OPT_HOSTNAME],
	       		                			  "Envoy Remote Command",
	               		        			  myentity, find);
					if(hisentity)
					{
						if(trans = AllocTransactionA(alloc))
						{
							trans->trans_RequestData = (STRPTR)opts[OPT_COMMAND];
							trans->trans_ReqDataLength = trans->trans_ReqDataActual = 
								strlen((STRPTR)opts[OPT_COMMAND])+1;
							trans->trans_Timeout = 10;  /*  10 Second time-out  */
							BeginTransaction(hisentity, myentity, trans);
						/*
						**  The following line may cause not-so-bright compilers to complain that
						**  we are using a variable (entsig) that we haven't initialized.  Don't
						**  worry, CreateEntityA() initialized it for us using the pointer we passed
						**  it.
						*/
							mask = Wait(1L << entsig | SIGBREAKF_CTRL_C);
							if(mask & 1L << entsig)
							{
								if( GetTransaction(myentity) )  /*  pull the reply off. */
								{
									PutStr(trans->trans_ResponseData);
									rc=trans->trans_Error;
								}
							}
							if(mask & SIGBREAKF_CTRL_C)
							{
								AbortTransaction(trans);  /* Sure it's OK to abort even if we got it above.  Of course, the odds are damn slim! */
								PutStr("Please wait, aborting transaction with server.\n");
								WaitTransaction(trans);  /* always wait on aborted transactions */
							}
							FreeTransaction(trans);
						}else
						{
							PutStr("Can't alloc transaction.\n");
						}
						LoseService(hisentity);
					}else
					{
						PutStr("Can't find server program.\n");
					}
					DeleteEntity(myentity);
				}else
				{
					PutStr("Can't create entity.\n");
				}
				CloseLibrary(ServicesBase);
			}else
			{
				PutStr("Can't open Services.\n");
			}
			CloseLibrary(NIPCBase);
		}else
		{
			PutStr("Can't open NIPC.\n");
		}
		FreeArgs(rdargs);
	}else
	{
			PrintFault(IoErr(),NULL);
	}
	CloseLibrary(DOSBase);
	return(rc);
}
