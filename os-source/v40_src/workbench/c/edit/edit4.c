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
/* Command: Edit4.c							 */
/* Author:  John A. Toebes, VIII					 */
/* Change History:							 */
/*  Date    Person	  Action					 */
/* -------  ------------- -----------------				 */
/* 05NOV89  John Toebes   Initial Creation				 */
/* Notes:								 */
/*-----------------------------------------------------------------------*/
#include "edit.h"

/* renumber all lines in store	*/
void renumber(n)
int n;
{
   struct LINE *l;
   current = n;

   l = currentline;
   for (l = currentline; l != NULL; l = l->l_next)
   {
      l->l_numb = n;
      if (n != -1) n++;
   }

   if (n != -1) currentinput->f_lc = n-1;
   l = currentline->l_prev;

   /* Set the numbers of everything before the current line to -1 */
   for (l = currentline->l_prev; l != NULL; l = l->l_prev)
      l->l_numb = -1;
}


/* split the current line	*/
void split(p)
int p;
{
   struct LINE *l;

   /* Get us an empty line from the pool.  Note this will never be empty */
   l = freelines;
   freelines = l->l_next;

   l->l_prev = currentline;
   l->l_next = currentline->l_next;
   if (currentline->l_next != NULL)
      currentline->l_next->l_prev = l;
   currentline->l_next = l;
   nosubs = FALSE;
   l->l_len = linel-p;
   l->l_numb = current;
   l->l_cch = cch;
   memcpy((char *)(l->l_buf), (char *)(linev+p+1), (linel-p)*CHARSIZE);
   cch = '\n';
   linel = p;
   exhausted = FALSE;
   putline();
   currentline = l;
   getline();
   if (currentline->l_next == NULL)
      exhausted = currentinput->f_ex;
   current = -1;
   nosubs = FALSE;
   /* Ensure that there is at least one empty line in the pool */
   if (freelines == NULL) writeline();
}


/* concatenate the next line	*/
void concatenate()
{
   struct LINE *l;
   int p;
   int s;

   p = pointer;
   nosubs = TRUE;
   s = linel;  /* Save the current length */
   nextline();
   putline();
   l = currentline;
   currentline = currentline->l_prev;
   getline();
   if (s > linel)
      memset((char *)(linev+linel), ' ', (s-linel)*CHARSIZE);
   linel = s;
   subst(linel, linel, &l->l_len);
   pointer = p;
   currentline->l_next = l->l_next;
   if (l->l_next != NULL)
      l->l_next->l_prev = currentline;

   /* Add the used up line to the free chain */
   l->l_next = freelines;
   freelines = l;
}


/* insert material before the current line	*/
void insert()
{
   char v[FMAX];
   BPTR i;
   struct LINE *l;
   long pointersave;
   BOOL subssave;

   l = currentline;
   pointersave = pointer;
   subssave    = nosubs;

   if (readfiletitle(v) == 0)
   {
      while (comm != '\n') commrdch();
      SelectInput(edits);
   }
   else
   {
      i = Open(v+1, MODE_OLDFILE);
      if (i == NULL) error(ERROR_FF, v);
      SelectInput(i);
   }
   nosubs = TRUE;
   putline();
   current = -1;

   for(;;)
   {
      currentline = freelines;
      readline();
      if ((i == NULL) && (linel == z_match[0]) &&
	 /* Look for Precisely the string, in Upper or lower case	*/
	 (index(linev, 0, linel, z_match, 0-'P') == 0))
	 break;
      if (linel == 0 && cch == ENDSTREAMCH)
	 break;
      freelines = currentline->l_next;
      currentline->l_next = l;
      currentline->l_prev = l->l_prev;
      if (l->l_prev != 0)
	 l->l_prev->l_next = currentline;
      l->l_prev = currentline;
      if (oldestline == l) oldestline = currentline;
      putline();
      if (freelines == NULL) writeline();
      if (CheckSignal(SIGBREAKF_CTRL_C))
      {
	 if (i != NULL) Close(i);
	 currentline = l;
	 getline();
	 error(ERROR_BRK);
      }
   }

   if (i != NULL) Close(i);
   currentline = l;
   getline();
   nosubs = subssave;
   pointer = pointersave;
}


/* read an input line	*/
void readline()
{
   linev = &currentline->l_len;
   linel = 0;

   for(;;)
   {
      cch = FGetC(Input());
      if (cch == '\n'   ||
	  cch == '\r'   ||
	  cch == '\f'   ||
	  cch == ENDSTREAMCH) break;
      linel++;
      if (linel < maxlinel) linev[linel] = cch;
   }

   if (truncate(linel)) linel = maxlinel;
   if (!trailing)
   {
      while ((linel > pointer) && (linev[linel] == ' '))
	 linel--;
   }
   nosubs = TRUE;
   expanded = FALSE;
}

/* write an output line 	*/
void writeline()
{
   struct LINE *l;
   short *v;
   int i;

   l = oldestline;
   v = oldestline->l_buf;
   if (l == currentline) putline();
   SelectOutput(textout);
   for (i = 0; i < oldestline->l_len; i++) FPutC(textout, (UBYTE)v[i]);
   if (l->l_cch != ENDSTREAMCH) FPutC(textout, (UBYTE)l->l_cch);
   SelectOutput(verout);
   oldestline = l->l_next;
   if (oldestline != NULL) oldestline->l_prev = NULL;
   l->l_next = freelines;
   freelines = l;
}


/* set up a new current line	*/
void getline()
{
   linev = &currentline->l_len;  /* Gets us to the buffer with a length */
   linel = currentline->l_len;
   cch = currentline->l_cch;
   current = currentline->l_numb;
   nosubs = TRUE;
   expanded = FALSE;
}


