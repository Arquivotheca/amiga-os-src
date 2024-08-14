/* *********************************************************************** */
/* * =================================================================== * */
/* *      Macro Assembler for the Motorola MC68000 Microprocessor        * */
/* * =================================================================== * */
/* *								         * */
/* *         (c) Copyright 1985, Tenchstar Ltd., Bristol, UK.		 * */
/* *                      All rights reserved.                           * */
/* *								         * */
/* *               Last Modified :  11-APR-85	   (PJF)	         * */
/* *********************************************************************** */

/* * M68KASM1 : Automatically translated from BCPL to C on 26-Feb-85     * */


#include "libhdr"
#include "ahdr"

/******************************************/
/*  Local functions returning a value	  */
/******************************************/

SCBPTR checkopen();
struct DIRNODE *insert_name();

/******************************************/
/*  External functions returning a value  */
/******************************************/

extern int *newvec();			  /*  asm7  */
extern struct CROSSREFNODE *heap3pp();	  /*  asm7  */
extern int absolute();			  /*  asm6  */
extern string msg();			  /*  asm7  */
extern long freenewvec();

int quiet;

/* OK */
start ()
{
   int	i, j;
   char parm[257];

#ifdef TRIPOS
   int av[50];
   int *sv[15];
   string as = "\072PROG=FROM/A,CODE=TO/K,VER/K,LIST/K,HDR/K,EQU/K,INC/K,OPT/K";
   string nulldevice = "\004nil:";
#endif

#ifdef U42
   int av[50];
   string as = "\062PROG=FROM/A,-o/K,-v/K,-l/K,-h/K,-e/K,-i/K,-c=OPT/K";
   string nulldevice = "\011/dev/null";
#endif

#ifdef AMI
   string av[50];
   string as = (string)"\062PROG=FROM/A,-o/K,-v/K,-l/K,-h/K,-e/K,-i/K,-c=OPT/K";
#ifdef SUN
   string nulldevice = (string)"\011/dev/null";
#endif
#ifdef IBM
   string nulldevice = (string)"\004NUL:";
#endif
#endif

#ifdef SUN
   int stvdefsize = 80000;               /* Bytes */
#endif
#ifdef IBM
   unsigned stvdefsize = 20000;          /* Bytes */
#endif

   string name_liststream   = NULL;
   string name_verstream    = NULL;
   string name_equstream    = NULL;

   long totalcodesize = 0;

   if ( setjmp( fatalerrorp ) != 0 ) goto fatalerror;

#ifdef TRIPOS
   sv	      = datstring(sv);
   datestring = sv + 0;
   timestring = sv + 5;
#endif

#ifdef U42
   datestring = "\000";
   timestring = "\000";
#endif

#ifdef AMI
   datestring = (string)"\000";
   timestring = (string)"\000";
#endif

   version	    = 11;
   release	    = 183;
   tagv             = (string)getvec(tagsize);
   macroname        = (string)getvec(tagsize);
   inputbuff        = (string)getvec(maxllen/BYTESPERWORD);
   expvec           = getvec(expsize);
   labelvec         = (string)getvec(tagsize);
   titlevec         = (string)getvec(titlecharsmax/BYTESPERWORD);
   outbuff          = (string)getvec(maxllen/BYTESPERWORD);
   codebuff         = getvec(codesize*(sizeof(struct SVNODE))/BYTESPERWORD);
   errorvec         = (int *)getvec(errorsize*2);
   stvec            = NULL;
   expvecp	    = expvec + expsize;
   extrnsymbols     = 0;
   entrysymbols     = 0;
   sectionlist	    = 0;
   PUname	    = (string)"\000";
   reloc32info	    = 0;
   reloc16info	    = 0;
   reloc_32count    = 0;
   reloc_16count    = 0;
   sourcestream     = 0;
   liststream	    = 0;
   codestream	    = 0;
   verstream	    = 0;
   sysout	    = output();
   failed	    = no;
   in_movem	    = no;
   errormessages    = no;
   crossreference   = no;
   paging	    = yes;
   parmlisting	    = no;
   dumpsymbols	    = no;
   xref 	    = no;
   quiet            = no;
   UCflag	    = no;
   no_labels	    = yes;
   extrnref	    = no;
   objectmodule     = o_none;
   pass1	    = no;
   pass2	    = no;
   sourcelinenumber = 0;
   listlinenumber   = 0;
   getlocallabel    = no;
   output_local_labels = yes;
   name_codestream   = NULL;

#ifdef TRIPOS
   maxextlength = 7;
#endif

#ifdef U42
   maxextlength = tagchars;
#endif

#ifdef AMI
   maxextlength = tagchars;
#endif

   ts_default = ts_word;

#ifdef TRIPOS
   if ( rdargs(as, av, 50) == 0 )
   {
      writes ( "\037***  Bad arguments for string:\n" );
      writef ( "\012***  \"%S\"\n", as );
      failed = yes;
   }
   else
   {
      word i; /* FOR loop variables */
      word ns = av[ 0 ];
      word nc = av[ 1 ];
      word nv = av[ 2 ];
      word nl = av[ 3 ];
      word nh = av[ 4 ];
      word neq = av[ 5 ];
      string defaultparm = "\002  ";
      word parmlength = defaultparm[ 0 ];
      word uparm = av[ 7 ] == 0 ? "\000" : av[ 7 ];
      word uparmlength = uparm[ 0 ];
      word plength = parmlength + uparmlength;

      for ( i = 1 ; i <= parmlength ; i++ )
	 parm[ i ] = defaultparm[ i ];
      for ( i = 1 ; i <= uparmlength ; i++ )
	 parm[ parmlength + i ] = capitalch(uparm[ i ]);
      parm[ 0 ] = plength;
      name_sourcestream = ns;
      name_codestream = nc == 0 ? "\004nil:" : nc;
      name_verstream = nv == 0 ? "\001*" : nv;
      name_liststream = nl == 0 ? "\004nil:" : nl;
      name_equstream = neq;
      name_hdrfile = nh;
      sourcestream = findinput(name_sourcestream, no);
      checkopen ( sourcestream, result2, name_sourcestream, "\005input"
	  );
      liststream = findoutput(name_liststream, no);
      checkopen ( liststream, result2, name_liststream, "\006output" );
      if ( ! ( nl == 0 ) )
	 parm[ 1 ] = 'L';
      codestream = findoutput(name_codestream, yes);
      checkopen ( codestream, result2, name_codestream, "\006output" );
      if ( ! ( nc == 0 ) )
	 parm[ 2 ] = 'T';
      verstream = findoutput(name_verstream, no);
      checkopen ( verstream, result2, name_verstream, "\006output" );
   }
#endif

#ifdef U42
   if ( rdargs(as, av, 50) == 0 )
   {
      writes ( "\037***  Bad arguments for string:\n" );
      writef ( "\012***  \"%S\"\n", as );
      failed = yes;
   }
   else
   {
      int i; /* FOR loop variables */
      word ns = av[ 0 ];
      word nc = av[ 1 ];
      word nv = av[ 2 ];
      word nl = av[ 3 ];
      word nh = av[ 4 ];
      word neq = av[ 5 ];
      string defaultparm = "\002  ";
      int parmlength = defaultparm[ 0 ];
      string uparm = (string)av[ 7 ] == NULL ? "\000" : (string)av[ 7 ];
      int uparmlength = uparm[ 0 ];
      int plength = parmlength + uparmlength;

      name_sourcestream = ns;
      sourcestream = checkopen(name_sourcestream, findinput, no);

      name_codestream = nc == 0 ? nulldevice : nc;
      codestream = checkopen(name_codestream, findoutput);

      name_liststream = nl == 0 ? nulldevice : nl;
      liststream = checkopen(name_liststream, findoutput);

      name_equstream = neq;
      name_hdrfile   = nh;
      name_verstream = nv;

      verstream = nv == 0 ? output() : checkopen(name_verstream, findoutput);

      for ( i = 1 ; i <= parmlength ; i++ )
	 parm[ i ] = defaultparm[ i ];

      for ( i = 1 ; i <= uparmlength ; i++ )
	 parm[ parmlength + i ] = capitalch(uparm[ i ]);

      parm[ 0 ] = plength;

      if ( ! ( nl == 0 ) )
	 parm[ 1 ] = 'L';

      if ( ! ( nc == 0 ) )
	 parm[ 2 ] = 'U';
   }
#endif

#ifdef AMI
   if ( rdargs(as, av, 50) == NULL )
   {
      writes ( "\037***  Bad arguments for string:\n" );
      writef ( "\012***  \"%S\"\n", (long)(as) );
      failed = yes;
   }

   else
   {
      int i;
      string ns  = av[0];
      string nc  = av[1];
      string nv  = av[2];
      string nl  = av[3];
      string nh  = av[4];
      string neq = av[5];

      string defaultparm = (string)"\002  ";
      int parmlength	 = defaultparm[0];

      string uparm = (av[7] == NULL) ?
			  (string)"\000" : av[7];
      int uparmlength = uparm[0];
      int plength = parmlength + uparmlength;

      name_sourcestream = ns;
      {
	 char buff[257];
	 bst2cst(name_sourcestream,buff);
         sourcestream = fopen(buff, "r");
      }
      if ( sourcestream == NULL )
      {
         writef ( "\035*** Cannot open %S for input\n",(long)name_sourcestream);
         failed = yes;
      }

      name_codestream	= (nc == NULL) ? nulldevice : nc;
      codestream = checkopen(name_codestream, findoutput, yes);
      name_codestream = nc;    /* To enable deletion later */

      name_liststream	= (nl == NULL) ? nulldevice : nl;
      liststream = checkopen(name_liststream, findoutput, no);

      name_equstream = neq;
      name_hdrfile   = nh;

      name_verstream = nv;
      verstream = ( nv == NULL ) ? output() :
			  checkopen(name_verstream, findoutput, no);

      for ( i = 1 ; i <= parmlength ; i++ )
	 parm[ i ] = defaultparm[ i ];

      for ( i = 1 ; i <= uparmlength ; i++ )
	 parm[parmlength + i] = capitalch(uparm[i]);

      parm[0] = plength;

      if ( nl != NULL )
	 parm[ 1 ] = 'L';

      if ( nc != NULL )
	 parm[ 2 ] = 'A';
   }
#endif

   if (failed) abortassembly();

   for ( i = 1; i <= parm[0]; i++ )
      switch ( capitalch(parm[ i ]) )
      {
      case 'Q':
	 quiet = yes;
	 continue;
      case 'X':
	 xref = yes;
	 continue;
      case 'L':
	 parmlisting = yes;
	 continue;
      case 'S':
	 dumpsymbols = yes;
	 continue;
      case 'C':
	 UCflag = yes;
	 continue;
      case 'D':
         output_local_labels = no;
         continue;

#ifdef TRIPOS
      case 'T':
	 objectmodule = o_tripos;
	 maxextlength = 7;
	 continue;
      case 'M':
	 objectmodule = o_motorola;
	 continue;
      case 'H':
	 objectmodule = o_intelhex;
	 continue;
      case 'U':
	 objectmodule = o_unix4_2;
	 maxextlength = tagchars;
	 continue;
      case 'A':
	 objectmodule = o_amiga;
	 maxextlength = tagchars;
	 continue;
#endif

#ifdef U42
      case 'U':
	 objectmodule = o_unix4_2;
	 maxextlength = tagchars;
	 continue;
      case 'A':
	 objectmodule = o_amiga;
	 maxextlength = tagchars;
	 continue;
#endif

#ifdef AMI
      case 'A':
	 objectmodule = o_amiga;
	 maxextlength = tagchars;
	 continue;
#endif

      case 'E':
	 maxextlength = 15;
	 continue;

/*
      case 'W':
	 stvdefsize = 0;
	 while ( (i + 1) <= parm[0] )
	 {
	    int ch = parm[i+1];
	    if ( ! (('0' <= ch) && (ch <= '9')) )
	       break;
	    stvdefsize = ( (stvdefsize*10) + ch ) - '0';
	    i++;
	 }
	 continue;
*/

      case ' ':
      case '\t':
      case ',':
	 continue;
      default:
	 writef ( "\034Bad option: \"%C\"  - Ignored\n", (long)(parm[i]) );
	 continue;
      }

/*
   stvecupb = stvdefsize/BYTESPERWORD;
   stvec    = getvec(stvecupb + tagtablesize);
   stvecp   = stvec + stvecupb + 1;
*/
   stvec = (struct NEWVECNODE *)getvec(newvecsize);
   stvec->link = NULL;
   stvec->length = NEWNODEPTRSIZE;

   stvecp = NULL;

   if ( (tagv == NULL)    || (macroname == NULL) || (inputbuff == NULL) ||
	(expvec == NULL)  || (labelvec == NULL)  || (titlevec == NULL)  ||
	(outbuff == NULL) || (codebuff == NULL)  || (errorvec == NULL)  ||
	(stvec == NULL) )
   {
      writes( "\042*** Insufficient heap for M68KASM\n" );
      abortassembly();
   }

/*
   tagtable = (struct SYMNODE **)stvecp;
*/
   tagtable = (struct SYMNODE **)newvec(
        ((sizeof(struct SYMNODE*)*tagtablesize/BYTESPERWORD) - 1));

   for ( j = 0 ; j <= ( tagtablesize - 1 ) ; j++ )
      tagtable[ j ] = NULL;

#ifdef TRIPOS
   makefilelist( av[6], "\011sys:g.asm" );
#endif

#ifdef U42
   makefilelist( av[6], "\000" );
#endif

#ifdef AMI
   makefilelist( av[6], "\000" );
#endif

   sysout = verstream;
   selectoutput ( sysout );

   if (!quiet)
   {
      writef ( "\047MC68000 Macro Assembler  Version %N.%N\n",
        	   (long)version, (long)release);
      writes ( "\065Copyright (C) 1985 by Tenchstar Ltd., T/A Metacomco.\n" );
      writes ( "\025All rights reserved.\n");
   }

   current_filename = (struct FNAMENODE*)newvec(
             (sizeof(struct FNAMENODE)/BYTESPERWORD) );
   current_filename->link = NULL;
   current_filename->fname = name_sourcestream;

   current_directory = (struct FNAMENODE*)newvec(
             (sizeof(struct FNAMENODE)/BYTESPERWORD) );
   current_directory->link = NULL;
   current_directory->fname = "\002./";

   systemwords = yes;
   declsyswords();
   systemwords = no;
   selectoutput( liststream );
   firstpass();
   if (!quiet)
   {
     selectoutput(verstream);
     newline();
   }
   selectoutput( liststream );
   secondpass();

   if ( ! noobj )
      switch (objectmodule)
      {
#ifndef AMI
	 case o_tripos:
	     triposmodule();
	     break;
	 case o_motorola:
	     motorolamodule();
	     break;
	 case o_intelhex:
	     intelhexmodule();
	     break;
	 case o_unix4_2:
	     unix4_2module();
	     break;
#endif
	 case o_amiga:
	     amigamodule();
	     break;
	 case o_none:
	     break;
	 default:
	     complain(1000);
      }

   errors = errors - warnings;

   if ( listing )
   {
      clearbuffer();
      listed = no;

      if ( errors == 0 )
      {
	 spacelines(3);
	 listed  = no;
	 linepos = 0;
	 writestring ( "\040No errors found in this Assembly" );
	 printbuffer();
      }
      else
      {
	 int i;

	 if ( aborted )
	 {
	    spacelines(3);
	    listed = no;
	    writestring ( "\040Fatal Error  -  Assembly Aborted" );
	    printbuffer();
	 }

	 settitle ( "\021ERROR-DIAGNOSTICS" );
	 errormessages = yes;
	 onpage = 0;

	 for ( i = 0 ; i <= ( errors + warnings - 1 ) ; i++ )
	 {
	    int offset = i*2;
	    int ln     = errorvec[offset++];
	    int err    = errorvec[offset];

	    clearbuffer();
	    linepos = 10;

	    writestring("\004M68K");
	    write0( err, 4 );
	    writestring("\012  on line ");
	    writenumber(ln, 5);
	    writestring("\005:    ");
	    writestring(msg(err));
	    printbuffer();
	 }

	 clearbuffer();
	 spacelines(3);
	 listed  = no;
	 linepos = 0;
	 writenumber( errors, 4 );
	 writestring( "\006 error" );
	 if ( errors != 1 )
	    writechar ( 's' );
	 writestring( "\027 found in this Assembly" );
	 printbuffer();
      }
   }

   if ( xref && listing )
   {
      int i;

      errormessages = no;
      crossreference = yes;
      xreftable = 0;

      for ( i = 0 ; i <= (tagtablesize - 1) ; i++ )
      {
	 struct SYMNODE *t = tagtable[i];
	 while ( t != NULL )
	 {
	    if ( t->st_definition != NULL )
	       putinxreftable ( t, & xreftable	);
	    t = t->st_link;
	 }
      }
      printxreftable();
   }

   selectoutput (sysout);

#ifdef TRIPOS
   if ( ! ( name_equstream == NULL ) )
   {
      word stream = findoutput(name_equstream, no);
      checkopen ( stream, result2, name_equstream, "\006output" );
      if ( failed )
	 abortassembly (  );
      printequates ( stream, name_sourcestream );
   }
#endif

#ifdef U42
   if ( ! ( name_equstream == 0 ) )
   {
      word stream = checkopen(name_equstream, findoutput);
      if ( failed )
	 abortassembly (  );
      printequates ( stream, name_sourcestream );
   }
#endif

#ifdef AMI
   if ( name_equstream != NULL )
   {
      SCBPTR stream = checkopen(name_equstream, findoutput, no);
      if ( failed )
	 abortassembly();
      printequates(stream, name_sourcestream);
   }
#endif

   if ( errors == 0 )
   {
      if (!quiet) writes ( "\003No " );
   }
   else
      writef ( "\003%N ", (long)errors );

   if ( !(quiet && (errors == 0)) )
      writef ( "\037error%S found in this Assembly\n",
	       errors == 1 ? (long)"\000" : (long)"\001s" );

   if (!quiet)
       writef ( "\032\nCode size : %N Bytes (%S)", (long)(relcodewords*4),
	     (relrp16==0) && (relrp32==0) ? (long)"\024Position Independent" :
				            (long)"\013Relocatable" );

fatalerror:
   if ( aborted )
      writes ( "\030\n\n***  Assembly Aborted\n" );

   totalcodesize = freenewvec();

   if (!quiet)
      writef( "\031\nWorkspace Used %N Bytes\n", totalcodesize );

   tidyup( aborted        ? rc_aborted :
           (errors > 0)   ? rc_error_found :
           (warnings > 0) ? rc_warning_given : 0 );
}


