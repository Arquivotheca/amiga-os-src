#include "flash.h"

#ifdef  DEBUG
extern void kprintf(char *,...);
#define D(a)    kprintf a
#else
#define D(a)
#endif

LONG main( VOID )
{
	
struct cmdVars vars;
register struct cmdVars *cv;
struct Process *process;
struct RDArgs *rdargs;

int error;

	cv = & vars;

	memset(cv,0,sizeof(struct cmdVars));

	SysBase = (*((struct ExecBase **) 4));

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
	CIABBase = OpenResource(CIABNAME);
	AslBase = OpenLibrary("asl.library",37);

		if(DOSBase = OpenLibrary("dos.library",37))
		{
			if(GfxBase = OpenLibrary("graphics.library",37))
			{
				if(IntuitionBase = OpenLibrary("intuition.library",37))
				{
					if(GadToolsBase = OpenLibrary("gadtools.library",37))
					{

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
							error = StartCard(cv);
						}

						if(rdargs) FreeArgs(rdargs);

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
		ReplyMsg((struct Message *)WBenchMsg);
	}

	if(AslBase)	CloseLibrary(AslBase);
	if(freq)	FreeAslRequest(freq);

	return(error);
}


/* Having opened the basic libraries, we'll decide what kind of user
 * interface applies.
 */

LONG StartCard( struct cmdVars *cmv )
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
			DisplayError(cv,"Card in use as System RAM");
		}
		else
		{
/* if not credit-card memory, then the only (current) alternative is there
 * really is no card.resource because there is no GAYLE chip, and card slot.
 * In this case this tool cannot be run on 'this' machine.
 */

			DisplayError(cv,"No card slot");

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

	cv->cv_StatusChange.is_Data = (APTR)cv;
	cv->cv_StatusChange.is_Code = (APTR)&StatusInt;

/* obtain a pointer to a memory map */

	cv->cv_CardMemMap = GetCardMap();

/* set-up insert/own handle */

	ch = &cv->cv_CardHandle;
	
	ch->cah_CardNode.ln_Type = 0;

/* priority is not max, but it is higher than any other device should be */

	ch->cah_CardNode.ln_Pri = 120;
	ch->cah_CardNode.ln_Name = FLASH_TOOL;

	ch->cah_CardInserted = &cv->cv_Inserted;
	ch->cah_CardRemoved = &cv->cv_Removed;
	ch->cah_CardStatus = &cv->cv_StatusChange;

	ch->cah_CardFlags = CARDF_DELAYOWNERSHIP;

/* set neither inserted, or removed */

	cv->cv_IsRemoved = FALSE;
	cv->cv_IsInserted = FALSE;

/* Initialize some defaults */

	cv->cv_EraseOn = TRUE;
	cv->cv_VerifyOn = FALSE;
	cv->cv_BootOn = FALSE;

	cv->cv_SourceIndex = 0;	/* trackdisk default */
	cv->cv_ManufIndex = 0;	/* Intel default */
	cv->cv_TotalIndex = 4;	/* 1 MB default */
	cv->cv_ZoneIndex = 5;	/* 256K default */
	cv->cv_SpeedIndex = 3;	/* 250ns default */

/* do not allow the tool to be run twice */

	cv->cv_ParentPort.mp_Node.ln_Name = FLASH_TOOL;

	Forbid();

	if(!(FindPort(FLASH_TOOL)))
	{
		AddPort(&cv->cv_ParentPort);
		Permit();


                if(cv->cv_Signal = AllocSignal(-1L))
		{
			SetSignal(0L,1L<<cv->cv_Signal);

/* Final step is to allocate a CIAB interval timer */

			if(AllocateCIA(cv))
			{

/* Use CIA timer for polling purposes only */

	                        AbleICR(CIABBase,1L<<cv->cv_ciatimerbit);

/* Looks good, so start up the interface */

				error = StartGUI(cv);

				RemICRVector(CIABBase,cv->cv_ciatimerbit,&cv->cv_ciabint);

				RemPort(&cv->cv_ParentPort);

			}

			FreeSignal(cv->cv_Signal);
		}
	}
	else
	{

/* else just return, and exit */

		Permit();
		error = RETURN_WARN;
	}

	return(error);

}


