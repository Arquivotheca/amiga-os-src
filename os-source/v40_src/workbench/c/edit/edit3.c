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
/* Command: Edit3.c							 */
/* Author:  John A. Toebes, VIII					 */
/* Change History:							 */
/*  Date    Person	  Action					 */
/* -------  ------------- -----------------				 */
/* 03NOV89  John Toebes   Initial Creation				 */
/* 17APR91  John Toebes   Corrected delimiter list to allow .		 */
/* Notes:								 */
/*-----------------------------------------------------------------------*/
#include "edit.h"


void checkvalidchar()
{
   if (comm == ' '  ||
       comm == '\n' ||
       comm == ';'  ||
       comm == '*'  ||
       comm == '.'  ||
       (comm >= '0' && comm <= '9'))
      uncommrdch();
   else
      error(ERROR_UDC, sw_comm, comm);
}


void checkspaceornl()
{
   if (comm == ' '  || comm == '\n' || comm == ';')
      uncommrdch();
   else
      error(ERROR_UDC, sw_comm, comm);
}

void readcommline()
{
   int ch;

   commlinel = 0;
   SelectInput(edits);

   while(1)
   {
      Flush(verout);
      ch = FGetC(edits);
      if (ch == '\n' || ch == '\r' || ch == '\f')
	 break;

      if (ch == ENDSTREAMCH)
      {
	 if (commlinel == 0) commlinel = -1;
	 break;
      }

      commlinel++;
      if (commlinel <= maxlinel)
	 commbuf[commlinel] = ch;
   }

   if (commlinel > maxlinel)
   {
      commlinel = maxlinel;
      PutStr("****** Command line truncated\n");
      rc = 10;
   }
   commpoint = 0;
}


int commrdch()
{
   commpoint++;
   if (commlinel == -1)
      comm = ENDSTREAMCH;
   else if (commpoint > commlinel)
      comm = '\n';
   else
   {
      comm = commbuf[commpoint];
      if (comm >= 'a' && comm <= 'z') comm += 'A'-'a';
   }

   return(comm);
}


void uncommrdch()
{
   commpoint--;
}

void nextcomm()
{
   while(commrdch() == ' ');
}

BOOL readplusminus()
{
   commrdch();
   if (comm == '+') return(TRUE);
   if (comm == '-') return(FALSE);
   error(ERROR_PM, sw_comm);
}


int commreadn()
{
   int a;

   a = 0;

   do {
      a = a*10 + comm -'0';
      commrdch();
      } while (comm >= '0' && comm <= '9');
   uncommrdch();
   return(a);
}


/* read a number argument	*/
/* '*' ==> end of document      */
/* '_' ==>  -> 1, CURRENT       */
int numarg(add, opt, def)
long add, opt, def;
{
   nextcomm();
   if (comm == '_')
      return(add ? 1 : current);
   else if (comm == '*')
      return(MAXINT);
   else if (comm >= '0' && comm <= '9')
      return(commreadn());
   else if (opt)
   {
      uncommrdch();
      return(def);
   }
   error(ERROR_NUM, sw_comm);
 }


/* read a context string argument */
void readcontext(v)
short *v;
{
   int i;

   i = 0;
   if (delim == 0) return; /* Use previous string */
   for (;;)
   {
      commrdch();
      if ((comm == delim) || (comm=='\n')) break;
      if (i >= SMAX) error(ERROR_STR);
      v[++i] = commbuf[commpoint];
   }
   v[0] = i;
   v[i+1] = 0; /* Ensure that it is nul terminated */
}


void abe_args(c)
int c;
{
   if (!repeating)
   {
      str_comm = c;
      str_qual = getstring(str_match, TRUE );
      readcontext(str_repl);
   }
}


