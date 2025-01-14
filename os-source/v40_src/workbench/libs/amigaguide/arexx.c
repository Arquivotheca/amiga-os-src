/*
 * Simple ARexx interface...
 *
 * This is a very "Simple" interface to the world of ARexx...
 * For more complex interfaces into ARexx, it is best that you
 * understand the functions that are provided by ARexx.
 * In many cases they are more powerful than what is presented
 * here.
 *
 * This code is fully re-entrant and self-contained other than
 * the use of SysBase/AbsExecBase and the ARexx RVI support
 * library which is also self-contained...
 */

#include "amigaguidebase.h"

/*****************************************************************************/

STRPTR ARexxPortName (struct AmigaGuideLib * ag, AREXXCONTEXT ac)
{
    if (ac)
	return ac->ARexxPort->mp_Node.ln_Name;
    return NULL;
}

/*****************************************************************************/

/*
 * This function returns the signal mask that the Rexx port is
 * using.  It returns NULL if there is no signal...
 *
 * Use this signal bit in your Wait() loop...
 */
ULONG ARexxSignal (struct AmigaGuideLib * ag, AREXXCONTEXT RexxContext)
{

    if (RexxContext)
	return ((ULONG) (1L << (RexxContext->ARexxPort->mp_SigBit)));
    return (0);
}

/*****************************************************************************/

/*
 * This function returns a structure that contains the commands sent from
 * ARexx...  You will need to parse it and return the structure back
 * so that the memory can be freed...
 *
 * This returns NULL if there was no message...
 */
struct RexxMsg *GetARexxMsg (struct AmigaGuideLib * ag, AREXXCONTEXT RexxContext)
{
    register struct RexxMsg *tmp = NULL;
    register short flag;

    if (RexxContext)
    {
	if (tmp = (struct RexxMsg *) GetMsg (RexxContext->ARexxPort))
	{
	    if (tmp->rm_Node.mn_Node.ln_Type == NT_REPLYMSG)
	    {

		/*
		 * If we had sent a command, it would come this way...
		 *
		 * Since we don't in this simple example, we just throw away anything that looks "strange"
		 */
		flag = FALSE;
		if (tmp->rm_Result1)
		    flag = TRUE;

		/*
		 * Free the arguments and the message...
		 */
		DeleteArgstring (tmp->rm_Args[0]);
		DeleteRexxMsg (tmp);
		RexxContext->Outstanding -= 1;

		/*
		 * Return the error if there was one...
		 */
		tmp = flag ? REXX_RETURN_ERROR : NULL;
	    }
	}
    }
    return (tmp);
}

/*****************************************************************************/

/*
 * Use this to return a ARexx message...
 *
 * If you wish to return something, it must be in the RString.
 * If you wish to return an Error, it must be in the Error.
 * If there is an error, the RString is ignored.
 */
void ReplyARexxMsg (struct AmigaGuideLib * ag, AREXXCONTEXT RexxContext, struct RexxMsg * rmsg, char *RString, LONG Error)
{
    if (RexxContext && rmsg)
    {
	if (rmsg != REXX_RETURN_ERROR)
	{
	    rmsg->rm_Result2 = 0;
	    if (!(rmsg->rm_Result1 = Error))
	    {

		/*
		 * if you did not have an error we return the string
		 */
		if (rmsg->rm_Action & (1L << RXFB_RESULT))
		    if (RString)
		    {
			rmsg->rm_Result2 = (LONG) CreateArgstring (RString,
								   (LONG) strlen (RString));
		    }
	    }

	    /*
	     * Reply the message to ARexx...
	     */
	    ReplyMsg ((struct Message *) rmsg);
	}
    }
}

/*****************************************************************************/

/*
 * This function will set an error string for the ARexx
 * application in the variable defined as <appname>.LASTERROR
 *
 * Note that this can only happen if there is an ARexx message...
 *
 * This returns TRUE if it worked, FALSE if it did not...
 */