struct DIRNODE *insert_name( ptr, strstart, length )
string ptr;
int strstart, length;
{
  int i;
  struct DIRNODE *space =
      (struct DIRNODE *)newvec( (sizeof(struct DIRNODE)-1)/BYTESPERWORD );
  string s = (string)newvec( length/BYTESPERWORD );

  space->link = NULL;
  space->file = s;

  for (i=1; i<=length; i++)
    s[i] = ptr[strstart + i - 1];

  s[0] = length;

  return space;
}


makefilelist( s, sysdef )
string s, sysdef;
{
  struct DIRNODE *listptr;
  struct DIRNODE *space =
      (struct DIRNODE *)newvec( (sizeof(struct DIRNODE)-1)/BYTESPERWORD );

  dir_incl = space;

  space->link = NULL;
  space->file = NULL;   /* filled in later */

  listptr = dir_incl;

  if ( s != NULL )
  {
    int strl  = s[0];
    int count = 1;

    while (count<strl)
    {
      int stringstart;
      int length = 0;
      int ch = s[count];

      while ( (ch == '+') || (ch == ',') || (ch == ' '))
      {
	if (++count > strl) break;
	ch = s[count];
      }
      stringstart = count;

      while ( (ch != '+') && (ch != ',') && (ch != ' '))
      {
	length++;
	if (++count > strl) break;
	ch = s[count];
      }

      listptr->link = insert_name( s, stringstart, length );
      listptr = listptr->link;

      count++;
    }
  }

  listptr->link = insert_name( sysdef, 1, sysdef[0] );
}


