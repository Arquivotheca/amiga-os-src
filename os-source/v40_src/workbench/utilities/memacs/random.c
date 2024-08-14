/*
 * This file contains the
 * command processing functions for
 * a number of random commands. There is
 * no functional grouping here, for
 * sure.
 */
#include	"ed.h"

extern int      tabsize;     /* Tab size (0: use real tabs)  */
extern int	indentsize;
extern UBYTE	screenname[];
/*
 * Display the current position of the cursor,
 * in origin 1 X-Y coordinates, the character that is
 * under the cursor (in octal), and the fraction of the
 * text that is before the cursor. The displayed column
 * is not the current column, but the column that would
 * be used on an infinite width display. Normally this
 * is bound to "C-X =".
 */
int showcpos(f, n)
int f,n;
{
	 LINE	*clp;
	 LONG	nch=0;
	 LONG	cbo=0;
	 LONG	nbc;
/*	 LONG	cac; */
	 LONG	ratio=0;
	 LONG	col;
	 LONG	i=0;
	 LONG	cline;

	clp = lforw(curbp->b_linep);		/* Grovel the data.	*/
	for (;;) {
		if (clp==curwp->w_dotp && cbo==curwp->w_doto) {
			cline=i;
			nbc = nch;
/*
seems unused
			if (cbo == llength(clp)) cac = '\n';
			else cac = lgetc(clp, cbo);
*/
		}
		if (cbo == llength(clp)) {
			if (clp == curbp->b_linep) break;
			clp = lforw(clp);
			i++;
			cbo = 0;
		}
		else ++cbo;
		++nch;
	}
	col = getccol(FALSE);		/* Get real column.	*/
	if (nch != 0) ratio = (100L*nbc) / nch;
	mlwrite("Line %d Column %d  (%d%%) [%s]", cline+1, col+1, ratio,
		screenname);
	return (TRUE);
}

/*
 * Return current column.  Stop at first non-blank given TRUE argument.
 */

int getccol(bflg)
int bflg;
{
        int c, i, col;

        col = 0;
        for (i=0; i<curwp->w_doto; ++i) {
                c = lgetc(curwp->w_dotp, i);
                if (c!=' ' && c!='\t' && bflg)
                        break;
                if (c == '\t')
                        col |= 0x07;
                else if (c<0x20 || c==0x7F)
                        ++col;
                ++col;
	}
        return(col);
}

/* get the current line */
int getcline()
{
	 LINE	*clp;
	 LONG	cbo=0;
/*	 LONG	cac; */
	 LONG	i=0;
	 LONG	cline;

	clp = lforw(curbp->b_linep);		/* Grovel the data.	*/
	for (;;) {
		if (clp==curwp->w_dotp && cbo==curwp->w_doto) {
			cline=i;
/*
seems unused ?
			if (cbo == llength(clp)) cac = '\n';
			else cac = lgetc(clp, cbo);
*/
		}
		if (cbo == llength(clp)) {
			if (clp == curbp->b_linep) break;
			clp = lforw(clp);
			i++;
			cbo = 0;
		}
		else ++cbo;
	}
return(cline);
}

/*
 * Twiddle the two characters on either side of
 * dot. If dot is at the end of the line twiddle the
 * two characters before it. Return with an error if dot
 * is at the beginning of line; it seems to be a bit
 * pointless to make this work. This fixes up a very
 * common typo with a single stroke. Normally bound
 * to "C-T". This always works within a line, so
 * "WFEDIT" is good enough.
 */
int twiddle(f, n)
    int f,n;
{
	 LINE	*dotp;
	 int	doto;
	 int	cl;
	 int	cr;

	dotp = curwp->w_dotp;
	doto = curwp->w_doto;
	if (doto==llength(dotp) && --doto<0)
		return (FALSE);
	cr = lgetc(dotp, doto);
	if (--doto < 0)
		return (FALSE);
	cl = lgetc(dotp, doto);
	lputc(dotp, doto+0, cr);
	lputc(dotp, doto+1, cl);
	lchange(WFEDIT);
	return (TRUE);
}

/*
 * Quote the next character, and
 * insert it into the buffer. All the characters
 * are taken literally, with the exception of the newline,
 * which always has its line splitting meaning. The character
 * is always read, even if it is inserted 0 times, for
 * regularity. Bound to "M-Q" (for me) and "C-Q" (for Rich,
 * and only on terminals that dont need XON-XOFF).
 */
int quote(f, n)
int f,n;
{
	 int	s;
	 int	c;

	c = (*term.t_getchar)();
	if (n < 0) return (FALSE);
	if (n == 0) return (TRUE);
	if (c == '\n') {
		do {
		    s = lnewline();
		} while (s==TRUE && --n);
		return (s);
	}
	return (linsert(n, c));
}

/*
 * Open up some blank space. The basic plan
 * is to insert a bunch of newlines, and then back
 * up over them. Everything is done by the subcommand
 * procerssors. They even handle the looping. Normally
 * this is bound to "C-O".
 */
