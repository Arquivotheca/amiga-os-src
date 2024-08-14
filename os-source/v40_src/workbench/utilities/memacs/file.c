/*
 * The routines in this file
 * handle the reading and writing of
 * disk files. All of details about the
 * reading and writing of the disk are
 * in "fileio.c".
 */
#include	"ed.h"

/*
 * Read a file into the current
 * buffer. This is really easy; all you do it
 * find the name of the file, and call the standard
 * "read a file into the current buffer" code.
 * Bound to "C-X C-R".
 */
int fileread(f, n)
    int f,n;
{
	 int	s;
	UBYTE fname[NFILEN];

	if ((s=mlreply("Read file: ", fname, NFILEN)) != TRUE)
		return (s);
	return (readin(fname));
}

/*
 * Select a file for editing.
 * Look around to see if you can find the
 * fine in another buffer; if you can find it
 * just switch to the buffer. If you cannot find
 * the file, create a new buffer, read in the
 * text, and switch to the new buffer.
 * Bound to C-X C-V.
 */
int filevisit(f, n)
    int f,n;
{
	 BUFFER	*bp;
	 EWINDOW *wp;
	 LINE	*lp;
	 int	i;
	 int	s;
	UBYTE bname[NBUFN];
	UBYTE fname[NFILEN];

	if ((s=mlreply("Visit file: ", fname, NFILEN)) != TRUE)
		return (s);
	for (bp=bheadp; bp!=NULL; bp=bp->b_bufp) {
		if(((bp->b_flag&BFTEMP)==0)&&(strcmp(bp->b_fname,fname)==0)){
			if (--curbp->b_nwnd == 0) {
				curbp->b_dotp  = curwp->w_dotp;
				curbp->b_doto  = curwp->w_doto;
				curbp->b_markp = curwp->w_markp;
				curbp->b_marko = curwp->w_marko;
			}
			curbp = bp;
			curwp->w_bufp  = bp;
			if (bp->b_nwnd++ == 0) {
				curwp->w_dotp  = bp->b_dotp;
				curwp->w_doto  = bp->b_doto;
				curwp->w_markp = bp->b_markp;
				curwp->w_marko = bp->b_marko;
			} else {
				wp = wheadp;
				while (wp) {
					if (wp!=curwp && wp->w_bufp==bp) {
						curwp->w_dotp  = wp->w_dotp;
						curwp->w_doto  = wp->w_doto;
						curwp->w_markp = wp->w_markp;
						curwp->w_marko = wp->w_marko;
						break;
					}
					wp = wp->w_wndp;
				}
			}
			lp = curwp->w_dotp;
			i = curwp->w_ntrows/2;
			while (i-- && lback(lp)!=curbp->b_linep)
				lp = lback(lp);
			curwp->w_bufp =bp;    /* switch */
			curwp->w_linep = lp;
			curwp->w_flag |= WFMODE|WFHARD;
			mlwrite("[Old buffer]");
			return (TRUE);
		}
	}
	makename(bname, fname);			/* New buffer name.	*/
	while ((bp=bfind(bname, FALSE, 0))) {
		s = mlreply("Buffer name: ", bname, NBUFN);
		if (s == ABORT)			/* ^G to just quit	*/
			return (s);
		if (s == FALSE) {		/* CR to clobber it	*/
			makename(bname, fname);
			break;
		}
	}
	if (bp==NULL && (bp=bfind(bname, TRUE, 0))==NULL) {
		mlwrite("Cannot create buffer");
		return (FALSE);
	}
	if (--curbp->b_nwnd == 0) {		/* Undisplay.		*/
		curbp->b_dotp = curwp->w_dotp;
		curbp->b_doto = curwp->w_doto;
		curbp->b_markp = curwp->w_markp;
		curbp->b_marko = curwp->w_marko;
	}
	curbp = bp;				/* Switch to it.	*/
	curwp->w_bufp = bp;
	curbp->b_nwnd++;
	return (readin(fname));			/* Read it in.		*/
}

