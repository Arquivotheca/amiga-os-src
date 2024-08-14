/* arexx.c
 *
 */

#include "amigaguidebase.h"

/*****************************************************************************/

struct MsgPort *CreatePort (struct AGLib *ag, STRPTR name, BYTE pri)
{
    struct MsgPort *mp;

    if (mp = CreateMsgPort ())
    {
	mp->mp_Node.ln_Name = name;
	mp->mp_Node.ln_Pri = pri;

	Forbid ();
	if (!FindPort (name))
	{
	    AddPort (mp);
	}
	else
	{
	    DeleteMsgPort (mp);
	    mp = NULL;
	}
	Permit ();
    }
    return (mp);
}

/*****************************************************************************/

VOID DeletePort (struct AGLib * ag, struct MsgPort * mp)
{
    if (mp)
    {
	if (mp->mp_Node.ln_Name)
	    RemPort (mp);
	DeleteMsgPort (mp);
    }
}

/*****************************************************************************/

/* This function returns the signal mask that the Rexx port is
 * using.  It returns NULL if there is no signal...
 *
 * Use this signal bit in your Wait() loop...
 */
ULONG ARexxSignal (struct AGLib *ag, struct ARexxContext *rc)
{
    if (rc)
	return ((ULONG) (1L << (rc->ARexxPort->mp_SigBit)));
    return (0);
}

/*****************************************************************************/

/* This function returns a pointer to the ARexx message port */
struct MsgPort *ARexxPort (struct AGLib *ag, struct ARexxContext *rc)
{
    if (rc)
	return (rc->ARexxPort);
    return (NULL);
}

/*****************************************************************************/

/*
 * This function returns a structure that contains the commands sent from
 * ARexx...  You will need to parse it and return the structure back
 * so that the memory can be freed...
 *
 * This returns NULL if there was no message...
 */
struct RexxMsg *GetARexxMsg (struct AGLib *ag, struct ARexxContext *rc)
{
    struct RexxMsg *tmp = NULL;
    short flag;

