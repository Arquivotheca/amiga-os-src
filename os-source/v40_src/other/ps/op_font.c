/************************************************************************

  Module :	Postscript "Character & Font Operators"		© Commodore-Amiga

  Purpose:	This file contains the all-important "show", "kshow", "definefont"
			etc...
			Since the font cache mechanism is so closely related to font
			operators, cache routines are also in this file.

			Note that "findfont" is not implemented in 'C' but in Postscript
			itself.

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
#include "fonts.h"

#define	TEST_AXS	0x01	// Validate checks ON/OFF
#define GS			ctxt->space.GState

#define DEB	fprintf
//--------------------------------------------------------------------
error ps_definefont		(DPSContext);
error ps_scalefont		(DPSContext);
error ps_makefont		(DPSContext);
error ps_setfont		(DPSContext);
error ps_currentfont	(DPSContext);
error ps_show			(DPSContext);
error ps_ashow			(DPSContext);
error ps_widthshow		(DPSContext);
error ps_awidthshow		(DPSContext);
error ps_kshow			(DPSContext);
error ps_stringwidth	(DPSContext);
error ps_charpath		(DPSContext);

// Font cache operators
error ps_cachestatus	(DPSContext);
error ps_setcachedevice	(DPSContext);
error ps_setcharwidth	(DPSContext);
error ps_setcachelimit	(DPSContext);
error ps_setcacheparams (DPSContext);
error ps_currentcacheparams (DPSContext);

error GetLeftSideBearing(DPSContext, uchar ch, float *lx, float *ly);
error GetCharWidth		(DPSContext, uchar ch, float *wx, float *wy);
error GetCharProc		(DPSContext, uchar ch);
error ShowLetter		(DPSContext, uchar ch);
error AWKShowLetter		(DPSContext, uchar ch);
error ValidateFont 		(DPSContext, pso * font, int checks_mask);
error awshow			(DPSContext, char mode, pso* cx, pso* cy,
									pso* charint,pso* ax, pso* ay);
error RenderCharPath	(DPSContext);
void  fixer				(float a, float b);
//--------------------------------------------------------------------

IMPORT pso  * FindDictEntry	(DPSContext, pso *dict, pso *any);
IMPORT error	  ExecPSobj (DPSContext, pso *ob);
IMPORT pso	*	   InitDict	(DPSContext, int entries);
IMPORT char *		AddName	(DPSContext, char *);
IMPORT BOOL			 Define	(DPSContext, pso*, pso *key, pso *val);
IMPORT error	 push2reals (DPSContext, float, float);
IMPORT error	   IsMatrix	(pso*);				// returns 0 (FALSE) if OK!!

IMPORT error ps_currentmatrix	(DPSContext);
IMPORT error ps_invertmatrix	(DPSContext);
IMPORT error ps_currentpoint	(DPSContext);
IMPORT error ps_concatmatrix	(DPSContext);
IMPORT error ps_setlinewidth	(DPSContext);
IMPORT error ps_strokepath		(DPSContext);
IMPORT error ps_dtransform		(DPSContext);
IMPORT error ps_itransform		(DPSContext);
IMPORT error ps_translate		(DPSContext);
IMPORT error ps_grestore		(DPSContext);
IMPORT error ps_readonly		(DPSContext);
IMPORT error ps_setdash			(DPSContext);
IMPORT error ps_rmoveto			(DPSContext);
IMPORT error ps_newpath			(DPSContext);
IMPORT error ps_matrix			(DPSContext);
IMPORT error ps_moveto			(DPSContext);
IMPORT error ps_concat			(DPSContext);
IMPORT error ps_stroke			(DPSContext);
IMPORT error ps_gsave			(DPSContext);
IMPORT error ps_array			(DPSContext);
IMPORT error ps_fill			(DPSContext);
IMPORT error ps_copy    		(DPSContext);
IMPORT error ps_exec    		(DPSContext);
IMPORT error ps_exch    		(DPSContext);
IMPORT error ps_get     		(DPSContext);

#define	A_SHOW	0x01		/* bit values ! (can be ORed) */
#define	W_SHOW	0x02		/* awidthshow uses mode combination A_SHOW+W_SHOW */
#define DUMMY	&n_obj		/* unused args to awshow() point to dummy obj */

pso arr = {TYPE_ARRAY,0,0,0};		// a global READ-ONLY array skeleton

/************************************************************************/
/******************* Character and Font Operators ***********************/
/************************************************************************/

/** Definefont **********************************************************/
// Check font dictionary argument for correct internal Font structure.
// Then add FID entry to font and add font to FontDirectory dictionary.
/************************************************************************/

error ps_definefont	(DPSContext ctxt) { //	key font "definefont" | ...

	pso *fkey,*font;
	pso *obj;
	pso name={TYPE_NAME,0,0,0};
	pso fid ={TYPE_FONTID,0,0,0};
	error err;

	NEED_ARGS(2);						// font "key" and font itself
	font = OPERSP(ctxt);
	fkey = font +1;

// Check wether font dictionary contains all the required, initialized entries.

	if (err = ValidateFont(ctxt,font,TEST_AXS) ) return err;

// Now add the FID font entry. Check whether dict has room first..
// if length == maxlength then dict is full !

	if (font->obj.dictval->len == (font->obj.dictval+1)->len) {
//		fprintf(OP,"Can't add FID entry, font dict full! [%d entries]\n",
//			font->obj.dictval->len);
		return ERR_dictfull;		// font dict can't hold extra FID entry
	}

// dictionary has room for at least one additional entry, add FID entry.

	name.obj.nameval = UNIQUE_STRPTR("FID");
	Define(ctxt,font,&name,&fid);

// Font is ready to be added to FontDirectory in systemdict, check whether
// FDir exists and if it can accept extra font.

	name.obj.nameval = UNIQUE_STRPTR("FontDirectory");
	if (! (obj = FindDictEntry(ctxt,&ctxt->space.sysdict,&name)) )
		return ERR_invalidfont;		// FontDirectory doesn't exist !!!

	if (obj->obj.dictval->len == (obj->obj.dictval+1)->len) {
//		fprintf(OP,"Can't add font, FontDirectory full!\n");
		return ERR_dictfull;		// FontDirectory is FULL !!!!
	}

// Write protect font before submitting to FontDir

	ps_readonly(ctxt);

// Add validated and protected font to FontDirectory dictionary.

	if (! Define(ctxt,obj,fkey,font)) {
//		DEB(OP,"Could not add font to FontDir\n");
	}

	ps_exch(ctxt);

	POPOPER(ctxt);					// return font dict
	return ERR_OK;
}
/** ValidateFont ****** (INTERNAL) **************************************/
// Check font dictionary argument for correct internal Font structure.
// The writeable dictionary needs:
// - FontType	a small int (if 3 then a proc BuildChar needs to be there too)
// - FontMatrix an array of the form [ num num  num num  num num]
// - FontBBox	an array of the form [ num num  num num ]
// - Encoding	an array of length 256 full of names.
/************************************************************************/

