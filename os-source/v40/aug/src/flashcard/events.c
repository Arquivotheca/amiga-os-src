#include "flash.h"

#define DEBUG 1
#ifdef  DEBUG
extern void kprintf(char *,...);
#define D(a)    kprintf a
#else
#define D(a)
#endif

#define	ACTION_NONE	0

#define	ACTION_ERASE	1
#define	ACTION_CHECK	2
#define	ACTION_DUP	3

extern ULONG totalsizes[];
extern ULONG zonesizes[];

void RadBootBlock( struct cmdVars *cv )
{
	if(cv->cv_SourceIndex == 2)
	{
		ChangeAttrs(cv,cv->cv_Bootable,
			GA_Disabled, (BOOL)FALSE,
			TAG_DONE);

	}
	else
	{
		cv->cv_BootOn = FALSE;

		ChangeAttrs(cv,cv->cv_Bootable,
			GA_Disabled, (BOOL)TRUE,
			GTCB_Checked, (BOOL)cv->cv_BootOn,
			TAG_DONE);

	}

}

void DefaultInfo( struct cmdVars *cv )
{

	StatusBox(cv,0,0,TRUE,STAT_BOX_ID,"Select Options Above");
	StatusBox(cv,0,1,TRUE,STAT_BOX_ID,"then");
	StatusBox(cv,0,2,TRUE,STAT_BOX_ID,"Select an Action on the Left");

}

/*
 * Verify options are valid
 */
BOOL ValidOptions( struct cmdVars *cv )
{
register ULONG total,zone,temp;

	total = totalsizes[cv->cv_TotalIndex];
	zone = zonesizes[cv->cv_ZoneIndex];

	if(total < zone)
	{
		DisplayError(cv,"ERROR: Total Size < Zone Size");
		return(FALSE);
	}	

	temp = total / zone;

	if(total != (temp * zone))
	{
		DisplayError(cv,"ERROR: Invalid Zone Size");
		return(FALSE);
	}

	return(TRUE);

}

void HandleEvents( struct cmdVars *cmv )
{
register struct cmdVars *cv;
struct IntuiMessage *imsg;
ULONG	class;
UWORD	code;
struct	Gadget *gad;
ULONG	signals;
int action;

	cv = cmv;
	action = ACTION_NONE;

	OwnCard(&cv->cv_CardHandle);

	DefaultInfo(cv);

	/* quick check for signals - cheat during erase/write! */

	cv->cv_ZoneHandle.zh_signals = &cv->cv_Task->tc_SigRecvd;
	cv->cv_ZoneHandle.zh_sigmask = (1L<<cv->cv_win->UserPort->mp_SigBit)|(1L<<cv->cv_Signal);
	cv->cv_ZoneHandle.zh_cv = (APTR)cv;

	while(cv->cv_QuitProgram == FALSE)
	{


		if(action)
		{
			GadgetsOnOff(cv,TRUE);
			AbortOnOff(cv,FALSE);

			if(ValidOptions(cv))
			{
				switch(action)
				{
					case ACTION_CHECK:
						DoCheck(cv);
						break;
 
					case ACTION_ERASE:
						DoErase(cv);
						break;
 
					case ACTION_DUP:
						DoProgram(cv);
						break;
 
					default:
						break;
 
				}

			}

			ClearBox(cv,STAT_BOX_ID);

			GadgetsOnOff(cv,FALSE);
			ConfirmOnOff(cv,TRUE);
			AbortOnOff(cv,TRUE);

			action = ACTION_NONE;

			DefaultInfo(cv);

		}

		signals = Wait((1L<<cv->cv_win->UserPort->mp_SigBit)|(1L<<cv->cv_Signal));

		if(signals & (1L<<cv->cv_Signal))
		{
			if(cv->cv_IsRemoved)
			{
				cv->cv_IsRemoved = FALSE;
				cv->cv_IsInserted = FALSE;
				ReleaseCard(&cv->cv_CardHandle,0L);
			}
		}
		while(imsg = GT_GetIMsg(cv->cv_win->UserPort))
		{

			class = imsg->Class;
			code = imsg->Code;
			gad = imsg->IAddress;

			GT_ReplyIMsg( imsg );

			switch(class)
			{
				case IDCMP_CLOSEWINDOW:
					cv->cv_QuitProgram = TRUE;
					break;

				case IDCMP_GADGETUP:

					switch((ULONG)gad->UserData)
					{
						case CM_GADGET_SOURCE:
							cv->cv_SourceIndex = code;
							RadBootBlock(cv);
							break;

						case CM_GADGET_MANUFACTURER:
							cv->cv_ManufIndex = code;
							break;

						case CM_GADGET_TOTALSIZE:
							cv->cv_TotalIndex = code;
							break;

						case CM_GADGET_ZONESIZE:
							cv->cv_ZoneIndex = code;
							break;

						case CM_GADGET_SPEED:
							cv->cv_SpeedIndex = code;
							break;

						case CM_GADGET_ERASE:
							action = ACTION_ERASE;
							break;

						case CM_GADGET_CHECK:
							action = ACTION_CHECK;
							break;

						case CM_GADGET_DUP:
							action = ACTION_DUP;
							break;

						case CM_GADGET_ERASEON:
							cv->cv_EraseOn = cv->cv_EraseFirst->Flags & GFLG_SELECTED;
							break;

						case CM_GADGET_WVERIFY:
							cv->cv_VerifyOn = cv->cv_VerifyWrite->Flags & GFLG_SELECTED;
							break;

						case CM_GADGET_INSTALL:
							cv->cv_BootOn = cv->cv_Bootable->Flags & GFLG_SELECTED;
							break;


						default:
							break;
					}

					break;

				default:
					break;

			}
		}
	}

	ReleaseCard(&cv->cv_CardHandle, CARDF_REMOVEHANDLE);
}

