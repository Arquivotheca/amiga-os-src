/************************************************************************

  Module :	Postscript "Math Operators"		© Commodore-Amiga 1991

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

#include "limits.h"
#include "m68881.h"
#include "math.h"

#define log10_e	0.4342944819032517611

//--------------------------------------------------------------------
IMPORT void clear_AEXC(void);		// start new floating point calc sequence
IMPORT BOOL check_INEX(void);		// check FPU overflow, underflow

IMPORT error ps_add	(DPSContext);	// Assembly language implementation
IMPORT error ps_mul	(DPSContext);	// Assembly language implementation
IMPORT error ps_sub	(DPSContext);	// Assembly language implementation

error ps_div		(DPSContext);
error ps_idiv		(DPSContext);
error ps_mod		(DPSContext);
error ps_abs		(DPSContext);
error ps_neg		(DPSContext);
error ps_ceiling	(DPSContext);
error ps_floor		(DPSContext);	// **!! WARNING (see code)
error ps_round		(DPSContext);	// **!! WARNING
error ps_truncate	(DPSContext);
error ps_sqrt		(DPSContext);
error ps_atan		(DPSContext);
error ps_cos		(DPSContext);
error ps_sin		(DPSContext);
error ps_exp		(DPSContext);
error ps_ln			(DPSContext);
error ps_log		(DPSContext);
error ps_rand		(DPSContext);
error ps_srand		(DPSContext);
error ps_rrand		(DPSContext);
//--------------------------------------------------------------------

/************************************************************************/
/************************ Arithmetic Operators **************************/
/************************************************************************/

/*	This 'C' version stays here in case M/C version isn't appropriate.

error ps_add(DPSContext ctxt) {		// num1 num2 "add"
	pso	*tos,*nos;
	uchar	tt,nt;			// TOS && NOS types
	double	a,b;			// accums in case any is REAL

	NEED_ARGS(2);

	tos = OPERSP(ctxt);	tt = OBJ_TYPE(tos);
	nos = tos+1;		nt = OBJ_TYPE(nos);	// cache OBJ_TYPES

	if ((tt != TYPE_INT && tt != TYPE_REAL)||
	    (nt != TYPE_INT && nt != TYPE_REAL))
		return ERR_typecheck;

	if (tt==TYPE_INT && nt==TYPE_INT) {
	    nos->obj.intval += tos->obj.intval;	// do ADD
	    POPOPER(ctxt);			// Pop garbage off
	    return ERR_OK;
	}

	if (tt==TYPE_REAL)	a = (double) tos->obj.realval;
	else			a = (double) tos->obj.intval;

	if (nt==TYPE_REAL)	b = (double) nos->obj.realval;
	else			b = (double) nos->obj.intval;

	nos->type = TYPE_REAL;		// int && real = real
	nos->obj.realval = a+b;
	POPOPER(ctxt);			// Pop garbage off
	return ERR_OK;
}

END OF COMMENTED-OUT BLOCK	*/

/************************************************************************/

/*	This 'C' version stays here in case M/C version isn't appropriate.

error ps_sub(DPSContext ctxt) {		// num1 num2 "sub"
	pso		*tos,*nos;
	uchar	tt,nt;			// TOS && NOS types
	double	a,b;			// accums in case any is REAL

	NEED_ARGS(2);			

	tos = OPERSP(ctxt);	tt = OBJ_TYPE(tos);
	nos = tos+1;		nt = OBJ_TYPE(nos);	// cache OBJ_TYPES

	if ((tt != TYPE_INT && tt != TYPE_REAL)||
	    (nt != TYPE_INT && nt != TYPE_REAL))
		return ERR_typecheck;

	if (tt==TYPE_INT && nt==TYPE_INT) {
	    nos->obj.intval -= tos->obj.intval;	// do all-integer SUB
	    POPOPER(ctxt);			// Pop garbage off
	    return ERR_OK;
	}

	if (tt==TYPE_REAL)	a = (double) tos->obj.realval;
	else			a = (double) tos->obj.intval;

	if (nt==TYPE_REAL)	b = (double) nos->obj.realval;
	else			b = (double) nos->obj.intval;

	nos->type = TYPE_REAL;		// int && real = real
	nos->obj.realval = a-b;
	POPOPER(ctxt);			// Pop garbage off
	return ERR_OK;
}

END OF COMMENTED-OUT BLOCK	*/

