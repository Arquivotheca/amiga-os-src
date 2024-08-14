#include	"exec/types.h"
#include	"exec/nodes.h"
#include	"exec/lists.h"
#include	"exec/memory.h"
#include	"exec/ports.h"
#include	"exec/libraries.h"
#include	"exec/devices.h"
#include	"exec/tasks.h"
#include	"exec/io.h"
#include	"printer.h"
#include	"prtbase.h"

#include "printer_iprotos.h"

#define DEBUGR		0
#define DEBUGCSI	0
#define DEBUGGC		0
#define DEBUGPCW	0
#define DEBUGPS		0

extern struct PrinterData *PD;
extern struct PrinterExtendedData *PED;
extern char *Default8BitChars[];
extern int  HoldPrefs;

/* the master input buffer */
UBYTE InputBuffer[P_BUFSIZE + 1];
/* the master output buffer */
UBYTE OutputBuffer[P_BUFSIZE + 1 + P_SAFESIZE];
/* an intermediate output buffer used by GetCommand */
UBYTE outputBuffer[P_BUFSIZE + 1];

BYTE vline; /* relative line position (up 1/2 , down 1, etc) */
BYTE crlfFlag; /* controls lf into cr/lf expansion */
UBYTE currentVMI; /* line spacing in VMI */
int Pos; /* Pos is the index counter into the users buffer */
int Position; /* Position is the index counter into InputBuffer */
int EscPos; /* position of last escape encountered */
BOOL EscFlag; /* EscFlag is a flag to indicate there is */
              /* a chance of a broken Esc sequence */
int InitCount;
int Counter;
int ParmCount;
int index; /* index into output buffer */
int comLength; /* length of expanded escape command */

