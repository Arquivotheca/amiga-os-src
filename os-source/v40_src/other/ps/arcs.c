#include <exec/types.h>
#include <intuition/intuition.h>
#include <exec/memory.h>
#include <proto/intuition.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/graphics.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <m68881.h>


/* ======================================================================== */


#define PI_180	(PI/180.0)

#define MAGIC_NUMBER	0.394

extern struct RastPort *rp;

struct Point2d {
    double x,y;
};
typedef struct Point2d P2d;


extern P2d P[4];

/* ======================================================================== */
/* ======== Protos ======================================================== */
/* ======================================================================== */

void arcto(double ,double ,double ,double ,double, double,double ,double);

extern void DoFatLine(struct RastPort *rp, double x1, double y1, double x2, double y2);

extern void Bezier(double);

void arc(double xc,double yc,double start,double end,double r,double error,int flag);

extern void sincosr(double,double *,double *,double );


//---------------------------------------------------------------------------
//
//   This file contains Routines for arc,arn and arcto.
//
//---------------------------------------------------------------------------



//-----------------------------------------------------------------------------
//  arcto(x0, y0, x1, y1, x2, y2, r, flattness) 
//
// This routine calculates the arc subtended by the two tangent lines from
// x1->x0 & x1->x2. It initially creates unitvectors in those directions and
// then calculates the intersection of the two lines which have been 
// moved perpendicular to the tangent. The point of intersection is the 
// centre of the circle common to both tangent vectors. A rotation is
// then applied to these unitvectors to get the 2 points of intersection.
// The angle corresponding to these two tangent points are then calculated
// (using the arc tangent function). After that, we have start/end angles,
// a radius and the centre of the arc so we can give all that stuff
// to the arc routine.
//
// Incidently, the arc() function handles both +ve and -ve arcs.
//
//-----------------------------------------------------------------------------

void arcto(double x0,double y0,double x1,double y1,double x2,double y2,double r,double error) {
	double dx0,dy0,dx2,dy2;	// Unitvectors
	double len,xt0,yt0,xt2,yt2,xc,yc;
	double a1,b1,c1,a2,b2,c2,denom,x3,y3;
	double sx,sy,start,end,dx,dy,dist,dist2;


// Current point is remembered here 
	sx = x0; sy = y0;

// Unitise vectors P1->P0 and P1->P2

	dx0 = x0 - x1; dy0 = y0 - y1;
	dx2 = x2 - x1; dy2 = y2 - y1;

	len = sqrt((dx0*dx0 + dy0*dy0));
	dx0 /= len; dy0 /= len;

	len = sqrt((dx2*dx2 + dy2*dy2));
	dx2 /= len; dy2 /= len;

// calc the correct direction vectors 

	dist = (x2-x0)*(x2-x0) + (y2-y0)*(y2-y0);
	
	dist2 = (x2-x0+ -dy0)*(x2-x0+ -dy0) + (y2-y0+dx0)*(y2-y0+dx0);

	if(dist2<dist) {
		dx0 = -dx0;
		dy0 = -dy0;
	}
	
	dist2 = (x2-x0+ -dy2)*(x2-x0+ -dy2) + (y2-y0+dx2)*(y2-y0+dx2);

	if(dist2<dist) {
		dx2 = -dx2;
		dy2 = -dy2;
	}

// At this point we have 3 Points P0, P1 and P2
// However I need 4 Points (2 for each line segment), so
// lets introduce another point (x3,y3) and shift the others
// up. 

	x3 = x2 + dy2*r; y3 = y2 + -dx2*r;
	x2 = x1 + dy2*r; y2 = y1 + -dx2*r;

	x0 += -dy0*r; y0 += dx0*r;
	x1 += -dy0*r; y1 += dx0*r;

// We now have line segments (x0,y0)->(x1,y1) & (x2,y2)->(x3,y3)

// Convert both lines to General form, ie  Ax + By + C = 0;

	a1 = (y1 - y0);
	b1 = -(x1 - x0);
	c1 = x1*y0 - x0*y1;

	a2 = (y3 - y2);
	b2 = -(x3 - x2);
	c2 = x3*y2 - x2*y3;
	
// If the denominator of the equation is less than some tolerance (1.0E-4),
// then this means the lines are Collinear and we simply add a line
// segment from X0,Y0 to X1,Y1.

	denom = b2*a1 - b1*a2;
	
	if(abs(denom)<1.0e-4) {
		printf("Lines are parallel \n");
		return ;
	}

// Ok,lines are not Collinear so calculate the intersection of the lines 
// (xc,yc)....

	xc = (b1*c2 - b2*c1) / denom;
	yc = (c1*a2 - c2*a1) / denom;

// and then find the two tangent points.

	xt0 = dy0*r + xc;
	yt0 = -dx0*r + yc;

	xt2 = -dy2*r + xc;
	yt2 = dx2*r + yc;

// Now lets calculate the 2 angles at the tangent points
// (NB) PID2 from math.h

// If tangent point is at +/-90 degree
	if(xc == xt0) {
		if(yt0>yc) {
			start = PID2;
		} else {
			start = -PID2;
		}
	} else {
		dx =  xt0 - xc;
		dy =  yt0 - yc;
		start = atan((dy/dx));
	}
// Adjust angle if outside +PI/2 or -PI/2
//(NB) the arc tangent function only returns values in the range +/- PI/2

	if(xt0>xc && yt0<yc) {
		start += 2*PI;
	} else if(xt0<xc) {
		start += PI;
	}

// Convert radians -> degrees
	start *= 180.0/PI;

// As above but this time it's for the second tangent point

	if(xc == xt2) {
		if(yt2>yc) {
			end = PID2;
		} else {
			end = -PID2;
		}
	} else {
		dx =  xt2 - xc;
		dy =  yt2 - yc;
		end = atan((dy/dx));
	}
	if(xt2>xc && yt2<yc) {
		end += 2*PI;
	} else if(xt2<xc) {
		end += PI;
	}

// Convert radians -> degrees
	end *= 180.0/PI;

// If we are in a negative quadrant convert this to 0..+360 Deg
	if(start<0.0) start +=360.0;
	if(end<0.0) end +=360.0;

//-----------
//
// At this point you should do a Line segment from X0,Y0 to xt0,yt0
// (NB) you can use sx,sy as your starting point
//
//------------

// AddPathSegment(sx,sy,xt0,yt0);


// Now ask arc() to do the rest

	if(start<end) {
		dist = end - start;
		if(dist<180.0) {
			arc(xc,yc,start,end,r,error,0);		// arc
		} else {
			arc(xc,yc,start,end,r,error,1);		// arcn
		}
	} else {
		dist = start - end;
		if(dist<180.0) {
			arc(xc,yc,start,end,r,error,1);		// arcn
		} else {
			arc(xc,yc,start,end,r,error,0);		// arc
		}
	}
}

