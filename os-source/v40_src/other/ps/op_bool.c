/************************************************************************

  Module :	Postscript "Boolean, Relational & Bitwise Operators" © Commodore-Amiga

  Purpose:	This file contains C-entries for operators called directly
			by the interpreter using the Postscript execution model.

  Conventions: -The order in which functions appear is identical to the one
				in the Adobe Red Book (Chapter 6 Operator Summary).
		       -Variables called 'tos' and 'nos' point to the Top Of Stack
				and Next Of Stack elements resp. (on Operand stack).

  NOTES : Many functions (ge,gt,le,lt etc...) share the same
          outer code. The only difference being the small comparison
          in the kernal of the routine, this will be changed using
          a common routine which calls a particular function at that
          point.

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

#include "string.h"

//--------------------------------------------------------------------
error ps_eq		(DPSContext);
error ps_ne		(DPSContext);
error ps_ge		(DPSContext);
error ps_gt		(DPSContext);
error ps_le		(DPSContext);
error ps_lt		(DPSContext);
error ps_and	(DPSContext);
error ps_not	(DPSContext);
error ps_or		(DPSContext);
error ps_xor	(DPSContext);
error ps_bitshift(DPSContext);

//--------------------------------------------------------------------
// Prototypes for other functions..

error geltlegt(DPSContext);

/************************************************************************/
/************ Relational, Boolean & Bitwise operators *******************/
/************************************************************************/
error ps_eq(DPSContext ctxt) {		// any1 any2 "eq" | TRUE,FALSE

	ps_obj	*tos,*nos;
	char	*s1,*s2;					// two string pointers
	uchar	t2;						// 2nd argument type
	int		i,j;
	BOOL	equal=FALSE;

	NEED_ARGS(2);

	tos = OPERSP(ctxt);
	nos = tos +1;

	t2 = OBJ_TYPE(nos);

	switch(OBJ_TYPE(tos)) {
		//----------------------------------------------------------------
	    case TYPE_INT:
			if(t2==TYPE_INT) {
				if (tos->obj.intval == nos->obj.intval) equal=TRUE;
			} else if (t2==TYPE_REAL) {
				if ( ((float)tos->obj.intval) == nos->obj.realval) equal=TRUE;
			}
			break;

		//----------------------------------------------------------------
	    case TYPE_REAL:
			if(t2==TYPE_REAL) {
				if (tos->obj.realval == nos->obj.realval) equal=TRUE;
			} else if (t2==TYPE_INT) {
				if (tos->obj.realval == ( (float)nos->obj.intval) ) equal=TRUE;
			}
			break;

		//----------------------------------------------------------------
		case TYPE_STRING:
			if (OBJ_AXES(tos) >= ACCESS_EXEC_ONLY)
				return ERR_invalidaccess;

	    case TYPE_NAME:
			if (t2==TYPE_NAME || t2==TYPE_STRING) {

				if (t2==TYPE_STRING && OBJ_AXES(nos)>=ACCESS_EXEC_ONLY)
					return ERR_invalidaccess;

				j = (OBJ_TYPE(tos)==TYPE_NAME) ? strlen(tos->obj.nameval) : tos->len;
				i = (t2==TYPE_NAME) ? strlen(nos->obj.nameval) : nos->len;

				if (i != j) break;

				s1 = tos->obj.nameval;
				s2 = nos->obj.nameval;

				while ( i--) {
					if (*s1++ != *s2++) break;
				}
				if (i == -1) equal = TRUE;
			}
			break;
		//----------------------------------------------------------------
	    default:
			if (OBJ_AXES(tos)!=ACCESS_UNLIMITED && OBJ_AXES(tos)!=ACCESS_READ_ONLY)
			    return ERR_invalidaccess;
			if (OBJ_AXES(nos)!=ACCESS_UNLIMITED && OBJ_AXES(nos)!=ACCESS_READ_ONLY)
			    return ERR_invalidaccess;

			if ((OBJ_TYPE(tos) == t2) && (tos->obj.intval == nos->obj.intval))
				equal=TRUE;
	}

	nos->type = TYPE_BOOL;
	nos->obj.boolval = equal;
	nos->len = nos->tag = 0;

	POPOPER(ctxt);			// Pop garbage off (old any2)

	return ERR_OK;
}


	
/************************************************************************/
error ps_ne(DPSContext ctxt) {		// any1 any2 "NE" | TRUE, FALSE

	ps_obj *tos;
	error err;

	if (! (err = ps_eq(ctxt) )) {	// call EQ to do the hard work
	    tos = OPERSP(ctxt);
	    tos->obj.boolval = (!tos->obj.boolval) & TRUE ;  // just NOT TOS
	    return ERR_OK;		// if EQ didn't return an ERROR !
	} else {
	    return err;
	}
}


