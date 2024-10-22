/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* |_o_o|\\ Copyright (c) 1989 The Software Distillery.  All Rights Reserved */
/* |. o.| || This program may not be distributed without the permission of   */
/* | .	| || the authors:			      BBS: (919) 382-8265    */
/* | o	| ||   Dave Baker      Alan Beale	  Jim Cooper		     */
/* |  . |//    Jay Denebeim    Bruce Drake	  Gordon Keener 	     */
/* ======      John Mainwaring Andy Mercier	  Jack Rouse		     */
/*             John Toebes     Mary Ellen Toebes  Doug Walker   Mike Witcher */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**	(C) Copyright 1989 Commodore-Amiga, Inc.
 **	    All Rights Reserved
**/

/*-----------------------------------------------------------------------*/
/* Command: Edit2.c							 */
/* Author:  John A. Toebes, VIII					 */
/* Change History:							 */
/*  Date    Person	  Action					 */
/* -------  ------------- -----------------				 */
/* 25NOV90  John Toebes   Corrected TL command to print out line numbers */
/* 03NOV89  John Toebes   Initial Creation				 */
/* 17APR91  John Toebes   Corrected PA command                           */
/* Notes:								 */
/*-----------------------------------------------------------------------*/
#include "edit.h"

void edit(n)
int n;
{
   BOOL counting;
   int count, countp;
   int qual, i, a1, a2;
   int e, s, c, p, q, t;
   LEVELBUF level;
   void (*movefunc)(void);

   counting = FALSE;
   count = countp = 0;
   e = s = c = p = q = 0;

editlab:
   if (n==0)
   {
      if (setjmp(level)) goto editlab;
      editlevel = &level;
      readcommline();
   }
   counting = FALSE;
   /* repeat loop to get commands */
   for(;;)
   {
again:
      if (CheckSignal(SIGBREAKF_CTRL_C)) error(ERROR_BRK);
      nextcomm();
      quiet = !verifying;
      deleting = FALSE;
      repeating = FALSE;

sw:   sw_comm = comm;
      switch(comm)
      {
	 default:
	    error(ERROR_UC, comm);

	 case '0':case '1':case '2':case '3':case '4':
	 case '5':case '6':case '7':case '8':case '9':
	    count = commreadn();

	    if ((count == 0) &&
		(zerolevel == 0))
	       zerolevel = editlevel;

	    countp = commpoint;
	    counting = TRUE;
	 case ' ': case ';':
	    goto again;

	 case '[': case '(':
	    edit(n+1);
	    break;

	 case ']':case ')':
	    if (n <= 0) error(ERROR_BRA);
	    return;

	 /* end of file or Q -> drop back one level */
	 case ENDSTREAMCH: case 'Q':
	    if (cfsp != 0)
	    {
	       revertcom();
	       return;
	    }
	 /* Drop to here if primary stream */
	 case 'W' :
	    nextcomm();
	    if (comm != '\n' && comm != ENDSTREAMCH)
	       error(ERROR_QW, sw_comm);
	    move(MAXINT);
	    closestreams();
	    windup();
	    while(cfsp != 0) revertcom();
	    longjmp(*quitlevel, QUITLAB);
	    break;

	 case '\\':
	    while (comm != '\n' && comm != ';')
	       commrdch();
	 case '\n':

nl:	    if (n != 0) error(ERROR_BRA);
	    if (IsInteractive(edits))
	    {
		if (quiet || unchanged) {
		   PutStr(":");
		   Flush(Output());
		}
		else
		   ver(sw_comm, '\0');
	    }
	    goto editlab;

	 case '?': case '!':
	    nextcomm();
	    if ((comm == '\n') && IsInteractive(edits))
	    {
	       quiet = FALSE;
	       unchanged = FALSE;
	       goto nl;
	    }
	    uncommrdch();
	    ver(sw_comm, '\n');
	    break;

	 case '>':
	    /* Increment pointer */
	    incrementp();
	    break;

	 case '<':
	    /* Decrement pointer */
	    if (pointer == 0) break;
	    pointer = pointer - 1;
	    unchanged = FALSE;
	    break;

	 case '#':
	    /* Delete current character */
	    if (incrementp())
	    {
	       linev[pointer] = -1;
	       condensed = FALSE;
	       nosubs = FALSE;
	    }
	    break;

	 case '_':
	    /* Turn character at pointer into a space */
	    if (incrementp())
	    {
	       linev[pointer] = ' ';
	       nosubs = FALSE;
	    }
	    break;

	 case '%':
	    /* Upper case character at pointer */
	    if (incrementp())
	    {
	       if (linev[pointer] >= 'a' &&
		   linev[pointer] <= 'z')
	       {
		  linev[pointer] += 'A'-'a';
		  nosubs = FALSE;
	       }
	     }
	     break;

	 case '$':
	    /* Lower case character at pointer */
	    if (incrementp())
	    {
	       if (linev[pointer] >= 'A' &&
		   linev[pointer] <= 'Z')
	       {
		  linev[pointer] += 'a'-'A';
		  nosubs = FALSE;
	       }
	    }
	    break;

	 case 'V':
	    /* Set verification on/off */
	    verifying = readplusminus();
	    break;

	 case 'Z':
	    /* Set input terminator to string t */
	    n = z_match[0];
	    getstring( z_match, FALSE);
	    if (z_match[0] == 0)  /* No new string */
	    {
	       z_match[0] = n;
	       error(ERROR_CNTX, 'Z');
	    }
	    break;

	 case '=':
	    renumber(numarg(FALSE,FALSE, 0));
	    break;

	 case 'N':
	    nextline();
	    break;

	 case 'M':
	    /* Move to line.  */
	    /* M+ Move to highest line in memory */
	    /* M- Move to lowest line in memory */
	    /* Mn Move to line n */
	    commrdch();
	    if (comm == '+')
	       while(currentline->l_next != NULL) nextline();
	    else if (comm == '-')
	       while(currentline->l_prev != NULL) prevline();
	    else
	    {
	       uncommrdch();
	       move(numarg(FALSE,FALSE, 0));
	    }
	    break;

	 case 'I':
	    /* Insert material from terminal before line */
	    quiet = TRUE;
	    move(numarg(FALSE,TRUE,current));
	    insert();
	    break;

	 case 'T':
	    /* TR+- Set/unsed trailing space removal	 */
	    /* TO   Revert to original destination	 */
	    /* TO t Place output lines in file t	 */
	    /* TP   M-, then type to last line in buffer */
	    /* TN   Type intil buffer changed		 */
	    /* T    Type to end of file 		 */
	    /* Tn   Type n lines			 */
	    /* TLn  Type n lines with line numbers	 */
	    commrdch();
	    c = comm;
	    switch(c)
	    {
	       case 'R':
		  trailing = readplusminus();
		  break;
	       case 'O': /* Switch output */
		  changeout();
		  break;


	       case 'P':
		  while (currentline->l_prev != NULL)
		     prevline();
	       case 'N':
		  e = maxplines;
		  goto tlab;

	       default:
		  checkvalidchar();
	       case 'L':
		  e =  numarg(TRUE,TRUE,MAXINT);
tlab:		  quiet = TRUE;
		  for (i = 0; i < e; i++)
		  {
		     if ((linel != NULL) || !exhausted)
		     {
			if (c=='L')
			{
			   if (current == -1)
			      PutStr("  ******  ");
			   else
			      VPrintf("%5ld  ", &current);
			}
			verline('?');
		     }
		     if (exhausted) break;
		     nextline();
		  }
		  unchanged = FALSE;
	    }
	    break;

	 case 'H':
	    /* Hn Set halt at line n.  If n = * then halt and unset h */
	    ceiling = numarg(FALSE, FALSE, 0);
	    break;

	 case 'C':
	    /* C t  Take commands from file t				*/
	    /* CL t Concatenate current line, string t, and next line	*/
	    /* CG n Cancel global n					*/
	    /* CF t Close file t					*/

	    commrdch();
	    switch(comm)
	    {
	       default:
		  uncommrdch();
		  changecom();
		  edit(0);
		  break;
	       case 'L':
		  getstring( svec, FALSE ); /* Get unqualified */
		  subst( linel, linel, svec ); /* at end of line */
		  compress();
		  concatenate();
		  break;

	       case 'G':
		  alterglobal( G_CANCEL );
		  break;

	       case 'F':
		  closefile();
		  break;
	    }
	    break;

	 case 'S':
	    /* SA qs  Split line after qs	*/
	    /* SB qs  Split line before qs	*/
	    /* SHD    Show data 		*/
	    /* SHG    Show globals		*/
	    /* STOP   Stop			*/
	    commrdch();
	    switch(comm)
	    {
	       default:
		  error(ERROR_UDC, 'S', comm);

	       case 'A': case 'B':
		  c = (comm == 'A'); /* TRUE is split after */
		  qual = getstring(svec, TRUE);
		  compress();
		  e = index(linev, pointer, linel, svec, qual);
		  if (e < 0) error(ERROR_NOM);
		  if (c) e += svec[0];
		  split(e);
		  break;

	       case 'H':
		  commrdch();
		  switch(comm)
		  {
		     default:
			error(ERROR_UDC);

		     case 'D': /* Show data */
			PutStr("' cmd: ");
			if (str_comm == C_NC)
			   PutStr("unset");
			else
			{
			   p = str_comm >> 8;
			   if (p != 0) FPutC(Output(),(UBYTE)p);
			   FPutC(Output(),(UBYTE)str_comm);
			   PutStr(" ");
			   showdata( str_qual,str_match);
			   showdata( NO_QUAL, str_repl );
			}
			PutStr("\nSearch string: ");
			if (f_qual == C_NC)
			   PutStr("unset");
			else
			   showdata(f_qual,f_match);
			PutStr("\nInput terminator: ");
			showdata( NO_QUAL, z_match);
			PutStr("\n");
			break;

		     case 'G': /* Show globals */
			for(i = 1; i <= globcount; i++)
			{
			   long subs[4];
			   t = g_type[i];
			   subs[0] = i;
			   subs[1] = (t<0) ? 'D' : ' ';
			   subs[2] = g_count[i];
			   subs[3] = (t<0) ? -t : t;
			   VPrintf("%2ld %lc %3ld G%lc", subs);
#if 0
			   VPrintf("%2ld %lc %3ld G%lc",
				    i,(t<0)?'D':' ', g_count[i], ABS(t));
#endif
			   showdata( g_qual[i], g_match[i]);
			   showdata( NO_QUAL, g_repl[i] );
			   PutStr("\n");
			}
		  }
		  break;

	       case 'T':  /* STOP command; check in full */
		  if (commrdch() != 'O') error(ERROR_UDC);
		  if (commrdch() != 'P') error(ERROR_UDC);
		  closestreams();
		  while (cfsp != NULL) revertcom();
		  /* could set rc here */
		  longjmp(*quitlevel, QUITLAB );

	    }
	    break;

	 case 'P':
	    /* P     Previous line				*/
	    /* PA qs Move character pointer to after qs 	*/
	    /* PB qs Move character pointer to before qs	*/
	    /* PR    Reset character pointer to start of line	*/
	    commrdch();
psw:	    switch(comm)
	    {
	       default:
		  uncommrdch();
		  prevline();
		  break;

	       case 'A': case 'B': /* Point to ( after or before ) */
		  c = comm;
		  qual = getstring( svec, TRUE );
		  compress();
		  e = index(linev, pointer, linel, svec, qual);
		  if (e<0) error(ERROR_NOM);
		  if (c == 'A') e += svec[0];
		  pointer = e;
		  nosubs = FALSE;
		  break;

	       case 'R': /* Pointer reset */
		  condense();
		  if (pointer != 0) unchanged = FALSE;
		  pointer = 0;
		  break;
	    }
	    break;

	 case 'B':
	    /* B qs t  Place string t before qs 		*/
	    /* BP qs t Same as B, but move character pointer	*/
	    /* BF qs   Backward find sting qs			*/
	    commrdch();
	    if (comm == 'F')
	    {
	       movefunc = prevline;
	       goto fsw;
	    }
	    goto abesw;

	 case 'E':
	    /* E qs t  Exchange string qs with string t 	*/
	    /* EP qs t Same as E but move character pointer.	*/
	    /* EG n    Enable global n (all if n omitted)       */
	    if (commrdch() == 'G')
	    {
	       alterglobal( G_ENABLE );
	       break;
	    }
	    goto abesw;

	 case 'A':
	    commrdch();
abesw:	    c = FALSE;
	    if (comm == 'P') /* EP or AP or BP */
	       c = TRUE;    /* Flag if P found */
	    else
	       uncommrdch();
	    abe_args(sw_comm);
	    compress();
	    p = index(linev, pointer,linel, str_match, str_qual);
	    if (p<0) error(ERROR_NOM);
	    /* Increment pointer if reqrd */
	    if (c)
	    {
	       /* Always add P+length replacement */
	       pointer = p+str_repl[0];
	       if (str_comm != 'E') pointer += str_match[0];
	    }
	    q = p+str_match[0];
	    if (str_comm == 'B') q = p;
	    if (str_comm == 'A') p = q;
	    subst(p, q, str_repl);
	    break;

	 case 'F':
	    /* FROM    Take source from original	*/
	    /* FROM t  Take source from file t		*/
	    /* F qs    Find string qs			*/
	    if (commrdch() =='R')
	    {
	       /* FROM command */
	       if (commrdch() != 'O' || commrdch() !='M')
		  error( ERROR_UDC);
	       changein();
	       break;
	    }
	    /* Find command */
	    uncommrdch();
	    movefunc = nextline;
	    /* p is nextline or prevline; set to prev for BF */
fsw:	    lf_arg(comm);
	    compress();
	    while (index(linev, pointer, linel, f_match, f_qual) < 0)
	       (*movefunc)();
	    break;

	 case 'G':
	    /* GA qs t	Globally place t after qs	*/
	    /* GB qs t	Globally place t before qs	*/
	    /* GE qs t	Globally exchange qs for t	*/
	    readglobal();
	    break;

	 case 'D':
	    /* D      Delete current line				*/
	    /* DF qs  Find string qs, delete lines as they are passed	*/
	    /* DFA qs Delete from after qs to end of line		*/
	    /* DFB qs Delete from before qs to end of line		*/
	    /* DTA qs Delete from start of line to after qs		*/
	    /* DTB qs Delete from start of line to before qs		*/
	    commrdch();
	    switch(comm)
	    {
	       case 'F':
		  /* Could be DFA or DFB */
		  s = FALSE; /* In case next test is true */
		  commrdch();
		  if (comm == 'A' || comm == 'B') goto dtsw;
		  uncommrdch(); /* Its not! */
		  deleting = TRUE;
		  quiet = TRUE;
		  movefunc = nextline;
		  goto fsw;

	       case 'T':
		  s = TRUE; /* Flags if DT */
		  commrdch();
		  if ((comm != 'A') && (comm != 'B'))
		     error( ERROR_UDC );
dtsw:		  c = comm;
		  qual = getstring( svec, TRUE );
		  compress();
		  e = index(linev, pointer,linel, svec, qual );
		  if (e < 0) error(ERROR_NOM);
		  if (c == 'A') e += svec[0];
		  if (e != pointer)
		  {
		     if (s)
		     {
			/* Delete TO */
			for (i = 0; i < linel-e; i++)
			   linev[pointer+i] = linev[e+i];
			linel = pointer+linel-e;
		     }
		     else
		     {
			/* DF case */
			linel = e;
		     }
		     nosubs = FALSE;
		  }
		  break;

	       case 'G':
		  /* Disable global */
		  alterglobal( G_DISABLE );
		  break;

	       default:
		  checkvalidchar();
		  goto drlab;
	    }
	    break;

drlab:
	 case 'R':
	    /* R    Replace material from terminal		*/
	    /* R t  Replace material from file t		*/
	    /* REWI Rewind input file				*/
	    if (commrdch() == 'E')
	    {
	       /* Rewind */
	       if (commrdch() != 'W' || commrdch() != 'I')
		  error(ERROR_UDC);

	       /* flush rest of line without comment */
	       while(comm >= 'A' && comm <= 'Z') commrdch();
	       uncommrdch();
	       move( MAXINT );
	       closestreams();
	       rewind();
	       openstreams();
	       break;
	    }
	    /* Replace */
	    checkvalidchar();

	    a1 = numarg(FALSE,TRUE,current);
	    a2 = numarg(FALSE,TRUE,a1);
	    if (sw_comm=='R') quiet = TRUE;
	    move(a1);
	       deleting = TRUE;
	       quiet = TRUE;
	       move(a2);
	       if (exhausted)
	       {
		  linel = 0;
		  pointer = 0;
		  unchanged = FALSE;
		  cch = ENDSTREAMCH;
	       }
	       else
	       {
		   nextline();
	       }
	       if (sw_comm=='R') insert();
	       break;

	 case '"':
	    nextline();

	 case '\'':
	    repeating = TRUE;
	    comm = str_comm;
	    goto sw;

	 case C_NC:
	 /* Unknown command */
	    error(ERROR_REP);
      }

      if (nosubs == 0) unchanged = FALSE;

      if (counting)
      {
	 if (count != 0)
	 count--;
	 if (count == 0)
	 {
	    counting = FALSE;
	    goto again;
	 }
	 commpoint = countp;
      }
   }
}
