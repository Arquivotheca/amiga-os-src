
#ifndef REQSUPP_H
#define REQSUPP_H


/* **************************************************************************
 *
 * Include File for the Requester Support Mechanism
 *     from Book 1 of the Amiga Programmers' Suite by RJ Mical
 *
 * Copyright (C) 1986, 1987, Robert J. Mical
 * All Rights Reserved.
 *
 * Created for Amiga developers.
 * Any or all of this code can be used in any program as long as this
 * entire notice is retained, ok?  Thanks.
 *
 * HISTORY      NAME            DESCRIPTION
 * -----------  --------------  --------------------------------------------
 * 12 Aug 86    RJ >:-{)*       Prepare (clean house) for release
 * 3 May 86     =RJ Mical=      Fix prop gadget for both 1.1 and 1.2
 * 1 Feb 86     =RJ Mical=      Created this file.
 *
 * *********************************************************************** */



/* === ReqSupport Structure ============================================== */
struct ReqSupport
	{
	struct Requester Requester;  /* Yes, a complete Requester structure! */

	struct Window *Window;       /* Window that will contain this requester */

	/* These are the special handler routines.  You can add any to these
	 * that you find appropriate if you plan on customizing DoRequest().
	 * You can also leave these NULL to specify that you're not interested.
	 */
	LONG (*StartRequest)();		/* Called after the requester is created */
	LONG (*GadgetHandler)();	/* Called with each GADGETUP event */
	LONG (*NewDiskHandler)();	/* Called with each DISKINSERTED event */
	LONG (*KeyHandler)();		/* Called with each VANILLAKEY event */
	LONG (*MouseMoveHandler)(); /* Called when the mouse is seen to move */

	/* When DoRequest() returns, the GadgetID of the last selected gadget
	 * can be found here.
	 */
	SHORT SelectedGadgetID;
	};



#endif /* of REQSUPP_H */


