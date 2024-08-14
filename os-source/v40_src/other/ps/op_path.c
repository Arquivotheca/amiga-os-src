/************************************************************************

  Module :	Postscript "Path Construction Operators"		© Commodore-Amiga

  Purpose:	This file contains C-entries for operators called directly
			by the interpreter using the Postscript execution model.

  Conventions: -The order in which functions appear is identical to the one
				in the Adobe Red Book (Chapter 6 Operator Summary).
		       -Variables called 'tos' and 'nos' point to the Top Of Stack
				and Next Of Stack elements resp. (on Operand stack).

*************************************************************************/

#include "exec/types.h"
#include "exec/memory.h"

#include "errors.h"
#include "pstypes.h"
#include "stream.h"
#include "memory.h"
#include "objects.h"

#include "misc.h"
#include "stack.h"
#include "gstate.h"
#include "context.h"

#include "proto/exec.h"

#include "math.h"
#include "m68881.h"

#include "stdio.h"
//--------------------------------------------------------------------
error ps_newpath		(DPSContext);
error ps_currentpoint	(DPSContext);

error ps_moveto			(DPSContext);
error ps_rmoveto		(DPSContext);
error ps_lineto			(DPSContext);
error ps_rlineto		(DPSContext);
error ps_arc			(DPSContext);
error ps_arcn			(DPSContext);
error ps_arcto			(DPSContext);
error ps_curveto		(DPSContext);
error ps_rcurveto		(DPSContext);
error ps_closepath		(DPSContext);

error ps_pathforall		(DPSContext);
error intern_pathforall	(DPSContext);

error ps_flattenpath	(DPSContext);
error ps_reversepath	(DPSContext);
error ps_strokepath		(DPSContext);
error ps_pathbbox		(DPSContext);

error ps_initclip		(DPSContext);
error ps_clip			(DPSContext);
error ps_eoclip			(DPSContext);

//--------------------------------------------------------------------
IMPORT error   ps_transform	(DPSContext);
IMPORT error   ps_itransform(DPSContext);
IMPORT error   ps_dtransform(DPSContext);

IMPORT error Bezier	(DPSContext ctxt, float x, float y, float *P);
IMPORT error CopyPrivateToPath(DPSContext);
IMPORT void  TransformPoint  (float *, float, float, float *, float *);
IMPORT void  DTransformPoint (float *, float, float, float *, float *);
IMPORT void  ArrayTransformPoint (float *, float *);
IMPORT void  ArrayDTransformPoint (float *, float *);
IMPORT error CheckICTM		(DPSContext);

IMPORT void sincosr(double,double *,double *,double );
IMPORT error StrokeIt(DPSContext, int);

//--------------------------------------------------------------------
// Prototypes for other functions..

void 		 AddOpsPath	(DPSContext);
float 	* 	NextPathSeg (DPSContext, int slots);
error 			 movlin	(DPSContext ctxt, int segtype, int relative);
error 		 push2reals (DPSContext ctxt, float a, float b);
error		 CurveCommon(DPSContext ctxt, int relative);
int arc(double xc,double yc,double r,double start,double end,int flag, float *results);
error		arc_common  (DPSContext ctxt, int flag);
error		FlattenIt(DPSContext ctxt);

error GetNReals (DPSContext,int, float *);
error PushNReals(DPSContext,int, float *);

/************************************************************************/
/******************* Path Construction Operators ************************/
/************************************************************************/

