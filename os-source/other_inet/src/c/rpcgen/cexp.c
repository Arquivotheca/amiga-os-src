/* ---------------------------------------------------------------------------------
 * cexp.c
 *
 * $Locker:  $
 *
 * $Id: cexp.c,v 1.1 90/11/09 13:44:42 bj Exp $
 *
 * $Revision: 1.1 $
 *
 * $Header: Hog:Other/inet/src/c/rpcgen/RCS/cexp.c,v 1.1 90/11/09 13:44:42 bj Exp $
 *
 *-----------------------------------------------------------------------------------
 */


/*  A Bison parser, made from cexp.y  */

#define	INT	258
#define	CHAR	259
#define	NAME	260
#define	ERROR	261
#define	OR	262
#define	AND	263
#define	EQUAL	264
#define	NOTEQUAL	265
#define	LEQ	266
#define	GEQ	267
#define	LSH	268
#define	RSH	269
#define	UNARY	270

#line 109 "cexp.y"

#include <setjmp.h>
/* #define YYDEBUG 1 */

  static int yylex ();
  static yyerror ();
  int expression_value;

  static jmp_buf parse_return_error;

  /* some external tables of character types */
  extern unsigned char is_idstart[], is_idchar[];


#line 124 "cexp.y"
typedef union {
  long lval;
  int voidval;
  char *sval;
} YYSTYPE;

#ifndef YYLTYPE
typedef
  struct yyltype
    {
      int timestamp;
      int first_line;
      int first_column;
      int last_line;
      int last_column;
      char *text;
   }
  yyltype;

#define YYLTYPE yyltype
#endif

#define	YYACCEPT	return(0)
#define	YYABORT	return(1)
#define	YYERROR	return(1)
#include <stdio.h>



#define	YYFINAL		59
#define	YYFLAG		-32768
#define	YYNTBASE	33

#define YYTRANSLATE(x) (yytranslate[x])

static char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    29,     2,     2,     2,    27,    14,     2,    31,
    32,    25,    23,     9,    24,     2,    26,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     8,     2,    17,
     2,    18,     7,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,    13,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,    12,     2,    30,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     2,     3,     4,     5,
     6,    10,    11,    15,    16,    19,    20,    21,    22,    28
};

static short yyrline[] = {     0,
   153,   158,   159,   164,   166,   168,   170,   175,   177,   179,
   181,   183,   185,   187,   189,   191,   193,   195,   197,   199,
   201,   203,   205,   207,   209,   211,   213,   215,   217
};

static char * yytname[] = {     0,
"error","$illegal.","INT","CHAR","NAME","ERROR","'?'","':'","','","OR",
"AND","'|'","'^'","'&'","EQUAL","NOTEQUAL","'<'","'>'","LEQ","GEQ",
"LSH","RSH","'+'","'-'","'*'","'/'","'%'","UNARY","'!'","'~'",
"'('","')'","start"
};

static short yyr1[] = {     0,
    33,    34,    34,    35,    35,    35,    35,    35,    35,    35,
    35,    35,    35,    35,    35,    35,    35,    35,    35,    35,
    35,    35,    35,    35,    35,    35,    35,    35,    35
};

static short yyr2[] = {     0,
     1,     1,     3,     2,     2,     2,     3,     3,     3,     3,
     3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
     3,     3,     3,     3,     3,     5,     1,     1,     1
};

static short yydefact[] = {     0,
    27,    28,    29,     0,     0,     0,     0,     1,     2,     4,
     5,     6,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     7,     3,     0,    25,    24,    23,    22,
    21,    15,    16,    19,    20,    17,    18,    13,    14,    11,
    12,     8,     9,    10,     0,    26,     0,     0,     0
};

static short yydefgoto[] = {    57,
     8,     9
};

static short yypact[] = {    28,
-32768,-32768,-32768,    28,    28,    28,    28,    -3,    74,-32768,
-32768,-32768,    -2,    28,    28,    28,    28,    28,    28,    28,
    28,    28,    28,    28,    28,    28,    28,    28,    28,    28,
    28,    28,    28,-32768,    74,    53,    23,    90,   105,   119,
   132,   143,   143,   150,   150,   150,   150,   155,   155,   -22,
   -22,-32768,-32768,-32768,    28,    74,     8,     9,-32768
};