error ValidateFont (DPSContext ctxt, pso * font, int checks_mask) {

	pso *el,*obj;
	pso name={TYPE_NAME,0,0,0};
	int i;

// "Font" has to be a dictionary (and has to be writable initialy)

	if (OBJ_TYPE(font) != TYPE_DICT) return ERR_typecheck;

	if (checks_mask & TEST_AXS) {
		if (OBJ_AXES(font->obj.dictval) != ACCESS_UNLIMITED)
			return ERR_invalidfont;
//		DEB (OP,"Font is writable\n");
	}
//	DEB (OP,"Font is a dict\n");

// Font needs an INTEGER "FontType" entry

	name.obj.nameval = UNIQUE_STRPTR("FontType");
	if (obj=FindDictEntry(ctxt,font,&name)) {
		if (OBJ_TYPE(obj)!=TYPE_INT) return ERR_invalidfont;

		// If FontType == 3 then font also needs a "BuildChar" procedure

		if (obj->obj.intval == USER_FONT) {
			name.obj.nameval = UNIQUE_STRPTR("BuildChar");
			if (obj=FindDictEntry(ctxt,font,&name)) {
				if (OBJ_TYPE(obj)!=TYPE_ARRAY && OBJ_TYPE(obj)!=TYPE_PACKED)
					return ERR_invalidfont;
				if (OBJ_ATTR(obj)!=ATTR_EXECUTE) return ERR_invalidfont;
			} else return ERR_invalidfont;
//			DEB (OP,"Font is user font and contains BuildChar\n");

		} else
			if (obj->obj.intval > 5) {
//				DEB (OP,"FontType huge ! [=%d]\n",obj->obj.intval);
				return ERR_invalidfont;
			}

	} else return ERR_invalidfont;
//	DEB (OP,"Font has required FontType entry\n");

// Font needs a 6-element array "FontMatrix" entry filled with numbers

	name.obj.nameval = UNIQUE_STRPTR("FontMatrix");
	if (obj=FindDictEntry(ctxt,font,&name)) {
		if ( IsMatrix(obj) ) return ERR_invalidfont;

	} else return ERR_invalidfont;
//	DEB (OP,"Font has FontMatrix\n");

// Font needs a 4-element array "FontBBox" entry filled with numbers

	name.obj.nameval = UNIQUE_STRPTR("FontBBox");
	if (obj=FindDictEntry(ctxt,font,&name)) {
		if (OBJ_TYPE(obj)!= TYPE_ARRAY) return ERR_invalidfont;
		if (obj->len != 4) return ERR_invalidfont;

		el = (pso*) obj->obj.arrayval;
		for(i=0;i<4;i++) {
			if (OBJ_TYPE(el) != TYPE_REAL && OBJ_TYPE(el) != TYPE_INT)
				return ERR_invalidfont;
			el++;
		}
	} else return ERR_invalidfont;
//	DEB (OP,"Font has FontBBox\n");

// Font needs a 256-element array "Encoding" entry filled with names.

	name.obj.nameval = UNIQUE_STRPTR("Encoding");
	if (obj=FindDictEntry(ctxt,font,&name)) {
		if (OBJ_TYPE(obj)!= TYPE_ARRAY) return ERR_invalidfont;
		if (obj->len != 256) return ERR_invalidfont;
//		DEB(OP,"Font has 256 element array (gonna check elements..)\n");
		el = (pso*) obj->obj.arrayval;
		for (i=0; i<256; i++) {
			if (OBJ_TYPE(el++) != TYPE_NAME) return ERR_invalidfont;
		}
	} else return ERR_invalidfont;
//	DEB (OP,"Font has Encoding vector\n");

// This dictionary contains all required entries to be called a "font".

	return ERR_OK;		// give blessings...
}
/** ps_makefont *********************************************************/
// Given an existing font, make a copy of it but change the copy's FontMatrix
// to be the concatenation of old and new FontMatrices.
/************************************************************************/

