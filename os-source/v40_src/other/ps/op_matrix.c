/************************************************************************

  Module :	Postscript "matrix" operators

  Purpose:	This file contains C-entries for operators called directly
			by the interpreter using the Postscript execution model.
			Also contains the routine that adds all the operators to
			systemdict when initialising everything...
			Also contains support matrix routines

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

#include <math.h>
#include <m68881.h>

//--------------------------------------------------------------------

error ps_matrix			(DPSContext);
error ps_initmatrix		(DPSContext);
error ps_identmatrix	(DPSContext);
error ps_defaultmatrix	(DPSContext);
error ps_currentmatrix	(DPSContext);
error ps_setmatrix		(DPSContext);
error ps_translate		(DPSContext);
error ps_scale			(DPSContext);
error ps_rotate			(DPSContext);
error ps_concat			(DPSContext);
error ps_concatmatrix	(DPSContext);
error ps_transform		(DPSContext);
error ps_dtransform		(DPSContext);
error ps_itransform		(DPSContext);
error ps_idtransform	(DPSContext);
error ps_invertmatrix	(DPSContext);

/* support routines for matrix operators */
void MultMatrix( float *, float *, float *);
error InvertMatrix( float *, float *);

void TransformPoint(float *, float, float, float *, float *);
void DTransformPoint(float *, float, float, float *, float *);

error IsMatrix(ps_obj *);
error IsMatrixSized(ps_obj *);
void FetchMatrix(ps_obj *, float *);
void PutMatrix(ps_obj *, float *);
error CheckMatrixArgs(DPSContext, ps_obj *,int);
error CheckICTM(DPSContext);

//--------------------------------------------------------------------



// a static dummy array object for creating matrix arrays
static ps_obj mat = { TYPE_ARRAY|ATTR_LITERAL,0,6,0 };

// static identity matrix array
static float ident[] = { 1.0,0.0,0.0,1.0,0.0,0.0 };

/************************************************************************/
/************************************************************************

					- MATRIX - matrix

 Creates a 6 element PostScript array object, fills it with the values of
 an identity matrix and pushes this array on the operand stack.
 ************************************************************************/
error ps_matrix(DPSContext ctxt) {

ps_obj *a,*tos;

	ENOUGH_ROOM(1);							// need room for one array
	if(a=AllocVM(VM,6*sizeof(ps_obj))) {	// make an array of floats
		PUSHOPER(ctxt,&mat);				// push the array object
		tos = OPERSP(ctxt);					// get top of stack
		tos->obj.arrayval = (g_obj *)a;		// point at array
		SAVE_LEVEL(tos);					// set the save level
		return(ps_identmatrix(ctxt));		// fill in identity matrix
	}
	else return ERR_VMerror;				// couldn't allocate memory
}

/************************************************************************/
/************************************************************************

					- INITMATRIX -

 Sets the current transformation matrix (CTM) to the default matrix for
 the current output device.  This matrix transforms the default user
 coordinate system to device space.

 Also makes sure that the inverse of CTM is valid.  Since our default
 matrix is currently an identity matrix the inverse is the same anyway.

 Will have to change this to fetch a REAL default CTM for real devices.
 ************************************************************************/
error ps_initmatrix(DPSContext ctxt) {
short i;
float *a = &(ctxt->space.GState->CTM[0]);	// point at CTM
float *b = &(ctxt->space.GState->ICTM[0]);	// and the inverse CTM

	for(i=0; i<6; i++) *a++ = *b++ = ident[i];	// initiialise from identity
	ctxt->space.GState->ICTMValid = TRUE;		// mark inverse matrix as valid
	return ERR_OK;								// all done
}

/************************************************************************/
/************************************************************************

			matrix - IDENTMATRIX - matrix

 Replaces the value of the top matrix on the operand stack with the value
 of the identity matrix and pushes modified matrix back on the operand
 stack.  (Actually, I just do the copy in place here).  The provided
 matrix must be a six element PostScript array of reals and/or integers.
 ************************************************************************/
