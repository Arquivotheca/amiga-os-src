/*
 * The functions in this file
 * handle redisplay. There are two halves,
 * the ones that update the virtual display
 * screen, and the ones that make the physical
 * display screen the same as the virtual
 * display screen. These functions use hints
 * that are left in the windows by the
 * commands.
 */
#include	"ed.h"

#define	WFDEBUG	0			/* Window flag debug.		*/

typedef	struct	VIDEO {
	short	v_flag;			/* Flags			*/
	char	v_text[1];		/* Screen data.			*/
}	VIDEO;

#define	VFCHG	0x0001			/* Changed.			*/

int	sgarbf	= TRUE;			/* TRUE if screen is garbage	*/
int	mpresf	= FALSE;		/* TRUE if message in last line	*/
int	vtrow	= 0;			/* Row location of SW cursor	*/
int	vtcol	= 0;			/* Column location of SW cursor	*/
int	ttrow	= HUGE;			/* Row location of HW cursor	*/
int	ttcol	= HUGE;			/* Column location of HW cursor	*/

VIDEO	**vscreen;			/* Virtual screen.		*/
VIDEO	**pscreen;			/* Physical screen.		*/

/*
 * Initialize the data structures used
 * by the display code. The edge vectors used
 * to access the screens are set up. The operating
 * system terminal I/O channel is set up. All the
 * other things get initialized at compile time.
 * The original window has "WFCHG" set, so that it
 * will get completely redrawn on the first
 * call to "update".
 */
void vtinit()
{
	 int	i;
	 VIDEO	*vp;

	(*term.t_open)();
	vscreen = (VIDEO **) malloc(term.t_nrow*sizeof(VIDEO *));
	if (vscreen == NULL) abort();
	for (i=0; i<term.t_nrow; ++i) {
		vp = (VIDEO *) malloc(sizeof(VIDEO)+term.t_ncol+8);
		if (vp == NULL) abort();
		vscreen[i] = vp;
	}

	pscreen = (VIDEO **) malloc(term.t_nrow*sizeof(VIDEO *));
	if (pscreen == NULL) abort();
	for (i=0; i<term.t_nrow; ++i) {
		vp = (VIDEO *) malloc(sizeof(VIDEO)+term.t_ncol+8);
		if (vp == NULL)abort();
		pscreen[i] = vp;
	}
}

/* free up virtual terminal memory */
void vtfree()
{
 int i;

if(vscreen) {
        for (i=0; i<term.t_nrow; ++i)free((UBYTE *)vscreen[i]);
	free((UBYTE *)vscreen);
}

if(pscreen)
        for (i=0; i<term.t_nrow; ++i)free((UBYTE *)pscreen[i]);
	free((UBYTE *)pscreen);
}

/*
 * Clean up the virtual terminal
 * system, in anticipation for a return to the
 * operating system. Move down to the last line and
 * clear it out (the next system prompt will be
 * written in the line). Shut down the channel
 * to the terminal.
 */
void vttidy()
{
	movecursor(term.t_nrow, 0);
	(*term.t_eeol)();
	(*term.t_close)();
}


/*
 * Set the virtual cursor to
 * the specified row and column on the
 * virtual screen. There is no checking for
 * nonsense values; this might be a good
 * idea during the early stages.
 */
void vtmove(row, col)
int row,col;
{
	vtrow = row;
	vtcol = col;
}

/*
 * Write a character to the
 * virtual screen. The virtual row and
 * column are updated. If the line is too
 * long put a "$" in the last column.
 * This routine only puts printing characters
 * into the virtual terminal buffers.
 * Only column overflow is checked.
 */
void vtputc(c)
 int	c;
{
	 VIDEO	*vp;

	vp = vscreen[vtrow];
	if (vtcol >= RightMargin) vp->v_text[RightMargin-1] = '$';
	else if (c == '\t') {
		do {
		    vtputc(' ');
		}   while ( ((vtcol&0x07) != 0) && (vtcol < RightMargin));
	}
	else if (c<0x20 || c == 0x7F) {
		vtputc('^');
		vtputc(c ^ 0x40);
	} 
	else vp->v_text[vtcol++] = c;		
}
/*
 * Write a character to the
 * virtual screen. The virtual row and
 * column are updated. If the line is too
 * long put a "$" in the last column.
 * Only column overflow is checked.
 */