/************************************************************************/
error ps_newpath(DPSContext ctxt) {	//	- "newpath" | -

register gstate *gs = ctxt->space.GState;

	gs->PathIndex 	=		// Reset all path indices to new start position
	gs->PathStart 	=		// may have been different to PathNew
	gs->SubPath   	=		// really is no subpath now
	gs->PathNew;

	gs->cp_valid    =		// nocurrentpoint
	gs->path_open   =		// no path at all so open is OK
	gs->charpathed  =		// no charpath done yet
	gs->PathIsCurved=FALSE;	// path is flat (very :-)

	return ERR_OK;
}
/************************************************************************/
error ps_currentpoint(DPSContext ctxt) {	// "currentpoint" | x y

register gstate *gs = ctxt->space.GState;
error err = ERR_nocurrentpoint;	// assume we get this error

	if(gs->cp_valid) {
		if(!(err=PushNReals(ctxt,2,gs->CurrentPoint))) {
			if(err=ps_itransform(ctxt)) {
				POPNUMOPER(ctxt,2);
			}
		}
	}
	return err;
}

/************************************************************************/
error ps_moveto(DPSContext ctxt) {	//	num num "moveto" | ...
	return movlin(ctxt,SEG_MOVETO,FALSE);
}
/************************************************************************/
error ps_lineto(DPSContext ctxt) {	//	num num "lineto" | ...
	return movlin(ctxt,SEG_LINETO,FALSE);
}
/************************************************************************/
error ps_rmoveto(DPSContext ctxt) {	// dx dy "rmoveto" | ...
	return movlin(ctxt,SEG_MOVETO,TRUE);
}
/************************************************************************/
error ps_rlineto(DPSContext ctxt) {	// dx dy "rlineto" | ...
	return movlin(ctxt,SEG_LINETO,TRUE);
}
/************************************************************************
 Common routine for absolute and relative moveto or lineto.  Checks that
 args are correct types and calls transform or dtransform routines
 directly.  This is a lot quicker than call ps_[d]transform since there
 is no need to check for a matrix argument (in fact that would be wrong
 anyway).  We only take two numbers on the stack.
************************************************************************/
error movlin(DPSContext ctxt, int segtype, int relative) { // num num "[r]moveto" | "[r]lineto"

register gstate *gs = ctxt->space.GState;
register float *ptr;
register short offset;
error err;

	// if lineto then we need to ensure current point is valid
	if((segtype==SEG_LINETO) && (!gs->cp_valid)) return ERR_nocurrentpoint;

	// make sure there is room for one path segment ([r]moveto or [r]lineto)
	if((offset=gs->PathIndex)==gs->PathArgs) return ERR_limitcheck;

	// calculate offset into path buffer for storing x,y arguments
	ptr=(float *)(gs->PathBase+gs->PathArgs + (offset << 3));

	// fetch two floats from operand stack and store as an x,y pair in path
	if(err=GetNReals(ctxt,2,ptr)) return err;

	if(relative) {
		ArrayDTransformPoint(gs->CTM,ptr);
		ptr[0] += gs->CurrentPoint[0];
		ptr[1] += gs->CurrentPoint[1];
	}
	else ArrayTransformPoint(gs->CTM,ptr);

	// if moveto: start new subpath and drop ptr to this spot
	if(segtype==SEG_MOVETO) gs->SubPath = offset;

	*(gs->PathBase+offset)=segtype;	// add moveto or lineto segment code
	gs->PathIndex++;				// update index to next free slot

	gs->CurrentPoint[0] = ptr[0];	// update currentpoint
	gs->CurrentPoint[1] = ptr[1];
	gs->cp_valid  = TRUE;
	gs->path_open = TRUE;			// in both cases (move & line)

	POPNUMOPER(ctxt,2);				// discard arguments
	return ERR_OK;					// no errors
}

/************************************************************************/
error ps_arc(DPSContext ctxt) {	//
	return arc_common(ctxt,FALSE);
}
/************************************************************************/
error ps_arcn(DPSContext ctxt) {	//
	return arc_common(ctxt,TRUE);
}
/************************************************************************/
error ps_arcto(DPSContext ctxt) {	//
	return ERR_NOT_IMPLEMENTED;
}