error ps_identmatrix(DPSContext ctxt) {
ps_obj *tos;
short i;
error err;

	NEED_ARGS(1);			// need a matrix as an argument
	tos = OPERSP(ctxt);		// point at the top argument
	if( (err=IsMatrixSized(tos)) != ERR_OK) return err;	// make sure array is OK

	tos=(ps_obj *)tos->obj.arrayval;	// point at the array members
	for(i=0; i<6; i++) {				// make it an array of 6 floats (identity matrix)
		tos->type = TYPE_REAL|ATTR_LITERAL;
		SAVE_LEVEL(tos);				// jam in the current save level
		tos->len = 0;					// floats have zero length
		tos->obj.realval = ident[i];	// setup the actual float values
		++tos;							// point at next array member
    }
	return ERR_OK;
}

/************************************************************************/
/************************************************************************

			matrix - DEFAULTMATRIX - matrix

 Replaces the value of the top matrix on the operand stack with the
 value of the default matrix for the current output device.  Currently
 just assumes this is an identity matrix.

 Will have to change this to fetch a REAL default CTM for real devices. 
 ************************************************************************/
error ps_defaultmatrix(DPSContext ctxt) {
	return ps_identmatrix(ctxt);	// default is an identity matrix for now
}

/************************************************************************/
/************************************************************************

			matrix - CURRENTMATRIX - matrix

 Replaces the value of the top matrix on the operand stack with the
 value of the current transformation matrix in the graphics state and
 pushes the modified matrix back on the operand stack (actually does the
 copy in place).
 ************************************************************************/
error ps_currentmatrix(DPSContext ctxt) {
float *a = &(ctxt->space.GState->CTM[0]);
ps_obj *tos;
error err;
short i;

	if((err = ps_identmatrix(ctxt)) != ERR_OK) return err;
	tos=OPERSP(ctxt);
	tos=(ps_obj *)tos->obj.arrayval;// point at the array
	i = 6;
	while(i--) {			// now fill in the array from CTM
		tos->obj.realval = *a++;
		++tos;
	}
	return ERR_OK;
}

/************************************************************************/
/************************************************************************

			matrix - SETMATRIX -

 Replaces the CTM with the matrix on the operand stack and marks the
 inverse transformation of the CTM as invalid (will be validated next
 time an inverse CTM is needed).
 ************************************************************************/
error ps_setmatrix(DPSContext ctxt) {
ps_obj *tos,*f;
short i;
float *a;
error err;

	NEED_ARGS(1);						// need one matrix as an argument
	tos = OPERSP(ctxt);					// point at the array object
	if((err = IsMatrix(tos)) != ERR_OK) return err;	// not correct type

	a = &(ctxt->space.GState->CTM[0]);	// point at the CTM
	f=(ps_obj *)tos->obj.arrayval;		// point at supplied array
	for(i=0; i<6; i++) {				// copy array into the CTM
		if(OBJ_TYPE(f) == TYPE_INT) a[i] = (float)f->obj.intval;	// conversion
		else a[i] = f->obj.realval;		// no conversion needed
		++f;
	}

	POPOPER(ctxt);						// pop the matrix
	ctxt->space.GState->ICTMValid = FALSE;	// mark inverse matrix as invalid
	return ERR_OK;						// all done
}

/************************************************************************/
/************************************************************************
 This operator can be called in two different ways :-

			tx ty - TRANSLATE -
	 tx ty matrix - TRANSLATE - matrix

 In both cases, a temporary translation matrix is constructed from tx,ty
 and then concatenated with the CTM (first case) or the supplied matrix
 (second case).  If the matrix to translate is supplied then it is
 returned on the operand stack.  If the CTM is translated then ICTMValid
 is marked FALSE, the inverse transformation of CTM has been invalidated.
 ************************************************************************/