error ps_makefont	(DPSContext ctxt) { //	font matrix "makefont" | font'

	pso *mat,*font,*newfont,*oldmat,*newmat;
	pso *memory;
	pso name={TYPE_NAME,0,0,0};
	int srcsize;
	error err;

	NEED_ARGS(2);					// need a matrix and a font as args..

	mat = OPERSP(ctxt);
	font = mat +1;

	if (err = IsMatrix(mat) ) return err;			// need a clean matrix
	if (err = ValidateFont(ctxt,font,0) ) return err;	// need a proper font dict

	ENOUGH_ROOM(4);		// gonna push some extra stuff...

	srcsize = font->obj.dictval->len;				// get length of orig font

// Allocate a new dictionary to hold the copy of the argument font
	memory = InitDict(ctxt, srcsize + 2); if (!memory) return ERR_VMerror;
	PUSHOPER(ctxt,font);		// get font on TOS
	PUSHOPER(ctxt,&dict_obj);	// push empty recipient dictionary
	ctxt->space.stacks.o.sp->obj.dictval = (g_obj*) memory; // fill in TOS
	ps_copy(ctxt);			// copy original font entries into new empty one.

	newfont = OPERSP(ctxt);

// To new font, add entries ScaleMatrix and OrigFont
	name.obj.nameval = UNIQUE_STRPTR("ScaleMatrix");
	Define(ctxt,newfont,&name,mat);			// Add ScaleMatrix entry
	name.obj.nameval = UNIQUE_STRPTR("OrigFont");
	Define(ctxt,newfont,&name,font);		// Add OrigFont entry

// Now change this new font's FontMatrix to be the concatenation of the old
// one and the new one.

	name.obj.nameval = UNIQUE_STRPTR("FontMatrix");
	oldmat = FindDictEntry(ctxt,font,&name);	// get font's original matrix
	PUSHOPER(ctxt,oldmat);				// push it (matrix1)
	PUSHOPER(ctxt,mat);					// push copy of new one (matrix2)
	PUSHINT( 6 ); err = ps_array(ctxt);	// "6 array" ==> dest matrix (matrix3)
		if (err) return err;

	err = ps_concatmatrix(ctxt);		// use "concatmatrix" to do the mul
		if (err) { //DEB(OP,"Makefont BUG\n"); 
					return err;}

	newmat = OPERSP(ctxt);				// get pointer to new matrix
	Define(ctxt,newfont,&name,newmat);	// replace old one by new one in dict
		
	POPNUMOPER(ctxt,4);					// discard args and copy result

	PUSHOPER(ctxt,&dict_obj);			// push resultant new font
	ctxt->space.stacks.o.sp->obj.dictval = (g_obj*) memory; // fill in TOS
	
	return ERR_OK;
}
/** ps_scalefont ********************************************************/
// This routine is basically like makefont but with a uniform scaling, so
// make a matrix from the given scale and call makefont to do all the dirty
// work..
/************************************************************************/

error ps_scalefont	(DPSContext ctxt) { //	font scale "makefont" | font'

	pso *scale,*font,*el;
	pso	scaleobj;				// holds a copy of entire scale argument
	error err;
	int i;

	NEED_ARGS(2);

	scale = OPERSP(ctxt);
	font = scale +1;

	MUST_BE_NUMERIC(scale);

	if (err = ValidateFont(ctxt,font,0) ) return err; // need a proper font dict

	scaleobj = *scale;			// copy scale argument

	scale->type = TYPE_INT;		// use scale arg to make an INT "6"
	scale->obj.intval = 6;
	ps_array(ctxt);				// replace scale by a 6-element array

	el = (pso*) scale->obj.arrayval;	// get base of empty array
	i = 6;
	while (i--)
		*el++ = int_obj;		// fill array with integer zeros (int_obj = 0)

	el = (pso*) scale->obj.arrayval;	// get base of array again
	*el = scaleobj;				// put scale factor on head diagonal
	*(el+3) = scaleobj;			// mat[0,0] = scalex, mat[1,1] = scaley

	ps_makefont(ctxt);			// hard work is common to makefont...

	return ERR_OK;
}

/** ps_setfont **********************************************************/
// Make argument font dictionary the current font for show and copy some
// frequently used font dictionary entries into gstate to avoid future
// dictionary look-ups.
// Since SETFONT can NOT return an invalidfont error, if we detect that
// one of the obligatory fields is not present, we set a flag for show
// so that it doesn't crash !
// >>>> "10 dict setfont" is LEGAL Postscript !!
/************************************************************************/

error ps_setfont	(DPSContext ctxt) { //	font "setfont" | ...

	pso *tos,*obj;
	pso name={TYPE_NAME,0,0,0};

	NEED_ARGS(1);			// need one postscript dictionary on O-stack
	tos = OPERSP(ctxt);
	if (OBJ_TYPE(tos) != TYPE_DICT) return ERR_typecheck;

	GS->goodfont = TRUE;		// assume font argument OK.

// Before setting GS->CurrentFont to argument, check and cache required entries

// Copy FontType
	name.obj.nameval = UNIQUE_STRPTR("FontType");
	if (obj = FindDictEntry(ctxt,tos,&name)) {
		GS->FontType = obj->obj.intval;					// USER / BUILT_IN
	} else
		GS->goodfont = FALSE;

// Copy FontMatrix
	name.obj.nameval = UNIQUE_STRPTR("FontMatrix");
	if (obj = FindDictEntry(ctxt,tos,&name)) {
		GS->FontMatrix = *obj;
	} else
		GS->goodfont = FALSE;
		
// Copy Encoding
	name.obj.nameval = UNIQUE_STRPTR("Encoding");
	if (obj = FindDictEntry(ctxt,tos,&name)) {
		GS->Encoding = *obj;
	} else
		GS->goodfont = FALSE;

// Add this point, argument font dict has passed safety checks, so make current

	GS->CurrentFont = *tos;		// make argument currentfont

	GS->CharWidthX = 500.0;		// just in case a font doesn't call
	GS->CharWidthY = 0.0;		// setcharwidth...

// IF font has a PaintType entry, use its value for chartype ,else use
// a default value.

	name.obj.nameval = UNIQUE_STRPTR("PaintType");
	if (obj = FindDictEntry(ctxt,tos,&name)) {
		GS->chartype = obj->obj.intval;					// 0..3
	} else {
		GS->chartype = CHAR_RESPONSIBLE;				// default
	}

// IF font has a Metrics entry, cache it, otherwise set gstate Metrics
// entry to NULL.

	name.obj.nameval = UNIQUE_STRPTR("Metrics");
	if (obj = FindDictEntry(ctxt,tos,&name)) {
		GS->Metrics = *obj;				// if present, cache Metrics dict
	} else {
		GS->Metrics = n_obj;			// default (obj.dictval == NULL !!)
	}

// IF font has a CharStrings entry, cache it.

	name.obj.nameval = UNIQUE_STRPTR("CharStrings");
	if (obj = FindDictEntry(ctxt,tos,&name)) {
		GS->CharStrings = *obj;
	} else {
		GS->CharStrings = n_obj;
	}

// That's it, done everything: discard argument

	POPOPER(ctxt);			// clean up stack
	return ERR_OK;
}
/************************************************************************/
error ps_currentfont(DPSContext ctxt) { //	"currentfont" | ...

	ENOUGH_ROOM(1);
	PUSHOPER(ctxt,&GS->CurrentFont);
	return ERR_OK;
}
/************************************************************************/
/************************ SS-HH-OO-WW OPERATORS *************************/
/************************************************************************/

