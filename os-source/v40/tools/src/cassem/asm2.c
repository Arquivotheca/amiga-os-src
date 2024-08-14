/* ****************************************************************************** */
/* *   Assembler for the Motorola MC68000 Microprocessor:  Section 2		* */
/* *	      (c) Copyright 1985, Tenchstar Ltd., Bristol, UK.			* */
/* *										* */
/* *		    Last Modified :  11-APR-85	   (PJF)			* */
/* ****************************************************************************** */

/* M68KASM2 : Automatically translated from BCPL to C on 26-Feb-85 */


#include  "libhdr"
#include  "asmhdr"


/******************************************/
/*  External functions returning a value  */
/******************************************/

extern struct REFNODE	*heap2i();		  /*  asm7  */
extern struct MACRONODE *heap3ip();		  /*  asm7  */
extern struct EXPRESSNODE *effective_address();   /*  asm5  */

extern int  eafield();			   /*  asm6  */
extern int  evalexp();			   /*  asm5  */
extern int  *gvec();			   /*  asm7  */
extern int  *newvec();			   /*  asm7  */
extern int  sizevalue();		   /*  asm5  */

/******************************************/
/*  Local functions returning a value	  */
/******************************************/

int rewind();
int aligned();
int digits();
int newasmlabel();


/* OK */
firstpass()
{
   sectptr	    = (struct SECTNODE *)-1;
   changesection ( no, "\000", d_text );

   location	    = 0;
   fwdreftype	    = s_abs32;
   symbcount	    = 0;
   charpos	    = 1;
   ended	    = no;
   aborted	    = no;
   errors	    = 0;
   warnings         = 0;
   skiplevel	    = 0;
   skipping	    = 0;
   macrodepth	    = 0;
   getlevel	    = 0;
   macrobase	    = NULL;
   macroend	    = NULL;
   asmlabel	    = 0;
   pass1	    = yes;
   pass2	    = no;
   inmacro	    = no;
   noobj	    = objectmodule == o_none;
   listing	    = parmlisting;
   errormessages    = no;
   nargstack	    = NULL;
   length	    =  - 1 ;
   charpos	    = 0;
   linepos	    = 0;
   onpage	    = 0;
   sourcelinenumber = 0;
   listlinenumber   = 0;
   pagenumber	    = 0;
   linesperpage     = 60;
   charsperline     = 132;
   llenfixed	    = no;
   plenfixed	    = no;
   totalwords	    = 0;

   settitle ( "\000" );

   resetflags();
   listed = yes;

#ifdef TRIPOS
   if ( name_hdrfile != NULL )
      triposget(name_hdrfile);
#endif

#ifdef U42
   if ( name_hdrfile != NULL )
      unixget(name_hdrfile);
#endif

#ifdef AMI
   if ( name_hdrfile != NULL )
      amigaget(name_hdrfile);
#endif

   listed = no;

   while ( !( ended || aborted ) )
   {
      resetflags();
      doline();
   }

   amendlocvalues();
   alignbuffers();
}

/* OK */
amendlocvalues ( )
{
   struct SECTNODE *ptr;

   for ( ptr = sectionlist; ptr != NULL; ptr = ptr->sect_link )
   {
      changesection ( yes, ptr );

      if ( minloc == MAXINT )
	 minloc = 0;

      if ( maxloc == MININT )
	 maxloc = 0;

      if ( secttype == d_abs )
	 minloc -= ( minloc % bytesper68000word );
   }
}

/* OK */
alignbuffers ( )
{
   struct SECTNODE *ptr;

   for ( ptr = sectionlist; ptr != NULL; ptr = ptr->sect_link )
   {
      changesection(yes, ptr);
      align(bytesper68000word);
   }

   changesection(yes, sectionlist);
}

/* OK */
initlocvalues ( )
{
   struct SECTNODE *ptr = sectionlist;

   for ( ptr = sectionlist; ptr != NULL; ptr = ptr->sect_link )
   {
      changesection(yes, ptr);
      minloc   = 0;
      maxloc   = MININT;
      location = 0;
   }
}


