/*************************************************************************

  Module :	Postscript "Array & Packed Array Operators"		© Commodore-Amiga

  Purpose:	This file contains C-entries for operators called directly
			by the interpreter using the Postscript execution model.

*************************************************************************/

#include "errors.h"
#include <exec/types.h>
#include <proto/exec.h>
#include "pstypes.h"
#include "stream.h"
#include "memory.h"
#include "objects.h"

#include "misc.h"
#include "stack.h"
#include "gstate.h"
#include "context.h"
#include "dict.h"

#include "stdio.h"

//--------------------------------------------------------------------
IMPORT pso *FindDictEntry(DPSContext, pso * dict, pso * key);
IMPORT ps_counttomark	(DPSContext);
IMPORT ps_exch			(DPSContext);
IMPORT ps_pop			(DPSContext);
IMPORT BOOL Define		(DPSContext, pso*, pso *key, pso *val);

error ps_array			(DPSContext);
error ps_closesquare	(DPSContext);	// ']' in other words !
error ps_length			(DPSContext);	// poly-morphic (arr,packed,str,dict)
error ps_getinterval	(DPSContext);
error ps_putinterval	(DPSContext);
error ps_aload			(DPSContext);
error ps_astore			(DPSContext);
error ps_get			(DPSContext);	// poly-morphic (arr,packed,str,dict)
error ps_put			(DPSContext);	// poly-morphic (arr,packed,str,dict)
error ps_currentpacking	(DPSContext);
error ps_setpacking		(DPSContext);
error ps_packedarray	(DPSContext);

uchar * PackObject		(DPSContext, pso* srcobj, uchar *dst);
uchar * UnPackObject	(DPSContext, uchar *src, pso *destobj);

/************************************************************************/
/************************** Array Operators *****************************/
/************************************************************************/

error ps_array(DPSContext ctxt) {	// size "array" | empty_array

	int i,len;						// to traverse array elements..
	ps_obj *array=0,*ar_el;
	ps_obj *tos;

	NEED_ARGS(1); tos = OPERSP(ctxt); MUST_BE_INT(tos);	// size must be an int

    len = tos->obj.intval;								// and be reasonable !
    if (len > MAX_ARRAY || len <0) return ERR_rangecheck;

    if (len) {
		array = AllocVM(VM, len * sizeof(ps_obj) );
		if (! array) return ERR_VMerror;

					// now init all array elements to NULL objects...
		    for (i=0,ar_el=array; i< len; i++) {
				*ar_el++ = n_obj;
	    	}
    }
			// Change INT TOS into ARRAY TOS

    tos->type = TYPE_ARRAY;
    tos->len = len;							// quickly before overwriting !
    tos->obj.arrayval = (g_obj*) array;		// can be NULL !
	SAVE_LEVEL(tos);
    return ERR_OK;
}		
/************************************************************************/
// Pack any N objects on O-stack into a single PACKED ARRAY

error ps_packedarray(DPSContext ctxt) {	// ob ob ob num "parray" | parray

	int i,len;
	uchar *base,*ptr,*array=0,*org=0;
	ps_obj *tos;

	NEED_ARGS(1);
	tos = OPERSP(ctxt);
	MUST_BE_INT(tos);				// size must be an int

    len = tos->obj.intval;			// and be reasonable !
    if (len > MAX_ARRAY || len <0) return ERR_rangecheck;

	tos += len;						// point to 1st object to be compressed

    if (len) {						// if 0-length array is requested,
		NEED_ARGS(len+1);

		base = ptr = AllocMem(len * sizeof(ps_obj), 0 );	// get scratch memory
		if (! base) return ERR_VMerror;

		i = len;
		while (i--) {
				ptr = PackObject(ctxt, tos, ptr);	// compress all objects into scratchmem
				tos--;
		}
	
		i = ptr - base;					// calc length of packed array
		org = array = AllocVM(VM, i);			// allocate real packed array space
		if (!org) {
			FreeMem(base, len*sizeof(pso));
			return ERR_VMerror;
		}

		ptr = base;
		while (i--) {
			*array++ = *ptr++;	// copy packed array to VM
//			fprintf(OP,"0x%x,", *(array-1));
		}
//		fprintf(OP,"\n");

		FreeMem(base, ptr-base);

		POPNUMOPER(ctxt,len);
	}

	tos = OPERSP(ctxt);

			// Change TOS into read-only ARRAY (guard against put)

    tos->type = TYPE_PACKED | ACCESS_READ_ONLY;
	tos->tag = 0;
    tos->len = len;
    tos->obj.arrayval = (g_obj*) org;		// can be NULL !

	SAVE_LEVEL(tos);
    return ERR_OK;
}
/************************************************************************/
// " ] " is the array constructor.
// Just do postscript sequence: counttomark array astore exch pop 

