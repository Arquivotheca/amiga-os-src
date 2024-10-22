/* rexxcb.c
 * Dispatcher for the RexxUtil REXX function library
 */

#include "datatypesbase.h"
#include <dos/datetime.h>
#include <rexx/storage.h>
#include <rexx/rxslib.h>
#include <rexx/errors.h>
#include <clib/rexxsyslib_protos.h>
#include <pragmas/rexxsyslib_pragmas.h>

/*****************************************************************************/

#define	DB(x)	;

/*****************************************************************************/

LONG __stdargs SetRexxVar (struct RexxMsg *, char *, char *, ULONG);

/*****************************************************************************/

#define REXX_NO_ERROR		0
#define REXX_PROGRAM_NOT_FOUND	ERR10_001	/* program not found */
#define REXX_NO_MEMORY		ERR10_003	/* no memory available */
#define REXX_NO_LIBRARY		ERR10_014	/* required library not found */
#define REXX_WRONG_NUMBER_ARGS	ERR10_017	/* wrong number of arguments */
#define REXX_BAD_ARGS		ERR10_018	/* invalid argument to function */
#define REXX_INVALID_OPERAND	ERR10_048	/* invalid operand */

/*****************************************************************************/

/* structure for holding return information */
struct RetBlock
{
    LONG Type;
    union
    {
	LONG IntVal;
	STRPTR ArgString;
    } values;
};

/* variable types */
#define	RBINT		1
#define	RBARGSTRING	2

/*****************************************************************************/

#define ARG2(rmp) (rmp->rm_Args[2])
#define ARG3(rmp) (rmp->rm_Args[3])
#define ARG4(rmp) (rmp->rm_Args[4])
#define ARG5(rmp) (rmp->rm_Args[5])

/*****************************************************************************/

/* structure for holding the function records */
struct RexxCmds
{
   STRPTR rc_FName;				/* function name */
   LONG (*rc_Func)(struct RexxMsg *, struct RetBlock *, struct DataTypesLib *);
   STRPTR rc_Template;				/* option template */
   WORD rc_NumArgs;				/* number of arguments */
};

/* array of functions that this library implements */
struct RexxCmds rc_table[] =
{
    {"EXAMINEDT", rExamineDT, "FILE,VARNAME,[STEM/s,VAR/s]", 3},

    {NULL,},
};

/****** datatypes.library/--rexxhost-- ****************************************
*
*   HOST INTERFACE
*	datatypes.library provides an ARexx function host interface that
*	enables ARexx programs to take advantage of the features of data
*	types.  The functions provided by the interface are directly
*	related to the functions described herein, with the differences
*	mostly being in the way they are called.
*
*	The function host library vector is located at offset -30 from
*	the library. This is the value you provide to ARexx in the
*	AddLib() function call.
*
*   FUNCTIONS
*	ExamineDT(FILENAME/A,VARIABLENAME,STEM/S,VAR/S)
*
*   EXAMPLE
*	\* datatypes.rexx *\
*	PARSE ARG fname
*	OPTIONS RESULTS
*
*	\* Load the datatypes.library as a function host *\
*	IF ~SHOW('L','datatypes.library') THEN
*	   CALL ADDLIB('datatypes.library',0,-30)
*
*	   IF fname="" THEN DO
*	      SAY "Usage:"
*	      SAY "  rx datatypes <filename>"
*	      EXIT
*	   END
*
*	   SAY 'var test'
*	   type = ExamineDT(fname,,VAR)
*	   SAY type
*
*	   SAY 'stem test'
*	   CALL ExamineDT(fname,dtstem.,STEM)
*
*	   SAY '      Disk Key:' dtstem.DiskKey
*	   SAY 'Dir Entry Type:' dtstem.DirEntryType
*	   SAY '     File Name:' dtstem.FileName
*	   SAY '    Protection:' dtstem.Protection
*	   SAY '    Entry Type:' dtstem.EntryType
*	   SAY '          Size:' dtstem.Size
*	   SAY '        Blocks:' dtstem.NumBlocks
*	   SAY '          Date:' dtstem.Date
*	   SAY '       Comment:' dtstem.Comment
*	   SAY '          Type:' dtstem.BaseType
*	   SAY '     File Type:' dtstem.DataType
*	   SAY '     Base Name:' dtstem.BaseName
*	EXIT
*
*******************************************************************************
*
* Created:  27-Feb-92, David N. Junod
*
*/

