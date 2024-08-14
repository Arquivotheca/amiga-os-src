/************************************************************************

 Module :	Postscript "Program Control Operators"	© Commodore-Amiga

 Purpose:	This file contains C-entries for operators called directly
			by the interpreter using the Postscript execution model.

 Conventions: -The order in which functions appear is identical to the one
			   in the Adobe Red Book (Chapter 6 Operator Summary).
		      -Variables called 'tos' and 'nos' point to the Top Of Stack
			   and Next Of Stack elements resp. (on Operand stack).
			  -Whenever an operator is called, the function is guaranteed
			   to have at least one free E-stack slot (previously held by
			   itself).
			   Operators that only do one PUSHEXEC therefore do not do a
			   ENOUGH_EROOM(n).
 Internals:
	Looping contexts are implemented by having (basically) TWO procedures for
	every operator.
	One is the standard "ps_" entry point and is only executed once	per in-
	vocation (for example: ps_loop, ps_repeat, ps_forall, ...).
	The second entry point is an INTERNAL "operator" that does the actual
	looping. It's just a TYPE_OPERATOR as far as the interpreter is concerned,
	but isn't accessible by user Postscript programs.
	These operators use the pso.len field to tell "exit" howmany more
	E-stack objects to pop off on an exit (the same field also indicates
	exit whether an operator is INTERNAL or not: normal operators have a
	length field of zero (ALWAYS!!) ).
	This is because all the state information for the looping context is kept
	in 1 or more additional objects above the "looper" operator.
	A clean procedure object is always stored in this E-stack "frame" so that
	the "looper" can re-execute procedures by copying this object onto the
	E-stack.
***************************************************************************/

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
#include "stdio.h"

#define	STOP_MARKER		22220		// similar to RUN_MARKER (see misc.h)

//--------------------------------------------------------------------
error ps_exec			(DPSContext);
error ps_if				(DPSContext);
error ps_ifelse			(DPSContext);
error ps_for			(DPSContext);
error ps_repeat			(DPSContext);
error ps_loop			(DPSContext);
error ps_exit			(DPSContext);
error ps_stop			(DPSContext);
error ps_stopped		(DPSContext);
error ps_countexecstack	(DPSContext);
error ps_forall			(DPSContext);
error ps_execstack		(DPSContext);
error ps_quit			(DPSContext);
error ps_start			(DPSContext);

error intern_repeat		(DPSContext);
error intern_stopped	(DPSContext);
error intern_forall		(DPSContext);
error dict_forall		(DPSContext);
error intern_loop		(DPSContext);
error intern_intfor		(DPSContext);
error intern_floatfor	(DPSContext);
//--------------------------------------------------------------------

IMPORT error stuff_array	(DPSContext, pso *src, int n);
IMPORT error dummy			(DPSContext);		// Paul's "run" context marker
IMPORT error ps_itransform	(DPSContext);
IMPORT uchar *UnPackObject	(DPSContext, uchar *src, pso *destobj);