/****** printer.device/CMD_WRITE ******************************************
*
*   NAME
*      CMD_WRITE -- send output to the printer
*
*   FUNCTION
*      This function causes a buffer of characters to be written to the
*      current printer port (usually parallel or serial).  The number of
*      characters is specified in io_Length, unless -1 is used, in which
*      case output is sent until a 0x00 is encountered.
*
*      The Printer device, like the Console device, maps ANSI X3.64 style
*      7-bit printer control codes to the control code set of the current
*      printer.  The ANSI codes supported can be found below.
*
*   NOTES
*      Not all printers will support all functions.  In particular you may
*      not assume that the MARGINS or TABS can be set.  Close to half the
*      supported printers don't fully implement one or the other.  If you
*      want the features of margins or tabs you will need to fake it
*      internally by sending out spaces.
*
*      Note that the printer device may have already sent out a "set
*      margins" command to the printer.  If you are faking your own
*      margins, be sure to cancel the old ones first.  (use the "aCAM"
*      command)
*
*      Defaults are set up so that if a normal AmigaDOS text file
*      is sent to PRT:, it has the greatest chance of working.
*      (AmigaDOS text files are defined as follows:)
*              tabs            - every 8
*              CR (0x0D)       - moves to start of current line
*              LF (0x0A)       - moves to start of next line
*
*   IO REQUEST
*       io_Message      mn_ReplyPort set
*       io_Device       preset by OpenDevice
*       io_Unit         preset by OpenDevice
*       io_Command      CMD_WRITE
*       io_Length       number of characters to process, or if -1,
*                       process until 0x00 encountered
*       io_Data         pointer to block of data to process
*
*   RESULTS
*       io_Error : if CMD_WRITE succeeded, then io_Error will be zero.
*              Otherwise io_Error will be non-zero.
*
*   SEE ALSO
*       printer.h, parallel.device, serial.device, Preferences
*
*
*   ANSI X3.64 style COMMANDS
*
* aRIS         ESCc            hard reset
* aRIN         ESC#1           initialize to defaults
* aIND         ESCD            true linefeed (lf)
* aNEL         ESCE            return,lf
* aRI          ESCM            reverse lf              *
*
* aSGR0        ESC[0m          normal character set
* aSGR3        ESC[3m          italics on
* aSGR23       ESC[23m         italics off
* aSGR4        ESC[4m          underline on
* aSGR24       ESC[24m         underline off
* aSGR1        ESC[1m          boldface on
* aSGR22       ESC[22m         boldface off
* aSFC         SGR30-39        set foreground color
* aSBC         SGR40-49        set background color
*
* aSHORP0      ESC[0w          normal pitch
* aSHORP2      ESC[2w          elite on
* aSHORP1      ESC[1w          elite off
* aSHORP4      ESC[4w          condensed on
* aSHORP3      ESC[3w          condensed off
* aSHORP6      ESC[6w          enlarged on
* aSHORP5      ESC[5w          enlarged off
*
* aDEN6        ESC[6"z         shadow print on
* aDEN5        ESC[5"z         shadow print off
* aDEN4        ESC[4"z         doublestrike on
* aDEN3        ESC[3"z         doublestrike off
* aDEN2        ESC[2"z         Near Letter Quality (NLQ) on
* aDEN1        ESC[1"z         NLQ off
*
* aSUS2        ESC[2v          superscript on
* aSUS1        ESC[1v          superscript off
* aSUS4        ESC[4v          subscript on
* aSUS3        ESC[3v          subscript off
* aSUS0        ESC[0v          normalize the line      *
* aPLU         ESCL            partial line up         *
* aPLD         ESCK            partial line down       *
*
* aFNT0        ESC(B           US char set (default)   or Font 0
* aFNT1        ESC(R           French char set         or Font 1
* aFNT2        ESC(K           German char set         or Font 2
* aFNT3        ESC(A           UK char set             or Font 3
* aFNT4        ESC(E           Danish I char set       or Font 4
* aFNT5        ESC(H           Sweden char set         or Font 5
* aFNT6        ESC(Y           Italian char set        or Font 6
* aFNT7        ESC(Z           Spanish char set        or Font 7
* aFNT8        ESC(J           Japanese char set       or Font 8
* aFNT9        ESC(6           Norweign char set       or Font 9
* aFNT10       ESC(C           Danish II char set      or Font 10
*
* aPROP2       ESC[2p          proportional on         *
* aPROP1       ESC[1p          proportional off        *
* aPROP0       ESC[0p          proportional clear      *
* aTSS         ESC[n E         set proportional offset *
* aJFY5        ESC[5 F         auto left justify       *
* aJFY7        ESC[7 F         auto right justify      *
* aJFY6        ESC[6 F         auto full justify       *
* aJFY0        ESC[0 F         auto justify off        *
* aJFY3        ESC[3 F         letter space (justify)  *
* aJFY1        ESC[1 F         word fill(auto center)  *
*
* aVERP0       ESC[0z          1/8" line spacing
* aVERP1       ESC[1z          1/6" line spacing
* aSLPP        ESC[nt          set form length n
* aPERF        ESC[nq          set perforation skip to n lines (n>0)
* aPERF0       ESC[0q          perforation skip off
*
* aLMS         ESC#9           Left margin set         *
* aRMS         ESC#0           Right margin set        *
* aTMS         ESC#8           Top margin set          *
* aBMS         ESC#2           Bottom margin set       *
* aSTBM        ESC[Pn1;Pn2r    set T&B margins         *
* aSLRM        ESC[Pn1;Pn2s    set L&R margin          *
* aCAM         ESC#3           Clear margins
*
* aHTS         ESCH            Set horiz tab           *
* aVTS         ESCJ            Set vertical tabs       *
* aTBC0        ESC[0g          Clr horiz tab           *
* aTBC3        ESC[3g          Clear all h tab         *
* aTBC1        ESC[1g          Clr vertical tabs       *
* aTBC4        ESC[4g          Clr all v tabs          *
* aTBCALL      ESC#4           Clr all h & v tabs      *
* aTBSALL      ESC#5           Set default tabs (every 8)
*
* aEXTEND      ESC[Pn"x        Extended commands
*                              This is a mechanism for printer drivers to
*                              support extra commands which can be called
*                              by ANSI control sequences
* aRAW         ESC[Pn"r        Next 'Pn' chars are raw (ie. they are not
*                              parsed by the printer device, instead they
*                              are sent directly to the printer.
*
*
* (*) indicates that sending this command may cause unexpected results
*     on a large number of printers.
*
************************************************************************/