LONG __stdargs RLDispatch (struct RexxMsg * rmsg, STRPTR * result)
{
    struct DataTypesLib * dtl = (struct DataTypesLib *) getreg (REG_A6);
    struct RetBlock block = {NULL};
    struct RexxCmds *rc;
    UBYTE buffer[30];
    WORD numargs;
    LONG retcode;
    WORD i;

    *result = NULL;
    if (!(dtl->dtl_RexxSysBase = OpenLibrary ("rexxsyslib.library", 0)))
	return (REXX_NO_LIBRARY);

    retcode = REXX_PROGRAM_NOT_FOUND;
    rc = NULL;
    i = 0L;

    while (!rc && rc_table[i].rc_FName)
    {
	if ((Stricmp (ARG0(rmsg), rc_table[i].rc_FName)) == 0)
	{
	    rc = &(rc_table[i]);
	}
	i++;
    }

    if (rc)
    {
	/* get the number of arguments passed */
	numargs = (rmsg->rm_Action) & 0xFF;

	/* see if first variable is a ? */
	if ((numargs == 1) && (Stricmp (ARG1 (rmsg), "?") == 0))
	{
	    *result = CreateArgstring (rc->rc_Template, strlen (rc->rc_Template));
	    retcode = REXX_NO_ERROR;
	}
	else if (numargs != rc->rc_NumArgs)
	{
	    retcode = REXX_WRONG_NUMBER_ARGS;
	}
	else if ((retcode = (*(rc->rc_Func)) (rmsg, &block, dtl)) == REXX_NO_ERROR)
	{
	    switch (block.Type)
	    {
		case RBINT:
		    sprintf (buffer, "%ld", (LONG) block.values.IntVal);
		    *result = CreateArgstring (buffer, strlen (buffer));
		    break;

		case RBARGSTRING:
		    *result = block.values.ArgString;
		    break;
	    }

	    if (*result)
		retcode = REXX_NO_ERROR;
	    else
		retcode = REXX_NO_MEMORY;
	}
    }
    CloseLibrary (dtl->dtl_RexxSysBase);
    return (retcode);
}

