/*
 * The functions in this file
 * negotiate with the operating system
 * for characters, and write characters in
 * a barely buffered fashion on the display.
 * All operating systems.
 */
#include	"ed.h"

#if	AMIGA

#define BLUE 0
#define WHITE 1
#define BLACK 2
#define RED 3

extern void ttflush(); 
extern UBYTE *lineBuffer;
extern int linecounter;

extern struct Window *window;
extern struct MsgPort consoleMsgPort;
extern struct IntuiMessage *message;

extern struct Menu menus[];
extern struct MenuItem macroitems[];
extern struct IntuiText macrotext[];
int ignoreFlag;

extern LONG pcount;
extern struct MinList paste;
extern struct MinNode *pastenode;

#endif

#if	VMS
#include	<stsdef.h>
#include	<ssdef.h>
#include	<descrip.h>
#include	<iodef.h>
#include	<ttdef.h>

#define	NIBUF	128			/* Input  buffer size		*/
#define	NOBUF	1024			/* MM says bug buffers win!	*/
#define	EFN	0			/* Event flag			*/

char	obuf[NOBUF];			/* Output buffer		*/
int	nobuf;				/* # of bytes in above		*/
char	ibuf[NIBUF];			/* Input buffer			*/
int	nibuf;				/* # of bytes in above		*/
int	ibufi;				/* Read index			*/
int	oldmode[2];			/* Old TTY mode bits		*/
int	newmode[2];			/* New TTY mode bits		*/
short	iochan;				/* TTY I/O channel		*/
#endif

#if	CPM
#include	<bdos.h>
#endif

#if	MSDOS
#include	<dos.h>
#endif

#if V7
#undef CTRL
#include	<sgtty.h>		/* for stty/gtty functions */
struct	sgttyb	ostate;			/* saved tty state */
struct	sgttyb	nstate;			/* values for editor mode */
#undef CTRL
#define CTRL    0x0100
#endif


/*
 * This function is called once
 * to set up the terminal device streams.
 * On VMS, it translates SYS$INPUT until it
 * finds the terminal, then assigns a channel to it
 * and sets it raw. On CPM it is a no-op.
 */
void ttopen()
{
#if	VMS
	struct	dsc$descriptor	idsc;
	struct	dsc$descriptor	odsc;
	char	oname[40];
	int	iosb[2];
	int	status;

	odsc.dsc$a_pointer = "SYS$INPUT";
	odsc.dsc$w_length  = strlen(odsc.dsc$a_pointer);
	odsc.dsc$b_dtype   = DSC$K_DTYPE_T;
	odsc.dsc$b_class   = DSC$K_CLASS_S;
	idsc.dsc$b_dtype   = DSC$K_DTYPE_T;
	idsc.dsc$b_class   = DSC$K_CLASS_S;
	do {
		idsc.dsc$a_pointer = odsc.dsc$a_pointer;
		idsc.dsc$w_length  = odsc.dsc$w_length;
		odsc.dsc$a_pointer = &oname[0];
		odsc.dsc$w_length  = sizeof(oname);
		status = LIB$SYS_TRNLOG(&idsc, &odsc.dsc$w_length, &odsc);
		if (status!=SS$_NORMAL && status!=SS$_NOTRAN)
			exit(status);
		if (oname[0] == 0x1B) {
			odsc.dsc$a_pointer += 4;
			odsc.dsc$w_length  -= 4;
		}
	} while (status == SS$_NORMAL);
	status = SYS$ASSIGN(&odsc, &iochan, 0, 0);
	if (status != SS$_NORMAL)
		exit(status);
	status = SYS$QIOW(EFN, iochan, IO$_SENSEMODE, iosb, 0, 0,
			  oldmode, sizeof(oldmode), 0, 0, 0, 0);
	if (status!=SS$_NORMAL || (iosb[0]&0xFFFF)!=SS$_NORMAL)
		exit(status);
	newmode[0] = oldmode[0];
	newmode[1] = oldmode[1] | TT$M_PASSALL | TT$M_NOECHO;
	status = SYS$QIOW(EFN, iochan, IO$_SETMODE, iosb, 0, 0,
			  newmode, sizeof(newmode), 0, 0, 0, 0);
	if (status!=SS$_NORMAL || (iosb[0]&0xFFFF)!=SS$_NORMAL)
		exit(status);
#endif
#if	CPM
#endif
#if	MSDOS
#endif
#if	V7
	gtty(1, &ostate);			/* save old state */
	gtty(1, &nstate);			/* get base of new state */
	nstate.sg_flags |= RAW;
	nstate.sg_flags &= ~(ECHO|CRMOD);	/* no echo for now... */
	stty(1, &nstate);			/* set mode */
#endif

#if AMIGA
if (CDOpen(window) !=0 ) errorExit(10);  /* open the console device */

if( InitKeyMap() == FALSE )errorExit(10);
SetKeyPad();

CDPutStr("[?7l");
#endif
}

