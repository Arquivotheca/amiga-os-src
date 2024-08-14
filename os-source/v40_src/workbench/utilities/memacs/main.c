/*
 * This file contains the
 * main driving routine, and some
 * keyboard processing code, for the
 * MicroEMACS screen editor.
 */
#include	"ed.h"
#if AMIGA
#include "workbench/startup.h"
LONG   OrigDir;
struct WBArg *Arg;

extern struct WBStartup *WBenchMsg;
extern struct Window *window;
extern struct MsgPort consoleMsgPort;

/*#include "clib/macros.h"*/


#define MAX(a,b)    ((a)>(b)?(a):(b))
#define MIN(a,b)    ((a)<(b)?(a):(b))
#define ABS(x)      ((x<0)?(-(x)):(x))

#endif

/*
#if	VMS
#include	<ssdef.h>
#define	GOOD	(SS$_NORMAL)
#endif
*/

#ifndef	GOOD
#define	GOOD	0
#endif

int	currow;				/* Working cursor row		*/
int	curcol;				/* Working cursor column	*/
int	thisflag;			/* Flags, this command		*/
int	lastflag;			/* Flags, last command		*/
int	curgoal;			/* Goal column			*/
int	fillcol;			/* Current fill column		*/
int	LeftMargin;			/* default left margin 		*/
int	RightMargin;			/* default right margin 	*/
int	FontWidth;
int	FontHeight;
int	CheckCase;			/* should we check the case 	*/
int gotoLine;				/* use for auto goto option	*/
BUFFER	*curbp;				/* Current buffer		*/
EWINDOW	*curwp;				/* Current window		*/
BUFFER	*bheadp;			/* BUFFER listhead		*/
EWINDOW	*wheadp;			/* EWINDOW listhead		*/
BUFFER	*blistp;			/* Buffer list BUFFER		*/
UWORD	kbdm[NKBDM] = {CTLX|')'};	/* Macro			*/
UWORD	*kbdmip;			/* Input  for above		*/
UWORD	*kbdmop;			/* Output for above		*/
UBYTE	pat[NPAT];			/* Search Pattern		*/
UBYTE	rpat[NPAT];			/* Replace Pattern		*/
int	QueryFlag;
int	MacroLevel;
int     BackupBefore;
UBYTE	*fkdmip;			/* Input for fkm		*/
UBYTE	*fkdmop;			/* Output for fkm		*/
UBYTE   *cmdmop;			/* Output for command */
int	fkinflag;			/* Allow input during function keys */
int 	nkeytab;			/* allow other files into the commands */
int	tabsize;
int	indentsize;
int	nkeybound;
long opts[OPT_COUNT];
struct RDargs *rdargs;
int version;
UBYTE lastname[80];
UBYTE lastline[25];
int RememberFile;

typedef struct  {
        short   k_code;                 /* Key code                     */
        int     (*k_fp)(int f, int n);  /* Routine to handle it         */
        struct IntuiText  *k_string;
}       KEYTAB;
extern KEYTAB keytab[];