#ifdef TRIPOS
checkopen ( stream, r2, name, type )
word stream, r2, name, type;
{
   if ( stream == 0 )
   {
      writef ( "\034*** Cannot open %S for %S:  ",
			 (long)name, (long)type );
      fault ( r2 );
      failed = yes;
   }
}
#endif

#ifdef U42
word checkopen ( name, fn )
word name, fn;
{
   word stream = fn(name);
   if ( stream == 0 )
   {
      writef ( "\032*** Cannot open %S for %S\n", name, fn == findinput
	  ? "\005input" : "\006output" );
      failed = yes;
   }
   return stream;
}
#endif

#ifdef AMI
SCBPTR checkopen ( name, fn, binary )
string name;
SCBPTR (*fn)();
int binary;
{
   SCBPTR stream = (*fn)(name, binary);
   if ( stream == (SCBPTR)0 )
   {
      writef ( "\032*** Cannot open %S for %S\n", (long)name,
	   (fn == findinput) ? (long)"\005input" : (long)"\006output" );
      failed = yes;
   }
   return stream;
}
#endif

abortassembly()
{
   freenewvec();
   tidyup( rc_catastrophic );
}


/* OK */
putinxreftable ( node, ptr )
struct SYMNODE *node;
struct CROSSREFNODE **ptr;
{
   if ( (*ptr) == NULL )
      (*ptr) = heap3pp(node, NULL, NULL);
   else
   {
      struct CROSSREFNODE *p = (*ptr);
      if ( compstring(node->st_name, (p->symbol)->st_name) < 0 )
	 putinxreftable ( node, &(p->left) );
      else
	 putinxreftable ( node, &(p->right) );
   }
}


