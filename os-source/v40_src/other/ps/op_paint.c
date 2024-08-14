/************************************************************************

  Module :	Postscript "Painting Operators"		© Commodore-Amiga

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

#include "exec/memory.h"

#include "misc.h"
#include "stack.h"
#include "gstate.h"
#include "context.h"

#include "math.h"
#include "m68881.h"
#include "stdio.h"

#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/intuition.h>

//--------------------------------------------------------------------
error ps_eofill		(DPSContext);
error ps_fill		(DPSContext);
error ps_stroke		(DPSContext);

// these are here to give us a display for writing to, just testing
error ps_klugeon	(DPSContext);
error ps_klugeoff	(DPSContext);
error ps_clearscreen (DPSContext);

//--------------------------------------------------------------------

IMPORT error ps_newpath(DPSContext ctxt);
IMPORT void TransformPoint(float *, float, float, volatile float *, volatile float *);
IMPORT void DTransformPoint(float *, float, float, volatile float *, volatile float *);
IMPORT error CheckICTM(DPSContext);
IMPORT error ps_flattenpath	(DPSContext);

IMPORT error CopyPrivateToPath(DPSContext);
IMPORT error FlattenIt(DPSContext);
IMPORT void  ArrayTransformPoint (float *, float *);
IMPORT void  ArrayDTransformPoint (float *, float *);
IMPORT void  MultMatrix( float *, float *, float *);

IMPORT error Bezier(DPSContext ctxt, float x, float y, float *P);

IMPORT void fixer(float,float);

/************************************************************************
 Structure used for all private path rendering operations.
************************************************************************/
typedef struct LineSegment {
	float	x1,y1;			// start point of this line in user space
	float	x2,y2;			// end point (start of next line segment too)
	float	dx,dy,len;		// unit vectors and length in user space
	float	rdx,rdy;		// deltas rotated 90 deg anticlockwise (user space)
} lineseg;

//--------------------------------------------------------------------
// Prototypes for other functions..

error StrokeIt(DPSContext, int);
void  PrivateFill(DPSContext);
error DoJoin(DPSContext, lineseg *seg1, lineseg *seg2, float *P,int savepath);
error DoFirstCap(DPSContext, lineseg *seg1, float *P, int savepath);
error DoLastCap(DPSContext, lineseg *seg1, float *P, int savepath);
error DoFullCircle(DPSContext ctxt, float x, float y, float r, int savepath);

// these are here so that we can keep lib pointers in the context
#define IntuitionBase ctxt->IntuitionBase
#define GfxBase ctxt->GfxBase

// a temp screen that we render to after calling the klugeon operator
static struct NewScreen ns = {
	0,0,640,400,4,		/* left, top, width, height, depth */
	0,15,				/* detailpen, blockpen */
	HIRES|LACE|SPRITES,	/* viewmodes */
	CUSTOMSCREEN,		/* type */
	0,"Amiga PostScript",				/* font, title */
	0,0					/* gadgets, bitmap */
};

struct NewWindow mnw = {
		0,0,			/* Starting corner */
		640,400,		/* Width, height */
		0,0,			/* detail, block pens */
	    0,				/* IDCMP flags */
	    BACKDROP|BORDERLESS, /* Window flags */
		NULL,			/* Pointer to first gadget */
		NULL,			/* Pointer to checkmark */
		0,				/* title */
		NULL,			/* screen pointer */
		NULL,			/* bitmap pointer */
		50,50,640,400,		/* min and max sizes */
		CUSTOMSCREEN		/* type of screen */
};

// the colors we will be using for this screen (white background)
static short __c[] = { 0xfff,0xeee,0xddd,0xccc,0xbbb,0xaaa,0x999,0x888,
					   0x777,0x666,0x555,0x444,0x333,0x222,0x111,0x000 };

/************************************************************************/
/**************************** Painting Operators ************************/
/************************************************************************/

/************************************************************************/
error ps_eofill(DPSContext ctxt) {	//	- "eofill" | -

	return ERR_NOT_IMPLEMENTED;
}

/************************************************************************/
error ps_fill(DPSContext ctxt) {	//	- "fill" | -

error err;

	// if show is being called by stringwidth, don't actually draw anything !!
	if (ctxt->space.GState->NoOutput) return ERR_OK;

	// always flatten the path before stroking it (copies to PrivatePath)
	if(err=FlattenIt(ctxt)) return err;

	// now fill from the private path area
	PrivateFill(ctxt);

	// and clear the user path
	return ps_newpath(ctxt);
}

