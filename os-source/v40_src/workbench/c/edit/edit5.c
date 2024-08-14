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
/* 03NOV89  John Toebes   Initial Creation				 */
/* Notes:								 */
/*-----------------------------------------------------------------------*/
#include "edit.h"

void error(n)
int n;
{
   int r;
   int i;
   LEVEL z;
   char *s;

   r = 10;
   z = zerolevel;

   switch(n)
   {
      case ERROR_UC:   s = "Unknown command - %lc\n";           break;
      case ERROR_UDC:  s = "Unknown command\n";                 break;
      case ERROR_BRA:  s = "Unmatched parenthesis\n";           break;
      case ERROR_CNTX: s = "Null context after %lc\n";          break;
      case ERROR_PM:   s = "+ or - expected after %lc\n";       break;
      case ERROR_NUM:  s = "Number expected after %lc\n";       break;
      case ERROR_LINE: s = "Line number %ld too small\n";       break;
      case ERROR_FNX:  s = "Filename expected\n";               break;
      case ERROR_STR:  s = "String too long\n";                 break;
      case ERROR_NOM:  s = "No match\n";                        break;
      case ERROR_REP:  s = "Nothing to repeat\n";               break;
      case ERROR_NOIN: s = "Input exhausted\n";                 break;
      case ERROR_NOPR: s = "No more previous lines\n";          break;
      case ERROR_CR:   s = "Ceiling reached\n";                 break;
      case ERROR_GLOB: s = "Too many globals\n";                break;
      case ERROR_FFA:                                   r = 20;        
      case ERROR_FF:   s = "Can't open %s\n";                   break;
      case ERROR_ARG:  s = "Bad args\n";                r = 20; break;
      case ERROR_OPT:  s = "Invalid option values\n";   r = 20; break;
      case ERROR_RN:   s = "Can't rename %s as %s\n";   r = 20; break;
      case ERROR_GV:   s = "Run out of store\n";        r = 20; break;
      case ERROR_CFV:  s = "Command file stack ovf\n";  r = 20; break;
      case ERROR_QW:   s = "Invalid %lc command\n";             break;
      case ERROR_BRK:  s = "****BREAK\n";                       break;
      case ERROR_UQL:  s = "Unknown qualifier\n";               break;
      case ERROR_IQL:  s = "Illegal qualifiers\n";              break;
      default:	       s = "Unknown error\n";           r = 20; break;
   }

   zerolevel = NULL;

   for(;;)
   {
      if ((editlevel == z) && (n == ERROR_NOIN))
      {
	 if (IsInteractive(edits))
	    if (!verifying) PutStr("\n");
	 goto l;
      }
      if (cfsp == 0) break;
      revertcom();
   }

   if ((commlinel > 0) && !IsInteractive(edits))
   {
      for(i = 1; i <= commlinel; i++)
	 FPutC(Output(), (UBYTE)commbuf[i]);
      if (commbuf[commlinel] != '\n') PutStr("\n");
   }
   if (commpoint != 0)
   {
      for(i = 1; i < commpoint; i++) PutStr(" ");
      PutStr(">\n");
   }
   VPrintf(s, (long *)(&n+1));
   rc = r;
   if ((rc == 20) || !IsInteractive(edits))
   {
      if (rc != 20) ver('?', '\n');
      if (opened) closestreams();
      longjmp(*quitlevel, QUITLAB);
   }
l:
   if (quiet || unchanged) {
	PutStr(":");
	Flush(Output());
   }
   else ver( '?', '\0' );
   longjmp(*editlevel, EDITLAB);
}


BOOL truncate(p)
int p;
{
   if (p <= maxlinel) return(FALSE);
   if (current == -1)
      PutStr("****** Inserted line truncated\n");
   else
      VPrintf("****** Line %ld truncated\n", &current);
   rc = 10;
   return(TRUE);
}


