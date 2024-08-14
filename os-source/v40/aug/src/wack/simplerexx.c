/*
 * Amiga Grand Wack
 *
 * simplerexx.c -- Mike Sinz's simplified ARexx handling stuff
 *
 * $Id: simplerexx.c,v 39.1 93/05/03 14:39:37 peter Exp $
 *
 * Code to handle interfacing to ARexx.
 */

/*
 * Simple ARexx interface...
 *
 * This is a very "Simple" interface...
 */

#include	<exec/types.h>
#include	<exec/nodes.h>
#include	<exec/lists.h>
#include	<exec/ports.h>
#include	<exec/memory.h>

#include	<clib/exec_protos.h>
#include	<pragmas/exec_pragmas.h>

#include	<rexx/storage.h>
#include	<rexx/rxslib.h>

#include "sys_proto.h"
#include "asm_proto.h"

/*
 * The prototypes for the few ARexx functions we will call...
 */
struct RexxMsg *CreateRexxMsg(struct MsgPort *, char *, char *);
VOID *CreateArgstring(char *, LONG);
void DeleteRexxMsg(struct RexxMsg *);
void DeleteArgstring(char *);
BOOL IsRexxMsg(struct Message *);

/*
 * Pragmas for the above functions...  (To make this all self-contained...)
 */
#pragma libcall RexxContext->RexxSysBase CreateRexxMsg 90 09803
#pragma libcall RexxContext->RexxSysBase CreateArgstring 7E 0802
#pragma libcall RexxContext->RexxSysBase DeleteRexxMsg 96 801
#pragma libcall RexxContext->RexxSysBase DeleteArgstring 84 801
#pragma libcall RexxContext->RexxSysBase IsRexxMsg A8 801

/*
 * Now, we have made the pragmas needed, let's get to work...
 */

#include	"SimpleRexx.h"

#define	SysBase	RexxContext->SysBase

/*
 * This function returns the signal mask that the Rexx port is
 * using.  It returns NULL if there is no signal...
 *
 * Use this signal bit in your Wait() loop...
 */
ULONG ARexxSignal(struct ARexxContext *RexxContext)
{
register	ULONG	tmp=NULL;

	if (RexxContext) tmp=1L << (RexxContext->ARexxPort->mp_SigBit);
	return(tmp);
}

/*
 * This function returns a structure that contains the commands sent from
 * ARexx...  You will need to parse it and return the structure back
 * so that the memory can be freed...
 *
 * This returns NULL if there was no message...
 */
struct RexxMsg *GetARexxMsg(struct ARexxContext *RexxContext)
{
register	struct	RexxMsg	*tmp=NULL;
register		LONG	flag;

	if (RexxContext) if (tmp=(struct RexxMsg *)GetMsg(RexxContext->ARexxPort))
	{
		if (tmp->rm_Node.mn_Node.ln_Type==NT_REPLYMSG)
		{
			/*
			 * If we had sent a command, it would come this way...
			 *
			 * Since we don't in this simple example, we just throw
			 * away anything that looks "strange"
			 */
			flag=FALSE;
			if (tmp->rm_Result1) flag=TRUE;

			/*
			 * Free the arguments and the message...
			 */
			DeleteArgstring(tmp->rm_Args[0]);
			DeleteRexxMsg(tmp);
			RexxContext->Outstanding-=1;

			/*
			 * Return the error if there was one...
			 */
			tmp=flag ? REXX_RETURN_ERROR : NULL;

			/*
			 * Re-issue our signal just to be sure we don't miss anything
			 */
			SetSignal(ARexxSignal(RexxContext),ARexxSignal(RexxContext));
		}
	}
	return(tmp);
}

/*
 * Use this to return a ARexx message...
 *
 * If you wish to return something, it must be in the RString.
 * If you wish to return an Error, it must be in the Error.
 */
void ReplyARexxMsg(struct ARexxContext *RexxContext,struct RexxMsg *rmsg,char *RString,LONG Error)
{
	if (RexxContext) if (rmsg) if (rmsg!=REXX_RETURN_ERROR)
	{
		rmsg->rm_Result2=0;
		if (!(rmsg->rm_Result1=Error))
		{
			/*
			 * if you did not have an error we can return the string
			 */
			if (rmsg->rm_Action & (1L << RXFB_RESULT))
			{
				/*
				 * ARexx is expecting a result, so...
				 */
				if (RString)
				{
					rmsg->rm_Result2=(LONG)CreateArgstring(RString,(LONG)strlen(RString));
				}
			}
		}

		/*
		 * Reply the message to ARexx...
		 */
		ReplyMsg((struct Message *)rmsg);
	}
}

