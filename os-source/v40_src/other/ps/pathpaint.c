/************************************************************************

  Module :	internal support for path and paint ops.

  Purpose:	support routines for op_path.c and op_paint.c
			mostly concerned with bezier curves and arcs.

  Internals: see docs.path (when I've written it, SMB)

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

#include "proto/exec.h"

#include "math.h"
#include "m68881.h"

#include "string.h"
#include "stdio.h"

//--------------------------------------------------------------------
// The initial step size
#define STEP	0.25

// Step size squared
#define STEP2	0.0625

// Step size cubed
#define	STEP3	0.015625


//----------external callable routines ---------------------------------------
error Bezier(DPSContext ctxt, float x, float y, float *P);
error CopyPrivateToPath(DPSContext);

//-----------------------------------------------------------------------------
//
// error = Bezier( context, startx, starty, float *otherthreepoints )
//
// This routine uses forward differencing to trace the locus of a curve which
// would normaly be given by the Bezier/Bernstien Basis functions.
//
// It uses the mid point of the current line segment and the mid point of the
// current curve segment to determine the flatness or smoothness of the
// approximating polygonal line.
//
// If the distance is outside the tolerance limit, then a reduction (half), in
// the forward differences is made. This continues until the local error is
// within the given tolerance at which point the line segment is advanced.
//
// If, at the current segment, the error is within bounds then a quick check
// is made to see if we can double the step size, thus reducing the amount
// of line segments needed and subsequent rendering time.
//
// The routine has the characteristic of 'zooming' into tight curves while
// at the same time being more conservative with more linear sections.
//
// Arguments to this routine comprise the first control point (x,y) and a
// pointer to an array of 6 floats comprising the other three control points.
// This looks a bit clumsy but is really suited to the way curves are
// represented in the path.  The local error is found from the Flatness
// parameter in the current GState (hanging off ctxt).
//
// As line segments are generated they are saved as lineto segments in the
// PrivatePath area.  If the area overflows then a limitcheck error is
// generated, else this routine returns ERR_OK.
//-----------------------------------------------------------------------------
error Bezier(DPSContext ctxt, float x, float y, float *P) {

float ax,bx,cx,dx; 		// Curve coef's
float ay,by,cy,dy;			//  ''    ''
float fx0,fx1,fx2,fx3;		// Forward Diffs
float fy0,fy1,fy2,fy3;

float xs,ys,xe,ye; 		// Line segment start - finish
volatile float xm,ym,cxm,cym;		// Mid Point on line & curve section

float dist2;
short loop,max_segs=1;
short segs_left;

uchar *segs;
float *coords;

float flat;
error err=ERR_OK;

gstate *gs=ctxt->space.GState;


	// initialise pointers into private path
	segs = gs->PrivatePath+gs->PrivPathIndex;
	coords = (float *)(gs->PrivatePath+gs->PathArgs+(gs->PrivPathIndex<<3));
	segs_left = gs->PathArgs-gs->PrivPathIndex;

	flat = gs->Flatness*gs->Flatness;	// always use error squared

	// Calculate coef's of the bezier curve for the current control points.

	ax = -x + 3.0*(-P[2] + P[0]) + P[4];
	ay = -y + 3.0*(-P[3] + P[1]) + P[5];
	
	bx = 3.0*(x + (-2.0*P[0]) + P[2]);
	by = 3.0*(y + (-2.0*P[1]) + P[3]);

	cx = 3.0*(-x + P[0]);
	cy = 3.0*(-y + P[1]);

	dx = x;
	dy = y;

	// Work out forward diff's from the above coef's with a step size of 0.25

	fx0 = x;
	fy0 = y;

	fx1 = ax*STEP3 + bx*STEP2 + cx*STEP;
	fy1 = ay*STEP3 + by*STEP2 + cy*STEP;

	fx2 = ax*STEP3*6.0 + bx*STEP2*2.0;
	fy2 = ay*STEP3*6.0 + by*STEP2*2.0;

	fx3 = ax*STEP3*6.0;
	fy3 = ay*STEP3*6.0;

	max_segs = 4;

	// -------- Main loop.----------------------------
	for(loop=0;loop<max_segs;loop++) {

		// LINE segment start is current point
		xs = fx0; ys = fy0;

		// LINE segment end is current + 1st forward diff
		xe = fx0 + fx1; ye = fy0 + fy1;

		// LINE segment mid-point 	
		xm = (xs+xe)*0.5; ym = (ys+ye)*0.5;

		// CURVE segment mid-point 
		cxm = fx0 + 0.5*(fx1-0.25*fx2 + 0.125*fx3);
		cym = fy0 + 0.5*(fy1-0.25*fy2 + 0.125*fy3);
		
		dist2 = (xm-cxm)*(xm-cxm) + (ym-cym)*(ym-cym);

		// The distance is greater than allowed flat so reduce step size
		if(dist2>flat) {
			// The current and max number of segments double, thus REDUCING
			// the step size by 50%
			max_segs <<=1; loop <<=1;

			// Reduce the forward diffs by 50%
			fx3 *= 0.125;
			fy3 *= 0.125;
			fx2 = fx2*0.25 - fx3;
			fy2 = fy2*0.25 - fy3;
			fx1 = (fx1 - fx2)*0.5; 
			fy1 = (fy1 - fy2)*0.5; 
			loop--; continue;
		}

		// Check to see if we are being too over cautious with our step size by
		// trying the above distance check with a step twice the current one
		else {
			cxm = fx0 + fx1;
			cym = fy0 + fy1;

			xs = (2.0*(fx0+fx1)+fx2)*0.5;
			ys = (2.0*(fy0+fy1)+fy2)*0.5;

			dist2 = (cxm-xs)*(cxm-xs) + (cym-ys)*(cym-ys);
			if((dist2<flat) && !(loop&1)) {	// don't double step on odd segments

				// OK, we can use a higher step size so reduce the current
				// and max_segs by two, thus INCREASING the step by 50%
				max_segs >>=1; loop >>=1; 

				// This is the opposite of the above if we solve the forward
				// difference equations for the old diff's and run them backwards

				fx1 = fx1*2.0 + fx2; 
				fy1 = fy1*2.0 + fy2;
	
				fx2 = (fx2 + fx3) * 4.0;
				fy2 = (fy2 + fy3) * 4.0;
	
				fx3 = fx3 * 8.0;
				fy3 = fy3 * 8.0;
				loop--; continue;
			}
		}
		fx0 += fx1; fy0 += fy1;
		fx1 += fx2; fy1 += fy2;
		fx2 += fx3;	fy2 += fy3;

		// add the next segment to the private path
		if(segs_left) {
			*segs++ = SEG_LINETO;
			*coords++ = fx0;
			*coords++ = fy0;
			++(gs->PrivPathIndex);
			--segs_left;
		}
		else return(ERR_limitcheck);
	}

//----- BUG FIX   14/06/91 ---------------------------------
// If the error tolerance is larger than the real error the above
// piece of code sometimes increases the step size so much that it falls out of
// the loop without ever drawing at least one segment.
// The following test is used to check for that occurance and if it's true
// drops in a line segment from control point 0 to control point 3
//
	if( (fx0 == x) && (fy0 == y) ) {
		if(segs_left) {
			*segs++ = SEG_LINETO;
			*coords++ = P[4];
			*coords++ = P[5];
			++(gs->PrivPathIndex);
			--segs_left;
		}
		else return(ERR_limitcheck);
	}
	return(ERR_OK);
}

/***************************************************************************

  error = CopyPrivateToPath( DPSContext ctxt )

  Appends the current private path to the current path.  There must be room
  for the copy to take place (ie. check before calling this routine).
****************************************************************************/
error CopyPrivateToPath(DPSContext ctxt) {

gstate *gs = ctxt->space.GState;

register short count,subpath,segcount;
register char *psegs,*segs,*up;
register char *pcoords,*coords;

	count=gs->PrivPathIndex;				// number of x,y pairs to copy

	psegs = gs->PrivatePath;
	segs = gs->PathBase+gs->PathIndex;

	coords  = gs->PathBase+gs->PathArgs+(gs->PathIndex<<3);
	pcoords = gs->PrivatePath+gs->PathArgs;

	CopyMem(pcoords,coords,count*8);	// copy float coordinates
	CopyMem(psegs,segs,count);			// copy segment codes

	gs->PathIndex += count;					// setup path index correctly

	// now we walk through the user path ensuring subpath is setup OK
	up=gs->PathBase+gs->PathStart;	// get pointer to start of user path
	count=gs->PathStart;

	while(count<gs->PathIndex) {		// while still in the user path
		switch( *up++ ) {
			case SEG_MOVETO:			// doing a moveto operation
				subpath=count;			// so subpath starts here
										// drop through to update pointers
			case SEG_LINETO:			// doing a lineto operation
			case SEG_CLOSE:
				count+=1;
				break;

			case SEG_CURVETO:			// doing a curveto operation
				gs->PathIsCurved = TRUE;
				count += 3; up+=2;
				break;

			default:
				printf("BUG!!!!\n");
		}
	}	
	gs->SubPath = subpath;		// set the last subpath
	return ERR_OK;
}