/************************************************************************/
/******************** Program Control Operators *************************/
/************************************************************************/
error ps_exec(DPSContext ctxt) {	// any "exec" | ...

	pso *tos;

	NEED_ARGS(1);

	tos = OPERSP(ctxt);		// point to any object on Op stack

	PUSHEXEC(ctxt,tos);		// Move it to the execution stack

	POPOPER(ctxt);			// Discard original

	return ERR_OK;
}
/************************************************************************/
error ps_if(DPSContext ctxt) {		// bool proc if | ...

	pso *true_proc,*cond;

	NEED_ARGS(2);

	true_proc = OPERSP(ctxt);		// point to procedure
	cond = true_proc+1;				// and boolean

	if (OBJ_TYPE(cond) != TYPE_BOOL) return ERR_typecheck;
	MUST_BE_PROC(true_proc);

	if (cond->obj.boolval) {		// only if TRUE, do "IF"
	    PUSHEXEC(ctxt,true_proc);	// transfer procedure to exec stack
	}
	POPNUMOPER(ctxt,2);				// discard boolean & copy of proc
	return ERR_OK;
}
/************************************************************************/
error ps_ifelse(DPSContext ctxt) {		// bool Tproc Fproc if | ...

	pso *true_proc,*false_proc,*cond;

	NEED_ARGS(3);

	false_proc = OPERSP(ctxt);			// point to procedures
	true_proc  = false_proc +1;
	cond       = false_proc +2;			// and condition (a boolean)

	if (OBJ_TYPE(cond) != TYPE_BOOL) return ERR_typecheck;
	MUST_BE_PROC(true_proc);
	MUST_BE_PROC(false_proc);

    if (cond->obj.boolval) {	  		// do TRUE proc if true
		PUSHEXEC(ctxt,true_proc); 		//
   	} else {					  		// else
		PUSHEXEC(ctxt,false_proc);		// do FALSE proc
    }

    POPNUMOPER(ctxt,3);			// discard condition & copy of procs
   	return ERR_OK;
}
/************************************************************************/
// "FOR" is an operator that has to repeat a given procedure.
// It does this by creating a stack frame on the execution stack that
// contains all the context information for the for plus a private operator
// that does all the hard work (intern_for).
//
// E-STACK FRAME STRUCTURE : 	PROC | lim/inc | counter |internal-OPERATOR
/************************************************************************/

//-----------------------------
struct for_cb {				// define "For" Control Block
	union {
		float real;			// This struct contains either two floats OR
		int	integer;		// two ints.
	} incr;					// First field is always the "for" increment
	union {					// Second field is always the "for" limit
		float real;			// The "for" count itself is stored in a proper
		int	integer;		// ps_obj below this FCB in the "for" stack frame.
	} limit;
};
//-----------------------------

error ps_for(DPSContext ctxt)	{	// 	start incr limit proc "for" | .....

	pso *proc,*lim,*inc,*start;

	struct for_cb fcb;		

	NEED_ARGS(4);

	proc = OPERSP(ctxt);		// get pointers to all 4 args
	lim  = proc +1;
	inc  = proc +2;
	start= proc +3;

// check if we get a real procedure from the user...
	MUST_BE_PROC(proc);

// now check if remaining args are all numeric (real or int)...
	MUST_BE_NUMERIC(start);
	MUST_BE_NUMERIC(inc);
	MUST_BE_NUMERIC(lim);

	ENOUGH_EROOM(4);			// need 4 free slots on E-stack !

	PUSHEXEC(ctxt,proc);		// copy procedure to exec stack (as part of SF)
	
// optimize "for"s that only use integers (do a separate for for reals/ints)
	if (OBJ_TYPE(start)==TYPE_REAL || OBJ_TYPE(inc)==TYPE_REAL || OBJ_TYPE(lim)==TYPE_REAL) {
	//--CREATE float FOR ------------------------------------------------

		FORCE_REAL(start); start->type = TYPE_REAL;
		FORCE_REAL(inc);
		FORCE_REAL(lim);

		fcb.incr.real = inc->obj.realval;
		fcb.limit.real = lim->obj.realval;

		proc->obj.operval = intern_floatfor;	// FLOAT "for" !

	//--CREATE int FOR---------------------------------------------------
	} else {
		fcb.incr.integer = inc->obj.intval;
		fcb.limit.integer = lim->obj.intval;

		proc->obj.operval = intern_intfor;	// INTEGER "for" !
	}

	PUSHEXEC(ctxt,((pso*)&fcb)); // push private "for" looping vars (real OR int)
	PUSHEXEC(ctxt,start);		// push initial (pre-decremented) counter value

	proc->type = TYPE_OPERATOR | ATTR_EXECUTE;	// finish off TYPE_OPERATOR obj
	proc->len = 3; proc->tag = 0;				// length of "stack frame" above us
	PUSHEXEC(ctxt,proc);

	POPNUMOPER(ctxt,4);			// discard original arguments

	return ERR_OK;
}
/************************************************************************/
// **!! INTERNAL ROUTINE used by "for" while actually repeating