/************************************************************************/
error ps_mod(DPSContext ctxt) {		// int1 int2 "mod" | remainder

	pso	*tos,*nos;
	int result;

	NEED_ARGS(2);

	tos = OPERSP(ctxt); nos = tos +1;

	if (OBJ_TYPE(tos)!=TYPE_INT || OBJ_TYPE(nos)!=TYPE_INT) return ERR_typecheck;
	
	result = nos->obj.intval % tos->obj.intval;
	
	POPNUMOPER(ctxt,2);
	PUSHINT( result );

	return ERR_OK;
}
/************************************************************************/
/* This operator is implemented in assembler. NOTE: function commented-out

error ps_mul(DPSContext ctxt) {		// num1 num2 "mul"
	pso		*tos,*nos;
	uchar	tt,nt;			// TOS && NOS types
	double	a,b;			// accums in case any is REAL

	NEED_ARGS(2);			

	tos = OPERSP(ctxt);	tt = OBJ_TYPE(tos);
	nos = tos+1;		nt = OBJ_TYPE(nos);	// cache OBJ_TYPES

	if ((tt != TYPE_INT && tt != TYPE_REAL)||
	    (nt != TYPE_INT && nt != TYPE_REAL))
		return ERR_typecheck;

	if (tt==TYPE_INT && nt==TYPE_INT) {
	    nos->obj.intval *= tos->obj.intval;	// do all-integer MUL
	    POPOPER(ctxt);			// Pop garbage off
	    return ERR_OK;
	}

	if (tt==TYPE_REAL)	a = (double) tos->obj.realval;
	else			a = (double) tos->obj.intval;

	if (nt==TYPE_REAL)	b = (double) nos->obj.realval;
	else			b = (double) nos->obj.intval;

	nos->type = TYPE_REAL;		// int && real = real
	nos->obj.realval = a*b;
	POPOPER(ctxt);			// Pop garbage off
	return ERR_OK;
}

END OF COMMENTED-OUT BLOCK	*/