error ps_translate(DPSContext ctxt) {

float tmp[6],tmat[6],*m,tx,ty;
short mthere=FALSE;			// assume translating CTM
ps_obj *tos,*nos;
error err;

	tos=OPERSP(ctxt);
	// we need at least two reals, or two reals and a matrix
	if((err=CheckMatrixArgs(ctxt,tos,2))!=ERR_OK) return err;

	// deal with the case of a supplied matrix
	if(OBJ_TYPE(tos) == TYPE_ARRAY) {
		FetchMatrix(tos,tmp); // fetch to tmp[6] and convert ints if needed
		m = tmp;			// using this matrix
		mthere = TRUE;		// we're translating supplied matrix
		nos=tos+1;			// nos points at ty
		ty = nos->obj.realval;
		++nos;
		tx = nos->obj.realval;
		*nos = *tos;		// copy array to x real (nos=arrayptr)
		POPOPER(ctxt);		// pop old copy of array
		POPOPER(ctxt);		// pop y real
	}

	// deal with the case of translating the CTM
	else {
		m = &(ctxt->space.GState->CTM[0]);
		ctxt->space.GState->ICTMValid = FALSE;	// mark inverse matrix as invalid
	    nos = tos;
		ty = nos->obj.realval;
		++nos;
		tx = nos->obj.realval;
		POPOPER(ctxt);		// pop old copy of array
		POPOPER(ctxt);		// pop y real
	}

	tmat[0]=ident[0]; tmat[1]=ident[1]; tmat[2]=ident[2]; tmat[3]=ident[3];
	tmat[4]=tx; tmat[5]=ty;
	MultMatrix(tmat,m,m);	// do the translation
	if(mthere == TRUE)	{	// if matrix was supplied
		PutMatrix(nos,m);	// copy results back into it
	}
	return ERR_OK;
}

/************************************************************************/
/************************************************************************
 This operator can be called in two different ways :-

			sx sy - SCALE -
	 sx sy matrix - SCALE - matrix

 In both cases, a temporary scaling matrix is constructed from sx,sy
 and then concatenated with the CTM (first case) or the supplied matrix
 (second case).  If the matrix to be scaled is supplied then it is
 returned on the operand stack.  If the CTM is scaled then ICTMValid
 is marked FALSE, the inverse transformation of CTM has been invalidated.
 ************************************************************************/
error ps_scale(DPSContext ctxt) {
float tmp[6],tmat[6],*m,sx,sy;
short mthere=FALSE;			// assume scaling CTM
ps_obj *tos,*nos;
error err;

	tos=OPERSP(ctxt);
	// we need at least two reals, or two reals and a matrix
	if((err=CheckMatrixArgs(ctxt,tos,2))!=ERR_OK) return err;

	// deal with the case of a supplied matrix
	if(OBJ_TYPE(tos) == TYPE_ARRAY) {
		FetchMatrix(tos,tmp); // fetch to tmp[6] and convert ints if needed
		m = tmp;			// using this matrix
		mthere = TRUE;		// we're scaling supplied matrix
		nos=tos+1;			// nos points at sy
		sy = nos->obj.realval;
		++nos;
		sx = nos->obj.realval;
		*nos = *tos;		// copy array to x real (nos=arrayptr)
		POPOPER(ctxt);		// pop old copy of array
		POPOPER(ctxt);		// pop y real
	}

	// deal with the case of scaling the CTM
	else {
		m = &(ctxt->space.GState->CTM[0]);
		ctxt->space.GState->ICTMValid = FALSE;	// mark inverse matrix as invalid
	    nos = tos;
		sy = nos->obj.realval;
		++nos;
		sx = nos->obj.realval;
		POPOPER(ctxt);		// pop old copy of array
		POPOPER(ctxt);		// pop y real
	}

	tmat[1]=ident[1]; tmat[2]=ident[2]; tmat[4]=ident[4]; tmat[5]=ident[5];
	tmat[0]=sx; tmat[3]=sy;
	MultMatrix(tmat,m,m);	// do the scale
	if(mthere == TRUE)	{	// if matrix was supplied
		PutMatrix(nos,m);	// copy results back into it
	}
	return ERR_OK;
}

/************************************************************************/
/************************************************************************
 This operator can be called in two different ways :-

			angle - ROTATE -
	 angle matrix - ROTATE - matrix

 In both cases, a temporary rotation matrix is constructed from angle
 and then concatenated with the CTM (first case) or the supplied matrix
 (second case).  If the matrix to be rotated is supplied then it is
 returned on the operand stack.  If the CTM is rotated then ICTMValid
 is marked FALSE, the inverse transformation of CTM has been invalidated.
 ************************************************************************/
