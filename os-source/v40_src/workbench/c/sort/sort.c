; /*
lc -d -j73 -o/obj/Sort.o -i/include -v -csf Sort
blink /obj/Sort.o to /bin/Sort sc sd nd
protect /bin/Sort +p
quit
*/

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* |_o_o|\\ Copyright (c) 1989 The Software Distillery.  All Rights Reserved */
/* |. o.| || This program may not be distributed without the permission of   */
/* | .	| || the authors:			      BBS: (919) 460-7430    */
/* | o	| ||   Dave Baker      Alan Beale	  Jim Cooper		     */
/* |  . |//    Jay Denebeim    Bruce Drake	  Gordon Keener 	     */
/* ======      John Mainwaring Andy Mercier	  Jack Rouse		     */
/*	       John Toebes     Mary Ellen Toebes  Doug Walker	Mike Witcher */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**	(C) Copyright 1989 Commodore-Amiga, Inc.
 **	    All Rights Reserved
**/

/*---------------------------------------------------------------------------*/
/* Command: Sort							     */
/* Author:  Mike Whitcher						     */
/* Change History:							     */
/*  Date    Person	  Action					     */
/* -------  ------------- -----------------				     */
/* 19MAR89  John Toebes   Initial Creation				     */
/* 31AUG90  Jim Cooper	  Fixed!  It no longer crashes, and it frees all     */
/*			  memory on exit!!				     */
/* 06Dec90  Mike Whitcher Made OPT_CASE 1.3 compatable; Added OPT_DESC	     */
/* 01Apr91  Mike Whitcher Bug fix for freeing up some memory twice.	     */
/*                                Only occured in error condition.	     */
/* 05Apr91  John Toebes   #ifdefed out the DESCENDING 			     */
/*									     */
/* Notes:   This algorithm differs from the standard quick sort in the	     */
/*          following ways.						     */
/*  1: This routine is non-recursive.  It instead uses its own stack to      */
/*     keep track of the sublists.					     */
/*  2: There is no penalty for having an already completely or partially     */
/*     sorted data file.						     */
/*  3: This routine uses linked lists to keep track of the data instead      */
/*     of arrays.							     */
/*---------------------------------------------------------------------------*/

#include "internal/commands.h"
#include "sort_rev.h"
#define MSG_CANTOPEN  "Can't open %s\n"
#define MSG_NOMEM     "Out of Memory\n"
#define MSG_EMPTY     "Input file is empty\n"
#define MSG_WRITERR   "Error writing to output file\n"

#ifdef NEW_DESCEND
#define TEMPLATE    "FROM/A,TO/A,COLSTART/K,CASE/S,NUMERIC/S,DESCENDING/s" CMDREV
#else
#define TEMPLATE    "FROM/A,TO/A,COLSTART/K,CASE/S,NUMERIC/S" CMDREV
#endif
#define OPT_FROM     0	     /* input file name 			 */
#define OPT_TO	     1	     /* output file name			 */
#define OPT_COLSTART 2	     /* column compare start position		 */
#define OPT_CASE     3       /* case insensitive                         */
#define OPT_NUMERIC  4       /* sort numerically                         */
#ifdef NEW_DESCEND
#define OPT_DESC     5       /* sort in descending order                 */
#define OPT_COUNT    6
#else
#define OPT_COUNT    5
#endif

#define OPTNBLKS 1
#define OPTSCOL  2
#define OPTCASE  4
#define OPTNUM	 8
#define OPTDESC 16

/* these are my favorite macros to allocate and free memory */
#define GETMEM(sz)   AllocVec((long)sz, MEMF_PUBLIC)
#define FREEMEM(ptr) FreeVec(ptr)
#define Toupper(c)   ( ((c >= 97) && (c <= 122))? c-32: c)

/* These are a couple of my favorite return codes */
#define OK	0
#define OKOK	1
#define ERROR  -1

#define min __builtin_min

/* struct that will hold all of the data */
/* the data immediately follows this	 */
/* pointer and is NULL terminated.	 */
struct llnode {
   struct llnode *next;
   char *data;
   };