/************************************************************************/
error ps_div(DPSContext ctxt) {		// num1 num2 "div" | num1/num2
	pso		*tos,*nos;
	uchar	tt,nt;			// TOS && NOS types
	double	a,b;			// accums in case any is REAL

	NEED_ARGS(2);			

	tos = OPERSP(ctxt);	tt = OBJ_TYPE(tos);
	nos = tos+1;		nt = OBJ_TYPE(nos);	// cache OBJ_TYPES

	if ((tt != TYPE_INT && tt != TYPE_REAL)||
	    (nt != TYPE_INT && nt != TYPE_REAL))
		return ERR_typecheck;

	if (tt==TYPE_REAL)	a = (double) tos->obj.realval;
	else			a = (double) tos->obj.intval;

	if (nt==TYPE_REAL)	b = (double) nos->obj.realval;
	else			b = (double) nos->obj.intval;

	if (a == 0.0) return ERR_undefinedresult;

	nos->type = TYPE_REAL;		// int && real = real
	nos->obj.realval = b/a;
	POPOPER(ctxt);				// Pop garbage off
	return ERR_OK;
}
/************************************************************************/
error ps_idiv(DPSContext ctxt) {	// int1 int2 "idiv" | quotient

	pso *int1,*int2;
	int result;

	NEED_ARGS(2);
	
	int2 = OPERSP(ctxt); int1 = int2 + 1;

	if(OBJ_TYPE(int1)!=TYPE_INT||OBJ_TYPE(int2)!=TYPE_INT) return ERR_typecheck;

	if(int2->obj.intval == 0) return ERR_undefinedresult;

	result = int1->obj.intval / int2->obj.intval;

	POPNUMOPER(ctxt,2);
	PUSHINT( result );
	
	return ERR_OK;
}
/************************************************************************/
error ps_abs(DPSContext ctxt) {		// num "abs" | poznum

	pso *tos,result;

	NEED_ARGS(1);
	tos = OPERSP(ctxt);

	result.tag = result.len  =0;
		
	if(OBJ_TYPE(tos)==TYPE_INT) {
		if(tos->obj.intval != INT_MIN) {
			result.type = TYPE_INT|ATTR_LITERAL;
			result.obj.intval = abs(tos->obj.intval);
		} else {
			result.type = TYPE_REAL|ATTR_LITERAL;
			result.obj.realval = -(float)(INT_MIN);
		}
	} else if(OBJ_TYPE(tos)==TYPE_REAL) {
		result.type = TYPE_REAL|ATTR_LITERAL;
		result.obj.realval = abs(tos->obj.realval);
	} else return ERR_typecheck;
		
	POPOPER(ctxt);
	PUSHOPER(ctxt,&result);
	return ERR_OK;
}	
/************************************************************************/
error ps_neg(DPSContext ctxt) {		// num "neg" | -num
	pso	*tos;

	NEED_ARGS(1);

	tos = OPERSP(ctxt); MUST_BE_NUMERIC(tos);

	if (OBJ_TYPE(tos)==TYPE_REAL) {
	    tos->obj.realval = - tos->obj.realval;
	} else {
	    if (tos->obj.intval == INT_MIN) {
			tos->type = TYPE_REAL;		// negation promotes
			tos->obj.realval = - (double)tos->obj.intval;
	    } else {
			tos->obj.intval = - tos->obj.intval;
	    }
	}
	return ERR_OK;
}
/************************************************************************/
error ps_sqrt(DPSContext ctxt) {		//		num "sqrt" | real

	pso *tos;

	NEED_ARGS(1);

	tos = OPERSP(ctxt); MUST_BE_NUMERIC(tos);

	if (OBJ_TYPE(tos)==TYPE_REAL) {
		if (tos->obj.realval < 0) return ERR_rangecheck;

	    tos->obj.realval = sqrt(tos->obj.realval);
	} else {
	    if (tos->obj.intval < 0) return ERR_rangecheck;

		tos->type = TYPE_REAL;		// promote to REAL
		tos->obj.realval = sqrt( (float)tos->obj.intval);
	}
	return ERR_OK;
}
/************************************************************************/
error ps_cos(DPSContext ctxt) {		//		angle "cos" | real

	pso *tos;
	double angle;

	NEED_ARGS(1);

	tos = OPERSP(ctxt); MUST_BE_NUMERIC(tos);

	if (OBJ_TYPE(tos)==TYPE_REAL) {

		angle = tos->obj.realval;

	} else {
		tos->type = TYPE_REAL;		// promote to REAL

		angle = (double) tos->obj.intval;
	}

	angle = angle*PI/180;			// convert to radians

	tos->obj.realval = (float) cos(angle);

	return ERR_OK;
}
/************************************************************************/
error ps_sin(DPSContext ctxt) {		//		angle "sin" | real

	pso *tos;
	double angle;

	NEED_ARGS(1);

	tos = OPERSP(ctxt); MUST_BE_NUMERIC(tos);

	if (OBJ_TYPE(tos)==TYPE_REAL) {

		angle = tos->obj.realval;

	} else {
		tos->type = TYPE_REAL;		// promote to REAL

		angle = (double) tos->obj.intval;
	}

	angle = angle*PI/180;			// convert to radians

	tos->obj.realval = (float) sin(angle);

	return ERR_OK;
}
/************************************************************************/
error ps_ln(DPSContext ctxt) {		//		num "ln" | real

	pso *tos;

	NEED_ARGS(1);

	tos = OPERSP(ctxt); MUST_BE_NUMERIC(tos);

	if (OBJ_TYPE(tos)==TYPE_REAL) {

		tos->obj.realval = (float) log10( (double) tos->obj.realval)/log10_e;

	} else {
		tos->type = TYPE_REAL;		// promote to REAL
		tos->obj.realval = (float) log10( (double) tos->obj.intval)/log10_e;
	}
	return ERR_OK;
}
/************************************************************************/
error ps_log(DPSContext ctxt) {		//		num "log" | real

	pso *tos;

	NEED_ARGS(1);

	tos = OPERSP(ctxt); MUST_BE_NUMERIC(tos);

	if (OBJ_TYPE(tos)==TYPE_REAL) {

		tos->obj.realval = (float) log10( (double) tos->obj.realval);

	} else {
		tos->type = TYPE_REAL;		// promote to REAL
		tos->obj.realval = (float) log10( (double) tos->obj.intval);
	}
	return ERR_OK;
}
/************************************************************************/
error ps_ceiling(DPSContext ctxt)	{	//		num "ceiling" | num
	pso	*tos;

	NEED_ARGS(1);

	tos = OPERSP(ctxt); MUST_BE_NUMERIC(tos);

// Only if number is a real need we do a ceiling(); ints are always correct!
	if (OBJ_TYPE(tos)==TYPE_REAL) {
	    tos->obj.realval = ceil (tos->obj.realval);
	}

	return ERR_OK;
}

/** Floor ***************************************************************/
// This function and all others that use Lattice C their "floor(x)"
// implementation look strange because of a bug in the floor function.
// Unlike ceil(x), floor misbehaves for negative numbers (sh*t).
/************************************************************************/

