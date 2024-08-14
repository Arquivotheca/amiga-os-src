// This file contains all the possible Postscript error returns.
// March 1991 L.V. © CBM-Amiga
// Submitted to RCS on 05/APR/91

#ifndef	ERRORS_H
#define	ERRORS_H

enum ps_errors {
 ERR_OK,					// This means NO ERROR

 ERR_dictfull,				//
 ERR_dictstackoverflow,		//
 ERR_dictstackunderflow,	//
 ERR_execstackoverflow,		//
 ERR_handleerror,			//
 ERR_interrupt,				//
 ERR_invalidaccess,			//
 ERR_invalidexit,			//
 ERR_invalidfileaccess,		//
 ERR_invalidfont,			//
 ERR_invalidrestore,		//
 ERR_ioerror,				//
 ERR_limitcheck,			//
 ERR_nocurrentpoint,		//
 ERR_rangecheck,			//
 ERR_stackoverflow,			//
 ERR_stackunderflow,		//
 ERR_syntaxerror,			//
 ERR_timeout,				//
 ERR_typecheck,				//
 ERR_undefined,				//
 ERR_undefinedfilename,		//
 ERR_undefinedresult,		//
 ERR_unmatchedmark,			//
 ERR_unregistered,			//
 ERR_VMerror,				//
 ERR_invalidcontext,		// DPS addition
 ERR_invalidid,				// DPS addition

 ERR_QUIT_INTERPRETER,		// used by the "quit" operator
 ERR_NOT_IMPLEMENTED,		// development error only, not to be included in final release

 NUM_ERRORS };				// analog to sizeof().... NOT a real error.

typedef	int error;

#endif
