#include <exec/types.h>
#include <graphics/gfxbase.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <libraries/dos.h>
#include <janus/janus.h>
#include <libraries/dos.h>
#include <libraries/expansion.h>
#include <libraries/expansionbase.h>
#include "mydata.h"

extern struct Gadget CancelGadget, A000Gadget, D000Gadget, E000Gadget,
		     MonoGadget, ColorGadget, ShadowGadget;
#if 0
extern struct Gadget SerialGadget;
#endif

extern void MainProgram();

struct Window *PrefWindow;	/* this programs window */
struct MsgPort *MPort;		/* this programs message port */
ULONG	MSigs;			/* signal bit from message port */
ULONG	FinishedFlag;		/* flag, time to exit the program */
UWORD	FinishCode;		/* what to do when finished */
BPTR	PrefsFile;		/* file handle for prefs file */
#if 0
UBYTE	SerialState;		/* state of serial enable/disable */
#endif
UBYTE	ShadowState;
UBYTE	MonoState;		/* state of mono enable/disable */
UBYTE	ColorState;		/* state of color enable/disable */
UBYTE	RamBank;		/* position of 64k RAM bank */
int	top_offs;		/* offset from top */

char *PName = "sys:PC/System/2500prefs";

struct Library *ExpansionBase;

struct NewWindow prefwindow = {
	170,50,300,83,				/* left,top,width,height */
	2,1,					/* detailpen,blockpen */
	REFRESHWINDOW | GADGETUP | GADGETDOWN,	/* idcmp */
	WINDOWDRAG | WINDOWDEPTH | SMART_REFRESH,	/* flags */
	&CancelGadget,				/* first gadget */
	0,					/* checkmark */
	"PC Configuration V2.0",		/* title *not relocatable* */
	0,					/* screen */
	0,					/* bitmap */
	0,0,0,0,				/* min/max width/height */
	WBENCHSCREEN				/* window type */
};

extern struct GfxBase *GfxBase;
extern struct IntuitionBase *IntuitionBase;

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

void main()
{
struct JanusBase *JanusBase;
UBYTE *JIOBase;
UBYTE prefs[2];
struct Screen wbscr;
struct Gadget *g;
int n;

/* Open janus library to get the base address of the IO register block.
 * It can be closed immediately after because we don't need it any more.
 */
	if (!(JanusBase = (struct JanusBase *)OpenLibrary("janus.library", 0)))
		goto exit1;

	JIOBase = JanusBase->jb_IoBase;
	CloseLibrary(JanusBase);

	if (!(GfxBase = (struct GfxBase *)OpenLibrary("graphics.library", 0)))
		goto exit1;

	if (!(IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library", 0)))
		goto exit1;

/* Open the 2500 preferences file and read in the current state variables.
 * If the file doesn't exist then fill in default values in the state vars.
 */
	ShadowState = DEFSHAD;
#if 0
	SerialState = DEFSER;
#endif
	MonoState = DEFMON;
	ColorState = DEFCOL;
	RamBank = DEFRAM;

	if (PrefsFile = Open(PName, MODE_OLDFILE)) {
		n = Read(PrefsFile, &prefs[0], 2);
		if (n == 1 || n == 2) {
#if 0
			SerialState = (prefs[0] >> 0) & 0x01;	/* bit 0 */
#endif
			MonoState = (prefs[0] >> 3) & 0x01;	/* bit 3 */
			ColorState = (prefs[0] >> 4) & 0x01;	/* bit 4 */
			RamBank = (prefs[0] >> 5) & 0x03;	/* bit 6,5 */
		}

		if (n == 2) {
			ShadowState = (prefs[1] >> 0) & 0x01;
		}

		Close(PrefsFile);
	}

#if 0
	if (SerialState != 0 && SerialState != 1)
		SerialState = DEFSER;
#endif

	if (ShadowState != 0 && ShadowState != 1)
		ShadowState = DEFSHAD;

	if (MonoState != 0 && MonoState != 1)
		MonoState = DEFMON;

	if (ColorState != 0 && ColorState != 1)
		ColorState = DEFMON;

	if (RamBank < 1 || RamBank > 3)
		RamBank = DEFRAM;

	if (!(*(JIOBase+jio_PCconfiguration) & 0x80) && (RamBank == 3))
		RamBank = DEFRAM;

#if 0
	if (SerialState)
		SerialGadget.Flags |= SELECTED;
#endif

	if (ShadowState)
		ShadowGadget.Flags |= SELECTED;

	if (MonoState)
		MonoGadget.Flags |= SELECTED;

	if (ColorState)
		ColorGadget.Flags |= SELECTED;

	if (RamBank == 1)
		A000Gadget.Flags |= SELECTED;
	else if (RamBank == 2)
		D000Gadget.Flags |= SELECTED;
	else if (RamBank == 3)
		E000Gadget.Flags |= SELECTED;

	if (GetScreenData((APTR)&wbscr, sizeof(struct Screen), WBENCHSCREEN, NULL)) {
		top_offs = wbscr.BarHeight - 10;
	} else {
		top_offs = 0;
	}

	prefwindow.Height += top_offs;

	for (g = &CancelGadget; g; g = g->NextGadget) {
		g->TopEdge += top_offs;
	}

	if (!(PrefWindow = OpenWindow(&prefwindow)))
		goto exit4;
	
	if (!(*(JIOBase+jio_PCconfiguration) & 0x80))
		OffGadget(&E000Gadget, PrefWindow, 0);

	/* if this is a 386 bb, do the shadow gadget */
	if (ExpansionBase = (struct Library *) OpenLibrary("expansion.library",0)) {
		if (!FindConfigDev(0, 513, 103)) {
			OffGadget(&ShadowGadget, PrefWindow, 0);
		}
		CloseLibrary(ExpansionBase);
	}

#if 0
	OffGadget(&SerialGadget, PrefWindow, 0);
#endif

	MPort = PrefWindow->UserPort;
	MSigs = 1 << MPort->mp_SigBit;

	MainProgram();

	if (FinishCode != 0) {
		/* set bit 7 always,
		 * set bit 2 (keyboard enable),
		 * set bit 1 (parallel enable)
		 */
		prefs[0] = 0x86;
		prefs[1] = 0;
#if 0
		prefs[0] |= (SerialState & 0x01) << 0;
#endif
		prefs[0] |= (MonoState & 0x01) << 3;
		prefs[0] |= (ColorState & 0x01) << 4;
		prefs[0] |= (RamBank & 0x03) << 5;
	
		prefs[1] |= (ShadowState & 0x01) << 0;

		*(JIOBase+jio_PCconfiguration) = prefs[0];

		if (FinishCode == 2) {
			if (PrefsFile = Open(PName, MODE_NEWFILE)) {
				Write(PrefsFile, &prefs[0], 2);
				Close(PrefsFile);
			}
		}
	}

	CloseWindow(PrefWindow);

/* =========================================================================
 * These are the various exits from the program, no error codes returned
 * =========================================================================
 */
exit4:
exit3:
	CloseLibrary(IntuitionBase);

exit2:
	CloseLibrary(GfxBase);

exit1:

	exit(0);
}