int main(argc, argv)
int	argc;
char	*argv[];
{
	int	c;
	int	f;
	int	n;
	int menuflag;
	char bname[NBUFN];
	char tname[NBUFN];
	int i;
	BUFFER *bp;
#if AMIGA
	char **argptr;
	struct Process *process;
#endif

initKeys();	/* set the key table number */
gotoLine=0;

#if AMIGA
version=(int)((struct Library *)DOSBase)->lib_Version;
memset((UBYTE *)opts,0,sizeof(opts));
memset(&lastname,0,79);
aStart(argc,argv); /* set arguments and constants */
#else
RightMargin=term.t_ncol;
#endif

LeftMargin=0;
BackupBefore=1;
CheckCase=0;
QueryFlag=FALSE;
fkinflag=FALSE;
tabsize=0;
indentsize=4;
nkeybound=0;
strcpy(bname, "main");			/* Work out the name of	*/
						/* default buffer */
#if AMIGA
	if (argc==0) {		/* workbench startup */
		Arg=(WBenchMsg->sm_ArgList);
		if (WBenchMsg->sm_NumArgs > 1) {
			Arg++;
			makename(bname, Arg->wa_Name);
		}
	}
	else {
	    if(version >= 36) {
	        if(opts[OPT_FROM]) {
		    if(argptr=(char **)opts[OPT_FROM]) {
		        makename(bname,(UBYTE *)*argptr);
		    }
		}
		else {
		    if(GetVar("memacs_file",lastname,79,GVF_LOCAL_ONLY)) {
		    	if(strlen(lastname))makename(bname, lastname);
		        if(GetVar("memacs_line",lastline,24,GVF_LOCAL_ONLY))
			    gotoLine=atoi(lastline);	
		    }
		}
	    }
	   else if (argc>1) {
		/* if its not just an opt */
		if((argc != 3) || stricmp(argv[1],"OPT")) {
		    makename(bname, argv[1]);
		}
	   }
	}
#else
	if ((argc>1) && (*argv[1]))makename(bname, argv[1]);
#endif

 	vtinit();				/* Displays.		*/
	edinit(bname);				/* Buffers, windows.	*/
	update();

#if AMIGA
	if (!argc) {
		if (WBenchMsg->sm_NumArgs > 1) {
	  	    OrigDir=CurrentDir(Arg->wa_Lock);
		    readin(Arg->wa_Name);
		}
	}
	else {
	    if(version >= 36) {
	    	if(opts[OPT_FROM]) {
		    strcpy(lastname,(UBYTE *)*argptr);
		    readin(lastname);
		/* now handle the rest of the arguments */
		    i=0;
		    while (*++argptr) {
		        makename(tname,(UBYTE *)*argptr);
		        bp=bfind((UBYTE *)tname, TRUE,0);
			if(bp) {
			    if(i++ < 3) {
				splitwind(0,0);
				nextwind(0,0);
			    }
			    if(musebuffer(tname))readin(*argptr);
			}
		    }
		    nextwind(0,0);
		}
		else {
		    /* already version >= 36 */
		    if(lastname)readin(lastname);
		}
	    }
	    else if(argc>1) {
		if((argc != 3) || stricmp(argv[1],"OPT")) readin(argv[1]);
	    }
	}
#else
	if((argc>1) && (*argv[1]))readin(argv[1]);
#endif
	lastflag = 0;				/* Fake last flags.	*/
	MacroLevel=0;				/* do startup 		*/
	MacroXe(FALSE,1);			/* in function key null	*/
#if AMIGA
	process = (struct Process *)FindTask(NULL); /* reset error window */
	/* no error requesters for a minute */
	process->pr_WindowPtr= (APTR)-1; 
#endif
/*	if(!(ExecuteFile(TRUE,1,"emacs_pro"))) */
	ExecuteFile(TRUE,1,"S:emacs_pro"); /* quietly */
#if AMIGA
	process->pr_WindowPtr=(APTR)window;   /* errors back on */
#endif

	if(gotoLine) {	/* command line goto logic */
	    gotobob(0,0);
	    forwline(TRUE,gotoLine);
	}
loop:
	menuflag=FALSE;			/* assume keyboard */
	update();			/* Fix up the screen	*/
	c = getkey();
	if(c < 0 ) {
	    menuflag=TRUE;
	    c= -c;
	}
	if (mpresf != FALSE) {
		mlerase();
		update();
		if (c == ' ')	/* ITS EMACS does this	*/
			goto loop;
	}

	f = FALSE; /* defaults */
	n = 1;
	if (c == (CTRL|'U')) {
    	    f = TRUE; 
	    n=ctlu(&c);
	}
	if (c == (CTRL|'X')) c = CTLX | getctl(); /* ^X is a prefix	*/

	if (kbdmip != NULL) {			/* Save macro strokes.	*/
		if (c!=(CTLX|')') && kbdmip>&kbdm[NKBDM-6]) {
			ctrlg(FALSE, 0);
			goto loop;
		}
		if (f != FALSE) {
			*kbdmip++ = (CTRL|'U');
			*kbdmip++ = n;
		}
		*kbdmip++ = c;
	}
	execute(c, f, n, menuflag);		/* Do it.		*/
	goto loop;
}

