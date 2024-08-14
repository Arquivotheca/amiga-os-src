/************************************************************************

  Module :	Postscript "Dictionary Operators"	© Commodore-Amiga

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

#include "stdio.h"
#include "dict.h"

//--------------------------------------------------------------------
error ps_dict			(DPSContext);
error ps_maxlength		(DPSContext);
error ps_begin			(DPSContext);
error ps_end			(DPSContext);
error ps_def			(DPSContext);
error ps_load			(DPSContext);
error ps_store			(DPSContext);
error ps_known			(DPSContext);
error ps_where			(DPSContext);
error ps_currentdict	(DPSContext);
error ps_countdictstack	(DPSContext);
error ps_dictstack		(DPSContext);

error ps_cleardictstack(DPSContext);

error stuff_array(DPSContext,pso *src,int n);	// common sub-routine
//--------------------------------------------------------------------

IMPORT BOOL	 	Define		(DPSContext, ps_obj* dict, ps_obj *key, ps_obj *val);
IMPORT ps_obj *FindDictEntry(DPSContext, ps_obj* dict, ps_obj *);
IMPORT ps_obj * InitDict	(DPSContext, int entries);
IMPORT ps_obj * NameLookUp	(DPSContext, ps_obj *); //also sets 'last_dict' to last dict

/************************************************************************/
/*********************** Dictionary Operators ***************************/
/************************************************************************/

error ps_dict(DPSContext ctxt) {	// size "dict" |  dictobj
	ps_obj *tos;
	ps_obj *dict;

	NEED_ARGS(1);
	tos = OPERSP(ctxt);			// point to requested size

	MUST_BE_INT(tos);
    if (tos->obj.intval >65536 || tos->obj.intval <0) return ERR_rangecheck;

	dict = InitDict(ctxt, tos->obj.intval);	// create dict
	if (! dict) return ERR_VMerror;			// ERROR if no RAM left

	tos->type = TYPE_DICT;					//"in-stack" replace
	tos->obj.dictval = (g_obj*) dict;
	tos->len = tos->tag =0;					// both aren't used !
	return ERR_OK;
}
/************************************************************************/
error ps_maxlength(DPSContext ctxt) {	// dict "maxlength" | int
	ps_obj *tos;

	NEED_ARGS(1);

	tos = OPERSP(ctxt);
	if (OBJ_TYPE(tos) != TYPE_DICT) return ERR_typecheck;
    if (OBJ_AXES(tos->obj.dictval) > ACCESS_READ_ONLY) return ERR_invalidaccess;

    tos->type = TYPE_INT;
    tos->obj.intval = ((dict_entry*)tos->obj.dictval)->val.len;
    tos->len = tos->tag = 0;

    return ERR_OK;
}
/************************************************************************/
error ps_begin(DPSContext ctxt) {	// dictobj "begin" | ...
	ps_obj *tos;

	NEED_ARGS(1);

	tos = OPERSP(ctxt);					// point to TOS

	if (OBJ_TYPE(tos) != TYPE_DICT) return ERR_typecheck; // must be dict
    if (OBJ_AXES(tos->obj.dictval) > ACCESS_READ_ONLY) return ERR_invalidaccess;

// enough room in dict stack ?
    if (NUMDICT(ctxt) == D_SIZE) return ERR_dictstackoverflow;

	PUSHDICT(ctxt,tos);			// move to D stack
	POPOPER(ctxt);				// discard on O stack
	return ERR_OK;
}
/************************************************************************/
error ps_end(DPSContext ctxt) {		//  "end"  | ...

	if (NUMDICT(ctxt) > 3) {		// don't pop past user,shared & system dicts
	    POPDICT(ctxt);
	} else {
	    return ERR_dictstackunderflow;
	}
	return ERR_OK;
}
/************************************************************************/
error ps_def(DPSContext ctxt) {		// key value "def" |  ...
	ps_obj	*tos,*nos;
	ps_obj	*curr_dict;

	NEED_ARGS(2);
	tos = OPERSP(ctxt);			// tos-> VALUE
	nos = tos+1;				// nos-> KEY
	curr_dict = DICTSP(ctxt);	// "def" defines in current dict only..

	if (OBJ_AXES(curr_dict->obj.dictval)!=ACCESS_UNLIMITED)
		return ERR_invalidaccess;

	if ( Define(ctxt, curr_dict, nos, tos) ) {	// if OK,
	    POPOPER(ctxt);			// Discard used arguments
	    POPOPER(ctxt);
	    return ERR_OK;
	} else {
	  return ERR_dictfull;
	}
}
/************************************************************************/
error ps_load(DPSContext ctxt)	{	//		key "load" | value

	pso *key;

	NEED_ARGS(1);

	key = OPERSP(ctxt);

	if ( key = NameLookUp(ctxt, key) ) {	// try and find associated value

		if (OBJ_AXES(ctxt->space.last_dict->obj.dictval) > ACCESS_READ_ONLY)
			return ERR_invalidaccess;

		POPOPER(ctxt);
		PUSHOPER(ctxt,key);					// push value if match found
		return ERR_OK;
	}

	return ERR_undefined;					// else ...
}
/************************************************************************/
error ps_store(DPSContext ctxt)	{	//		key value "store" | ...

	pso *tos,*nos;
	pso *val;

	NEED_ARGS(2);

	tos = OPERSP(ctxt);
	nos = tos +1;

	if (val = NameLookUp(ctxt, nos) ) {	// does key already exist anywhere ?

		if (OBJ_ATTR(ctxt->space.last_dict->obj.dictval) != ACCESS_UNLIMITED)
			return ERR_invalidaccess;

		*val = *tos;					// modify old value

		POPNUMOPER(ctxt,2);				// discard parameters

		return ERR_OK;

	} else {
		return ps_def(ctxt);			// do "def" if key not found.
	}
}
/************************************************************************/
error ps_known(DPSContext ctxt) {	// dict key "known" |  boolean
	ps_obj *keyarg,*dictarg,*valptr;

	NEED_ARGS(2);

	keyarg = OPERSP(ctxt);		// tos-> Key
	dictarg = keyarg+1;			// nos-> Dict

	if ( OBJ_TYPE(dictarg) != TYPE_DICT ) return ERR_typecheck;

    if (OBJ_AXES(dictarg->obj.dictval) > ACCESS_READ_ONLY)
		return ERR_invalidaccess;

    valptr = FindDictEntry(ctxt, dictarg, keyarg);

    if (valptr)	*dictarg = t_obj;	// if found: return TRUE
    else		*dictarg = f_obj;	//	else FALSE

    POPOPER(ctxt);					// discard garbage (old key)
    return ERR_OK;
}
/************************************************************************/
// This routine uses a side-effect of NameLookUp()  **!!