int getstring(v, qsw)
short *v;
BOOL qsw;
{
   int q;

   commrdch();
   if ((comm >= 'A') && (comm <= 'Z'))
      error( ERROR_UDC );
   uncommrdch();
   q = getdelim( FALSE );
   if ((!qsw) || (q != NO_QUAL))
      error( ERROR_IQL);
   readcontext(v);
   return(q);
}


void lf_arg(c)
int c;
{
   commrdch();
   if ((comm >= 'A') && (comm <= 'Z'))
      error( ERROR_UDC );

   uncommrdch();
   f_qual = getdelim( TRUE );
   if ((delim == 0) && (f_qual == C_NC))
      error( ERROR_REP );
   readcontext(f_match);
}

int getdelim( flag )
BOOL flag;
{
   /* Sets global delim and returns string qualifier */
   int qual;
   BOOL ufound;
   qual = NO_QUAL; /* signal as no qualifier given */
   ufound = FALSE;
   for(;;)
   {
      commrdch();
      switch(comm)
      {
	 case ' ': break;
	 case '\n': case ';':
	    /* Repeat last command */
	    if (!flag) error( ERROR_CNTX, sw_comm );
	    if (qual != NO_QUAL) error( ERROR_IQL );
	    delim = 0;
	    return(f_qual);

	 case 'P':
	    if (!flag) error( ERROR_IQL );
	 case 'B': case 'E': case 'L':
	    if (qual != NO_QUAL) error( ERROR_IQL );
	    qual = comm; /* Valid qualifier */
	    break;
	 case 'U':  /* Upper case switch */
	    ufound = TRUE;
	    break;
	 case '/': case '+': case '-': case '*':
	 case ',': case '.': case ':': case '?':
	    delim = comm;   /* Valid delimiter */
	    return(ufound ? -qual : qual);
	 default:
	    error( ERROR_UQL );
      }
   }
}


/* read a file title argument */
int readfiletitle(v)
char *v;
{
   int len;
   int i;

   nextcomm(); /* Skip past spaces */
   if ((comm == '\n') | (comm == ';'))
   {
      v[0] = 0;
      return(0);   /* No filetitle given */
   }
   uncommrdch();   /* Put it back */
   getstring( svec, FALSE ); /* Get unqualified string */
   len = svec[0];
   if (len > FMAX) error(ERROR_STR);
   /* Copy it over, ensuring that there is a NULL at the end */
   for (i=0; i <= len+1; i++)
      v[i] = svec[i];
   return(len);
}


/* add a file spec to the file list */
struct FILESTRUC *addfilespec(v, type)
char *v;
int type;
{
   struct FILESTRUC *p;
   BPTR s;
   p = newvec(sizeof(struct FILESTRUC));

   s = Open(v+1, (type == S_IN) ? MODE_OLDFILE : MODE_NEWFILE);
   if (s == NULL) error(ERROR_FF, v);

   p->f_lk = filelist;
   filelist = p;
   memcpy(p->f_fn, v, v[0]+2);
   p->f_lc = 0;
   p->f_ex = FALSE;
   p->f_io = type;
   p->f_sp = s;
   return(p);
}


/* find a file spec in the file list */
struct FILESTRUC *findfilespec(v, type)
char *v;
int type;
{
   struct FILESTRUC *p;
   for (p = filelist; p != NULL; p = p->f_lk)
   {
      /* Is it opened with the same mode and the same name */
      if (type == p->f_io &&
	  !stricmp(p->f_fn+1, v+1))
	 /* Yes, so stop looking. */
	 break;
   }
   return(p);
}


/* close a file and remove it from the list */
void losefilespec(pf)
struct FILESTRUC *pf;
{
   struct FILESTRUC *t;
   if (pf == filelist)
      filelist = pf->f_lk;
   else
   {
      for(t = filelist; t != NULL; t = t->f_lk)
	 if (t->f_lk == pf) break;
      /* We didn't find it on the list and probably should give some */
      /* sort of error, but the BCPL one did not.  Add it later when */
      /* We are sure it doesn't use this feature.                    */
      if (t == NULL) return;
      t->f_lk = pf->f_lk;
   }
   /*Close(pf->f_sp); */
   if((pf->f_io) == S_IN)closein(pf->f_sp);
   else closeout(pf->f_sp);
   discardvec(pf);
}


