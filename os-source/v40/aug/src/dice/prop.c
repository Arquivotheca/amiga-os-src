/* prop.c -- dropshadow gadgets	*/

/* this program copyright 1987, james mackraz.  may not be distributed
 * for profit.  copies of the source may be made for not-for-profit
 * distribution, but must include this notice.
 *
 * james mackraz, 4021 Second Street, Palo Alto, CA, 94306
 */

#include "ds.h"

extern struct Window   *window;

#define DARKID	8
#define DARKBODY 0xFFFF/16
#define TWXTBODY 0xFFFF/16

#define	GLEFT	55
#define GTOP0  	13
#define GTOP1  	(GTOP0 + GHEIGHT + -1)
#define GWIDTH	(-(GLEFT + 25))
#define GHEIGHT 7

extern struct Gadget	twxt;

struct PropInfo darkinfo = {
	AUTOKNOB | FREEHORIZ, 0xffff >> 1, 0, DARKBODY, 0
};

struct Image darkimage;

struct Gadget	darkness = {
	&twxt,  GLEFT, GTOP0, GWIDTH, GHEIGHT,
	GADGHCOMP | GADGIMAGE  | GRELWIDTH,
	GADGIMMEDIATE | FOLLOWMOUSE | RELVERIFY,
	PROPGADGET,
	(APTR) &darkimage,
	NULL,
	NULL,
	0,
	(APTR) &darkinfo,
	DARKID,
	0
};

struct PropInfo twxtinfo = {
	AUTOKNOB | FREEHORIZ, 4 << TWXTSHIFT , 0, TWXTBODY, 0
};

struct Image twxtimage;

struct Gadget	twxt = {
	NULL,  GLEFT, GTOP1, GWIDTH, GHEIGHT,
	GADGHCOMP | GADGIMAGE  | GRELWIDTH,
	RELVERIFY,
	PROPGADGET,
	(APTR) &twxtimage,
	NULL,
	NULL,
	0,
	(APTR) &twxtinfo,
	TWIXTID,
	0
};