unsigned char GetParms();
unsigned char ReadBuffer();

UBYTE PCWrite(ior) /* CMD_WRITE */
struct IOStdReq *ior;
{
	int OutLength, status;
	long InputLength;
	int Ilength, tstart, tlength;
	UBYTE EnPWait;

	status = 0;
/*
	redundent since a PBothReady is done after every PWrite
	if ((status = PBothReady()) !=0 ) {
		return(status);
	}
*/
	if ((InputLength = (ior->io_Length)) == -1) { /* if no length specified */
		InputLength = 0; /* zero length, and then count it */
		while(((unsigned char *)(ior->io_Data))[InputLength++] !=0 );
	}
	HoldPrefs=0; /* kluge because dos opens the printer 3 times */
	Pos = 0;
	if (PD->pd_SegmentData->ps_Version >= 35) { /* if >= V1.3 */
		PED->ped_PrintMode = 1; /* flagged that alpha data has been sent */
	}
	while((status == 0) && (Pos < InputLength)) {
		InitCount = Counter;
		EscFlag = FALSE;
		/* copy the data into the input buffer */
		while((Pos < InputLength) && (Counter < P_BUFSIZE)) {
			InputBuffer[Counter++] = ((unsigned char *)(ior->io_Data))[Pos++];
		}
		Ilength = Counter;
		OutLength = ProcessString(Ilength, OutputBuffer);/* send the string */
		EnPWait = PD->pd_PWaitEnabled;
		if (EnPWait == 0) {
			status = PWrite(OutputBuffer, OutLength);
			if (status == 0) {
				status=PBothReady(); /* wait until buffer is empty */
			}
		}
		else {
			tlength=0;
			tstart=0;
			status=0;
			while ((tstart < OutLength) && (status == 0)) {
				while (((tstart + tlength) < OutLength) &&
					(OutputBuffer[tstart + tlength] != EnPWait)) {
					tlength++;
				}
				status = PWrite(&OutputBuffer[tstart], tlength);
				if (status == 0) {
					status=PBothReady();
				}
				if ((status==0) &&(OutputBuffer[tstart+tlength] == EnPWait)) {
#if DEBUGPCW
					kprintf("PCW: waiting...");
#endif /* DEBUGPCW */
					status = PWait(1, 0);
#if DEBUGPCW
					kprintf("ok\n");
#endif /* DEBUGPCW */
				}
				tstart = tstart + tlength + 1; /* skip the wait character */
				tlength = 0;
			}
		}
	}
	Counter=0;
	if ((status == 0) && EscFlag) {
		/* copy the uncompleted escape for the next InputBuffer */
		while (EscPos < Ilength) {
			InputBuffer[Counter++] = InputBuffer[EscPos++];
		}
	}
	if (status == 0) {
		(ior->io_Actual) = InputLength;
	}
	else {
		(ior->io_Actual) = 0;
	}
	return((UBYTE)status);
}

#define NUMSINGLE	8
#define NUMDOUBLE	11
#define NUMCROSS	8
#define NUMTRIPLE	14
#define NUMMASS		7
#define MAXMASS		7

/*
	Commands with single character escape sequences:
	aRIS	ESCc	reset
	aIND	ESCD	lf
	aNEL	ESCE	return, lf
	aRI		ESCM	reverse lf
	aPLU	ESCL	partial line up
	aPLD	ESCK	partial line down
	aHTS	ESCH	set horiz tab
	aVTS	ESCJ	set vert tab
*/
static char SingleChars[NUMSINGLE]= {'c','D','E','M','L','K','H','J'};

static BYTE SingleComs[NUMSINGLE] =
	{aRIS, aIND, aNEL, aRI, aPLU, aPLD, aHTS, aVTS};

/*
	Commands with 2 character escape sequences:
	aFNT0	ESC(B	US char set
	aFNT1	ESC(R	French char set
	aFNT2	ESC(K	German char set
	aFNT3	ESC(A	UK char set
	aFNT4	ESC(E	Danish I char set
	aFNT5	ESC(H	Sweden char set
	aFNT6	ESC(Y	Italian char set
	aFNT7	ESC(Z	Spanish char set
	aFNT8	ESC(J	Japanese char set
	aFNT9	ESC(6	Norweign char set
	aFNT10	ESC(C	Danish II char set
*/
static char DoubleChars[NUMDOUBLE] = {'B','R','K','A','E','H','Y','Z','J','6','C'};