static short yypgoto[] = {-32768,
    46,    -4
};


#define	YYLAST		182


static short yytable[] = {    10,
    11,    12,    31,    32,    33,    14,    14,    58,    59,    35,
    36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
    46,    47,    48,    49,    50,    51,    52,    53,    54,    34,
     1,     2,     3,    17,    18,    19,    20,    21,    22,    23,
    24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
    56,     4,    13,     0,     0,     0,     5,     6,     7,    15,
    55,     0,    16,    17,    18,    19,    20,    21,    22,    23,
    24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
    15,     0,     0,    16,    17,    18,    19,    20,    21,    22,
    23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
    33,    18,    19,    20,    21,    22,    23,    24,    25,    26,
    27,    28,    29,    30,    31,    32,    33,    19,    20,    21,
    22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
    32,    33,    20,    21,    22,    23,    24,    25,    26,    27,
    28,    29,    30,    31,    32,    33,    21,    22,    23,    24,
    25,    26,    27,    28,    29,    30,    31,    32,    33,    23,
    24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
    27,    28,    29,    30,    31,    32,    33,    29,    30,    31,
    32,    33
};

static short yycheck[] = {     4,
     5,     6,    25,    26,    27,     9,     9,     0,     0,    14,
    15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
    25,    26,    27,    28,    29,    30,    31,    32,    33,    32,
     3,     4,     5,    11,    12,    13,    14,    15,    16,    17,
    18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
    55,    24,     7,    -1,    -1,    -1,    29,    30,    31,     7,
     8,    -1,    10,    11,    12,    13,    14,    15,    16,    17,
    18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
     7,    -1,    -1,    10,    11,    12,    13,    14,    15,    16,
    17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
    27,    12,    13,    14,    15,    16,    17,    18,    19,    20,
    21,    22,    23,    24,    25,    26,    27,    13,    14,    15,
    16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
    26,    27,    14,    15,    16,    17,    18,    19,    20,    21,
    22,    23,    24,    25,    26,    27,    15,    16,    17,    18,
    19,    20,    21,    22,    23,    24,    25,    26,    27,    17,
    18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
    21,    22,    23,    24,    25,    26,    27,    23,    24,    25,
    26,    27
};
#define YYPURE 1

#line 2 "bison.simple"

/* Skeleton output parser for bison,
   copyright (C) 1984 Bob Corbett and Richard Stallman

   Permission is granted to anyone to make or distribute verbatim copies of this program
   provided that the copyright notice and this permission notice are preserved;
   and provided that the recipient is not asked to waive or limit his right to
   redistribute copies as permitted by this permission notice;
   and provided that anyone possessing an executable copy
   is granted access to copy the source code, in machine-readable form,
   in some reasonable manner.

   Permission is granted to distribute derived works or enhanced versions of
   this program under the above conditions with the additional condition
   that the entire derivative or enhanced work
   must be covered by a permission notice identical to this one.

   Anything distributed as part of a package containing portions derived
   from this program, which cannot in current practice perform its function usefully
   in the absense of what was derived directly from this program,
   is to be considered as forming, together with the latter,
   a single work derived from this program,
   which must be entirely covered by a permission notice identical to this one
   in order for distribution of the package to be permitted.

 In other words, you are welcome to use, share and improve this program.
 You are forbidden to forbid anyone else to use, share and improve
 what you give them.   Help stamp out software-hoarding!  */

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYFAIL		goto yyerrlab;

#define YYTERROR	1

#ifndef YYIMPURE
#define YYLEX		yylex()
#endif

#ifndef YYPURE
#define YYLEX		yylex(&yylval, &yylloc)
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYIMPURE

int	yychar;			/*  the lookahead symbol		*/
YYSTYPE	yylval;			/*  the semantic value of the		*/
				/*  lookahead symbol			*/

YYLTYPE yylloc;			/*  location data for the lookahead	*/
				/*  symbol				*/

int yydebug = 0;		/*  nonzero means print parse trace	*/

#endif  /* YYIMPURE */


/*  YYMAXDEPTH indicates the initial size of the parser's stacks	*/

#ifndef	YYMAXDEPTH
#define YYMAXDEPTH 200
#endif