error ps_rotate(DPSContext ctxt) {

float tmp[6],tmat[6],*m,r,sr,cr;
short mthere=FALSE;			// assume rotating CTM
ps_obj *tos,*nos;
error err;

	tos=OPERSP(ctxt);
	// we need at least one real, or one real and a matrix
	if((err=CheckMatrixArgs(ctxt,tos,1)) != ERR_OK) return err;

	if(OBJ_TYPE(tos) == TYPE_ARRAY) {
		FetchMatrix(tos,tmp);
		m = tmp;
		mthere = TRUE;		// we're rotating supplied matrix
		nos=tos+1;			// nos points at angle
		r = nos->obj.realval;
		*nos = *tos;		// copy array to angle real (nos=arrayptr)
		POPOPER(ctxt);		// pop old copy of array
	}
	else {
		m = &(ctxt->space.GState->CTM[0]);
		ctxt->space.GState->ICTMValid = FALSE;	// mark inverse matrix as invalid
		r = tos->obj.realval;
		POPOPER(ctxt);		// pop angle real
	}

	tmat[4]=ident[4]; tmat[5]=ident[5];

	r = r*(PI/180.0);
	sr = sin(r); cr=cos(r);
	tmat[0]=cr; tmat[1]=sr; tmat[2]=-sr; tmat[3]=cr;
	MultMatrix(tmat,m,m);	// do the rotation
	if(mthere == TRUE)	{	// if matrix was supplied
		PutMatrix(nos,m);	// copy results back into it
	}
	return ERR_OK;
}

/************************************************************************/
/************************************************************************

			matrix - CONCAT -

 concatenates the supplied matrix with the CTM and marks the inverse CTM
 as invalid.
 ************************************************************************/
error ps_concat(DPSContext ctxt) {
float tmp[6];
ps_obj *tos;
error err;

	NEED_ARGS(1);			// need a matrix as an argument
	tos=OPERSP(ctxt);		// point at the matrix on operand stack
	if( (err=IsMatrix(tos)) != ERR_OK) return err;
	FetchMatrix(tos,tmp);	// fetch into array of floats
	MultMatrix(tmp,&(ctxt->space.GState->CTM[0]),&(ctxt->space.GState->CTM[0]));
	POPOPER(ctxt);			// pop the supplied matrix
	ctxt->space.GState->ICTMValid = FALSE;	// mark inverse matrix as invalid
	return ERR_OK;
}

/************************************************************************/
/************************************************************************

		matrix1 matrix2 matrix3 - CONCATMATRIX - matrix3

 Replaces the value of matrix3 with the result of multiplying matrix1 and
 matrix2 and pushes the modified matrix3 back on the operand stack.  Does
 not affect the CTM or the inverse of CTM in the current graphics state.
 ************************************************************************/
error ps_concatmatrix(DPSContext ctxt) {

float m1[6],m2[6];
ps_obj *tos,*nos,*bos;
error err;

	NEED_ARGS(3);
	tos=OPERSP(ctxt); nos=tos+1; bos=tos+2;

	if( (err=IsMatrixSized(tos)) != ERR_OK) return err; // just 6 element array
	if( (err=IsMatrix(nos)) != ERR_OK) return err;	// need proper matrices here
	if( (err=IsMatrix(bos)) != ERR_OK) return err;	// need proper matrices here
	FetchMatrix(bos,m1);
	FetchMatrix(nos,m2);
	MultMatrix(m1,m2,m1);			// m3=m1*m2
	*bos = *tos;					// move result matrix into final place
	POPOPER(ctxt); POPOPER(ctxt);	// pop other two matrix objects
	PutMatrix(bos,m1);				// store results in top matrix
	return ERR_OK;					// all done
}

/************************************************************************/
/************************************************************************
 This operator can be called in two different ways :-

			x y - TRANSFORM - x' y'
	 x y matrix - TRANSFORM - x' y'

 With no matrix operand, transforms (x,y) by CTM to produce corresponding
 device space coordinate (x',y').  If the matrix is supplied then we use
 matrix instead of CTM to transform (x,y).  Does not affect CTM.
 ************************************************************************/
error ps_transform(DPSContext ctxt) {
ps_obj *tos,*nos;
float tmp[6],*m;
error err;

	tos=OPERSP(ctxt);
	// need two reals and a matrix or just two reals
	if( (err=CheckMatrixArgs(ctxt,tos,2)) != ERR_OK) return err;

	if(OBJ_TYPE(tos) == TYPE_ARRAY) {
		FetchMatrix(tos,tmp);	// copy supplied matrix to array
		m = tmp;				// using supplied matrix
		++tos;					// point at the reals
		POPOPER(ctxt);			// done with the matrix
	}
	else m = &(ctxt->space.GState->CTM[0]);	// using CTM instead

	nos = tos+1;				// nos points at x, tos points at y;
	TransformPoint(m,nos->obj.realval,tos->obj.realval,
					 &nos->obj.realval,&tos->obj.realval);
	return ERR_OK;				// all done
}