error ps_show		(DPSContext ctxt) { //	string "show" | ...
	
	pso *string,*obj;
	pso name={TYPE_NAME,0,0,0};
	error err;
	float cpx,cpy;
	register char *s;
	unsigned short i;

	NEED_ARGS(1);					// need one string as argument

	string = OPERSP(ctxt);

	if (OBJ_TYPE(string) != TYPE_STRING) return ERR_typecheck;
	if (OBJ_AXES(string) >= ACCESS_EXEC_ONLY) return ERR_invalidaccess;

// See if there's a current font available for show...

	if (! GS->CurrentFont.obj.dictval) return ERR_invalidfont;
	if (! GS->goodfont ) return ERR_invalidfont;
	if (! GS->cp_valid ) return ERR_nocurrentpoint;

	cpx = GS->CurrentPoint[0];
	cpy = GS->CurrentPoint[1];

// Do a gsave so that the character path can be added to a clean path and
// rendered independently from the existing user path.
	if (err = ps_gsave(ctxt)) return err;
	if (err = ps_newpath(ctxt) ) return err;

	GS->CurrentPoint[0] = cpx;
	GS->CurrentPoint[1] = cpy;
	GS->cp_valid = TRUE;

// Allow setcharwidth calls and forbid color operators.
// Switch off dash patterns
	GS->inBuildChar = TRUE;
	PUSHOPER(ctxt,&arr); PUSHINT( 0 ); ps_setdash(ctxt); // [] 0 setdash

// Change the CTM so that huge 1000*1000 char coord system is scaled to
// requested user coord system size.
	PUSHOPER(ctxt,&GS->FontMatrix);	// push FM
	if (err = ps_concat(ctxt)) return err;			// use font's scaling

// For Outline fonts only, set the optional StrokeWidth.

	if (GS->chartype == CHAR_OUTLINED) {
		name.obj.nameval = UNIQUE_STRPTR("StrokeWidth");
		if (obj = FindDictEntry(ctxt,&GS->CurrentFont,&name)) {
			PUSHOPER(ctxt, obj);
			ps_setlinewidth(ctxt);
		}
	}


// Now enter the main loop of show : fetch a character from string and render!

	s = string->obj.stringval;
	i = string->len;

	while (i--) {					// FORALL chars in string: show them
		ShowLetter(ctxt, *s++);
	}

// All characters have been drawn. Grestore to gstate that was current before
// show was called. (Thus restoring the normal CTM,path,inBuildChar flag).
// Before grestoring, save the currentpoint position so that the old gstate
// can be modified to reflect show's pen movements.

	cpx = GS->CurrentPoint[0];
	cpy = GS->CurrentPoint[1];

	ps_grestore(ctxt);

	GS->CurrentPoint[0] = cpx;
	GS->CurrentPoint[1] = cpy;

	POPOPER(ctxt);						// discard string argument
	return ERR_OK;
}
/************************************************************************/
// "kshow" is (unlike its name suggests) quite different from "show".
// Kshow needs to execute two postscript procedures inside two different
// gsave/grestore contexts.
// At the core of kshow lies AWKShow(), the general purpose awidth,width,a
// and k-show character printer.
// The inner loop also needs to push 2 character codes on O-stack.
/************************************************************************/

error ps_kshow		(DPSContext ctxt) { //	proc string "kshow" | ...
	
	pso *str,*pr;
	pso string,proc;
	char *chars;
	error err;

	NEED_ARGS(2);					// need a string and a procedure

	str = OPERSP(ctxt); pr = str +1;

	if (OBJ_TYPE(str) != TYPE_STRING) return ERR_typecheck;
	if (OBJ_AXES(str) >= ACCESS_EXEC_ONLY) return ERR_invalidaccess;
	MUST_BE_PROC(pr);

// Is there a current font? (We need one)
	if (! GS->CurrentFont.obj.dictval) return ERR_invalidfont;
	if (! GS->goodfont ) return ERR_invalidfont;
	if (! GS->cp_valid ) return ERR_nocurrentpoint;

	string = *str;						// copy arguments to local objects
	proc = *pr;

	POPNUMOPER(ctxt,2);					// discard arguments

	while (string.len--) {				// WHILE characters left

		if (err = AWKShowLetter(ctxt, *string.obj.stringval++)) return err;

		if (string.len) {
			chars = string.obj.stringval-1;	// point to prev char
			PUSHINT(*chars++);			// push code of char just drawn
			PUSHINT(*chars++);			// push code of next char to be drawn
	
			if (err = ExecPSobj(ctxt,&proc)) return err;
		}
	}

// All characters have been drawn.

	GS->inBuildChar = FALSE;			// out of show sequence

	return ERR_OK;
}
/************************************************************************/
// This routine just does some preliminary argument checking before calling
// the a/width/awidth show workhorse: awshow().

error ps_ashow		(DPSContext ctxt) { //	ax ay string "ashow" | ...
	
	pso *ax,*ay;

	NEED_ARGS(3);			// a string and two numbers

// no need to "pass" string since string arg is always TOS in all 3 cases
	ay = OPERSP(ctxt) +1; ax = ay + 1;

	return awshow(ctxt, A_SHOW, DUMMY, DUMMY, DUMMY, ax, ay);
}
/************************************************************************/
error ps_widthshow	(DPSContext ctxt) { //	cx cy char string "widthshow" | ...

	pso *charint,*cx,*cy;

	NEED_ARGS(4);			// a string, an int and two numbers

	charint = OPERSP(ctxt) +1; cy = charint +1; cx = charint +2;
	return awshow(ctxt, W_SHOW, cx, cy, charint, DUMMY, DUMMY);
}
/************************************************************************/
error ps_awidthshow	(DPSContext ctxt) { //	cx cy char ax ay string "" | ...

	pso *ax,*ay,*charint,*cx,*cy;

	NEED_ARGS(6);

	ay = OPERSP(ctxt) +1; ax = ay +1; charint = ay+2; cy = ay+3; cx = ay+4;
	return awshow(ctxt, A_SHOW | W_SHOW, cx, cy, charint, ax, ay);
}
/************************************************************************/
// This support routine is common to ashow, widthshow and awidthshow.
// Depending on mode (A_SHOW, W_SHOW or both) it completes the argument checks
// and does the required show variant.
/************************************************************************/