/* OK */
allocatevecs ( )
{
   long rel_base_address = 0;
   struct SECTNODE *ptr;

   bavec   = (long*)newvec( ((maxsectnumber+1)*(sizeof(long)))/BYTESPERWORD );
   typevec = newvec( maxsectnumber );
   cvecs   = (string *)newvec( ((maxsectnumber+1)*(sizeof(string)))/BYTESPERWORD );

   relcodewords = 0;
   abscodewords = 0;
   bsswords	= 0;

   for ( ptr = sectionlist; ptr != NULL; ptr = ptr->sect_link )
   {
      string v	    = NULL;

      long mn	    = ptr->sect_min;
      long mx	    = ptr->sect_max;

      int type	    = ptr->sect_type;
      int snum	    = ptr->sect_number;
      int cw        = (mx - mn)/bytesper68000word;

      switch ( type )
      {
      case d_abs:
	 abscodewords += cw;
	 v = (string)gvec(cw*wordsper68000word) - mn/bytesperword ;
	 bavec[ snum ] = 0;
	 break;

      case d_bss:
	 bsswords += cw;
	 if ( objectmodule == o_unix4_2 )
	 {
	    bavec[ snum ] = 0;
	    break;
	 }
      case d_text:
      case d_data:
	 relcodewords += cw;
	 v = (string)gvec(mx/bytesperword);
	 bavec[snum] = rel_base_address;
	 rel_base_address += mx;
	 break;

      case d_offset:
	 break;
      }

      ptr->sect_codevec   = v;
      typevec[snum]	  = type;
      cvecs[snum]	  = ptr->sect_codevec;

      /*  Zero the code vector */

      if ( v != NULL )
      {
	 int i;
	 for ( i = mn ; i <= (mx - 1) ; i++ )
	    v[i] = 0;
      }
   }
}


secondpass ()
{
   allocatevecs();
   initlocvalues();
   changesection( yes, sectionlist );

   location	    = 0;
   symbcount	    = 0;
   fwdreftype	    = s_abs32;
   ended	    = no;
   aborted	    = no;
   errors	    = 0;
   warnings         = 0;
   skiplevel	    = 0;
   skipping	    = 0;
   macrodepth	    = 0;
   getlevel	    = 0;
   macrobase	    = NULL;
   macroend	    = NULL;
   asmlabel	    = 0;
   pass1	    = no;
   pass2	    = yes;
   inmacro	    = no;
   noobj	    = objectmodule == o_none;
   listing	    = parmlisting;
   absrp16	    = 0;
   absrp32	    = 0;
   relrp16	    = 0;
   relrp32	    = 0;
   errormessages    = no;
   length	    = -1;
   charpos	    = 0;
   linepos	    = 0;
   onpage	    = 0;
   sourcelinenumber = 0;
   listlinenumber   = 0;
   pagenumber	    = 0;
   llenfixed	    = no;
   plenfixed	    = no;
   totalwords	    = 0;

   clearbits();
   settitle ( "\000" );

   if ( ! rewind(name_sourcestream) )
   {
      selectoutput(sysout);
      writef( "\051***Error:  Unable to rewind input stream\n" );
      abortassembly();
   }

   current_filename->fname = name_sourcestream;
   current_filename->link  = NULL;

   current_directory->fname = "\002./";
   current_directory->link  = NULL;

   resetflags();
   listed = yes;

#ifdef TRIPOS
   if ( name_hdrfile != NULL )
      triposget(name_hdrfile);
#endif

#ifdef U42
   if ( name_hdrfile != NULL )
      unixget(name_hdrfile);
#endif

#ifdef AMI
   if ( name_hdrfile != NULL )
      amigaget(name_hdrfile);
#endif

   listed = no;

   while ( ! (ended || aborted) )
   {
      resetflags();
      doline();
   }

   if ( skipping != 0 )
      warning ( 103 );
   if ( inmacro )
      warning ( 113 );

   amendlocvalues();
   alignbuffers();
}


int rewind ( name )
string name;
{
   char buff[257];
   fclose(sourcestream);
   bst2cst(name,buff);
   sourcestream = fopen(buff, "r");
   return (sourcestream != NULL);
}