/************************************************************************/
error ps_stroke(DPSContext ctxt) {	//	- "stroke" | -
error err;

	if (ctxt->space.GState->NoOutput) return ERR_OK;
	err = StrokeIt(ctxt,FALSE);			// stroke and don't save path
	if(!err) err=ps_newpath(ctxt);		// if it worked, clear user path
	return err;
}


/************************************************************************

 error = StrokeIt(DPSContext ctxt, int savepath)

 The main engine for implementing stroke and strokepath.  Takes the
 current path and generates a new path ready for incremental filling
 or saving for strokepath.  If we are actually stroking then each line
 segment is filled as it is generated to prevent the private path from
 overflowing.  If we are performing strokepath then curves (for circular
 linejoin or linecaps) are not flattened and the path is allowed to
 grow until it overflows or the whole path has been processed.

 Since linewidth is not checked until stroke time, all co-ordinates are
 inverse transformed back into user space before applying linejoins and
 linecaps.  The resulting path is re-transformed back into device space
 ready for filling or moving back into the user path.

 The only special cases we need to deal with are 0 width lines and the
 degenerate path that only contains one point.  In the latter case, we
 only render anything if circle linecaps are specified in which case we
 render a circle at the given point.  Butt caps and projecting square
 caps are not rendered in this case (no orientation info).

************************************************************************/
error StrokeIt(DPSContext ctxt, int savepath) {

// state variables and structures for current and previous line segment
lineseg seg1,seg2,first;	// instances
lineseg *currline,*prevline;// pointers
float  P[4];				// temp line endpoints
float  offset;				// current offset along a line
float  currlen;				// current line segment length
short  thinlines;			// TRUE = doing 0 width lines
short  first_visible;		// TRUE = linecap of first segment visible

// state variables for maintaining the dash pattern across line segments
short dash_element;			// current entry in dash array
short dash_space;			// if TRUE doing dash else doing space
short dash_max;				// size of dash array (if>0 then dash is on)
short first_dash;			// element to start at for each subpath
short first_state;			// TRUE or FALSE for starting state of dash
short in_subpath;			// BOOLEAN to indicate when subpath is finished
float dashleft;				// amount left before going to next dash element
float dashstart;			// initial value for dashleft when starting a subpath
ps_obj *realdash;			// temporary for fetching to local copy from gstate
float dasharray[MAX_DASH];	// local copy of dash array

// miscellaneous pointers, flags and variables
gstate *gs;					// pointer to the current gstate
float *points;				// current x,y pair in user path
uchar *segments;			// current segment pointer (moveto,lineto,closepath)
float halfwidth;			// linewidth / 2.0
float dx,dy;				// temp x,y deltas
float *coords;				// current x,y pair in privaye path
uchar *segs;				// current segment in private path
short path_index;			// current index into user path
error err;

	// stash pointer to gstate
	gs = ctxt->space.GState;

	// make sure there is something to be stroked
	if(gs->PathIndex==gs->PathStart) return ERR_OK;

	// make sure that the inverse of the CTM is valid
	if( (err=CheckICTM(ctxt)) != ERR_OK) return err;

	// always flatten the path before stroking it (and copy back to user path)
	if( (err=ps_flattenpath(ctxt)) != ERR_OK) return err;

	// initialise dash variables if dash pattern is present in gstate
	if(dash_max=gs->DashArray.len) {
		realdash=(ps_obj *)gs->DashArray.obj.arrayval; // copy to local array
		for(dash_element=0;dash_element<dash_max;dash_element++) {
			if(OBJ_TYPE(realdash)==TYPE_REAL) dashleft=realdash->obj.realval;
			else dashleft = (float)(realdash->obj.intval);
			dasharray[dash_element] = dashleft;
			++realdash;
		}

		// now synchronise everything with initial dash offset
		dash_element=0;		// current element we are looking at
		dash_space=TRUE;	// will be drawing a dash segment
		dashleft=gs->DashOffset;
		while(dashleft >= dasharray[dash_element]) {
			dashleft -= dasharray[dash_element];	// skip this entry
			dash_space = dash_space == TRUE ? FALSE : TRUE; // flip state
			dash_element += 1;
			if(dash_element==dash_max) dash_element=0;	// cyclic array access
		}
		first_dash=dash_element;	// where we start for each subpath
		dashstart=dasharray[dash_element]-dashleft;	// amount left in first segment
		first_state=dash_space;		// what state we are in when we start
	}

	// initialise pointers into the user path and setup index variable
	path_index=gs->PathStart;
	segments = gs->PathBase+path_index;
	points = (float *)(gs->PathBase+gs->PathArgs+(path_index<<3));

	// ensure private path starts out empty
	gs->PrivPathIndex = 0;

	// initialise stashed halfwidth variable
	halfwidth = gs->LineWidth * 0.5; thinlines=FALSE;
	if(halfwidth < 0.001) thinlines=TRUE;

	// Scan through the user path and generate fat line segments with the
	// correct linejoins, linecaps and dash pattern.  We treat each subpath
	// as a separate entity because the dash pattern is started again on
	// subpath boundaries.  This also makes it easier to deal with linejoins
	// or linecaps on the first moveto.  If the savepath flag is TRUE then
	// curves (for circle linejoins or linecaps) are not flattened and FillIt
	// is not called to render the resultant paths.  Once StrokeIt has
	// finished, the resultant path will be copied back into the user path.
	// When we are rendering (savepath==FALSE) each line segment is rendered
	// separately to prevent PrivatePath from overflowing.
	while(path_index < gs->PathIndex) {	// for whole path

		// initialise the dash pattern variables if dash is active
		if(dash_max) {
			dashleft=dashstart;			// this much of current dash is left
			dash_space=first_state;		// TRUE=rendering, FALSE=space
			dash_element=first_dash;	// at this element of the array
		}
		else {
			dash_space = TRUE;			// always rendering when no dash pattern
		}

		// this flag is used when handling the end of an open subpath to
		// determine if any linecap or linejoin needs rendering.  Since
		// we don't know if path is open or closed yet we must save this
		// flag for the time when we do know (at the end).
		first_visible = dash_space;

		// initialise the "first" line record for the original moveto.
		// This could also be a closepath from a previous line segment
		// but the semantics are exactly the same; "points" is pointing
		// at the co-ordinate pair in both cases.
		first.x1 = first.x2 = *(points);	// get x1,y1 from moveto or closepath
		first.y1 = first.y2 = *(points+1);	// assume zero length line to start

		// if next segment is moveto or closepath or there is no next
		// segment then this is a degenerate case that should only render
		// a circle if circle linecaps are specified in the gstate.  Any
		// other linecaps produce no output.  We prefetch the start point
		// of the next line so that we can get length and deltas in user
		// space co-ordinates ready for applying the dash pattern and
		// any linecaps that are needed.
		if(((path_index+1)<gs->PathIndex) && (*(segments+1)!=SEG_MOVETO)) {
			first.x2 = *(points+2);
			first.y2 = *(points+3);
		}

		ArrayTransformPoint(gs->ICTM,&first.x1);	// start to user space
		ArrayTransformPoint(gs->ICTM,&first.x2);	//   end to user space
		first.dx = first.x2-first.x1;	// get user space deltas
		first.dy = first.y2-first.y1;
		first.len= sqrt(first.dx*first.dx+first.dy*first.dy);
		if(first.len) { // not a degenerate line
			first.dx /= first.len;
			first.dy /= first.len;
			// create perpendicular deltas that are half linewidth long
			// we rotate 90 degrees by assigning rdx=-dy and rdy=dx
			first.rdx = halfwidth * -first.dy;
			first.rdy = halfwidth *  first.dx;
		}

		in_subpath=TRUE;	// still in the subpath
		prevline=0;			// no previous segment yet
		currline=&first;	// first segment is the current one

		// now apply the dash to the current sub-path and render as we go
		while(in_subpath) { // for each subpath
			offset = 0.0;	// reset current offset along line
			while(offset < currline->len) {		// while still in line segment
				if(dash_max) currlen=min(dashleft,currline->len - offset);
				else		 currlen=currline->len;

				if(dash_space) {	// if we are rendering
					if(dash_max) {	// compute line segment being drawn
						P[0] = currline->x1 + offset*currline->dx;
						P[1] = currline->y1 + offset*currline->dy;
						P[2] = P[0] + currlen*currline->dx;
						P[3] = P[1] + currlen*currline->dy;
					}
					else {	// we're drawing the whole line in one go
						P[0] = currline->x1;
						P[1] = currline->y1;
						P[2] = currline->x2;
						P[3] = currline->y2;
					}

					if(thinlines) {	// zero width lines, no joins needed
						if((gs->PrivPathIndex+2) >= gs->PathArgs) {
							if(!savepath) PrivateFill(ctxt);
							else return ERR_limitcheck;
						}
						segs = gs->PrivatePath+gs->PrivPathIndex;
						coords = (float *)(gs->PrivatePath+gs->PathArgs+(gs->PrivPathIndex<<3));
						gs->PrivPathIndex+=2;

						segs[0]=SEG_MOVETO;
						segs[1]=SEG_LINETO;

						coords[0]=P[0];			// moveto
						coords[1]=P[1];
						ArrayTransformPoint(gs->CTM,coords);

						coords[2]=P[2];			// lineto
						coords[3]=P[3];
						ArrayTransformPoint(gs->CTM,coords+2);
					}
					else {	// making a fat line segment, needs joins and linecap
						// make sure there's room for 5 segments
						if((gs->PrivPathIndex+5) >= gs->PathArgs) {
							if(!savepath) PrivateFill(ctxt);
							else return ERR_limitcheck;
						}
						segs = gs->PrivatePath+gs->PrivPathIndex;
						coords = (float *)(gs->PrivatePath+gs->PathArgs+(gs->PrivPathIndex<<3));
						gs->PrivPathIndex+=5;

						segs[0]=SEG_MOVETO;
						segs[1]=SEG_LINETO;
						segs[2]=SEG_LINETO;
						segs[3]=SEG_LINETO;
						segs[4]=SEG_CLOSE;
						
						dx=currline->rdx; dy=currline->rdy;

						coords[0]=P[0]+dx;			// moveto
						coords[1]=P[1]+dy;
						ArrayTransformPoint(gs->CTM,coords);

						coords[2]=P[2]+dx;			// lineto
						coords[3]=P[3]+dy;
						ArrayTransformPoint(gs->CTM,coords+2);

						coords[4]=P[2]-dx;			// lineto
						coords[5]=P[3]-dy;
						ArrayTransformPoint(gs->CTM,coords+4);

						coords[6]=P[0]-currline->rdx;
						coords[7]=P[1]-currline->rdy;
						ArrayTransformPoint(gs->CTM,coords+6);	// lineto

						coords[8]=coords[0];					// close
						coords[9]=coords[1];

						// see if we can do join to previous segment.  We don't
						// care if the previous segment was invisible (in space
						// part of dash) we do the join anyway.
						if( (prevline) && (offset == 0.0) ) {
							err = DoJoin(ctxt,prevline,currline,P,savepath);
						}

						// see if we should do linecaps on the dash segment
						if(dash_max) {	// if doing dash pattern
							if(dashleft==dasharray[dash_element]) { // beginning
								if(!savepath) PrivateFill(ctxt);
								err |= DoFirstCap(ctxt,currline,P,savepath);
							}
							if(dashleft==currlen) { // end
								if(!savepath) PrivateFill(ctxt);
								err |= DoLastCap(ctxt,currline,P,savepath);
							}
						}
					}
					if(err) return err;	// always limitcheck error
				}

				offset += currlen;			// update offset
				if(dash_max) {	// update current dash phase
					dashleft -= currlen;	// used this much of dash pattern
					if(dashleft <= 0.0) {
						dash_space=(dash_space==TRUE) ? FALSE : TRUE; // flip state
						dash_element += 1;
						if(dash_element==dash_max) dash_element=0;	// cyclic array access
						dashleft=dasharray[dash_element];
					}
				}
			}

			// Finished the current line segment, time to move to the next.
			// Little trick below; currline is initially set to &firstline
			// so prevline will get pointed there after we finish the first
			// line segment.  Subsequent times through this code will switch
			// currline between &seg1 and &seg2 so that firstline remains valid
			// when we drop out of the subpath loop.  We need firstline to
			// handle closepath segments at the end (linejoin or linecap)

			if(!savepath) PrivateFill(ctxt);

			prevline=currline;
			currline = (prevline == &seg1) ? &seg2 : &seg1;
			path_index+=1;				// processing next segment
			points += 2;				// go to next coordinate pair
			segments += 1;				// and bump the segment pointer

			// see if we have finished the current subpath
			if((path_index==gs->PathIndex) || (*segments==SEG_MOVETO)) {
				in_subpath=FALSE;	// finished the current subpath
			}

			// if not, then initialise currline for the next line segment
			else {
				currline->x1 = currline->x2 = *(points);
				currline->y1 = currline->y2 = *(points+1);
				if(((path_index+1)<gs->PathIndex) && (*(segments+1)!=SEG_MOVETO)) {
					currline->x2 = *(points+2);
					currline->y2 = *(points+3);
				}
				ArrayTransformPoint(gs->ICTM,&(currline->x1));
				ArrayTransformPoint(gs->ICTM,&(currline->x2));
				currline->dx = currline->x2-currline->x1;
				currline->dy = currline->y2-currline->y1;
				currline->len= sqrt(currline->dx*currline->dx+currline->dy*currline->dy);
				if(currline->len) { // not a degenerate line
					currline->dx /= currline->len;
					currline->dy /= currline->len;
					// create perpendicular deltas that are half linewidth long
					// we rotate 90 degrees by assigning rdx=-dy and rdy=dx
					currline->rdx = halfwidth * -currline->dy;
					currline->rdy = halfwidth *  currline->dx;
				}
			}
		}
		// We finished the current subpath so now we have to deal with the
		// ending condition.  We have two choices; do a linejoin between
		// the current line and the first or put linecaps on the beginning
		// of the first line and the end of the current one.  In the case
		// where there is no current line (there was only an initial moveto)
		// then we just do a linecap on the first line only if circle line
		// caps are called for (no orientation for butt or square caps).
		if(prevline==&first) {	// there was only one segment
			if(gs->LineCap == 2) {	// only render if circle linecaps
				DoFirstCap(ctxt,currline,P,savepath); // P is not valid, but not used for circles
			}
		}
		else { // more than one segment, deal with join or caps
			if( *(segments-1) == SEG_CLOSE ) {
				err = DoJoin(ctxt,currline,&first,P,savepath);
			}
			else {	// open path, cap the beginning and end
				DoFirstCap(ctxt,&first,P,savepath);
				DoLastCap(ctxt,prevline,P,savepath);
			}
		}
	}
	// we have processed the whole path.  StrokePath will copy back into
	// the user path if we were saving the outputs.
	if(!savepath) PrivateFill(ctxt);
	return(ps_newpath(ctxt));
}