/*****************************************************************************

 error = arc_common(DPSContext ctxt, flag)

 Common routine for arc and arcn.  Checks all arguments and calls arc with
 the appropriate flag value for clockwise or counter-clockwise arc segments.
*****************************************************************************/
error arc_common(DPSContext ctxt, int flag) {

register gstate *gs = ctxt->space.GState;
register float *ptr;
register short segs,copairs,offset;
register uchar *path;
error err;
float args[5];	// unpack arc arguments into here

	// make sure there is room for the worst case of 13 segments (moveto
	// followed by 4 curveto triplets.  Curveto is only counted as one
	// segment but it is actually 3 separate segment types (CURVE,C2,C3)
	offset=gs->PathIndex;		// fetch current offset
	if( (gs->PathArgs-offset) < 13) return ERR_limitcheck;

	// fetch five floats from op stack and store in buffer (x,y,r,ang1,ang2)
	if(err=GetNReals(ctxt,5,args)) return err;

	// calculate offset into path buffer for storing x,y arguments
	ptr=(float *)(gs->PathBase+gs->PathArgs + (offset << 3));

	// generate bezier control points directly into args area of path
	segs=arc(args[0],args[1],args[2],args[3],args[4],flag,ptr);

	// fill in initial moveto or lineto based on validity of currentpoint
	path=gs->PathBase+offset;
	if( gs->cp_valid ) {		// currentpoint is valid
		*path++ = SEG_LINETO;	// just do a lineto first point
	}
	else {						// currentpoint invalid
		*path++ = SEG_MOVETO;	// do a moveto first point
		gs->SubPath=offset;		// set new subpath
		gs->cp_valid = TRUE;	// currentpoint is valid now (filled in later)
	}

	// fill in SEG_CURVETO,SEG_CTRL_PT,SEG_CTRL_PT for each bezier curve
	copairs=segs;
	while(copairs--) {
		*path++ = SEG_CURVETO;
		*path++ = SEG_CTRL_PT2;
		*path++ = SEG_CTRL_PT3;
	}

	// adjust path fields to reflect segments we just added
	copairs = segs*3+1;			// number of co-ordinate pairs
	gs->PathIndex += copairs;	// also number of pathcodes added to path

	// transform all co-ordinates/control points to device space in place
	while(copairs--) {
		ArrayTransformPoint(gs->CTM,ptr);
		ptr+=2;
	}

	gs->CurrentPoint[0] = *(ptr-2);	// update currentpoint
	gs->CurrentPoint[1] = *(ptr-1);	// ptr was bumped once too often
	gs->path_open =	gs->PathIsCurved = TRUE;

	POPNUMOPER(ctxt,5);		// discard original args
	return ERR_OK;			// all done
}