/* this is the format for a sublist on the stack */
struct llist  {
   struct llist  *next;
   struct llnode *head;
   struct llnode *tail;
   };

#define NODESZ	sizeof(struct llnode)
#define LISTSZ	sizeof(struct llist )
#define BUFSZ	1024

/* global control information */
struct srtglob {
   int options;
   int startcol;
   char *data;
   char *dpos;
   int	dlen;
   char *item;
   int	itemlen;
   struct DosLibrary *dosbase;
   };

int getnxlin(BPTR, struct srtglob *, struct lnode **);
int compare(char *, char *, struct srtglob *);

#define DOSBase sgcb.dosbase

int cmd_sort(void)
{
struct Library *SysBase = (*((struct Library **) 4));
int rc;
long opts[OPT_COUNT];
struct RDargs *rdargs;

BPTR fh;	       /* input and output file handles       */
 int endlist;  /* true when at end of a sublist       */
int i;

struct llist *stack;   /* stack for the sublists	      */
struct llist *s;       /* current stack sublist in use	      */
struct llist *cur;     /* next/current sublist going on stack */
char   *msg;	       /* holds an error message	      */

struct llnode *data;   /* linked list of data to sort	      */
register struct llnode *ptr; /* generic pointer to use as needed*/

struct llnode *div;    /* sublist divider		                 */
struct llnode *modiv;  /* sublist divider minus one	      */

/* sort command gcb */
struct srtglob sgcb;

msg = (char *)NULL;
rc  = RETURN_FAIL;
memset((char *)&sgcb, 0, sizeof(struct srtglob));

if ((DOSBase = (struct DosLibrary *)OpenLibrary(DOSLIB, DOSVER)))
   {
   memset((char *)opts, 0, sizeof(opts));
   rdargs = ReadArgs(TEMPLATE, opts, NULL);


   if (rdargs == NULL)
      PrintFault(IoErr(), NULL);
   else {
      /* from here on out it anything bad happens it's an error */
      rc = RETURN_ERROR;
      endlist = 0;

      if (opts[OPT_COLSTART])
         {
         StrToLong((char *)opts[OPT_COLSTART], &sgcb.startcol);
         if (sgcb.startcol > 0L)
            {
            sgcb.startcol--;
            endlist |= OPTSCOL;
	         }
         if (sgcb.startcol < 0L)
            {
            sgcb.startcol = 0L;
            }
         }
      if (opts[OPT_CASE])
	      endlist |= OPTCASE;
      if (opts[OPT_NUMERIC])
	      endlist |= OPTNUM;
#ifdef NEW_DESCEND
      if (opts[OPT_DESC])
         endlist |= OPTDESC;
#endif
      sgcb.options = endlist;

      /* open the input file */
      if (!(fh = Open((char *)opts[OPT_FROM], MODE_OLDFILE)))
         {
         msg = MSG_CANTOPEN;
         goto sortclose;
         }

      if (!(sgcb.data = GETMEM(BUFSZ)) )
         {
         msg  = MSG_NOMEM;
         goto fileclose;
         }

    /* read in the input file */
    modiv = data = (struct llnode *)NULL;
    endlist = 1;
    while ((i = getnxlin(fh, &sgcb, &div)) == OKOK) {
	 /* I alternate putting the data on the head and tail	*/
	 /* because the quick sort works best when the data is	*/
	 /* completly random.					                         */
    ptr = div;
	 if (endlist & 1)
	    {
	    if (!ptr) ptr=data;
	    ptr->next = data;
	    data = ptr;
	    }
	   else
	    {
	    if (!modiv) modiv = data;
	    modiv->next = ptr;
	    modiv	= ptr;
	    }
	 endlist++;
	 }

      Close(fh);
      FREEMEM(sgcb.data);

      fh=0;
      if (!data && i == OK) msg = MSG_EMPTY;

      else if (i== ERROR)
      {
         if (modiv)
	    modiv->next = (struct llnode *)NULL;
	 goto dataclose;
      }
      else {
	 if (modiv)
    	     modiv->next = (struct llnode *)NULL;

	 /* open the output file to make sure we can */
	 /* before we go to the trouble to sort data */
	 opts[0] = opts[OPT_TO];
	 if (!(fh = Open((char *)opts[OPT_TO], MODE_NEWFILE)))
	    msg = MSG_CANTOPEN;

	  /* allocate initial stack space */
	 if (!msg && !(stack = (struct llist *)GETMEM(LISTSZ))) {
	    msg = MSG_NOMEM;
	    goto dataclose;
	    }
	 if (!msg) {
	    /* initialize the stack */
	    stack->next = (struct llist *)NULL;
	    stack->head = data;
	    stack->tail = modiv;
	    /* sort the list */
	    while (stack) {
	       /* check for those <cntl>C's */
	       if (CheckSignal(SIGBREAKF_CTRL_C)) {
		  PrintFault(ERROR_BREAK, NULL);
		  rc = RETURN_WARN;
		  goto stckclose;
		  }

	       /* get next sublist off the stack */
	       s = stack;
	       stack = stack->next;
	       /* places everyone; its curtain time */
	       modiv = div = s->head;
	       ptr = s->head->next;
	       endlist = 0;

	       /* process this sublist */
	       while (ptr && !endlist) {
		  if (ptr == s->tail) endlist++;

		  if (compare(s->head->data, ptr->data, &sgcb)) {
		     /* swap these two data pointers */
		     modiv = div;
		     div = div->next;
		     sgcb.dpos = div->data;
		     div->data = ptr->data;
		     ptr->data = sgcb.dpos;
		     }
		  /* increment ptr */
		  ptr = ptr->next;
		  }

	       /* swap the comparer and the divider */
	       sgcb.dpos = div->data;
	       div->data = s->head->data;
	       s->head->data = sgcb.dpos;
	       /* adjust the stack */
	       if (s->head != modiv) {
		  if (!(cur = (struct llist *)GETMEM(LISTSZ))) {
		     msg = MSG_NOMEM;
		     FREEMEM(s);
		     goto stckclose;
		     }
		  else {
		     cur->head = s->head;
		     cur->tail = modiv;
		     cur->next = stack;
		     stack     = cur;
		     }
		  }

	       div = div->next;

	       if (div && (div != s->tail) && (s->tail->next != div)) {
		  if (!(cur = (struct llist *)GETMEM(LISTSZ))) {
		     msg = MSG_NOMEM;
		     FREEMEM(s);
		     goto stckclose;
		     }
		  else
		     {
		     cur->head = div;
		     cur->tail = s->tail;
		     cur->next = stack;
		     stack     = cur;
		     }
		  }

	       /* free up the current sublist stack frame */
	       FREEMEM(s);
	       }  /* while stack */
	    if (!msg)
	       {
	       /* print out the data */
	       for (ptr = data; ptr; ptr = ptr->next)
		  {
		  /* check for those <cntl>C's */
		  if (CheckSignal(SIGBREAKF_CTRL_C))
		     {
		     PrintFault(ERROR_BREAK, NULL);
		     rc = RETURN_WARN;
		     ptr = NULL;
		     continue;
		     }
		  /* change the newline back from a null to a newline */
		  i = strlen(ptr->data);
		  (ptr->data)[i] = '\n';
		  if (Write(fh, ptr->data, i+1) < (i+1) ) {
		     msg = MSG_WRITERR;
		     ptr = NULL;
		     }
		  }
stckclose:     /* make sure our stack is empty */
	       while ((s = stack) != NULL)
		  {
		  stack = stack->next;
		  FREEMEM(s);
		  }

dataclose:     /* make sure data list is empty */
	       while (data)
		  {
		  ptr = data->next;
		  FREEMEM(data);
		  data = ptr;
		  }
	       }
fileclose:
	    if(fh) {
		Close(fh);
		if (!msg)rc = RETURN_OK;
	    }
	    if (sgcb.item)
	       {
	       FREEMEM(sgcb.item);
	       }
	 }
      }
sortclose:
      if (msg)VPrintf(msg, opts);
      FreeArgs(rdargs);
      }
   CloseLibrary((struct Library *)DOSBase);
   }
else
{
   OPENFAIL;
}
return(rc);
}