error awshow(DPSContext ctxt, char mode, pso* cx, pso* cy, pso* charint,
										pso* ax, pso* ay) {

pso *string;
float dx,dy;
uchar ch;
register char *s;
int i;
error err;

	string = OPERSP(ctxt);

	if (OBJ_TYPE(string) != TYPE_STRING) return ERR_typecheck;
	if (OBJ_AXES(string) >= ACCESS_EXEC_ONLY) return ERR_invalidaccess;

	if (mode & A_SHOW) {
		MUST_BE_NUMERIC(ax);
		MUST_BE_NUMERIC(ay);
	}

	if (mode & W_SHOW) {
		if (OBJ_TYPE(charint) != TYPE_INT) return ERR_typecheck;
		if (charint->obj.intval > 255) return ERR_rangecheck;
		MUST_BE_NUMERIC(cx);
		MUST_BE_NUMERIC(cy);
	}

	if (! GS->CurrentFont.obj.dictval) return ERR_invalidfont;
	if (! GS->goodfont ) return ERR_invalidfont;
	if (! GS->cp_valid ) return ERR_nocurrentpoint;

	if (mode & A_SHOW) {			// if ax,ay params are valid,
		FORCE_REAL(ax);				// make them floats if they're ints
		FORCE_REAL(ay);
	}

	if (mode & W_SHOW) {
		FORCE_REAL(cx);
		FORCE_REAL(cy);
	}

	if (err = ps_gsave(ctxt) ) return err;	// protect gstate from changes
//--------------------------------------------------------
	s = string->obj.stringval;
	i = string->len;

	while (i--) {							// FORALL chars in string

		ch = *s++;
		if (err = AWKShowLetter(ctxt, ch)) return err;

		dx = dy = 0.0;						// default special displ.

		fixer(dx,dy);				// GET ROUND Lattice BUG **!!

		if (mode & W_SHOW) {
			if ( ch == charint->obj.intval ) {
				dx += cx->obj.realval;		// add cx
				dy += cy->obj.realval;		// add cy
			}
		}

		if (mode & A_SHOW) {
			dx += ax->obj.realval;			// add ax
			dy += ay->obj.realval;			// add ay
		}

		push2reals(ctxt, dx, dy);
		ps_rmoveto(ctxt);
	}
//--------------------------------------------------------
	ps_currentpoint(ctxt);					// preserve advanced pen pointer
	ps_grestore(ctxt);
	ps_moveto(ctxt);

	if (mode == (A_SHOW|W_SHOW)) {			// pop original arguments
		POPNUMOPER(ctxt,6); }
	else if (mode == W_SHOW) {
		POPNUMOPER(ctxt,4); }
	else if (mode == A_SHOW) {
		POPNUMOPER(ctxt,3); }

	return ERR_OK;
}
/************************************************************************/
// This is an optimized version of AWKShow() which doesn't do a gsave/grestore,
// Concat, etc.. per character. (but is still called in a gsave/grestore ctxt).
//
// This 'show' primitive "shows" a character at the CURRENT POINT and
// advances the CP by the character's width.
// It also handles left side bearing displacements if Metrics entries are
// present (only for built-in fonts).
/************************************************************************/
error ShowLetter(DPSContext ctxt, uchar ch) {

float chwx,chwy,lsbx,lsby;
pso charproc,*tos;
error err;

//	DEB (OP,"%c (code:%o)\n",ch,ch);

	GetLeftSideBearing(ctxt, ch, &lsbx, &lsby );

// Since characters are defined using absolute moves,linetos and curvetos
// (relative to 0,0), we've got to TRANSLATE the origin to the current point
// so that the character is printed at the right spot.

// Get Current Point
	ps_currentpoint(ctxt);

// if LEFT side bearing displacement is required, adjust origin.
	if (lsbx != 0.0 || lsby != 0.0) {
		tos = OPERSP(ctxt);
		tos->obj.realval += lsby;
		(tos+1)->obj.realval += lsbx;
	}

// Move the origin to where character should be printed
	if (err = ps_translate(ctxt)) return err;

// Now get character procedure from font and execute it.
	if (err = GetCharProc(ctxt, ch)) return err;
	charproc = * (OPERSP(ctxt));	// and copy locally
	POPOPER(ctxt);					// before restoring O-stack

	if (err = ExecPSobj(ctxt,&charproc)) return err;

// For font types that don't do their own stroke/fill-ing, do it here instead.
	RenderCharPath(ctxt);

	GetCharWidth(ctxt, ch, &chwx, &chwy );
	push2reals(ctxt, chwx, chwy);
	if (err = ps_moveto(ctxt)) return err;

	return ERR_OK;
}

