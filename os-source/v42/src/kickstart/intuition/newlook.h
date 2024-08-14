/*** newlook.h **************************************************************
 *
 * newlook.h -- yep
 *
 *  $Id: newlook.h,v 40.0 94/02/15 17:33:12 davidj Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (C) 1989, Commodore-Amiga, Inc.
 *  All Rights Reserved
 *
 ****************************************************************************/

#ifndef INTUITION_NEWLOOK_H
#define INTUITION_NEWLOOK_H

#ifndef INTUITION_INTUINTERNAL_H
#include "intuinternal.h"
#endif

#define FRAMEVTHICK	1
#define FRAMEHTHICK	1

#define newlookProp( g ) ( TESTFLAG( PI(g)->Flags, PROPNEWLOOK ) )

/* right-side gadget positions: if you change
 * these, be sure to recompile windows.c and newlook.c
 */
#define ZOOMRELRIGHT	(-45)
#define DEPTHRELRIGHT	(-22)

#define LZOOMRELRIGHT	(-33)
#define LDEPTHRELRIGHT	(-16)

/* embossedBoxTrim() has a parameter which describes how to render the
 * box-joins (where the two colors meet).  The joins are in the
 * upper-right and lower-left of the box.
 *
 * JOINS_UPPER_LEFT_WINS - The color used to render the upper and left
 *	sides of the box is used in the joins.
 * JOINS_LOWER_RIGHT_WINS - The color used to render the lower and right
 *	sides of the box is used in the joins.
 * JOINS_NONE - The joins are not rendered.
 * JOINS_ANGLED - The joins are angled, in the style that GadTools uses.
 */

#define JOINS_UPPER_LEFT_WINS	0	/* used for window borders */
#define JOINS_LOWER_RIGHT_WINS	1	/* used for newlook props */
#define JOINS_NONE		2	/* used for default frameiclass frames */
#define JOINS_ANGLED		3	/* used for new style frameiclass frames */

#endif	/*  INTUITION_NEWLOOK_H */