#undef	DOSBase
#define DOSBase sgcb->dosbase


/*-----------------------------------------------------------------*/
/* compare -- compare two strings to see which one should come	   */
/*	      first. Return 0 if string one should come first;	   */
/*	      otherwise return a 1 indicating that they should be       */
/*       swapped.                                                  */
/*-----------------------------------------------------------------*/
static int compare(sl1, sl2, sgcb)
  char *sl1, *sl2;
  struct srtglob *sgcb;
{
long l1, l2;
register long i1, i2, i3;
register char *s1, *s2;

s1 = sl1;
s2 = sl2;
i3 = sgcb->options;

if (i3 & OPTSCOL)
   {
   i2 = i1 = sgcb->startcol;
   while (*s1 && i1--) s1++;
   while (*s2 && i2--) s2++;
   }

if (i3 & OPTNUM)
   {
   /* do a numeric compare */
   StrToLong(s1, &l1);
   StrToLong(s2, &l2);
   i1 = l1;
   i2 = l2;
   }
else
   {
   if (i3 & OPTCASE)
      {
      i1 = strcmp(s1, s2);
      }
   else
      {
      i1 = stricmp(s1, s2);
      }

   i2 = 0;
   }

/* if we had the peepholer to the compiler working */
/* the code for these lines would be much nicer.   */
#ifdef NEW_DESCEND
if (i3 & OPTDESC)
   return i1 < i2;
else
#endif
   return i1 > i2;
}