error intern_intfor(DPSContext ctxt) {

	pso *e_tos;
	register int inc,val,lim;

	e_tos = EXECSP(ctxt);
	
	inc = ((struct for_cb*)(e_tos+1))->incr.integer;
	lim = ((struct for_cb*)(e_tos+1))->limit.integer;
	val = e_tos->obj.intval;

// Test loop termination codition FIRST

	if (inc >= 0) {
		if (val > lim ) {
			POPEXEC(ctxt);		// pop counter
			POPEXEC(ctxt);		// pop incr/limit control vars
			POPEXEC(ctxt);		// pop procedure
			return ERR_OK;
		}	
	} else {
		if (val < lim ) {
			POPEXEC(ctxt);		// pop counter
			POPEXEC(ctxt);		// pop incr/limit control vars
			POPEXEC(ctxt);		// pop procedure
			return ERR_OK;
		}
	}

// If limit not exceeded, push control variable on O-stack and increment it
// for next time...

	ENOUGH_ROOM(1);
	PUSHOPER(ctxt,e_tos);		// pass "for" control counter to procedure

	ENOUGH_EROOM(2);			// need room on E-stack
	PUSHEXEC(ctxt, (e_tos-1));	// stuff internal operator on stack again.
	PUSHEXEC(ctxt, (e_tos+2));	// copy "to be for-ed" proc down

// Increment loop control variable
	e_tos->obj.intval += inc;	// update counter
	return ERR_OK;
}
/************************************************************************/
// **!! INTERNAL ROUTINE used by "for" while actually repeating

error intern_floatfor(DPSContext ctxt) {

	pso *e_tos;
	float inc,val,lim;

	e_tos = EXECSP(ctxt);
	
	inc = ((struct for_cb*)(e_tos+1))->incr.real;
	lim = ((struct for_cb*)(e_tos+1))->limit.real;
	val = e_tos->obj.realval;

	if (inc >= 0) {
		if (val > lim ) {
			POPEXEC(ctxt);		// pop counter
			POPEXEC(ctxt);		// pop incr/limit control vars
			POPEXEC(ctxt);		// pop procedure
			return ERR_OK;
		}	
	} else {
		if (val < lim ) {
			POPEXEC(ctxt);		// pop counter
			POPEXEC(ctxt);		// pop incr/limit control vars
			POPEXEC(ctxt);		// pop procedure
			return ERR_OK;
		}
	}

	ENOUGH_ROOM(1);
	PUSHOPER(ctxt,e_tos);		// pass "for" control counter to procedure

	ENOUGH_EROOM(2);
	PUSHEXEC(ctxt, (e_tos-1));	// stuff internal operator on stack again.
	PUSHEXEC(ctxt, (e_tos+2));	// copy "to be for-ed" proc down

	e_tos->obj.realval += inc;	// update counter
	return ERR_OK;
}
/************************************************************************/
// E-STACK FRAME STRUCTURE : 		PROC | RCB | internal-OPERATOR

struct private {		// define "Repeat" Control Block
    int	counter;
    int	__pad;			// not used
};

error ps_repeat(DPSContext ctxt) {	// int proc "repeat" | ...

	pso *proc,*count;

	struct private rep_cb;		// room for an Repeat COntrol Block

	NEED_ARGS(2);

	proc = OPERSP(ctxt);
	count = proc +1;

	MUST_BE_PROC(proc);
    if (OBJ_TYPE(count) != TYPE_INT) return ERR_typecheck;

	ENOUGH_EROOM(3);			// don't blow e-stack !
	PUSHEXEC(ctxt, proc);		// copy proc to E-stack

	rep_cb.counter = count->obj.intval;
	PUSHEXEC(ctxt, ((pso*)(&rep_cb)) );

	proc->type = TYPE_OPERATOR | ATTR_EXECUTE;
	proc->obj.operval = intern_repeat;
	proc->tag = 0;
	proc->len = 2;				// length of "stack frame" above us
	PUSHEXEC(ctxt,proc);

	POPNUMOPER(ctxt,2);
	return ERR_OK;
}
/************************************************************************/
// **!! INTERNAL ROUTINE used by "repeat" while actually repeating