/*  YYMAXLIMIT is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#ifndef YYMAXLIMIT
#define YYMAXLIMIT 10000
#endif


#line 87 "bison.simple"
int
yyparse()
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YYSTYPE *yyvsp;
  YYLTYPE *yylsp;
  int yyerrstatus;	/*  number of tokens to shift before error messages enabled */
  int yychar1;		/*  lookahead token as an internal (translated) token number */

  short	yyssa[YYMAXDEPTH];	/*  the state stack			*/
  YYSTYPE yyvsa[YYMAXDEPTH];	/*  the semantic value stack		*/
  YYLTYPE yylsa[YYMAXDEPTH];	/*  the location stack			*/

  short *yyss = yyssa;		/*  refer to the stacks thru separate pointers */
  YYSTYPE *yyvs = yyvsa;	/*  to allow yyoverflow to reallocate them elsewhere */
  YYLTYPE *yyls = yylsa;

  int yymaxdepth = YYMAXDEPTH;

#ifndef YYPURE

  int yychar;
  YYSTYPE yylval;
  YYLTYPE yylloc;

  extern int yydebug;

#endif


  YYSTYPE yyval;		/*  the variable used to return		*/
				/*  semantic values from the action	*/
				/*  routines				*/

  int yylen;

  if (yydebug)
    fprintf(stderr, "Starting parse\n");

  yystate = 0;
  yyerrstatus = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
  yylsp = yyls;

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

  *++yyssp = yystate;

  if (yyssp >= yyss + yymaxdepth - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *yyvs1 = yyvs;
      YYLTYPE *yyls1 = yyls;
      short *yyss1 = yyss;

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yyls1, size * sizeof (*yylsp),
		 &yymaxdepth);

      yyss = yyss1; yyvs = yyvs1; yyls = yyls1;
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yymaxdepth >= YYMAXLIMIT)
	yyerror("parser stack overflow");
      yymaxdepth *= 2;
      if (yymaxdepth > YYMAXLIMIT)
	yymaxdepth = YYMAXLIMIT;
      yyss = (short *) alloca (yymaxdepth * sizeof (*yyssp));
      bcopy ((char *)yyss1, (char *)yyss, size * sizeof (*yyssp));
      yyls = (YYLTYPE *) alloca (yymaxdepth * sizeof (*yylsp));
      bcopy ((char *)yyls1, (char *)yyls, size * sizeof (*yylsp));
      yyvs = (YYSTYPE *) alloca (yymaxdepth * sizeof (*yyvsp));
      bcopy ((char *)yyvs1, (char *)yyvs, size * sizeof (*yyvsp));
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yylsp = yyls + size - 1;
      yyvsp = yyvs + size - 1;

      if (yydebug)
	fprintf(stderr, "Stack size increased to %d\n", yymaxdepth);

      if (yyssp >= yyss + yymaxdepth - 1)
	YYERROR;
    }

  if (yydebug)
    fprintf(stderr, "Entering state %d\n", yystate);

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
yyresume:

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

      if (yydebug)
	fprintf(stderr, "Now at end of input.\n");
    }
  else
    {
      yychar1 = YYTRANSLATE(yychar);

      if (yydebug)
	fprintf(stderr, "Parsing next token; it is %d (%s)\n", yychar, yytname[yychar1]);
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

  if (yydebug)
    fprintf(stderr, "Shifting token %d (%s), ", yychar, yytname[yychar1]);

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
  *++yylsp = yylloc;

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  goto yynewstate;

/* Do the default action for the current state.  */
yydefault:

  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
yyreduce:
  yylen = yyr2[yyn];
  yyval = yyvsp[1-yylen]; /* implement default value of the action */

  if (yydebug)
    {
      if (yylen == 1)
	fprintf (stderr, "Reducing 1 value via line %d, ",
		 yyrline[yyn]);
      else
	fprintf (stderr, "Reducing %d values via line %d, ",
		 yylen, yyrline[yyn]);
    }


  switch (yyn) {

case 1:
#line 154 "cexp.y"
{ expression_value = yyvsp[0].lval; ;
    break;}
case 3:
#line 160 "cexp.y"
{ yyval.lval = yyvsp[0].lval; ;
    break;}
case 4:
#line 165 "cexp.y"
{ yyval.lval = - yyvsp[0].lval; ;
    break;}
case 5:
#line 167 "cexp.y"
{ yyval.lval = ! yyvsp[0].lval; ;
    break;}
case 6:
#line 169 "cexp.y"
{ yyval.lval = ~ yyvsp[0].lval; ;
    break;}
case 7:
#line 171 "cexp.y"
{ yyval.lval = yyvsp[-1].lval; ;
    break;}
case 8:
#line 176 "cexp.y"
{ yyval.lval = yyvsp[-2].lval * yyvsp[0].lval; ;
    break;}
case 9:
#line 178 "cexp.y"
{ yyval.lval = yyvsp[-2].lval / yyvsp[0].lval; ;
    break;}
case 10:
#line 180 "cexp.y"
{ yyval.lval = yyvsp[-2].lval % yyvsp[0].lval; ;
    break;}
case 11:
#line 182 "cexp.y"
{ yyval.lval = yyvsp[-2].lval + yyvsp[0].lval; ;
    break;}
case 12:
#line 184 "cexp.y"
{ yyval.lval = yyvsp[-2].lval - yyvsp[0].lval; ;
    break;}
case 13:
#line 186 "cexp.y"
{ yyval.lval = yyvsp[-2].lval << yyvsp[0].lval; ;
    break;}
case 14:
#line 188 "cexp.y"
{ yyval.lval = yyvsp[-2].lval >> yyvsp[0].lval; ;
    break;}
case 15:
#line 190 "cexp.y"
{ yyval.lval = (yyvsp[-2].lval == yyvsp[0].lval); ;
    break;}
case 16:
#line 192 "cexp.y"
{ yyval.lval = (yyvsp[-2].lval != yyvsp[0].lval); ;
    break;}
case 17:
#line 194 "cexp.y"
{ yyval.lval = (yyvsp[-2].lval <= yyvsp[0].lval); ;
    break;}
case 18:
#line 196 "cexp.y"
{ yyval.lval = (yyvsp[-2].lval >= yyvsp[0].lval); ;
    break;}
case 19:
#line 198 "cexp.y"
{ yyval.lval = (yyvsp[-2].lval < yyvsp[0].lval); ;
    break;}
case 20:
#line 200 "cexp.y"
{ yyval.lval = (yyvsp[-2].lval > yyvsp[0].lval); ;
    break;}
case 21:
#line 202 "cexp.y"
{ yyval.lval = (yyvsp[-2].lval & yyvsp[0].lval); ;
    break;}
case 22:
#line 204 "cexp.y"
{ yyval.lval = (yyvsp[-2].lval ^ yyvsp[0].lval); ;
    break;}
case 23:
#line 206 "cexp.y"
{ yyval.lval = (yyvsp[-2].lval | yyvsp[0].lval); ;
    break;}
case 24:
#line 208 "cexp.y"
{ yyval.lval = (yyvsp[-2].lval && yyvsp[0].lval); ;
    break;}
case 25:
#line 210 "cexp.y"
{ yyval.lval = (yyvsp[-2].lval || yyvsp[0].lval); ;
    break;}
case 26:
#line 212 "cexp.y"
{ yyval.lval = yyvsp[-4].lval ? yyvsp[-2].lval : yyvsp[0].lval; ;
    break;}
case 27:
#line 214 "cexp.y"
{ yyval.lval = yylval.lval; ;
    break;}
case 28:
#line 216 "cexp.y"
{ yyval.lval = yylval.lval; ;
    break;}
case 29:
#line 218 "cexp.y"
{ yyval.lval = 0; ;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 303 "bison.simple"

  yyvsp -= yylen;
  yylsp -= yylen;
  yyssp -= yylen;

  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now", yyssp-yyss);
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }

  *++yyvsp = yyval;

  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = yylloc.first_line;
      yylsp->first_column = yylloc.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;

yyerrlab:   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      yyerror("parse error");
    }

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYERROR;

      if (yydebug)
	fprintf(stderr, "Discarding token %d (%s).\n", yychar, yytname[yychar1]);

      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;

yyerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) goto yydefault;
#endif