/************************************************************************/
/************************************************************************
 This operator can be called in two different ways :-

			x y - DTRANSFORM - x' y'
	 x y matrix - DTRANSFORM - x' y'

 With no matrix operand, delta transforms (x,y) by CTM to produce a
 device space distance (x',y').  If the matrix is supplied then we use
 matrix instead of CTM to delta transform (x,y).  Does not affect CTM.

 This is almost identical to transform except that tx,ty are not used
 so that we get a positionless "distance" transformation.
 ************************************************************************/
error ps_dtransform(DPSContext ctxt) {
ps_obj *tos,*nos;
float tmp[6],*m;
error err;


	tos=OPERSP(ctxt);
	// need two reals and a matrix or just two reals
	if( (err=CheckMatrixArgs(ctxt,tos,2)) != ERR_OK) return err;

	if(OBJ_TYPE(tos) == TYPE_ARRAY) {
		FetchMatrix(tos,tmp);	// copy supplied matrix to array
		m = tmp;				// using supplied matrix
		++tos;					// point at the reals
		POPOPER(ctxt);			// done with the matrix
	}
	else m = &(ctxt->space.GState->CTM[0]);	// using CTM instead

	nos = tos+1;				// nos points at x, tos points at y;
	DTransformPoint(m,nos->obj.realval,tos->obj.realval,
					 &nos->obj.realval,&tos->obj.realval);
	return ERR_OK;				// all done
}

/************************************************************************/
/************************************************************************
 This operator can be called in two different ways :-

			x' y' - ITRANSFORM - x y
	 x' y' matrix - ITRANSFORM - x y

 With no matrix operand, transforms (x',y') by the inverse of CTM to
 produce a user space coordinate (x,y).  If the matrix is supplied then
 we use the inverse of matrix instead of inverse CTM to transform (x',y').
 Does not affect CTM but may update ICTM if it is initially invalid.
 ************************************************************************/
error ps_itransform(DPSContext ctxt) {
ps_obj *tos,*nos;
float tmp[6],*m;
error err;

	tos=OPERSP(ctxt);
	// need at least two reals or two reals and a matrix
	if( (err=CheckMatrixArgs(ctxt,tos,2)) != ERR_OK) return err;

	if(OBJ_TYPE(tos) == TYPE_ARRAY) {	// matrix supplied as an operand
		FetchMatrix(tos,tmp);			// fetch it to float array
		m = tmp;			// using this matrix
		if( (err=InvertMatrix(m,tmp)) != ERR_OK) return err; // need it inverted
		++tos;				// point at the reals
		POPOPER(ctxt);		// done with the matrix
	}
	else {	// must use inverse of CTM, matrix not supplied
		if((err=CheckICTM(ctxt))!=ERR_OK) return err; 	// cant invert CTM
		m = &(ctxt->space.GState->ICTM[0]);  			// using this matrix
	}

	nos = tos+1;		// nos points at x, tos points at y;
	TransformPoint(m,nos->obj.realval,tos->obj.realval,
					 &nos->obj.realval,&tos->obj.realval);
	return ERR_OK;
}

/************************************************************************/
/************************************************************************
 This operator can be called in two different ways :-

			x' y' - IDTRANSFORM - x y
	 x' y' matrix - IDTRANSFORM - x y

 With no matrix operand, transforms (x',y') by the inverse of CTM to
 produce a user space distance (x,y).  If the matrix is supplied then
 we use the inverse of matrix instead of inverse CTM to transform (x',y').
 Does not affect CTM but may update ICTM if it is initially invalid.
 ************************************************************************/