/*
 * Read file "fname" into the current
 * buffer, blowing away any text found there. Called
 * by both the read and visit commands. Return the final
 * status of the read. Also called by the mainline,
 * to read in a file specified on the command line as
 * an argument.
 */
int readin(fname)
    UBYTE fname[];
{
	 LINE	*lp1;
	 LINE	*lp2;
	 int	i;
	 EWINDOW	*wp;
	 BUFFER	*bp;
	 int	s;
	int nbytes;
	int nline;
	UBYTE line[NLINE];

	bp = curbp;				/* Cheap.		*/
	if ((s=bclear(bp)) != TRUE)		/* Might be old.	*/
		return (s);
	bp->b_flag &= ~(BFTEMP|BFCHG);
	strcpy(bp->b_fname, fname);

	s=strlen(fname)-2;
	if(!strnicmp(&fname[s],".c",2)||(!strnicmp(&fname[s],".h",2)))
		bp->b_mode |= CMODE;
	else bp->b_mode &= ~CMODE;
	bp->b_protection = getprot(fname);
	if ((s=ffropen(fname)) == FIOERR) 	/* Hard file open.	*/
		goto out;
	if (s == FIOFNF) {			/* File not found.	*/
		mlwrite("[New file]");
		goto out;
	}
	mlwrite("[Reading file]");
	nline = 0;
	while ((s=ffgetline(line, NLINE, &nbytes)) == FIOSUC) {
		if (!(lp1=lalloc(nbytes))) {
			s = FIOERR;		/* Keep message on the	*/
			break;			/* display.		*/
		}
		lp2 = lback(curbp->b_linep);
		lp2->l_fp = lp1;
		lp1->l_fp = curbp->b_linep;
		lp1->l_bp = lp2;
		curbp->b_linep->l_bp = lp1;
		for (i=0; i<nbytes; ++i) lputc(lp1, i, line[i]);
		++nline;
	}
	ffclose();				/* Ignore errors.	*/
/*	if(s == FIONNL) curbp->b_flag |= BNNL;*/
/*	else curbp->b_flag &= ~BNNL; */

	if (s == FIOEOF) {	/* Don't zap message!	*/
		if (nline == 1)
			mlwrite("[Read 1 line]");
		else
			mlwrite("[Read %d lines]", nline);
	}
out:
	for (wp=wheadp; wp!=NULL; wp=wp->w_wndp) {
		if (wp->w_bufp == curbp) {
			wp->w_linep = lforw(curbp->b_linep);
			wp->w_dotp  = lforw(curbp->b_linep);
			wp->w_doto  = 0;
			wp->w_markp = NULL;
			wp->w_marko = 0;
			wp->w_flag |= WFMODE|WFHARD;
		}
	}
	if (s == FIOERR)			/* False if error.	*/
		return (FALSE);
	return (TRUE);
}

/*
 * Take a file name, and from it
 * fabricate a buffer name. This routine knows
 * about the syntax of file names on the target system.
 * I suppose that this information could be put in
 * a better place than a line of code.
 */
void makename(bname, fname)
   UBYTE	bname[];
   UBYTE	fname[];
{
	 UBYTE	*cp1;
	 UBYTE	*cp2;

	cp1 = &fname[0];
	while (*cp1 != 0) ++cp1;

#if	VMS
	while (cp1!=&fname[0] && cp1[-1]!=':' && cp1[-1]!=']')
		--cp1;
#endif
#if	CPM
	while (cp1!=&fname[0] && cp1[-1]!=':')
		--cp1;
#endif
#if	MSDOS
	while (cp1!=&fname[0] && cp1[-1]!=':' && cp1[-1]!='\\')
		--cp1;
#endif
#if	V7
	while (cp1!=&fname[0] && cp1[-1]!='/')
		--cp1;
#endif
#if	AMIGA
	while (cp1!=&fname[0] && cp1[-1]!=':' && cp1[-1]!='/') --cp1;
#endif
	cp2 = &bname[0];
	while (cp2!=&bname[NBUFN-1] && *cp1!=0 && *cp1!=';')
		*cp2++ = *cp1++;
	*cp2 = 0;
}

