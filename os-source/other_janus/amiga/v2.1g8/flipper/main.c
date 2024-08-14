#include <stdio.h>

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

extern UBYTE *john_reg;


int haveoutput;

void output(char *str)
{
	if (haveoutput)
		Write(Output(), str, strlen(str));
}

void getstring(char *buf, int len)
{
int n;

	n = Read(Input(), buf, len - 1);
	if (n == -1)
		buf[0] = 0;
	else
		buf[n] = 0;
}

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

	if (newmode & MODE_QUIT_DAMMIT) {
		going = 0;
		newmode = MODE_MANUAL | SELECT_AMIGA;
	}

	return newmode;
}

#define DISKBIT_PC	0
#define DISKBIT_AMIGA	1

#define MODE_00		0
#define MODE_01		1
#define MODE_10		2

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
extern struct CIA volatile ciab;

	ciab.ciaprb |= CIAF_DSKMOTOR;			/* motor off */
	ciab.ciaprb &= ~(CIAF_DSKSEL0 << amiga_unit);	/* select unit */
	ciab.ciaprb |= (CIAF_DSKSEL0 << amiga_unit);	/* deselect unit */
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

void diskbit(int state)
{
	if (state == DISKBIT_AMIGA)
		*john_reg = 0x0a;
	else
		*john_reg = 0x0b;
}

int autosniff(int state)
{
	diskbit(DISKBIT_AMIGA);			/* drive to amiga mode */
	modechange(MODE_10);			/* give drive to amiga */
	HandleDisk();				/* try to read it */
	if (currentdisk == DISK_PC) {		/* pc? */
		diskbit(DISKBIT_PC);		/* drive to pc mode */
		if (dualspeed_kludge) {		/* yes - kludge? */
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

	while (going || newmode) {
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
				diskbit(DISKBIT_AMIGA);
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
				diskbit(DISKBIT_PC);
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
			output("Flipper: not running, can't set Auto mode.\n");
			Quit(PARAMETER_ERROR);
		}

		if (newmode & SELECT_PC) {
			diskbit(DISKBIT_PC);
			writemode(MODE_00);
		}

		if (newmode & SELECT_AMIGA) {
			diskbit(DISKBIT_AMIGA);
			writemode(MODE_10);
		}

		if (newmode & MODE_QUIT_DAMMIT)
			return;

		if (newmode & MODE_QUIT) {
			/* Quit only makes sense to an already running flipper. */
			output("Flipper: not running, not possible to quit.\n");
			Quit(PARAMETER_ERROR);
		}
	}
}

void main(argc, argv)
int argc;
char **argv;
{
BPTR f;
UBYTE p[2];
char *av[4];
char ibuf[80];
char *token;
int x;

#define BRKCHR	" \t"

	extern int window_position(char *);

	if (argv > 0)
		haveoutput = 1;
	else
		haveoutput = 0;

	DebugInit();
/*
   Check for hardware first, not much sense in going through the whole
   business to find out they don't have a board!
 */
	if (!FindJanus()) {
		if (argc > 0)
			output("Flipper: Janus.library or Flipper hardware not found.\n");
		Quit(JANUS_ERROR);
	}

	for (x = 0; x < argc; x++)
		av[x] = argv[x];

again:
	if (argc > 3) {
		output("Flipper: too many arguments\n");
		Quit(PARAMETER_ERROR);
	}

	if (argc == 1) {
		output("Flipper: required argument missing\n");
		Quit(PARAMETER_ERROR);
	}

	if (argc == 2 && !strcmp(av[1], "?")) {
		output("AMIGA/S,PC/S,AUTO/S,QUIT/S,WINDOW=/K: ");
		getstring(ibuf, sizeof(ibuf) - 1);
		x = strlen(ibuf);
		if (x) {
			if (ibuf[x-1] == '\n')
				ibuf[x-1] = 0;
		}
		argc = 1;
		token = strtok(ibuf, BRKCHR);
		while (token && token[0]) {
			av[argc++] = token;
			if (argc == 4)
				break;
			token = strtok(0, BRKCHR);
		}

		goto again;
	}

/*** magic prefs ***/
	if (!(f = Open("sys:pc/system/2500prefs", MODE_OLDFILE)))
		Quit(JANUS_ERROR);

	p[0] = p[1] = 0;

	if (Read(f, &p[0],  sizeof(p)) > 1)
		Close(f);

	if (p[1] & 0x02)
		dualspeed_kludge = 1;
	else
		dualspeed_kludge = 0;

	switch (p[1] & 0x38) {
	case 0x20:
	case 0x30:
		pc_number = PC_A;
		pc_name = pc_a;
		break;
				
	case 0x08:
	case 0x28:
		pc_number = PC_B;
		pc_name = pc_b;
		break;

	default:
		NewMode(MODE_QUIT_DAMMIT);
		Quit(JANUS_NOTSHARED);
		break;
	}

	if (p[1] & 0x04) {
		amiga_number = AMIGA_DF1;
		amiga_name = amiga_df1;
	} else {
		amiga_number = AMIGA_DF0;
		amiga_name = amiga_df0;
	}
/*** magic prefs ***/


	if (argc == 0) {
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
			} else {
				FlipperLoop();
			}
		} else {
			NewMode(currentmode| currentselect);
		}

		DebugEnd();
		exit(0);
	}

	if (argc == 3) {
		/* 
			Command line has all possible arguments: E.g.
			flipper auto window=xxx,yyy
			We are concerned with window spec. here.
		 */
		if (strnicmp(window_spec , av[2], strlen(window_spec)) == 0) {
			if (!window_position(av[2] + strlen(window_spec))) {
				Debug1("Bad window specification %s\n", av[4]);
				Quit(PARAMETER_ERROR);
			}
			window_mode = 1;
		} else {
			Debug1("Illegal window specification: %s\n", av[4]);
			output("Flipper: bad window specification\n");
			Quit(PARAMETER_ERROR);
		}
	}

	if (argc >= 2) {
		if (stricmp(av[1], "amiga") == 0) {
			NewMode(MODE_MANUAL | SELECT_AMIGA);
		} else if (stricmp(av[1], "pc") == 0) {
			NewMode(MODE_MANUAL | SELECT_PC);
		} else if (stricmp(av[1], "auto") == 0) {
			currentmode = MODE_AUTO;
			if (FindPort(SHARENAME)) {
				/* Flipper is already running, send newmode message */
				NewMode(MODE_AUTO);
			} else {
				/* Try to start the flipper loop */
				if (argc == 2) {
					window_position("450,0");
				}
				FlipperLoop();
			}
		} else if (stricmp(av[1], "quit") == 0) {
			NewMode(MODE_QUIT);
		} else {
			Debug1("Bad command %s\n", av[1]);
			output("Flipper: bad switch\n");
			Quit(PARAMETER_ERROR);
		}
	}

	DebugEnd();
}