error intern_repeat(DPSContext ctxt) {

	struct private *e_tos;		// E-stack TOS is private control block

	e_tos = (struct private *) EXECSP(ctxt);

    if (e_tos->counter-- > 0) {	// if repeat cnt not exhausted..
		ENOUGH_EROOM(2);
		PUSHEXEC(ctxt, ((pso*)e_tos-1));
		PUSHEXEC(ctxt, ((pso*)e_tos+1));
		return ERR_OK;
    } else {					// if exhausted just discard repeat stuff
		POPEXEC(ctxt);			// pop RCB
		POPEXEC(ctxt);			// pop procedure
		return ERR_OK;
    }
}
/************************************************************************/
// E-STACK FRAME STRUCTURE : 		PROC | internal-OPERATOR

error ps_loop(DPSContext ctxt) {		// proc "loop" |  ...

	pso *proc;
	pso	priv_loop = {TYPE_OPERATOR|ATTR_EXECUTE, 0,1,intern_loop};

	NEED_ARGS(1);

	proc = OPERSP(ctxt); MUST_BE_PROC(proc);

	ENOUGH_EROOM(2);
	PUSHEXEC(ctxt,proc);		// copy procedure to E-stack
	PUSHEXEC(ctxt,&priv_loop);	// put private "looper" beneath it.

	POPOPER(ctxt);				// discard procedure on O-stack
	return ERR_OK;
}
/************************************************************************/
// **!! INTERNAL ROUTINE used by "loop" while actually looping

error intern_loop(DPSContext ctxt) {

	pso *e_tos;

	e_tos = EXECSP(ctxt);

	ENOUGH_EROOM(2);
	PUSHEXEC(ctxt, (e_tos-1));	// copy "looper" to E-TOS again.
	PUSHEXEC(ctxt, e_tos);		// and put looping procedure in front of it
	return ERR_OK;
}
/************************************************************************/
// Internals: this routine scans the E-stack to find any INTERNAL operator;
// it then uses the (normally unused) obj->len field of the object to determine
// how many more objects to pop off the E-stack, thus killing off the inner-
// most looping context.
/************************************************************************/

error ps_exit(DPSContext ctxt) {		// any { ... exit ...} | any

	pso *e_tos;
	int i;
	register short len;
	BOOL safe=FALSE;		// at least one context to exit from ?

//------------------------------------------------------------------------
// First, scan E-stack to see if first looping context isn't a run or stopped
// context. If so, return error and leave E-stack intact !!!!!

		e_tos = EXECSP(ctxt);
		i = NUMEXEC(ctxt);				// how many objects to scan (max)?

		while (i--) {
			if (OBJ_TYPE(e_tos) == TYPE_OPERATOR) {
				if (len = e_tos->len) {		// is this an INTERNAL "operator" ?

											// "run" or "stopped" ?
						if (len == RUN_MARKER || len == STOP_MARKER) {
								return ERR_invalidexit;
						} else {
							safe = TRUE;	// OK. At least one good context
							break;
						}	
				}
			}
			e_tos++;	// move up the E-stack looking for loop contexts
		}

		if (!safe) {
//			printf ("ps_exit ERROR: no context to exit from. (LVA)\n");
			return ERR_invalidexit;
		}
//------------------------------------------------------------------------
// First e-stack context isn't a run or stopped, so go ahead and pop stuff
// off until I hit the innermost context.

		e_tos = EXECSP(ctxt);
		i = NUMEXEC(ctxt);				// how many objects to scan ?

		while (i--) {
			if (OBJ_TYPE(e_tos) == TYPE_OPERATOR) {
				if (e_tos->len) {		// is this an INTERNAL "operator" ?

						i = e_tos->len+1;	// # of extra stack frame slots
						while (i--) {
							POPEXEC(ctxt);	// discard stack frame too.
						}
						return ERR_OK;
				}
			}
			e_tos++;
			POPEXEC(ctxt);	// move up the E-stack looking for loop contexts
		}
//	printf("Logic error in ps_exit !\n");
}
/************************************************************************/
error ps_stopped(DPSContext ctxt) {		// any "stopped" | bool

	pso *any;
	pso intern;

	NEED_ARGS(1);

	ENOUGH_EROOM(2);

	any = OPERSP(ctxt);

	intern.type = TYPE_OPERATOR | ATTR_EXECUTE; intern.tag = 0;
	intern.obj.operval = intern_stopped;	// point to internal routine
	intern.len = STOP_MARKER;	

	PUSHEXEC(ctxt,&intern);		// push routine that will put TRUE
					// on O-stack if any executes completely

	PUSHEXEC(ctxt, any);		// copy any to E-stack

	POPOPER(ctxt);			// discard any on O-stack
	return ERR_OK;
}
/************************************************************************/
// **!! INTERNAL ROUTINE used by "stopped" after complete execution of stopped
// routine. If routine did a "stop" to prematurely abort execution, then this
// will never get called !