error ps_closesquare(DPSContext ctxt) {	// [ el el el ] | array

	error err;

	if (err = ps_counttomark(ctxt)) return err;
	if (err = ps_array(ctxt)) return err;
	if (err = ps_astore(ctxt)) return err;
	if (err = ps_exch(ctxt)) return err;
	if (err = ps_pop(ctxt)) return err;
	return ERR_OK;
}
/************************************************************************/
error ps_length(DPSContext ctxt) {	// arr.dict.string "length" | int

	ps_obj *tos;
	int length;

	NEED_ARGS(1);

	tos = OPERSP(ctxt);

	switch (OBJ_TYPE(tos)) {
	    case TYPE_ARRAY:		// only Composite objects have a length
		case TYPE_PACKED:
	    case TYPE_STRING:
			if (OBJ_AXES(tos)!=ACCESS_UNLIMITED && OBJ_AXES(tos)!=ACCESS_READ_ONLY)
			    return ERR_invalidaccess;
			length = tos->len;
			break;
		//---------------------------------------------------------------
	    case TYPE_DICT:
			if (OBJ_AXES(tos->obj.dictval)!=ACCESS_UNLIMITED &&
				OBJ_AXES(tos->obj.dictval)!=ACCESS_READ_ONLY)
		    	return ERR_invalidaccess;

			length = ((dict_entry*)tos->obj.dictval)->key.len;
			break;
		//---------------------------------------------------------------
	    default: return ERR_typecheck;
	}

	tos->type = TYPE_INT;
	tos->tag = tos->len = 0;
	tos->obj.intval = length;
	return ERR_OK;
}
/************************************************************************/
error ps_putinterval(DPSContext ctxt)	{	// dst index interv "putinterval" | ...

	pso *tos,*nos,*nnos;
	pso *src,*dst;
	char *s,*d;
	int i;

	NEED_ARGS(3);
	tos = OPERSP(ctxt);			// smaller string or array to put into bigger one
	nos = tos +1;				// index (0..length(src)-1 )
	nnos = nos +1;				// dst object

	MUST_BE_INT(nos);			// index has to be INT

	switch (OBJ_TYPE(nnos)) {
		//-----------------------------------------------------------------
		case TYPE_PACKED:
		case TYPE_ARRAY:
			if (OBJ_TYPE(tos) != TYPE_ARRAY) return ERR_typecheck;
				// only if count positive and le than src object....
			if (OBJ_AXES(tos)!=ACCESS_UNLIMITED)
			    return ERR_invalidaccess;
			if (nos->obj.intval < 0 || nos->obj.intval >= nnos->len)
				return ERR_rangecheck;

				// and index+count doesn't reach out of src object
			if ((nos->obj.intval + tos->len) >nnos->len)
				return ERR_rangecheck;

			i = tos->len;			// # of elements to copy
			dst = (pso*)nnos->obj.stringval +nos->obj.intval;	// d = orig + index
			src = (pso*)tos->obj.stringval;	// source is just start addr of interval

			while (i--) {
					*dst++ = *src++;
			}
			break;

		//-----------------------------------------------------------------
		case TYPE_STRING:		// src and dest needs to be of same type !
			if (OBJ_TYPE(tos) != TYPE_STRING) return ERR_typecheck;

			if (OBJ_AXES(tos)!=ACCESS_UNLIMITED)
			    return ERR_invalidaccess;
			if (nos->obj.intval < 0 || nos->obj.intval >= nnos->len)
				return ERR_rangecheck;
			if ((nos->obj.intval + tos->len) >nnos->len)
				return ERR_rangecheck;

			i = tos->len;			// # of elements to copy
			d = nnos->obj.stringval +nos->obj.intval;	// d = orig + index
			s = tos->obj.stringval;	// source is just start addr of interval

			while (i--) {
					*d++ = *s++;
			}
			break;
			
		//-----------------------------------------------------------------
		default : return ERR_typecheck;
	}

	POPNUMOPER(ctxt,3);
	return ERR_OK;
}
/************************************************************************/
error ps_getinterval(DPSContext ctxt)	{	// ar.str index cnt "getinterval" | sub-ar.str

	pso *tos,*nos,*nnos;
	pso dummy;
	int i;
	pso part;					// will hold subinterval array/string
	uchar * ptr;

	NEED_ARGS(3);
	tos = OPERSP(ctxt);			// cnt (size of interval)
	nos = tos +1;				// index (0..length(src)-1 )
	nnos = nos +1;				// src object

	MUST_BE_INT(tos);			// count and index have to be INT
	MUST_BE_INT(nos);

	switch (OBJ_TYPE(nnos)) {
		//-----------------------------------------------------------------
		case TYPE_ARRAY:
			if (OBJ_AXES(nnos) >= ACCESS_EXEC_ONLY ) return ERR_invalidaccess;

				// only if count positive and le than src object....
			if (nos->obj.intval < 0 || nos->obj.intval >= nnos->len)
				return ERR_rangecheck;

				// and index+count doesn't reach out of src object
			if ((nos->obj.intval + tos->obj.intval) >nnos->len)
				return ERR_rangecheck;

			part = *nnos;
			part.len = tos->obj.intval;
			part.obj.arrayval += nos->obj.intval;
			break;

		//-----------------------------------------------------------------
		// Find the spot where the subinterval starts and put that ptr in
		// the generated subinterval object.

		case TYPE_PACKED:
			if (OBJ_AXES(nnos) >= ACCESS_EXEC_ONLY ) return ERR_invalidaccess;

		// only if count positive and le than src object...
			i = nos->obj.intval;		// get index
			if (i < 0 || i >= nnos->len) return ERR_rangecheck;

		// and index+count doesn't reach out of src object
			if ( (tos->obj.intval + i) >nnos->len)
				return ERR_rangecheck;

			ptr = nnos->obj.packval;	// -> 1st packed arr "element"
		// skip forwards to index position
			do {
				ptr = UnPackObject(ctxt, ptr, &dummy);
			} while (i--);				// **!! DO-WHILE

			part = *nnos;
			part.len = tos->obj.intval;
			part.obj.packval = ptr;
			break;

		//-----------------------------------------------------------------
		case TYPE_STRING:
			if (OBJ_AXES(nnos) >= ACCESS_EXEC_ONLY ) return ERR_invalidaccess;

			if (nos->obj.intval < 0 || nos->obj.intval >= nnos->len)
				return ERR_rangecheck;
			if ((nos->obj.intval + tos->obj.intval) >nnos->len)
				return ERR_rangecheck;

			part = *nnos;
			part.len = tos->obj.intval;
			part.obj.stringval += nos->obj.intval;
			break;
			
		//-----------------------------------------------------------------
		default : return ERR_typecheck;
	}

	if ( ! part.len) part.obj.intval = NULL;	// zero pointer if zero length

	POPNUMOPER(ctxt,3);
	PUSHOPER(ctxt,&part);
	return ERR_OK;
}
/************************************************************************/
// Copy as many stack items as can fit into given array.