/*****************************************************************************

 numsegs = arc(xc,yc,start_angle,end_angle,radius,flag,buffer);

 This routine uses bezier curve segments to approximate an arc.
 Some simple trig is used to find the chord length and subsequent
 tangent vectors. A magic number is used to determine the magnitude
 of the tangent vectors with respect to the chord length.
 This code is used for clockwise and anti-clockwise arcs. The direction
 of the arc is determined by the flag value
             0 - arc
             1 - arcn

 (!! see below)If the arc is clockwise (arcn) then we negate the chord_length.
 Eh, I hear you say? If we assume the control points are listed in an
 anti-clockwise direction then we rotate the radius vectors 90 deg to get
 the tangent vectors. These are used for the bezier segment.
 P1 is radius Vector rotated +90, and P2 is the same except angle is -90 deg.
 Now if we give the control points in a clockwise direction and use the above
 formula, the tangent vectors end up facing away from each other. Therefore
 by negating the chord_length the multiplication below reverses the 
 tangent vectors thus it elegantly 'kills two birds with one stone'.

 segs is the total number of curveto segments that have been added to buffer.
 On return, buffer will contain an x,y co-ordinate pair (the start point of
 the curve) followed by curveto co-ordinate triplets (maximum of 4). Buffer
 must be large enough to hold 26 floats (x,y plus 4 sets of control points).

 Point0 = prevpt[0] and prevpt[1]	(original point and previous endpoint)

 Point1 = result[0] and result[1]	(control points for bezier curve segments)
 Point2 = result[2] and result[3]
 Point3 = result[4] and result[5]
*****************************************************************************/
int arc(double xc,double yc,double r,double start,double end,int flag, float *result) {

#define PI_180 (PI/180.0)
#define MAGIC_NUMBER 0.394

double chord_length,tx,ty,step,diff;
double theta,tmp,inc90;
double c,s,dx,dy;
int loop,segs;
float *prevpt;

	// Gap between angles is determined according to direction.
	start *= PI_180; end *= PI_180;
	if(!flag) { diff = end - start;	inc90 = PID2; }
	else { diff = start - end;	inc90 = -PID2; }
	if(diff<=0.0) diff += (PI*2.0);

	// If we divide the arc into 90 degree segments it makes life easier.
	// Therefore we calculate the acute angle left by this quartering

	step = fmod(diff,PID2);	
	if(step==0.0) step = PID2;
	if(flag) step = -step;

	// figure out how many curveto segments we are going to add
	tmp=diff-step;
	segs = loop = (1 + (int)(tmp/PID2));	// save segs for return value

	// calculate start point of arc for moveto or lineto and save in buffer
	sincosr(start,&c,&s,r);
	prevpt = result;				// stash pointer to start point
	*result++ = (xc + c);			// result points where first...
	*result++ = (yc + s);			// ...bezier control points go

	while(loop--) {
		theta = fmod((start+step),(PI*2.0));
		sincosr(theta,&c,&s,r);
		result[4] = xc + c;
		result[5] = yc + s;

		dx=prevpt[0]-result[4]; dy=prevpt[1]-result[5];
		chord_length = sqrt(dx*dx+dy*dy);

		// (!! see above)
		if(flag) chord_length = -chord_length;

		tx = (prevpt[0]-xc)/r;
		ty = (prevpt[1]-yc)/r;

		result[0] = (-ty*(chord_length*MAGIC_NUMBER))+prevpt[0];
		result[1] =  (tx*(chord_length*MAGIC_NUMBER))+prevpt[1];

		tx = (result[4]-xc)/r;
		ty = (result[5]-yc)/r;

		result[2] =  (ty*(chord_length*MAGIC_NUMBER))+result[4];
		result[3] = (-tx*(chord_length*MAGIC_NUMBER))+result[5];

		start = fmod((start+step),(PI*2.0));
		step = inc90;		// not doing fractional bits anymore

		// point prevpt at the last point we calculated (result[4],result[5])
		prevpt += 6;
		result += 6;
	}
	return segs;
}

/************************************************************************/
error ps_curveto(DPSContext ctxt) {	//	x1 y1  x2 y2  x3 y3 "curveto" | ...
	return CurveCommon(ctxt,FALSE);
}

/************************************************************************/
error ps_rcurveto(DPSContext ctxt) {	//	dx1 dy1  dx2 dy2  dx3 dy3 "curveto" | ...
	return CurveCommon(ctxt,TRUE);
}