/*
 * This function gets called just
 * before we go back home to the command interpreter.
 * On VMS it puts the terminal back in a reasonable state.
 * Another no-operation on CPM.
 */
void ttclose()
{
#if	VMS
	int	status;
	int	iosb[1];

	ttflush();
	status = SYS$QIOW(EFN, iochan, IO$_SETMODE, iosb, 0, 0,
	         oldmode, sizeof(oldmode), 0, 0, 0, 0);
	if (status!=SS$_NORMAL || (iosb[0]&0xFFFF)!=SS$_NORMAL)
		exit(status);
	status = SYS$DASSGN(iochan);
	if (status != SS$_NORMAL)
		exit(status);
#endif
#if	CPM
#endif
#if	MSDOS
#endif
#if	V7
	stty(1, &ostate);
#endif
#if AMIGA
ttflush();

ResetKeyPad();

CDClose();      /* close the console */
#endif
}

/*
 * Write a character to the display.
 * On VMS, terminal output is buffered, and
 * we just put the characters in the big array,
 * after cheching for overflow. On CPM terminal I/O
 * unbuffered, so we just write the byte out.
 * Ditto on MS-DOS (use the very very raw console
 * output routine).
 */
void ttputc(c)
int c;
{

#if	VMS
	if (nobuf >= NOBUF)
		ttflush();
	obuf[nobuf++] = c;
#endif
#if	CPM
	bios(BCONOUT, c, 0);
#endif
#if	MSDOS
	dosb(CONDIO, c, 0);
#endif
#if	V7
	fputc(c, stdout);
#endif
#if	AMIGA
	lineBuffer[linecounter++]=c;
	if(linecounter >= MAXLINE)ttflush();
#endif

}

/*
 * Flush terminal buffer. Does real work
 * where the terminal output is buffered up. A
 * no-operation on systems where byte at a time
 * terminal I/O is done.
 */
void ttflush()
{
#if	VMS
	int	status;
	int	iosb[2];

	status = SS$_NORMAL;
	if (nobuf != 0) {
		status = SYS$QIOW(EFN, iochan, IO$_WRITELBLK|IO$M_NOFORMAT,
			 iosb, 0, 0, obuf, nobuf, 0, 0, 0, 0);
		if (status == SS$_NORMAL)
			status = iosb[0] & 0xFFFF;
		nobuf = 0;
	}
	return (status);
#endif
#if	CPM
#endif
#if	MSDOS
#endif
#if	V7
	fflush(stdout);
#endif
#if	AMIGA
	if(linecounter)CDPutStrL(lineBuffer,linecounter);
	linecounter=0;
#endif
}

void ttcursoroff()
{
#if AMIGA
	ttputc('\x9B');
	ttputc('0');
	ttputc(' ');
	ttputc('p');
#endif
}

void ttcursoron()
{
#if	AMIGA
	ttputc('\x9B');
	ttputc('1');
	ttputc(' ');
	ttputc('p');
#endif
}


/*
 * Read a character from the terminal,
 * performing no editing and doing no echo at all.
 * More complex in VMS that almost anyplace else, which
 * figures. Very simple on CPM, because the system can
 * do exactly what you want.
 */