/* expand tabs in the current line with dummy characters */
void expand()
{
   int t;
   int p;
   BOOL f;
   int j;
   int c;
   int i;

   if (expanded) return;
   j = 0;
   c = 0;
   f = FALSE;
   t = maxlinel-linel;
   p = t+pointer;

   for (i = linel; i >= 1; i--)
      linev[t+i] = linev[i];
   while (t < maxlinel)
   {
      if ((j + (c == '\t')) > t)
      {
	 /* We have a TAB to expand */
	 t++;
	 for (i = linel; i > t; i--)
	    linev[i] = linev[i-1];
	 f = TRUE;
      }
      else
      {
	 /* No tab that pushes us more than one position */
	 j = j+1;
	 if (c=='\t')
	    linev[j] = -1;
	 else
	 {
	    t++;
	    c = linev[t];
	    linev[j] = c;
	 }

	 if ((j & 7) == 0) c = 0;
	 if (t == p) pointer = j;
      }
   }
   if (f) truncate(MAXINT);
   linel = j;
   expanded = TRUE;
   condensed = TRUE;
}

/* remove all dummy characters from the current line */
void compress()
{
   int i,j;

   if (!expanded) return;
   i = j = 0;

   while (i < linel)
   {
      i++;
      if (linev[i] >= 0)
      {
	 j++;
	 linev[j] = linev[i];
      }
      if (pointer == i) pointer = j;
   }
   linel = j;
   expanded = FALSE;
}


/* remove all dummy characters from the current line */
/* leaving tabs expanded */
void condense()
{
   int i, j;
   if ((!expanded) || condensed) return;

   i = j = 0;

   while(i < linel)
   {
      i++;
      if (pointer == i) pointer = j+(linev[i] >= 0);
      if (linev[i] >= 0)
      {
	 j++;
	 linev[j] = linev[i];
	 if (linev[i] == '\t')
	 {
	    while(j & 7)
	    {
	       j++;
	       linev[j] = -1;
	    }
	 }
      }
   }
   linel = j;
   condensed = TRUE;
}


/* step the character pointer */
BOOL incrementp()
{
   expand();
   if (pointer == LMAX) return(FALSE);
   pointer++;
   unchanged = FALSE;
   if (pointer > linel)
   {
      linev[pointer] = ' ';
      linel = pointer;
   }
   return(TRUE);
}


/* substitute a string for line positions P+1 to Q */
void subst(p, q, v)
int p;
int q;
short *v;
{
   int s;
   int t;
   int r;
   int i;

   s = v[0];
   t = linel-q;
   truncate(p+s+t);
   if ((p+s) > maxlinel) s = maxlinel-p;
   r = p+s;
   if ((r+t) > maxlinel) t = maxlinel-r;
   linel = r+t;
   if (r != q)
   {
      if (r > q)
	 for (i = t; i > 0; i--)
	    linev[r+i] = linev[q+i];
      else
	 for (i = 1; i <= t; i++)
	    linev[r+i] = linev[q+i];
   }
   for (i = 1; i <= s; i++) linev[p+i] = v[i];
   nosubs = FALSE;
}


/* search line positions P+1 to Q for a string */
/* Uses qualifier to restrict search */
int index(l, p, q, v, qual)
short *l;
int p, q;
short *v;
int qual;
{
   int s;
   short *r;
   int i;
   BOOL upcase;
   BOOL backscan;
   int c1, c2;

   s = v[0];
   upcase = FALSE;
   if (qual < 0)
   {
      qual = -qual;
      upcase = TRUE;
   }
   backscan = FALSE;
   if (s > (q-p)) return(-1); /* Search too long */

   /* Decide on action to be taken */
   switch(qual)
   {
      case 'P': if (s != q) return(-1); /* As always called from Find */
      case 'B': q = p; break;
      case 'E': p = q-s;
      case 'L': backscan = TRUE;
      case 'O': q = q-s; break;
      DEFAULT: error( ERROR_UQL );
   }
   while (p <= q)
   {
      r = l + ((backscan) ? q : p);
      for (i = 1; i <= s; i++)
      {
	 c1 = r[i];
	 c2 = v[i];
	 if (upcase)
	 {
	    if ((c1 >= 'a') && (c1 <= 'z')) c1 += 'A'-'a';
	    if ((c2 >= 'a') && (c2 <= 'z')) c2 += 'A'-'a';
	 }
	 if (c1 != c2) goto l;
      }
      /* We got through the comparison, tell them so */
      return(backscan ? q : p);
l:    if (backscan) q--; else p++;
   }
   return(-1);
}


