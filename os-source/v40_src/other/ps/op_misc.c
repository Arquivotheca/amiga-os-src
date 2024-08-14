/************************************************************************

  Module :	Postscript "Miscellaneous Operators"	© Commodore-Amiga

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

#include "intuition/intuition.h"

#include <proto/exec.h>
#include <proto/intuition.h>

//--------------------------------------------------------------------
error ps_bind					(DPSContext);
error ps_usertime				(DPSContext);

error ps_currentstrokeadjust	(DPSContext);
error ps_setstrokeadjust		(DPSContext);
//--------------------------------------------------------------------

IMPORT ps_obj	*NameLookUp	(DPSContext, ps_obj *);

/************************************************************************/
/********************* Miscellaneous Operators **************************/
/************************************************************************/
error ps_bind(DPSContext ctxt) {	// proc "bind" | proc

	ps_obj *tos,*proc,*val;
	error err;
	int i;

	NEED_ARGS(1);			// need a procedure body on stack
	
	tos = OPERSP(ctxt);		// point to procedure (array) object
	MUST_BE_PROC(tos);

    proc = (pso*) tos->obj.arrayval;		// 1st procedure "line"

    for (i=0; i<tos->len; i++) {	// forall procedure array elements do:

		switch (OBJ_TYPE(proc)) {
			//----------------------------------------------------------------
			case TYPE_NAME:
				if (OBJ_ATTR(proc)==ATTR_EXECUTE) {
				    if (val = NameLookUp(ctxt,proc)) {	
						// if name maps to an operator: replace by op itself
						if (OBJ_TYPE(val) == TYPE_OPERATOR)
						*proc = *val;
					}
				}
				break;
			//----------------------------------------------------------------
			case TYPE_ARRAY:
				if (OBJ_ATTR(proc)==ATTR_EXECUTE && OBJ_AXES(proc)==ACCESS_UNLIMITED) {
					
					ENOUGH_ROOM(1);
					PUSHOPER(ctxt,proc);
			// RECURSIVELY CALL BIND
					if (err = ps_bind(ctxt)) return err;
			// Change procedure to READ-ONLY access
					proc->type = TYPE_ARRAY | ATTR_EXECUTE | ACCESS_READ_ONLY;
					POPOPER(ctxt);
				}
		}
		proc++;		// goto next procedure "line" (next array element)
    }

	return ERR_OK;
}
/************************************************************************/
error ps_usertime(DPSContext ctxt) {	// "bind" | int
struct Library *IntuitionBase;
unsigned long secs,micros;

	ENOUGH_ROOM(1);
	if(IntuitionBase=OpenLibrary("intuition.library",0)) {
		CurrentTime(&secs,&micros);
		secs %= 4000000;						// make sure result < 42 days
		PUSHINT( secs*1000+micros/1000 );		// convert to milliseconds
		CloseLibrary(IntuitionBase);
		return ERR_OK;
	}

	PUSHOPER(ctxt,&int_obj);		// push zero if no Intuition (?!)
	return ERR_OK;
}
/************************************************************************/
error ps_currentstrokeadjust(DPSContext ctxt) { //
	return ERR_NOT_IMPLEMENTED;
}
/************************************************************************/
error ps_setstrokeadjust(DPSContext ctxt) { //
	return ERR_NOT_IMPLEMENTED;
}
/************************************************************************/
