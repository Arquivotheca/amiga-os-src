#include "prepcard.h"

static struct TextAttr far Topaz80 = {
	"topaz.font",8,FS_NORMAL,FPF_ROMFONT
};

LONG StartGUI( struct cmdVars *cmv )
{
int error;
register struct cmdVars *cv;


	cv = cmv;
	error = RETURN_FAIL;

	if(cv->cv_font = OpenFont(&Topaz80))
	{
		if(cv->cv_sp = LockPubScreen(NULL))
		{
			if(cv->cv_VI = GetVisualInfoA(cv->cv_sp,NULL))
			{
				if(MakeGadgets( cv ))
				{
					if(MakeWindow( cv ))
					{
						if(MakeBegMenu( cv ))
						{
							error = RETURN_OK;
							cv->cv_FromCLI = FALSE;

							HandleEvents( cv );

							ClearMenuStrip(cv->cv_win);
							FreeMenus( cv->cv_begmenu );

							CloseAdvWindow( cv );
						}
						CloseWindow( cv->cv_win );
					}
					FreeGadgets( cv->cv_gadgets );
				}
				FreeVisualInfo( cv->cv_VI );
			}
			UnlockPubScreen( NULL, cv->cv_sp );
		}
		CloseFont( cv->cv_font );
	}

	if( error )
	{
	    DisplayError(cv,MSG_PREP_NO_MEM_ERROR);
	}


	return( error );

}

LONG StartCLI( struct cmdVars *cmv )
{
register struct cmdVars *cv;
BOOL abort;
STRPTR prompt;
LONG error, doCLI;
UBYTE chbuf[2];

	cv = cmv;

	doCLI = CM_DEFAULT;

	error = RETURN_FAIL;

	if(cv->cv_opts[OPT_DISK])
	{
		prompt = GetString(cv,MSG_PREP_FORMAT_PROMPT);
		doCLI = CM_MAKEDISK;
	}
	else
	{
		if(cv->cv_opts[OPT_RAM])
		{
			prompt = GetString(cv,MSG_PREP_RAM_PROMPT);
			doCLI = CM_MAKERAM;
		}
	}


	if( doCLI)
	{
/* if args, use CLI prompting */

		PutStr(prompt);

		PutStr(GetString(cv,MSG_PREP_WARN_PROMPT));

		PutStr(GetString(cv,MSG_PREP_INSERT_PROMPT));

/* Wait for RETURN or CTRL_C */

		Flush(Output());

		abort = FALSE;

		chbuf[0] = 0x0;

		while(WaitForChar(Input(),30))
		{
			if(Read(Input(),chbuf,1L) != 1)
				chbuf[0] = -1;
		}

		while(TRUE)
		{

			if(WaitForChar(Input(),100000L))
			{
				if(Read(Input(),chbuf,1L) == 1)
				{
					break;
				}
			}
			else
			{
				if(CheckSignal(SIGBREAKF_CTRL_C))
				{
					abort = TRUE;
					break;
				}
			}

		}

		if(!(abort))
		{
			if(chbuf[0] != '\n')
				Read(Input(),chbuf,1);

/* check if card yet, if not, wait a moment, and then try */
/* assume card was inserted as RETURN was pressed */

			if(!cv->cv_IsInserted)
			{
				Delay(25L);
			}

			error = DoCommand(cv,doCLI);

		}
		else
		{
			error = ERROR_BREAK;
			PrintFault(ERROR_BREAK,NULL);
		}


	}
	else
	{

/* no args?  assume they want GUI interface */

		error = StartGUI(cv);
	}

	return(error);

}

