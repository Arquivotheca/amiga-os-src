/*
**  body.c  -- Improved Envoy Remote Command example program, server.
**	       Main library functions: StartService(), GetServiceAttrs(), 
**	       process.
**
**  	       Requires NIPC, Services.
**
**  Copyright 1992, Commodore-Amiga, Inc.
**  Permission to use granted this notice remains intact.
**
**  TO-DO:  Plug any theoretically possible holes in process between getting
**          timerrequest back and dying and StartService() being called in
**          the middle.
*/
#include "ercbase.h"
#define EVER ;;

ULONG LIBENT StartService(register __a0 STRPTR userName,
                          register __a1 STRPTR passWord,
                          register __a2 STRPTR entName,
                          register __a6 struct ERCSvc *ERCBase)
{
ULONG sigbit;
struct TagItem proctags[] =
{
	{NP_Entry, },
	{NP_Name, },
	{TAG_DONE},
};
/*
**  We know that only Services Manager calls StartService(), so we don't have
**  provide any kind of Semaphore protection.  We make sure that we start only
**  one Server process by checking for the existence of its Entity.  We make
**  sure that it has stayed around by setting Exit to FALSE.  The process
**  checks Exit before exiting and goes back to it's main loop if it is FALSE.
**
*/
//	strncpy(entName,"ERC_Service", FOO);
	strcpy(entName,"ERC_Service");
	if(!Entity)
	{
		Status = ENVOYERR_NORESOURCES;  /* unless newproc changes it */
		if(sigbit = AllocSignal(-1L))
		{
			LibProc = FindTask(0L);
			SigMask = (1L<<sigbit);
//			ERCUser = userName;
//			ERCpassWord = passWord;
			proctags[0].ti_Data = (ULONG)Server;
			proctags[1].ti_Data = (ULONG)"erc.library supervisor";
			if((SpawnedProc = CreateNewProc(proctags)) )
			{
				Wait(1L<<sigbit);
			}
			FreeSignal(sigbit);
		}
	}else
	{
		Signal(SpawnedProc, ResetSig);
		Status = ENVOYERR_NOERROR;
	}
	return Status;
}


VOID LIBENT GetServiceAttrsA(register __a0 struct TagItem *tagList,
                             register __a6 struct ERCSvc *ERCBase)
{
struct TagItem *tag;

	if(tag=FindTagItem(SVCAttrs_Name, tagList))
	{
		strcpy((STRPTR)tag->ti_Data, "Envoy Remote Command");
	}
}


VOID __saveds mainLoop(struct ERCSvc *ERCBase, ULONG ent_sigbit,
			struct MsgPort *timerMP, 
			struct timerequest *timerIO);

VOID __saveds Server(VOID)
{
struct ERCSvc *ERCBase;
struct MsgPort *timerMP;
ULONG ent_sigbit;
struct timerequest *timerIO;
struct TagItem enttags[] =
{
	{ENT_Name, }, 
	{ENT_AllocSignal, }, 
	{ENT_Public, TRUE}, 
	{TAG_DONE},
};

/*
**  Opening one's self can't possibly fail.  We use this to get at the library
**  base and to keep the service from expunging while the process is excecuting.
**  We know that StartService() will protect us against multiple processes being
**  started.
*/
#undef  SysBase
#define SysBase (*((struct Library **) 4))
	ERCBase = (struct ERCSvc *)OpenLibrary("erc.service", 0L);
#undef  SysBase
#define SysBase (ERCBase->ERC_SysBase)
/*
**  After open, create an Entity, otherwise init then signal the StartSerice()
**  context that we've stuffed Status appropriately. Finally, go into
**  mainLoop().
*/
	enttags[0].ti_Data = (ULONG) "ERC_Service";
	enttags[1].ti_Data = (ULONG) &ent_sigbit;
	if(Entity = CreateEntityA(enttags))
 	{
		if(timerMP=CreateMsgPort())
		{
			if(timerIO=CreateIORequest(timerMP, sizeof(struct timerequest)))
			{
				if( (ResetSig = AllocSignal(-1)) != -1)
				{
					if(OpenDevice(TIMERNAME, UNIT_VBLANK, timerIO, 0) == 0)
					{
						Status = ENVOYERR_NOERROR;
						Signal(LibProc, SigMask);
						timerIO->tr_node.io_Command = TR_ADDREQUEST;
						timerIO->tr_time.tv_secs = 60;
						timerIO->tr_time.tv_micro = 0;
						SendIO(timerIO);
						mainLoop(ERCBase, ent_sigbit, timerMP, timerIO);
					}
					FreeSignal(ResetSig);
				}
				DeleteIORequest(timerIO);
			}
			DeleteMsgPort(timerMP);
		}
		DeleteEntity(Entity);
/*
**  We want to make sure that StartService() isn't about to assume that we
**  are around when we are on our way out, so Forbid() before cleaning up.
**  Our RTS breaks the Forbid().  There is a small hole here in that if the
**  timerrequest returned at the same time as a Transaction came in, someone
**  could have already called StartService() and we'd never know about it.
**  C'est la vie.
*/
		Forbid();
		Entity = NULL;
	}
	CloseLibrary((struct Library *)ERCBase);
	if(Status != ENVOYERR_NOERROR)  /*  this is for if we failed  */
	{
		Signal(LibProc, SigMask);
	}
}