/************************************************************************/
// This show primitive "shows" a character at the CURRENT POINT and
// advances the CP by the character's width.
// It also handles left side bearing displacements if Metrics entries are
// present (only for built-in fonts).
/************************************************************************/
error AWKShowLetter(DPSContext ctxt, uchar ch) {

float chwx,chwy,lsbx,lsby,cpx,cpy;
pso charproc,*obj;
pso arr = {TYPE_ARRAY,0,0,0};
pso name = {TYPE_NAME,0,0,0};
error err;

	GetLeftSideBearing(ctxt, ch, &lsbx, &lsby );

// Since characters are defined using absolute moves,linetos and curvetos
// (relative to 0,0), we've got to TRANSLATE the origin to the current point
// so that the character is printed at the right spot.

// Get Current Point
	if (err = ps_currentpoint(ctxt)) return err;	// does ENOUGH_ROOM(2)

// Do a gsave so that
// - the character path can be started in a clean path
// - the char can be rendered independently from the existing user path.
// - the dash pattern can be disabled temporarily changed 
// - the linewidth can be set to the font's StrokeWidth (for Outline fonts)

	if (err = ps_gsave(ctxt)) return err;
	if (err = ps_newpath(ctxt) ) return err;

// Move the origin to where character should be printed
	if (err = ps_translate(ctxt)) return err;

// Allow setcharwidth calls and forbid color operators.
// Switch off dash patterns, change linewidth if necc.
	GS->inBuildChar = TRUE;
	PUSHOPER(ctxt,&arr); PUSHINT( 0 ); ps_setdash(ctxt); // [] 0 setdash

	if (GS->chartype == CHAR_OUTLINED) {
		name.obj.nameval = UNIQUE_STRPTR("StrokeWidth");
		if (obj = FindDictEntry(ctxt,&GS->CurrentFont,&name)) {
			PUSHOPER(ctxt, obj);
			ps_setlinewidth(ctxt);
		}
	}

// Change the CTM so that huge 1000*1000 char coord system is scaled to
// requested user coord system size.
	PUSHOPER(ctxt,&GS->FontMatrix);	// push FM
	if (err = ps_concat(ctxt)) return err;			// use font's scaling

// if LEFT side bearing displacement is required, additionally translate.
	if (lsbx != 0.0 || lsby != 0.0) {
		fixer(lsbx,lsby);					// get round Lattice BUG **!!
		push2reals(ctxt, lsbx, lsby);
		if (err = ps_translate(ctxt)) return err;
	}

// Now get character procedure from font and execute it.
	if (err = GetCharProc(ctxt, ch)) return err;
	charproc = * (OPERSP(ctxt));	// and copy locally
	POPOPER(ctxt);					// before restoring O-stack

	if (err = ExecPSobj(ctxt,&charproc)) return err;

// For font types that don't do their own stroke/fill-ing, do it here instead.
	RenderCharPath(ctxt);

	GetCharWidth(ctxt, ch, &chwx, &chwy );
	push2reals(ctxt, chwx, chwy);
	if (err = ps_moveto(ctxt)) return err;

	cpx = GS->CurrentPoint[0];
	cpy = GS->CurrentPoint[1];

// Restore all.
	if (err = ps_grestore(ctxt)) return err;

	fixer(cpx,cpy);
	push2reals(ctxt, cpx, cpy);
	ps_itransform(ctxt);
	if (err = ps_moveto(ctxt)) return err;		// and set advanced CP

	return ERR_OK;
}

/************************************************************************/
// Internal routine for all show-type operators.
// Given a character (0..255 code), find the routine in the current font's
// CharStrings dictionary that's responsible for rendering this character.
// Leave procedure on O-stack.
// For USER fonts, put BuildChar proc on stack with its args instead.
/************************************************************************/

error GetCharProc	(DPSContext ctxt, uchar ch) {

	error err;

//	DEB (OP,"%c (code:%o)\n",ch,ch);

	if (GS->FontType == BUILTIN_FONT) {
		ENOUGH_ROOM(3);						// don't blow stack..
	
		PUSHOPER(ctxt,&GS->CharStrings);

		PUSHOPER(ctxt,&GS->Encoding);
		PUSHINT( (uchar) ch);
		if (err = ps_get(ctxt)) return err;	// encodArray index get
											// produces character name
	
		if (err = ps_get(ctxt)) return err;	// CharStrings charname get

	} else
		if (GS->FontType == USER_FONT) {

		ENOUGH_ROOM(4);						// for BuildChar args

		PUSHOPER(ctxt,&GS->CurrentFont);	// BuildChar args
		PUSHINT( (uchar) ch);				// : font char

		PUSHOPER(ctxt,&GS->CurrentFont);
		PUSHNAME("BuildChar");
		if (err = ps_get(ctxt)) return err;	// curfont /BuildChar get
		
	} else {
//		DEB (OP,"Bug: GetCharProcs detects unknown font type !\n");
		return ERR_undefined;
	}

	return ERR_OK;
}

/** GetLeftSideBearing **************************************************/
// Internal routine for ShowLetter primitive.
// If a Metrics dict does NOT exist in the current font, return 0,0.
// If Metrics does exist, look up info in dict instead.
/************************************************************************/

error GetLeftSideBearing (DPSContext ctxt, uchar ch, float *lx, float *ly) {

	pso *tos,*el;
	error err;

	*lx = 0.0;		// default side bearings if nothing explicitly set
	*ly = 0.0;

	if (GS->FontType != USER_FONT ) {
		if (GS->Metrics.obj.dictval) {

			ENOUGH_ROOM(3);						// don't blow stack..
	
			PUSHOPER(ctxt, &GS->Metrics);

			PUSHOPER(ctxt, &GS->Encoding);
			PUSHINT(ch);
			if (err = ps_get(ctxt)) return err;	// encodArray index GET

			if ( ps_get(ctxt) ) {				// Metrics charname get
				POPNUMOPER(ctxt,2);
				return ERR_OK;
			}

// Metric entries can be numbers or ARRAYs, nothing else.

			tos = OPERSP(ctxt);
			switch (OBJ_TYPE(tos)) {
				//-------------------------------------------------------------
				// A single number indicates a new x width (y width is 0)

				case TYPE_INT:
				case TYPE_REAL:	break;
				//-------------------------------------------------------------
				// An array entry can be filled with either 2 or 4 numbers.

				case TYPE_ARRAY:
					el = (pso*) tos->obj.arrayval;		// point to array el#1

					switch (tos->len) {

						//-------------------------------------------
						// number 1 indicates left x side bearing
						// number 2 indicates new x width

						case 2:
							MUST_BE_NUMERIC(el);
							MUST_BE_NUMERIC( (el+1) );

							if (OBJ_TYPE(el)==TYPE_INT) {
								*lx = (float) el->obj.intval;
							} else {
								*lx = el->obj.realval;
							}
		
							break;

						//-------------------------------------------
						// pair 1 indicates left side bearing vector
						// pair 2 indicates new width vector (x & y)

						case 4:
							MUST_BE_NUMERIC(el);
							MUST_BE_NUMERIC( (el+1) );
							MUST_BE_NUMERIC( (el+2) );
							MUST_BE_NUMERIC( (el+3) );

							if (OBJ_TYPE(el)==TYPE_INT) {
								*lx = (float) el->obj.intval;
							} else {
								*lx = el->obj.realval;}
							el++;

							if (OBJ_TYPE(el)==TYPE_INT) {
								*ly = (float) el->obj.intval;
							} else {
								*ly = el->obj.realval;}

						break;

						//-------------------------------------------
						default: //DEB(OP,"Metrics array entry <>2 or 4\n");
							break;
					}
					break;
				//-------------------------------------------------------------
				default: //DEB (OP,"Font Metrics corrupt!\n");
						break;
			}

			POPOPER(ctxt);		// discard Metrics entry
		}
	}

	return ERR_OK;
}
/************************************************************************/
// Internal routine for ShowLetter primitive.
// If a Metrics dict does NOT exist in the current font, just pick up info
// that setcharwidth left behind.
// If Metrics does exist, look up info in dict instead.
/************************************************************************/

