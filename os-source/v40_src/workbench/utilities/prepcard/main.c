
#include "prepcard.h"

static ULONG ProcTags[] = {
	NP_Entry,	StartChild,
	TAG_DONE,
};

struct Catalog *OpenACatalog( struct cmdVars *cv, STRPTR name, ULONG tag1, ... );

LONG main( VOID )
{
struct cmdVars vars;
register struct cmdVars *cv;
struct Process *process;
struct RDArgs *rdargs;

int error;

	cv = &vars;

	/* Clear all variables */

	memset(cv,0,sizeof(struct cmdVars));
	
	SysBase	= (*((struct ExecBase **) 4));

	geta4();

	process = (struct Process *)FindTask(NULL);

	cv->cv_Task = (struct Task *)process;

	if(!process->pr_CLI)
	{
		WaitPort(&process->pr_MsgPort);
		WBenchMsg = (struct WBStartup *) GetMsg(&process->pr_MsgPort);

	}

	error = RETURN_FAIL;

	CardResource = OpenResource(CARDRESNAME);

		if(DOSBase = OpenLibrary("dos.library",37))
		{
			if(GfxBase = OpenLibrary("graphics.library",37))
			{
				if(IntuitionBase = OpenLibrary("intuition.library",37))
				{
					if(GadToolsBase = OpenLibrary("gadtools.library",37))
					{
						if(LocaleBase = OpenLibrary("locale.library",38))
							cv->cv_Catalog = OpenACatalog(cv,CATALOG,TAG_DONE);

						error = RETURN_OK;

						if(!(WBenchMsg))
						{
							cv->cv_FromCLI = TRUE;
							if(!(rdargs = ReadArgs(TEMPLATE,&cv->cv_opts[0],NULL)))
							{
								PrintFault(IoErr(),NULL);
								error = RETURN_FAIL;
							}

						}

						if(error == RETURN_OK)
						{
							error = StartUI(cv);
						}

						if(rdargs) FreeArgs(rdargs);

						if(LocaleBase)
						{
							if(cv->cv_Catalog)
							{
								CloseCatalog(cv->cv_Catalog);
							}
							CloseLibrary(LocaleBase);
						}

						CloseLibrary(GadToolsBase);
					}

					CloseLibrary(IntuitionBase);
				}

				CloseLibrary(GfxBase);
			}

			CloseLibrary(DOSBase);
		}

	if(WBenchMsg)
	{
		Forbid();
		ReplyMsg(WBenchMsg);
	}

	return(error);
}

/* Having opened the basic libraries, we'll decide what kind of user
 * interface applies.
 */

LONG StartUI( struct cmdVars *cmv )
{
register struct cmdVars *cv;
register struct CardHandle *ch;
struct MemHeader *mh;
BOOL isCARDMEM;
int error;

	error = RETURN_FAIL;

	cv = cmv;

/* Hmm, if no resource, they either have a machine without a card slot, or
 * they've inserted a card which is being used as SRAM
 */

	if(!(CardResource))
	{
/* check memory for credit-card memory */

		isCARDMEM = FALSE;

		Forbid();

		mh = (struct MemHeader *)SysBase->MemList.lh_Head;


		while(mh->mh_Node.ln_Succ)
		{
			if(mh->mh_Node.ln_Name)
			{
				if(!(strcmp(CARDRESNAME,mh->mh_Node.ln_Name)))
				{
					isCARDMEM = TRUE;
					break;
				}
			}
			mh = (struct MemHeader *)mh->mh_Node.ln_Succ;
		}

		Permit();

		if(isCARDMEM)
		{
			DisplayError(cv,MSG_PREP_ERROR_ISRAM);
		}
		else
		{
/* if not credit-card memory, then the only (current) alternative is there
 * really is no card.resource because there is no GAYLE chip, and card slot.
 * In this case this tool cannot be run on 'this' machine.
 */

			DisplayError(cv,MSG_PREP_ERROR_NORES);

		}
		return(RETURN_ERROR);
	}

/* We know there is a card.resource.  Setup some other stuff before
 * deciding on UI to use.
 */

	cv->cv_Inserted.is_Data = (APTR)cv;
	cv->cv_Inserted.is_Code = (APTR)&InsertInt;

	cv->cv_Removed.is_Data = (APTR)cv;
	cv->cv_Removed.is_Code = (APTR)&RemovedInt;

/* obtain a pointer to a memory map */

	cv->cv_CardMemMap = GetCardMap();


/* set-up insert/own handle */

	ch = &cv->cv_CardHandle;
	
	ch->cah_CardNode.ln_Type = 0;

/* priority is not max, but it is higher than any other device should be */

	ch->cah_CardNode.ln_Pri = 120;
	ch->cah_CardNode.ln_Name = GetString(cv,MSG_PREP_TITLE);

	ch->cah_CardInserted = &cv->cv_Inserted;
	ch->cah_CardRemoved = &cv->cv_Removed;

	ch->cah_CardFlags = CARDF_DELAYOWNERSHIP;

/* set neither inserted, or removed */

	cv->cv_IsRemoved = FALSE;
	cv->cv_IsInserted = FALSE;

/* some additional set-up before going any further */

	if(cv->cv_Signal = AllocSignal(-1L))
	{

		SetSignal(0L,1L<<cv->cv_Signal);

/* init line list */

		NewList((struct List *)&cv->cv_StatsList);

/* setup FORMAT info defaults */

		Defaults( cv );

/* start card handling process */

/* init parent/child semaphore for card release protocol */

		InitSemaphore(&cv->cv_CardSemaphore);

		if(cv->cv_ReplyPort = CreateMsgPort())
		{
			cv->cv_ParentPort.mp_Node.ln_Name = PARENT_PORT;
			cv->cv_ParentPort.mp_Node.ln_Pri = 0;

			Forbid();
			if(!(FindPort(PARENT_PORT)))
			{
				AddPort(&cv->cv_ParentPort);

				Permit();

				if(CreateNewProc((struct TagItem *)&ProcTags[0]))
				{

/* wait for child to startup - signal */


					Wait(1<<cv->cv_Signal);


					if(FindPort(CHILD_PORT))
					{

						if(WBenchMsg)
						{
							error = StartGUI(cv);
						}
						else
						{
							error = StartCLI(cv);
						}


						PutChildMsg( cv, CHILD_QUIT, NULL );				
					}
			
				}
				RemPort(&cv->cv_ParentPort);

			}
			else
			{
				error = RETURN_WARN;
				Permit();
			}

			DeleteMsgPort(cv->cv_ReplyPort);
		}
		FreeSignal(cv->cv_Signal);
	}
	return(error);
}

/**
 ** Put/Wait message to child
 **/

VOID PutChildMsg( struct cmdVars *cmv, ULONG command, APTR data )
{
register struct cmdVars *cv;
struct PrepMsg pm;

	cv = cmv;

	pm.pm_msg.mn_Node.ln_Type = 0;
	pm.pm_msg.mn_Length = sizeof(struct PrepMsg);
	pm.pm_msg.mn_ReplyPort = cv->cv_ReplyPort;
	pm.pm_Command = command;
	pm.pm_Data = data;

	PutMsg( cv->cv_ChildPort, (struct Message *)&pm );

	WaitPort( cv->cv_ReplyPort );

	while(GetMsg(cv->cv_ReplyPort));
	
}

/**
 ** Open a locale catalog - tags passed in
 **/
struct Catalog *OpenACatalog( struct cmdVars *cv, STRPTR name, ULONG tag1, ... )
{
	return( OpenCatalogA(NULL,name,(struct TagItem *)&tag1) );
}

