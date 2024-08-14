/************************************************************************

  Module :	Postscript "String Operators"		© Commodore-Amiga
			Written by L. Vanhelsuwe (started March 1991)

  Purpose:	This file contains C-entries for operators called directly
			by the interpreter using the Postscript execution model.

  Conventions: -The order in which functions appear is identical to the one
				in the Adobe Red Book (Chapter 6 Operator Summary).
		       -Variables called 'tos' and 'nos' point to the Top Of Stack
				and Next Of Stack elements resp. (on Operand stack).

*************************************************************************/

#include "errors.h"
#include "exec/types.h"
#include "pstypes.h"
#include "stream.h"
#include "memory.h"
#include "objects.h"

#include "misc.h"
#include "stack.h"
#include "gstate.h"
#include "context.h"

//--------------------------------------------------------------------
error ps_string		(DPSContext);
error ps_anchorsearch(DPSContext);
error ps_search		(DPSContext);
//--------------------------------------------------------------------

/************************************************************************/
/************************* String Operators *****************************/
/************************************************************************/
error ps_string(DPSContext ctxt) {	// size "string" | empty_string

	char *string=NULL;
	ps_obj *tos;
	int len;

	NEED_ARGS(1);

	tos = OPERSP(ctxt);

	if (OBJ_TYPE(tos) != TYPE_INT) return ERR_typecheck;
    len = tos->obj.intval;
    if (len > MAX_STRING || len <0)	return ERR_rangecheck;

    if (len) {
		string = AllocVM(VM, len);
		if (! string) return ERR_VMerror;
    }
						// Change INT TOS into STRING
    tos->type = TYPE_STRING;
    tos->len = len;
    tos->obj.stringval = string;	// maybe NULL !!
    tos->tag = 0;

	while(len--) *string++ = 0;		// wipe string clean

    return ERR_OK;
}
/************************************************************************/
error ps_anchorsearch(DPSContext ctxt) {// string seek "anchorsearch" post match TRUE
										// string seek "anchorsearch" string FALSE
	pso *tos,*nos;
	int i;
	char *src,*dst;

	NEED_ARGS(2);

	tos = OPERSP(ctxt);					// -> sub- string
	nos = tos +1;						// -> master string

	if (OBJ_TYPE(tos) != TYPE_STRING || OBJ_TYPE(nos) != TYPE_STRING)
		return ERR_typecheck;
	if (OBJ_ATTR(tos) > ACCESS_READ_ONLY) return ERR_invalidaccess;
	if (OBJ_ATTR(nos) > ACCESS_READ_ONLY) return ERR_invalidaccess;

	if (nos->len >= tos->len) {
		src = tos->obj.stringval;
		dst = nos->obj.stringval;

		for ( i=0; i< tos->len; i++) {
			if (*src++ != *dst++) goto nomatch;
		}

	ENOUGH_ROOM(1);						// need room for TRUE

	nos->obj.stringval = dst;			// change orig string to "post"
	nos->len -= tos->len;

	PUSHTRUE;

	return ERR_OK;
	}
//---------------------------------------------------------------
nomatch:
	*tos = f_obj;			// return FALSE

	return ERR_OK;
}
/************************************************************************/
error ps_search(DPSContext ctxt) { // string seek "search" | post match pre TRUE
								   // string seek "search" | string FALSE
	pso *tos,*nos;
	int i,n;
	char *src,*dst,*start;

	NEED_ARGS(2);

	tos = OPERSP(ctxt);					// -> sub- string
	nos = tos +1;						// -> master string

	if (OBJ_TYPE(tos) != TYPE_STRING || OBJ_TYPE(nos) != TYPE_STRING)
		return ERR_typecheck;
	if (OBJ_ATTR(tos) > ACCESS_READ_ONLY) return ERR_invalidaccess;
	if (OBJ_ATTR(nos) > ACCESS_READ_ONLY) return ERR_invalidaccess;

	if (nos->len >= tos->len) {			// sub-string has to fit

		start = nos->obj.stringval;		// start is advancing base ptr

		for (n=0; n< (nos->len - tos->len +1); n++) {

			src = start++;				// src is inner loop ptr
			dst = tos->obj.stringval;	// search string ptr (fixed)

			for ( i=0; i< tos->len; i++) {
				if (*src++ != *dst++) break;
			}

			if (i == tos->len) {		// complete match ?

				ENOUGH_ROOM(2);

				PUSHOPER(ctxt,nos);		// copy orig string as "pre"
				PUSHTRUE;				// push true

				nos->len -= ((tos--)->len+n); // change orig string to "post"
				nos->obj.stringval = nos->len ? src : NULL;
 
				if (!(tos->len = n))			// set length of "pre"
					tos->obj.stringval = NULL;	// and clear ptr if zero.
				
				return ERR_OK;
			}
		}		
	}

	*tos = f_obj;			// return FALSE

	return ERR_OK;
}
/************************************************************************/