yyerrpop:   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYERROR;
  yyvsp--;
  yylsp--;
  yystate = *--yyssp;

  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now", yyssp-yyss);
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }

yyerrhandle:

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

  if (yydebug)
    fprintf(stderr, "Shifting error token, ");

  *++yyvsp = yylval;
  *++yylsp = yylloc;

  yystate = yyn;
  goto yynewstate;
}
#line 220 "cexp.y"


/* During parsing of a C expression, the pointer to the next character
   is in this variable.  */

static char *lexptr;

/* Take care of parsing a number (anything that starts with a digit).
   Set yylval and return the token type; update lexptr.
   LEN is the number of characters in it.  */

/* maybe needs to actually deal with floating point numbers */

static int
parse_number (olen)
     int olen;
{
  register char *p = lexptr;
  register long n = 0;
  register int c;
  register int base = 10;
  register len = olen;
  char *err_copy;

  extern double atof ();

  for (c = 0; c < len; c++)
    if (p[c] == '.') {
      /* It's a float since it contains a point.  */
      yyerror ("floating point numbers not allowed in #if expressions");
      return ERROR;
      
/* ****************
	 yylval.dval = atof (p);
	 lexptr += len;
	 return FLOAT;
		 ****************  */
    }
  
  if (len >= 3 && (!strncmp (p, "0x", 2) || !strncmp (p, "0X", 2))) {
    p += 2;
    base = 16;
    len -= 2;
  }
  else if (*p == '0')
    base = 8;
  
  while (len-- > 0) {
    c = *p++;
    n *= base;
    if (c >= '0' && c <= '9')
      n += c - '0';
    else {
      if (c >= 'A' && c <= 'Z') c += 'a' - 'A';
      if (base == 16 && c >= 'a' && c <= 'f')
	n += c - 'a' + 10;
      else if (len == 0 && c == 'l')
	;
      else {
	yyerror ("Invalid number in #if expression");
	return ERROR;
      }
    }
  }

  lexptr = p;
  yylval.lval = n;
  return INT;
}