/*
 * Initialize all of the buffers
 * and windows. The buffer name is passed down as
 * an argument, because the main routine may have been
 * told to read in a file by default, and we want the
 * buffer name to be right.
 */
void edinit(bname)
char	bname[];
{
	 BUFFER	*bp;
	 EWINDOW	*wp;
	bp = bfind(bname, TRUE, 0);		/* First buffer		*/
	blistp = bfind("[List]", TRUE, BFTEMP);	/* Buffer list buffer	*/
	wp = (EWINDOW *) malloc(sizeof(EWINDOW)); /* First window */
	if (bp==NULL || wp==NULL || blistp==NULL) {
		abort(); /* failed */
	}
	curbp  = bp;				/* Make this current	*/
	wheadp = wp;
	curwp  = wp;
	wp->w_wndp  = NULL;			/* Initialize window	*/
	wp->w_bufp  = bp;
	bp->b_nwnd  = 1;			/* Displayed.		*/
	wp->w_linep = bp->b_linep;
	wp->w_dotp  = bp->b_linep;
	wp->w_doto  = 0;
	wp->w_markp = NULL;
	wp->w_marko = 0;
	wp->w_toprow = 0;
	wp->w_ntrows = term.t_nrow-1;		/* "-1" for mode line.	*/
	wp->w_force = 0;
	wp->w_flag  = WFMODE|WFHARD;		/* Full.		*/
}

void edfree()
{
	BUFFER *bp=bheadp;
	EWINDOW *wp=wheadp;
	BUFFER *bp1;
	EWINDOW *wp1;
	LINE *lp,*lp1;
	while(bp) {
		lp= bp->b_linep->l_fp;
		while (lp != bp->b_linep) {
			lp1=lp->l_fp;
		        free((char *)lp);
			lp=lp1;
		}
		free((char *)lp);
		bp1=bp->b_bufp;
		free((char *) bp);
	        bp=bp1;
	}

	while(wp) {
		wp1=wp->w_wndp;
		free((char *) wp);
	        wp=wp1;
	}
}

/*
 * This is the general command execution
 * routine. It handles the fake binding of all the
 * keys to "self-insert". It also clears out the "thisflag"
 * word, and arranges to move it to the "lastflag", so that
 * the next command can look at it. Return the status of
 * command.
 */
int execute(c, f, n, flag)
int c,f,n,flag;
{
	 KEYTAB *ktp;
	 int	status;
         int tfillcol;

	if(!flag)c=dobindkey(c);		/* if flag skip bindings */
	ktp = &keytab[0];			/* Look in key table.	*/
	while (ktp < &keytab[nkeytab]) {
		if (ktp->k_code == c) {
			thisflag = 0;
			status   = (*ktp->k_fp)(f, n);
			lastflag = thisflag;
			return (status);
		}
		++ktp;
	}

	/*
	 * If a space was typed, fill column is defined, the argument is non-
	 * negative, and we are now past fill column, perform word wrap.
	*/
	tfillcol = (curbp->b_mode&WRAPMODE) ? fillcol : 0;
/* ???? was fillcol in next line, prob should be tfillcol */
	if (c == ' ' && tfillcol > 0 && n>=0 && getccol(FALSE) > tfillcol)
		wrapword(FALSE,0);

	if ((c>=0x20 && c<=0x7E)		/* Self inserting.	*/
	||  (c>=0xA0 && c<=0xFE)) {
		if (n <= 0) {			/* Fenceposts.		*/
			lastflag = 0;
			return (n<0 ? FALSE : TRUE);
		}
		thisflag = 0;			/* For the future.	*/
		status   = linsert(n, c);
		lastflag = thisflag;

		/* add CMODE check */
		if(curbp->b_mode & CMODE) {
		    if ((c == '}'||c==')'||c==']')) fencematch(c);
		}
		if(!status)status=lnewline();
		return (status);
	}
	lastflag = 0;				/* Fake last flags.	*/
	return (FALSE);
}