/* OK */
printxreftable ( )
{
   listing = (paging = yes);
   listed  = no;
   onpage  = 0;

   settitle("\017CROSS-REFERENCE");
   clearbuffer();
   printnode(xreftable);
   clearbuffer();
}


/* OK */
printnode ( node )
struct CROSSREFNODE *node;
{
   if ( node != NULL )
   {
      struct SYMNODE *t      = node->symbol;
      struct CROSSREFNODE *l = node->left;
      struct CROSSREFNODE *r = node->right;
      string s = t->st_name;

      printnode(l);
      linepos = 0;
      writestring(s);
      linepos = 32;

      {
	 int ln = t->st_definition;

	 if ( ((t->st_type) & st_type_mask) == s_ext )
	    writestring ( "\016***EXTERNAL***" );
	 else
	    switch (ln)
	    {
	      case cr_undefined:
		 writestring ( "\017***UNDEFINED***" );
		 break;
	      case cr_multiple:
		 writestring ( "\016***MULTIPLE***" );
		 break;
	      case cr_setsymbol:
		 writestring ( "\016*****SET******" );
		 break;
	      default:
		 writenumber ( ln, 5 );
		 break;
	    }
      }

      linepos = 40;
      {
	 int type = (t->st_type) & st_type_mask;
	 value = t->st_value;

	 if ( t->st_definition > 0 )
	    switch ( type )
	    {
	    case s_rel:
	       writehexvalue ( value, 4 );
	       writechar ( '\'' );
	       break;
	    case s_abs16:
	       writehexvalue ( value, 4 );
	       break;
	    case s_abs32:
	       writehexvalue ( value, 8 );
	       break;
	    case s_Dr:
	       writechar ( 'D' );
	       writehexvalue ( value, 1 );
	       break;
	    case s_Ar:
	       writechar ( 'A' );
	       writehexvalue ( value, 1 );
	       break;
	    case s_macro:
	       writestring ( "\005Macro" );
	       break;
	    default:
	       writestring ( "\004????" );
	       break;
	    }

	 {
	    struct REFNODE *reflist = t->st_references;
	    int online = 0;

	    linepos = 52;
	    if ( reflist == NULL )
	    {
	       linepos = 37;
	       if ( t->st_definition > 0 )
		  writechar ( 'U' );
	    }
	    else
	       while ( reflist != NULL )
	       {
		  if ( online == 12 )
		  {
		     printbuffer();
		     clearbuffer();
		     linepos = 50;
		     writestring ( "\002- " );
		     online = 0;
		  }

		  writenumber ( reflist->value, 5 );
		  writechar ( ' ' );
		  reflist = reflist->link;
		  online++;
	       }

	    printbuffer();
	    clearbuffer();
	 }
      }

      printnode(r);
   }
}