/*
	Commands with 2 character (1 of which is a '#') escape sequences:
	aRIN	ESC#1	initialize
	aLMS	ESC#9	left margin set
	aRMS	ESC#0	right margin set
	aTMS	ESC#8	top margin set
	aBMS	ESC#2	bottom margin set
	aCAM	ESC#3	clear all margins
	aTBCALL	ESC#4	clear all tabs (horiz and vert)
	aTBSALL	ESC#5	set all tabs
*/
static char CrossChars[NUMCROSS] = {'1','9','0','8','2','3','4','5'};
static char CrossComs[NUMCROSS] =
	{aRIN,aLMS,aRMS,aTMS,aBMS,aCAM,aTBCALL,aTBSALL};

/*
	Commands with 3 (or more ) character escape sequences:
	aSGR0	ESC[0m			normal character set
	aSHORP0	ESC[0w			normal pitch
	aDEN6	ESC[6"z			shadow print on
	aSUS2	ESC[2v			superscript on
	aPROP2	ESC[2p			proportional on
	aJFY5	ESC[5 F			auto left justify
	aTBC0	ESC[0g			clear horiz tab
	aTSS	ESC[n E			set proportional offset
	aSLPP	ESC[nt			set form length n
	aPERF	ESC[nq			perf skip n
	aSTBM	ESC[Pn1;Pn2r	set top and bottom margins
	aSLRM	ESC[Pn1;pn2s	set left and right margins
	aEXTEND	ESC[pn"x		extended commands
	aRAW	ESC[Pn"r		set raw mode for next 'Pn' characters
*/
static char TripleChars[NUMTRIPLE] = {'m','w','z','v','p','F','g','E','t','q','r','s','x','r'};
static BYTE TripleComs[NUMTRIPLE] = {
	aSGR0, aSHORP0, aDEN6, aSUS2, aPROP2, aJFY5, aTBC0,
	aTSS, aSLPP, aPERF, aSTBM, aSLRM, aEXTEND, aRAW
};

/*
	Beats me what this array does!
*/
static BYTE MassArray[NUMMASS][MAXMASS] = {
	{0, 3, 23, 4, 24, 1, 22},	/* aSGR 0, 3, 23, 4, 24, 1, 22 */
	{0, 2, 1, 4, 3, 6, 5},		/* aSHORP 0, 2, 1, 4, 3, 6, 5 */
	{6, 5, 4, 3, 2, 1, -1},		/* aDEN 6, 5, 4, 3, 2, 1, NA */
	{2, 1, 4, 3, 0, -1, -1},	/* aSUS 2, 1, 0, NA, NA, NA, NA */
	{2, 1, 0, -1, -1, -1, -1},	/* aPROP 2, 1, 0, NA, NA, NA, NA */
	{5, 7, 6, 0, 3, 1, -1},		/* aJFY 5, 7, 6, 0, 3, 1, NA */
	{0, 3, 1, 4, -1, -1, -1}	/* aTBC 0, 3, 1, 4, NA, NA, NA */
};

#define INTER_SPACE	1
#define INTER_QUOTE	2

unsigned char InChar;
UBYTE values[10];
int interFlag, Status, RawCount;
BYTE ThisCommand;