/************************************************************************/
void PrivateFill(DPSContext ctxt) {

gstate *gs = ctxt->space.GState;	// stash current gstate

uchar  *p;
float  *args;
short ended = TRUE;

	if(gs->PrivPathIndex) {
		if(ctxt->rp == 0) ps_klugeon(ctxt);

		p=gs->PrivatePath;	// pointer to the internal path
		args=(float *)(p+gs->PathArgs);

		SetAPen(ctxt->rp,15-(short)(ctxt->space.GState->Greyness * 15.0));

		while(p < (gs->PrivatePath+gs->PrivPathIndex)) {
			switch( *p++ ) {
				case SEG_MOVETO:		// doing a moveto operation
					AreaMove(ctxt->rp,(short)args[0],(short)args[1]);
					ended = FALSE;
					args += 2;
					break;

				case SEG_LINETO:
					AreaDraw(ctxt->rp,(short)args[0],(short)args[1]);
					ended = FALSE;
					args += 2;
					break;

				case SEG_CLOSE:
					AreaDraw(ctxt->rp,(short)args[0],(short)args[1]);
//					AreaEnd(ctxt->rp);	// AreaXxxx does an EOFill!!!
//					AreaMove(ctxt->rp,(short)args[0],(short)args[1]);
					ended = FALSE;
					args += 2;
					break;

				default:
					fprintf(OP,"bad op in path during stroke\n");
			}
		}
		if(!ended) AreaEnd(ctxt->rp);
		gs->PrivPathIndex=0;	// newpath on the internal private path
	}
}