/*
 * This function will send a string to ARexx...
 *
 * The default host port will be that of your task...
 *
 * If you set StringFile to TRUE, it will set that bit for the message...
 *
 * Returns TRUE if it send the message, FALSE if it did not...
 */
LONG SendARexxMsg(struct ARexxContext *RexxContext,char *RString,LONG StringFile)
{
register	struct	MsgPort	*RexxPort;
register	struct	RexxMsg	*rmsg;
register		LONG	flag=FALSE;

	if (RexxContext) if (RString)
	{
		if (rmsg=CreateRexxMsg(RexxContext->ARexxPort,RexxContext->Extension,RexxContext->PortName))
		{
			rmsg->rm_Action=RXCOMM | (StringFile ? (1L << RXFB_STRING) : 0);
			if (rmsg->rm_Args[0]=CreateArgstring(RString,(LONG)strlen(RString)))
			{
				/*
				 * We need to find the RexxPort and this needs to
				 * be done in a Forbid()
				 */
				Forbid();
				if (RexxPort=FindPort(RXSDIR))
				{
					/*
					 * We found the port, so put the message...
					 */
					PutMsg(RexxPort,(struct Message *)rmsg);
					RexxContext->Outstanding+=1;
					flag=TRUE;
				}
				else
				{
					/*
					 * No port, so clean up...
					 */
					DeleteArgstring(rmsg->rm_Args[0]);
					DeleteRexxMsg(rmsg);
				}
				Permit();
			}
			else DeleteRexxMsg(rmsg);
		}
	}
	return(flag);
}

/*
 * This function closes down the ARexx context that was opened
 * with InitARexx...
 */
void FreeARexx(struct ARexxContext *RexxContext)
{
register	struct	RexxMsg	*rmsg;

	if (RexxContext)
	{
		/*
		 * Clear port name so it can't be found...
		 */
		RexxContext->PortName = "";

		/*
		 * Clean out any outstanding messages we had sent out...
		 */
		while (RexxContext->Outstanding)
		{
			WaitPort(RexxContext->ARexxPort);
			while (rmsg=GetARexxMsg(RexxContext))
			{
				if (rmsg!=REXX_RETURN_ERROR)
				{
					/*
					 * Any messages that come now are blown away...
					 */
					ReplyARexxMsg(RexxContext,rmsg,NULL,100);
				}
			}
		}

		/*
		 * Clean up the port and delete it...
		 */
		if (RexxContext->ARexxPort)
		{
			while (rmsg=GetARexxMsg(RexxContext))
			{
				ReplyARexxMsg(RexxContext,rmsg,NULL,100);
			}
		}

		/*
		 * Make sure we close the library...
		 */
		if (RexxContext->RexxSysBase) CloseLibrary(RexxContext->RexxSysBase);

		/*
		 * Free the memory of the RexxContext
		 */
		FreeMem(RexxContext,sizeof(struct ARexxContext));
	}
}

/*
 * We need the SysBase that is passed in, not the one
 * within RexxContext
 */
#ifdef	SysBase
#undef	SysBase
#endif

#define EXECBASE (*(struct Library **)4)

/*
 * This routine initializes an ARexx port for your process
 * This should only be done once per process.  You must call it
 * with a valid application name and you must use the handle it
 * returns in all other calls...
 *
 * NOTE:  The AppName should not have spaces in it...
 *        Example AppNames:  "MyWord" or "FastCalc" etc...
 *        The name *MUST* be less that 16 characters...
 *        If it is not, it will be trimmed...
 *        The name will also be UPPER-CASED...
 *
 * NOTE:  The Default file name extension, if NULL will be
 *        "rexx"  (the "." is automatic)
 */
struct ARexxContext *InitARexx(struct MsgPort *port, char *AppName,char *Extension)
{
register	struct	Library		*SysBase=EXECBASE;
register	struct	ARexxContext	*RexxContext;
register		LONG		loop;
register		char		*tmp;

	if (RexxContext=AllocMem(sizeof(struct ARexxContext),MEMF_PUBLIC|MEMF_CLEAR))
	{
		RexxContext->SysBase=SysBase;

		if (RexxContext->RexxSysBase=OpenLibrary("rexxsyslib.library",NULL))
		{
			RexxContext->PortName = port->mp_Node.ln_Name;
			/*
			 * Set up the extension...
			 */
			if (!Extension) Extension="rexx";
			tmp=RexxContext->Extension;
			for (loop=0;(loop<7)&&(Extension[loop]);loop++)
			{
				*tmp++=Extension[loop];
			}
			*tmp='\0';

			RexxContext->ARexxPort=port;
		}

		if (!(RexxContext->RexxSysBase))
		{
			FreeARexx(RexxContext);
			RexxContext=NULL;
		}
	}
	return(RexxContext);
}