/*
 * Ask for a file name, and write the
 * contents of the current buffer to that file.
 * Update the remembered file name and clear the
 * buffer changed flag. This handling of file names
 * is different from the earlier versions, and
 * is more compatable with Gosling EMACS than
 * with ITS EMACS. Bound to "C-X C-W".
 */
int filewrite(f, n)
    int f,n;
{
	 EWINDOW	*wp;
	 int	s;
	 UBYTE fname[NFILEN];
	 ULONG protectbits;

	if ((s=mlreply("Write file: ", fname, NFILEN)) != TRUE)
		return (s);
	protectbits = getprot(fname);
	if ((s=writeout(fname)) == TRUE) {
		strcpy(curbp->b_fname, fname);
		curbp->b_flag &= ~BFCHG;
		setprot(fname,protectbits);
		wp = wheadp;			/* Update mode lines.	*/

		while (wp != NULL) {
			if (wp->w_bufp == curbp)
				wp->w_flag |= WFMODE;
			wp = wp->w_wndp;
		}
	}
	return (s);
}

/*
 * Save the contents of the current
 * buffer in its associated file. No nothing
 * if nothing has changed (this may be a bug, not a
 * feature). Error if there is no remembered file
 * name for the buffer. Bound to "C-X C-S". May
 * get called by "C-Z".
 */
int filesave(f, n)
    int f,n;
{
	 EWINDOW	*wp;
	 int	s;
	 ULONG protectbits;

	if ((curbp->b_flag&BFCHG) == 0)		/* Return, no changes.	*/
		return (TRUE);
	if (curbp->b_fname[0] == 0) {		/* Must have a name.	*/
		mlwrite("No file name");
		return (FALSE);
	}
	protectbits = getprot(curbp->b_fname);
	if ((s=writeout(curbp->b_fname)) == TRUE) {
		curbp->b_flag &= ~BFCHG;
		setprot(curbp->b_fname,protectbits);
		wp = wheadp;			/* Update mode lines.	*/
		while (wp != NULL) {
			if (wp->w_bufp == curbp)
				wp->w_flag |= WFMODE;
			wp = wp->w_wndp;
		}
	}
	return (s);
}

/*
 * This function performs the details of file
 * writing. Uses the file management routines in the
 * "fileio.c" package. The number of lines written is
 * displayed. Sadly, it looks inside a LINE; provide
 * a macro for this. Most of the grief is error
 * checking of some sort.
 */
int writeout(fn)
UBYTE *fn;
{
	 int	s;
	 LINE	*lp;
	 int	nline;

	if ((s=ffwopen(fn)) != FIOSUC)		/* Open writes message.	*/
		return (FALSE);
	lp = lforw(curbp->b_linep);		/* First line.		*/
	nline = 0;				/* Number of lines.	*/
	while (lp != curbp->b_linep) {
		if ((s=ffputline(&lp->l_text[0], llength(lp))) != FIOSUC)
			break;
		++nline;
		lp = lforw(lp);
	}
	if (s == FIOSUC) {			/* No write error.	*/
		s = ffclose();
		if (s == FIOSUC) {		/* No close error.	*/
			if (nline == 1)
				mlwrite("[Wrote 1 line]");
			else
				mlwrite("[Wrote %d lines]", nline);
		}
	} else					/* Ignore close error	*/
		ffclose();			/* if a write error.	*/
	if (s != FIOSUC)			/* Some sort of error.	*/
		return (FALSE);
	return (TRUE);
}

/*
 * The command allows the user
 * to modify the file name associated with
 * the current buffer. It is like the "f" command
 * in UNIX "ed". The operation is simple; just zap
 * the name in the BUFFER structure, and mark the windows
 * as needing an update. You can type a blank line at the
 * prompt if you wish.
 */
