/*
 * The routines in this file
 * are called to create a subjob running
 * a command interpreter. This code is a big
 * fat nothing on CP/M-86. You lose.
 */
#include	"ed.h"

#if	VMS
#define	EFN	0				/* Event flag.		*/

#include	<ssdef.h>			/* Random headers.	*/
#include	<stsdef.h>
#include	<descrip.h>
#include	<iodef.h>

extern	int	oldmode[];			/* In "termio.c"	*/
extern	int	newmode[];			/* In "termio.c"	*/
extern	short	iochan;				/* In "termio.c"	*/
#endif

#if	MSDOS
#include	<dos.h>
#endif

#if	V7
#include	<signal.h>
#endif

#if	AMIGA

extern struct Screen *screen;
extern struct Window *window;
extern char bufn[];
extern UBYTE screenname[];
extern int version;
#endif

/*
 * Create a subjob with a copy
 * of the command intrepreter in it. When the
 * command interpreter exits, mark the screen as
 * garbage so that you do a full repaint. Bound
 * to "C-C". The message at
 * the start in VMS puts out a newline. Under
 * some (unknown) condition, you don't get one
 * free when DCL starts up.
 */
int spawncli(f, n)
    int f,n;
{
#if	V7
	 char *cp;
	char	*getenv();
#endif
#if	VMS
	movecursor(term.t_nrow, 0);		/* In last line.	*/
	mlputs("[Starting DCL]\r\n");
	(*term.t_flush)();			/* Ignore "ttcol".	*/
	sgarbf = TRUE;
	return (sys(NULL));			/* NULL => DCL.		*/
#endif
#if	CPM
	mlwrite("Not in CP/M-86");
#endif
#if	MSDOS
	movecursor(term.t_nrow, 0);		/* Seek to last line.	*/
	(*term.t_flush)();
	sys("a:command.com", "");		/* Run CLI.		*/
	sgarbf = TRUE;
	return(TRUE);
#endif
#if	V7
	movecursor(term.t_nrow, 0);		/* Seek to last line.	*/
	(*term.t_flush)();
	ttclose();				/* stty to old settings	*/
	if ((cp = getenv("SHELL")) != NULL && *cp != '\0')
		system(cp);
	else
		system("exec /bin/sh");
	sgarbf = TRUE;
	sleep(2);
	ttopen();
	return(TRUE);
#endif
#if	AMIGA
ULONG clifile;
UBYTE openname[80];

	movecursor(term.t_nrow, 0);		/* Seek to last line.	*/
	(*term.t_flush)();
	WindowToBack(window);

	ttclose();				/* stty to old settings	*/
	strcpy(openname,"CON:0/0/640/200/spawn_window");
	if(screen && (version >= 36)) {
	    /* ask window to open on the Emacs screen itself */
	    strcat(openname,"/SCREEN");
	    strcat(openname,screenname);
	}
	else {
	    /* requesters on Workbench while this is active */
	    ((struct Process *)FindTask(NULL))->pr_WindowPtr=0;
	    if(screen)ScreenToBack(screen);
	}
	if(clifile= Open(openname,MODE_NEWFILE)) {
	    Execute("",clifile,0);
	    Close(clifile);
	}
	/* make sure requesters are pointing back at the screen */
	((struct Process *)FindTask(NULL))->pr_WindowPtr=(APTR)window;
	sgarbf = TRUE;
	Delay(50);
	ttopen();
	if(screen)ScreenToFront(screen);
	WindowToFront(window);
	return(TRUE);
#endif
}

/*
 * Run a one-liner in a subjob.
 * When the command returns, wait for a single
 * character to be typed, then mark the screen as
 * garbage so a full repaint is done.
 * Bound to "C-X !".
 */