/*
 * Read in a key.
 * Do the standard keyboard preprocessing.
 * Convert the keys to the internal character set. On
 * the LK201, which lacks a reasonable ESC key, make the
 * grave accent a meta key too; this is a fairly common
 * customization around Digital. Also read and decode
 * the arrow keys, and other special keys. This is
 * done in Rainbow mode; does this work on all
 * the terminals with LK201 keyboards?
 */
int getkey()
{
	 int	c;
#if	LK201
	 int	n;
loop:
	c = (*term.t_getchar)();
	if (c == AGRAVE) {			/* Alternate M- prefix.	*/
		c = getctl();
		return (META | c);
	}
	if (c == METACH) {			/* M-, or special key.	*/
		c = (*term.t_getchar)();
		if (c == '[') {			/* Arrows and extras.	*/
			c = (*term.t_getchar)();
			if (c == 'A')
				return (CTRL | 'P');
			if (c == 'B')
				return (CTRL | 'N');
			if (c == 'C')
				return (CTRL | 'F');
			if (c == 'D')
				return (CTRL | 'B');
			if (c>='0' && c<='9') {
				n = 0;
				do {
					n = 10*n + c - '0';
					c = (*term.t_getchar)();
				} while (c>='0' && c<='9');
				if (c=='~' && n<=34 && (c=lkmap[n])!=0)
					return (c);
			}
			goto loop;
		}
		if (c == 'O') {
			c = (*term.t_getchar)();
			if (c == 'P')		/* PF1 => M-X (Future)	*/
				return (META | 'X');
			if (c == 'Q')		/* PF2 => C-Q		*/
				return (CTRL | 'Q');
			if (c == 'R')		/* PF3 => C-S		*/
				return (CTRL | 'S');
			if (c == 'S')		/* PF4 => C-R		*/
				return (CTRL | 'R');
			goto loop;
		}
		if (c>='a' && c<='z')		/* Force to upper	*/
			c -= 0x20;
		if (c>=0x00 && c<=0x1F)		/* C0 control -> C-	*/
			c = CTRL | (c+'@');
		return (META | c);
	}
#endif
#if	VT100
loop:
	c = (*term.t_getchar)();
	if (c == METACH) {			/* Apply M- prefix	*/
		c = (*term.t_getchar)();
		if (c == '[') {			/* Arrow keys.		*/
			c = (*term.t_getchar)();
			if (c == 'A')
				return (CTRL | 'P');
			if (c == 'B')
				return (CTRL | 'N');
			if (c == 'C')
				return (CTRL | 'F');
			if (c == 'D')
				return (CTRL | 'B');
			goto loop;
		}
		if (c == 'O') {
			c = (*term.t_getchar)();
			if (c == 'P')		/* PF1 => M-X (Future)	*/
				return (META | 'X');
			if (c == 'Q')		/* PF2 => C-Q		*/
				return (CTRL | 'Q');
			if (c == 'R')		/* PF3 => C-S		*/
				return (CTRL | 'S');
			if (c == 'S')		/* PF4 => C-R		*/
				return (CTRL | 'R');
			goto loop;
		}
		if (c>='a' && c<='z')		/* Force to upper	*/
			c -= 0x20;
		if (c>=0x00 && c<=0x1F)		/* C0 control -> C-	*/
			c = CTRL | (c+'@');
		return (META | c);
	}
#endif

#if	AMIGA
	 int secondC;
loop:
	c=getboth();
	if (c <0)return(c); /* intuition message */
	if (c == METACH8) {			/* Apply M- prefix	*/
		c = (*term.t_getchar)(); /* arrow keys */
			if (c == 'A')
				return (CTRL | 'P');
			if (c == 'B')
				return (CTRL | 'N');
			if (c == 'C')
				return (CTRL | 'F');
			if (c == 'D')
				return (CTRL | 'B');
			if (c == 'T') /* shifted up */
				return (META | ',' );
			if (c == 'S') /* shifted down */
				return (META | '.' );
			if (c == ' ') {
				 c = (*term.t_getchar)(); /* another char */
				if (c == 'A') return( CTRL | 'A'); /* l */
				if (c == '@') return (CTRL | 'E'); /* r */
			}
		if (((c >= '#') && (c <= '9')) || (c== '?')) {
		    if ((secondC=(*term.t_getchar)()) == '~') { /* function key */
			if (c == '?') MacroLevel=11;
			else if ((c >= ' ') && (c <= '/'))
				MacroLevel = c - '#' + 12;
			else MacroLevel = c - '0' + 1;
			return (META | '~');
		    }
		    else if ((secondC >= '0') && (secondC <= '9') && (c == '1')) {
		        secondC = secondC-'0'+25;
			if ( (*term.t_getchar)() == '~') {
				MacroLevel = secondC;
				return(META | '~');
			}
		    }
		}
	}
	else if (c == METACH) {
		c= (*term.t_getchar)();
		if (c>='a' && c<='z')		/* Force to upper	*/
			c -= 0x20;
		if (c>=0x00 && c<=0x1F)		/* C0 control -> C-	*/
			c = CTRL | (c+'@');
		return (META|c);
	}
#endif
#if	!LK201 && !VT100 && !AMIGA
	c = (*term.t_getchar)();
	if (c == METACH) {			/* Apply M- prefix	*/
		c = getctl();
		return (META | c);
	}
#endif

	if (c == CTRLCH) {			/* Apply C- prefix	*/
		c = getctl();
		return (CTRL|c);
	}
	if (c == CTMECH) {			/* Apply C-M- prefix	*/
		c = getctl();
		return (CTRL | META | c);
	}
	if (c>=0x00 && c<=0x1F)			/* C0 control -> C-	*/
		c = CTRL | (c+'@');
	return (c);
}