ProcessCSI(Length, OutBuffer)
int Length; /* length of string to output */
unsigned char OutBuffer[];
{
	int i, j, p;

	InChar = GetParms(&Position, Length, values, &interFlag);
	ThisCommand = (-1);
	if (((InChar == 'h') || (InChar == 'l')) && (values[0] == 20)) {
		crlfFlag = (InChar & 4) / 4;
	}
	else {
#if DEBUGCSI
		kprintf("PCSI: InChar=%ld, values[0]=%ld, interFlag=%ld\n",
			InChar, values[0], interFlag);
#endif /* DEBUGCSI */
		for (p = 0; p <= ParmCount; p++) {
			for (i = 0; i < NUMTRIPLE; i++) {
#if DEBUGCSI
	kprintf("PCSI: i=%ld, InChar=%ld, values[0]=%ld, interFlag=%ld\n",
					i, InChar, values[0], interFlag);
#endif /* DEBUGCSI */
				if (TripleChars[i] == InChar) {
					if ((interFlag == FALSE) && (i == 2)) {
						/* line spacing */
						ThisCommand = aVERP0 + (values[0] & 1);
						break;
					}
					else if ((interFlag != INTER_SPACE) && (i == 5)) {
						ThisCommand = (-1);
						break;
					}
					else if ((i == 9) && values[0] == 0) {
						/* perf skip off */
						ThisCommand = aPERF0;
						break;
					}
					else if ((interFlag == FALSE) && (i == 10)) {
						/* set top&bot margins */
						ThisCommand = aSTBM;
						break;
					}
					else if ((interFlag == INTER_QUOTE) && (i == 12)) {
						/* extended cmds */
						ThisCommand = aEXTEND;
						break;
					}
					else if ((interFlag == INTER_QUOTE) && (i == 13)) {
						/* set raw mode for next 'RawCount' chars */
						RawCount = values[0];
#if DEBUGR
						kprintf("RawCount=%ld\n", RawCount);
#endif /* DEBUGR */
						ThisCommand = aRAW;
						break;
					}
					else if (i > 6 && i != 10) {
						ThisCommand = TripleComs[i];
						break;
					}
					else if ((i == 0) && (values[0] > 29)) {
						/* set foreground color */
						ThisCommand = aSFC;
						break;
					}
					else if (i < 7) { /* i is 0 to 6 */
						for (j = 0; j < MAXMASS; j++) {
							if (MassArray[i][j] == values[0]) {
								ThisCommand = j + TripleComs[i];
								break;
							}
						}
						if (MassArray[i][j] == values[0]) {
							break;
						}
					}
				}
			}
			/* if cmd is 'set left & right' or 'set top & bottom' margins */
			if ((ThisCommand == aSLRM) || (ThisCommand == aSTBM)) {
				ParmCount = 0;
			}
			if (ThisCommand >= 0) {
				Status = GetCommand(ThisCommand,Length,OutBuffer,values,0);
#if DEBUGCSI
				kprintf("PCSI: Status=%ld\n", Status);
#endif /* DEBUGCSI */
			}
			for (i = 0; i < 9; i++) {
				values[i] = values[i + 1];
			}
			values[9] = 0;
		}
	}
#if DEBUGCSI
	kprintf("PCSI: ThisCommand=%ld\n", ThisCommand);
#endif /* DEBUGCSI */
}

