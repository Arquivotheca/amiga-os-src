/************************************************************************

  Module :	Postscript "Graphic State Operators"		© Commodore-Amiga

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

#define GS	ctxt->space.GState

//--------------------------------------------------------------------
error ps_gsave				(DPSContext);
error ps_grestore			(DPSContext);
error ps_grestoreall		(DPSContext);
error ps_initgraphics		(DPSContext);
error ps_setlinewidth		(DPSContext);
error ps_setlinecap			(DPSContext);
error ps_setlinejoin		(DPSContext);
error ps_setmiterlimit		(DPSContext);
error ps_setdash			(DPSContext);
error ps_setflat			(DPSContext);
error ps_setgray			(DPSContext);
error ps_sethsbcolor		(DPSContext);
error ps_setrgbcolor		(DPSContext);
error ps_setscreen			(DPSContext);
error ps_settransfer		(DPSContext);

error ps_currentlinewidth	(DPSContext);
error ps_currentlinecap		(DPSContext);
error ps_currentlinejoin	(DPSContext);
error ps_currentmiterlimit	(DPSContext);
error ps_currentdash		(DPSContext);
error ps_currentflat		(DPSContext);
error ps_currentgray		(DPSContext);
error ps_currenthsbcolor	(DPSContext);
error ps_currentrgbcolor	(DPSContext);
error ps_currentscreen		(DPSContext);
error ps_currenttransfer	(DPSContext);

//--------------------------------------------------------------------
IMPORT error ps_initmatrix	(DPSContext);
IMPORT error ps_initclip	(DPSContext);
IMPORT error ps_newpath		(DPSContext);

IMPORT void fixer(float,float);
//--------------------------------------------------------------------
// Prototypes for other functions..

error common_set	(DPSContext ctxt, int type);
error common_get	(DPSContext ctxt, int type);

float percentage(float h);

// common set/current routine IDs

#define	LINEWIDTH	0
#define	LINECAP		1
#define	LINEJOIN	2
#define	MITERLIMIT	3
#define	FLATNESS	4
#define	GRAYNESS	5

/************************************************************************/
/********************* Graphic State Operators **************************/
/************************************************************************/