error GetCharWidth	(DPSContext ctxt, uchar ch, float *wx, float *wy) {

	pso *tos,*el;
	error err;

	*wx = GS->CharWidthX;
	*wy = GS->CharWidthY;

	if (GS->FontType != USER_FONT ) {
		if (GS->Metrics.obj.dictval) {

			ENOUGH_ROOM(3);						// don't blow stack..
	
			PUSHOPER(ctxt, &GS->Metrics);

			PUSHOPER(ctxt, &GS->Encoding);
			PUSHINT( (uchar) ch);
			if (err = ps_get(ctxt)) return err;	// encodArray index GET

			if ( ps_get(ctxt) ) {				// Metrics charname get
				POPNUMOPER(ctxt,2);
				return ERR_OK;
			}

			tos = OPERSP(ctxt);
			switch (OBJ_TYPE(tos)) {
				//-------------------------------------------------------------
				// A single number indicates a new x width (y width is 0)

				case TYPE_INT:
					*wx = (float) tos->obj.intval;
					*wy = 0.0;
					break;
				//-------------------------------------------------------------

				case TYPE_REAL:
					*wx = tos->obj.realval;
					*wy = 0.0;
					break;
				//-------------------------------------------------------------
				// An array entry can be filled with either 2 or 4 numbers.

				case TYPE_ARRAY:
					el = (pso*) tos->obj.arrayval;		// point to array el#1

					switch (tos->len) {

						//-------------------------------------------
						// number 1 indicates left x side bearing
						// number 2 indicates new x width

						case 2:
							MUST_BE_NUMERIC(el);		//**!! hope this isn't
							MUST_BE_NUMERIC( (el+1) );	// the bug ...

							el++;
							if (OBJ_TYPE(el)==TYPE_INT) {
								*wx = (float) el->obj.intval;
							} else {
								*wx = el->obj.realval;}

							*wy = 0.0;

							break;

						//-------------------------------------------
						// pair 1 indicates left side bearing vector
						// pair 2 indicates new width vector (x & y)

						case 4:
							MUST_BE_NUMERIC(el);
							MUST_BE_NUMERIC( (el+1) );
							MUST_BE_NUMERIC( (el+2) );
							MUST_BE_NUMERIC( (el+3) );

							el +=2;

							if (OBJ_TYPE(el)==TYPE_INT) {
								*wx = (float) el->obj.intval;
							} else {
								*wx = el->obj.realval;}
							el++;

							if (OBJ_TYPE(el)==TYPE_INT) {
								*wy = (float) el->obj.intval;
							} else {
								*wy = el->obj.realval;}

						break;

						//-------------------------------------------
						default: //DEB(OP,"Metrics array entry <>2 or 4(GCW)\n");
							break;
					}
					break;
				//-------------------------------------------------------------
				default: //DEB (OP,"Font Metrics corrupt (GCW)!\n");
					break;
			}

			POPOPER(ctxt);		// discard Metrics entry
		}
	}

	return ERR_OK;
}
/************************************************************************/
// Internal subroutine for show operators: render path representing char.
// Depending on the Font rendering type, stroke or fill the path or leave
// rendering entirely up to the character procedure itself.
/************************************************************************/

error RenderCharPath(DPSContext ctxt) {

error err;

	switch (GS->chartype) {
		case CHAR_FILLED:
			if (err = ps_fill(ctxt)) return err;
			break;
		case CHAR_STROKED:
		case CHAR_OUTLINED:
			if (err = ps_stroke(ctxt)) return err;
		case CHAR_RESPONSIBLE:
			break;
		default: //DEB(OP,"Illegal PaintType in font! (RCP)\n");
			break;
	}
	return ERR_OK;
}

/** ps_stringwidth ******************************************************/
// This routine basically calls "show" within a gsave/grestore pair.
// It disables all drawing so stroke or fill don't alter the page.
/************************************************************************/

error ps_stringwidth(DPSContext ctxt) { //	string "stringwidth" | wx wy

	error err;

	ENOUGH_ROOM(2);							// will need to push 2 numbers

	if (err = ps_gsave(ctxt)) return err;	// make sure path & CP don't change	

	GS->NoOutput = TRUE;	// disable rendering...

	push2reals(ctxt, (float) 0.0 , (float) 0.0);	// 0 0 moveto
	if (err = ps_moveto(ctxt)) {
		ps_grestore(ctxt);
		return err;
	}

	if (err = ps_show(ctxt)) return err;	// advance CP (basically)

	ps_currentpoint(ctxt);					// get CP (= wx wy)
	ps_grestore(ctxt);

	return ERR_OK;
}
/** ps_charpath *********************************************************/
// Basically, do a kind of show that doesn't destroy the character paths
// after executing the character procedure.
// All rendering operators are disabled so stroke or fill don't alter the page.
/************************************************************************/

