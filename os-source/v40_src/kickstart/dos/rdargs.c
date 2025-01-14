/* DO NOT USE CTYPE.H! */
#include <exec/types.h>
#include "dos/dos.h"
#include "dos/dosextens.h"
#include <string.h>

#include "libhdr.h"

#ifdef LATTICE
#include "klib_protos.h"
#include "blib_protos.h"
#include "cli_protos.h"
#endif

#include "fctype.h"	/* function versions of ctype macros */

#define TOBPTR(x)	(((LONG) (x)) >> 2)

/*************************** RDARGS **********************************/
/*  Special Implementation of RDARGS for UNIX			     */
/*    Essentially a direct crib from TRIPOS BLIB		     */
/*    with RDCH/UNRDCH redefined as rdgetc/rdungetc		     */
/*    N.B. UNIX provides us with an 'argvector' (here it's 'cargv' - */
/*    a BCPL pointer) and an 'argcount' (here it's 'cargc')          */
/*    Example:- 						     */
/*	   asm fred to bill					     */
/*		results in the following:			     */
/*	   cargc = 4						     */
/*					 All pointers are MC ptrs    */
/*					 to C strings		     */
/*	   cargv ----->   ----------				     */
/*		     !0   |    ----|-------> a s m 0		     */
/*			  ----------				     */
/*		     !1   |    ----|-------> f r e d 0		     */
/*			  ----------				     */
/*		     !2   |    ----|-------> t o 0		     */
/*			  ----------				     */
/*		     !3   |    ----|-------> b i l l 0		     */
/*			  ----------				     */
/*			  |    0   |				     */
/*			  ----------				     */
/*								     */
/*********************************************************************/

/* size is size in LONGs of argv array */

LONG * ASM
rdargs (REG(d1) UBYTE *keys, REG(d2) LONG *argv, REG(d3) LONG size,
	REG(d0) LONG isbcpl)
{
	LONG *w = argv;	   /* array of LONGs */
	LONG numbargs,len;
	UBYTE *temp;
	UBYTE ch;

	*w = 0;
	temp = keys;
	if (isbcpl)
		len = *temp++;
	else
		len = strlen(temp);

	while (len-- > 0) {
		ch = *temp++;
		if (ch == '/')
		{
			int c = toupper(*temp++);
			len--;
			if (c == 'A')	*w |= 1;
			if (c == 'K')	*w |= 2;
			if (c == 'S')	*w |= 4;
			continue;
		}
		if (ch == ',')
			*++w = 0;
	}
	w++;
	numbargs = ((ULONG) w - (ULONG) argv)/sizeof(*w);	/* in LONGs */

/*
// At this stage, the argument elements of argv have been
// initialised to  0    -
//                 1   /A
//                 2   /K
//                 3   /A/K
//                 4   /S
//                 5   /S/A
//                 6   /S/K
//                 7   /S/A/K
*/
	while (1) {
		LONG i;
		LONG argno = -1;
		LONG wsize = size - numbargs;	/* number of args left */

		switch (rditem((UBYTE *) w,wsize,isbcpl)) {
		default:	/* aka negative values */
err:			do {
				ch = rdch();	/*FIX*/
			} while (ch != '\n' && ch != ';' && ch != ENDSTREAMCH);

			setresult2(120);
			return 0;

		case 0:		/* *N, ;, endstreamch */
			for (i = 0; i < numbargs; i++)
			{
				LONG a;

				a = argv[i];
				if (0 <= a && a <= 7)
				{
					if ((a & 1) == 0)
						argv[i] = 0;
					else
						goto err;
				}
			}
			rdch();	/*FIX*/
			return w;

		case 1: /* ordinary item */
			argno = findarg(keys,(UBYTE *) w,isbcpl);
			if (argno >= 0) 	/* get and check argument */
			{
				/* get confused by two of same switch! */
				if (4 <= argv[argno] && argv[argno] <= 7)
				{
					/* no value for key - switch */
					argv[argno] = -1;	/* turned on */
					continue;
				} else {
					/* arg needs '=' parameter */
					LONG item = rditem((UBYTE *) w,wsize,
							   isbcpl);
					if (item == -2)
						/* found '=', get value after */
						item = rditem((UBYTE *) w,wsize,
							      isbcpl);
					if (item <= 0)
						goto err;
				}
			} else {
				/* didn't find argument in keys */
				/* evil coding! */
				if (rdch() == '\n' && 	/*FIX*/
				    *((WORD *) w) == (isbcpl ? '\001?' : '?\0'))
				{
					/* help facility */
					if (isbcpl)
						bwrites(TOBPTR(keys));
					else
						writes((LONG) keys);

					writes((LONG) ": ");
					wrch(0);	/* force output */
					break;
				} else
					unrdch();	/*FIX*/
			}
			/* fall thru! */

		case 2: /* quoted item (i.e. arg value) */
			if (argno < 0)
				for (i = 0; i < numbargs; i++)
				{
					switch (argv[i]) {
					case 0: case 1:	  /* nothing, /A */
						argno = i;
						goto break_for;
					case 2: case 3:   /* /K, /A/K */
						goto err;
					}
				}

break_for:		/* did we find a /A arg to fill it into? */
			if (argno < 0)
				goto err;

			/* allocate space on argv array for the string(!) */
			/* must be a number of longwords */
			/* for BCPL, w must be LW aligned! */

			argv[argno] = isbcpl ? TOBPTR(w) : (LONG) w;

			/* remember, w is a LONG *!    */
			/* keep LW alignment for BCPL! */

			if (isbcpl)
				w += ((*((UBYTE *) w)) >> 2) + 1;
			else
				w += (strlen((UBYTE *) w) >> 2) + 1;

		} /* switch */
	} /* while 1 */

	/*NOTREACHED*/
}