/************************************************************************/
/************************************************************************
 Internal routine to get the relationship between two objects on the
 operand stack and return a special BOOL on the operand stack if there
 is no error.  The bool will be set to -1,0 or 1 depending on the
 relationship between the two objects. Example:-

	obj1 < obj2 - BOOL=-1
	obj1 = obj2 - BOOL= 0
	obj1 > obj2 - BOOL= 1

 The Postscript operators (ge,gt,le,lt) must modify this bool to return
 the correct type depending on thier function.  For example, le would
 modify -1 or 0 to be TRUE while a 1 would be modified to FALSE.

 This routine is required to deal with PostScript strings, reals and ints
 ************************************************************************/
error geltlegt(DPSContext ctxt) {
ps_obj *tos,*nos;
int result,i1,i2,ot;
float f1,f2;
unsigned char *s1,*s2;

	NEED_ARGS(2);
	tos=OPERSP(ctxt); nos=tos+1;	// point at the two objects
	ot=OBJ_TYPE(tos);				// cache second object type
	result = 0;						// assume equality to start
	switch(OBJ_TYPE(nos)) {			// index on type of first object

		case TYPE_STRING:
			if(ot!=TYPE_STRING) return ERR_typecheck;
			s1=nos->obj.stringval;
			s2=tos->obj.stringval;
			i1=nos->len;
			if(i1>tos->len) i1=tos->len;
			while(i1 && !result) {	// while still equal and not at end
				if(*s1 < *s2) result=-1;
				else if(*s1 > *s2) result=1;
				++s1; ++s2; --i1;
			}
			// handle the case of strings being a different length
			// but comparing equal for the length of the shorter string
			if((result==0) && (nos->len != tos->len)) {
				if(nos->len > tos->len) result=1;	// first is greater
				else result=-1;						// second is greater
			}
			break;

		case TYPE_INT:
			if(ot==TYPE_INT) {
				i2=tos->obj.intval;		// comparing ints
				i1=nos->obj.intval;
				if(i1<i2) result=-1;
				else if(i1>i2) result=1;
			}
			else if(ot==TYPE_REAL) {
				f2=tos->obj.realval;
				f1=(float)nos->obj.intval;	// always promote mixed to reals
				if(f1<f2) result=-1;
				else if(f1>f2) result=1;
			}
			else return ERR_typecheck;
			break;

		case TYPE_REAL:
			if(ot==TYPE_REAL) f2=tos->obj.realval;
			else if(ot==TYPE_INT) f2=(float)tos->obj.intval;
			else return ERR_typecheck;
			f1=nos->obj.realval;
			if(f1<f2) result=-1;
			else if(f1>f2) result=1;
			break;

		// wrong kinds of objects on the operand stack, return an error
		default:
			return ERR_typecheck;
	}

	// everything worked and 'result' holds the relationship
	POPOPER(ctxt);					// pop the top stack item
	nos->type=TYPE_BOOL|ATTR_LITERAL;
	nos->obj.intval = result;		// convert to internal bool
	nos->len = nos->tag = 0;
	return ERR_OK;
}

/************************************************************************/
error ps_ge(DPSContext ctxt)	{	//
error err;
ps_obj *tos;
	if((err=geltlegt(ctxt))!=ERR_OK) return err;	// get relationship
	tos=OPERSP(ctxt);
	if(tos->obj.intval == -1) tos->obj.intval=FALSE;	// only ge returns true
	else tos->obj.intval=TRUE;
	return err;
}

/************************************************************************/
error ps_gt(DPSContext ctxt)	{	//
error err;
ps_obj *tos;
	if((err=geltlegt(ctxt))!=ERR_OK) return err;	// get relationship
	tos=OPERSP(ctxt);
	if(tos->obj.intval == 1) tos->obj.intval=TRUE;	// only gt returns true
	else tos->obj.intval=FALSE;
	return err;
}

