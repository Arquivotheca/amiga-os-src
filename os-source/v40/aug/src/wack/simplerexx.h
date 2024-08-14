/*
 * Amiga Grand Wack
 *
 * $Id: simplerexx.h,v 39.1 93/05/03 14:39:44 peter Exp $
 *
 * Mike Sinz's simple ARexx interface...
 *
 */

#ifndef	SIMPLE_REXX_H
#define	SIMPLE_REXX_H

#include	<exec/types.h>
#include	<exec/nodes.h>
#include	<exec/lists.h>
#include	<exec/ports.h>

#include	<rexx/storage.h>
#include	<rexx/rxslib.h>

/*
 * A structure for the ARexx handler context
 * This is *VERY* *PRIVATE* and should not be touched...
 */
struct	ARexxContext
{
struct	MsgPort	*ARexxPort;	/* The port messages come in at... */
struct	Library	*RexxSysBase;	/* We will hide the library pointer here... */
struct	Library	*SysBase;	/* Storage for ExecBase */
	long	Outstanding;	/* The count of outstanding ARexx messages... */
	char	*PortName;	/* The port name goes here... */
	char	Extension[8];	/* Default file name extension... */
};

/*
 * The value of RexxMsg (from GetARexxMsg) if there was an error returned
 */
#define	REXX_RETURN_ERROR	((struct RexxMsg *)-1L)

/*
 * This function closes down the ARexx context that was opened
 * with InitARexx...
 */
void FreeARexx(struct ARexxContext *);

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
struct ARexxContext *InitARexx(struct MsgPort *,char *,char *);

/*
 * This function returns the signal mask that the Rexx port is
 * using.  It returns NULL if there is no signal...
 *
 * Use this signal bit in your Wait() loop...
 */
ULONG ARexxSignal(struct ARexxContext *);

/*
 * This function returns a structure that contains the commands sent from
 * ARexx...  You will need to parse it and return the structure back
 * so that the memory can be freed...
 *
 * This returns NULL if there was no message...
 */
struct RexxMsg *GetARexxMsg(struct ARexxContext *);

/*
 * Use this to return a ARexx message...
 *
 * If you wish to return something, it must be in the RString.
 * If you wish to return an Error, it must be in the Error.
 */
void ReplyARexxMsg(struct ARexxContext *,struct RexxMsg *,char *,LONG);

/*
 * This function will send a string to ARexx...
 *
 * The default host port will be that of your task...
 *
 * If you set StringFile to TRUE, it will set that bit for the message...
 *
 * Returns TRUE if it send the message, FALSE if it did not...
 */
LONG SendARexxMsg(struct ARexxContext *,char *,LONG);

#endif	/* SIMPLE_REXX_H */
