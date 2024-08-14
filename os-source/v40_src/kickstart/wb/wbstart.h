/*
 * $Id: wbstart.h,v 38.1 91/06/24 11:39:31 mks Exp $
 *
 * $Log:	wbstart.h,v $
 * Revision 38.1  91/06/24  11:39:31  mks
 * Initial V38 tree checkin - Log file stripped
 * 
 */

#ifndef	WBSTART_H
#define	WBSTART_H

/*
 * These bits are from the bitfield that is used to start workbench.
 * Old LoadWB commands only set WBARGF_DEBUGON  New LoadWB may set
 * more bits.
 */

/* wb_Argument bits */
#define	WBARGB_DEBUGON	0		/* Turn on debugging */
#define	WBARGB_DELAYON	1		/* Delay start until ready... */
#define	WBARGB_CLEANUP	2		/* Auto CleanUp on initial open */
#define	WBARGB_NEWPATH	3		/* Install new path */

#define	WBARGF_DEBUGON	(1L << WBARGB_DEBUGON)
#define	WBARGF_DELAYON	(1L << WBARGB_DELAYON)
#define	WBARGF_CLEANUP	(1L << WBARGB_CLEANUP)
#define	WBARGF_NEWPATH	(1L << WBARGB_NEWPATH)

#endif	/* WBSTART_H */