int filename(f, n)
    int f,n;
{
	 EWINDOW	*wp;
	 int	s;
	UBYTE fname[NFILEN];

	if ((s=mlreply("New file name: ", fname, NFILEN)) == ABORT)
		return (s);
	if (s == FALSE)
		strcpy(curbp->b_fname, "");
	else
		strcpy(curbp->b_fname, fname);
	curbp->b_flag &= ~(BFTEMP);	/* must be considered real now */

	wp = wheadp;				/* Update mode lines.	*/
	while (wp != NULL) {
		if (wp->w_bufp == curbp)
			wp->w_flag |= WFHARD|WFMODE;
		wp = wp->w_wndp;
	}
	return (TRUE);
}

int mfilename(fname)
	UBYTE fname[];
{
	 EWINDOW *wp;

	strcpy(curbp->b_fname, fname);
	wp = wheadp;				/* Update mode lines.	*/
	while (wp != NULL) {
		if (wp->w_bufp == curbp)
			wp->w_flag |= WFHARD|WFMODE;
		wp = wp->w_wndp;
	}
	return (TRUE);
}


/*
 * Insert a file into the current
 * buffer. This is really easy; all you do it
 * find the name of the file, and call the standard
 * "insert a file into the current buffer" code.
 * Bound to "C-X C-I".
 */
int fileinsert(f, n)
    int f,n;
{
	 int	s;
	UBYTE fname[NFILEN];

	if ((s=mlreply("Insert file: ", fname, NFILEN)) != TRUE)
		return (s);
	return (ireadin(fname));
}


int ireadin(fname)
    UBYTE fname[];
{
	 int	i;
	 BUFFER	*bp;
	int	s;
	int	nbytes;
	int	nline;
	int oldcol,oldrow;
	UBYTE		line[NLINE];

 	bp = curbp;	/* Cheap. */
	bp->b_flag &= ~(BFTEMP|BFCHG);
	if (((s=ffropen(fname)) == FIOERR) || (s == FIOFNF)) {
		mlwrite("[File not found]");
		return(FALSE);
	}
	mlwrite("[Inserting file]");
	oldcol=curwp->w_doto;
	oldrow=getcline();
	nline = 0;
	while ((s=ffgetline(line, NLINE, &nbytes)) == FIOSUC) {
		i=0;
		s=TRUE;
		while ((i<nbytes) && s==TRUE) s=linsert(1, line[i++]);
		if(s==FALSE) { 
		    s=FIOERR;
		    break;
		}
		if((s=lnewline())==FALSE) {
			s=FIOERR; 
			break;
		}
		++nline;
	}
	ffclose();				/* Ignore errors.	*/
	if (s != FIOERR) {			/* Dont zap message!	*/
		if (nline == 1)
			mlwrite("[Inserted 1 line]");
		else
			mlwrite("[Inserted %d lines]", nline);
	}
        /* restore cursor position */
	gotobob(0,0); 
	forwline(TRUE,oldrow);
	forwchar(TRUE,oldcol);
/*	curwp->w_dotp  = savelp;*/
/*	curwp->w_doto  = savebo;*/
	curwp->w_flag |= WFHARD;

	update();
	(*term.t_flush)();

	if (s == FIOERR)			/* False if error.	*/
		return (FALSE);
	return (TRUE);
}


/*
 * Look through the list of
 * buffers to see what should be saved
 * (any changed buffers). Buffers
 * that hold magic internal stuff are
 * not considered;
 * Return FALSE if can't save any of the
 * buffers;
 */
int filemodsave(f ,n)
    int f,n;
{
         BUFFER *bp;
	 BUFFER *tempbp;
	 int s=TRUE;

	tempbp=curbp;		/* save current buffer */
        bp = bheadp;
        while (bp != NULL) {
                if ((bp->b_flag&BFTEMP)==0 && (bp->b_flag&BFCHG)!=0) {
			curbp=bp;
			if(filesave(f ,n) == FALSE) s=FALSE;
		}
                bp = bp->b_bufp;
	}
	curbp=tempbp;		/* restore current buffer */
        return (s);
}