doline ( )
{
#ifdef TRIPOS
   if ( testflags(0x1) )
      error ( 149 );
   if ( testflags(0x8) )
   {
      selectoutput ( sysout );
      writef ( "\040*** Pass %N  Line %N  Errors %N\n", pass1 ? 1 : 2, (long)sourcelinenumber
	 , (long)errors );
      selectoutput ( liststream );
   }
#endif

   labelset    = no;
   loclabelset = no;
   undefined   = no;
   commentline = no;
   codentimes  = 1;

   if ( setjmp( recoverlevel ) != 0 ) goto recoverlabel;

   rch();
   switch ( ch )
   {
   case '*':
   case '\n':
   case ';':
      skiprest();
      symb	  = s_none;
      commentline = yes;
      break;
   case 'A':   case 'B':   case 'C':   case 'D':   case 'E':
   case 'F':   case 'G':   case 'H':   case 'I':   case 'J':
   case 'K':   case 'L':   case 'M':   case 'N':   case 'O':
   case 'P':   case 'Q':   case 'R':   case 'S':   case 'T':
   case 'U':   case 'V':   case 'W':   case 'X':   case 'Y':
   case 'Z':
   case 'a':   case 'b':   case 'c':   case 'd':   case 'e':
   case 'f':   case 'g':   case 'h':   case 'i':   case 'j':
   case 'k':   case 'l':   case 'm':   case 'n':   case 'o':
   case 'p':   case 'q':   case 'r':   case 's':   case 't':
   case 'u':   case 'v':   case 'w':   case 'x':   case 'y':
   case 'z':
   case '.':   case '_':
      if ( !(skiplevel>0) )
      {
         readsymb();
         symbcount++;
         processlabel();
      }
      else
	 symb = s_none;
      break;

   case '0':   case '1':   case '2':   case '3':   case '4':
   case '5':   case '6':   case '7':   case '8':   case '9':
      if ( !(skiplevel > 0) )
      {	
         readsymb();
         if ( symb == s_number )
	    complain(2);
         loclabelset = yes;
         processlabel();
      }
      else
	 symb = s_none;
      break;

   case '\\':
      if ( ! inmacro )
	 if ( ! (skiplevel > 0) )
	    complain ( 117 );
      while ( ! ((ch == ' ') || (ch == '\t') || (ch == '\n')) )
	 rch();
   case ' ':
   case '\t':
      skiplayout();
      if ( (ch == '\n') || (ch == ';') )
      {
	 symb = s_none;
	 commentline = yes;
	 break;
      }
      if ( ch == '\\' )
      {
	 if ( ! inmacro )
	    if ( !(skiplevel >0) )
	       complain ( 117 );
	 while ( ! ((ch == ' ') || (ch == '\t') || (ch == '\n')) )
	    rch();
	 symb = s_none;
	 break;
      }

      if (  (('A' <= ch) && (ch <= 'Z')) ||
	    (('a' <= ch) && (ch <= 'z')) ||
	    (ch == '.')                  ||
	    (ch == '_')   )
      {
	 readsymb();
	 if ( ch == ':' )
	    processlabel();
      }
      else
	 if ( ('0' <= ch) && (ch <= '9') )
	 {
	    readsymb (	);
	    if ( ( symb == s_number ) || ( ch != ':' ) )
	       complain(2);
	    loclabelset = yes;
	    processlabel();
	 }
	 else
	    complain(2);
      break;

   case ENDSTREAMCH:
      symb = s_none;

      if ( getlevel != 0 )
      {
	 getlevel--;
	 listlinenumber--;
	 ended = no;
	 return;
      }

      warning(4);

      ended = yes;
      break;

   default:
      if ( !(skiplevel > 0) )
         complain(5);
   }

   if ( skiplevel > 0 )
   {
      if (symb == s_dir)
	switch( symbtype->st_value_low )
	{
	  case d_endc:
	  case d_ifeq:
	  case d_ifne:
	  case d_iflt:
	  case d_ifle:
	  case d_ifgt:
	  case d_ifge:
	  case d_ifc :
	  case d_ifnc:
          case d_ifd :
          case d_ifnd:
	     dodir();
	  default:
	     commentline = yes;
	}
   }
   else
      if ( undefined && pass2 )
         complain(96);
  
   else
      if (inmacro)
      {
	 if ( (symb == s_dir) &&
	      ((symbtype->st_value_low == d_macro) ||
	       (symbtype->st_value_low == d_endm) )	 )
	    dodir();
	 else
	    if (pass1)
	    {
	       int i;
	       string newbuff = (string)newvec(length / BYTESPERWORD);
	       struct MACRONODE *newnode = heap3ip(NULL, 0, NULL);

	       for ( i = 0 ; i <= ( length - 1 ) ; i++ )
		  newbuff[i] = inputbuff[i];

	       macroend->lineptr = newbuff;
	       macroend->length  = length;
	       macroend->link	 = newnode;

	       macroend = newnode;
	    }

	 commentline = yes;
      }
      else
	 switch (symb)
	 {
	   case s_instr:
	      doinstr();
	      break;
	   case s_dir:
	      dodir();
	      break;
	   case s_macro:
	      domacro();
	      break;
	   case s_none:
	      if (labelset)
		 setlabel (locmode, location, no);
	      break;
	   default:
	      complain ( 3 );
	      break;
	 }

recoverlabel:
   skiprest();
   listline();
}

