/* mymodel.c -- :ts=8
 * Attribute constant definitions for "mymodelclass".
 */

/*
Copyright (c) 1989, 1990 Commodore-Amiga, Inc.

Executables based on this information may be used in software
for Commodore Amiga computers. All other rights reserved.
This information is provided "as is"; no warranties are made.
All use is at your own risk, and no liability or responsibility
is assumed.
*/

/* these are "private" attribute values that won't collide
 * with system values or other private values used in
 * different contexts.
 */
#define MYMODA_CURRVAL	(TAG_USER + 1)	/* the current unsigned value	*/
#define MYMODA_RANGE	(TAG_USER + 2)	/* currval <= (range-1)		*/

/* these fake "strobe" attributes are used by things like "arrow gadgets" */
#define MYMODA_INCRSTROBE	(TAG_USER + 3)
#define MYMODA_DECRSTROBE	(TAG_USER + 4)