error intern_stopped(DPSContext ctxt) {

	ENOUGH_ROOM(1);

	PUSHFALSE;				// unconditionally push a FALSE on O-stack

	return ERR_OK;
}
/************************************************************************/
error ps_stop(DPSContext ctxt) {	// ... "stop" | ....

	pso *e_sp;
	int i;

	ENOUGH_ROOM(1);

	e_sp = EXECSP(ctxt);
	i = NUMEXEC(ctxt);

	// First check to see if we're currently inside a stopped context !
	// If not, bomb out with a fatal error (should never occur in user progs).
	while (i--) {
		if ( (OBJ_TYPE(e_sp)==TYPE_OPERATOR) &&
			(e_sp->obj.operval == intern_stopped)) break;
		e_sp++;
	}

	if (i == -1) {
//		printf ("RED ALERT: stop fell off the end of the execution stack!\n");
		return ERR_unregistered;
	}

	e_sp = EXECSP(ctxt);		// reset stack pointer to ETOS
	while ( (OBJ_TYPE(e_sp)!=TYPE_OPERATOR) ||
			(e_sp->obj.operval != intern_stopped)) { 

		POPEXEC(ctxt);
		e_sp++;
	}

	POPEXEC(ctxt);				// discard stopped context.

	PUSHTRUE;					// push a TRUE on O-stack
	return ERR_OK;
}
/************************************************************************/
error ps_countexecstack(DPSContext ctxt) { // "countexecstack" | num

	ENOUGH_ROOM(1);
	PUSHINT( NUMEXEC(ctxt) );
	return ERR_OK;
}
/************************************************************************/
// E-STACK FRAME:		

error ps_forall(DPSContext ctxt) {	// array.dict.string proc "forall" | ...

	pso *proc,*nos;
	pso intern;
	int entries;

	NEED_ARGS(2);					// need a proc and another object
	proc = OPERSP(ctxt);
	nos = proc +1;

	MUST_BE_PROC(proc);

	switch (OBJ_TYPE(nos)) {
		//-----------------------------------------------------------------
		case TYPE_PACKED:
		case TYPE_ARRAY:
		case TYPE_STRING:
			if (OBJ_AXES(nos) >= ACCESS_EXEC_ONLY) return ERR_invalidaccess;

			if (! nos->len) {
				POPNUMOPER(ctxt,2);	// zero length array or string: NOP
				return ERR_OK;
			}

			break;		// OK, length non-zero: just do common action.

		//-----------------------------------------------------------------
		case TYPE_DICT:
			if (OBJ_AXES(nos->obj.dictval) >= ACCESS_EXEC_ONLY)
				return ERR_invalidaccess;

			entries = ((dict_entry*)nos->obj.dictval)->key.len; // 'length'

			if ( ! entries) {
				POPNUMOPER(ctxt,2);	// empty dictionary: forall does nothing
				return ERR_OK;
			}

			// DICT obj contains maximum iterations counter !!
			nos->len = ((dict_entry*)nos->obj.dictval)->val.len; // maxlength !
			nos->len++;		// compensate for "dowto 0" algorithm in internal.

			((dict_entry*)nos->obj.dictval)++;	//point to real 1st entry

			break;

		//-----------------------------------------------------------------
		default:		return ERR_typecheck;
	}

	ENOUGH_EROOM(3);

	PUSHEXEC(ctxt,proc);		// copy procedure to E-stack
	PUSHEXEC(ctxt,nos);			// copy object too (array,string OR dict)

	intern.type = TYPE_OPERATOR | ATTR_EXECUTE; intern.tag = 0;
	intern.obj.operval = intern_forall;	// point to internal routine
	intern.len = 2;				// size of stack frame above us
	PUSHEXEC(ctxt,&intern);		// push routine that will 'forall' repeatedly

	POPNUMOPER(ctxt,2);			// discard arguments
	return ERR_OK;
}
/************************************************************************/
// **!! INTERNAL ROUTINE used by "forall" while actually forall-ing