void vtzapc(c)
 int	c;
{
	 VIDEO	*vp;

	vp = vscreen[vtrow];
	if (vtcol >= term.t_ncol)  vp->v_text[term.t_ncol-1] = '$';
	else if (c == '\t') {
		do {
		    vtputc(' ');
		}   while ( ((vtcol&0x07) != 0) && (vtcol < term.t_ncol));
	}
	else vp->v_text[vtcol++] = c;		
}

/*
 * Erase from the end of the
 * software cursor to the end of the
 * line on which the software cursor is
 * located.
 */
void vteeol()
{
	 VIDEO	*vp;

	vp = vscreen[vtrow];
	while (vtcol < term.t_ncol)
		vp->v_text[vtcol++] = ' ';
}

/*
 * Make sure that the display is
 * right. This is a three part process. First,
 * scan through all of the windows looking for dirty
 * ones. Check the framing, and refresh the screen.
 * Second, make sure that "currow" and "curcol" are
 * correct for the current window. Third, make the
 * virtual and physical screens the same.
 */
void update()
{
	 LINE	*lp;
	 EWINDOW	*wp;
	 VIDEO	*vp1;
	 VIDEO	*vp2;
	 int	i;
	 int	j;
	 int	c;
	ttcursoroff();
	wp = wheadp;
	while (wp != NULL) {
	/* Look at any window with update flags set on.		*/
		if (wp->w_flag != 0) {
			/* If not force reframe, check the framing.	*/
			if ((wp->w_flag&WFFORCE) == 0) {
				lp = wp->w_linep;
				for (i=0; i<wp->w_ntrows; ++i) {
					if (lp == wp->w_dotp) goto out;
					if (lp == wp->w_bufp->b_linep) break;
					lp = lforw(lp);
				}
			}
			/* Not acceptable, better compute a new value	*/
			/* for the line at the top of the window. Then	*/
			/* set the "WFHARD" flag to force full redraw.	*/
			i = wp->w_force;
			if (i > 0) {
				--i;
				if (i >= wp->w_ntrows) i = wp->w_ntrows-1;
			} 
			else if (i < 0) {
				i += wp->w_ntrows;
				if (i < 0) i = 0;
			} 
			else i = wp->w_ntrows/2;
			lp = wp->w_dotp;
			while (i!=0 && lback(lp)!=wp->w_bufp->b_linep) {
				--i;
				lp = lback(lp);
			}
			wp->w_linep = lp;
			wp->w_flag |= WFHARD;	/* Force full.		*/
		out:
			/* Try to use reduced update. Mode line update	*/
			/* has its own special flag. The fast update is	*/
			/* used if the only thing to do is within the	*/
			/* line editing.				*/
			lp = wp->w_linep;
			i  = wp->w_toprow;
			if ((wp->w_flag&~WFMODE) == WFEDIT) {
				while (lp != wp->w_dotp) {
					++i;
					lp = lforw(lp);
				}
				vscreen[i]->v_flag |= VFCHG;
				vtmove(i, 0);
				for (j=0; j<llength(lp); ++j)
					vtputc(lgetc(lp, j));
				vteeol();
			}
	 		else if ((wp->w_flag&(WFEDIT|WFHARD)) != 0) {
				while (i < wp->w_toprow+wp->w_ntrows) {
					vscreen[i]->v_flag |= VFCHG;
					vtmove(i, 0);
					if (lp != wp->w_bufp->b_linep) {
						for (j=0; j<llength(lp); ++j)
							vtputc(lgetc(lp, j));
						lp = lforw(lp);
					}
					vteeol();
					++i;
				}
			}
#if	!WFDEBUG
			if ((wp->w_flag&WFMODE) != 0)
				modeline(wp);
			wp->w_flag  = 0;
			wp->w_force = 0;
#endif
		}		
#if	WFDEBUG
		modeline(wp);
		wp->w_flag =  0;
		wp->w_force = 0;
#endif
		wp = wp->w_wndp;
	}
	/* Always recompute the row and column number of the hardware	*/
	/* cursor. This is the only update for simple moves.		*/
	lp = curwp->w_linep;
	currow = curwp->w_toprow;
	while (lp != curwp->w_dotp) {
		++currow;
		lp = lforw(lp);
	}
	curcol = 0;
	i = 0;
	while (i < curwp->w_doto) {
		c = lgetc(lp, i++);
		if (c == '\t')
			curcol |= 0x07;
		else if (c<0x20 || c==0x7F)
			++curcol;
		++curcol;
	}
				/* Long line.		*/
	if (curcol >= RightMargin) curcol = RightMargin-1;
	if (curcol >= term.t_ncol) curcol = term.t_ncol-1;


	/* Special hacking if the screen is garbage. Clear the hardware	*/
	/* screen, and update your copy to agree with it. Set all the	*/
	/* virtual screen change bits, to force a full update.		*/
	if (sgarbf != FALSE) {
		for (i=0; i<term.t_nrow; ++i) {
			vscreen[i]->v_flag |= VFCHG;
			vp1 = pscreen[i];
			for (j=0; j<term.t_ncol; ++j)
				vp1->v_text[j] = ' ';
		}
		movecursor(0, 0);		/* Erase the screen.	*/
		(*term.t_eeop)();
		sgarbf = FALSE;			/* Erase-page clears	*/
		mpresf = FALSE;			/* the message area.	*/
	}
	/* Make sure that the physical and virtual displays agree.	*/
	/* Unlike before, the "updateline" code is only called with a	*/
	/* line that has been updated for sure.				*/	

	for (i=0; i<term.t_nrow; ++i) {
		vp1 = vscreen[i];
		if ((vp1->v_flag&VFCHG) != 0) {
			vp1->v_flag &= ~VFCHG;
			vp2 = pscreen[i];
			if(!updateline(i, &vp1->v_text[0], &vp2->v_text[0]))
				vp1->v_flag &= ~VFCHG;
		}
	}
	/* Finally, update the hardware cursor and flush out buffers.	*/
	movecursor(currow, curcol);
	ttcursoron();
	(*term.t_flush)();
}