/************************************************************************/
error ps_gsave(DPSContext ctxt) {		//	- "gsave" | -

	gstate *gs;

	gs = AllocVM(VM,sizeof(gstate));	// allocate space for a new gstate
	if (!gs) return ERR_limitcheck;

	*gs = *(GS);		// copy entire current gstate to new one

		// The current path doesn't get copied on a gsave.
		// See DOCS for mechanism.

	gs->PathNew = gs->PathIndex;		// where newpath must reset to

	gs->next = GS;		// link old curr to new one
	GS = gs;			// new gstate becomes current

	return ERR_OK;
}
/************************************************************************/
error ps_grestore(DPSContext ctxt) {	//	- "grestore" | -

	gstate *gs;

	gs = GS;			// get address of current gstate
	if(gs->next) {
		GS = gs->next;	// unlink it (next one becomes curr)
		FreeVM(VM,gs,sizeof(gstate));	// free mem used by old gstate
	} else {
//		fprintf (OP,"No more gstates saved!\n");
	}

	return ERR_OK;
}
/************************************************************************/
error ps_grestoreall(DPSContext ctxt) {	//
	return ERR_NOT_IMPLEMENTED;
}
/************************************************************************/
error ps_initgraphics(DPSContext ctxt) {	//	- "initgraphics" | -

	pso arr = { TYPE_ARRAY,0,0,0};		// null array argument to setdash
	gstate * gs;

	gs = (gstate *) AllocVM(VM,sizeof(gstate)); if (!gs) return ERR_VMerror;

	// allocate memory for the path and an internal private path
	gs->PathBase    = AllocVM(VM,MAXPATH+MAXPATH*8);
	 if ( ! gs->PathBase) return ERR_VMerror;
	gs->PrivatePath = AllocVM(VM,MAXPATH+MAXPATH*8);
	 if ( ! gs->PrivatePath) return ERR_VMerror;

	gs->PathArgs = MAXPATH;			// maximum size of a path
	gs->PathStart =
		gs->PathNew =
		gs->PathIndex =
		gs->SubPath = 0;

	gs->PrivPathIndex = gs->PrivSubPath = 0;

	gs->CurrentFont = n_obj;		// initialize curfont to NULL obj !!
	gs->Flatness	= 1.0;			// default flatness

	gs->PathIsCurved	= FALSE;	// no curvetos in path
	gs->inBuildChar		= FALSE;
	gs->ICTMValid		= FALSE;
	gs->NoOutput		= FALSE;	// stroke, fill etc.. ENABLED
	gs->let_grow		= FALSE;	// show does gsave/grestores per char !
	gs->SCD				= FALSE;	// no SetCacheDevice called yet...

	gs->next = NULL;				// ensure no garbage link !!!
	GS = gs;	// link gstate to current context
	
// **!! WARNING: none of the following operators can gen an ERROR !
// **!! O-stack has to be clean (to avoid possible stackoverflows) !

	ps_initmatrix(ctxt);		// set CTM (in gstate) to default
	ps_newpath(ctxt);			// clear path indices, invalidate currentpoint
	ps_initclip(ctxt);			// set clipping path to device default

	PUSHINT( 1 ); ps_setlinewidth(ctxt);	// thin lines
	PUSHINT( 0 ); ps_setgray(ctxt);			// default color = BLACK
	PUSHINT( 0 ); ps_setlinejoin(ctxt);		// mitered joins (sharp angles)
	PUSHINT( 0 ); ps_setlinecap(ctxt);		// butt caps (no frills ends)
	PUSHINT( 10); ps_setmiterlimit(ctxt);	//
	PUSHOPER(ctxt,&arr); PUSHINT( 0 ); ps_setdash(ctxt); // [] 0 setdash

	return ERR_OK;
}
/************************************************************************/
error ps_setlinewidth(DPSContext ctxt) {return common_set(ctxt, LINEWIDTH);}
/************************************************************************/
error ps_setlinecap(DPSContext ctxt) {return common_set(ctxt, LINECAP);}
/************************************************************************/
error ps_setlinejoin(DPSContext ctxt) {return common_set(ctxt, LINEJOIN);}
/************************************************************************/
error ps_setmiterlimit(DPSContext ctxt) {return common_set(ctxt, MITERLIMIT);}
/************************************************************************/
error ps_setdash(DPSContext ctxt) {	//	array offset "setdash" | ...

	pso *offs,*array,*el;
	int i;
	BOOL all;						// flag to detect if any array el is nonzero

	NEED_ARGS(2);

	offs = OPERSP(ctxt);			// point to required array and offset
	array = offs + 1;				// and do some type checking

	MUST_BE_NUMERIC(offs);

	if (OBJ_TYPE(array) != TYPE_ARRAY) return ERR_typecheck;
	
	if (array->len > MAX_DASH) return ERR_limitcheck;	// array has to be small

	if (array->len) {	// if this is a non-empty array, all elements should

		el = (pso*) array->obj.arrayval;		// be positive and not all zero
		all = TRUE;

		for (i=0; i<array->len; i++) {
			MUST_BE_NUMERIC(el);

			if (OBJ_TYPE(el) == TYPE_INT) {		// got to have positive numbers
				if (el->obj.intval < 0) return ERR_typecheck;
				if (el->obj.intval) all=FALSE;	// if any el nonzero: remember
			} else {
				if (el->obj.realval < 0.0) return ERR_typecheck;
				if (el->obj.realval != 0.0) all=FALSE;
			}

			el++;
		}

		if (all) return ERR_typecheck;		// if flag still true: all-zero array
	}

	if (OBJ_TYPE(offs) == TYPE_INT) 
		GS->DashOffset = (float) offs->obj.intval;
	else
		GS->DashOffset = offs->obj.realval;

	GS->DashArray = *array;	// copy array object

	POPNUMOPER(ctxt,2);
	return ERR_OK;
}
/************************************************************************/
error ps_setflat(DPSContext ctxt) {return common_set(ctxt, FLATNESS);}
/************************************************************************/
error ps_setgray(DPSContext ctxt) {	//	num "setgray" | ...

	NEED_ARGS(1);
	if ( ! GS->SCD)
		return common_set(ctxt, GRAYNESS);
	else
		return ERR_undefined;
}
/************************************************************************/
error ps_sethsbcolor(DPSContext ctxt) {	// h s b "sethsbcolor" | ...

	pso	*hue,*sat,*bri;			// Hue, Saturation, Brightness
	float r,g,b;
	float h,s;

	NEED_ARGS(3);
	bri = OPERSP(ctxt);		// point to three color parameters (nums)
	sat = bri +1;
	hue = bri +2;

	if ( ! GS->SCD) {	// are we allowed to change HSB ?

		MUST_BE_NUMERIC(hue);
		MUST_BE_NUMERIC(sat);
		MUST_BE_NUMERIC(bri);

		FORCE_REAL(hue); FORCE_REAL(sat); FORCE_REAL(bri);

		h = hue->obj.realval * 6.0;			// map 0,0..1,0 to 0,0..6,0
		s = sat->obj.realval;

	fixer(h,h);
		r = percentage(h); h = h - 2.0;
	fixer(h,h);
		g = percentage(h); h = h - 2.0;
	fixer(h,h);
		b = percentage(h);

		r = (s * r + (1.0 - s)) * bri->obj.realval;
		g = (s * g + (1.0 - s)) * bri->obj.realval;
		b = (s * b + (1.0 - s)) * bri->obj.realval;

		GS->color[0] = r;
		GS->color[1] = g;
		GS->color[2] = b;

		GS->Greyness = b;
		
		POPNUMOPER(ctxt,3);
		return ERR_OK;
	}
	else
		return ERR_undefined;
}
/************************************************************************/
// Given a Hue (0..6) this routine calculates the percentage of RED in the
// color.
// For GREEN and BLUE color contents, just "shift" the original hue by 1/3,
// i.e 2.0 if the hue range is 0.0 ... 6.0