/*************************************************************************
 This routine is common to both curveto and rcurveto.  It simply takes an
 extra flag to determine if the given control points are relative to the
 currentpoint.  Controls points are transformed to device space (or
 dtransformed for rcurveto) and stored as 3 x,y pairs in the current path.
**************************************************************************/
error CurveCommon(DPSContext ctxt, int relative) {

register gstate *gs=ctxt->space.GState;
register float *ptr,*tmp;
register short offset,i;
error err;

	// currentpoint must be valid for curveto and rcurveto to work
	if (!gs->cp_valid) return ERR_nocurrentpoint;

	// make sure there is room in the path for 3 control points
	offset=gs->PathIndex;		// fetch current offset
	if( (gs->PathArgs-offset) < 3) return ERR_limitcheck;

	// calculate offset into path buffer for storing x,y arguments
	ptr=(float *)(gs->PathBase+gs->PathArgs + (offset << 3));

	// fetch six floats from op stack and store in path args area
	if(err=GetNReals(ctxt,6,ptr)) return err;

	// fill in SEG_CURVETO,SEG_CTRL_PT,SEG_CTRL_PT for bezier curve
	*(gs->PathBase+offset)   = SEG_CURVETO;
	*(gs->PathBase+offset+1) = SEG_CTRL_PT2;
	*(gs->PathBase+offset+2) = SEG_CTRL_PT3;

	// transform all control points to device space in place
	tmp=ptr;
	for(i=0; i<3; i++) {
		if(relative) {
			ArrayDTransformPoint(gs->CTM,tmp);
			tmp[0] += gs->CurrentPoint[0];
			tmp[1] += gs->CurrentPoint[1];
		}
		else {
			ArrayTransformPoint(gs->CTM,tmp);
		}
		tmp += 2;
	}

	// adjust path indices to reflect the curveto segment we just added
	gs->PathIndex += 3;				// adjust pathcodes added to path

	// ensure currentpoint and path state flags are valid
	gs->CurrentPoint[0] = ptr[4];	// update currentpoint
	gs->CurrentPoint[1] = ptr[5];
	gs->path_open = TRUE;			// path will now be open
	gs->PathIsCurved = TRUE;		// and it's definitely curved

	POPNUMOPER(ctxt,6);				// discard original args
	return ERR_OK;
}


/************************************************************************
 Only if a path exists and is currently open, add a close segment to path.
 We store the original start co-ordinates of the path after the close
 token to make it easier for stroke and fill (no need to save subpath ptr).
************************************************************************/
error ps_closepath(DPSContext ctxt) {	// ... "closepath" | ...

register gstate *gs=ctxt->space.GState;
register float *ptr,*tmp;
register short offset;

	// if path is alredy closed or current subpath is empty, do nothing
	if( (!gs->path_open) || (gs->SubPath==gs->PathIndex) ) return ERR_OK;

	// make sure there is room for one path segment (closepath)
	if((offset=gs->PathIndex)==gs->PathArgs) return ERR_limitcheck;

	// calculate offset into path buffer for storing x,y arguments
	ptr=(float *)(gs->PathBase+gs->PathArgs + (offset << 3));

	// calculate offset to original moveto of subpath (device coords)
	tmp=(float *)(gs->PathBase+gs->PathArgs + (gs->SubPath << 3));

	// copy original moveto coords into currentpoint and closepath args
	gs->CurrentPoint[0]=ptr[0]=tmp[0];
	gs->CurrentPoint[1]=ptr[1]=tmp[1];

	*(gs->PathBase+offset)=SEG_CLOSE;	// stuff in the CLOSEPATH token
	gs->PathIndex += 1;					// added one token
	gs->path_open = FALSE;				// path not open anymore
	gs->cp_valid  = TRUE;				// currentpoint is valid now

	gs->SubPath = gs->PathIndex;		// no current subpath

	return ERR_OK;
}

