/*
 * The functions in this file
 * implement commands that search in the
 * forward and backward directions. There are
 * no special characters in the search strings.
 * Probably should have a regular expression
 * search, or something like that.
 */
#include	"ed.h"

extern int CheckCase;

/*
 * Search forward.
 * Get a search string from the
 * user, and search, beginning at ".",
 * for the string. If found, reset the
 * "." to be just after the match string,
 * and [perhaps] repaint the display.
 * Bound to "M-S".
 */
int forwsearch(f, n)
    int f,n;
{
	 int	s;

	if ((s=readpattern("Search",pat)) != TRUE)return (s);
	s=forsearch(f,n);
	return(s);
}


int forsearch(f,n)
int f,n;
{
	 LINE	*clp;
	 int	cbo;
	 LINE	*tlp;
	 int	tbo;
	 int	c;
	 char	*pp;

	clp = curwp->w_dotp;
	cbo = curwp->w_doto;
	while (clp != curbp->b_linep)  {
		if (cbo == llength(clp)) {
			clp = lforw(clp);
			cbo = 0;
			c   = '\n';
		} 
		else c = lgetc(clp, cbo++);
		if (eq(c, pat[0]) != FALSE) {
			tlp = clp;
			tbo = cbo;
			pp  = &pat[1];
			while (*pp != 0) {
				if (tlp == curbp->b_linep) goto fail;
				if (tbo == llength(tlp)) {
					tlp = lforw(tlp);
					if (tlp == curbp->b_linep) goto fail;
					tbo = 0;
					c   = '\n';
				} 
				else c = lgetc(tlp, tbo++);
				if (eq(c, *pp++) == FALSE) goto fail;
			}
			curwp->w_dotp  = tlp;
			curwp->w_doto  = tbo;
			curwp->w_flag |= WFMOVE;
			return (TRUE);
		}
	fail:	;
	}
	mlwrite("Not found");
	return (FALSE);
}

/*
 * Reverse search.
 * Get a search string from the
 * user, and search, starting at "."
 * and proceeding toward the front of
 * the buffer. If found "." is left
 * pointing at the first character of
 * the pattern [the last character that
 * was matched]. Bound to "M-R".
 */
int backsearch(f, n)
    int f,n;
    {
	 LINE	*clp;
	 int	cbo;
	 LINE	*tlp;
	 int	tbo;
	 int	c;
	 char	*epp;
	 char	*pp;
	 int	s;

	if ((s=readpattern("Reverse search",pat)) != TRUE)
		return (s);
	for (epp = &pat[0]; epp[1] != 0; ++epp);
	clp = curwp->w_dotp;
	cbo = curwp->w_doto;
	for (;;) {
		if (cbo == 0) {
			clp = lback(clp);
			if (clp == curbp->b_linep) {
				mlwrite("Not found");
				return (FALSE);
			}
			cbo = llength(clp)+1;
		}
		if (--cbo == llength(clp)) c = '\n';
		else c = lgetc(clp, cbo);
		if (eq(c, *epp) != FALSE) {
			tlp = clp;
			tbo = cbo;
			pp  = epp;
			while (pp != &pat[0]) {
				if (tbo == 0) {
					tlp = lback(tlp);
					if (tlp == curbp->b_linep)
						goto fail;
					tbo = llength(tlp)+1;
				}
				if (--tbo == llength(tlp)) c = '\n';
				else c = lgetc(tlp, tbo);
				if (eq(c, *--pp) == FALSE) goto fail;
			}
			curwp->w_dotp  = tlp;
 			curwp->w_doto  = tbo;
			curwp->w_flag |= WFMOVE;
			return (TRUE);
		}
	fail:	;
	}
}

/*
 * Compare two characters.
 * The "bc" comes from the buffer.
 * It has it's case folde out. The
 * "pc" is from the pattern.
 */
int eq(bc, pc)
 int	bc;
 int	pc;
{
if(!CheckCase) {
	if (bc>='a' && bc<='z') bc -= 0x20;
	if (pc>='a' && pc<='z') pc -= 0x20;
}
	if (bc == pc) return (TRUE);
	return (FALSE);
}

/*
 * Read a pattern.
 * Stash it in the external
 * variable "pat". The "pat" is
 * not updated if the user types in
 * an empty line. If the user typed
 * an empty line, and there is no
 * old pattern, it is an error.
 * Display the old pattern, in the
 * style of Jeff Lomicka. There is
 * some do-it-yourself control
 * expansion.
 */