short SetARexxLastError (struct AmigaGuideLib * ag, AREXXCONTEXT RexxContext, struct RexxMsg * rmsg,
			  char *ErrorString)
{
    register short OkFlag = FALSE;

    if (RexxContext)
	if (rmsg)
	    if (CheckRexxMsg ((struct Message *) rmsg))
	    {

		/*
		 * Note that SetRexxVar() has more than just a TRUE/FALSE return code, but for this "basic" case, we just care if
		 * it works or not.
		 */
		if (!SetRexxVar ((struct Message *) rmsg, RexxContext->ErrorName, ErrorString,
				 (long) strlen (ErrorString)))
		{
		    OkFlag = TRUE;
		}
	    }
    return (OkFlag);
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

short SendARexxMsg (struct AmigaGuideLib * ag, AREXXCONTEXT RexxContext, char *RString, short StringFile)
{
    register struct MsgPort *RexxPort;
    register struct RexxMsg *rmsg;
    register short flag = FALSE;

    if (RexxContext)
    {
	if (RString)
	{
	    if (rmsg = CreateRexxMsg (RexxContext->ARexxPort,
				      RexxContext->Extension,
				      RexxContext->PortName))
	    {
		rmsg->rm_Action = RXCOMM | (StringFile ? (1L << RXFB_STRING) : 0);

		if (rmsg->rm_Args[0] =
		    CreateArgstring (RString, (LONG) strlen (RString)))
		{
		    Forbid ();
		    if (RexxPort = FindPort (RXSDIR))
		    {
			PutMsg (RexxPort, (struct Message *) rmsg);
			RexxContext->Outstanding += 1;
			flag = TRUE;
		    }
		    else
		    {
			DeleteArgstring (rmsg->rm_Args[0]);
			DeleteRexxMsg (rmsg);
		    }
		    Permit ();
		}
		else
		{
		    DeleteRexxMsg (rmsg);
		}
	    }
	}
    }

    return (flag);
}

/*
 * This function closes down the ARexx context that was opened
 * with InitARexx...
 */
void FreeARexx (struct AmigaGuideLib * ag, AREXXCONTEXT RexxContext)
{
    register struct RexxMsg *rmsg;

    if (RexxContext)
    {
	/* Clear port name so it can't be found... */
	RexxContext->PortName[0] = '\0';

	/* Clean out any outstanding messages we had sent out... */
	while (RexxContext->Outstanding)
	{
	    WaitPort (RexxContext->ARexxPort);
	    while (rmsg = GetARexxMsg (ag, RexxContext))
	    {
		if (rmsg != REXX_RETURN_ERROR)
		{
		    /* Any messages that come now are blown away... */
		    SetARexxLastError (ag, RexxContext, rmsg, "99: Port Closed!");
		    ReplyARexxMsg (ag, RexxContext, rmsg, NULL, 100);
		}
	    }
	}

	/*
	 * Clean up the port and delete it...
	 */
	if (RexxContext->ARexxPort)
	{
	    while (rmsg = GetARexxMsg (ag, RexxContext))
	    {

		/*
		 * Any messages that still are coming in are "dead"  We just set the LASTERROR and reply an error of 100...
		 */
		SetARexxLastError (ag, RexxContext, rmsg, "99: Port Closed!");
		ReplyARexxMsg (ag, RexxContext, rmsg, NULL, 100);
	    }
	    DeletePort (ag, RexxContext->ARexxPort);
	}

	/* Free the memory of the RexxContext */
	FreeVec (RexxContext);
    }
}

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
VOID *InitARexx (struct AmigaGuideLib * ag, char *AppName, char *Extension, LONG unique)
{
    register AREXXCONTEXT RexxContext;
    register short count;
    register short loop;
    register char *tmp;

    if (RexxContext = AllocVec (sizeof (struct ARexxContext), MEMF_PUBLIC | MEMF_CLEAR))
    {
	tmp = RexxContext->Extension;
	for (loop = 0; (loop < 7) && (Extension[loop]); loop++)
	    *tmp++ = Extension[loop];
	*tmp = '\0';

	/*
	 * Set up the last error RVI name...
	 *
	 * This is <appname>.LASTERROR
	 */
	tmp = RexxContext->ErrorName;
	for (loop = 0; (loop < 16) && (AppName[loop]); loop++)
	    *tmp++ = ToUpper (AppName[loop]);
	strcpy (tmp, ".LASTERROR");

	/* Set up a port name... */
	tmp = RexxContext->PortName;
	for (loop = 0; (loop < 16) && (AppName[loop]); loop++)
	    *tmp++ = ToUpper (AppName[loop]);

	if (unique)
	{
	    Forbid ();
	    if (!(FindPort (RexxContext->PortName)))
		RexxContext->ARexxPort = CreatePort (ag, RexxContext->PortName, NULL);
	    Permit ();
	}
	else
	{
	    *tmp++ = '.';

	    /* We need to make a unique port name... */
	    Forbid ();
	    for (count = 1, RexxContext->ARexxPort = (VOID *) 1; RexxContext->ARexxPort; count++)
	    {
#if 1
		stci_d (tmp, count);
#endif
		RexxContext->ARexxPort = FindPort (RexxContext->PortName);
	    }

	    RexxContext->ARexxPort = CreatePort (ag, RexxContext->PortName, NULL);
	    Permit ();
	}

	if (!(RexxContext->ARexxPort))
	{
	    FreeARexx (ag, RexxContext);
	    RexxContext = NULL;
	}
    }

    return ((VOID *) RexxContext);
}