/* Get status of card in our out */

int GetStatus( struct cmdVars *cv )
{

int status;

	status = 2;

	if(ReadCardStatus() & CARD_STATUSF_CCDET)
	{
		status = 1;

		if(cv->cv_IsInserted)
		{

			status = 2;

			if(cv->cv_IsRemoved == FALSE)
			{
				status = 0;
			}
		}

	}

	return(status);
}

/* Display Insert state */

void DisplayCardStatus( struct cmdVars *cv, STRPTR str, int status, BOOL blink )
{

	ClearBox(cv,STAT_BOX_ID);

	if(!blink)
	{
		StatusBox(cv,0,0,TRUE,STAT_BOX_ID,str);
	}

	switch(status)
	{
		case 0:

			if(!blink)
			{
				StatusBox(cv,0,2,TRUE,STAT_BOX_ID,"Select CONFIRM to continue");
				ConfirmOnOff(cv,FALSE);
			}
			break;

		case 1:
			if(!blink)
			{
				StatusBox(cv,0,2,TRUE,STAT_BOX_ID,"REMOVE card in slot");
				ConfirmOnOff(cv,TRUE);
			}
			break;

		case 2:
			if(!blink)
			{
				StatusBox(cv,0,2,TRUE,STAT_BOX_ID,"INSERT FlashROM card");
				ConfirmOnOff(cv,TRUE);
			}
			break;

	}
}

/*
 * Prompt for FlashROM insertion
 */

