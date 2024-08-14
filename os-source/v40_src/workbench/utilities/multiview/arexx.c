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

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/ports.h>
#include <exec/memory.h>
#include <rexx/storage.h>
#include <rexx/rxslib.h>
#include <string.h>

#include <clib/macros.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/utility_protos.h>
#include <clib/rexxsyslib_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/rexxsyslib_pragmas.h>

/*****************************************************************************/

struct ARexxContext
{
    struct Library	*ac_SysBase;
    struct Library	*ac_UtilityBase;
    struct Library	*ac_RexxSysBase;

    struct MsgPort	*ARexxPort;		/* The port messages come in at... */
    long		 Outstanding;		/* The count of outstanding ARexx messages... */
    char		 PortName[24];		/* The port name goes here... */
    char		 ErrorName[28];		/* The name of the <base>.LASTERROR... */
    char		 Extension[8];		/* Default file name extension... */
};

/*****************************************************************************/

#define	UtilityBase	ac->ac_UtilityBase
#define	RexxSysBase	ac->ac_RexxSysBase

/*****************************************************************************/

#define	AREXXCONTEXT	struct ARexxContext *

/*****************************************************************************/

#define	REXX_RETURN_ERROR	((struct RexxMsg *)-1L)

/*****************************************************************************/

BOOL __stdargs CheckRexxMsg( struct Message *rexxmsg );
LONG __stdargs GetRexxVar( struct Message *rexxmsg, UBYTE *name, UBYTE **result );
LONG __stdargs SetRexxVar( struct Message *rexxmsg, UBYTE *name, UBYTE *value, long length);

/*****************************************************************************/

/* arexx.c */
ULONG ARexxSignal ( AREXXCONTEXT ac );
struct RexxMsg *GetARexxMsg ( AREXXCONTEXT ac );
void ReplyARexxMsg ( AREXXCONTEXT ac , struct RexxMsg *rmsg , char *RString , LONG Error );
short SetARexxLastError ( AREXXCONTEXT ac , struct RexxMsg *rmsg , char *ErrorString );
short SendARexxMsg ( AREXXCONTEXT ac , char *RString , short StringFile );
void FreeARexx ( AREXXCONTEXT ac );
AREXXCONTEXT InitARexx ( char *AppName , char *Extension , LONG unique );

/*****************************************************************************/

static struct MsgPort *CreatePort (STRPTR name, LONG pri)
{
    struct ExecBase *SysBase = (*((struct ExecBase **) 4));
    struct MsgPort *mp;

    if (mp = CreateMsgPort ())
    {
	mp->mp_Node.ln_Name = name;
	mp->mp_Node.ln_Pri  = (BYTE) pri;

	AddPort (mp);
    }
    return (mp);
}

/*****************************************************************************/