error ps_floor(DPSContext ctxt)	{	//		num "floor" | num

	pso	*tos;

	NEED_ARGS(1);

	tos = OPERSP(ctxt); MUST_BE_NUMERIC(tos);

	if (OBJ_TYPE(tos)==TYPE_REAL) {
		if (tos->obj.realval >=0 )
		    tos->obj.realval = floor(tos->obj.realval);	// works OK when poz
		else
		    tos->obj.realval = -ceil(-tos->obj.realval); // the fix when neg
	}

	return ERR_OK;
}
/************************************************************************/
error ps_round(DPSContext ctxt)	{	//		num "round" | num

	pso	*tos;

	NEED_ARGS(1);

	tos = OPERSP(ctxt); MUST_BE_NUMERIC(tos);

	if (OBJ_TYPE(tos)==TYPE_REAL) {
		if (tos->obj.realval >=0 )
		    tos->obj.realval = floor(tos->obj.realval +0.5);
		else
		    tos->obj.realval = -ceil( -(tos->obj.realval + 0.5));
	}

	return ERR_OK;
}
/************************************************************************/
error ps_truncate(DPSContext ctxt)	{	// num.bla "truncate" | num.0

	pso	*tos;

	NEED_ARGS(1);

	tos = OPERSP(ctxt); MUST_BE_NUMERIC(tos);

// Only if number is a real need we do a floor(); ints are always correct!
	if (OBJ_TYPE(tos)==TYPE_REAL) {
	    tos->obj.realval = floor(tos->obj.realval);	// **!! Using bug
	}

	return ERR_OK;
}
/************************************************************************/
error ps_atan(DPSContext ctxt)	{	//	num den "atan" | angle

	pso *num,*den,result;
	double numerator,denominator;

	NEED_ARGS(2);

	den = OPERSP(ctxt); num = den + 1;

	if((OBJ_TYPE(den)==TYPE_REAL||OBJ_TYPE(den)==TYPE_INT) &&
	  ( OBJ_TYPE(num)==TYPE_REAL||OBJ_TYPE(num)==TYPE_INT)) {

		if(OBJ_TYPE(den)==TYPE_INT) denominator = (double)den->obj.intval;
		else 						denominator = (double)den->obj.realval;

		if(OBJ_TYPE(num)==TYPE_INT) numerator = (double)num->obj.intval;
		else					 	numerator = (double)num->obj.realval;
		
		
		if(numerator==0.0 && denominator==0.0) return ERR_undefinedresult;

		result.obj.realval = (180.0*atan2(numerator,denominator))/PI;

// Adjust value to correct quadrant
		if(result.obj.realval<0.0) {
			result.obj.realval += 360.0;
		}
		result.type = TYPE_REAL|ATTR_LITERAL;
		result.tag = 0;
		result.len = 0;

		POPNUMOPER(ctxt,2);
		PUSHOPER(ctxt,&result);

	} else return ERR_typecheck;

	return ERR_OK;
}
/************************************************************************/
error ps_exp(DPSContext ctxt)	{	//	base exponent "exp" | base^exp

	pso *tos,*nos;
	float a,b;

	NEED_ARGS(2);

	tos = OPERSP(ctxt);	MUST_BE_NUMERIC(tos);
	nos = tos +1; 		MUST_BE_NUMERIC(nos);

	if (OBJ_TYPE(tos) == TYPE_INT)	a = (float) tos->obj.intval;
	else							a = tos->obj.realval;
	if (OBJ_TYPE(nos) == TYPE_INT)	b = (float) nos->obj.intval;
	else							b = nos->obj.realval;

	clear_AEXC();			// wipe FPU exception bits
	a = pow (b, a);			// attempt calculation

	if (check_INEX()) return ERR_undefinedresult;

	nos->type = TYPE_REAL;	// promote previous arg regardless..
	nos->obj.realval = a;

	POPOPER(ctxt);			// discard old exponent, leave result on stack
	return ERR_OK;
}
/************************************************************************/
error ps_rand(DPSContext ctxt)	{	//		... "rand" | int

	pso * tos;

	ENOUGH_ROOM(1);
	srand(ctxt->space.seed);
	PUSHINT( rand() );

	tos = OPERSP(ctxt); ctxt->space.seed = tos->obj.intval;
	return ERR_OK;
}
/************************************************************************/
error ps_srand(DPSContext ctxt)	{	//
	pso *tos;

	NEED_ARGS(1);
	tos = OPERSP(ctxt);

	if(OBJ_TYPE(tos)!=TYPE_INT) return ERR_typecheck;

	ctxt->space.seed = tos->obj.intval;
	
	POPOPER(ctxt);
	return ERR_OK;
}
/************************************************************************/
error ps_rrand(DPSContext ctxt)	{	//

	ENOUGH_ROOM(1);
	PUSHINT( ctxt->space.seed );
	return ERR_OK;
}
/************************************************************************/
