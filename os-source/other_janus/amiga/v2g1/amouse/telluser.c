
#include <intuition/intuition.h>

#include <exec/alerts.h>
#include <exec/devices.h>
#include <exec/exec.h>
#include <exec/execbase.h>
#include <exec/io.h>
#include <exec/libraries.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include <exec/nodes.h>
#include <exec/ports.h>
#include <exec/types.h>
#include <workbench/workbench.h>
#include <libraries/dosextens.h>

extern	long	IntuitionBase;

/*
	routine to find the name of the process telling the user something
*/

static char Buf1[32];
char *WhoAmI()
	{
	struct Process			*Process;
	struct CommandLineInterface	*CLI;

	Process = FindTask(0);
	CLI = (struct CommandLineInterface *) BADDR(Process->pr_CLI);
	if( CLI )
		{
		moveBSTR(CLI->cli_CommandName, Buf1, 32);
		return(Buf1);
		}
	else
		return(Process->pr_Task.tc_Node.ln_Name);
	}	/* end of WhoAmI */

/*
	check that intuition is available
*/

int CheckIntui()
	{
	if( !IntuitionBase )	/* NULL */
		{	/* will be opened again! problems ? */
		if( IntuitionBase = OpenLibrary("intuition.library",0) )
			return(FALSE);
		else
			Alert(AT_Recovery+AG_OpenLib+AO_Intuition,NULL);
		return(TRUE);
		}
	return(FALSE);
	}	/* end of CheckIntui */

static struct IntuiText WhoText = {1,0,JAM2, 20,20,NULL,NULL, NULL};
static struct IntuiText MsgText = {0,1,JAM2, 20,30,NULL,NULL, &WhoText};
static struct IntuiText OKText  = {0,1,JAM2, 5,3,NULL,"OK", NULL};

/*
	issue a requester to tell the user something
*/

TellUser(Text)
char *Text;			/* what to tell the user */
	{
	int	Width,
		L1;
	if( CheckIntui() )	/* open intuition.library if not already open */
		return;
	MsgText.IText = Text;
	WhoText.IText = WhoAmI();	/* process name */
	if( (Width = IntuiTextLength(&MsgText) ) <
			(L1 = IntuiTextLength(&WhoText) ) )
		Width = L1;
	AutoRequest(
		NULL,
		&MsgText,
		&OKText,
		&OKText,
		0L,0L,
		Width+70,80);
	}	/* end of TellUser */


/*
	moveBSTR copies a BSTR to a C char string.
*/

moveBSTR(bptr, buffer, maxlen)
BSTR bptr;
char *buffer;
int maxlen; /* size of buffer[] */
	{
	register char *ptr;
	register unsigned int len, i;
	unsigned char l;

	ptr = (char *) BADDR(bptr); /* Make a char* to the length */

	l = (unsigned int) (*ptr++); /* Get the length and increment ptr */

	if( !(len = l) )
		{
		*buffer = '\0'; /* Mark the end of the string. */
		return;
		}
	if( len > maxlen )
		len = maxlen;
	for(	i = 0;
		i < len;
		i++)
		*buffer++ = *ptr++; /* Copy it. */

	if(i < maxlen)
		*buffer = '\0'; /* If there is room, mark the end */
	return;
	}	/* end of moveBSTR */