struct token {
  char *operator;
  int token;
};

#define NULL 0

static struct token tokentab2[] = {
  {"&&", AND},
  {"||", OR},
  {"<<", LSH},
  {">>", RSH},
  {"==", EQUAL},
  {"!=", NOTEQUAL},
  {"<=", LEQ},
  {">=", GEQ},
  {NULL, ERROR}
};

/* Read one token, getting characters through lexptr.  */

static int
yylex ()
{
  register int c;
  register int namelen;
  register char *tokstart;
  register struct token *toktab;

 retry:

  tokstart = lexptr;
  c = *tokstart;
  /* See if it is a special token of length 2.  */
  for (toktab = tokentab2; toktab->operator != NULL; toktab++)
    if (c == *toktab->operator && tokstart[1] == toktab->operator[1]) {
      lexptr += 2;
      return toktab->token;
    }

  switch (c) {
  case 0:
    return 0;
    
  case ' ':
  case '\t':
  case '\n':
    lexptr++;
    goto retry;
    
  case '\'':
    lexptr++;
    c = *lexptr++;
    if (c == '\\')
      c = parse_escape (&lexptr);
    yylval.lval = c;
    c = *lexptr++;
    if (c != '\'') {
      yyerror ("Invalid character constant in #if");
      return ERROR;
    }
    
    return CHAR;

    /* some of these chars are invalid in constant expressions;
       maybe do something about them later */
  case '/':
  case '+':
  case '-':
  case '*':
  case '%':
  case '|':
  case '&':
  case '^':
  case '~':
  case '!':
  case '@':
  case '<':
  case '>':
  case '(':
  case ')':
  case '[':
  case ']':
  case '.':
  case '?':
  case ':':
  case '=':
  case '{':
  case '}':
  case ',':
    lexptr++;
    return c;
    
  case '"':
    yyerror ("double quoted strings not allowed in #if expressions");
    return ERROR;
  }
  if (c >= '0' && c <= '9') {
    /* It's a number */
    for (namelen = 0;
	 c = tokstart[namelen], is_idchar[c] || c == '.'; 
	 namelen++)
      ;
    return parse_number (namelen);
  }
  
  if (!is_idstart[c]) {
    yyerror ("Invalid token in expression");
    return ERROR;
  }
  
  /* It is a name.  See how long it is.  */
  
  for (namelen = 0; is_idchar[tokstart[namelen]]; namelen++)
    ;
  
  lexptr += namelen;
  return NAME;
}