/*
 * Update a single line. This
 * does not know how to use insert
 * or delete character sequences; we are
 * using VT52 functionality. Update the physical
 * row and column variables. It does try an
 * exploit erase to end of line. The RAINBOW version
 * of this routine uses fast video.
 */
int updateline(row, vline, pline)
int row;
UBYTE *vline;
UBYTE *pline;
{
#if	RAINBOW
	 char	*cp1;
	 char	*cp2;
	 int	nch;

	cp1 = &vline[0];			/* Use fast video.	*/
	cp2 = &pline[0];
	putline(row+1, 1, cp1);
	nch = term.t_ncol;
	do {
		*cp2 = *cp1;
		++cp2;
		++cp1;
	} while (--nch);
#else
	 char	*cp1;
	 char	*cp2;
	 char	*cp3;
	 char	*cp4;
	 char	*cp5;
	 int	nbflag;

	cp1 = &vline[0];			/* Compute left match.	*/
	cp2 = &pline[0];
	while (cp1!=&vline[term.t_ncol] && cp1[0]==cp2[0]) {
		++cp1;
		++cp2;
	}
	/* This can still happen, even though we only call this routine	*/
	/* on changed lines. A hard update is always done when a line	*/
	/* splits, a massive change is done, or a buffer is displayed	*/
	/* twice. This optimizes out most of the excess updating. A lot	*/
	/* of computes are used, but these tend to be hard operations	*/
	/* that do a lot of update, so I do not really care.		*/

	if (cp1 == &vline[term.t_ncol]) {
/*		vp1->v_flag &= ~VFCHG;*/
		return(FALSE); 	/* All equal.	*/
	}
 	nbflag = FALSE;
	cp3 = &vline[term.t_ncol];		/* Compute right match.	*/
	cp4 = &pline[term.t_ncol];
	while (cp3[-1] == cp4[-1]) {
		--cp3;
		--cp4;
		if (cp3[0] != ' ')		/* Note if any nonblank	*/
			nbflag = TRUE;		/* in right match.	*/
	}
	cp5 = cp3;
	if (nbflag == FALSE) {			/* Erase to EOL ?	*/
		while (cp5!=cp1 && cp5[-1]==' ')
			--cp5;
		if (cp3-cp5 <= 3)		/* Use only if erase is	*/
			cp5 = cp3;		/* fewer characters.	*/
	}
	movecursor(row, cp1-&vline[0]);		/* Go to start of line.	*/
	while (cp1 != cp5) {			/* Ordinary.		*/
		(*term.t_putchar)(*cp1);
		++ttcol;
		*cp2++ = *cp1++;
	}
	if (cp5 != cp3) {			/* Erase.		*/
		(*term.t_eeol)();
		while (cp1 != cp3)
			*cp2++ = *cp1++;
	}
#endif
return(TRUE);
}

/*
 * Redisplay the mode line for
 * the window pointed to by the "wp".
 * This is the only routine that has any idea
 * of how the modeline is formatted. You can
 * change the modeline format by hacking at
 * this routine. Called by "update" any time
 * there is a dirty window.
 */