/* OK */
printequates ( stream, sourcename )
SCBPTR stream;
string sourcename;
{
   int i;

   SCBPTR o = output();
   selectoutput(stream);
   printbanner();

   writef ( "\055*  Equates for file \"%S\" written on %S at %S\n",
	    (long)sourcename, (long)datestring, (long)timestring );
   printbanner();
   newline();

   for ( i = 0; i <= (tagtablesize - 1); i++ )
   {
      struct SYMNODE *t = tagtable[ i ];
      while ( t != NULL )
      {
	 if ( t->st_definition != 0 )
	 {
	    int line  = t->st_definition;
	    int type  = (t->st_type) & st_type_mask;
	    long value = t->st_value;

	    if ( (line > 0) && absolute(type) )
	    {
	       int j; /* FOR loop variables */
	       string name = t->st_name;
	       int length  = name[0];
	       writef ( "\004%S  ", (long)name );
	       for ( j = length; j <= 30 ;j++ )
		  wrch ( ' ' );
	       writef ( "\014EQU    $%X8\n", (long)value );
	    }
	 }
	 t = t->st_link;
      }
   }
   endwrite();
   selectoutput(o);
}

/* OK */
printbanner ( )
{
   int i;
   for ( i = 1 ; i <= 80 ; i++ )
      wrch ( '*' );
   newline();
}


