#ifndef DOS_DOSTAGS_H
#define DOS_DOSTAGS_H

/*** dostags.h **************************************************************
 *
 *  dostags.h - include file for dos routines that use tags
 *
 *
 ****************************************************************************
 * CONFIDENTIAL and PROPRIETARY
 * Copyright (C) 1989, COMMODORE-AMIGA, INC.
 * All Rights Reserved
 ****************************************************************************/

#ifndef EXEC_TYPES_H
#include "exec/types.h"
#endif

#ifndef UTILITY_TAGITEM_H
#include "utility/tagitem.h"
#endif

/* definitions for the System() call */

enum {	SYS_Dummy = TAG_USER + 32,
	SYS_Input,			/* specifies the input filehandle  */
	SYS_Output,			/* specifies the output filehandle */
/*	SYS_Error, */
};

/* definitions for the CreateNewProc() call */
/* you MUST specify one of NP_Seglist or NP_Entry.  All else is optional. */

enum {	NP_Dummy = TAG_USER + 1000,
	NP_Seglist,		/* seglist of code to run for the process  */
	NP_FreeSeglist,		/* free seglist on exit - only valid for   */
				/* for NP_Seglist.  Default is TRUE.	   */
	NP_Entry,		/* entry point to run - mutually exclusive */
				/* with NP_Seglist! */
	NP_Input,		/* filehandle - default is Open("NIL:"...) */
	NP_Output,		/* filehandle - default is Open("NIL:"...) */
	NP_CloseInput,		/* close input filehandle on exit	   */
				/* default TRUE				   */
	NP_CloseOutput,		/* close output filehandle on exit	   */
				/* default TRUE				   */
	NP_Error,		/* filehandle - default is Open("NIL:"...) */
	NP_CloseError,		/* close error filehandle on exit	   */
				/* default TRUE				   */
	NP_CurrentDir,		/* lock - default is parent's current dir  */
	NP_StackSize,		/* stacksize for process - default 4000    */
	NP_Name,		/* name for process - default "New Process"*/
	NP_Priority,		/* priority - default same as parent	   */
	NP_ConsoleTask,		/* consoletask - default same as parent    */
	NP_WindowPtr,		/* window ptr - default is same as parent  */
	NP_HomeDir,		/* home directory - default current dir	   */
	NP_CopyVars,		/* boolean to copy local vars-default TRUE */
	NP_Cli,			/* create cli structure - default FALSE    */
	NP_Path,		/* path - default is copy of parents path  */
				/* only valid if a cli process!            */
	NP_CommandName,		/* commandname - valid only for CLI	   */
	NP_Arguments,		/* cstring of arguments - passed with str  */
				/* in a0, length in d0.  (copied and freed */
				/* on exit.  Default is empty string.      */
/* FIX! should this be only for cli's? */
	NP_NotifyOnDeath,	/* notify parent on death - default FALSE  */
	NP_Synchronous,		/* don't return until process finishes -   */
				/* default FALSE.			   */
	NP_ExitCode,		/* code to be called on process exit       */
	NP_ExitData,		/* optional argument for NP_EndCode rtn -  */
				/* default NULL				   */
};

/* tags for AllocDosObject */
/* no tags are defined yet for AllocDosObject */

/* tags for NewLoadSeg */
/* no tags are defined yet for NewLoadSeg */

#endif /* DOS_DOSTAGS_H */