int openline(f, n)
int f,n;
{
	 int	i;
	 int	s;

	if (n < 0)
		return (FALSE);
	if (n == 0)
		return (TRUE);
	i = n;					/* Insert newlines.	*/
	do {
		s = lnewline();
	} while (s==TRUE && --i);
	if (s == TRUE)				/* Then back up overtop	*/
		s = backchar(f, n);		/* of them all.		*/
	return (s);
}

/*
 * Insert a newline. Bound to "C-M".
 * If you are at the end of the line and the
 * next line is a blank line, just move into the
 * blank line. This makes "C-O" and "C-X C-O" work
 * nicely, and reduces the ammount of screen
 * update that has to be done. This would not be
 * as critical if screen update were a lot
 * more efficient.
 */
int newline(f, n)
{
	 LINE	*lp;
	 int	s;

	if (n < 0)
		return (FALSE);
	while (n--) {
		lp = curwp->w_dotp;
		if (llength(lp) == curwp->w_doto
		&& lp != curbp->b_linep
		&& llength(lforw(lp)) == 0) {
			if ((s=forwchar(FALSE, 1)) != TRUE)
				return (s);
		} else if ((s=lnewline()) != TRUE) return (s);
	lmargin();
	}
	return (TRUE);
}

/*
 * Delete blank lines around dot.
 * What this command does depends if dot is
 * sitting on a blank line. If dot is sitting on a
 * blank line, this command deletes all the blank lines
 * above and below the current line. If it is sitting
 * on a non blank line then it deletes all of the
 * blank lines after the line. Normally this command
 * is bound to "C-X C-O". Any argument is ignored.
 */
int deblank(f, n)
int f,n;
{
	 LINE	*lp1;
	 LINE	*lp2;
	 int	nld;

	lp1 = curwp->w_dotp;
	while (llength(lp1)==0 && (lp2=lback(lp1))!=curbp->b_linep)
		lp1 = lp2;
	lp2 = lp1;
	nld = 0;
	while ((lp2=lforw(lp2))!=curbp->b_linep && llength(lp2)==0)
		++nld;
	if (nld == 0)
		return (TRUE);
	curwp->w_dotp = lforw(lp1);
	curwp->w_doto = 0;
	return (ldelete(nld,0));
}

/*
 * Insert a newline, then enough
 * tabs and spaces to duplicate the indentation
 * of the previous line. Assumes tabs are every eight
 * characters. Quite simple. Figure out the indentation
 * of the current line. Insert a newline by calling
 * the standard routine. Insert the indentation by
 * inserting the right number of tabs and spaces.
 * Return TRUE if all ok. Return FALSE if one
 * of the subcomands failed. Normally bound
 * to "C-J".
 */
int indent(f, n)
int f,n;
{
    int nicol=0;
    LINE *line;

	if (n < 0) return (FALSE);
	while (n--) {

	    if(curbp->b_mode & CMODE) {
		if((line = curwp->w_dotp) == curbp->b_linep)line=lback(line);
		while (line != curbp->b_linep) {
		    if(line->l_text[line->l_used-1] == '{') {
			nicol = getindent(line)+indentsize;
			break;
		    }
		line=lback(line);
		}
	    }
	    else nicol=getindent(curwp->w_dotp);
	    if (lnewline() == FALSE)return(FALSE);
	    if(!(doindent(nicol)))return(FALSE);
	}
return(TRUE);
}

int getindent(line)
LINE *line;
{
int nicol = 0,i,c;

    for (i=0; i<llength(line); ++i) {
	c = lgetc(line, i);
	if (c!=' ' && c!='\t') break;
	if (c == '\t') nicol |= 0x07;
	++nicol;
    }
return(nicol);
}

int doindent(nicol)
int nicol;
{
int i;

    if (tabsize != 0) {
	if ((nicol !=0 ) && linsert(nicol,  ' ')==FALSE) return(FALSE);
    }
    else  {
	if ((((i=nicol/8)!=0) && !linsert(i, '\t')) || 
	    ((i=nicol%8)!=0 && !linsert(i,  ' '))) return (FALSE);
    }
return(TRUE);
}

/*
 * Insert enough tabs and spaces to get to the left margin.
 * Assumes tabs are every eight characters.
 * Insert the indentation by
 * inserting the right number of tabs and spaces.
 * Return TRUE if all ok. Return FALSE if one
 * of the subcomands failed.
 */

int lmargin()
{
         int    nicol;
         int    i;

        if(((nicol = (LeftMargin-1)) == -1) || (getccol(FALSE) >= LeftMargin))
                return(TRUE);

	if (tabsize != 0) {
	    if ((nicol !=0 ) && linsert(nicol,  ' ')==FALSE) return(FALSE);
	    return(TRUE);
	}

        if ( ((i=nicol/8)!=0 && linsert(i, '\t')==FALSE)
        || ((i=nicol%8)!=0 && linsert(i,  ' ')==FALSE))
                return (FALSE);
        else return (TRUE);
}