/*  FINDARG - Cribbed from TRIPOS BLIB (BSTR->CSTR)	*/
/*  Routine which searches the KEYS supplied		*/
/*  to RDARGS to find whether an input WORD (w) 	*/
/*  is part of the specified argument list in KEYS.	*/
/*  If it is, it returns the argument number, otherwise */
/*  -1 is returned.					*/

#define MATCHING	0
#define SKIPPING	1

LONG ASM
findarg (REG(d1) UBYTE *keys, REG(d2) UBYTE *w, REG(d3) LONG isbcpl )
{
   WORD len,klen;

   WORD state = MATCHING;
   WORD wp    = 0;
   LONG argno = 0;		/* in keys */

   if (isbcpl)
   {
	len  = *w++;
	klen = *keys++;
   } else {
	len  = strlen(w);
	klen = strlen(keys);
   }

   while (klen-- > 0)
   {
      int kch = *keys++;

      if (state == MATCHING)
      {
	 if (((kch == '=') ||
	      (kch == '/') ||
	      (kch == ',')) &&
	     (wp == len))
	 {
	    return argno;
	 }

	 if (tolower(kch) != tolower(w[wp++]))	/* CAN'T use ctype.h!!! */
	    state = SKIPPING;
      }

      /* handles things like p=pat/k */
      if ((kch == ',') || (kch == '='))
      {
	 state = MATCHING;
	 wp = 0;
      }

      if (kch == ',')
	 argno++;

   } /* while */

   if ((state == MATCHING) && (wp == len))
      return argno;

   return -1 ;
}


/* read a parameter in from a string into commandname 
// returns -2    "=" Symbol
//         -1    error
//          0    *N, ;, endstreamch
//          1    unquoted item
//          2    quoted item
*/

/* STUPID shell.b assumes null-terminated BSTRs are returned */

LONG ASM
rditem (REG(d1) UBYTE *commandname, REG(d2) LONG maxlongs, REG(d3) LONG isbcpl)
{
	LONG currpos = 0;
	LONG ch;
	LONG inquote = 0;

	maxlongs *= 4;	/* bytes */
	maxlongs -= 1;	/* for null/size */

	*commandname = '\0';	/* empty string to both BCPL and C */

	/* skip white space */
	do {
		ch = rdch();	/*FIX*/
	} while (ch == ' ' || ch == '\t');

	if (ch == '"')
	{
		inquote = TRUE;
		ch = rdch();	/*FIX*/
	}

	while (ch != '\n' && ch != ENDSTREAMCH) {

		if (inquote)
		{
			if (ch == '"')		/* end of quote - FIX! */
				return 2;
			if (ch == '*')		/* *x inside quoted string */
			{
				ch = rdch();	/*FIX*/
				if (ch == 'e' || ch == 'E')
					ch = '\x1b';
				else if (ch == 'n' || ch == 'N')
					ch = '\n';
			}
		} else
			if (ch == ';' || ch == ' ' || ch == '\t' || ch == '=')
				break;

		if (currpos >= maxlongs)
			return -1;

		/* always keep it a valid BSTR/CSTR */
		if (isbcpl)
		{
			/* DAMN shell.b assumes null-terminated BSTRs! */
			commandname[++currpos] = ch;
			commandname[currpos+1] = '\0';
			commandname[0]         = currpos;
		} else {
			commandname[currpos++] = ch;
			commandname[currpos]   = 0;
		}

		ch = rdch();	/*FIX*/
	} /* while */

	unrdch();	/*FIX*/

	if (inquote)	/* unterminated quote */
		return -1;

	if (currpos == 0)	/* didn't read anything */
	{
		if (ch == '=')	/* first char was an '=' */
		{
			rdch();		/* eat the '=' */	/*FIX*/
			return -2;
		}
		return 0;	/* didn't read anything at all */
	}
	return 1;		/* read something */
}