void closefile()
{
   char v[FMAX];
   struct FILESTRUC *e;

   if (readfiletitle(v) == 0) error(ERROR_FNX);
   e = findfilespec(v, S_OUT);
   if (e != NULL)
   {
      if (e==currentoutput)
      {
	 while(oldestline != currentline) writeline();
	 currentoutput = primaryoutput;
	 textout = currentoutput->f_sp;
      }
      losefilespec(e);
   }
   e = findfilespec(v, S_IN);
   if (e != NULL)
   {
      if (e==currentinput)
      {
	 renumber(-1);
	 currentinput = primaryinput;
	 current = currentinput->f_lc;
	 exhausted = currentinput->f_ex;
	 textin = currentinput->f_sp;
      }
      losefilespec(e);
   }
}


/* change the command input stream			*/
/* stack the current command line and its pointers	*/
void changecom()
{
   char v[FMAX];
   struct CMDFILE *f;
   BPTR e;

   if (readfiletitle(v) == 0) error(ERROR_FNX);
   if (cfsp > CFMAX) error(ERROR_CFV);
   e = Open(v+1, MODE_OLDFILE);
   if (e == NULL) error(ERROR_FF, v);
   f = newvec(sizeof(struct CMDFILE)+commlinel);
   f->cf_cp = commpoint;
   f->cf_cl = commlinel;
   f->cf_sp = edits;
   f->cf_el = editlevel;
   memcpy(f->cf_cb, commbuf, commlinel+2);
   cfstack[cfsp++] = f;
   edits = e;
}


/* revert to the previous command stream	*/
void revertcom()
{
   struct CMDFILE *f;

   /* The next Error didn't exist in the BCPL one, but I added it       */
   /* because I am not convinced they check it all the time		*/
   if (cfsp == 0) error(ERROR_CFV);
   closein(edits);
   f = cfstack[--cfsp];
   commpoint = f->cf_cp;
   commlinel = f->cf_cl;
   edits = f->cf_sp;
   editlevel = f->cf_el;
   memcpy(commbuf, f->cf_cb, commlinel+2);
   discardvec(f);
}


/* change the current output stream		*/
/* read file name and look it up		*/
/* if not found then open it			*/
void changeout()
{
   char v[FMAX];
   struct FILESTRUC *e;

   if (readfiletitle(v) == 0 ||
       ((v[0] == 1) && (v[1] == '#')))
      e = primaryoutput;
   else
   {
      e = findfilespec(v, S_OUT);
      if (e == NULL) e = addfilespec(v, S_OUT);
   }
   while (oldestline != currentline) writeline();
   currentoutput = e;
   textout = e->f_sp;
}


/* change the current input stream		*/
void changein()
{
   char v[FMAX];
   struct FILESTRUC *e;

   if (readfiletitle(v) == 0 ||
       ((v[0] == 1) && (v[1] == '#')))
      e = primaryinput;
   else
   {
      e = findfilespec(v, S_IN);
      if (e == NULL) e = addfilespec(v, S_IN);
   }
   renumber(-1);
   currentinput = e;
   textin = e->f_sp;
   if (currentline->l_next == 0)
      exhausted = e->f_ex;
}


/* Writes out info on qualified strings 	*/
void showdata( qual, string )
int qual;
short *string;
{
   int i;

   if (qual < 0)
   {
      PutStr("U");
      qual = -qual;
   }
   if (qual != NO_QUAL) FPutC(Output(), (UBYTE)qual );
   PutStr("/");
   for (i = 1; i <= string[0]; i++)
      FPutC(Output(),(UBYTE)string[i]);
}