error ps_astore(DPSContext ctxt) {	// any any... array "astore" | array

	ps_obj *tos,*old_tos,*arr_el;
	int i,arr_size;

	NEED_ARGS(1);
	tos = OPERSP(ctxt);

	if (OBJ_TYPE(tos) != TYPE_ARRAY && OBJ_TYPE(tos) != TYPE_PACKED)
		return ERR_typecheck;
    if (OBJ_AXES(tos) != ACCESS_UNLIMITED) return ERR_invalidaccess;

// need enough stuff on stack to fill array
	if (NUMOPER(ctxt) < tos->len +1) return ERR_stackunderflow;

    old_tos = tos;					// REM addr to original array obj.

// point to last El. in array
    arr_el = (ps_obj*) tos->obj.arrayval + tos->len;
   	arr_size = (tos++)->len;

    for (i=0; i<arr_size; i++)
		*(--arr_el) = *tos++; 		// copy stacked items into array

    tos--;							// back-track to last arr el.
   	*tos = *old_tos;				//move original array up ...

// **!! WHAT DO I DO ?	SAVE_LEVEL(tos);	//stuff current save level in

	POPNUMOPER(ctxt,arr_size);
	return ERR_OK;
}
/************************************************************************/
error ps_aload(DPSContext ctxt) {	// array  "aload" | el el el.. array
									// packed "aload" | el el el.. packed
	ps_obj *tos, *arr_el;
	ps_obj tmp,tempel;
	uchar *packed;
	int i;							// array elements counter

	NEED_ARGS(1);
	tos = OPERSP(ctxt);

	if (OBJ_TYPE(tos) != TYPE_ARRAY && OBJ_TYPE(tos) != TYPE_PACKED)
		 return ERR_typecheck;
    if (OBJ_AXES(tos) != ACCESS_UNLIMITED && OBJ_AXES(tos) != ACCESS_READ_ONLY)
		return ERR_invalidaccess;

	ENOUGH_ROOM( tos->len );	// gonna create loads !

	tmp = *tos;					// save source array for end push

	POPOPER(ctxt);				// now discard TOS array.

	if (OBJ_TYPE(tos) == TYPE_ARRAY) {

		arr_el = (ps_obj*)tmp.obj.arrayval;	// point to first array element

		for (i=0; i<tmp.len; i++) {
	    	PUSHOPER(ctxt,arr_el++);
		}
	} else {		// OBJ_TYPE == PACKED

		packed = tmp.obj.packval;	// point to packed array
		tos = OPERSP(ctxt);

		for (i=0; i<tmp.len; i++) {
			packed = UnPackObject(ctxt, packed, &tempel);
			PUSHOPER(ctxt,&tempel);
		}
	}


	PUSHOPER(ctxt,&tmp);		// now push saved array back !
	return ERR_OK;
}
/************************************************************************/
error ps_put(DPSContext ctxt) {		// array  index any "put" |
									// string index int "put" | ...
									//   dict  key  any "put" |
	ps_obj *tos,*nos,*nnos;
	void *ptr;

	NEED_ARGS(3);					// THREE args needed.
	tos = OPERSP(ctxt);
	nos = tos +1;
	nnos = nos +1;

	switch ( OBJ_TYPE(nnos) ) {		// depending on "put" recipient..
		//-----------------------------------------------------------------
		case TYPE_PACKED:
	    case TYPE_ARRAY:
			MUST_BE_INT(nos);
			if (nos->obj.intval <0 || nos->obj.intval >= nnos->len)
			    return ERR_rangecheck;
			if (OBJ_AXES(nnos)!=ACCESS_UNLIMITED) return ERR_invalidaccess;
	
			ptr = nnos->obj.arrayval +nos->obj.intval;
			*((ps_obj*)ptr) = *tos;
	
  // THIS GENS A Lattice CXERR: *((ps_obj*) (nnos->obj.arrayval +nos->obj.intval)) = *tos;
			break;

		//-----------------------------------------------------------------
	    case TYPE_STRING:

			if (OBJ_AXES(nnos)!=ACCESS_UNLIMITED) return ERR_invalidaccess;

			if (OBJ_TYPE(nos) != TYPE_INT || OBJ_TYPE(tos) != TYPE_INT)
			    return ERR_typecheck;
			if (tos->obj.intval >255 || tos->obj.intval <0)
			    return ERR_rangecheck;
			if (nos->obj.intval <0 || nos->obj.intval >= nnos->len)
			    return ERR_rangecheck;
			ptr = nnos->obj.stringval + nos->obj.intval;
			*((char*)ptr) = tos->obj.intval;	// put ASCII char
			break;
		
		//-----------------------------------------------------------------
	    case TYPE_DICT:

			if (OBJ_AXES(nnos->obj.dictval)!= ACCESS_UNLIMITED)
				return ERR_invalidaccess;

			if (Define(ctxt,nnos,nos,tos)) {
				POPNUMOPER(ctxt,3);
				return ERR_OK;
			} else {
				return ERR_dictfull;
			}

		//-----------------------------------------------------------------
	    default: return ERR_typecheck;
	}

	POPNUMOPER(ctxt,3);		// "put" consumes all.
	return ERR_OK;
}
/************************************************************************/
error ps_get(DPSContext ctxt) {		// array  index "get" | any
									// packed index "get" | any
									// string index "get" | int
									// dict     key "get" | val
	ps_obj *tos,*nos,*val;
	pso dummy;						// for unpacking a packed array
	int i;
	void *ptr;

	NEED_ARGS(2);
	tos = OPERSP(ctxt);				// an integer index (or a dict key)
	nos = tos+1;					// a composite object

	switch (OBJ_TYPE(nos) ) {
		//-----------------------------------------------------------------
	    case TYPE_ARRAY:
			MUST_BE_INT(tos);
			if (tos->obj.intval <0 || tos->obj.intval >= nos->len) return ERR_rangecheck;
			if (OBJ_AXES(nos)!=ACCESS_UNLIMITED && OBJ_AXES(nos)!=ACCESS_READ_ONLY)
			    return ERR_invalidaccess;
		
			ptr = nos->obj.arrayval + tos->obj.intval;
			*nos = *((ps_obj*)ptr);	// copy array element to TOS
			break;
		//-----------------------------------------------------------------
		// For packed arrays we can't just zip to the required element, we've
		// got to unpack as we search for the element we need.

	    case TYPE_PACKED:
			MUST_BE_INT(tos);

			i = tos->obj.intval;		
			if (i <0 || i >= nos->len) return ERR_rangecheck;
			if (OBJ_AXES(nos)!=ACCESS_UNLIMITED && OBJ_AXES(nos)!=ACCESS_READ_ONLY)
			    return ERR_invalidaccess;

			ptr = nos->obj.packval;		// -> 1st "element" of packed array

			do {
				ptr = UnPackObject(ctxt, (uchar*) ptr, &dummy);
			} while (i--);				// **!! DO-WHILE

			*nos = dummy;				// copy unpacked array element to TOS
			break;

		//-----------------------------------------------------------------
	    case TYPE_STRING:
			MUST_BE_INT(tos);
			if (tos->obj.intval <0 || tos->obj.intval >= nos->len)
			    return ERR_rangecheck;
			if (OBJ_AXES(nos)!=ACCESS_UNLIMITED && OBJ_AXES(nos)!=ACCESS_READ_ONLY)
			    return ERR_invalidaccess;
			
			ptr = nos->obj.stringval + tos->obj.intval; //point to char
	
			nos->type = TYPE_INT;
			nos->obj.intval = *((char*)ptr) & 0x00FF;   //get char in TOS
			nos->tag = nos->len = 0;
			break;
			
		//-----------------------------------------------------------------
	    case TYPE_DICT:			// basically a NameLookUp in a specific dict

			if (OBJ_AXES(nos->obj.dictval)!=ACCESS_UNLIMITED &&
				OBJ_AXES(nos->obj.dictval)!=ACCESS_READ_ONLY)
			    return ERR_invalidaccess;	

			if (val = FindDictEntry(ctxt,nos,tos)) {	
				POPOPER(ctxt);		// discard inputs
				POPOPER(ctxt);
				PUSHOPER(ctxt,val);	// leave behind value from K-V pair
				return ERR_OK;
			} else {
				return ERR_undefined;
			}

		//-----------------------------------------------------------------
	    default: return ERR_typecheck;
	}

	POPOPER(ctxt);			// discard garbage
	return ERR_OK;
}
/************************************************************************/
error ps_setpacking(DPSContext ctxt) {			// bool "setpacking" | ...

	pso *tos;

	NEED_ARGS(1);

	tos = OPERSP(ctxt);

	if (OBJ_TYPE(tos) != TYPE_BOOL) return ERR_typecheck;

	ctxt->space.PackingOn = tos->obj.boolval;

	POPOPER(ctxt);
	return ERR_OK;
}
/************************************************************************/
error ps_currentpacking(DPSContext ctxt) {			// "currentpacking" | ...

	ENOUGH_ROOM(1);

	if (ctxt->space.PackingOn)		{
		PUSHTRUE;					}
	else							{
		PUSHFALSE;					}

	return ERR_OK;
}
/************************************************************************/
// Given any kind of Postscript 8-byte object, compress it in destination
// area and return updated destination pointer.