/* Parse a C escape sequence.  STRING_PTR points to a variable
   containing a pointer to the string to parse.  That pointer
   is updated past the characters we use.  The value of the
   escape sequence is returned.

   A negative value means the sequence \ newline was seen,
   which is supposed to be equivalent to nothing at all.

   If \ is followed by a null character, we return a negative
   value and leave the string pointer pointing at the null character.

   If \ is followed by 000, we return 0 and leave the string pointer
   after the zeros.  A value of 0 does not mean end of string.  */

static int
parse_escape (string_ptr)
     char **string_ptr;
{
  register int c = *(*string_ptr)++;
  switch (c)
    {
    case 'a':
      return '\a';
    case 'b':
      return '\b';
    case 'e':
      return 033;
    case 'f':
      return '\f';
    case 'n':
      return '\n';
    case 'r':
      return '\r';
    case 't':
      return '\t';
    case 'v':
      return '\v';
    case '\n':
      return -2;
    case 0:
      (*string_ptr)--;
      return 0;
    case '^':
      c = *(*string_ptr)++;
      if (c == '\\')
	c = parse_escape (string_ptr);
      if (c == '?')
	return 0177;
      return (c & 0200) | (c & 037);
      
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
      {
	register int i = c - '0';
	register int count = 0;
	while (++count < 3)
	  {
	    if ((c = *(*string_ptr)++) >= '0' && c <= '7')
	      {
		i *= 8;
		i += c - '0';
	      }
	    else
	      {
		(*string_ptr)--;
		break;
	      }
	  }
	return i;
      }
    default:
      return c;
    }
}

static
yyerror (s)
     char *s;
{
  error (s);
  longjmp (parse_return_error, 1);
}

/* This page contains the entry point to this file.  */

/* Parse STRING as an expression, and complain if this fails
   to use up all of the contents of STRING.  */
/* We do not support C comments.  They should be removed before
   this function is called.  */

int
parse_c_expression (string)
     char *string;
{
  lexptr = string;
  
  if (lexptr == 0 || *lexptr == 0) {
    error ("empty #if expression");
    return 0;			/* don't include the #if group */
  }

  /* if there is some sort of scanning error, just return 0 and assume
     the parsing routine has printed an error message somewhere.
     there is surely a better thing to do than this.     */
  if (setjmp(parse_return_error))
    return 0;

  if (yyparse ())
    return 0;			/* actually this is never reached
				   the way things stand. */
  if (*lexptr)
    error ("Junk after end of expression.");

  return expression_value;	/* set by yyparse() */
}

#ifdef TEST_EXP_READER
/* main program, for testing purposes. */
main()
{
  int n;
  char buf[1024];
  extern int yydebug;
/*
  yydebug = 1;
*/
  initialize_random_junk ();

  for (;;) {
    printf("enter expression: ");
    n = 0;
    while ((buf[n] = getchar()) != '\n')
      n++;
    buf[n] = '\0';
    printf("parser returned %d\n", parse_c_expression(buf));
  }
}

/* table to tell if char can be part of a C identifier. */
char is_idchar[256];
/* table to tell if char can be first char of a c identifier. */
char is_idstart[256];
/* table to tell if c is horizontal space.  isspace() thinks that
   newline is space; this is not a good idea for this program. */
char is_hor_space[256];

/*
 * initialize random junk in the hash table and maybe other places
 */
initialize_random_junk()
{
  register int i;

  /*
   * Set up is_idchar and is_idstart tables.  These should be
   * faster than saying (is_alpha(c) || c == '_'), etc.
   * Must do set up these things before calling any routines tthat
   * refer to them.
   */
  for (i = 'a'; i <= 'z'; i++) {
    ++is_idchar[i - 'a' + 'A'];
    ++is_idchar[i];
    ++is_idstart[i - 'a' + 'A'];
    ++is_idstart[i];
  }
  for (i = '0'; i <= '9'; i++)
    ++is_idchar[i];
  ++is_idchar['_'];
  ++is_idstart['_'];
#ifdef DOLLARS_IN_IDENTIFIERS
  ++is_idchar['$'];
  ++is_idstart['$'];
#endif

  /* horizontal space table */
  ++is_hor_space[' '];
  ++is_hor_space['\t'];
}

error (msg)
{
  printf("error: %s\n", msg);
}
#endif
