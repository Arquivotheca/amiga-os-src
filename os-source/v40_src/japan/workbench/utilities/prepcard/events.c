#include "prepcard.h"

VOID HandleEvents( struct cmdVars *cmv )
{
register struct cmdVars *cv;
struct IntuiMessage *imsg;
struct MenuItem *item;
ULONG	mask, sigs;
ULONG	class;
UWORD	code;
struct	Gadget *gad;
BOOL advloop;

	cv = cmv;

	cv->cv_IsBusy = TRUE;

/* draw display anyway if no card is inserted yet */

	while(cv->cv_IsBusy)
	{
		mask = (1L<<cv->cv_win->UserPort->mp_SigBit);
		mask |= SIGBREAKF_CTRL_C;
		mask |= (1L<<cv->cv_Signal);

		if(cv->cv_IsAdvanced)
		{

			mask |= (1L<<cv->cv_awin->UserPort->mp_SigBit);
		}

		if(!( SetSignal(0L,0L) & (1L<<cv->cv_Signal)))
		{
			if(!(cv->cv_FirstInfoDraw))
			{
				CardCheck(cv);
			}
		}

		sigs = Wait(mask);


		if(sigs & SIGBREAKF_CTRL_C) cv->cv_IsBusy = FALSE;

		if(sigs & (1L<<cv->cv_Signal))
		{


			CardCheck( cv );
		}

		while(imsg = GT_GetIMsg(cv->cv_win->UserPort))
		{

			class = imsg->Class;
			code = imsg->Code;
			gad = imsg->IAddress;

			GT_ReplyIMsg( imsg );

			switch(class)
			{
				case IDCMP_GADGETUP:

					DoCommand(cv,(ULONG)gad->UserData);
					break;

				case IDCMP_MENUPICK:
					if(item = ItemAddress(cv->cv_begmenu,(ULONG)code))
					{
						DoCommand(cv,(ULONG)MENU_USERDATA(item));
					}

				default:
					break;

			}
		}

		if(cv->cv_IsAdvanced)
		{
			advloop = TRUE;
			while(advloop)
			{

				if(imsg = GT_GetIMsg(cv->cv_awin->UserPort))
				{

					class = imsg->Class;
					code = imsg->Code;
					gad = imsg->IAddress;

					GT_ReplyIMsg( imsg );

					switch(class)
					{
						case IDCMP_MOUSEMOVE:
						case IDCMP_GADGETUP:
							NewGadget(cv,gad,code);
							break;

						default:
							break;
					}

/* check for closed advanced window - break loop if so */

					if(!cv->cv_IsAdvanced) advloop = FALSE;
				}
				else
				{
					advloop = FALSE;
				}
			}
		}
	}
}
	