float percentage(float h) {

	if (h<0.0) h = -h;			// color function is symmetrical around 0 degrees
	if (h>3.0) h = 6.0 -h;		// and around 180 degrees too (3.0 in 6.0 circle)


// Now that we've normalized the input to a positive range, we can easily
// calc the RED contents.

	if (h>2.0) return 0.0;				// range 2.0 ... 
	if (h>1.0) return (float)(2.0-h);	// range 1.0 ... 2.0 decreases linearly

	return 1.0;							// range 0.0 ... 1.0 is pure RED
}
/************************************************************************/
error ps_setrgbcolor(DPSContext ctxt) {	// red green blue "setrgbcolor" | ...

	pso *red,*green,*blue;
	float r,g,b;

// illegal when called from within BuildChar after a setcachedevice
	if (GS->SCD) return ERR_undefined;

	NEED_ARGS(3);

	blue = OPERSP(ctxt);		// point to arguments
	green = blue +1;
	red = blue +2;				// and do some typechecking

	MUST_BE_NUMERIC(red);
	MUST_BE_NUMERIC(green);
	MUST_BE_NUMERIC(blue);

	if (OBJ_TYPE(red) == TYPE_INT) 	r = (float) red->obj.intval;
	else							r = red->obj.realval;

	if (OBJ_TYPE(green) == TYPE_INT)g = (float) green->obj.intval;
	else							g = green->obj.realval;

	if (OBJ_TYPE(blue) == TYPE_INT)	b = (float) blue->obj.intval;
	else							b = blue->obj.realval;

	if (r>=0.0 && r <=1.0 && g>=0.0 && g <=1.0 && b>=0.0 && b <=1.0) {
		GS->color[0]=r;
		GS->color[1]=g;
		GS->color[2]=b;
	} else
		return ERR_rangecheck;

	POPNUMOPER(ctxt,3);
	return ERR_OK;
}
/************************************************************************/
error ps_setscreen(DPSContext ctxt) {	// freq angle proc "setscreen" | ...

	pso *proc,*ang,*freq;

	NEED_ARGS(3);

	proc = OPERSP(ctxt);			// point to the three arguments
	ang = proc +1;
	freq = ang +1;

	MUST_BE_PROC(proc);

	if (OBJ_TYPE(ang)  != TYPE_REAL) return ERR_typecheck;
	if (OBJ_TYPE(freq) != TYPE_REAL) return ERR_typecheck;

	GS->SpotFunc	= *proc;	// set halftone scr spot function
	GS->ScreenAngle	= ang->obj.realval;
	GS->ScreenFreq	= freq->obj.realval;

	POPNUMOPER(ctxt,3);

	return ERR_OK;
}
/************************************************************************/
error ps_settransfer(DPSContext ctxt) {	//	proc "settransfer" | ...

	pso *proc;

	if (GS->SCD) return ERR_undefined;

	NEED_ARGS(1);

	proc = OPERSP(ctxt); MUST_BE_PROC(proc);

	GS->GrayTransfer = *proc; // set gray mapping function

	POPOPER(ctxt);
	return ERR_OK;
}
/************************************************************************/
// The following routine is common to all set.. operators that take ints
// or reals as args.