/* OK */
processlabel ( )
{
   int i;

   if ( ch == ':' )
      rch();

   if (tagsize_given != ts_none)
      complain ( 1 );

   for ( i = 0 ; i <= ( tagsize - 1 ) ; i++ )
      ((int*)labelvec)[ i ] = ((int*)tagv)[ i ];

   labelset = yes;
   undefined = no;
   nextsymb();
}

/* OK */
doinstr ( )
{
   int t     = symbtype->st_type;
   int vh    = symbtype->st_value_high;
   int vl    = symbtype->st_value_low;
   int sizes = 0;

   if ( (secttype == d_bss) || (secttype == d_offset) )
      complain ( 193 );

   instr_mask	 = (symbtype->uval).st_template;
   inst_masktype = (t >> 4) & 0xF;
   source_ea	 = vh;
   dest_ea	 = vl;

   if ( ! aligned(2) )
   {
      warning ( 102 );
      align ( 2 );
   }

   if (labelset)
      setlabel( locmode, location, no );

   nargs = (t >> 11) & 0x3;
   sizes = (t >> 8) & 0x7;

   if ( tagsize_given != ts_none )
   {
      if (tagsize_given != ts_short)
      {
	 int sizebit = 1 << ( tagsize_given - 1 );
	 if ( ( sizes & sizebit ) != 0 )
	    tagsize_given = sizevalue(sizebit);
	 else
	    complain ( 6 );
      }
   }
   else
      if ( ( sizes &  - sizes  ) == sizes )
	 tagsize_given = sizevalue(sizes);
      else
	 tagsize_given = ts_none;

   instr_size = tagsize_given;

   if ( inst_masktype == 0 )
      specialinstruction ( dest_ea );
   else
   {
      if (nargs == 0)
	 readsymb();

      if (instr_size == ts_short)
	 complain ( 86 );

      if (nargs == 1)
      {
	 skiplayout();
	 readsymb();
	 evaluate(effective_address());
	 if ( (dest_ea & op_ea) == 0 )
	    complain(7);
      }

      if ( nargs == 2 )
      {
	 skiplayout();
	 readsymb();
	 evaluate(effective_address());

	 if ( (source_ea & op_ea) == 0 )
	    complain(8);

	 swapoperands();
	 checkfor(s_comma, 10);
	 evaluate(effective_address());

	 if ( (dest_ea & op_ea) == 0 )
	    complain(9);
      }

      if ( symb == s_comma )
	 complain(11);
      else
	 if ( symb != s_none )
	    complain(12);
	 else
	    skiprest();

      generate(inst_masktype);
   }
}


/* OK */
int aligned ( boundary )
int boundary;
{
    return ((location % boundary) == 0);
}


/* OK */
domacro()
{
   int i;
   int narg = 0;
   string argvec[11];
   struct MACRONODE *macrovalue = (symbtype->uval).st_macro;

   if (labelset)
      setlabel(locmode,location,no);

   if ( ((symbtype->st_flags) & stb_setnow) == 0 )
      complain ( 151 );

   for ( i = 1; i <= 10; i++ )
      argvec[i] = (string)"\000";

   argvec[0] = (tagsize_given == ts_byte ? (string)"\001B" :
		tagsize_given == ts_word ? (string)"\001W" :
		tagsize_given == ts_long ? (string)"\001L" :
		tagsize_given == ts_short? (string)"\001S" :
					   (string)"\000");

   skiplayout();

   for ( i = 1 ; i <= 10 ; i++ )
   {
      int j;
      char argbuffer [maxllen + 1];
      int arglength = 0;
      string argb = NULL;

      if ( (ch == '<') || (ch == '[') )
      {
	 int bracket = (ch == '<') ? '>' : ']';
	 narg++;
	 rch();

	 while ( ! ((ch == bracket) || (ch == '\n')) )
	 {
	    argbuffer[ ++arglength ] = ch;
	    rch();
	 }

	 if ( ch == '\n' )
	    complain(114);

	 else
	    rch();
      }
      else
      {
	 if ( ch != '\n' )
	    narg++;

	 while ( ! ((ch == ',')   ||
		    (ch == ' ')   ||
		    (ch == '\n')  ||
		    (ch == '\t'))    )
	 {
	    argbuffer[ ++arglength ] = ch;
	    rch();
	 }
      }

      argbuffer[0] = arglength;
      argb = (string)newvec((arglength/BYTESPERWORD)+1);

      for ( j = 0 ; j <= arglength ; j++ )
	 argb[j] = argbuffer[j];

      argvec[i] = argb;

      readsymb();
      if ( symb == s_none )
	 break;
      else
	 if ( symb != s_comma )
	    complain(115);
   }

   if ( symb == s_comma )
      complain ( 118 );

   expandmacro ( macrovalue, argvec, narg );
}