/************************************************************************

 error = FlattenIt(DPSContext ctxt)

 Copies the current path into the private path area flattening curveto
 segments into a bunch of linetos.  Returns ERR_limitcheck if the private
 path overflows, else returns ERR_OK.
************************************************************************/
error FlattenIt(DPSContext ctxt) {

register gstate *gs=ctxt->space.GState;
register uchar  *segs,*psegs;
register float *coords,*pcoords;
register short c,ppi,ppm;

error err;

	c=gs->PathStart;					// stash initial offset
	ppm=gs->PathArgs;					// stash maximum offset

	segs=gs->PathBase+c;				// get pointer to start of path
	coords=(float *)(gs->PathBase+ppm+(c << 3));

	gs->PrivPathIndex=ppi=0;				// reset private path pointer
	psegs=gs->PrivatePath;					// get pointer into private path
	pcoords=(float *)(gs->PrivatePath+ppm);	// get pointer to private coords

	while((c++) < gs->PathIndex) {
		if(ppi>=ppm) return ERR_limitcheck;	// ran out of space in private path
		switch( *segs ) {
			case SEG_MOVETO:				// simply copy op,x,y for these
			case SEG_LINETO:
			case SEG_CLOSE:
				*psegs++   = *segs++;
				*pcoords++ = *coords++;
				*pcoords++ = *coords++;
				++ppi;
				break;

			case SEG_CURVETO:				// convert curve to lines
				gs->PrivPathIndex=ppi;		// flush new index for Bezier
				if(err=Bezier(ctxt,*(coords-2),*(coords-1),coords)) return err;

				ppi=gs->PrivPathIndex;		// get new private index and ptrs
				psegs=gs->PrivatePath+ppi;
				pcoords=(float *)(gs->PrivatePath+ppm+(ppi << 3));

				coords += 6;				// bump the current source pointer
				segs += 3;					// skip bezier control point codes
				c+=2;						// curveto uses 3 segments
				break;

			default:
				printf("BAD OP IN FLATTENPATH\n");
		}
	}	
	gs->PrivPathIndex=ppi;	// flush private path index
	return ERR_OK;
}

/************************************************************************/
error ps_flattenpath(DPSContext ctxt) {	//	"flattenpath" | ...

register gstate *gs=ctxt->space.GState;

error err;

	if(!gs->PathIsCurved) return ERR_OK;	// nothing to do
	if(err=FlattenIt(ctxt)) return err;

	// path has been flattened into the private path area, check
	// if it will fit back into the active path area after newpath
	if(gs->PrivPathIndex > (gs->PathArgs-gs->PathNew)) return ERR_limitcheck;

	// current path can be replaced by private path, make it so Mr. Data.
	ps_newpath(ctxt);				// clear current path
	CopyPrivateToPath(ctxt);		// append private path to real one
	gs->PathIsCurved = FALSE;		// doesn't need flattening anymore
	return ERR_OK;
}

/************************************************************************/
error ps_reversepath(DPSContext ctxt) {	//
	return ERR_NOT_IMPLEMENTED;
}
/************************************************************************/
error ps_strokepath(DPSContext ctxt) {	//

register gstate *gs=ctxt->space.GState;
error err;

	if(err = StrokeIt(ctxt,TRUE)) return err;	// stroke saving the path
	if(gs->PrivPathIndex > (MAXPATH-gs->PathNew)) return ERR_limitcheck;
	CopyPrivateToPath(ctxt);		// append private path to real one
	return ERR_OK;
}
/************************************************************************/
error ps_pathbbox(DPSContext ctxt) {	//	.. "pathbbox" | x1 y1  x2 y2

register gstate *gs=ctxt->space.GState;
register float *coords;
register short count;
float x,y;
error err;

float limits[4];	// 0=minx, 1=miny, 2=maxx, 3=maxy

	// if path is empty then we return an error
	if(count=(gs->PathIndex-gs->PathStart)) { // something in the path
		if ( err = CheckICTM(ctxt)) return err;	// make sure ITransform is OK
		coords=(float *)(gs->PathBase+gs->PathArgs+(gs->PathStart<<3));
		limits[0]=limits[1]= 3.0e30;	// set min to extreme positive values
		limits[2]=limits[3]=-3.0e30;	// set max to extreme negative values

		// process all co-ordinate pairs to make a bounding box in limits
		while(count--) {
			x=*coords++; y=*coords++;
			if(x<limits[0]) limits[0]=x;
			if(x>limits[2]) limits[2]=x;
			if(y<limits[1]) limits[1]=y;
			if(y>limits[3]) limits[3]=y;
		}

		// inverse transform bounding box back to user space
		ArrayTransformPoint(gs->ICTM,limits);
		ArrayTransformPoint(gs->ICTM,limits+2);

		// ensure min and max are correct (could be wrong if rotated)
		if(limits[0]>limits[2]) {x=limits[0];limits[0]=limits[1];limits[1]=x;}
		if(limits[1]>limits[3]) {y=limits[1];limits[1]=limits[3];limits[3]=y;}

		// now push arguments onto the operand stack as four reals
		return PushNReals(ctxt,4,limits);
	}
	return ERR_nocurrentpoint;
}