void modeline(wp)
 EWINDOW	*wp;
{
	 char	*cp;
	 int	c;
	 int	n;
	 BUFFER	*bp;

	n = wp->w_toprow+wp->w_ntrows;		/* Location.		*/
	vscreen[n]->v_flag |= VFCHG;		/* Redraw next time.	*/
	vtmove(n, 0);				/* Seek to right line.	*/
/*
	vtzapc('\033');
	vtzapc('[');
	vtzapc('7');
	vtzapc('m');
*/
	vtzapc('-');
	bp = wp->w_bufp;
	if ((bp->b_flag&BFCHG) != 0)		/* "*" if changed.	*/
		vtzapc('*');
	else vtzapc('-');

	n  = 2;

	cp = " MicroEMACS -- ";			/* Buffer name.		*/
	while ((c = *cp++) != 0) {
		vtzapc(c);
		++n;
	}
	cp = &bp->b_bname[0];
	while ((c = *cp++) != 0) {
		vtzapc(c);
		++n;
	}
	vtzapc(' ');
	++n;
	if (bp->b_fname[0] != 0) {		/* File name.		*/
		cp = "-- File: ";
		while ((c = *cp++) != 0) {
			vtzapc(c);
			++n;
		}
		cp = &bp->b_fname[0];
		while ((c = *cp++) != 0) {
			vtzapc(c);
			++n;
		}
		vtzapc(' ');
		++n;
	}
	while (n < term.t_ncol-8) {
		vtzapc('-');
		++n;
	}
	vtzapc((curbp->b_mode&CMODE) ? 'C' : '-');
	vtzapc((curbp->b_mode&WRAPMODE) ? 'W' : '-');
	n += 2;

#if	WFDEBUG
	vtzapc('-');
	vtzapc((wp->w_flag&WFMODE)!=0  ? 'M' : '-');
	vtzapc((wp->w_flag&WFHARD)!=0  ? 'H' : '-');
	vtzapc((wp->w_flag&WFEDIT)!=0  ? 'E' : '-');
	vtzapc((wp->w_flag&WFMOVE)!=0  ? 'V' : '-');
	vtzapc((wp->w_flag&WFFORCE)!=0 ? 'F' : '-');
	n += 6;
#endif

	while (n < term.t_ncol) {		/* Pad to full width.	*/
		vtzapc('-');
		++n;
	}

/*
vtzapc('\033');
vtzapc('[');
vtzapc('0');
vtzapc('m');
*/
}

/*
 * Send a command to the terminal
 * to move the hardware cursor to row "row"
 * and column "col". The row and column arguments
 * are origin 0. Optimize out random calls.
 * Update "ttrow" and "ttcol".
 */

void movecursor(row, col)
int row,col;
{
	if (row!=ttrow || col!=ttcol) {
		ttrow = row;
		ttcol = col;
		(*term.t_move)(row, col);
	}
}

/*
 * Erase the message line.
 * This is a special routine because
 * the message line is not considered to be
 * part of the virtual screen. It always works
 * immediately; the terminal buffer is flushed
 * via a call to the flusher.
 */
void mlerase()
{
	movecursor(term.t_nrow, 0);
	(*term.t_eeol)();
	(*term.t_flush)();
	mpresf = FALSE;
}

/*
 * Ask a yes or no question in
 * the message line. Return either TRUE,
 * FALSE, or ABORT. The ABORT status is returned
 * if the user bumps out of the question with
 * a ^G. Used any time a confirmation is
 * required.
 */
int mlyesno(prompt)
UBYTE	*prompt;
{
	 int	s;
	char		buf[64];

	for (;;) {
		strcpy(buf, prompt);
		strcat(buf, " [y/n]? ");
		s = mlreply(buf, buf, sizeof(buf));
		if (s == ABORT)
			{return (ABORT);}
		if (s != FALSE) {
			if (buf[0]=='y' || buf[0]=='Y')
				{return (TRUE);}
			if (buf[0]=='n' || buf[0]=='N')
				{return (FALSE);}
		}
	}
}
/*
 * Ask a yes or no question in
 * the message line. Return either TRUE,
 * FALSE, or ABORT. The ABORT status is returned
 * if the user bumps out of the question with
 * a ^G. Used any time a confirmation is
 * required, with no cursor and no cr.
 */