error ps_where(DPSContext ctxt) {	// key "where" |  dict TRUE, FALSE
	ps_obj *tos;

	NEED_ARGS(1);
	tos = OPERSP(ctxt);					// tos-> Key

	if (NameLookUp(ctxt,tos) ) {		// if object exists in a dict..

		if (OBJ_AXES(ctxt->space.last_dict->obj.dictval) > ACCESS_READ_ONLY)
			return ERR_invalidaccess;

	    ENOUGH_ROOM(1);

	// create dict obj pointing to dict containing the name.

		*tos = *ctxt->space.last_dict;
	    PUSHTRUE;
	    return ERR_OK;

	} else {			// if name didn't exist : return FALSE
		POPOPER(ctxt);
		PUSHFALSE;
	    return ERR_OK;
	}
}
/************************************************************************/
error ps_currentdict(DPSContext ctxt) {	// "currentdict" | dictobj

	ENOUGH_ROOM(1);
	PUSHOPER(ctxt, DICTSP(ctxt) );
	return ERR_OK;
}
/************************************************************************/
error ps_countdictstack(DPSContext ctxt) {	// "countdictstack" | num

	ENOUGH_ROOM(1);
	PUSHINT( NUMDICT(ctxt) );
	return ERR_OK;
}
/************************************************************************/
// This routine is so similar to execstack that it's just a stub passing
// the variables to a common routine.

error ps_dictstack(DPSContext ctxt)	{	//	array "dictstack" | subarray

	int n;

	n = NUMDICT(ctxt);
	
	return stuff_array(ctxt,ctxt->space.stacks.d.sp +n-1,n);
}
/************************************************************************/
error stuff_array(DPSContext ctxt,pso *src,int n) {	// array "dictstack" | subarray
	
	pso *tos;

	NEED_ARGS(1);

	tos = OPERSP(ctxt);				// array that's gonna receive a STACK

	if (OBJ_TYPE(tos)!=TYPE_ARRAY) return ERR_typecheck;
	if (OBJ_ATTR(tos)!=ACCESS_UNLIMITED) return ERR_invalidaccess;
	if (tos->len < n ) return ERR_rangecheck;

	tos->len = n;			// set the size of the final sub-array in obj.

	tos = (pso*)tos->obj.arrayval;	// start of array

	while (n--) {
		*tos++ = *src--;			// arrays grow up, stacks grow down
	}
	return ERR_OK;
}
/************************************************************************/
/******** Display Postscript Dictionary Operators Extensions ************/
/************************************************************************/
error ps_cleardictstack(DPSContext ctxt) {	//	"cleardictstack" | ....

	while (NUMDICT(ctxt) > 3) {		// don't pop past user,shared & system dicts
	    POPDICT(ctxt);
	}
	return ERR_OK;
}
/************************************************************************/