error common_set(DPSContext ctxt, int type) { //	num "set..." | ...

	pso *tos;

	NEED_ARGS(1);
	tos = OPERSP(ctxt);

	if (OBJ_TYPE(tos) == TYPE_INT) {	// cases for integer argument
		switch (type) {
			case LINEWIDTH:
				if (tos->obj.intval >= 0 )
					GS->LineWidth = (float) tos->obj.intval;
				else return ERR_rangecheck;
				break;
			case LINECAP:
				if (tos->obj.intval >=0 && tos->obj.intval <=2)
					GS->LineCap = tos->obj.intval;
				else return ERR_rangecheck;
				break;
			case LINEJOIN:
				if (tos->obj.intval >=0 && tos->obj.intval <=2)
					GS->LineJoin = tos->obj.intval;
				else return ERR_rangecheck;
				break;				
			case MITERLIMIT:
				if (tos->obj.intval < 1) return ERR_rangecheck;
				GS->MiterLimit = (float) tos->obj.intval;
				break;
			case FLATNESS:
				if (tos->obj.intval < 1) {
					GS->Flatness = 0.2;	// clip to minimum
					break;
				} else 
				if (tos->obj.intval >100 ) {
					GS->Flatness = 100.0; // clip to maximum
					break;
				} else
					GS->Flatness = (float) tos->obj.intval;
				break;
					
			case GRAYNESS:
				if (tos->obj.intval <0 || tos->obj.intval >1)
					return ERR_rangecheck;
				GS->color[0] = (float) tos->obj.intval;
				GS->color[1] = (float) tos->obj.intval;
				GS->color[2] = (float) tos->obj.intval;
				GS->Greyness = (float) tos->obj.intval;
				break;

			default: //printf ("Common_set called with illegal arg ! (LVA)\n");
				break;
		}
		POPOPER(ctxt);
		return ERR_OK;

	} else
	if (OBJ_TYPE(tos) == TYPE_REAL) {	// cases for real argument
		switch (type) {
			case LINEWIDTH:
				if (tos->obj.realval >= 0 )
					GS->LineWidth = tos->obj.realval;
				else return ERR_rangecheck;
				break;
			case LINECAP:
			case LINEJOIN: return ERR_typecheck;

			case MITERLIMIT:
				if (tos->obj.realval < 1.0) return ERR_rangecheck;
				GS->MiterLimit = tos->obj.realval;
				break;
			case FLATNESS:
				if (tos->obj.realval < 0.2) {
					GS->Flatness = 0.2;	// clip to minimum
					break;
				} else 
				if (tos->obj.realval >100.0 ) {
					GS->Flatness = 100.0; // clip to maximum
					break;
				} else
					GS->Flatness = tos->obj.realval;
				break;

			case GRAYNESS:
				if (tos->obj.realval <0.0 || tos->obj.realval >1.0)
					return ERR_rangecheck;
				GS->color[0] = tos->obj.realval;
				GS->color[1] = tos->obj.realval;
				GS->color[2] = tos->obj.realval;
				GS->Greyness = tos->obj.realval;
				break;

			default: //printf ("Common_set called with illegal arg ! (LVA)\n");
				break;
		}
		POPOPER(ctxt);
		return ERR_OK;

	} else return ERR_typecheck;
}
/************************************************************************/
// The following routine is common to all operators returning a single numeric
// return.

