#include "ed.h"

/*********************************
 * Search for the next occurence of the character at dot.
 * If character is a (){}[]<>, search for matching bracket, taking
 * proper account of nesting.
 * Written by Walter Bright.
 */

static int searchstate,icol;

int searchparen(f, n)
int f,n;
    {
    LINE *clp;
    int cbo;
    int len;
    int i;
    UBYTE chinc,chdec,ch;
    int count;
    int forward;
    int s=TRUE;
    LINE *line;
    UBYTE buffer[256];
    int tlen;
    static UBYTE bracket[][2] = {{'(',')'},{'<','>'},{'[',']'},{'{','}'}};

    clp = curwp->w_dotp;		/* get pointer to current line	*/
    cbo = curwp->w_doto;		/* and offset into that line	*/
    count = 0;

    len = llength(clp);

    if (cbo >= len)chinc = '\n';
    else chinc = lgetc(clp,cbo);

    forward = TRUE;			/* assume search forward	*/
    chdec = chinc;
    for (i = 0; i < 4; i++)
	if (bracket[i][0] == chinc)
	{	chdec = bracket[i][1];
		break;
	}
    for (i = 0; i < 4; i++)
	if (bracket[i][1] == chinc)
	{	chdec = bracket[i][0];
		forward = FALSE;	/* search backwards		*/
		break;
	}

    searchstate = 0;       /* start state for ignore */
    
    while (1)
    {
	if (forward)
	{
	    if (cbo >= len)
	    {	/* proceed to next line */
		clp = lforw(clp);
		if (clp == curbp->b_linep)	/* if end of buffer	*/
		    break;
		len = llength(clp);
		cbo = 0;
	    }
	    else
		cbo++;
	}
	else /* backward */
	{
	    if (cbo == 0)
            {
		clp = lback(clp);
		if (clp == curbp->b_linep)
		    break;
		len = llength(clp);
		cbo = len;
            }
	    else
		--cbo;
	}

	ch = (cbo < len) ? lgetc(clp,cbo) : '\n';
	if (!searchignore(ch,forward)) 
	    {
	    if (eq(ch,chdec)) {
		if (count-- == 0) {
		    /* We have found it	*/
		    if(f) { /* called while user is typing */
			s=FALSE;
			for (i=0, line=curwp->w_linep; i < curwp->w_ntrows;
				 i++,line=lforw(line)) {
			    if(clp==line) {
				s=TRUE;
				break;
			    }
                	    if (line == curbp->b_linep) {
				break;	/* if end of buffer */
			    }
			}
		    }
		    if(s) {
	                curwp->w_dotp  = clp;
	                curwp->w_doto  = cbo;
	                curwp->w_flag |= WFMOVE;
		    }
		    else {	
/*			tlen= (clp->l_used > 255) ? 255 : clp->l_used; */
			tlen=llength(clp)-1;
			memset(&buffer,0,255);
			memcpy(buffer,clp->l_text,tlen);
			mlwrite("%s",clp->l_text);

			icol=getindent(clp); /* get indent factor for insert */
			return(FALSE);
		    }
		    icol=getindent(clp); /* get indent factor for auto insert */
	            return (TRUE);
		}
	    }
	else if (eq(ch,chinc)) count++;
        }
    }
    (*term.t_beep)();
    icol=0;
    mlwrite("Parenthesis mismatch");
    (*term.t_beep)();
    return (FALSE);
}

/* this routine determines when to ignore characters */
/* ie. those in comments and those in quotes         */
static searchstatus;

struct statetrans {
    int bslash;
    int fslash;
    int quote;
    int dquote;
    int star;
    int nl;
    int other;
    int ignore;
}  statetrans[] =     { /* bsl fsl quo dqu sta  nl oth ign */
    /* 0 normal         */  {0, 1,  4,  6,  0,  0,  0,  0},
    /* 1 normal seen /  */  {0, 1,  4,  6,  2,  0,  0,  0},
    /* 2 comment        */  {2, 2,  2,  2,  3,  2,  2,  1},
    /* 3 comment seen * */  {2, 0,  2,  2,  3,  2,  2,  1},
    /* 4 quote          */  {5, 4,  0,  4,  4,  0,  4,  1},
    /* 5 quote seen \   */  {4, 4,  4,  4,  4,  4,  4,  1},
    /* 6 string         */  {7, 6,  6,  0,  6,  0,  6,  1},
    /* 7 string seen \  */  {6, 6,  6,  6,  6,  6,  6,  1},

    /* backwards state diagram */ 
                        /* bsl fsl quo dqu sta  nl oth ign */
    /* 0 normal         */  {0, 1,  4,  6,  0,  0,  0,  0},
    /* 1 normal seen /  */  {0, 1,  4,  6,  2,  0,  0,  0},
    /* 2 comment        */  {2, 2,  2,  2,  3,  2,  2,  1},
    /* 3 comment seen * */  {2, 0,  2,  2,  3,  2,  2,  1},
    /* 4 quote          */  {4, 4,  5,  4,  4,  5,  4,  1},
    /* 5 quote seen end */  {4, 0,  0,  0,  0,  0,  0,  0},
    /* 6 string         */  {6, 6,  6,  7,  6,  7,  6,  1},
    /* 7 string seen end*/  {6, 0,  0,  0,  0,  0,  0,  0}
};

static int searchignore(ch, forward)
    int ch;
    int forward;
{
    int lss = searchstate;      /* local search state */
    

    /* if not cmode check all chars */
   /* all modes for now */
/*    if (!(curbp->b_mode & MDCMOD)) return (0); */

    if (!forward) lss += 8;     /* switch to backward table */

    switch(ch) {
    case '\\': searchstate = statetrans[lss].bslash; break;
    case '/' : searchstate = statetrans[lss].fslash; break;
    case '\'': searchstate = statetrans[lss].quote;  break;
    case '"' : searchstate = statetrans[lss].dquote; break;
    case '*' : searchstate = statetrans[lss].star;   break;
    case '\n': searchstate = statetrans[lss].nl;     break;
    default  : searchstate = statetrans[lss].other;  break;
    }    
    return(statetrans[lss].ignore);
}

int fencematch(ch)
    UBYTE ch;
{
int s,col,currcol;
#if !AMIGA
int i;
#endif

    backchar(FALSE, 1);
    s=searchparen(TRUE,0);
    col=icol;	/* pick off proper indent column */
    if(s) {
	update();
#if AMIGA
    	Delay(4);
#else
	for(i=0; i<100000; i++);
#endif
    	searchparen(TRUE,0);
    }
    ldelete(1,FALSE);	/* delete the bracket */
    if((currcol=getccol(FALSE)) < col) {
	doindent(col-currcol);
    }
    linsert(1,ch);
    return(s);
}