ProcessString(Length, OutBuffer)
int Length; /* length of string to output */
unsigned char OutBuffer[];
{
	int i, temp, version, revision;
	char *s, **b8c;

	Position = 0;
	interFlag = FALSE;
	EscPos = 0;
	ParmCount = 0;
	index = 0;
	Status = 0;
	comLength = 0;
	version = PD->pd_SegmentData->ps_Version;
	revision = PD->pd_SegmentData->ps_Revision;

#if DEBUGPS
	kprintf("PS: length=%ld, RawCount=%ld\n", Length, RawCount);
#endif /* DEBUGPS */
	while ((Position<Length) && ((index + comLength) < P_BUFSIZE) &&
		(Status<=0)) {
		InChar = InputBuffer[Position++]; /* read next character */

		if (RawCount > 0) { /* are we still trapping raw characters? */
			OutBuffer[index++] = InChar; /* yes, save character raw */
			RawCount--; /* one less to do */
		}
		else {
			temp = -1; /* assume no conversion function */
			/* if >= 1.3 and the Conversion Function exists... */
			if (version >= 35 && PED->ped_ConvFunc) {
				/* call the conversion function */
				temp =
					(*PED->ped_ConvFunc)(&OutBuffer[index], InChar, crlfFlag);
			}
			if (temp >= 0) { /* if character was converted or thown away */
				index += temp; /* account for added characters */
			}
			else if (InChar == '\033') { /* got an escape ? */
				/*now see which it is */
				InChar = ReadBuffer(&Position, Length);
				if (InChar == '#' ) { /* one of the private sequences ? */
					ThisCommand = (-1);
					InChar = ReadBuffer(&Position, Length); /* get nxt char */
					for (i = 0; i < 8; i++) {
						if (CrossChars[i] == InChar) { /* on list? */
							ThisCommand = CrossComs[i];
						}
					}
					if (ThisCommand > 0) { /* did we get a command ? */
						/* vars also reset on this */
						if (ThisCommand == aRIN) {
							vline = 0;
							crlfFlag = 0;
						}
						Status =
							GetCommand(ThisCommand,Length,OutBuffer,values,0);
#if DEBUGPS
					kprintf("PS: Status=%ld\n", Status);
#endif /* DEBUGPS */
					}
				}
				else if (InChar == '(') { /* Is it 1 of the FNT commands? */
					ThisCommand = (-1);
					InChar = ReadBuffer(&Position, Length); /* next char */
					for (i = 0; i < 11; i++) {
						if (DoubleChars[i] == InChar) {
							ThisCommand = i;
						}
					}
					if (ThisCommand >= 0) { /* is it a valid command ? */
						ThisCommand += aFNT0; /* select US char set */
						Status =
							GetCommand(ThisCommand,Length,OutBuffer,values,0);
#if DEBUGPS
					kprintf("PS: Status=%ld\n", Status);
#endif /* DEBUGPS */
					}
				}
				else if (InChar == '[') {
					ProcessCSI(Length, OutBuffer);
				}
				else {
					for (i = 0; i < 8; i++) {
						if (InChar == SingleChars[i]) {
							ThisCommand = SingleComs[i];
							if (ThisCommand == aRIS) { /* reset vars on */
								vline = 0; /* printer reset */
								crlfFlag = 0;
							}
							Status =
							GetCommand(ThisCommand,Length,OutBuffer,values,0);
#if DEBUGPS
					kprintf("PS: Status=%ld\n", Status);
#endif /* DEBUGPS */
						}
					}
				}
			}
			/* not an escape */
			/* translate a lf into a cr/lf if on */
			else if (InChar == '\012') {
				if (crlfFlag == 0) {
					ThisCommand = aNEL; /* return, lf */
				}
				else {
					ThisCommand = aIND; /* lf */
				}
				Status = GetCommand(ThisCommand, Length,
					OutBuffer, values, 1);
#if DEBUGPS
				kprintf("PS: Status=%ld\n", Status);
#endif /* DEBUGPS */
				if (Status < 0) {
					/* NEL or IND isn't implemented for this printer */
					if (crlfFlag == 0) {
						if ((index + 2) > P_BUFSIZE) {
							EscPos = Position - 1;
							comLength = 2;
						}
						else {
							OutBuffer[index++] = '\012';
							OutBuffer[index++] = '\015';
						}
					}
					else {
						OutBuffer[index++] = '\012';
					}
				}
			}
			/* not a linefeed */
			else if (InChar == '\233') { /* CSI ? */
				ProcessCSI(Length, OutBuffer);
			}
			/* not a CSI */
			else if (InChar >= 0xa0) { /* extended char ? (ie. > 127) */
				b8c = Default8BitChars; /* assume no 8bit table */
				/* if >= 1.2 and the 8BitChars table exists... */
				if ((version >= 33) && PED->ped_8BitChars) {
					b8c = PED->ped_8BitChars; /* use the 8BitChars table */
				}
				s = b8c[InChar - 0xa0]; /* get ptr to char's string */
				comLength = 0; /* clear length */
				while (*s) { /* null terminated, but ... */
					/* ignore anything after a backslash */
					if (*s++ == '\\') {
						if ((*s >= '0') && (*s <= '9')) {
							for (i = 0; i < 3; i++) {
								if ((*s >= '0') && (*s <= '9')) {
									s++;
								}
								else {
									break;
								}
							}
						}
						else {
							s++;
						}
					}
					comLength++;
				}
				if ((index + comLength) > P_BUFSIZE) {
					EscPos = Position - 1;
				}
				else {
					s = b8c[InChar - 0xa0]; /* get ptr to char's string */
					while (*s) {
						if (*s == '\\') {
							s++;
							if ((*s < '0') || (*s > '9')) {
								OutBuffer[index++] = *s++;
							}
							else {
								temp = 0;
								for(i = 0; i < 3; i++) {
									if ((*s >= '0') && (*s <= '9')) {
										temp = temp * 10 + (*s++) - '0';
									}
									else {
										break;
									}
								}
								OutBuffer[index++] = temp;
							}
						}
						else {
							OutBuffer[index++] = *s++;
						}
					}
				}
			}
			/* not an extended char */
			else { /* must be a regular char */
				OutBuffer[index++] = InChar;
			}
		}
#if DEBUGPS
		if (index + comLength > P_BUFSIZE) {
			kprintf("PS: total=%ld, index=%ld, comLength=%ld\n",
				index + comLength, index, comLength);
		}
#endif /* DEBUGPS */
	}
	/* input buffer backup */
	if ((EscPos == 0) || (Position <= EscPos)) {
		Pos -= (Length - Position);
	}
	else {
		Pos -= (Length - EscPos);
	}
	Counter = 0;
	return(index); /* tell how big the output string is */
}