int mlgetnumber(prompt,def)
UBYTE *prompt;
int def;
{
int num;
UBYTE buffer[82];
 
    if(!(mlreply(prompt,buffer,80)))return(def);
    num=atoi(buffer);
    mlerase();
    return(num);
}

int mlquery(prompt)
UBYTE *prompt;
{
	 int	s;
	 UBYTE	buf[64];

	for (;;) {
		strcpy(buf, prompt);
		strcat(buf, " [y/n/c/^G] ?");
		s = mlquiet(buf);
		if (s == 0x07 ) {	/* control G */
			(*term.t_flush)();
			ctrlg(FALSE, 0); 
			return (ABORT);
		}
		if(s == 'y' || s == 'Y' || s == ' ' || s == '\r')return(TRUE);
		if(s == 'n' || s == 'N') return (FALSE);
		if(s == 'c' || s == 'C') return(CONT);
	}
}


/*
 * Write a prompt into the message
 * line, then read back a response. Keep
 * track of the physical position of the cursor.
 * If we are in a keyboard macro throw the prompt
 * away, and return the remembered response. This
 * lets macros run at full speed. The reply is
 * always terminated by a carriage return. Handle
 * erase, kill, and abort keys.
 */

int mlreply(prompt, buf, nbuf)
UBYTE   *prompt;
UBYTE	*buf;
int nbuf;
{
	 int	cpos=0;
	 int	i;
	 int	c;
	 int	quoteFlag;

    if (kbdmop != NULL) {
	while ( ((c = *kbdmop++) != '\0') && (c != 0x0D)) buf[cpos++] = c;
	    buf[cpos] = 0;
	    if (buf[0] == 0) return (FALSE);
	    return (TRUE);
    }

    if (cmdmop) {
	while ( (c = cmdgetc()) > 0) buf[cpos++] = c;
	    buf[cpos] = 0;
	    if (buf[0] != 0) return (TRUE);
	    /* need more input ? ask the user! */
    }

    if(fkdmop) { /* if we are doing a function key */
        if(!fkinflag) { /* direct input allowed from function keys ? */
		while (((c = *fkdmop++) != '\0' ) && (c != 0x0D))
		    buf[cpos++] = c;
		buf[cpos] = 0;
		if (buf[0])return(TRUE);
		return (FALSE);
	}
    }

    mlwrite(prompt);
    quoteFlag=FALSE;
    for (;;) {
		c = (*term.t_getchar)();
		if(quoteFlag==TRUE) {
			quoteFlag=FALSE;
			if (cpos < nbuf-1) {
			    buf[cpos++] = c;
			    if ((c < ' ') || ((c > 127) && (c <160))) {
				(*term.t_putchar)('^');
				++ttcol;
				c ^= 0x40;
			    }
			    (*term.t_putchar)(c);
			    ++ttcol;
			    (*term.t_flush)();
			}
		}
		else switch (c) {

		case 0x9b: 	/* keypad character */
                        c = (*term.t_getchar)()+13; /* handle map */
		 	/* eat the ~ */
                        if((*term.t_getchar)() != '~')(*term.t_getchar)();
			/* keypad map is a little funny, so we need to */
			/* correct it here */
			if(c=='<')c='-';
			if (c==':')c='.';
			if(c != ';')goto enterchar;
			 /* else drop through in return case */
		case 0x0D:			/* Return, end of line	*/
			buf[cpos++] = 0;
			if (kbdmip != NULL) {
				if (kbdmip+cpos > &kbdm[NKBDM-3]) {
					ctrlg(FALSE, 0);
					(*term.t_flush)();
					return (ABORT);
				}
				for (i=0; i<cpos; ++i)
					*kbdmip++ = buf[i];
			}
			(*term.t_putchar)('\r');
			ttcol = 0;
			(*term.t_flush)();
			 mlerase();
			if (buf[0] == 0) return (FALSE);
			return (TRUE);

		case 0x07:			/* Bell, abort		*/
			(*term.t_putchar)('^');
			(*term.t_putchar)('G');
			ttcol += 2;
			ctrlg(FALSE, 0);
			(*term.t_flush)();
			return (ABORT);

		case 0x7F:			/* Rubout, erase	*/
		case 0x08:			/* Backspace, erase	*/
			if (cpos != 0) {
				(*term.t_putchar)('\b');
				(*term.t_putchar)(' ');
				(*term.t_putchar)('\b');
				--ttcol;
				if (buf[--cpos] < 0x20) {
					(*term.t_putchar)('\b');
					(*term.t_putchar)(' ');
					(*term.t_putchar)('\b');
					--ttcol;
				}
				(*term.t_flush)();
			}
			break;

		case 0x15:			/* C-U, kill		*/
			while (cpos != 0) {
				(*term.t_putchar)('\b');
				(*term.t_putchar)(' ');
				(*term.t_putchar)('\b');
				--ttcol;
				if (buf[--cpos] < 0x20) {
					(*term.t_putchar)('\b');
					(*term.t_putchar)(' ');
					(*term.t_putchar)('\b');
					--ttcol;
				}
			}
			(*term.t_flush)();
			break;

		case 0x11:
			quoteFlag=TRUE;
			(*term.t_flush)();
			break;

		default:
		    enterchar:
			if (cpos < nbuf-1) {
				buf[cpos++] = c;
				if ((c < ' ') || ((c > 127) && (c <160))) {
					(*term.t_putchar)('^');
					++ttcol;
					c ^= 0x40;
				}
				(*term.t_putchar)(c);
				++ttcol;
				(*term.t_flush)();
			}
		}
	}
}