/************************************************************************/
error ps_le(DPSContext ctxt)	{	//
error err;
ps_obj *tos;
	if((err=geltlegt(ctxt))!=ERR_OK) return err;	// get relationship
	tos=OPERSP(ctxt);
	if(tos->obj.intval == 1) tos->obj.intval=FALSE;	// only le returns true
	else tos->obj.intval=TRUE;
	return err;
}

/************************************************************************/
error ps_lt(DPSContext ctxt)	{	//
error err;
ps_obj *tos;
	if((err=geltlegt(ctxt))!=ERR_OK) return err;	// get relationship
	tos=OPERSP(ctxt);
	if(tos->obj.intval == -1) tos->obj.intval=TRUE;	// only lt returns true
	else tos->obj.intval=FALSE;
	return err;
}

/************************************************************************/
error ps_and(DPSContext ctxt) {		// num1 num2 "and" | num

	ps_obj *tos,*nos;

	NEED_ARGS(2);

	tos = OPERSP(ctxt);
	nos = tos+1;

	switch (OBJ_TYPE(tos)) {
	    case TYPE_BOOL:
			if (OBJ_TYPE(nos) != TYPE_BOOL) return ERR_typecheck;
			break;
	    case TYPE_INT:
			if (OBJ_TYPE(nos) != TYPE_INT ) return ERR_typecheck;
			break;
	    default: return ERR_typecheck;
	}

	nos->obj.boolval &= tos->obj.boolval;

	POPOPER(ctxt);			//discard garbage
	return ERR_OK;
}
/************************************************************************/
error ps_or(DPSContext ctxt) {		//
	ps_obj *tos,*nos;

	NEED_ARGS(2);

	tos = OPERSP(ctxt);
	nos = tos+1;

	switch (OBJ_TYPE(tos)) {
	    case TYPE_BOOL:
			if (OBJ_TYPE(nos) != TYPE_BOOL) return ERR_typecheck;
			break;
	    case TYPE_INT:
			if (OBJ_TYPE(nos) != TYPE_INT ) return ERR_typecheck;
			break;
	    default: return ERR_typecheck;
	}

	nos->obj.boolval |= tos->obj.boolval;

	POPOPER(ctxt);			//discard garbage
	return ERR_OK;
}
/************************************************************************/
error ps_xor(DPSContext ctxt) {		//
	ps_obj *tos,*nos;

	NEED_ARGS(2);

	tos = OPERSP(ctxt);
	nos = tos+1;

	switch (OBJ_TYPE(tos)) {
	    case TYPE_BOOL:
			if (OBJ_TYPE(nos) != TYPE_BOOL) return ERR_typecheck;
			break;
	    case TYPE_INT:
			if (OBJ_TYPE(nos) != TYPE_INT ) return ERR_typecheck;
			break;
	    default: return ERR_typecheck;
	}

	nos->obj.boolval ^= tos->obj.boolval;

	POPOPER(ctxt);			//discard garbage
	return ERR_OK;
}
/************************************************************************/
error ps_not(DPSContext ctxt) {		// num "not" | num
	ps_obj *tos;

	NEED_ARGS(1);

	tos = OPERSP(ctxt);

	if (OBJ_TYPE(tos) == TYPE_INT) {
	    tos->obj.intval = ~ tos->obj.intval;			// all bits
	    return ERR_OK;
	}

	if (OBJ_TYPE(tos) == TYPE_BOOL) {
	    tos->obj.boolval = (!tos->obj.intval) & TRUE;	// LSb only
	    return ERR_OK;
	}

	return ERR_typecheck;
}
/************************************************************************/
error ps_bitshift(DPSContext ctxt)	{	//
	ps_obj *num,*shift,result;
	
	NEED_ARGS(2);
	shift = OPERSP(ctxt); num = shift + 1;
	if(OBJ_TYPE(num)!=TYPE_INT||OBJ_TYPE(shift)!=TYPE_INT) return ERR_typecheck;
	result.type = TYPE_INT|ATTR_LITERAL;
	result.len = 0;
	result.tag = 0;
	if(shift->obj.intval > 0) {
		result.obj.intval = num->obj.intval<<shift->obj.intval;
	} else {

// In convention with the manual, negative num's shifted right
// give incorrect results ! Hence the (ulong).

		result.obj.intval = (ulong)num->obj.intval>>(-shift->obj.intval);
	}
	POPNUMOPER(ctxt,2);
	PUSHOPER(ctxt,&result);
	return ERR_OK;
}
/************************************************************************/