/************************************************************************/
error DoJoin(DPSContext ctxt, lineseg *seg1, lineseg *seg2, float *P,int savepath) {

gstate *gs = ctxt->space.GState;	// stash current gstate

float den,mlen;
float dx1,dy1;		// coordinate of corner on first line (for join)
float dx2,dy2;		// coordinate of corner on second line (for join)
float mx,my;		// used to compute mitre join point
uchar *segs;		// current segment pointer (moveto,lineto,closepath)
float *coords;		// current co-ordinate pair
short mitre;		// bool, mitres enabled

	if(gs->LineJoin == 1)	{	// doing round linejoins
		return(DoFirstCap(ctxt,seg2,P,savepath));
	}

	else {	// mitred or bevelled joins
		mitre=FALSE;	// assume bevelled joins
		den = seg1->dx*seg2->dy-seg1->dy*seg2->dx;	// sin(angle)

		if(gs->LineJoin == 0) {	// doing a mitre join
			mitre = TRUE;
			mx = seg1->dx+seg2->dx;	my=seg1->dy+seg2->dy;
			mlen = sqrt(mx*mx+my*my);	// vector in direction of mitre join
			if(mlen) {	// we do have a mitre join, make unit vectors
				mx/=mlen; my/=mlen;
				// distance from linejoin to point of mitre is
				// calculated as (0.5*linewidth)/sin(angle)
				// we use seg1->dy directly as the sign of the angle
			}
		}
		dx1=seg1->rdx; dy1=seg1->rdy;	// assume joining to "top" of segment
		dx2=seg2->rdx; dy2=seg2->rdy;	// assume joining to "top" of segment
		if(den > 0.0) { 
			dx1=-dx1; dy1=-dy1; dx2=-dx2; dy2=-dy2;
			if(mitre) { mx = -mx; my = -my; }
		}

		dx1+=seg1->x2; dy1+=seg1->y2;
		dx2+=seg2->x1; dy2+=seg2->y1;	// actually the same co-ordinates being added

		if((gs->PrivPathIndex+4) >= gs->PathArgs) return ERR_limitcheck;

		segs = gs->PrivatePath+gs->PrivPathIndex;
		coords = (float *)(gs->PrivatePath+gs->PathArgs+(gs->PrivPathIndex<<3));
		gs->PrivPathIndex+=4;

		segs[0]=SEG_MOVETO;
		segs[1]=SEG_LINETO;
		segs[2]=SEG_LINETO;
		segs[3]=SEG_CLOSE;

		coords[0] = seg1->x2;
		coords[1] = seg1->y2;
		ArrayTransformPoint(gs->CTM,coords);
		coords[6] = coords[0];	// closepath is same co-ordinates
		coords[7] = coords[1];

		coords[2] = dx1;
		coords[3] = dy1;
		ArrayTransformPoint(gs->CTM,coords+2);

		coords[4] = dx2;
		coords[5] = dy2;
		ArrayTransformPoint(gs->CTM,coords+4);
	}

	return ERR_OK;
}