int mlquiet(prompt)
UBYTE	*prompt;
{
 int	c;

	if (kbdmop != NULL) return( (int) *kbdmop++);
	if ((fkdmop != NULL) && (!fkinflag)) return( (int) *fkdmop++);
	if (cmdmop != NULL ) return(cmdgetc());

	mlwrite(prompt);
	movecursor(currow,getccol(FALSE)); /* restore the cursor */
	c= (*term.t_getchar)();
	return(c);
}
/*
 * Write a message into the message
 * line. Keep track of the physical cursor
 * position. A small class of printf like format
 * items is handled. Assumes the stack grows
 * down; this assumption is made by the "++"
 * in the argument scan loop. Set the "message
 * line" flag TRUE.
 */
void __stdargs mlwrite(fmt, arg)
UBYTE	*fmt;
LONG	arg;
{
	 int	c;
	 char	*ap;

	movecursor(term.t_nrow, 0);
	ap = (char *) &arg;

	while ((c = *fmt++) != 0) {
		if (c != '%') {
			(*term.t_putchar)(c);
			++ttcol;
		}
		else {
			c = *fmt++;
			switch (c) {
			case 'd':
				mlputi(*(int *)ap, 10);
				ap += sizeof(int);
				break;

			case 'o':
				mlputi(*(int *)ap,  8);
				ap += sizeof(int);
				break;

			case 'x':
				mlputi(*(int *)ap, 16);
				ap += sizeof(int);
				break;

			case 'D':
				mlputli(*(long *)ap, 10);
				ap += sizeof(long);
				break;

			case 's':
				mlputs(*(char **)ap);
				ap += sizeof(char *);
				break;

			default:
				(*term.t_putchar)(c);
				++ttcol;
			}
		}
	}
	(*term.t_eeol)();
	(*term.t_flush)();
	mpresf = TRUE;
}

/*
 * Write out a string.
 * Update the physical cursor position.
 * This assumes that the characters in the
 * string all have width "1"; if this is
 * not the case things will get screwed up
 * a little.
 */
void mlputs(s)
 UBYTE	*s;
{
	 int	c;

	while ((c = *s++) != 0) {
		(*term.t_putchar)(c);
		++ttcol;
	}
}

/*
 * Write out an integer, in
 * the specified radix. Update the physical
 * cursor position. This will not handle any
 * negative numbers; maybe it should.
 */
void mlputi(i, r)
int i,r;
{
	 int	q;
	static char hexdigits[] = "0123456789ABCDEF";

	if (i < 0) {
		i = -i;
		(*term.t_putchar)('-');
	}
	q = i/r;
	if (q != 0)
		mlputi(q, r);
	(*term.t_putchar)(hexdigits[i%r]);
	++ttcol;
}

/*
 * do the same except as a long integer.
 */
void mlputli(l, r)
long l;
int r;
{
	 long q;

	if (l < 0) {
		l = -l;
		(*term.t_putchar)('-');
	}
	q = l/r;
	if (q != 0)
		mlputli(q, r);
	(*term.t_putchar)((int)(l%r)+'0');
	++ttcol;
}


void abort()
{
mlwrite("Couldn't allocate memory\n");
vttidy();

vtfree();
edfree();
Delay(20);
errorExit(20);
}

int echo(f,n)
int f,n;
{
UBYTE buffer[82];

mlreply("",buffer,80);
mlwrite(buffer);
return(TRUE);
}