error ps_charpath(DPSContext ctxt) { //	smallstring bool "charpath" | ...

	pso *bo,*string;
	error err;
	BOOL flag;
	register char *s;
	float cpx,cpy;
	int index,subp,i;

	NEED_ARGS(2);

	bo = OPERSP(ctxt);				// point to Boolean argument.
	string = bo + 1;
	if (OBJ_TYPE(bo)!= TYPE_BOOL) return ERR_typecheck;
	flag = bo->obj.boolval;
	
	if (OBJ_TYPE(string) != TYPE_STRING) return ERR_typecheck;
	if (OBJ_AXES(string) >= ACCESS_EXEC_ONLY) return ERR_invalidaccess;

// See if there's a current font available for charpath...

	if (! GS->CurrentFont.obj.dictval) return ERR_invalidaccess;
	if (! GS->goodfont ) return ERR_invalidaccess;
	if (! GS->cp_valid ) return ERR_nocurrentpoint;

	POPOPER(ctxt);					// discard bool (string is tos)

// Save CP before doing gsave/newpath so that we can have the same CP within
// the loop.
	cpx = GS->CurrentPoint[0];
	cpy = GS->CurrentPoint[1];

// Do a gsave so that the character path can be added to a clean path and
// strokepathed independently from the existing user path.
	if (err = ps_gsave(ctxt)) return err;
	if (err = ps_newpath(ctxt) ) return err;

	GS->CurrentPoint[0] = cpx;		// restore original CP
	GS->CurrentPoint[1] = cpy;
	GS->cp_valid = TRUE;

// disable stroke,fill etc COMPLETELY, i.e. don't even clear the path !
	GS->NoOutput = TRUE;

// Allow setcharwidth calls and forbid color operators.
	GS->inBuildChar = TRUE;

// Switch off dash patterns
	PUSHOPER(ctxt,&arr); PUSHINT( 0 ); ps_setdash(ctxt); // [] 0 setdash

// Change the CTM so that huge 1000*1000 char coord system is scaled to
// requested user coord system size.
	PUSHOPER(ctxt,&GS->FontMatrix);	// push FM
	if (err = ps_concat(ctxt)) return err;			// use font's scaling

	s = string->obj.stringval;
	i = string->len;

	while (i--) {					// FORALL chars in string: build path
		if (err = ShowLetter(ctxt, *s++) ) {
			ps_grestore(ctxt);
			return err;
		}
	}

// All characters are in path. Grestore to gstate that was current before
// show was called. (Thus restoring the normal CTM,path,inBuildChar flag).
// Before grestoring, save the currentpoint position so that the old gstate
// can be modified to reflect show pen movements.

	cpx = GS->CurrentPoint[0];
	cpy = GS->CurrentPoint[1];

	if (flag) {							// in case of 'string TRUE charpath'

		if (err = ps_strokepath(ctxt)) {
			ps_grestore(ctxt);
			return err;					// limitcheck
		}
	}

	index = GS->PathIndex;
	subp = GS->SubPath;
	flag = GS->PathIsCurved;			// 

	ps_grestore(ctxt);

	GS->PathIndex = index;
	GS->SubPath = subp;
	GS->PathIsCurved = flag;

	GS->CurrentPoint[0] = cpx;
	GS->CurrentPoint[1] = cpy;

	POPOPER(ctxt);						// discard string
	return ERR_OK;
}
/************************************************************************/
error ps_cachestatus(DPSContext ctxt) { //
	return ERR_OK;
}
/************************************************************************/
error ps_setcachedevice(DPSContext ctxt) { // wx wy xl yl xh yh "setcachedevice" | ...

	pso *tos,*nos,*wy;			// int or float arguments
	int i;

	NEED_ARGS(6);

// This operator can only be called from within a BuildChar procedure !
	if ( ! GS->inBuildChar) return ERR_undefined;

	tos = OPERSP(ctxt);			// point to the 6 arguments
	nos = tos +1;

	for (i=0; i<6; i++) {
		MUST_BE_NUMERIC(tos+i);
	}
//------
	wy = tos +5;
	if (OBJ_TYPE(wy) == TYPE_REAL)
		GS->CharWidthX = wy->obj.realval;
	else GS->CharWidthX = (float) wy->obj.intval;
//------
	wy--;
	if (OBJ_TYPE(wy) == TYPE_REAL)
		GS->CharWidthY = wy->obj.realval;
	else GS->CharWidthY = (float) wy->obj.intval;
//------

// within this gsave/rest. : no more color operators allowed.

	GS->SCD = TRUE;

	POPNUMOPER(ctxt,6);
	return ERR_OK;
}
/************************************************************************/
error ps_setcharwidth(DPSContext ctxt) { // wx wy "setcharwidth" | ....

	pso *tos,*nos;			// int or float arguments

	NEED_ARGS(2);

// This operator can only be called from within a BuildChar procedure !
	if ( ! GS->inBuildChar) return ERR_undefined;

	tos = OPERSP(ctxt);			// point to the two arguments
	nos = tos +1;

	MUST_BE_NUMERIC(tos);
	MUST_BE_NUMERIC(nos);

//------
	if (OBJ_TYPE(nos) == TYPE_REAL)
		GS->CharWidthX = nos->obj.realval;
	else GS->CharWidthX = (float) nos->obj.intval;
//------
	if (OBJ_TYPE(tos) == TYPE_REAL)
		GS->CharWidthY = tos->obj.realval;
	else GS->CharWidthY = (float) tos->obj.intval;
//------

	POPNUMOPER(ctxt,2);
	return ERR_OK;
}
/************************************************************************/
error ps_setcachelimit(DPSContext ctxt) { //
	return ERR_OK;
}
/************************************************************************/
error ps_setcacheparams(DPSContext ctxt) { //
	return ERR_OK;
}
/************************************************************************/
error ps_currentcacheparams(DPSContext ctxt) { //
	return ERR_OK;
}
/************************************************************************/
// This routine is here PURELY and SIMPLY to fix a bug in our compiler.
// When passing floats to a function, the first time it never works.
// Just call this little proc to absorb the bug away !

void fixer (float a , float b) {
	// **!! WATCH OUT WHEN LETTING COMPILER OPTIMISE CALLING THIS FUNC !
}
/************************************************************************/
