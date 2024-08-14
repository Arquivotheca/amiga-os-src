#include <exec/types.h>
#include <exec/exec.h>
#include <libraries/dos.h>
#include <intuition/intuition.h>
#include <workbench/icon.h>
#include <workbench/workbench.h>
#include <workbench/startup.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>

#include <hardware/cia.h>
#include <resources/disk.h>

extern struct DiscResource *DiskBase;

#include "disk_res.h"

/* #include <local/sccs.h> */

#define MAIN 1

#include "share.h"
#include "string.h"

/* static char SccsId[] = SCCS_STRING; */
static char port_name[] = SHARENAME;

struct MsgPort *FlipperPort;
struct Message ToFlipper;

int dualspeed_kludge;

/*
   Quit is supposed to make sure all resources opened get closed.
*/

extern void writemode(UBYTE bits);

void Quit(error)
int error;
{
int retvalue = 10;

	switch (error) {

		case NO_ERROR:
			retvalue = 0;
			break;

		case PARAMETER_ERROR:
			Debug0("Bad or unknown parameter specification\n");
			retvalue = 5;
			break;

		default:
			Debug1("Unkown Error %d\n", error);
			retvalue = 20;
	}

	DebugEnd();
	Exit(retvalue);
}

void Usage(string)
char *string;
{
static char hints[] = "Flipper Amiga|PC|Auto|Quit \
[df0:|df1: [A:|B: window=xxx,yyy]]\n";

	Write(Output(), hints, strlen(hints));
	Write(Output(), string, strlen(string));
}

int CMDFromPort()
{
int newmode;
struct Message *flippermessage;
extern LocalMode(int);

	Debug0("CMDFromPort\n");

	flippermessage = GetMsg(FlipperPort);

	if (flippermessage == NULL) {
		Debug0("CMDFromPort: Null message received???\n");
		return 0;
	}

	newmode = flippermessage->mn_Length;
	ReplyMsg(flippermessage);

	Debug1("Got newmode: %lx\n", newmode);

	if (newmode & MODE_QUIT) 
		going = 0;

	return newmode;
}

#define MODE_00 0
#define MODE_01 1
#define MODE_10 2

#define MANUAL_PC	0
#define MANUAL_AMIGA	1
#define AUTO_PC_PRE	2
#define AUTO_PC		3
#define AUTO_AMIGA	4

int modetable[] = {	MODE_MANUAL,	MODE_MANUAL,	MODE_AUTO,
			MODE_AUTO,	MODE_AUTO };
int owntable[] = {	SELECT_PC,	SELECT_AMIGA,	SELECT_PC,
			SELECT_PC,	SELECT_AMIGA };

#define EVENT_DISKCHANGE	(1 << 0)
#define EVENT_MANUAL		(1 << 1)
#define EVENT_AUTO		(1 << 2)
#define EVENT_PC		(1 << 3)
#define EVENT_AMIGA		(1 << 4)

int getunit(int amiga_unit)
{
struct DiscResourceUnit *dru;
extern struct MsgPort *dru_port;

	if (!(dru = AllocMem(sizeof(*dru), MEMF_PUBLIC))) {
		return FALSE;
	}

	while (1) {
		memset(dru, 0, sizeof(*dru));
		dru->dru_Message.mn_Node.ln_Type = NT_MESSAGE;
		dru->dru_Message.mn_Node.ln_Name = "Flipper";
		dru->dru_Message.mn_ReplyPort = dru_port;

		if (GetUnit(dru))
			break;

		do {
			Wait(1 << dru_port->mp_SigBit);
		} while (!GetMsg(dru_port));
	}

	/* we got it */
	FreeMem(dru, sizeof(*dru));

	return TRUE;
}

void giveunit(int amiga_unit)
{
	GiveUnit();
}

void motor_off(int amiga_unit)
{
extern struct CIA * volatile ciab;

	ciab->ciaprb |= CIAF_DSKMOTOR;			/* motor off */
	ciab->ciaprb &= ~(CIAF_DSKSEL0 << amiga_unit);	/* select unit */
	ciab->ciaprb |= (CIAF_DSKSEL0 << amiga_unit);	/* deselect unit */
}