void readglobal()
{
   short v[SMAX];
   int qual;
   int globtype;
   int n;
   int s;
   int saveptr;
   short *p;
   struct LINE *l;

   l = currentline;

   globtype = commrdch();
   if (globtype != 'A' && globtype != 'B' && globtype != 'E')
     error( ERROR_UDC, 'G', globtype );

   qual = getstring( v, TRUE );
   s = v[0];
   if (s == 0) error( ERROR_CNTX, 'G' );
   if (globcount >= GMAX) error(ERROR_GLOB);
   globcount++;
   n = globcount;
   p = newvec((s+2)*CHARSIZE);
   memcpy((char *)p, (char *)v, (s+2)*CHARSIZE);
   g_match[n] = p;
   g_qual[n] = qual;
   g_type[n] = globtype;
   g_count[n] = 0;

   readcontext(v);
   s = v[0];
   p = newvec((s+2)*CHARSIZE);
   memcpy((char *)p, (char *)v, (s+2)*CHARSIZE);
   g_repl[n] = p;
   saveptr = pointer;
   s = nosubs;
   nosubs = TRUE;
   do {
      putline();
      currentline = currentline->l_next;
      if (currentline == NULL) currentline = l;
      getline();
      changeglobal(n);
   } while (currentline != l);
   if (nosubs) nosubs = s;
   pointer = saveptr;
   /* Verify */
   VPrintf("G%ld\n", &n);
}


void alterglobal( val )
int val;
{
   int k;
   int i;
   int nstart, nend;

   k = 0;
   nstart = 1;
   nend = globcount;

   nextcomm();
   if (comm >= '0' && comm <= '9')
      k = commreadn();
   if (k == 0)
   {
      /* I think this pairs up this way, the BCPL source was very strange */
      /* Here.	This could have been a bug in the old source */
      if (val == G_CANCEL)
	 globcount = 0;
   }
   else
   {
      if (k > globcount) error( ERROR_NOM );
      nstart = nend = k;
   }
   /* Do the change */
   for(i = nstart; i <= nend; i++)
   {
      if (val == G_CANCEL)
      {
	 discardvec(g_match[i]);
	 discardvec(g_repl[i]);
      }
      else if (val == G_DISABLE)
      {
	 if (g_type[i] > 0) g_type[i] = -g_type[i];
      }
      else
      {
	 if (g_type[i] < 0) g_type[i] = -g_type[i];
      }

      /* Modify if required */
      if ((k != 0) && (val == G_CANCEL))
      {
	 for (i = k; i < globcount; i++)
	 {
	    g_type[i]  = g_type[i+1];
	    g_count[i] = g_count[i+1];
	    g_match[i] = g_match[i+1];
	    g_repl[i]  = g_repl[i+1];
	 }
	 globcount--;
      }
   }
}


void changeglobal(i)
int i;
{
   int p;
   int n;
   int s;
   short *v;
   short *w;
   int type;

   p = 0;
   v = g_match[i];
   w = g_repl[i];
   type = g_type[i];

   if (type < 0) return; /* Global is disabled */
   for(;;)
   {
      n = index(linev, p, linel, v, g_qual[i] );
      s = n+v[0];
      if (n<0) break;
      subst( (type == 'A') ? s: n, (type == 'B') ? n : s, w);
      p = n+w[0];
      if (type != 'E') p += v[0];
      g_count[i]++;
   }
}
