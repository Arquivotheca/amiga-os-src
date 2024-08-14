#ifndef UFB_RA
/**
*
* The following structure is a UNIX file block that retains information about
* a file being accessed via the level 1 I/O functions.
*/
struct UFB
{
int ufbflg;		/* flags */
long ufbfh;		/* file handle */
};
#define NUFBS 40	/* number of UFBs defined */

/*
*
* UFB.ufbflg definitions
*
*/

#define UFB_RA 1	/* reading is allowed */
#define UFB_WA 2	/* writing is allowed */
#define UFB_NC 16	/* no close */

/**
*
* External definitions
*
*/

#ifndef NARGS
extern struct UFB *chkufb(int);
#else
extern struct UFB *chkufb();
#endif
#endif