void modechange(UBYTE bits)
{
	writemode(MODE_10);

	inhibit(amiga_name, 1);

	if (getunit(amiga_number)) {
		motor_off(amiga_number);
		Delay(25);		/* 1/2 second for spindown */
		writemode(bits);
		giveunit(amiga_number);
	} else {
		writemode(bits);
	}

	inhibit(amiga_name, 0);
}

int autosniff(int state)
{
	modechange(MODE_10);			/* give drive to amiga */
	HandleDisk();				/* try to read it */
	if (currentdisk == DISK_PC) {		/* pc? */
		if (dualspeed_kludge) {		/* yes - kludge? */
			modechange(MODE_00);	/* yes - give to pc only */
			return AUTO_PC_PRE;	/* enter kludge state */
		} else {			/* nokludge. */
			modechange(MODE_01);	/* give to pc */
			return AUTO_PC;		/* enter normal pc auto */
		}
	} else if (currentdisk == DISK_EMPTY) {	/* empty? */
		return state;			/* yes - no change */
	} else {				/* must be amiga! */
		modechange(MODE_10);		/* give to amiga */
		return AUTO_AMIGA;		/* enter amiga state */
	}
}

void FlipperLoop()
{
int newmode, oldmode;
int state;
int event;

	Debug0("Enter Flipper Auto Mode\n");

	if (!OpenDiskStuff()) {
		Debug0("Can't open the disk!\n");
		return;
	}

	Debug0("Open Disk OK\n");

	if (window_mode) {
		if (!OpenDisplay()) {
			Debug0("Can't open the display!\n");
			CloseDiskStuff();
			return;
		}
	}

	FlipperPort = CreatePort(SHARENAME, 0);
	port_mask = (1 << FlipperPort->mp_SigBit);

	going = 1;

	if (currentmode & MODE_AUTO) {
		if (currentmode & SELECT_PC) {
			state = autosniff(AUTO_PC);
		} else {
			state = autosniff(AUTO_AMIGA);
		}
	} else {
		if (currentmode & SELECT_PC) {
			modechange(MODE_00);
			state = MANUAL_PC;
		} else {
			modechange(MODE_10);
			state = MANUAL_AMIGA;
		}
	}

	if (window_mode)
		UpdateDisplay(modetable[state], owntable[state]);

	newmode = 0;

	wait_mask = (user_mask | port_mask | disk_mask);

	while (going) {
		int gotsignal;

		event = 0;

		if (newmode)
			gotsignal = 0;
		else
			gotsignal = Wait(wait_mask);

		Debug1("Got Signal %lx\n", gotsignal);

		/* did we get a disk change? */
		if (gotsignal & disk_mask) {
			event |= EVENT_DISKCHANGE;
		}

		if (gotsignal & user_mask) {
			Debug0("User Signal!\n");
			newmode |= HandleUser();
		}

		if (gotsignal & port_mask) {
			Debug0("Port Signal!\n");
			newmode |= CMDFromPort();
		}

		oldmode = modetable[state] | owntable[state];

		if (((oldmode ^ newmode) & MODE_MANUAL) && (newmode & MODE_MANUAL)) {
			newmode &= ~MODE_MANUAL;
			event |= EVENT_MANUAL;
		} else if (((oldmode ^ newmode) & MODE_AUTO) && (newmode & MODE_AUTO)) {
			newmode &= ~MODE_AUTO;
			event |= EVENT_AUTO;
		} else if (((oldmode ^ newmode) & SELECT_AMIGA) && (newmode & SELECT_AMIGA)) {
			newmode &= ~SELECT_AMIGA;
			event |= EVENT_AMIGA;
		} else if (((oldmode ^ newmode) & SELECT_PC) && (newmode & SELECT_PC)) {
			newmode &= ~SELECT_PC;
			event |= EVENT_PC;
		} else {
			/* nothing - nuke it */
			newmode = 0;
		}

		switch (state) {

		case MANUAL_PC:
			if (event & EVENT_DISKCHANGE) {
				/* never happens */
			}

			if (event & EVENT_MANUAL) {
				/* do nothing (already here) */
			}

			if (event & EVENT_PC) {
				/* do nothing (already here) */
			}

			if (event & EVENT_AMIGA) {
				modechange(MODE_10);
				state = MANUAL_AMIGA;
			}

			if (event & EVENT_AUTO) {
				state = autosniff(AUTO_PC);
			}

			break;

		case MANUAL_AMIGA:
			if (event & EVENT_DISKCHANGE) {
				/* do nothing */
			}

			if (event & EVENT_MANUAL) {
				/* do nothing (already here) */
			}

			if (event & EVENT_PC) {
				modechange(MODE_00);
				state = MANUAL_PC;
			}

			if (event & EVENT_AMIGA) {
				/* do nothing (already here) */
			}

			if (event & EVENT_AUTO) {
				state = autosniff(AUTO_AMIGA);
			}

			break;

		case AUTO_PC_PRE:
			if (event & EVENT_DISKCHANGE) {

/* PROBLEM with original PAL design & dual speed drives:
 *
 * when autosniff() sees a pc disk, it *should* go directly to AUTO_PC.
 * but NOOOooo - the dual speed drives forget about their disk in/out
 * state when you change the magic "amiga/pc" pin.  this means if we
 * went directly to the 01 state (pc auto) the amiga would check the
 * drive and see a diskchange.  in mode 01, if the amiga sees a diskchange
 * the hardware automatically reverts to mode 10 (amiga auto).  the  amiga
 * would see another diskchange, sniffer would run again, and the same
 * thing would happen over and over, until the disk is removed.

 * SOLUTION as we discussed:
 *
 * to fix this, the hardware has been modified to allow the amiga to
 * read diskchange and step the head even in state 00 (pc manual).
 * this is potentially hazardous to things operating on the pc disk,
 * since the heads can be moved without the pc knowing it.
 * autosniff() puts the drive in state 00 (flipping the state of the
 * magic pin, and causing the amiga to see a diskchange) and puts the
 * software into this state.  this state waits for the diskchange to
 * appear, and then puts the hardware in state 01 (auto pc) - now the
 * hardware is in the right state, and since the magic pin didn't change
 * state, the drive has the correct idea of disk in/out.

 * FLAW in the above solution:
 *
 * sequence of events:
 *	start at AUTO_AMIGA, no disk in drive.
 *	insert a pc disk.
 *	autosniff() sees pc format, sets hw to state 00 (man pc), sw to
 *		state AUTO_PC_PRE.
 *	trackdisk checks drive, and sees that the disk has been "removed,"
 *		because the dual speed drive's magic pin has changed.
 *	we wait for and receive our diskchange event, and set the hw
 *		to 01, and sw to AUTO_PC.
 *	trackdisk checks drive, and sees that a disk has been inserted.
 *	we receive a diskchange event and sniff the disk again!!!

 * FIX for the above flaw:
 *
 *	don't go to AUTO_PC until we see disk inserted (ie, two disk
 *		changes have passed).	
 *
 * result:
 *	start at AUTO_AMIGA, no disk in drive.
 *	insert a pc disk.
 *	autosniff() sees pc format, sets hw to state 00 (man pc), sw to
 *		state AUTO_PC_PRE.
 *	trackdisk checks drive, and sees that the disk has been "removed,"
 *		because the dual speed drive's magic pin has changed.
 *	we wait for and receive 1st diskchange event.  this event is
 *		a disk removal, so we ignore it.
 *	trackdisk checks drive, and sees that the disk has been "inserted,"
 *		since it was never really removed.
 *	we wait for and receive 2nd diskchange event.  this event is
 *		a disk insertion, so we set the hw to 01, and sw to AUTO_PC.
 *	trackdisk checks drive, and sees that nothing has changed, so
 *		flipper remains unchanged (both hw and sw).

 * FLAW in above FIX:
 *
 *	the normal amiga 880K drives don't have a magic pin, so they won't
 *	forget their diskchange status.  in other words, if an 880K drive
 * 	is being shared, autosniff() should set the hw directly to state
 *	01 and the sw to state AUTO_PC, skipping the AUTO_PC_PRE diskchange
 *	kludge state.

 */
				if (DiskInserted()) {
					writemode(MODE_01);
					state = AUTO_PC;
				}
			}

			if (event & EVENT_MANUAL) {
				/* ignore till later */
			}

			if (event & EVENT_PC) {
				/* never happens */
			}

			if (event & EVENT_AMIGA) {
				/* never happens */
			}

			if (event & EVENT_AUTO) {
				/* do nothing (already here) */
			}

			break;

		case AUTO_PC:
			if (event & EVENT_DISKCHANGE) {
				if (DiskInserted()) {
					state = autosniff(state);
				}
			}

			if (event & EVENT_MANUAL) {
				writemode(MODE_00);
				state = MANUAL_PC;
			}

			if (event & EVENT_PC) {
				/* never happens */
			}

			if (event & EVENT_AMIGA) {
				/* never happens */
			}

			if (event & EVENT_AUTO) {
				/* do nothing (already here) */
			}

			break;

		case AUTO_AMIGA:
			if (event & EVENT_DISKCHANGE) {
				if (DiskInserted()) {
					state = autosniff(state);
				}
			}

			if (event & EVENT_MANUAL) {
				writemode(MODE_10);
				state = MANUAL_AMIGA;
			}

			if (event & EVENT_PC) {
				/* never happens */
			}

			if (event & EVENT_AMIGA) {
				/* never happens */
			}

			if (event & EVENT_AUTO) {
				/* do nothing (already here) */
			}

			break;

		}

		if (window_mode)
			UpdateDisplay(modetable[state], owntable[state]);
	}

	/* switch flipper hw to appropriate manual mode before we go */

	if (owntable[state] == SELECT_PC) {
		writemode(MODE_00);
	} else {
		writemode(MODE_10);
	}

	DeletePort(FlipperPort);

	if (window_mode)
		CloseDisplay();

	CloseDiskStuff();
}

