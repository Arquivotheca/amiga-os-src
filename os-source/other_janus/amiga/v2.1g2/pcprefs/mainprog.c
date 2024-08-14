#include <exec/types.h>
#include <libraries/dos.h>
#include <intuition/intuition.h>
#include <janus/janus.h>

#include "mydata.h"

extern struct Gadget A000Gadget, D000Gadget, E000Gadget,
		     MonoGadget, ColorGadget, ShadowGadget;
extern struct IntuiText OffText, OnText;

void HLine(int x1, int y1, int x2)
{
	Move(PrefWindow->RPort, x1, y1);
	Draw(PrefWindow->RPort, x2, y1);
}

void VLine(int x1, int y1, int y2)
{
	Move(PrefWindow->RPort, x1, y1);
	Draw(PrefWindow->RPort, x1, y2);
}

/* =========================================================================
 * Handles the highlight draw/erase of an individual gadget.  Pen should
 * already be set to the required color ( 0 or 3 ) and a0 points to gadget
 * =========================================================================
 */
void GadgetHighlight(struct Gadget *g)
{
	/* First draw two horizontal bars above the gadgets hit box */
	HLine(g->LeftEdge - 4, g->TopEdge - 1, g->LeftEdge + g->Width + 3);
	HLine(g->LeftEdge - 4, g->TopEdge - 2, g->LeftEdge + g->Width + 3);

	/* Now draw two horizontal bars below the gadgets hit box */
	HLine(g->LeftEdge - 4, g->TopEdge + g->Height, g->LeftEdge + g->Width + 3);
	HLine(g->LeftEdge - 4, g->TopEdge + g->Height + 1, g->LeftEdge + g->Width + 3);

	/* Draw four vertical bars against the left edge of the gadget */
	VLine(g->LeftEdge - 4, g->TopEdge, g->TopEdge + g->Height - 1);
	VLine(g->LeftEdge - 3, g->TopEdge, g->TopEdge + g->Height - 1);
	VLine(g->LeftEdge - 2, g->TopEdge, g->TopEdge + g->Height - 1);
	VLine(g->LeftEdge - 1, g->TopEdge, g->TopEdge + g->Height - 1);

	/* Draw four vertical bars against the right edge of the gadget */
	VLine(g->LeftEdge + g->Width + 3, g->TopEdge, g->TopEdge + g->Height - 1);
	VLine(g->LeftEdge + g->Width + 2, g->TopEdge, g->TopEdge + g->Height - 1);
	VLine(g->LeftEdge + g->Width + 1, g->TopEdge, g->TopEdge + g->Height - 1);
	VLine(g->LeftEdge + g->Width + 0, g->TopEdge, g->TopEdge + g->Height - 1);
}

/* Refreshes the gadgets that have on or off printed into them depending on
 * thier current state.  Uses intuitext to render into the gadget box
 */
void RefreshSwitches()
{
struct Gadget *g;

	for (g = &MonoGadget; g; g = g->NextGadget) {
		if (g->Flags & SELECTED)
			PrintIText(PrefWindow->RPort, &OnText,
				   g->LeftEdge, g->TopEdge);
		else
			PrintIText(PrefWindow->RPort, &OffText,
				   g->LeftEdge, g->TopEdge);
	}
}

/* Handles gadgetup events from the boolean switches and the exit switches */
void HandleGadgetUp(struct IntuiMessage *msg)
{
struct Gadget *g;

	g = (struct Gadget *)msg->IAddress;

	if (g->GadgetID <= 2) {
		FinishedFlag = 0;
		FinishCode = g->GadgetID;
	} else {
#if 0
		SerialState = 0;
#endif
		MonoState = 0;
		ColorState = 0;
		ShadowState = 0;

		if (MonoGadget.Flags & SELECTED)
			MonoState = 1;

		if (ColorGadget.Flags & SELECTED)
			ColorState = 1;

		if (ShadowGadget.Flags & SELECTED)
			ShadowState = 1;

#if 0
		if (SerialGadget.Flags & SELECTED)
			SerialState = 1;
#endif

		RefreshSwitches();
	}
}

void RefreshGads()
{
struct Gadget *g, *s;
int i;

	g = &A000Gadget;
	s = 0;

	for (i = 0; i < 3; i++) {
		if (g->Flags & SELECTED) {
			s = g;
		} else {
			SetAPen(PrefWindow->RPort, 0);
			GadgetHighlight(g);
		}
		g = g->NextGadget;
	}

	if (s) {
		SetAPen(PrefWindow->RPort, 3);
		GadgetHighlight(s);
	}
}

/* =========================================================================
 *  Handle Gadget down events (ram bank gadgets only)
 * =========================================================================
 */

void HandleGadgetDown(struct IntuiMessage *msg)
{
struct Gadget *g;
int i;

	/*  */
	g = &A000Gadget;

	for (i = 0; i < 3; i++) {

		if (g == (struct Gadget *)msg->IAddress) {
			/* select it */
			g->Flags |= SELECTED;

			/* save state */
			RamBank = g->GadgetID;
		} else {
			/* unselect it */
			g->Flags &= ~SELECTED;
		}

		g = g->NextGadget;
	}

	RefreshGads();
}

/* =========================================================================
 * Handles REFRESHWINDOW event from the intuition IDCMP (A2 = message ptr)
 * =========================================================================
 */
void RefreshWindow()
{
	/* set pen to white */
	SetAPen(PrefWindow->RPort, 1);

	/* move to left edge below top row of gadgets */
	Move(PrefWindow->RPort, 0, 30 + top_offs);

	/* draw along to right edge */
	Draw(PrefWindow->RPort, 300, 30 + top_offs);

	/* draw down side of RAM gadgets */
	Draw(PrefWindow->RPort, 104, 30 + top_offs);
	Draw(PrefWindow->RPort, 104, 100 + top_offs);

	/* make it double thickness */
	Move(PrefWindow->RPort, 105, 30 + top_offs);
	Draw(PrefWindow->RPort, 105, 100 + top_offs);

	/* Highlight proper gadget */
	RefreshGads();

	/* do the on/off gadgets */
	RefreshSwitches();
}

void MainProgram()
{
struct IntuiMessage *msg;

	FinishedFlag = -1;		/* non zero = not finished */
	RefreshWindow();		/* draw non gadget stuff */

	while (FinishedFlag) {
		FinishedFlag = -1;		/* non zero = not finished */

		Wait(MSigs);			/* go to sleep */

		while (msg = (struct IntuiMsg *)GetMsg(MPort)) {
			if (msg->Class == REFRESHWINDOW) {
				BeginRefresh(PrefWindow);
				RefreshWindow();
				EndRefresh(PrefWindow);
			} else if (msg->Class == GADGETUP) {
				HandleGadgetUp(msg);
			} else if (msg->Class == GADGETDOWN) {
				HandleGadgetDown(msg);
			}

			/* Message has been received and dispatched,
			 * so now return it to intuition
			 */

			ReplyMsg(msg);

			if (!FinishedFlag)
				break;
		}
	}

	/* we're done - tell intuition to shut up */
	ModifyIDCMP(PrefWindow, 0);
}