static void DeletePort (struct MsgPort *mp)
{
    struct ExecBase *SysBase = (*((struct ExecBase **) 4));

    if (mp)
    {
	if (mp->mp_Node.ln_Name)
	    RemPort (mp);
	DeleteMsgPort (mp);
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
AREXXCONTEXT InitARexx (char *AppName, char *Extension, LONG unique)
{
    struct ExecBase *SysBase = (*((struct ExecBase **) 4));
    register AREXXCONTEXT ac = NULL;
    register short loop;
    register short count;
    register char *tmp;

    if (ac = AllocVec (sizeof (struct ARexxContext), MEMF_PUBLIC | MEMF_CLEAR))
    {
	ac->ac_SysBase = SysBase;

#define	SysBase		 ac->ac_SysBase

	UtilityBase = OpenLibrary ("utility.library", 37);

	if (RexxSysBase = OpenLibrary ("rexxsyslib.library", 0L))
	{
	    /* Set up the extension... */
	    if (!Extension)
		Extension = "rexx";

	    tmp = ac->Extension;
	    for (loop = 0; (loop < 7) && (Extension[loop]); loop++)
		*tmp++ = Extension[loop];
	    *tmp = '\0';

	    /*
	     * Set up the last error RVI name...
	     *
	     * This is <appname>.LASTERROR
	     */
	    tmp = ac->ErrorName;
	    for (loop = 0; (loop < 16) && (AppName[loop]); loop++)
		*tmp++ = ToUpper (AppName[loop]);
	    strcpy (tmp, ".LASTERROR");

	    /* Set up a port name...  */
	    tmp = ac->PortName;
	    for (loop = 0; (loop < 16) && (AppName[loop]); loop++)
		*tmp++ = ToUpper (AppName[loop]);

	    if (unique)
	    {
		Forbid ();
		if (!(FindPort (ac->PortName)))
		    ac->ARexxPort = CreatePort (ac->PortName, NULL);
		Permit ();
	    }
	    else
	    {
		*tmp++ = '.';

		/* We need to make a unique port name... */
		Forbid ();
		for (count = 1, ac->ARexxPort = (VOID *) 1;
		     ac->ARexxPort; count++)
		{
		    stci_d (tmp, count);
		    ac->ARexxPort =
			FindPort (ac->PortName);
		}

		ac->ARexxPort = CreatePort (ac->PortName, NULL);
		Permit ();
	    }
	}

	if ((!(RexxSysBase)) || (!(ac->ARexxPort)))
	{
	    FreeARexx (ac);
	    ac = NULL;
	}
    }

    return (ac);
}

/*****************************************************************************/

short SetARexxStem (AREXXCONTEXT ac, struct RexxMsg *rmsg, STRPTR name, STRPTR value)
{
    register short OkFlag = FALSE;

    if (ac)
	if (rmsg)
	    if (CheckRexxMsg ((struct Message *) rmsg))
		if (!SetRexxVar ((struct Message *) rmsg, name, value, strlen (value)))
		    OkFlag = TRUE;
    return (OkFlag);
}

/*****************************************************************************/

/*
 * This function returns the signal mask that the Rexx port is
 * using.  It returns NULL if there is no signal...
 *
 * Use this signal bit in your Wait() loop...
 */
ULONG ARexxSignal (AREXXCONTEXT ac)
{
    ULONG tmp = NULL;

    if (ac)
    {
	tmp = 1L << (ac->ARexxPort->mp_SigBit);
    }

    return (tmp);
}

/*****************************************************************************/

STRPTR ARexxPortName (AREXXCONTEXT ac)
{
    if (ac)
	return ac->ARexxPort->mp_Node.ln_Name;
    return NULL;
}

/*****************************************************************************/

/*
 * This function returns a structure that contains the commands sent from
 * ARexx...  You will need to parse it and return the structure back
 * so that the memory can be freed...
 *
 * This returns NULL if there was no message...
 */
struct RexxMsg *GetARexxMsg (AREXXCONTEXT ac)
{
    register struct RexxMsg *tmp = NULL;
    register short flag;

    if (ac)
	if (tmp = (struct RexxMsg *) GetMsg (ac->ARexxPort))
	{
	    if (tmp->rm_Node.mn_Node.ln_Type == NT_REPLYMSG)
	    {

		/*
		 * If we had sent a command, it would come this way...
		 *
		 * Since we don't in this simple example, we just throw away
		 * anything that looks "strange"
		 */
		flag = FALSE;
		if (tmp->rm_Result1)
		    flag = TRUE;

		/*
		 * Free the arguments and the message...
		 */
		DeleteArgstring (tmp->rm_Args[0]);
		DeleteRexxMsg (tmp);
		ac->Outstanding -= 1;

		/*
		 * Return the error if there was one...
		 */
		tmp = flag ? REXX_RETURN_ERROR : NULL;
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
void ReplyARexxMsg (AREXXCONTEXT ac, struct RexxMsg * rmsg,
		     char *RString, LONG Error)
{

    if (ac)
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
short SetARexxLastError (AREXXCONTEXT ac, struct RexxMsg * rmsg, char *ErrorString)
{
    register short OkFlag = FALSE;

    if (ac)
	if (rmsg)
	    if (CheckRexxMsg ((struct Message *) rmsg))
	    {

		/*
		 * Note that SetRexxVar() has more than just a TRUE/FALSE
		 * return code, but for this "basic" case, we just care if it
		 * works or not.
		 */
		if (!SetRexxVar ((struct Message *) rmsg, ac->ErrorName, ErrorString,
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

short SendARexxMsg (AREXXCONTEXT ac, char *RString,
		     short StringFile)
{
    register struct MsgPort *RexxPort;
    register struct RexxMsg *rmsg;
    register short flag = FALSE;

    if (ac)
    {
	if (RString)
	{
	    if (rmsg = CreateRexxMsg (ac->ARexxPort,
				      ac->Extension,
				      ac->PortName))
	    {
		rmsg->rm_Action = RXCOMM|(StringFile?(1L << RXFB_STRING):0);

		if (rmsg->rm_Args[0] =
			CreateArgstring (RString, (LONG)strlen (RString)))
		{
		    Forbid ();
		    if (RexxPort = FindPort (RXSDIR))
		    {
			PutMsg (RexxPort, (struct Message *) rmsg);
			ac->Outstanding += 1;
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
void FreeARexx (AREXXCONTEXT ac)
{
    register struct RexxMsg *rmsg;

    if (ac)
    {

	/*
	 * Clear port name so it can't be found...
	 */
	ac->PortName[0] = '\0';

	/*
	 * Clean out any outstanding messages we had sent out...
	 */
	while (ac->Outstanding)
	{
	    WaitPort (ac->ARexxPort);
	    while (rmsg = GetARexxMsg (ac))
	    {
		if (rmsg != REXX_RETURN_ERROR)
		{

		    /*
		     * Any messages that come now are blown away...
		     */
		    SetARexxLastError (ac, rmsg,
				       "99: Port Closed!");
		    ReplyARexxMsg (ac, rmsg,
				   NULL, 100);
		}
	    }
	}

	/*
	 * Clean up the port and delete it...
	 */
	if (ac->ARexxPort)
	{
	    while (rmsg = GetARexxMsg (ac))
	    {

		/*
		 * Any messages that still are coming in are "dead"  We just
		 * set the LASTERROR and reply an error of 100...
		 */
		SetARexxLastError (ac, rmsg,
				   "99: Port Closed!");
		ReplyARexxMsg (ac, rmsg, NULL, 100);
	    }
	    DeletePort (ac->ARexxPort);
	}

	CloseLibrary (RexxSysBase);
	CloseLibrary (UtilityBase);

	/* Free the memory of the ac */
	FreeVec (ac);
    }
}