/************************************************************************/
error ps_initclip(DPSContext ctxt) {	//
	return ERR_NOT_IMPLEMENTED;
}
/************************************************************************/
error ps_clip(DPSContext ctxt) {	//
	return ERR_NOT_IMPLEMENTED;
}
/************************************************************************/
error ps_eoclip(DPSContext ctxt) {	//
	return ERR_NOT_IMPLEMENTED;
}


/************************************************************************

 error = GetNReals(ctxt,count,*reals)

 Fetches count numbers from the operand stack and stores them as reals
 in the given float array.  Ints are converted to reals as needed. The
 values are not popped from OStack (nor are they altered there).  Also
 fetches values in reverse stacked order so they appear in the buffer
 in the same order in which they were stacked.
************************************************************************/
error GetNReals(DPSContext ctxt, int count, float *reals) {

register pso *tos;
register short type;

	NEED_ARGS(count);		// return ERR_stackunderflow if not enough

	// check args are numeric and convert ints to floats as needed.
	tos=OPERSP(ctxt);				// point to object on top of OStack
	tos += (count-1);				// now point to first one to fetch
	while(count--) {				// while more to convert
		type = OBJ_TYPE(tos);		// fetch the type
		if(type==TYPE_REAL) *reals++ = tos->obj.realval;	// already real
		else if(type==TYPE_INT) *reals++ = (float)(tos->obj.intval);
		else return ERR_typecheck;	// not int or real
		--tos;						// next one down the stack
	}
	return ERR_OK;
}

/************************************************************************

 error = PushNReals(ctxt,count,*reals)

 Pushes count reals onto the operand stack if there is room for them.
************************************************************************/
error PushNReals(DPSContext ctxt, int count, float *reals) {

	ENOUGH_ROOM(count);		// return ERR_stackoverflow if not enough
	while(count--) {
		PUSHREAL(*reals++);
	}

	return ERR_OK;
}

/************************************************************************/
// Internal convenience routine to push two reals on the O-stack
error push2reals(DPSContext ctxt, float a, float b) {

	ENOUGH_ROOM(2);							// don't blow the stack !
	PUSHREAL( a );							// push it (X coordinate)
	PUSHREAL( b );							// push it (Y coordinate)
	return ERR_OK;
}
/************************************************************************/