/************************************************************************/
error DoFirstCap(DPSContext ctxt, lineseg *seg1, float *P, int savepath) {

gstate *gs = ctxt->space.GState;	// stash current gstate
error err = ERR_OK;

float x,y,r;

	if(gs->LineCap == 1) {
		x = P[0];
		y = P[1];
		r = gs->LineWidth/2.0;
		fixer(x,y);
		err = DoFullCircle(ctxt,x,y,r,savepath);
	}
	return err;
}

/************************************************************************/
error DoLastCap(DPSContext ctxt, lineseg *seg1, float *P, int savepath) {

gstate *gs = ctxt->space.GState;	// stash current gstate
error err = ERR_OK;

float x,y,r;

	if(gs->LineCap == 1) {
		x = P[2];
		y = P[3];
		r = gs->LineWidth/2.0;
		fixer(x,y);
		err=DoFullCircle(ctxt,x,y,r,savepath);
	}
	return err;
}

/************************************************************************
  Special routine for doing circle linecaps and linejoins.  Since we're
  always doing a full circle we do it using only two bezier curve segs.
  Arc would use four which is pretty inefficient for a dashed line with
  round endcaps.  We use a magic ratio of 1.33 : 1 for the height and
  width of the control points bounding box, this yields a circle rather
  than an ellipse.
************************************************************************/
error DoFullCircle(DPSContext ctxt, float x, float y, float r, int savepath) {

gstate *gs = ctxt->space.GState;	// stash current gstate

float points[14];
float *tmp;

uchar *segs;
float *coords;

short i;
error err;


	// generate first set of control points.
	points[0] = x-r; points[1] = y;
	points[2] = points[0]; points[3] = y + r*1.33;
	points[4] = x+r; points[5] = points[3];
	// this next point is shared with the next curve segment
	points[6] = points[4]; points[7] = points[1];

	// now generate the second set of control points
	points[8] = points[4]; points[9] = y - r*1.33;
	points[10] = points[0]; points[11] = points[9];
	points[12] = points[0]; points[13] = points[1];

	// transform control points back into device space
	tmp = points;
	for(i=0; i<7; i++) {
		ArrayTransformPoint(gs->CTM,tmp);
		tmp += 2;
	}

	segs = gs->PrivatePath+gs->PrivPathIndex;
	coords = (float *)(gs->PrivatePath+gs->PathArgs+(gs->PrivPathIndex<<3));
	if( (gs->PathArgs-gs->PrivPathIndex) < 1) return ERR_limitcheck;

	// do an initial moveto
	*segs = SEG_MOVETO;
	*coords = points[0];
	*(coords+1) = points[1];
	gs->PrivPathIndex += 1;
	segs += 1;
	coords += 2;

	if(savepath) {	// don't flatten beziers if we're saving the path
		if( (gs->PathArgs-gs->PrivPathIndex) < 6) return ERR_limitcheck;
		tmp = points+2;
		for(i=0; i<12; i++) *coords++ = *tmp++;
		*segs = SEG_CURVETO;
		*(segs+1) = SEG_CTRL_PT2;
		*(segs+2) = SEG_CTRL_PT3;
		*(segs+3) = SEG_CURVETO;
		*(segs+4) = SEG_CTRL_PT2;
		*(segs+5) = SEG_CTRL_PT3;

		gs->PrivPathIndex += 6;
	}
	else {	// flatten the two curves into the private path area

		if(err=Bezier(ctxt,points[0],points[1],points+2)) return err;
		if(err=Bezier(ctxt,points[6],points[7],points+8)) return err;
		PrivateFill(ctxt);
	}
	return ERR_OK;
}