int spawn(f, n)
    int f,n;
{
	 int	s;
	char		line[NLINE];
#if	MSDOS
	 char	*cp1;
	 char	*cp2;
	char		file[NFILEN];
#endif
#if	AMIGA
ULONG	clifile;
ULONG	tlock;
UBYTE namebuffer[80];
#endif

#if	VMS
	if ((s=mlreply("DCL command: ", line, NLINE)) != TRUE)
		return (s);
	(*term.t_putchar)('\n');		/* Already have '\r'	*/
	(*term.t_flush)();
	s = sys(line);				/* Run the command.	*/
	mlputs("\r\n\n[End]");			/* Pause.		*/
	(*term.t_flush)();
	while ((*term.t_getchar)() != '\r')
		;
	sgarbf = TRUE;
	return (s);
#endif
#if	CPM
	mlwrite("Not in CP/M-86");
	return (FALSE);
#endif
#if	MSDOS
	if ((s=mlreply("MS-DOS command: ", line, NLINE)) != TRUE)
		return (s);
	cp1 = &line[0];				/* Ignore blanks.	*/
	while (*cp1 == ' ')
		++cp1;
	cp2 = &file[0];				/* Command name.	*/
	while (*cp1!=0 && *cp1!=' ') {
		if (cp2 < &file[NFILEN-1])	/* Truncate if long.	*/
			*cp2++ = *cp1;
		++cp1;
	}
	if (cp2 == &file[0])			/* Blank command line.	*/
		return (FALSE);
	cp1[-1] = strlen(cp1);			/* Tail length.		*/
	cp2[ 0] = 0;
	sys(file, cp1-1);
	while ((*term.t_getchar)() != '\r')	/* Pause.		*/
		;
	sgarbf = TRUE;
	return (TRUE);
#endif
#if	V7
	if ((s=mlreply("! ", line, NLINE)) != TRUE)
		return (s);
	(*term.t_putchar)('\n');		/* Already have '\r'	*/
	(*term.t_flush)();
	ttclose();				/* stty to old modes	*/
	system(line);
	sleep(2);
	ttopen();
	mlputs("[End]");			/* Pause.		*/
	(*term.t_flush)();
	while ((s = (*term.t_getchar)()) != '\r' && s != ' ');
	sgarbf = TRUE;
	return (TRUE);
#endif
#if	AMIGA
	if ((s=mlreply("! ", line, NLINE)) != TRUE) return(s);
	(*term.t_flush)();

	tlock=1;	
	if(filexists("ram:t") != (-1)) { /* is there a t directory ? */
		tlock=CreateDir("ram:t");  /* well, make one */
		if(tlock)UnLock(tlock);
	}
	/* build the file name */
	strcpy(namebuffer,"ram:t/");
	strcat(namebuffer,screenname);
	strcat(namebuffer,".SpawnOut");
	clifile=Open(namebuffer,MODE_NEWFILE); /* make temp file */
	if ((clifile) && (tlock )) {
/*	    stdin=clifile;
	    stderr=clifile;
	    stdout=clifile;
*/
	    Execute(line, 0, clifile ); /* do their command */
	    Close(clifile);

	    popupbuffer("spawn.output");

	    curbp->b_flag &= ~BFCHG;	/* mark it as unchanged */
	    curbp->b_flag |= BFTEMP;
	    s=readin(namebuffer); /* read temp file into buffer */
	    if(filexists(namebuffer))
		DeleteFile(namebuffer); /* delete temp file */
	    mfilename(""); /* blank out the filename */
	    nextwind(0,0);
	}
	else s= FALSE; /* failed for some reason */
	(*term.t_flush)();
	sgarbf = TRUE; /* I wonder what the screen looks like now */
	return (s);
#endif
}

#if	VMS
/*
 * Run a command. The "cmd" is a pointer
 * to a command string, or NULL if you want to run
 * a copy of DCL in the subjob (this is how the standard
 * routine LIB$SPAWN works. You have to do wierd stuff
 * with the terminal on the way in and the way out,
 * because DCL does not want the channel to be
 * in raw mode.
 */
int sys(cmd)
 char	*cmd;
{
	struct	dsc$descriptor	cdsc;
	struct	dsc$descriptor	*cdscp;
	long	status;
	long	substatus;
	long	iosb[2];

	status = SYS$QIOW(EFN, iochan, IO$_SETMODE, iosb, 0, 0,
			  oldmode, sizeof(oldmode), 0, 0, 0, 0);
	if (status!=SS$_NORMAL || (iosb[0]&0xFFFF)!=SS$_NORMAL)
		return (FALSE);
	cdscp = NULL;				/* Assume DCL.		*/
	if (cmd != NULL) {			/* Build descriptor.	*/
		cdsc.dsc$a_pointer = cmd;
		cdsc.dsc$w_length  = strlen(cmd);
		cdsc.dsc$b_dtype   = DSC$K_DTYPE_T;
		cdsc.dsc$b_class   = DSC$K_CLASS_S;
		cdscp = &cdsc;
	}
	status = LIB$SPAWN(cdscp, 0, 0, 0, 0, 0, &substatus, 0, 0, 0);
	if (status != SS$_NORMAL)
		substatus = status;
	status = SYS$QIOW(EFN, iochan, IO$_SETMODE, iosb, 0, 0,
			  newmode, sizeof(newmode), 0, 0, 0, 0);
	if (status!=SS$_NORMAL || (iosb[0]&0xFFFF)!=SS$_NORMAL)
		return (FALSE);
	if ((substatus&STS$M_SUCCESS) == 0)	/* Command failed.	*/
		return (FALSE);
	return (TRUE);
}
#endif

#if	MSDOS
/*
 * This routine, once again
 * by Bob McNamara, is a C translation
 * of the "system" routine in the MWC-86 run
 * time library. It differs from the "system" routine
 * in that it does not unconditionally append the
 * string ".exe" to the end of the command name.
 * We needed to do this because we want to be
 * able to spawn off "command.com". We really do
 * not understand what it does, but if you don't
 * do it exactly "malloc" starts doing very
 * very strange things.
 */
sys(cmd, tail)
char	*cmd;
char	*tail;
{
	 unsigned n;
	extern   char	  *__end;

	n = __end + 15;
	n >>= 4;
	n = ((n + dsreg() + 16) & 0xFFF0) + 16;
	return(execall(cmd, tail, n));
}
#endif