error common_get(DPSContext ctxt, int type) {

	pso num;

	ENOUGH_ROOM(1);

	num.len = num.tag = 0; num.type = TYPE_REAL;	// default result type
	switch (type) {
		case LINEWIDTH:
			num.obj.realval = GS->LineWidth;
			break;
		case LINECAP:
			num.type = TYPE_INT;
			num.obj.intval = GS->LineCap;
			break;
		case LINEJOIN:
			num.type = TYPE_INT;
			num.obj.intval = GS->LineJoin;
			break;
		case MITERLIMIT:
			num.obj.realval = GS->MiterLimit;
			break;
		case FLATNESS:
			num.obj.realval = GS->Flatness;
			break;
	}
	PUSHOPER(ctxt,&num);
	return ERR_OK;
}
/************************************************************************/
error ps_currentlinewidth(DPSContext ctxt) {return common_get(ctxt,LINEWIDTH);}
/************************************************************************/
error ps_currentlinecap(DPSContext ctxt) {return common_get(ctxt,LINECAP);}
/************************************************************************/
error ps_currentlinejoin(DPSContext ctxt) {return common_get(ctxt,LINEJOIN);}
/************************************************************************/
error ps_currentmiterlimit(DPSContext ctxt) {return common_get(ctxt,MITERLIMIT);}
/************************************************************************/
error ps_currentdash(DPSContext ctxt) {	// "currentdash" | array offset

	pso *ptr;

	ENOUGH_ROOM(2);

	PUSHOPER(ctxt,&real_obj);			// make room for array
	PUSHOPER(ctxt,&real_obj);			// and integer

	ptr = OPERSP(ctxt);					// now fill in slots

	(ptr++)->obj.realval = GS->DashOffset;
	*ptr = GS->DashArray;

	return ERR_OK;
}
/************************************************************************/
error ps_currentflat(DPSContext ctxt) {return common_get(ctxt,FLATNESS);}
/************************************************************************/
// The algorithm used here to convert from RGB to Adobe "HSB" (which is in
// reality an HSV model) is taken from Foley & Van Dam.

error ps_currenthsbcolor(DPSContext ctxt) {	//	"currenthsbcolor" | h s b

	float r,g,b;
	float rc,gc,bc;
	float h,s,v,max,min;

	ENOUGH_ROOM(3);						// can we push 3 ints ?

	r=GS->color[0];		// get current RGB from gstate
	g=GS->color[1];
	b=GS->color[2];

	max = max(r,max(g,b));
	min = min(r,min(g,b));

	v = max;
	if (max != 0.0) {				// avoid DIV BY ZERO
		s = (max-min)/max;			// saturation
	} else {
		s = 0.0;
	}

	if (s==0.0) {					// if saturation is zero, then we're
		h = 0.0;					// dealing with a pure GRAY : undefined hue
	} else {
		rc = (max-r)/(max-min);
		gc = (max-g)/(max-min);
		bc = (max-b)/(max-min);

		if 		(r==max) h = bc - gc;
		else if (g==max) h = 2.0 + rc - bc;
		else if (b==max) h = 4.0 + gc - rc;

		if (h < 0.0) h = h + 6.0;

		h = h / 6.0;				// return hue in [0..1] range
	}

	PUSHREAL(h);
	PUSHREAL(s);
	PUSHREAL(v);

	return ERR_OK;
}
/************************************************************************/
error ps_currentgray(DPSContext ctxt) {	//
	ENOUGH_ROOM(1);
	PUSHREAL(GS->Greyness);
	return ERR_OK;
}
/************************************************************************/
error ps_currentrgbcolor(DPSContext ctxt) {	// "currentrgbcolor" | r g b

	ENOUGH_ROOM(3);

	PUSHREAL( GS->color[0]);
	PUSHREAL( GS->color[1]);
	PUSHREAL( GS->color[2]);
	return ERR_OK;
}
/************************************************************************/
error ps_currentscreen(DPSContext ctxt) {	// "currentscreen" | fr an proc

	ENOUGH_ROOM(3);

	PUSHREAL(GS->ScreenFreq);
	PUSHREAL(GS->ScreenAngle);
	PUSHOPER(ctxt,(&GS->SpotFunc));
	return ERR_OK;
}
/************************************************************************/
error ps_currenttransfer(DPSContext ctxt) {	// "currenttransfer" | proc

	pso *tos,dummy;

	ENOUGH_ROOM(1);

	PUSHOPER(ctxt,&dummy);			// push a dummy obj to advance ptr

	tos = OPERSP(ctxt);				// get ptr to available slot

	*tos = GS->GrayTransfer;	// copy proc to Ostack
	return ERR_OK;
}
/************************************************************************/