/**************** HORRENDOUS KLUGE !!!!! **********************/
struct AreaInfo areainfo;
struct TmpRas tmpras;
short *areabuffer;
PLANEPTR planeptr;

/************************************************************************/
error ps_klugeon(DPSContext ctxt) {	//	- "klugeon" | -

	if(ctxt->rp) return ERR_OK;	// screen already opened
	if(IntuitionBase = OpenLibrary("intuition.library",0)) {
		if(GfxBase = OpenLibrary("graphics.library",0)) {
			if(ctxt->screen = OpenScreen(&ns)) {
				mnw.Screen = ctxt->screen;
				if(ctxt->window = OpenWindow(&mnw)) {
				    ctxt->rp = (ctxt->window->RPort);
				    LoadRGB4(&(ctxt->screen->ViewPort),__c,16);
					SetRast(ctxt->rp,0);
					SetAPen(ctxt->rp,1);

					areabuffer=(short *)AllocMem(5*MAXPATH,MEMF_PUBLIC|MEMF_CLEAR);
					InitArea(&areainfo,areabuffer,MAXPATH);
					ctxt->rp->AreaInfo=&areainfo;
					planeptr=AllocRaster(640,400);
					InitTmpRas(&tmpras,planeptr,RASSIZE(640,400));
					ctxt->rp->TmpRas=&tmpras;

					return ERR_OK;
				}
				CloseScreen(ctxt->screen);
			}
			CloseLibrary(GfxBase);
		}
		CloseLibrary(IntuitionBase);
	}				
	return ERR_VMerror;
}

/************************************************************************/
error ps_klugeoff(DPSContext ctxt) {	//	- "klugeoff" | -

	if(ctxt->rp) {
		FreeMem((char *)areabuffer,5*MAXPATH);
		FreeRaster(planeptr,640,400);
		CloseWindow(ctxt->window);
		CloseScreen(ctxt->screen);
		CloseLibrary(GfxBase);
		CloseLibrary(IntuitionBase);
		ctxt->rp = 0;
	}
	return ERR_OK;
}

/************************************************************************/
error ps_clearscreen(DPSContext ctxt) {	//	- "clearscreen" | -

	if(ctxt->rp) {
		SetRast(ctxt->rp,0);
	}
	return ERR_OK;
}