uchar * PackObject	(DPSContext ctxt, pso* srcobj, uchar *dst) {

	uchar type;
	BOOL addlen=FALSE;

	type = srcobj->type;

	switch (OBJ_TYPE(srcobj)) {
		case TYPE_NULL:
		case TYPE_MARK: *dst++ = type; return dst;

		case TYPE_BOOL: *dst = type;
						if (srcobj->obj.boolval)
							*dst++ |= 0x40;
						else
							*dst++ &= ~0x40;
						return dst;

		case TYPE_INT:	if ( srcobj->obj.intval == (srcobj->obj.intval & 1) ) {
							*dst = type;
							*dst &= ~0x20;	// clear 0/1 bit
							*dst |= (0x40 | (srcobj->obj.intval & 1)<<5 );
							return ++dst;
						}
						break;

		case TYPE_REAL: 
		case TYPE_FONTID:
		case TYPE_SAVE:
		case TYPE_DICT:
		case TYPE_FILE:
		case TYPE_OPERATOR:	break;

		case TYPE_ARRAY:
		case TYPE_PACKED:
		case TYPE_NAME:
		case TYPE_STRING:	addlen = TRUE; break;

		default: //fprintf(OP,"PackObject() unknown type !\n");
			break;
	}

	*dst++ = type;							// always copy type & attributes

	if (addlen) {
		*((uword*)dst) = srcobj->len;		// copy length into packed array
		dst +=2;
	}

	*((int*)dst) = srcobj->obj.intval;		// copy value into packed array

	dst +=4;
	return dst;								// return incremented pointer
}
/************************************************************************/
uchar * UnPackObject	(DPSContext ctxt, uchar *src, pso *destobj) {

	uchar type;

	type = *src;

//	fprintf(OP,"UNPACK $%x,$%x,$%x,$%x,$%x\n",
//				*src,*(src+1),*(src+2),*(src+3),*(src+4));

	switch ( OBJ_TYPE( (pso*) src) ) {
		case TYPE_NULL:
		case TYPE_MARK: *destobj = m_obj;
						destobj->type = *src++;	// this sets NULL or MARK
						return src;

		case TYPE_BOOL: if (*src & 0x40)
							*destobj = t_obj;
						else
							*destobj = f_obj;
						destobj->type = *src++;
						return src;
		//--------------------------------------------------
		case TYPE_INT:	*destobj = int_obj;
						if (*src & 0x40) {
							destobj->type = *src;
							destobj->obj.intval = (*src++ & 0x20)>>5;
							return src;
						}
						break;
		//--------------------------------------------------
		case TYPE_REAL:
		case TYPE_FONTID:	
		case TYPE_SAVE:
		case TYPE_DICT:
		case TYPE_FILE:
		case TYPE_OPERATOR:	break;
		//--------------------------------------------------
		case TYPE_ARRAY:	
		case TYPE_PACKED:
		case TYPE_NAME:
		case TYPE_STRING:
						destobj->len = *((uword*) (src+1));
						src += 2;		// skip len
						break;
		//--------------------------------------------------
		default: //fprintf (OP,"UnPackObject(): Unknown type!\n");
			break;
	}

	destobj->type = type;
	destobj->tag = 0;
	destobj->obj.intval = *((int*)(src+1));

	return src+5;
}
/************************************************************************/