/*
 * Get a key.
 * Apply control modifications
 * to the read key.
 */
int getctl()
{
	 int	c;

	c = (*term.t_getchar)();
	if (c>='a' && c<='z')			/* Force to upper	*/
		c -= 0x20;
	if (c>=0x00 && c<=0x1F)			/* C0 control -> C-	*/
		c = CTRL | (c+'@');
	return (c);
}

/*
 * Fancy quit command, as implemented
 * by Norm. Write modified buffers, and
 * exit emacs.
 */

void quickexit(f, n)
    int f,n;
{
#if	AMIGA
if(filemodsave(f, n)==FALSE)Delay(200);
#else
filemodsave(f, n);
#endif
quit(f, n);	/* conditionally quit	*/
}

/*
 * Quit command. If an argument, always
 * quit. Otherwise confirm if a buffer has been
 * changed and not written out. Normally bound
 * to "C-X C-C".
 */
int quit(f, n)
    int f,n;
{
	 int	s;

kbdmop=NULL;  /* end macro execution */
fkdmop=NULL;
cmdmop=NULL;

	if (f != FALSE				/* Argument forces it.	*/
	|| anycb() == FALSE			/* All buffers clean.	*/
	|| (s=mlyesno("Modified buffers exist, do you really want to exit?"))
	== TRUE) {	/* User says its OK.	*/
		vttidy();
		vtfree();
#if	AMIGA
	    if((version >= 36) && (RememberFile)) {
		/* save file name and line number */
		if(strlen(curbp->b_fname)) {
		    strcpy(lastname,curbp->b_fname);
		    SetVar("memacs_file",lastname,strlen(lastname),GVF_LOCAL_ONLY);	
		    itoa(lastline,5,getcline());
		    SetVar("memacs_line",lastline,strlen(lastline),GVF_LOCAL_ONLY);	
		}
	    }
		edfree();
		Cleanup();
#endif
		exit(GOOD);
	}
	return (s);
}