freevectors( upb, a, b, c, d, e, f, g, h, i1, j, k, l, m, n )
int upb;
int **a, **b, **c, **d, **e, **f, **g, **h, **i1, **j, **k, **l, **m, **n;
{
   int i;
   int ***vectors  =  & a ;

   for ( i = 0 ; i <= ( upb - 1 ) ; i++ )
      if ( (*(vectors[i])) != NULL )
      {
         int *temp = *(vectors[i]);
         *(vectors[i]) = NULL;      /* Zero the global just in case we
                                       abort later and try to freevec again */
	 freevec ( temp );
      }
}


closefile( stream )
SCBPTR stream;
{
  selectoutput( stream );      /* select the stream for input/output */
  endwrite();                  /* close the file */
}

tidyup ( rc )
int rc;
{
   if ( sourcestream != NULL )
     fclose(sourcestream);

   if ( liststream != NULL )
     closefile( liststream );

   if ( verstream != NULL )
     closefile( verstream );

   if ( codestream != NULL )
   {
     closefile( codestream );
     if ((rc > rc_warning_given) && (name_codestream != NULL)) 
     {
       /* Delete the output file */
       char buff[257];
       bst2cst(name_codestream,buff);
       unlink(buff);
     }
   }

   freevectors ( 9, &tagv, &macroname, &inputbuff, &expvec, &labelvec,
                     &titlevec, &outbuff, &codebuff, &errorvec );

   stop( rc );
}