LONG rExamineDT (struct RexxMsg * rmsg, struct RetBlock * block, struct DataTypesLib * dtl)
{
    LONG retval = REXX_INVALID_OPERAND;
    struct DataTypeHeader *dth;
    struct FileInfoBlock *fib;
    UBYTE date[LEN_DATSTRING];
    UBYTE time[LEN_DATSTRING];
    UBYTE day[LEN_DATSTRING];
    struct DataType *dtn;
    struct DateTime dat;
    STRPTR buffer;
    STRPTR type;
    STRPTR stem;
    BPTR lock;
    WORD i;

    /* return the value as an ArgString */
    block->Type = RBINT;
    block->values.IntVal = FALSE;

    if (lock = Lock (ARG1 (rmsg), ACCESS_READ))
    {
	if (fib = AllocVec (sizeof (struct FileInfoBlock), MEMF_CLEAR))
	{
	    if (dtn = ExamineDT (dtl, lock, fib))
	    {
		dth = dtn->dtn_Header;

		block->values.IntVal = TRUE;
		retval = REXX_NO_ERROR;

		if ((ARG3 (rmsg) == NULL) || (Stricmp (ARG3 (rmsg), "VAR")) == 0)
		{
		    /* return the value as an ArgString */
		    block->Type = RBARGSTRING;
		    block->values.ArgString = CreateArgstring (dth->dth_Name, strlen (dth->dth_Name));
		}
		else if ((Stricmp (ARG3 (rmsg), "STEM")) == 0)
		{
		    if (buffer = AllocVec (1024, MEMF_CLEAR))
		    {
			stem = &buffer[512];
			switch (dth->dth_Flags & 0xF)
			{
			    case DTF_BINARY:
				type = "binary";
				break;
			    case DTF_ASCII:
				type = "ascii";
				break;
			    case DTF_IFF:
				type = "iff";
				break;
			    default:
				type = "other";
				break;

			}

			sprintf (buffer, "%ld", fib->fib_DiskKey);
			sprintf (stem, "%sDISKKEY", ARG2 (rmsg));
			SetRexxVar (rmsg, stem, buffer, strlen (buffer));

			sprintf (buffer, "%ld", fib->fib_DirEntryType);
			sprintf (stem, "%sDIRENTRYTYPE", ARG2 (rmsg));
			SetRexxVar (rmsg, stem, buffer, strlen (buffer));

			NameFromLock (lock, buffer, 510);
			sprintf (stem, "%sFILENAME", ARG2 (rmsg));
			SetRexxVar (rmsg, stem, buffer, strlen (buffer));

			strcpy(buffer, "hsparwed");
			for (i = 0; i < 8; i++)
			{
			    if ((fib->fib_Protection & (1 << (7-i))) == ((i<4) ? NULL : (1 << (7-i))))
			    {
				buffer[i] = '-';
			    }
			}
			sprintf (stem, "%sPROTECTION", ARG2 (rmsg));
			SetRexxVar (rmsg, stem, buffer, strlen (buffer));

			sprintf (buffer, "%ld", fib->fib_EntryType);
			sprintf (stem, "%sENTRYTYPE", ARG2 (rmsg));
			SetRexxVar (rmsg, stem, buffer, strlen (buffer));

			sprintf (buffer, "%ld", fib->fib_Size);
			sprintf (stem, "%sSIZE", ARG2 (rmsg));
			SetRexxVar (rmsg, stem, buffer, strlen (buffer));

			sprintf (buffer, "%ld", fib->fib_NumBlocks);
			sprintf (stem, "%sNUMBLOCKS", ARG2 (rmsg));
			SetRexxVar (rmsg, stem, buffer, strlen (buffer));

                        dat.dat_Stamp   = *(&fib->fib_Date);
                        dat.dat_Format  = FORMAT_DOS;
			dat.dat_Flags   = NULL;
			dat.dat_StrDay  = day;
                        dat.dat_StrDate = date;
                        dat.dat_StrTime = time;
                        if (DateToStr (&dat))
                        {
                            sprintf (buffer, "%s %s %s", day, date, time);
                            sprintf (stem, "%sDATE", ARG2 (rmsg));
                            SetRexxVar (rmsg, stem, buffer, strlen (buffer));
                        }

			sprintf (stem, "%sCOMMENT", ARG2 (rmsg));
			SetRexxVar (rmsg, stem, fib->fib_Comment, strlen (fib->fib_Comment));

			sprintf (stem, "%sOWNERUID", ARG2 (rmsg));
			sprintf (buffer, "%ld", (ULONG)fib->fib_OwnerUID);
			SetRexxVar (rmsg, stem, buffer, strlen (buffer));

			sprintf (stem, "%sOWNERGID", ARG2 (rmsg));
			sprintf (buffer, "%ld", (ULONG)fib->fib_OwnerGID);
			SetRexxVar (rmsg, stem, buffer, strlen (buffer));

			sprintf (stem, "%sBASETYPE", ARG2 (rmsg));
			SetRexxVar (rmsg, stem, type, strlen (type));

			sprintf (stem, "%sDATATYPE", ARG2 (rmsg));
			SetRexxVar (rmsg, stem, dth->dth_Name, strlen (dth->dth_Name));

			sprintf (stem, "%sBASENAME", ARG2 (rmsg));
			SetRexxVar (rmsg, stem, dth->dth_BaseName, strlen (dth->dth_BaseName));

			sprintf (stem, "%sGROUPID", ARG2 (rmsg));
			IDtoStr (dth->dth_GroupID, buffer);
			SetRexxVar (rmsg, stem, buffer, strlen (buffer));

			sprintf (stem, "%sID", ARG2 (rmsg));
			IDtoStr (dth->dth_ID, buffer);
			SetRexxVar (rmsg, stem, buffer, strlen (buffer));

			FreeVec (buffer);
		    }
		}
	    }
	    FreeVec (fib);
	}
	UnLock (lock);
    }
    return (retval);
}