/*
 * Delete forward. This is real
 * easy, because the basic delete routine does
 * all of the work. Watches for negative arguments,
 * and does the right thing. If any argument is
 * present, it kills rather than deletes, to prevent
 * loss of text if typed with a big argument.
 * Normally bound to "C-D".
 */
int forwdel(f, n)
int f,n;
{
	if (n < 0)
		return (backdel(f, -n));
	if (f != FALSE) {			/* Really a kill.	*/
		if ((lastflag&CFKILL) == 0)
			kdelete();
		thisflag |= CFKILL;
	}
	return (ldelete(n, f));
}

/*
 * Delete backwards. This is quite easy too,
 * because its all done with other functions. Just
 * move the cursor back, and delete forwards.
 * Like delete forward, this actually does a kill
 * if presented with an argument. Bound to both
 * "RUBOUT" and "C-H".
 */

int backdel(f, n)
int f,n;
{

	 int	s;

	if (n < 0)
		return (forwdel(f, -n));
	if (f != FALSE) {			/* Really a kill.	*/
		if ((lastflag&CFKILL) == 0)
			kdelete();
		thisflag |= CFKILL;
	}
	if ((s=backchar(f, n)) == TRUE)
		s = ldelete(n, f);
	return (s);
}

/*
 * Kill text. If called without an argument,
 * it kills from dot to the end of the line, unless it
 * is at the end of the line, when it kills the newline.
 * If called with an argument of 0, it kills from the
 * start of the line to dot. If called with a positive
 * argument, it kills from dot forward over that number
 * of newlines. If called with a negative argument it
 * kills backwards that number of newlines. Normally
 * bound to "C-K".
 */
int kill(f, n)
    int f,n;
{
	 int	chunk;
	 LINE	*nextp;

	if ((lastflag&CFKILL) == 0)		/* Clear kill buffer if	*/
		kdelete();			/* last wasnt a kill.	*/
	thisflag |= CFKILL;
	if (f == FALSE) {
		chunk = llength(curwp->w_dotp)-curwp->w_doto;
		if (chunk == 0)
			chunk = 1;
	} else if (n == 0) {
		chunk = curwp->w_doto;
		curwp->w_doto = 0;
	} else if (n > 0) {
		chunk = llength(curwp->w_dotp)-curwp->w_doto+1;
		nextp = lforw(curwp->w_dotp);
		while (--n) {
			if (nextp == curbp->b_linep)
				return (FALSE);
			chunk += llength(nextp)+1;
			nextp = lforw(nextp);
		}
	} else {
		mlwrite("neg kill");
		return (FALSE);
	}
	return (ldelete(chunk, TRUE));
}

/*
 * Yank text back from the kill buffer. This
 * is really easy. All of the work is done by the
 * standard insert routines. All you do is run the loop,
 * and check for errors. Bound to "C-Y". The blank
 * lines are inserted with a call to "newline"
 * instead of a call to "lnewline" so that the magic
 * stuff that happens when you type a carriage
 * return also happens when a carriage return is
 * yanked back from the kill buffer.
 */
int yank(f, n)
    int f,n;
{
	 int	c;
	 int	i;
	extern	 int	kused;

	if (n < 0)
		return (FALSE);
	while (n--) {
		i = 0;
		while ((c=kremove(i)) >= 0) {
			if (c == '\n') {
				if (newline(FALSE, 1) == FALSE)
					return (FALSE);
			} else {
				if (linsert(1, c) == FALSE)
					return (FALSE);
			}
			++i;
		}
	}
	return (TRUE);
}

int deleteline(f, n)
    int f,n;
{
 int    s;
 int count=n;   

    do {
    	if (gotobol(0,0)==FALSE)return(FALSE);
    	s=kill(TRUE,1);
    	if (gotobol(0,0)==FALSE)return(FALSE);
    } while (--count > 0);
    return(s);
}

int gotoline(f, n)
int f,n;
{
 int temp;
 int s;

if((temp=mlgetnumber("goto line: ",-1)) >= 0) {
        mlerase();
        gotobob(0,0);
	temp--;
        s=forwline(TRUE,temp);
        return(s);
}
return(FALSE);
}


/*
 * Insert a tab into file.  If given argument, n, of zero, change to true tabs.
 * If n > 1, simulate tab stop every n-characters using spaces.
 * This has to be done in this slightly funny way because the
 * tab (in ASCII) has been turned into "C-I" (in 10
 * bit code) already. Bound to "C-I".
 */

int tab(f, n)
int f,n;
{
    if (tabsize==0) return(linsert(1, '\t'));
    return(linsert(tabsize - (getccol(FALSE) % tabsize), ' '));
}

#if !AMIGA
Delay(n)
int n;
{
int i;

for(i=0; i<n*100; i++);
}
#endif