/*
 * Begin a keyboard macro.
 * Error if not at the top level
 * in keyboard processing. Set up
 * variables and return.
 */
int ctlxlp(f, n)
    int f,n;
{
	if (kbdmip!=NULL || kbdmop!=NULL) {
		mlwrite("Already remembering!");
		return (FALSE);
	}
	mlwrite("[Start macro]");
	kbdmip = &kbdm[0];
	return (TRUE);
}

/*
 * End keyboard macro. Check for
 * the same limit conditions as the
 * above routine. Set up the variables
 * and return to the caller.
 */
int ctlxrp(f, n)
    int f,n;
{
	if (kbdmip == NULL) {
/*		mlwrite("Nothing to remember!");*/
		return (FALSE);
	}
	mlwrite("[End macro]");
	kbdmip = NULL;
	return (TRUE);
}

/*
 * Execute a macro.
 * The command argument is the
 * number of times to loop. Quit as
 * soon as a command gets an error.
 * Return TRUE if all ok, else
 * FALSE.
 */
int ctlxe(f, n)
    int f,n;
{
	 int	c;
	 int	af;
	 int	an;
	 int	s;

	if (kbdmip!=NULL || kbdmop!=NULL) {
		mlwrite("Already doing a macro!");
		return (FALSE);
	}
	if (n <= 0) 
		return (TRUE);
	do {
		kbdmop = &kbdm[0];
		do {
			af = FALSE;
			an = 1;
			if ((c = *kbdmop++) == (CTRL|'U')) {
				af = TRUE;
				an = *kbdmop++;
				c  = *kbdmop++;
			}
			s = TRUE;
		} while (c != (CTLX|')') && (s=execute(c, af, an, FALSE))==TRUE);
		kbdmop = NULL;
	} while (s==TRUE && --n);
	return (s);
}

/*
 * Abort.
 * Beep the beeper.
 * Kill off any keyboard macro,
 * etc., that is in progress.
 * Sometimes called as a routine,
 * to do general aborting of
 * stuff.
 */
int ctrlg(f, n)
int f,n;
{
	(*term.t_beep)();
	if (kbdmip != NULL) {
		kbdm[0] = (CTLX|')');
		kbdmip  = NULL;
	}
	return (ABORT);
}

int ctlu(ch)
int *ch;		/* current argument character */
{
int mflag=0,n=4;
ULONG c;
			/* start argument with argument of 4 */
    mlwrite("Arg: 4"); 	/* that can be discarded */

    while ( ((c=getkey()) >='0') && c<='9' || c== *ch || c=='-'){
	if (c == *ch) n = n*4;
	 /* If dash, and start of argument string, set arg.
	 	to -1.  Otherwise, insert it. */

	else if (c == '-') {
	    if (mflag) break;
	    n = 0;
	    mflag = -1;
	}
	/* If first digit entered, replace previous argument
	   with digit and set sign.  Otherwise, append to arg. */
	else {
	    if (!mflag) {
		n = 0;
		mflag = 1;
	    }
	    n = 10*n + c - '0';
	}
        mlwrite("Arg: %d", (mflag >=0) ? n : (n ? -n : -1));
    }
    /* Make arguments preceded by a minus sign negative and change
       the special argument "^U -" to an effective "^U -1". */
    if (mflag == -1) {
	if (n == 0) n++;
	n = -n;
    }
*ch = c;	/* pass back next character */
return(n);
}