error intern_forall(DPSContext ctxt) {

	pso *e_tos;				// E-stack TOS is CB, NOS is proc
	pso	charcode,tempel;
	dict_entry *de;

	e_tos = EXECSP(ctxt);

    if (e_tos->len-- > 0) {			// if repeat cnt not exhausted..

		ENOUGH_EROOM(2);

		switch (OBJ_TYPE(e_tos)) {
			//------------------------------------------------------------
			case TYPE_PACKED:			// in the case of packed arrays, unpack

				ENOUGH_ROOM(1);
				e_tos->obj.packval = UnPackObject(ctxt, e_tos->obj.packval, &tempel);
				PUSHOPER(ctxt, &tempel);		// elements 1 at a time
			break;

			//------------------------------------------------------------
			case TYPE_ARRAY:			// in the case of arrays, just push

				ENOUGH_ROOM(1);
				PUSHOPER(ctxt, ((pso*)e_tos->obj.arrayval++));	// elements 1 at a time
			break;

			//------------------------------------------------------------
			case TYPE_STRING:			// in case of strings, push an INT

				ENOUGH_ROOM(1);
				charcode.type = TYPE_INT;	// representing current char in string
				charcode.obj.intval = *e_tos->obj.stringval++;
				charcode.tag = charcode.len = 0;
				PUSHOPER(ctxt,&charcode);
			break;

			//------------------------------------------------------------
			case TYPE_DICT:				// push a key-value pair
				ENOUGH_ROOM(2);
				de = (dict_entry*)e_tos->obj.dictval;

				while ( e_tos->len && OBJ_TYPE(&de->key)==TYPE_NULL) {
					de++;				// skip over empty entries
					e_tos->len--;		// decr original maxlength count
				}

				if (e_tos->len) {		// if maxlen count not exhausted..

					(dict_entry*)e_tos->obj.dictval = de +1; //advance for next time

					PUSHOPER(ctxt, &de->key);	// pass forall args to proc
					PUSHOPER(ctxt, &de->val);
					break;
				} else	{
					POPEXEC(ctxt);
					POPEXEC(ctxt);
					return ERR_OK;
				}
			//------------------------------------------------------------
		}

		PUSHEXEC(ctxt, ((pso*)e_tos-1));	// push another internal forall
		PUSHEXEC(ctxt, ((pso*)e_tos+1));	// push procedure to be run
		return ERR_OK;

    } else {			// if cnt exhausted just discard repeat stuff

		POPEXEC(ctxt);
		POPEXEC(ctxt);
		return ERR_OK;
    }
}
/************************************************************************/
error ps_execstack(DPSContext ctxt) {		// array "execstack" | subarray

	int n;

	n = NUMEXEC(ctxt);
	
	return stuff_array(ctxt,ctxt->space.stacks.e.sp +n-1,n);
}
/************************************************************************/
error ps_start(DPSContext ctxt) {		//
	return ERR_NOT_IMPLEMENTED;
}
/************************************************************************/
error ps_quit(DPSContext ctxt) {		//
	return ERR_QUIT_INTERPRETER;
}
/************************************************************************/