GetCommand(ThisCommand, Length, OutBuffer, values, callFlag)
UBYTE OutBuffer[];
int Length;
BYTE ThisCommand;
UBYTE values[];
BYTE callFlag; /* whether esc or lf/cf expansion is in progress */
{
	int status, i;
	UWORD point;

#if DEBUGGC
	kprintf("GC: index=%ld, calling translateCommand\n", index);
#endif
	comLength = 0;
	point = ThisCommand;
	status = translateCommand(point, outputBuffer, values);
#if DEBUGGC
	kprintf("GC: status=%ld\n", status);
#endif
	if (status <= 0) {
#if DEBUGGC
		kprintf("GC: status <= 0, returning '%ld'\n", status);
#endif
		return(status);
	}
	if ((index + status) > P_BUFSIZE) {
		comLength = status;
		if (callFlag == 0) {
			FindLastEscape(); /* we were doing real esc */
		}
		else { /* we were adding a cr/lf */
			EscPos = Position - 1;
		}
#if DEBUGGC
		kprintf("GC: (index + status) > P_BUFSIZE, returning 1\n");
#endif
		return(1);
	}
#if DEBUGGC
	kprintf("GC: transferring %ld bytes to %ld\n", status, index);
#endif
	for (i = 0; i < status; i++) {
		OutBuffer[index++] = outputBuffer[i];
	}
#if DEBUGGC
	kprintf("GC: index=%ld, returning 0\n", index);
#endif
	return(0);
}

unsigned char
GetParms(Position, Length, values, flag)
int *Position,Length, *flag;
UBYTE values[];
{
	unsigned char InChar;
	int count;

	count = 0;
	values[0] = 0;
	while ((((InChar=ReadBuffer(Position,Length))>= '0' && InChar <= '9') ||
		(InChar == ' ') || (InChar == ';' ) || (InChar == '\"' ))) {
		if (InChar == ';') {
			values[++count] = 0;
		}
		else if (InChar == ' ') {
			*flag = INTER_SPACE;
		}
		else if (InChar == '\"') {
			*flag = INTER_QUOTE;
		}
		else {
			values[count] = values[count] * 10 + (InChar - '0');
		}
	}
	ParmCount = count;
	return(InChar); /* give the trailing character */
}

unsigned char
ReadBuffer(Position,Length)
int *Position, Length;
{
	unsigned char Char;

	Char = InputBuffer[(*Position)++];
	if ((*Position > Length) || (index >= P_BUFSIZE)) {
		FindLastEscape();
		return('\000');
	}
	return(Char);
}

FindLastEscape()
{
	EscPos = Position - 1; /* where did we leave that escape */
	while ((EscPos > 0) && ((InputBuffer[EscPos] & 0x7f) != '\033')) {
		EscPos--;
	}
	EscFlag = TRUE; /* there's a chance we have a broken esc sequence */
	return(EscPos);
}