/* store the current line	*/
void putline()
{
   pointer = 0;
   if (!(quiet || nosubs)) ver('?', '\n');
   compress();
   if (!trailing)
      while ((linel > 0) && (linev[linel] == ' '))
	 linel--;
   currentline->l_cch = cch;
   currentline->l_len = linel;
   currentline->l_numb = current;
}


/* move on to the next line	*/
void nextline()
{
   struct LINE *p;
   int i;

   if (CheckSignal(SIGBREAKF_CTRL_C)) error(ERROR_BRK);
   if ((current > 0) && (current >= ceiling)) error(ERROR_CR);
   if (exhausted) error(ERROR_NOIN);
   pointer = 0;
   if (!deleting) putline();
   if (currentline->l_next == NULL)
   {
      if (!deleting)
      {
	 freelines->l_prev = currentline;
	 currentline->l_next = freelines;
	 currentline = freelines;
	 freelines = freelines->l_next;
	 currentline->l_next = 0;
	 if (freelines==0) writeline();
      }
      current = currentinput->f_lc+1;
      SelectInput(textin);
      readline();
      for (i = 1; i <= globcount; i++) changeglobal(i);
      exhausted = (cch == ENDSTREAMCH);
      currentinput->f_lc = current;
      currentinput->f_ex = exhausted;
   }
   else
   {
      currentline = currentline->l_next;
      getline();
      if (currentline->l_next == NULL)
	 exhausted = currentinput->f_ex;
      if (deleting)
      {
	 p = currentline->l_prev;
	 currentline->l_prev = p->l_prev;
	 if (p->l_prev != NULL)
	    p->l_prev->l_next = currentline;
	 p->l_next = freelines;
	 freelines = p;
	 if (oldestline == p) oldestline = currentline;
      }
   }
   if (exhausted && (zerolevel != NULL)) error(ERROR_NOIN);
   unchanged = FALSE;
}


/*  move back to the previous line	*/
void prevline()
{
   if (currentline->l_prev == NULL) error(ERROR_NOPR);
   putline();
   currentline = currentline->l_prev;
   getline();
   exhausted = FALSE;
   unchanged = FALSE;
}


/* move on to line N	*/
void move(n)
int n;
{
   struct LINE *l;
   int m;
   if (n != current)
   {
      if (!deleting)
      {
	 for( l = currentline->l_prev; l != NULL; l = l->l_prev)
	 {
	    m = l->l_numb;
	    if (m != -1)
	    {
	       if (m == n)
	       {
		  putline();
		  currentline = l;
		  getline();
		  exhausted = FALSE;
		  unchanged = FALSE;
		  return;
	       }
	       if (m<n) break;
	    }
	 }
      }
      while (n != current)
      {
	 if ((current > 0) && (current >= n))
	    error(ERROR_LINE, n);
	 if (exhausted && (n == MAXINT))
	 {
	    if (deleting) linel = 0;
	    break;
	 }
	 nextline();
      }
   }
}

/* verify the current line	*/
void ver(c, n)
{
   int i;

   if (current == -1)
      PutStr("+++");
   else
      VPrintf("%ld", &current);
   if (exhausted)
      PutStr("*\n");
   else
      PutStr(".\n");

   if ((linel != 0) || ((current != 0) && !exhausted))
   {
      verline(c);
      if (pointer != 0)
      {
	 for(i = 1; i < pointer; i++) PutStr(" ");
	 PutStr(">");
	 FPutC(Output(), (UBYTE)n);
      }
   }
   unchanged = TRUE;
   nosubs = TRUE;
}


int vch1(ch)
int ch;
{
   /***  #40<==ch< #177 -> ch,	     ***/
   /*	 #X20 <== ch <	#X7F -> ch,    */
   /*	 #XA0 <== ch <== #XFF -> ch,   */
   if (((ch+1) & 0x7f) > ' ') return(ch);
   if (ch == '\t' || ch < 0) return(' ');
   return('?');
}

int vch2(ch)
int ch;
{
   /***  #40<==ch< #177 -> ch,	     ***/
   /*	 #X20 <== ch <	#X7F -> ch,    */
   /*	 #XA0 <== ch <== #XFF -> ch,   */
   if (((ch+1) & 0x7f) > ' ') return(ch);
   if (ch == '\t' || ch < 0) return(' ');
   return((int)("0123456789ABCDEF"[(ch >> 4)&0x0f]));
}

int vch3(ch)
int ch;
{
   /***  #40<==ch< #177 -> ch,	     ***/
   /*	 #X20 <== ch <	#X7F -> ch,    */
   /*	 #XA0 <== ch <== #XFF -> ch,   */
   if (ch >= 'A' && ch <= 'Z') return('_');
   if (((ch+1) & 0x7f) > ' ')  return(' ');
   if (ch == '\t')             return('T');
   if (ch < 0)                 return('.');
   return((int)("0123456789ABCDEF"[ch & 0x0f]));
}

void wrl(vch)
int (*vch)(int);
{
   int l = linel;
   int p;

   /* Trim trailing spaces from the output */
   while((l > 0) && ((*vch)(linev[l]) == ' ')) l--;
   for (p = 1; p <= l; p++)
       FPutC(Output(), (UBYTE)((*vch)(linev[p])));
   PutStr("\n");
}

/* write out a verification line	*/
/* change to allow top bit set chars to be displayed properly */
void verline(c)
{
   expand();
   condense();
   if (c=='!')
   {
      wrl(vch2);
      wrl(vch3);
   }
   else
      wrl(vch1);
}