//-----------------------------------------------------------------------------
//
//     arc(xc,yc,start_angle,end_angle,radius,flattness,flag);
//
// This routine uses bezier curve segments to approximate an arc.
// Some simple trig is used to find the chord length and subsequent
// tangent vectors. A magic number is used to determine the magnitude
// of the tangent vectors with respect to the chord length.
// This code is used for clockwise and anti-clockwise arcs. The direction
// of the arc is determined by the flag value
//             0 - arc
//             1 - arcn
//-----------------------------------------------------------------------------
// (!! see below)If the arc is clockwise (arcn) then we negate the chord_length.
// Eh, I hear you say? If we assume the control points are listed in an
// anti-clockwise direction then we rotate the radius vectors 90 deg to get
// the tangent vectors. These are used for the bezier segment.
// P1 is radius Vector rotated +90, and P2 is the same except angle is -90 deg.
// Now if we give the control points in a clockwise direction and use the above
// formula, the tangent vectors end up facing away from each other. Therefore
// by negating the chord_length the multiplication below reverses the 
// tangent vectors thus it elegantly 'kills two birds with one stone'.

void arc(double xc,double yc,double start,double end,double r,double error,int flag) {
	double chord_length,tx,ty,step,diff;
	double theta,elapsed=0.0,inc90;
	double c,s;
	int loop=4;

//  Gap between angles is determined according to direction.

// Convert all angles to RADIANS

	start *= PI_180; end *= PI_180;
	if(!flag) {
		diff = end - start;	inc90 = PID2;
	} else {
		diff = start - end;	inc90 = -PID2;
	}

	if(diff<=0.0) diff += (PI*2.0);

// If we divide the arc into 90 degree segments it makes life easier.
// Therefore we calculate the acute angle left by this quatering

	step = fmod(diff,PID2);	
	if(step==0.0) step = PID2;
	if(flag) step = -step;

// Max 4 times ( 4 * 90 = 360)

	while(loop--) {
		sincosr(start,&c,&s,r);
		P[0].x = xc + c;
		P[0].y = yc + s;
		
		theta = fmod((start+step),(PI*2.0));
		sincosr(theta,&c,&s,r);
		P[3].x = xc + c;
		P[3].y = yc + s;

		chord_length = sqrt(((P[0].x-P[3].x)*(P[0].x-P[3].x)+
							(P[0].y-P[3].y)*(P[0].y-P[3].y)));

// (!! see above)
		if(flag) chord_length = -chord_length;

		tx = (P[0].x-xc)/r;
		ty = (P[0].y-yc)/r;

		P[1].x =  (-ty*(chord_length*MAGIC_NUMBER))+P[0].x ;
		P[1].y =  (tx*(chord_length*MAGIC_NUMBER))+P[0].y ;

		tx = (P[3].x-xc)/r;
		ty = (P[3].y-yc)/r;

		P[2].x =  (ty*(chord_length*MAGIC_NUMBER))+P[3].x ;
		P[2].y = (-tx*(chord_length*MAGIC_NUMBER))+P[3].y ;

// Instead of calling bezier() you would insert a curveto with
// current point, P1, P2 and P3 into the path

		Bezier(error);

		start = fmod((start+step),(PI*2.0)); elapsed +=step;
		if(abs(elapsed)>=diff) break;
		step = inc90;
	}
}