/* OK */
expandmacro ( macroptr, argvec, narg )
struct MACRONODE *macroptr;
string *argvec;
int narg;
{
	register char *my_inputbuff = inputbuff;
   string macroline = macroptr->lineptr;
   int asml	    = 0;
   int depth	    = macrodepth;
   int skip	    = skipping;
   int skipl	    = skiplevel;

   if (macrodepth == maxmacrodepth)
      complain(108);

   commentline = yes;
   listline();
   stack_NARG(narg);
   macrodepth++;
   resetflags();

   while ( ! ((macroline == NULL) ||
	      (macrodepth == depth)  ||
	      ended		     ||
	      aborted)	)
   {
      register int i;
      register int sptr = 0;
      register int mptr = 0;
      int mlength = macroptr->length;

      for ( i = 0 ; i <= ( maxllen - 1 ) ; i++ )
	 my_inputbuff[ i ] = ' ';

      while ( mptr != mlength )
      {
	 int ch = macroline[mptr];

	 if (ch == '\\')
	 {
	    if ( ++mptr == mlength )
	    {
	       warning(109);
	       break;
	    }

	    ch = macroline[mptr];

	    if ( ch == '@' )
	    {
	       register int i;
	       int chbuff [ 10 ];
	       int size = 0;
	       int label = 0;

	       if (asml == 0)
		  asml = newasmlabel();

	       label = asml;
	       size = digits(asml);

	       if (size < 3)
		  size = 3;

	       for ( i = size; i >= 1; i-- )
	       {
		  chbuff[i] = (label % 10) + '0';
		  label /= 10;
	       }

	       my_inputbuff[sptr] = '.';

	       for ( i = 1 ; i <= size ; i++ )
		  my_inputbuff[sptr + i] = chbuff[i];
	       sptr += (size + 1);
	       mptr++;
	    }
	    else
	       if ( ('0' <= ch) && (ch <= '9') )
	       {
		  register int j;
		  int argnumber = ch - '0';
		  string arg = argvec[ argnumber ];

		  for ( j = 1 ; j <= arg[ 0 ] ; j++ )
		  {
		     my_inputbuff[sptr++] = arg[j];
		  }
		  mptr++;
	       }
	       else
	       {
		  warning(109);
		  break;
	       }
	 }
	 else
	    if (macroline[mptr] == ' ')
	    {
	       while ( (mptr < mlength) &&  (macroline[ mptr ] == ' ') )
		  mptr++;
	       do
		 my_inputbuff[ sptr++ ] = ' ';
		    while ( sptr < mptr );
	    }
	    else
	    {
	       my_inputbuff[sptr++] = ch;
	       mptr++;
	    }
      }

      if (error_found)
      {
	 register int i;

	 for ( i = 0; i <= (mlength - 1); i++ )
	    my_inputbuff[i] = macroline[i];

	 length = mlength;
	 my_inputbuff[length] = '\n';
	 commentline = yes;
	 listlinenumber++;
	 listline();
      }
      else
      {
	 length = sptr;
	 my_inputbuff[ length ] = '\n';
	 listlinenumber++;
	 charpos = 0;
	 doline();
      }

      macroptr	= macroptr->link;
      macroline = macroptr->lineptr;

      resetflags();
   }

   if (macrodepth == depth)
   {
      skipping	= skip;
      skiplevel = skipl;
   }

   destack_NARG();
   macrodepth = depth;
   listed     = yes;
}


/* OK */
stack_NARG ( newvalue )
int newvalue;
{
   lookup ( "\004NARG" );
   nargstack = heap2i(nargstack, (int)(symbtype->st_value));
   symbtype->st_value = newvalue;
}

/* OK */
destack_NARG ( )
{
   lookup ( "\004NARG" );
   symbtype->st_value = nargstack->value;
   nargstack = nargstack->link;
}

/* OK */
int digits ( value )
int value;
{
    return value < 10 ? 1 : digits(value / 10) + 1;
}

/* OK */
int newasmlabel ( )
{
   return ++asmlabel;
}