/*
   NewMode is mostly used to manipulate an already running flipper from
   the CLI. It sends a message to its messageport which induces the
   background process to change the hardware settings and display
   accordingly. If the named port cannot be found, only simple
   back and forth switches can be made; if the flipper process is found,
   it can be told to go into automatic mode too.
*/

void NewMode(newmode)
int newmode;
{
	struct MsgPort *hisport, *myport;

	Debug1("Trying to set new mode %lx\n", newmode);

	if ((hisport = FindPort(SHARENAME)) != NULL) {
		Debug0("Flipper is already running\n");
		myport = CreatePort(0,0);
		if (myport == NULL) {
			Debug0("Can't create reply port.. exiting\n");
			return;
		}
		ToFlipper.mn_Length = newmode;
		ToFlipper.mn_Node.ln_Type = NT_MESSAGE;
		ToFlipper.mn_ReplyPort = myport;
		PutMsg(hisport, &ToFlipper);
		WaitPort(myport);
		DeletePort(myport);
		Debug0("Received reply to newmode message\n");
		return;
	}
	else {
		/*
		   No port found, we don't create one either. We just write the
		   register to make the drive accessible to whichever machine
		   is supposed to get it. In the case of PC this means that
		   diskchange info won't get through to the Amiga anymore.
		 */
		Debug0("No Flipper running.. check for manual\n");

		if (newmode & MODE_AUTO) {
			/* Auto only makes sense to an already running flipper. */
			Usage("Flipper is not running, can't set Automatic mode\n");
			Quit(PARAMETER_ERROR);
		}

		if (newmode & SELECT_PC) {
			writemode(MODE_00);
		}

		if (newmode & SELECT_AMIGA) {
			writemode(MODE_10);
		}

		if (newmode & MODE_QUIT) {
			/* Quit only makes sense to an already running flipper. */
			Usage("Flipper is not running, not possible to quit it\n");
			Quit(PARAMETER_ERROR);
		}
	}
}