error ps_idtransform(DPSContext ctxt) {
ps_obj *tos,*nos;
float tmp[6],*m;
error err;

	tos=OPERSP(ctxt);
	// need at least two reals or two reals and a matrix
	if( (err=CheckMatrixArgs(ctxt,tos,2)) != ERR_OK) return err;

	if(OBJ_TYPE(tos) == TYPE_ARRAY) {	// matrix supplied as an operand
		FetchMatrix(tos,tmp);			// fetch it to float array
		m = tmp;			// using this matrix
		if( (err=InvertMatrix(m,tmp)) != ERR_OK) return err; // need it inverted
		++tos;				// point at the reals
		POPOPER(ctxt);		// done with the matrix
	}
	else {	// must use inverse of CTM, matrix not supplied
		if((err=CheckICTM(ctxt))!=ERR_OK) return err; 	// cant invert CTM
		m = &(ctxt->space.GState->ICTM[0]);  			// using this matrix
	}

	nos = tos+1;		// nos points at x, tos points at y;
	DTransformPoint(m,nos->obj.realval,tos->obj.realval,
					 &nos->obj.realval,&tos->obj.realval);
	return ERR_OK;
}


/************************************************************************/
/************************************************************************

		matrix1 matrix2 - INVERTMATRIX - matrix2

 Replaces the value of matrix2 with the result of inverting matrix1 and
 pushes the modified matrix2 back on the operand stack.
 ************************************************************************/
error ps_invertmatrix(DPSContext ctxt) {
float m[6];
ps_obj *tos,*nos;
error err;

	NEED_ARGS(2);		// need two matrices
	tos=OPERSP(ctxt); nos=tos+1;
	if( (err=IsMatrixSized(tos)) != ERR_OK) return err; // just 6 element array
	if( (err=IsMatrix(nos)) != ERR_OK) return err;  // this must be a proper matrix
	FetchMatrix(nos,m);	// fetch the original matrix values
	InvertMatrix(m,m);	// invert the matrix
	*nos=*tos;			// copy matrix 2 back up the stack
	POPOPER(ctxt);		// and pop the original copy
	PutMatrix(nos,m);	// now copy results into the matrix
	return ERR_OK;		// all done
}

/************************************************************************
 Concat matrix1 with matrix2 storing the result in matrix3.  We keep a
 local copy of the result matrix and copy when finished in case source
 and destination matrices are actually the same one.
************************************************************************/
void MultMatrix( float *m1, float *m2, float *m3 ) {

float *mr,tmp[6];	// local copy if destination is same as a source matrix
short i;

	if( (m3==m1) || (m3==m2) ) mr=tmp; else mr=m3;

	mr[0]=m1[0]*m2[0]+m1[1]*m2[2];
	mr[1]=m1[0]*m2[1]+m1[1]*m2[3];
	mr[2]=m1[2]*m2[0]+m1[3]*m2[2];
	mr[3]=m1[2]*m2[1]+m1[3]*m2[3];
	mr[4]=m1[4]*m2[0]+m1[5]*m2[2]+m2[4];
	mr[5]=m1[4]*m2[1]+m1[5]*m2[3]+m2[5];

	if(mr==tmp) {
		i=6; while(i--) *m3++ = *mr++;
	}
}

/************************************************************************
 Invert matrix1 storing the result in matrix2 (this can be the same one)
************************************************************************/
error InvertMatrix( float *m1, float *m2 ) {

float *mr,tmp[6];	// local copy if source and destination are the same
float det;
short i;
	if(m1 == m2) mr=tmp; else mr=m2;

	det=m1[0]*m1[3] - m1[1]*m1[2];
	if(det == 0.0) return ERR_undefinedresult;
	mr[0] = m1[3] / det;
	mr[1] = -m1[1]/det;
	mr[2] = -m1[2]/det;
	mr[3] = m1[0]/det;
	mr[4] = -(m1[4]*mr[0]+m1[5]*mr[2]);
	mr[5] = -(m1[4]*mr[1]+m1[5]*mr[3]);

	if(mr==tmp) {
		i=6; while(i--) *m2++ = *mr++;
	}

	return ERR_OK;
}

/************************************************************************
 Transform x,y by matrix storing x' y' in the given floats.  No errors
************************************************************************/
//void TransformPoint(float *m, float x, float y, float *xr, float *yr) {
//float xt,yt;	// local temporaries
//	xt = m[0]*x+m[2]*y+m[4];  yt = m[1]*x+m[3]*y+m[5];
//	*xr=xt; *yr=yt;
//}