    if (rc)
    {
	if (tmp = (struct RexxMsg *) GetMsg (rc->ARexxPort))
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

		/* Free the arguments and the message... */
		DeleteArgstring (tmp->rm_Args[0]);
		DeleteRexxMsg (tmp);
		rc->Outstanding -= 1;

		/* Return the error if there was one... */
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
void ReplyARexxMsg (struct AGLib *ag, struct ARexxContext *rc, struct RexxMsg *rmsg, char *RString, LONG Error)
{

    if (rc)
	if (rmsg)
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

/*****************************************************************************/

/*
 * This function will set an error string for the ARexx
 * application in the variable defined as <appname>.LASTERROR
 *
 * Note that this can only happen if there is an ARexx message...
 *
 * This returns TRUE if it worked, FALSE if it did not...
 */
short SetARexxLastError (struct AGLib *ag, struct ARexxContext *rc, struct RexxMsg *rmsg,
			 char *ErrorString)
{
    short OkFlag = FALSE;

    if (rc)
	if (rmsg)
	    if (CheckRexxMsg ((struct Message *) rmsg))
	    {

		/*
		 * Note that SetRexxVar() has more than just a TRUE/FALSE return code, but for this "basic" case, we just care if
		 * it works or not.
		 */
		if (!SetRexxVar ((struct Message *) rmsg, rc->ErrorName, ErrorString,
				 (long) strlen (ErrorString)))
		{
		    OkFlag = TRUE;
		}
	    }
    return (OkFlag);
}

/*****************************************************************************/

/*
 * This function will send a string to ARexx...
 *
 * The default host port will be that of your task...
 *
 * If you set StringFile to TRUE, it will set that bit for the message...
 *
 * Returns TRUE if it send the message, FALSE if it did not...
 */

short SendARexxMsg (struct AGLib *ag, struct ARexxContext *rc, char *RString, short StringFile)
{
    struct MsgPort *RexxPort;
    struct RexxMsg *rmsg;
    short flag = FALSE;

    if (rc)
    {
	if (RString)
	{
	    if (rmsg = CreateRexxMsg (rc->ARexxPort,
				      rc->Extension,
				      rc->PortName))
	    {
		rmsg->rm_Action = RXCOMM | (StringFile ? (1L << RXFB_STRING) : 0);

		if (rmsg->rm_Args[0] =
		    CreateArgstring (RString, (LONG) strlen (RString)))
		{
		    Forbid ();
		    if (RexxPort = FindPort (RXSDIR))
		    {
			PutMsg (RexxPort, (struct Message *) rmsg);
			rc->Outstanding += 1;
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

/*****************************************************************************/

/*
 * This function closes down the ARexx context that was opened
 * with InitARexx...
 */
void FreeARexx (struct AGLib *ag, struct ARexxContext *rc)
{
    struct RexxMsg *rmsg;

    if (rc)
    {
	/* Clear port name so it can't be found... */
	rc->PortName[0] = '\0';

	/* Clean out any outstanding messages we had sent out... */
	while (rc->Outstanding)
	{
	    WaitPort (rc->ARexxPort);
	    while (rmsg = GetARexxMsg (ag, rc))
	    {
		if (rmsg != REXX_RETURN_ERROR)
		{
		    /* Any messages that come now are blown away... */
		    SetARexxLastError (ag, rc, rmsg, "99: Port Closed!");
		    ReplyARexxMsg (ag, rc, rmsg, NULL, 100);
		}
	    }
	}

	/*
	 * Clean up the port and delete it...
	 */
	if (rc->ARexxPort)
	{
	    while (rmsg = GetARexxMsg (ag, rc))
	    {

		/*
		 * Any messages that still are coming in are "dead"  We just set the LASTERROR and reply an error of 100...
		 */
		SetARexxLastError (ag, rc, rmsg, "99: Port Closed!");
		ReplyARexxMsg (ag, rc, rmsg, NULL, 100);
	    }
	    DeletePort (ag, rc->ARexxPort);
	}

	/* Free the memory of the rc */
	FreeVec (rc);
    }
}

/*****************************************************************************/

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
VOID *InitARexx (struct AGLib *ag, char *AppName, char *Extension, LONG unique)
{
    struct ARexxContext *rc;
    short loop, count;
    char *tmp;

    if (rc = AllocVec (sizeof (struct ARexxContext), MEMF_PUBLIC | MEMF_CLEAR))
    {
	tmp = rc->Extension;
	for (loop = 0; (loop < 7) && (Extension[loop]); loop++)
	    *tmp++ = Extension[loop];
	*tmp = '\0';

	/* Set up the last error RVI name... This is <appname>.LASTERROR */
	if (AppName)
	{
	    tmp = rc->ErrorName;
	    for (loop = 0; (loop < 16) && (AppName[loop]); loop++)
		*tmp++ = ToUpper (AppName[loop]);
	    strcpy (tmp, ".LASTERROR");
	}
	else
	    strcpy (rc->ErrorName, "AGDAEMON.LASTERROR");

	if (AppName == NULL)
	{
	    rc->ARexxPort = CreateMsgPort ();
	}
	else
	{
	    /* Set up a port name... */
	    tmp = rc->PortName;
	    for (loop = 0; (loop < 16) && (AppName[loop]); loop++)
		*tmp++ = ToUpper (AppName[loop]);

	    if (unique)
	    {
		Forbid ();
		if (!(FindPort (rc->PortName)))
		    rc->ARexxPort = CreatePort (ag, rc->PortName, NULL);
		Permit ();
	    }
	    else
	    {
		*tmp++ = '.';

		/* We need to make a unique port name... */
		Forbid ();
		for (count = 1, rc->ARexxPort = (VOID *) 1; rc->ARexxPort; count++)
		{
		    stci_d (tmp, count);
		    rc->ARexxPort = FindPort (rc->PortName);
		}

		rc->ARexxPort = CreatePort (ag, rc->PortName, NULL);
		Permit ();
	    }
	}

	if (!(rc->ARexxPort))
	{
	    FreeARexx (ag, rc);
	    rc = NULL;
	}
    }

    return ((VOID *) rc);
}