int ttgetc()
{
#if	VMS
	int	status;
	int	iosb[2];
	int	term[2];

	while (ibufi >= nibuf) {
		ibufi = 0;
		term[0] = 0;
		term[1] = 0;
		status = SYS$QIOW(EFN, iochan, IO$_READLBLK|IO$M_TIMED,
			 iosb, 0, 0, ibuf, NIBUF, 0, term, 0, 0);
		if (status != SS$_NORMAL)
			exit(status);
		status = iosb[0] & 0xFFFF;
		if (status!=SS$_NORMAL && status!=SS$_TIMEOUT)
			exit(status);
		nibuf = (iosb[0]>>16) + (iosb[1]>>16);
		if (nibuf == 0) {
			status = sys$qiow(EFN, iochan, IO$_READLBLK,
				 iosb, 0, 0, ibuf, 1, 0, term, 0, 0);
			if (status != SS$_NORMAL
			|| (status = (iosb[0]&0xFFFF)) != SS$_NORMAL)
				exit(status);
			nibuf = (iosb[0]>>16) + (iosb[1]>>16);
		}
	}
	return (ibuf[ibufi++] & 0xFF);		/* Allow multinational	*/
#endif
#if	CPM
	return (biosb(BCONIN, 0, 0));
#endif
#if	MSDOS
	return (dosb(CONRAW, 0, 0));
#endif
#if	V7
	return(fgetc(stdin));
#endif
#if	AMIGA

#if 0
int inChar;


    inChar=getboth();
    if(inChar == (CTRL|'C'))inChar=3; /* change into control C */
    if(inChar == (-1))inChar = 7; /* cancel command modes */
    return(inChar);

#else
ULONG   WakeUpBit;
int inChar = -1;

ttflush();
while (inChar == (-1)) {
WakeUpBit= Wait((1<<window->UserPort->mp_SigBit)|(1<<consoleMsgPort.mp_SigBit)
	|SIGBREAKF_CTRL_C);

    /* check if it was an intuition message */
if(WakeUpBit & (1<<window->UserPort->mp_SigBit)) {
    while(message=(struct IntuiMessage *)GetMsg(window->UserPort))
	/* if pasting into command line is desired, has to be added here */
	/* in a test of menu state */
	if((message->Class)==CLOSEWINDOW) {
	    ReplyMsg(message);
	    return(3); /* like a control-C */
	}
	else if(((message->Class)==NEWSIZE) || (message->Class==REFRESHWINDOW)) {
			BeginRefresh(window);
			EndRefresh(window,TRUE);
			RefreshWindowFrame(window);
			SetLace(TRUE,4);
			ReplyMsg(message);
			return(0x0D); /* cancel command mode */
	}
	else ReplyMsg(message);
}
if(WakeUpBit & (1<<consoleMsgPort.mp_SigBit)){
     inChar=CDMayGetChar();
     return(inChar);
}}

if(WakeUpBit & SIGBREAKF_CTRL_C)return(3);
#endif
#endif
}

#if	AMIGA

int getboth()
{
int inChar= -1, exitFlag=FALSE;
ULONG   WakeUpBit,ThisOne;

ttflush();
while (exitFlag == FALSE) {
if(!pastenode)WakeUpBit= Wait((1<<window->UserPort->mp_SigBit)|(1<<consoleMsgPort.mp_SigBit)
	|SIGBREAKF_CTRL_C);
else WakeUpBit=NULL;

    /* check if it was an intuition message */
    if(WakeUpBit & (1<<window->UserPort->mp_SigBit)) {
        while(message=(struct IntuiMessage *)GetMsg(window->UserPort)) {
            switch(message->Class) {
                case MENUPICK:
                    if((message->Code) != MENUNULL) {
			ThisOne=message->Code;
			if ((MENUNUM(ThisOne) == 0)&&(ITEMNUM(ThisOne)==11)){
			    ReplyMsg(message);
			    TellAboutEmacs();
			    break;
			}
			else if (((MENUNUM(ThisOne) == 1)&&
			  (ITEMNUM(ThisOne)==16))) {
				CopyClip(0,0);
			        ReplyMsg(message);
				break;
			  }
			else if (((MENUNUM(ThisOne) == 1)&&
			  (ITEMNUM(ThisOne)==17))) {
				StartPaste();
			        ReplyMsg(message);
				break;
			  }
			inChar = -FindCommand(((struct IntuiText *)
				((struct MenuItem *)ItemAddress(menus,ThisOne))
					->ItemFill)->IText);
			if(inChar == 1)inChar = -1;
			exitFlag = TRUE;
		    }
		    ReplyMsg(message);
                    break;

		case MOUSEBUTTONS:
			if(ignoreFlag) ignoreFlag=FALSE;
			else MouseCursor(message);

			ReplyMsg(message);
			break;

		case CLOSEWINDOW:
			exitFlag=TRUE;
			inChar = CTRL|'C';
			ReplyMsg(message);
			break;

		case NEWSIZE:
		case REFRESHWINDOW:
			BeginRefresh(window);
			EndRefresh(window,TRUE);
			RefreshWindowFrame(window);
			SetLace(TRUE,4);
			ReplyMsg(message);
			break;

		case INACTIVEWINDOW:
			ignoreFlag = TRUE;
			ReplyMsg(message);
			inChar = -1;
			break;

                default:
                    ReplyMsg(message);

	    }
	}
    }
    if(WakeUpBit & SIGBREAKF_CTRL_C) {
	exitFlag=TRUE;
	inChar= CTRL|'C';
    }
    if(WakeUpBit & (1<<consoleMsgPort.mp_SigBit)) {
    	if(exitFlag==FALSE) {
	    if((inChar=CDMayGetChar()) != (-1))exitFlag=TRUE;
	}
    }
    /* is there anything to paste ? */
    if(pastenode && (exitFlag==FALSE)) {
	inChar=GetPaste();
	if(inChar)exitFlag=TRUE;
    }
}
return(inChar);
}
#endif