/************************************************************************
 Dtransform x,y by matrix storing x' y' in the given floats.  No errors
************************************************************************/
//void DTransformPoint(float *m, float x, float y, float *xr, float *yr) {
//float xt,yt;	// local temporaries
//	xt = m[0]*x+m[2]*y;  yt = m[1]*x+m[3]*y;
//	*xr=xt; *yr=yt;
//}

/************************************************************************
 Checks that the given object is an array of size 6.  If the object
 is not an array returns ERR_typecheck. If it is the wrong size then
 returns ERR_rangecheck.  Else returns ERR_OK.
************************************************************************/
error IsMatrixSized(ps_obj *o) {
    if(OBJ_TYPE(o) != TYPE_ARRAY) return ERR_typecheck;
	if(o->len != 6) return ERR_rangecheck;
	return ERR_OK;
}

/************************************************************************
 Calls IsMatrixSized and if that succeeds, checks that all members of
 the array are floats.  Returns ERR_typecheck if not, else ERR_OK.
************************************************************************/
error IsMatrix(ps_obj *o) {
short i;
error err;

	if((err=IsMatrixSized(o)) != ERR_OK) return err;

	o = (ps_obj*)o->obj.arrayval;
	i=6;
	while(i--) {					// all 6 array elements must be numbers
		MUST_BE_NUMERIC(o);
		o++;
	}
	return ERR_OK;
}

/************************************************************************
 Packs a matrix object into an array of 6 floats, may convert the ints
************************************************************************/
void FetchMatrix(ps_obj *o, float *a) {
short i;

	o = (ps_obj *)o->obj.arrayval;	/* get pointer to array members */
	i=6;
	while(i--) {
		if(OBJ_TYPE(o)==TYPE_REAL) *a++ = o->obj.realval;
		else *a++ = (float)o->obj.intval;
		++o;
	}
}

/************************************************************************
 Takes array of floats and jams it into the given postscript array object
************************************************************************/
void PutMatrix(ps_obj *o, float *a) {
short i;

	o = (ps_obj *)o->obj.arrayval;	/* get pointer to array members */
	i=6;
	while(i--) {
		o->obj.realval = *a++;
		o->type = TYPE_REAL|ATTR_LITERAL;
		o->len=0;
		++o;
	}
}

/************************************************************************
 Checks arguments for all polymorphic matrix operators that take two
 possible forms :-

	x y matrix - XXXXXXX - [x' y']
		   x y - XXXXXXX - [x' y']

 returns ERR_OK or an appropriate error.  Caller is still responsible
 for checking which form the arguments are in.  Top of operand stack
 is the usual argument to this routine.  All ints are converted to
 floats as required but not in the array.
************************************************************************/
error CheckMatrixArgs(DPSContext ctxt,ps_obj *o,int numreals) {
error err;
int i;

	NEED_ARGS(numreals);		// assume the simple case of no matrix
	if(OBJ_TYPE(o) == TYPE_ARRAY) {	// need numreals plus a matrix
		NEED_ARGS(numreals+1);
		if( (err=IsMatrix(o)) != ERR_OK) return err;
		++o;	// now point at the reals
	}
	for(i=0; i<numreals; i++) {	// check the real args
		if(OBJ_TYPE(o) == TYPE_INT) {
			o->type = TYPE_REAL|ATTR_LITERAL;
			o->obj.realval = (float)o->obj.intval;
		}
		else if(OBJ_TYPE(o) != TYPE_REAL) return ERR_typecheck;
		++o;
	}
	return ERR_OK;
}

/************************************************************************
 Checks if ICTM is the inverse of CTM and inverts it if not.  Checks by
 simply testing the ICTMValid flag for TRUE.  Can return undefinedresult
 if there is no valid inverse matrix for CTM (maybe scaled by 0).
************************************************************************/
error CheckICTM(DPSContext ctxt) {
error err=ERR_OK;
	
	if(ctxt->space.GState->ICTMValid == FALSE) { // if not valid
		err = InvertMatrix(&(ctxt->space.GState->CTM[0]),	// make it valid
						   &(ctxt->space.GState->ICTM[0]));
		if(err == ERR_OK) ctxt->space.GState->ICTMValid=TRUE;
	}
	return err;
}