/*
**  Main loop of server.  Wait().  If any Transactions have arrived, process 
**  and reply them.  If the timer request ever returns, die.  We'll be restared
**  if a new client calls StartService().
*/
VOID __saveds mainLoop(struct ERCSvc *ERCBase, ULONG ent_sigbit,
			struct MsgPort *timerMP, 
			struct timerequest *timerIO)
{
ULONG tweak;
struct Transaction *trans;

	for(EVER)
	{
		tweak = Wait(1L << ent_sigbit | 1L << ResetSig | 1L << timerMP->mp_SigBit);
		if(tweak & 1L << ent_sigbit)
		{
			while(trans = GetTransaction(Entity))
			{
				docmd(trans, ERCBase);
				ReplyTransaction(trans);
			}
		}
		if(tweak & 1L << ResetSig)
		{
			AbortIO(timerIO);
			WaitIO(timerIO);
			SendIO(timerIO);
			continue;  /* in case the timer was tripped simultaneously */
		}
		if(tweak & 1L << timerMP->mp_SigBit)
		{
				GetMsg(timerMP);  /* We know only one thing should arrive here */
				return;
		}
	}
}


VOID __asm docmd(register __a0 struct Transaction *trans, 
		 register __a6 struct ERCSvc *ERCBase)
{
LONG oldpos;
BPTR outputfh;
struct TagItem systags[] =
{
	{SYS_Output, },
	{NP_Name, },
	{SYS_Input, NULL},
	{NP_Priority, -1},
	{TAG_DONE},
};
/*
** A fancier implementation might also want to deal with NP_CurrentDir,
** and NP_StackSize 
*/

	if( !(outputfh = Open("t:erc_command_result", MODE_NEWFILE)) )
	{
		trans->trans_Error = ENVOYERR_NORESOURCES;  /* might be nice to know IoErr() value */
		return;
	}
	systags[0].ti_Data = (LONG) outputfh;
	systags[1].ti_Data = (LONG) "ERC Command";
	trans->trans_Error = SystemTagList((STRPTR)trans->trans_RequestData, 
					   systags);
	oldpos = Seek(outputfh,0,OFFSET_BEGINNING);
	if(oldpos == -1)
	{
		trans->trans_RespDataActual = 0L;
	}else
	{
		if(oldpos+1 > trans->trans_RespDataLength)
		{
			oldpos = trans->trans_RespDataLength-1;
		}
		trans->trans_RespDataActual = Read(outputfh, trans->trans_ResponseData, 
						   oldpos);
		((char *)trans->trans_ResponseData)[oldpos] = '\0';
	}
	trans->trans_RespDataActual++;
	Close(outputfh);
	DeleteFile("t:erc_command_result");
}

