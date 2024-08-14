/************************************************************************

  Module :	Postscript "Operand Stack Operators"	© Commodore-Amiga
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

#include "dict.h"

#include "math.h"
#include "m68881.h"

#define SIGN(x) ( ((x) > 0) - ((x) < 0) )

//--------------------------------------------------------------------
error ps_pop		(DPSContext);
error ps_exch		(DPSContext);
error ps_dup		(DPSContext);
error ps_copy		(DPSContext);
error ps_index		(DPSContext);
error ps_roll		(DPSContext);
error ps_clear		(DPSContext);
error ps_count		(DPSContext);
error ps_mark		(DPSContext);
error ps_cleartomark(DPSContext);
error ps_counttomark(DPSContext);

//--------------------------------------------------------------------
IMPORT BOOL		 Define		(DPSContext, pso* d, pso *k, pso *v);
IMPORT uchar * UnPackObject	(DPSContext, uchar *, pso *);

/************************************************************************/
/*************** Operand Stack Manipulation Operators *******************/
/************************************************************************/
error ps_pop(DPSContext ctxt) {		// anything "pop" |   ...

	NEED_ARGS(1);
	POPOPER(ctxt);
	return ERR_OK;
}
/************************************************************************/
error ps_exch(DPSContext ctxt) {	// any1 any2 "exch" | any2 any1

	pso *tos,*nos;
	pso temp;

	NEED_ARGS(2);
	tos = OPERSP(ctxt);
	nos = tos +1;

	temp = *nos; *nos = *tos; *tos = temp;	// swap items via temp object
	return ERR_OK;
}
/************************************************************************/
error ps_dup(DPSContext ctxt) {		//	any "dup" | any any

	pso *tos;

	NEED_ARGS(1);				// check for empty stacks !
	ENOUGH_ROOM(1);				// check for full stacks !
	tos = OPERSP(ctxt);
	PUSHOPER(ctxt,tos);			// duplicate TOS element
	return ERR_OK;
}	
/************************************************************************/
error ps_copy(DPSContext ctxt) {		//	a b c n "copy" | a b c a b c
						// ar.str.dct1  ar.str.dct2 "copy" | sub-ar.str.dct2

	pso		*dest,*source;			// dest is >= than source
	pso		*asrc,*adst;
	dict_entry *de_src,*de_dst;
	char	*src,*dst;
	uchar	*ptr;

	int n;

	NEED_ARGS(1);					// minimum !! ( "0 copy" )

	dest = OPERSP(ctxt);			// TOS
	source = dest +1;				// NOS


// just copy an N element stack frame to stack
	if (OBJ_TYPE(dest) == TYPE_INT) {

			n = dest->obj.intval;	// N elements to copy...
			if (! n) return ERR_OK;
			if (n < 0) return ERR_rangecheck;

			NEED_ARGS(n+1);			// stack has to contain requested items !!
			ENOUGH_ROOM(n-1);		// avoid stack overflow
			POPOPER(ctxt);			// pop count off
			dest += n;

			while (n--) {			// copy the stack frame
				PUSHOPER(ctxt,dest--);
			}
	} else

	switch ( OBJ_TYPE(source) ) {		// branch on source object !

		//------------------------------------------------------------
// In case of arrays and strings, copy contents of first into second

		case TYPE_ARRAY:
			if (OBJ_TYPE(dest) != TYPE_ARRAY)		return ERR_typecheck;
			if (source->len > dest->len)			return ERR_rangecheck;
			if (OBJ_AXES(dest)!=ACCESS_UNLIMITED)	return ERR_invalidaccess;
			if (OBJ_AXES(source)>= ACCESS_EXEC_ONLY)return ERR_invalidaccess;
			
			asrc = (pso*)source->obj.arrayval;	// point to src & dst arrays
			adst = (pso*)dest->obj.arrayval;
			dest->len = source->len;		// REM length of sub-array

			while (source->len--) {

				*adst++ = *asrc++;			// copy array elements
			}

			*source = *dest;	// put sub-array object in right place on O-stack
			POPOPER(ctxt);					// pop garbage off
			break;

		//------------------------------------------------------------
// A packed array can be copied into a normal array. This needs unpacking
		case TYPE_PACKED:
			if (OBJ_TYPE(dest) != TYPE_ARRAY)		return ERR_typecheck;
			if (source->len > dest->len) 			return ERR_rangecheck;
			if (OBJ_AXES(dest)!=ACCESS_UNLIMITED)	return ERR_invalidaccess;
			if (OBJ_AXES(source)>= ACCESS_EXEC_ONLY)return ERR_invalidaccess;
			
			ptr = source->obj.packval;		// point to src & dst arrays
			adst = (pso*)dest->obj.arrayval;
			dest->len = source->len;		// REM length of sub-array

			while (source->len--) {

				ptr = UnPackObject(ctxt, ptr, adst++);	// copy array elements
			}

			*source = *dest;	// put sub-array object in right place on O-stack
			POPOPER(ctxt);					// pop garbage off
			break;

		//------------------------------------------------------------
		case TYPE_STRING:
			if (OBJ_TYPE(dest) != TYPE_STRING) return ERR_typecheck;
			if (source->len > dest->len) 		return ERR_rangecheck;
			if (OBJ_AXES(dest)!=ACCESS_UNLIMITED)return ERR_invalidaccess;
			if (OBJ_AXES(source)>= ACCESS_EXEC_ONLY)return ERR_invalidaccess;

			src = source->obj.stringval;	// point to src & dst strings
			dst = dest->obj.stringval;
			dest->len = source->len;		// REM length of sub-strings

			while (source->len--) {

				*dst++ = *src++;			// copy string characters
			}

			*source = *dest;	// result has attributes of string2
			POPOPER(ctxt);
			break;

		//------------------------------------------------------------
		case TYPE_DICT:
			if (OBJ_TYPE(dest) != TYPE_DICT) return ERR_typecheck;

			if (OBJ_AXES(dest->obj.dictval)!=ACCESS_UNLIMITED)
				return ERR_invalidaccess;
			if (OBJ_AXES(source->obj.dictval) > ACCESS_READ_ONLY)
				return ERR_invalidaccess;

			de_dst = (dict_entry *)dest->obj.dictval;
			de_src = (dict_entry *)source->obj.dictval;

			if (de_dst->key.len) return ERR_rangecheck;	// length must be 0

			// and maxlength DST < length SRC
			if (de_dst->val.len < de_src->key.len) return ERR_rangecheck;
	
			n = de_src->val.len;		// MAXLENGTH of src dict

			de_dst++;					// move to first real dict pairs
			de_src++;


	// for all non-NULL keys in src dict, define same pair in dest dict.
			while (n--) {
				if (OBJ_TYPE(&de_src->key) != TYPE_NULL) {
					Define(ctxt,dest,&de_src->key,&de_src->val);
				}
				de_src++;
			}

			dest->type = source->type;	// result attributes come from dict1
			*source = *dest;
			POPOPER(ctxt);

			break;

		//------------------------------------------------------------
		default:	return ERR_typecheck;
	}

	return ERR_OK;
}
/************************************************************************/
error ps_index(DPSContext ctxt)	{	//	anyN...any0 N "index" | ..any0 anyN

	pso *tos;
	int index;

	NEED_ARGS(1);

	tos = OPERSP(ctxt);

	if(OBJ_TYPE(tos) != TYPE_INT) return ERR_typecheck;

	index = tos->obj.intval;			// get SF index (0..)
	if (index <0 ) return ERR_rangecheck;

	NEED_ARGS(index+1);					// check addressability of item

	ENOUGH_ROOM(1);
	POPOPER(ctxt);						// discard index
	PUSHOPER(ctxt, (tos+1+index));

	return ERR_OK;
}
/************************************************************************/
error ps_roll (DPSContext ctxt) {	// a b c N J "roll" | b a c

	pso *tos,*nos,*sfbase;
	int	n,num,jump;
	int i, dst_index;

	NEED_ARGS(2);

	tos = OPERSP(ctxt);				// roll direction and amount
	nos = tos +1;					// size of frame to roll

	if (OBJ_TYPE(tos) != TYPE_INT || OBJ_TYPE(nos) != TYPE_INT)
		return ERR_typecheck;

	if ((num = nos->obj.intval) < 0)	// get size of stack frame to roll
		return ERR_rangecheck;

	if (! num) return ERR_OK;			// zero stack frame ? : done.

	NEED_ARGS(num+2);					// SF really present ??

	jump = tos->obj.intval % num;

	n = abs(jump);						// number of times to do a SINGLE roll

	jump = -SIGN(jump);

	sfbase = nos +1;					// calc address of Stack Frame

	while( n-- ) {						// roll SF one item N times ...

		// Prime the loop

		dst_index = 0;
		*nos = *sfbase;					// copy 1st element into temp area

		for (i=0; i< num; i++) {

			dst_index = (dst_index+jump) % num;
				if (dst_index <0) dst_index += num;
	
//	printf ("Item %d -> %d\n", i, dst_index);

			*(sfbase) = *(sfbase+dst_index);
			*(sfbase+dst_index) = *nos;
			*nos = *(sfbase);
		}
	}

	POPNUMOPER(ctxt,2);					// discard roll parameters
	return ERR_OK;
}
/************************************************************************/
error ps_clear(DPSContext ctxt) {	//	"clear"	|  -empty-

	while (NUMOPER(ctxt)) {		// slow but compatible with future
	    POPOPER(ctxt);		// DPS VM garbage collection.
	}

	return ERR_OK;
}
/************************************************************************/
error ps_count(DPSContext ctxt) {	//	"count" |  int

	ENOUGH_ROOM(1);
	PUSHINT( NUMOPER(ctxt) );
	return ERR_OK;
}
/************************************************************************/
error ps_mark(DPSContext ctxt) {	//	"mark"	|   markobj

	ENOUGH_ROOM(1);
	PUSHMARK;
	return ERR_OK;
}
/************************************************************************/
error ps_counttomark(DPSContext ctxt) {	// [ obj...obj "counttomark" | int

	int cnt;
	pso *op_sp;

	ENOUGH_ROOM(1);			// to push possible count

	op_sp = OPERSP(ctxt);		// starting from TOS...
					// scan upwards until we hit '['
	for (cnt=0; cnt < NUMOPER(ctxt); cnt++) {

	    if (OBJ_TYPE(op_sp++) == TYPE_MARK) {
			PUSHINT( cnt );
			return ERR_OK;
	    }
	}

	return ERR_unmatchedmark;
}
/************************************************************************/
error ps_cleartomark(DPSContext ctxt)	{	// mark .. .. "cleartomark" | ...

	pso *tos,*tos2;
	int i;

	i = NUMOPER(ctxt);

	tos = tos2 = OPERSP(ctxt);

	while (i--) {			// without popping: find the required MARK first

		if (OBJ_TYPE(tos++) == TYPE_MARK) {

			while (OBJ_TYPE(tos2++) != TYPE_MARK) {

				POPOPER(ctxt);
			}

			POPOPER(ctxt);			// Pop delimiting mark off too.
			return ERR_OK;
		}
	}

	return ERR_unmatchedmark; // There ain't a friggin mark there in the first place !
}
/************************************************************************/