// --------------------------------------------------------------
struct pathfor_cb {			// define "Pathforall" Control Block
	int pathptr;			// running path ptr (index into path)
	long int segs;			// number of segments left
};
// --------------------------------------------------------------
error ps_pathforall(DPSContext ctxt) {	//	move line curve close "pathforall"

pso *move,*line,*curve,*close;
pso intern;
gstate *gs;
short segs;
struct pathfor_cb pfacb;

	NEED_ARGS(4);

	close = OPERSP(ctxt);		// point to procedures
	curve = close +1;
	line  = close +2;
	move  = close +3;

	for(segs=0; segs<4; segs++) { MUST_BE_PROC((close+segs)); };
//	MUST_BE_PROC(curve);
//	MUST_BE_PROC(line);
//	MUST_BE_PROC(move);

	gs = ctxt->space.GState;				// cache gstate ptr.
	
	if (gs->charpathed)						// if path contains a character def
		return ERR_invalidaccess;			// you're not allowed to peek !


	if (gs->PathStart<gs->PathIndex) {	// if path is empty: don't do anything
	
		ENOUGH_EROOM(6);		// 4 procs, 1 control, 1 operator
	
		PUSHEXEC(ctxt,close);
		PUSHEXEC(ctxt,curve);
		PUSHEXEC(ctxt,line);
		PUSHEXEC(ctxt,move);
	
		pfacb.pathptr = gs->PathStart;	// not a ptr anymore, using index
	
		intern.type = TYPE_OPERATOR | ATTR_EXECUTE;
		intern.obj.operval = intern_pathforall;
		intern.tag = 0;
		intern.len = 5;			// size of SF above operator slot
	
		PUSHEXEC(ctxt,((pso*) &pfacb));		// push control block
		PUSHEXEC(ctxt,(&intern));			// push looping operator
	}

    POPNUMOPER(ctxt,4);						// discard copy of procs
   	return ERR_OK;
}
/************************************************************************/
// This internal operator's SF looks like this:
// SF: control move line curve close

error intern_pathforall(DPSContext ctxt) {

register gstate *gs = ctxt->space.GState;
register float *coords;
error err;

	struct pathfor_cb *exec;

	exec = (struct pathfor_cb*) EXECSP(ctxt);

	if (exec->pathptr >= gs->PathIndex) {		// if no more segments in path : done !
		POPEXEC(ctxt);
		POPEXEC(ctxt); POPEXEC(ctxt); POPEXEC(ctxt); POPEXEC(ctxt);
		return ERR_OK;
	} else {				// if entire path not done yet, ....

		// before doing anything, get EXEC stack back in order.  Make
		// sure intern pathforall is ready to be picked up again
		PUSHEXEC(ctxt, ((pso*)exec-1));	// push another internal pforall

		exec->segs--;		// decr counter in control block

		// fetch pointer to float coordinates for current segment
		coords=(float *)(gs->PathBase+gs->PathArgs+(exec->pathptr<<3));

		switch ( *(gs->PathBase + exec->pathptr++) ) {

			//-------------------------------------------------------------
			case SEG_MOVETO:
				if(err=PushNReals(ctxt,2,coords)) return err;
				if(err=ps_itransform(ctxt)) return err;	// device coords -> user space

				PUSHEXEC(ctxt, ((pso*)exec+1));	// push procedure to be run
				break;

			//-------------------------------------------------------------
			case SEG_LINETO:
				if(err=PushNReals(ctxt,2,coords)) return err;
				if(err=ps_itransform(ctxt)) return err;	// device coords -> user space

				PUSHEXEC(ctxt, ((pso*)exec+2));	// LINETO procedure
				break;

			//-------------------------------------------------------------
			case SEG_CURVETO:
				ENOUGH_ROOM(6);			// need room for 3 x,y pairs on stack

				if(err=PushNReals(ctxt,2,coords)) return err;
				if(err=ps_itransform(ctxt)) return err;	// device coords -> user space

				if(err=PushNReals(ctxt,2,coords+2)) return err;
				if(err=ps_itransform(ctxt)) return err;	// device coords -> user space

				if(err=PushNReals(ctxt,2,coords+4)) return err;
				if(err=ps_itransform(ctxt)) return err;	// device coords -> user space

				PUSHEXEC(ctxt, ((pso*)exec+3));	// CURVETO
				exec->pathptr += 2;		// skip CTRL_PT segments
				break;

			//-------------------------------------------------------------
			case SEG_CLOSE:
				PUSHEXEC(ctxt, ((pso*)exec+4));	// CLOSEPATH
				break;

			//-------------------------------------------------------------
			default: printf ("Unknown path SEG type!\n");
//					printf("SEG TYPE:%d\n", *exec->pathptr );
		}
		return ERR_OK;
	}
}
/************************************************************************/