int jreadin(fname)
    UBYTE fname[];
{
	int	i;
	BUFFER	*bp;
	int	s;
	int nbytes;
	UBYTE c;
	UBYTE line[NLINE];
        int tfillcol;


	bp = curbp;	/* Cheap. */
	bp->b_flag &= ~BFCHG;    /* mark it as unchanged so no backtalk */
	if ((s=bclear(bp)) != TRUE)		/* Might be old.	*/
 		return (s);
	bp->b_flag &= ~(BFTEMP|BFCHG);
	if (((s=ffropen(fname)) == FIOERR) || (s == FIOFNF)) return(FALSE);

	tfillcol = (curbp->b_mode&WRAPMODE) ? fillcol : 0;
	while ((s=ffgetline(line, NLINE,&nbytes)) == FIOSUC) {
		i=0;
		s=TRUE;
		/* eat leading spaces and tabs */
		while (i<nbytes) { 
		    if ((line[i] == ' ') || (line[i] == '\t')) i++;
		    else break;
		}
		lmargin(); /* now put the left margin in correctly */

		while ((i<nbytes) && s==TRUE) {
		    s=linsert(1, c=line[i++]);
		    if (c == ' ' && tfillcol > 0 &&
			getccol(FALSE) > tfillcol) wrapword(TRUE,0);
		}
		if(s==FALSE) {
		    s=FIOERR;
		    break;
		}
		if(getccol(FALSE) > tfillcol) { /* only use a nl if need to */
		    if((s=lnewline())==FALSE) {
			s=FIOERR; 
			break;
		    }
		}
	}
	ffclose();				/* Ignore errors.	*/
	if (s == FIOERR)			/* False if error.	*/
		return (FALSE);
	return (TRUE);
}

int insertbuffer(f, n)
    int f,n;
{
         BUFFER *bp;
         BUFFER *tempbp;
         int    s;
        UBYTE bufn[NFILEN];

        if ((s=mlreply("Insert buffer: ", bufn, NBUFN)) != TRUE)
                return (s);

        if ((bp=bfind(bufn, FALSE, 0)) == NULL)
                return (FALSE);

	strcpy(bufn,"RAM:tempbuffer.out");
	tempbp=curbp;		/* save current buffer */
	curbp=bp;
	writeout(bufn);
	curbp=tempbp;		/* restore buffer */
	s=ireadin(bufn);
#if	AMIGA
	if(filexists(bufn) == TRUE) DeleteFile(bufn);
#endif
	mfilename(""); /* blank out the filename */
	return(s);
}

int justifybuffer(f ,n)
    int f,n;
{
int    s;
UBYTE bufn[NFILEN];


gotobob(FALSE,0);

strcpy(bufn,"RAM:tempbuffer.out");
if((s=writeout(bufn)) == TRUE) {
	s=jreadin(bufn);
}

#if	AMIGA
if(filexists(bufn) == TRUE) DeleteFile(bufn);
#endif

mfilename(""); /* blank out the filename */
mlerase();	/* clear the bottom line */

gotoeob(FALSE,0);
backdel(FALSE,2);
gotobob(FALSE,0);

return(s);
}