/*-----------------------------------------------------------------*/
/* getnxlin -- returns the line from the input file f		   */
/*-----------------------------------------------------------------*/
#define ITEMLEN 80	/* Item buffering size*/

static int getnxlin(f, sgcb, node)
   BPTR f;
   struct srtglob *sgcb;
   struct llnode **node;
{
struct Library *SysBase = (*((struct Library **) 4));
int len;
int rc;
char *p;
int  gotline;
register int dlen;   /* optimization */

rc = OKOK;
len =0;
gotline=0;
dlen = sgcb->dlen;

while (!gotline)
   {
   if (dlen <= 0)
      {
      dlen = Read(f, sgcb->data, (long)BUFSZ);
      if (dlen < 0)
         {
         p = "Error reading input file.\n";
         rc = ERROR;
         gotline++;
         dlen = 0;
         }
      else if (dlen == 0)
         {
         gotline++;
         if (len)
            {
            /* go fill in the structure */
            sgcb->item[len++] = '\0';
            }
         else
            rc = OK;
         }
     else
        sgcb->dpos = sgcb->data;
      }

   /* scan the data looking for a newline */
   while (dlen && !gotline)
      {
      if (len >= sgcb->itemlen)
         {
         /* not enough space in item; reallocate */
         if (!(p = GETMEM(ITEMLEN + sgcb->itemlen)))
            {
            p = "Out of Memory.\n";
            rc = ERROR;
            gotline++;
            continue;
            }
         if (len)
            {
            memcpy(p, sgcb->item, len);
            FREEMEM(sgcb->item);
            }
         sgcb->item = p;
         sgcb->itemlen += ITEMLEN;
         }

      if (*(sgcb->dpos) == '\n')
         {
         gotline++;
         *(sgcb->dpos) = '\0';
         }
     sgcb->item[len++] = *(sgcb->dpos);
     sgcb->dpos++;
     dlen--;
     }
   }

sgcb->dlen = dlen;

switch(rc)
   {
   case OKOK:
   if ( !(*node = (struct llnode *)GETMEM(NODESZ + len)))
      {
      p  = "Out of Memory.\n";
      rc = ERROR;
      goto read_error;
      }
   else
      {
      (*node)->data = ((char *)(*node))+ NODESZ;
      memcpy((*node)->data, sgcb->item, len);
      }
   break;

   case ERROR:
read_error:
   VPrintf(p, NULL);
   if (sgcb->item) FREEMEM(sgcb->item);
   sgcb->item = NULL;
   break;
   }
return(rc);
}