int readpattern(prompt,pattern)
char	*prompt;
char	*pattern;
{
	 char	*cp1;
	 char	*cp2;
	 int	c;
	 int	s;
	char		tpat[NPAT+20];

	cp1 = &tpat[0];				/* Copy prompt		*/
	cp2 = prompt;
	while ((c = *cp2++) != '\0')
		*cp1++ = c;
	if (pattern[0] != '\0') {	/* Old pattern		*/
		*cp1++ = ' ';
		*cp1++ = '[';
		cp2 = &pattern[0];
		while ((c = *cp2++) != 0) {
			if (cp1 < &tpat[NPAT+20-6]) {	/* "??]: \0"	*/
				if (c<0x20 || c==0x7F) {
					*cp1++ = '^';
					c ^= 0x40;
				} 
				else if (c == '%') *cp1++ = c;	/* Map "%" to	*/
				*cp1++ = c;			/* "%%".	*/
			}
		}
		*cp1++ = ']';
	}
	*cp1++ = ':';				/* Finish prompt	*/
	*cp1++ = ' ';
	*cp1 = '\0';
	s = mlreply(tpat, tpat, NPAT);		/* Read pattern		*/
	if (s == TRUE) strcpy(pattern, tpat);		/* Specified		*/
	else if (s==FALSE && pattern[0])s = TRUE;	/* CR, but old one	*/
	return (s);
}

int readrpat(prompt)
char    *prompt;
{
         char   *cp1;
         char   *cp2;
         int    c;
         int    s;
        char            tpat[NPAT+20];

        rpat[0]='\0';
        cp1 = &tpat[0];                         /* Copy prompt          */
        cp2 = prompt;
        while ((c = *cp2++) != '\0')
                *cp1++ = c;
        *cp1++ = ':';                           /* Finish prompt        */
        *cp1++ = ' ';
        *cp1 = '\0';
        s = mlreply(tpat, tpat, NPAT);          /* Read pattern         */
        if (s != ABORT) strcpy(rpat, tpat);
        return (s);
}

/* forward search with query option */

int qforwsearchr(f ,n)
    int f,n;
{
 int    s;

QueryFlag=TRUE;
s=forwsearchr(f ,n);
QueryFlag=FALSE;
return(s);
}

/* search & replace */

int forwsearchr(f, n)
    int f,n;
{
 int i;
 int s=TRUE,sr;
 int savecase=CheckCase;

  int oldcol,oldrow;
  int nlpresent=FALSE,nlflag=2;
  int reps=0;

oldcol=curwp->w_doto;
oldrow=getcline();
CheckCase=TRUE;

if ( forwsearch(f, n) != TRUE ) { 
	CheckCase=savecase;
	return(FALSE); /* ask the first time */
}
mlerase();

if(QueryFlag==FALSE) {
        if(readrpat("Replace") == ABORT) {
	    CheckCase=savecase;
	    return(FALSE);
	}
}
else if(readrpat("Query Replace") == ABORT) {
	CheckCase=savecase;
	return(FALSE);
}
for(i=0; i <strlen(rpat); i++)if(rpat[i]=='\n')nlpresent=TRUE;
for(i=0; i <strlen(pat); i++)if(pat[i]=='\n')nlpresent=TRUE;

do {

    if(QueryFlag==TRUE) {
        update();
        if((s=mlquery("Change string")) == CONT) {
		s = TRUE;
		QueryFlag=FALSE;
	}
        curwp->w_flag |= WFEDIT;
        update();
    }
    if (( s==TRUE ) || ( QueryFlag==FALSE )) {
	backchar(FALSE,strlen(pat));
        s=ldelete( strlen(pat), FALSE);
	if(s==FALSE)break;
            for(i=0; i<strlen(rpat); i++) {
		if( !( (rpat[i] == '\n') ? lnewline() : linsert(1,rpat[i])) ) {
			mlwrite("Out of memory");
			s=ABORT;
			break;
		}
	    }
	reps++;
    }

    if(nlpresent) {
	if( (lforw(curwp->w_dotp)==(curwp->w_bufp->b_linep)) ) {
		nlflag--;
	}
    }
    sr=forsearch(f,n);
} while ( sr && (s != ABORT) && (nlflag) );

mlerase();
curwp->w_flag |= WFHARD;
update();

gotobob(0,0);	/* reposition cursor */
forwline(TRUE,oldrow);
forwchar(TRUE,oldcol);
update();

(*term.t_flush)();
if(!reps)mlwrite("No replacements done");
else if (reps==1)mlwrite("Replaced 1 occurrence");
else mlwrite("Replaced %d occurrences",reps);
CheckCase=savecase;
return(TRUE); /* all done */
}