int PromptForInsert( struct cmdVars *cv, STRPTR str )
{
struct IntuiMessage *imsg;
ULONG	class;
UWORD	code;
struct	Gadget *gad;
ULONG	signals;
BOOL waiting,aborted;
UBYTE blink;
int status,temp;
int iticks;


	ConfirmOnOff(cv,FALSE);
	AbortOnOff(cv,FALSE);

	/* first, verify some of the arguments */

	waiting = TRUE;
	aborted = FALSE;
	iticks = 0;

	blink = 0x1;

	status = GetStatus(cv);
	DisplayCardStatus(cv,str,status,FALSE);

	while(waiting)
	{

		temp = GetStatus(cv);

		if(temp != status)
		{
			status = temp;
			DisplayCardStatus(cv,str,status,FALSE);
			blink = 1;
			iticks = 0;
		}

		if(iticks > 5)
		{
			DisplayCardStatus(cv,str,status,(BOOL)blink & 0x01);
			blink++;
			iticks = 0;
		}

		signals = Wait((1L<<cv->cv_win->UserPort->mp_SigBit)|(1L<<cv->cv_Signal));

		if(signals & (1L<<cv->cv_Signal))
		{
			if(cv->cv_IsRemoved)
			{
				cv->cv_IsRemoved = FALSE;
				cv->cv_IsInserted = FALSE;
				ReleaseCard(&cv->cv_CardHandle,0L);
			}
		}

		while(imsg = GT_GetIMsg(cv->cv_win->UserPort))
		{

			class = imsg->Class;
			code = imsg->Code;
			gad = imsg->IAddress;

			GT_ReplyIMsg( imsg );

			switch(class)
			{
				case IDCMP_CLOSEWINDOW:
					cv->cv_QuitProgram = TRUE;
					waiting = FALSE;
					aborted = TRUE;
					break;

				case IDCMP_GADGETUP:

					switch((ULONG)gad->UserData)
					{

						/* just in case any of these gadgets slip through */

						case CM_GADGET_SOURCE:
							cv->cv_SourceIndex = code;
							RadBootBlock(cv);
							break;

						case CM_GADGET_MANUFACTURER:
							cv->cv_ManufIndex = code;
							break;

						case CM_GADGET_TOTALSIZE:
							cv->cv_TotalIndex = code;
							break;

						case CM_GADGET_ZONESIZE:
							cv->cv_ZoneIndex = code;
							break;

						case CM_GADGET_SPEED:
							cv->cv_SpeedIndex = code;
							break;

						case CM_GADGET_ERASEON:
							cv->cv_EraseOn = cv->cv_EraseFirst->Flags & GFLG_SELECTED;
							break;

						case CM_GADGET_WVERIFY:
							cv->cv_VerifyOn = cv->cv_VerifyWrite->Flags & GFLG_SELECTED;
							break;

						case CM_GADGET_INSTALL:
							cv->cv_BootOn = cv->cv_Bootable->Flags & GFLG_SELECTED;
							break;

						case CM_GADGET_ABORT:
							waiting = FALSE;
							aborted = TRUE;
							break;

						case CM_GADGET_CONFIRM:
							waiting = FALSE;
							break;

						default:
							break;
					}

					break;

				case IDCMP_INTUITICKS:
					iticks++;
					break;

				default:
					break;

			}
		}
	}

	ClearBox(cv,STAT_BOX_ID);
	ConfirmOnOff(cv,TRUE);
	AbortOnOff(cv,TRUE);

	if(aborted == FALSE)
	{
		if(cv->cv_IsInserted)
		{
			if(cv->cv_IsRemoved == FALSE)
			{

				if((ReadCardStatus() & CARD_STATUSF_WR))
				{
					if(temp = LookUpFLASH(cv))
					{

						AbortOnOff(cv,FALSE);
						return(temp);
					}
					else
					{
						DisplayError(cv,"ERROR: Not FlashROM?");
						return(0);
					}

				}

				DisplayError(cv,"ERROR: Card is write-protected");
				return(0);
			}
		}

		DisplayError(cv,"ERROR: Unable to access card");
		return(0);
	}

	return(0);
}

/*
 * Check for aborted erase/write operation
 */
BOOL __asm CheckAbort(register __a1 struct cmdVars *cv )
{
struct IntuiMessage *imsg;
ULONG	class;
struct	Gadget *gad;
BOOL aborted;

	aborted = FALSE;

	SetSignal(0L,1L<<cv->cv_win->UserPort->mp_SigBit);

	while(imsg = GT_GetIMsg(cv->cv_win->UserPort))
	{

		class = imsg->Class;
		gad = imsg->IAddress;

		GT_ReplyIMsg( imsg );

		if(class == IDCMP_GADGETUP)
		{
			if((ULONG)gad->UserData == CM_GADGET_ABORT)
			{
				aborted = TRUE;
			}
		}
	}

	if(aborted)
	{
		AbortOnOff(cv,TRUE);
		if(!(DisplayQuery(cv,"Abort Operation?")))
		{
			aborted = FALSE;
			AbortOnOff(cv,FALSE);
		}
	}
	return(aborted);
}