#ifdef TAGS
int visittag(f, n)	/* visit a file (via a sorted tag file) */
    {
    FILE *fp;
    char fname[NFILEN];	    /* file that the user wishes to open */
    char tname[40];	    /* tag name to search for */
    char sstr[128];	    /* search string */
    char istr[128],*ptr;    /* input string */
    register WINDOW *wp;    /* scan for windows that need updating */
    register LINE *dotp;
    register int   doto,i;

    /* save the current position */
    dotp = curwp->w_dotp;
    doto = curwp->w_doto;

    /* now get the name of the tag */
    while (inword() == FALSE)
	if (forwchar(FALSE,1) == FALSE) return(FALSE);
    i = 0;
    while (inword() == TRUE || lgetc(curwp->w_dotp,curwp->w_doto) == '-') {
	tname[i++] = lgetc(curwp->w_dotp,curwp->w_doto);
	if (forwchar(FALSE,1) == FALSE) return(FALSE);
	}
    tname[i] = '\000';
    /* restore the buffer position */
    curwp->w_dotp = dotp;
    curwp->w_doto = doto;
    if (i == 0) return(FALSE);

    /* now open the tags file */
    if ((fp = fopen("tags","r")) == NULL) {
	mlwrite("No tags file found");
	return(FALSE);
	}

    /* now search for the tag in the file */
    fname[0] = '\000';
    ptr = &istr[0];
    while (fgets(ptr,127,fp) != NULL) {
	if (*ptr == tname[0] && strncmp(ptr,tname,i) == 0) {
	    ptr += i;
	    sscanf(ptr," %s %127c",fname,sstr);
	    break;
	    }
	}
    fclose(fp);
    if (fname[0] == '\000') {
	mlwrite("Tag not found in tag file");
	return(FALSE);
	}

    /* put the window up */
    if ((wp = wpopup()) != NULL) {
	curwp = wp;
	curbp = wp->w_bufp;
	}

    /* read the file in */
    if (! getfile(fname, FALSE)) return(FALSE);

    /* scan through and update mode lines of all windows */
    wp = wheadp;
    while (wp != NULL) {
	wp->w_flag |= WFMODE;
	wp = wp->w_wndp;
	}

    /* get rid of delimeters */
    ptr = sstr + strlen(sstr);
    while (ptr != sstr && *ptr != '/') ptr--;
    if (*ptr == '/') *ptr = '\000';
    while ((ptr = index(sstr,'*')) != NULL) *ptr = '.';
    while ((ptr = index(sstr,'+')) != NULL) *ptr = '.';
    while ((ptr = index(sstr,'[')) != NULL) *ptr = '.';
    while ((ptr = index(sstr,']')) != NULL) *ptr = '.';
    if ((ptr=index(sstr,'/')) != NULL) ptr++;
    else			       ptr = sstr;

    /* do the scan (leave pointer at beginning of string) */
    strcpy(pat,ptr);
    gotobob(FALSE,1);
    compile_bcode();
    return(dosearch(TRUE,1,FALSE));
    }


getfile(fname, lockfl)
char fname[];	    /* file name to find */
int lockfl;	/* check the file for locks? */

    {
    register BUFFER *bp;
    register WINDOW *wp;
    register LINE   *lp;
    register int    i;
    register int    s;
    char bname[NBUFN];	/* buffer name to put file */

    for (bp=bheadp; bp!=NULL; bp=bp->b_bufp) {
	if ((bp->b_flag&BFTEMP)==0 && strcmp(bp->b_fname, fname)==0) {
	    swbuffer(bp);
	    lp = curwp->w_dotp;
	    i = curwp->w_ntrows/2;
	    while (i-- && lback(lp)!=curbp->b_linep) lp = lback(lp);
	    curwp->w_linep = lp;
	    curwp->w_flag |= WFMODE|WFHARD;
	    mlwrite("[Old buffer]");
	    return (TRUE);
	    }
	}
    makename(bname, fname);                 /* New buffer name. */
    while ((bp=bfind(bname, FALSE, 0)) != NULL) {
	s = mlreply("Buffer name: ", bname, NBUFN);
	if (s == ABORT)                 /* ^G to just quit */
		 return (s);
	if (s == FALSE) {               /* CR to clobber it */
		 makename(bname, fname);
	    break;
	    }
	}
    if (bp==NULL && (bp=bfind(bname, TRUE, 0))==NULL) {
	mlwrite("Cannot create buffer");
	return (FALSE);
	}
    if (--curbp->b_nwnd == 0) {             /* Undisplay. */
		 curbp->b_dotp = curwp->w_dotp;
	curbp->b_doto = curwp->w_doto;
	curbp->b_markp = curwp->w_markp;
	curbp->b_marko = curwp->w_marko;
	}
    curbp = bp;                             /* Switch to it. */
    curwp->w_bufp = bp;
    curbp->b_nwnd++;
    return(readin(fname, lockfl));          /* Read it in. */
    }
#endif