void main(argc, argv)
int argc;
char **argv;
{
	extern int window_position(char *);

	DebugInit();
/*
   Check for hardware first, not much sense in going through the whole
   business to find out they don't have a board!
 */
	if (!FindJanus()) {
		Quit(JANUS_ERROR);
	}

	dualspeed_kludge = 0;

	switch (argc) {
		case 0:
			/* Called from Workbench, look at tooltypes. */
#ifdef WB_SUPPORT
			HandleWB((struct WBStartup *) argv);
#else
			Debug0("Tooltypes not supported....\n");
			Quit(PARAMETER_ERROR);
#endif
			if (currentmode & MODE_AUTO) {
				if (FindPort(SHARENAME)) {
					NewMode(MODE_AUTO);
				}
				else {
					FlipperLoop();
				}
			}
			else {
				NewMode(currentmode| currentselect);
			}

			break;
		case 5:
		/* 
			Command line has all possible arguments: E.g.
			flipper auto dfx: A: window=xxx,yyy [dualspeed]
			We are concerned with window and PC drive spec. here.
		 */
		{
			int len;
			len = strlen(window_spec);
			Debug2("Length(%): %d\n", window_spec, len);
		}
		if (strnicmp(window_spec , argv[4], strlen(window_spec)) == 0) {
			if (!window_position(argv[4] + strlen(window_spec))) {
				Debug1("Bad window specification %s\n", argv[4]);
				Quit(PARAMETER_ERROR);
			}
			if ( stricmp(argv[3], pc_a ) == 0) {
				Debug0("PC drive is A:\n");
				pc_number = PC_A;
				pc_name = pc_a;
			}
			else if ( stricmp(argv[3], pc_b ) == 0) {
				Debug0("PC drive is B:\n");
				pc_number = PC_B;
				pc_name = pc_b;
			}
			else {
				Debug1("Bad drive %s\n", argv[5]);
				Quit(PARAMETER_ERROR);
			}
			window_mode = 1;
		}
		else {
			Debug1("Illegal window specification: %s\n", argv[4]);
			Quit(PARAMETER_ERROR);
		}
		case 3:
			if (stricmp(argv[2], amiga_df0) == 0) {
				amiga_name = amiga_df0;
				amiga_number = AMIGA_DF0;
			}
			else if (stricmp(argv[2], amiga_df1) == 0) {
				amiga_name = amiga_df1;
				amiga_number = AMIGA_DF1;
			}
			else {
				Debug1("Unkown Amiga Drive %s\n", argv[2]);
				Quit(PARAMETER_ERROR);
			}
		case 2:
			if (stricmp(argv[1], "amiga") == 0) {
				NewMode(MODE_MANUAL | SELECT_AMIGA);
			}
			else if (stricmp(argv[1], "pc") == 0) {
				NewMode(MODE_MANUAL | SELECT_PC);
			}
			else if (stricmp(argv[1], "auto") == 0) {
				currentmode = MODE_AUTO;
				if (FindPort(SHARENAME)) {
					/* Flipper is already running, send newmode message */
					NewMode(MODE_AUTO);
				}
				else {
					/* Try to start the flipper loop */
					if (argc > 2) {
						FlipperLoop();
					}
					else {
						Debug0("Auto flipper requested, no flipper running!\n");
						Usage("Flipper not running\n");
						Quit(PARAMETER_ERROR);
					}
				}
			}
			else if (stricmp(argv[1], "quit") == 0) {
				NewMode(MODE_QUIT);
			}
			else {
				Debug1("Bad command %s\n", argv[1]);
				Quit(PARAMETER_ERROR);
			}
			break;
		case 1:
		case 4:
		default:
			Usage("Argument count\n");
			Quit(PARAMETER_ERROR);
	}
	DebugEnd();
}
