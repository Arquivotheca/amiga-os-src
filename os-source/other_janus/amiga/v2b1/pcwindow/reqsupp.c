
/* *** reqsupp.c ************************************************************
 *
 * Requester Support Routine
 *     from Book 1 of the Amiga Programmers' Suite by RJ Mical
 *
 * Copyright (C) 1986, 1987, Robert J. Mical
 * All Rights Reserved.
 *
 * Created for Amiga developers.
 * Any or all of this code can be used in any program as long as this
 * entire copyright notice is retained, ok?
 *
 * HISTORY      NAME            DESCRIPTION
 * -----------  --------------  --------------------------------------------
 * 4 Feb 87     RJ              Real release
 * 12 Aug 86    RJ >:-{)*       Prepare (clean house) for release
 * 3 May 86     =RJ Mical=      Fix prop gadget for both 1.1 and 1.2
 * 1 Feb 86     =RJ Mical=      Created this file.
 *
 * *********************************************************************** */


#include <prosuite/prosuite.h>
#include <prosuite/reqsupp.h>


struct IntuiMessage *GetMsg();



/* *** DoRequest() **********************************************************
 * 
 * NAME
 *     DoRequest  --  Creates and manages a requester
 * 
 * 
 * SYNOPSIS
 *     DoRequest(ReqSupport);
 * 
 * 
 * FUNCTION
 *     Creates a requester according to the specifications laid out
 *     by you in a ReqSupport structure, and manages the interaction
 *     with the requester for you.  In the end this routine returns control
 *     to you with an identifier describing which gadget the user selected
 *     to terminate the requester; this identifier can be found in the
 *     SelectedGadgetID field of your ReqSupport structure.
 * 
 *     Note that if anything goes wrong while trying to create the 
 *     requester (usually out of memory) this routine returns 
 *     immediately with the SelectedGadgetID field set to zero.
 *     Because of this, you should either avoid GadgetIDs of zero
 *     or at least you should have your Cancel Gadget have an
 *     ID of zero.
 * 
 *     You can specify routines that will be called when certain events
 *     occur while the requester is displayed.  For instance, you can
 *     specify that a particular routine be called every time the
 *     user selects any of the requester gadgets.  See the documentation
 *     and the ReqSupport structure for details about what
 *     routine vectors you can supply.
 * 
 * 
 * INPUTS
 *     ReqSupport = pointer to a ReqSupport structure
 * 
 * 
 * RESULT
 *     Returns the identifier of the gadget that ended the requester
 *     in the ReqSupport's SelectedGadgetID field.
 *     If anything goes wrong (usually out of memory) the SelectedGadgetID
 *     field is set to zero.
 */
DoRequest(reqsupp)
struct ReqSupport *reqsupp;
{
	ULONG class;
	SHORT x, y;
	struct IntuiMessage *message;
	struct Gadget *gadget;
	ULONG saveidcmp;
	BOOL IAintGotNoSatisfaction, mousemoved;
	struct Window *window;
	LONG seconds, micros;

	window = reqsupp->Window;

	saveidcmp = window->IDCMPFlags;
	ModifyIDCMP(window, 
			GADGETUP | GADGETDOWN | REQSET | REQCLEAR
			| MOUSEMOVE | DISKINSERTED);

	if (Request(&reqsupp->Requester, window) == FALSE)
		{
		reqsupp->SelectedGadgetID = 0;
		goto JUMP_SHIP;
		}

	IAintGotNoSatisfaction = TRUE;

	while (IAintGotNoSatisfaction)
		{
		Wait(1 << window->UserPort->mp_SigBit);

		mousemoved = FALSE;

		while (message = GetMsg(window->UserPort))
			{
			gadget = (struct Gadget *)message->IAddress;
			class = message->Class;
			x = message->MouseX;
			y = message->MouseY;
			seconds = message->Seconds;
			micros = message->Micros;
			ReplyMsg(message);

			if (IAintGotNoSatisfaction) switch (class)
				{
				case REQSET:
					/* Does the caller have some startup 
					 * stuff to perform now that the
					 * requester has been opened?
					 */
					if (reqsupp->StartRequest)
						(*reqsupp->StartRequest)();
					break;
				case DISKINSERTED:
					if (reqsupp->NewDiskHandler) (*reqsupp->NewDiskHandler)();
					break;
				case MOUSEMOVE:
					mousemoved = TRUE;
					break;
				case GADGETDOWN:
				case GADGETUP:
					reqsupp->SelectedGadgetID = gadget->GadgetID;
					if (reqsupp->GadgetHandler)
						{
						if ((*reqsupp->GadgetHandler)(gadget, 
								x, y, seconds, micros))
							{
							EndRequest(&reqsupp->Requester, window);
							IAintGotNoSatisfaction = FALSE;
							}
						}
					break;
				case REQCLEAR:
					IAintGotNoSatisfaction = FALSE;
					break;
				}
			}

		if (mousemoved && reqsupp->MouseMoveHandler && IAintGotNoSatisfaction)
			(*reqsupp->MouseMoveHandler)();
		}

JUMP_SHIP:
	ModifyIDCMP(window, saveidcmp);
}


