/* ****************************************************************************** */
/* *   Assembler for the Motorola MC68000 Microprocessor:  Section 4		* */
/* *	      (c) Copyright 1985, Tenchstar Ltd., Bristol, UK.			* */
/* *										* */
/* *		    Last Modified :  11-APR-85	   (PJF)			* */
/* ****************************************************************************** */

/* M68KASM4 : Automatically translated from BCPL to C on 26-Feb-85 */


#include "libhdr"
#include "asmhdr"


/******************************************/
/*  External functions returning a value  */
/******************************************/

extern struct EXPRESSNODE *expression();   /*  asm5  */
extern struct XDEFNODE	*heap2p();	   /*  asm7  */
extern struct MACRONODE *heap3ip();	   /*  asm7  */
extern int  checkregister();		   /*  asm6  */
extern int  evalexp();			   /*  asm5  */
extern int  *newvec();			   /*  asm7  */
extern int  wordsized();		   /*  asm6  */

/******************************************/
/*  Local functions returning a value	  */
/******************************************/

int readstring();     /* Returns length  */
int doget();	      /* Returns Logical */

/* OK */
dodir()
{
   int	restype = 0;
   int	ressize = 0;
   int	i;
#ifdef SUN
   unsigned char tempvec [256];
#endif
#ifdef IBM
   char tempvec [256];
#endif

   directive = symbtype->st_value_low;

   if ( !  ( (directive == d_org) ||
	     (directive == d_dc)  ||
	     (directive == d_ds)  ||
	     (directive == d_dcb)    ) )
      checktagsize();

   if (tagsize_given == ts_short)
      complain(86);

   switch ( directive )
   {
   case d_set:
   case d_equ:
   case d_equr:
      if (loclabelset)
	 complain(187);
      checklabel(yes);
      nextsymb();
      restype = s_mask & evalexp(expression());
      checkexpression(restype);
      setlabel(restype, value, directive == d_set);

      if (extrnref)
      {
	 lookup(labelvec);
	 symbtype->st_type	  = s_ext;
	 (symbtype->uval).st_xref = extrnlsymb;
      }
      listEQU(restype);
      break;

#ifndef AMI
   case d_org:
      {
	 switch ( tagsize_given )
	 {
	 case ts_long:
	    ressize = s_abs32; break;
	 case ts_none:
	 case ts_word:
	    ressize = s_abs16; break;
	 default:
	    complain ( 46 );
	 }
      };
      nextsymb (  );
      restype = evalexp(expression());
      checkexpression ( restype );
      if ( ! ( ( value & 0xFF000000 ) == 0 ) )
	 complain ( 138 );
      if ( locmode == s_rel )
	 changesection ( no, "\010ABSOLUTE", d_abs );
      fwdreftype = ressize;
      setloc ( value );
      if ( labelset )
	 setlabel ( locmode, location, no );
      break;
#endif

   case d_rorg:
      nextsymb();
      restype = evalexp(expression());
      checkexpression(restype);
      checknone();

      if ( locmode != s_rel )
	 changesection ( no, "\000", d_text );

      fwdreftype = s_abs32;
      setloc(value);
      if (labelset)
	 setlabel( locmode, location, no );
      break;

   case d_section:
      dosection(tempvec);
      break;

   case d_offset:
      checklabel ( no );
      nextsymb (  );
      restype = evalexp(expression());
      checkexpression ( restype );
      changesection ( no, "\006OFFSET", d_offset );
      setloc ( value );
      break;

   case d_text:
      checknone();
      changesection( no, "\000", d_text);
      fwdreftype = s_abs32;
      if ( labelset )
	 setlabel ( locmode, location, no );
      break;

   case d_data:
      checknone (  );
      changesection ( no, "\000", d_data );
      fwdreftype = s_abs32;
      if ( labelset )
	 setlabel ( locmode, location, no );
      break;

   case d_bss:
      checknone (  );
      changesection ( no, "\000", d_bss );
      fwdreftype = s_abs32;
      if ( labelset )
	 setlabel ( locmode, location, no );
      break;

   case d_idnt:
      doidnt ( tempvec );
      break;

   case d_noformat:
   case d_format:
   case d_mask2:
      checklabel_and_none ( no );
      break;

   case d_dc:
      defineconstants ( tagsize_given );
      break;

   case d_ds:
      definestorage ( tagsize_given );
      break;

   case d_dcb:
      defineconstantblock ( tagsize_given );
      break;

   case d_list:
      dolist ( d_list );
      break;

   case d_nolist:
      dolist ( d_nolist );
      break;

   case d_spc:
      checklabel ( no );
      nextsymb (  );
      checkfor ( s_number, 48 );
      checknone (  );
      spacelines ( number );
      listed = yes;
      break;

   case d_page:
      checklabel_and_none ( no );
      onpage = 0;
      listed = yes;
      break;

   case d_nopage:
      checklabel_and_none ( no );
      paging = no;
      commentline = yes;
      break;

   case d_plen:
   case d_llen:
      checklabel ( no );
      nextsymb (  );
      checkfor ( s_number, 48 );
      checknone (  );
      if ( directive == d_plen )
	 dolen ( &plenfixed, 124, minplen, maxplen, 100, &linesperpage);
      else
	 dolen ( &llenfixed, 125, minllen, maxllen, 101, &charsperline);
      break;

   case d_ttl:
      ressize = 1;
      checklabel ( no );
      skiplayout (  );
      while ( ! ( ( ressize > titlecharsmax ) || ( ch == '\n' ) ) )
      {
	 tempvec[ ressize++ ] = ch;
	 rch();
      }
      tempvec[ 0 ] = ressize - 1;
      if ( ch != '\n' )
	 warning(49);
      settitle(tempvec);
      listed = yes;
      break;

   case d_noobj:
      checklabel_and_none ( no );
      noobj = yes;
      commentline = yes;
      break;

   case d_ifd :
   case d_ifnd:
   case d_ifc :
   case d_ifnc:
   case d_ifeq:
   case d_ifne:
   case d_iflt:
   case d_ifle:
   case d_ifgt:
   case d_ifge:
      checklabel(no);
      skipping ++;

      if ( ( directive == d_ifc ) || ( directive == d_ifnc ) )
      {
	 string ptr1 = tempvec;
	 string ptr2 = NULL;
	 skiplayout();
	 getstring (ptr1, 2);
	 if ( ch != ',' )
	    complain ( 10 );
	 rch();
	 ptr2 = tempvec + ( ptr1[ 0 ] + 1 );
	 getstring ( ptr2, 2 );
	 checknone();
	 value = compstring(ptr1, ptr2);
      }
      else if ( (directive == d_ifd) || (directive == d_ifnd) )
      {
         nextsymb();
         
         if ( ((symb & s_mask) < s_star) )
           value = (symbtype->st_flags) & stb_setnow;
         else 
           complain( 196 );

         nextsymb();
         checkfor( s_none, 197 );
      }
      else 
      {
	 nextsymb ();
	 restype = evalexp(expression());
	 checkexpression ( restype );
      }

      switch ( directive )
      {
      case d_ifc:
      case d_ifeq:
      case d_ifnd: 
	 value = (value == 0);
	 break;
      case d_ifnc:
      case d_ifne:
      case d_ifd :
	 value = (value != 0);
	 break;
      case d_iflt:
	 value = (value < 0);
	 break;
      case d_ifle:
	 value = (value <= 0);
	 break;
      case d_ifgt:
	 value = (value > 0);
	 break;
      case d_ifge:
	 value = (value >= 0);
	 break;
      }

      if ( skiplevel <= 0 )
      {
	 if ( value == false )
	    skiplevel = skipping;

	 if ( pass2 && ( listing || error_found ) )
	 {
	    preparedefaultbuffer (  );
	    linepos = 0;
	    writestring ( "\006      " );
	    linepos = 11;
	    writestring ( value ? "\004True" : "\005False" );
	    printbuffer (  );
	    error_found = no;
	    listed = yes;
	 }
      }
      break;

   case d_endc:
      checklabel_and_none ( no );
      if ( skipping == 0 )
	 complain ( 107 );
      if ( --skipping < skiplevel )
	 skiplevel = 0;
      commentline = yes;
      break;

   case d_macro:
      if ( loclabelset )
	 complain ( 187 );
      checklabel_and_none ( yes );
      if ( inmacro )
	 complain ( 110 );
      if ( macrodepth > 0 )
	 complain ( 121 );
      for ( i = 0 ; i <= ( tagsize - 1 ) ; i++ )
	 macroname[ i ] = labelvec[ i ];
      macrobase = pass1 ? heap3ip(NULL, 0, NULL) : NULL;
      macroend = macrobase;
      macrodepth++;
      inmacro = yes;
      commentline = yes;
      break;

   case d_endm:
      checklabel_and_none ( no );
      if ( inmacro )
	 macrodepth--;
      else
	 complain ( macrodepth == 0 ? 111 : 120 );

      for ( i = 0 ; i <= ( tagsize - 1 ) ; i++ )
	 labelvec[ i ] = macroname[ i ];

      inmacro = no;
      setlabel ( s_macro, macrobase, no );
      commentline = yes;
      break;

   case d_mexit:
      checklabel_and_none ( no );
      if ( macrodepth == 0 )
	 complain ( 112 );
      else
	 macrodepth--;
      listed = yes;
      break;

   case d_reg:
      doreg();
      break;

   case d_end:
      if ( labelset )
	 setlabel ( locmode, location, no );
      checknone (  );
      if ( macrodepth > 0 )
	 complain ( 119 );
      if ( getlevel > 0 )
	 complain ( 126 );
      ended = yes;
      commentline =  ~ labelset ;
      break;

   case d_get:
      checklabel(no);
      skiplayout();

      if ( ( ch == '\'' ) || ( ch == '\"' ) )
      {
	 int quote = ch;
	 rch();
	 ressize = 0;
	 while ( ! ( ( ch == quote ) || ( ch == '\n' ) ) )
	 {
	    tempvec[ ++ressize ] = ch;
	    rch();
	 }
	 if ( ch == '\n' )
	    complain ( 127 );
	 if ( macrodepth > 0 )
	    complain ( 133 );
	 tempvec[ 0 ] = ressize;
	 skiprest (  );

#ifdef TRIPOS
	 triposget(tempvec);
#endif

#ifdef U42
	 unixget(tempvec);
#endif

#ifdef AMI
	 amigaget(tempvec);
#endif

	 listed =  ~ error_found ;
	 break;
      }
      else
	 complain ( 129 );

   case d_cnop:
      {
	 int  offset = 0;
	 int  base   = 0;

	 nextsymb();
	 if ( symb != s_number )
	    complain(48);
	 offset = (int)(number);
	 readsymb();
	 checkfor ( s_comma, 10 );
	 if ( symb != s_number )
	    complain(48);
	 base = number;
	 readsymb();
	 checkfor(s_none, 12);
	 if (base == 0)
	    complain(150);
	 else
	 {
	    long loc = location;
	    align ( base );
	    if (offset != 0)
	    {
	       setloc ( location + offset );
	       if ( (location - loc) >= base  )
		  setloc ( location - (long)base );
	    }
	 }
	 if ( labelset )
	    setlabel ( locmode, location, no );
      }
      break;

   case d_entry:
      if ( pass2 )
      {
	 checklabel ( no );
	 nextsymb (  );
	 do
	 {
	    switch ( symb & s_mask )
	    {
	    case s_abs32:
	    case s_abs16:
	    case s_rel:
	       entrysymbols = heap2p(entrysymbols, symbtype);
	       break;
	    default:
	       complain ( (symb == s_new) ? 153 :
			  (symb == s_ext) ? 154 : 155 );
	       break;
	    }

	    if ( (symbtype->st_name)[0] > maxextlength )
	       warning ( 165 );
	    readsymb (	);
	    if ( symb == s_comma )
	       readsymb (  );
	    else
	       break;
	 } while ( true );
	 checkfor ( s_none, 156 );
      }
      commentline = yes;
      break;

   case d_extrn:
      checklabel ( no );
      nextsymb (  );
      do
      {
	 int  correct = pass1 ? s_new : s_ext;
	 int  type    = (symbtype->st_type) & st_type_mask;
	 int  flags   = (symbtype->st_flags);

	 if ( symb == correct )
	 {
	    if ( locallabel )
	       complain ( 187 );
	    if ( pass2 )
	       if ( (symbtype->st_name)[0] > maxextlength )
		  warning(166);
	    if ( pass1 )
	    {
	       struct XREFNODE *space =
		    (struct XREFNODE *)newvec(sizeof(struct XREFNODE));
	       type		= s_ext;
	       flags		= stb_setever;
	       space->e_link	= extrnsymbols;
	       space->e_symbol	= symbtype;
	       space->e_count32 = 0;
	       space->e_refs32	= 0;
	       space->e_count16 = 0;
	       space->e_refs16	= 0;
	       space->e_count8	= 0;
	       space->e_refs8	= 0;
	       extrnsymbols = space;
	    }
	 }
	 else
	    complain ( (symb == s_abs16)	 ||
		       (symb == s_abs32)	 ||
		       ((s_mask & symb) == s_rel)  ? 157 : 158 );

	 symbtype->st_type  = type;
	 symbtype->st_flags = flags | stb_setnow;
	 readsymb();
	 if ( symb == s_comma )
	    readsymb();
	 else
	    break;
      } while (true);
      checkfor(s_none, 159);
      commentline = yes;
      break;

   case d_fail:
      complain(122);
      break;

   default:
      complain(1004);
   }
   skiprest();
}


/* OK */
dosection ( namevec )
string namevec;
{
   int	i;
   int	type = d_text;
   string s = (string)"\010ABSOLUTE";
   readstring ( namevec );

   if ( ch == ',' )
   {
      rch (  );
      readsymb (  );
      switch ( symbtype->st_value_low )
      {
      case d_text:
	 break;
      case d_data:
	 type = d_data;
	 break;
      case d_bss:
	 type = d_bss;
	 break;
      case d_abs:
	 for ( i = 0 ; i <= s[ 0 ] ; i++ )
	    namevec[ i ] = s[ i ];
	 type = d_abs;
	 break;
      default:
	 complain ( 189 );
      }
   }
   else
      if ( compstring(namevec, s) == 0 )
	 type = d_abs;
   checknone (	);
   changesection ( no, namevec, type );
   fwdreftype = s_abs32;
   if ( labelset )
      setlabel ( locmode, location, no );
}

/* OK */
doidnt ( namevec )
string namevec;
{
   int i;
   int length = readstring(namevec);
   PUname = (string)newvec(( (length - 1)/BYTESPERWORD) + 1);
   for ( i = 0 ; i <= length ; i++ )
      PUname[i] = namevec[i];
   checknone();
}

/* OK */
int readstring ( namevec )
string namevec;
{
   int quoted = no;
   int count = 0;

   while ( ! ((ch != ' ') && (ch != '\t')) )
      rch();

   if ( ch == '\"' )
   {
      quoted = yes;
      rch();
   }

   do
   {
      if ( quoted )
      {
	 if ( ch == '\"' )
	    break;
	 if ( ch == '\n' )
	    complain ( 188 );
      }
      else
	 if ( ( ( ch == ' ' ) || ( ch == ',' ) ) || ( ch == '\n' ) )
	    break;
      namevec[ ++count ] = ch;
      rch();
   } while ( true );

   if ( ch == '\"' )
      rch();

   namevec[0] = count;

   return count;
}


/* OK */
getstring ( ptr, n )
string ptr;
int n;
{
   int count = 0;
   if ( ch != '\'' )
      complain (191);
   do
   {
      rch();

      if (ch == '\'')
      {
	 rch();
	 if ( ch != '\'' )
	    break;
      }

      if ( ch == '\n' )
	 complain ( 57 );

      ptr[ ++count ] = ch;

   } while ( true );

   ptr[ 0 ] = count;
}

/* OK */
dolen ( fixedflag, err1, rmin, rmax, err2, parm )
int *fixedflag, err1, rmin, rmax, err2, *parm;
{
   if ( (*fixedflag) )
      complain(err1);
   if (  !((rmin <= number) && (number <= rmax)) )
      complain(err2);

   (*parm) = number;
   (*fixedflag) = yes;
   listed = yes;
}

/* OK */
checklabel_and_none ( flag )
int flag;
{
   checklabel(flag);
   checknone();
}

/* OK */
checknone ( )
{
   readsymb();
   if ( symb != s_none )
      complain ( 47 );
}

/* OK */
dolist ( parm )
int parm;
{
   checklabel_and_none(no);
   listing = (parm == d_list) ? parmlisting : no;
   listed  = yes;
}


#ifdef TRIPOS
triposget ( file )
string file;
{
   SCBPTR stream = findinput(file);

   if ( stream == NULL )
   {
      word save = currentdir;
      word lock = locateobj(dir_incl);

      if ( ! ( lock == 0 ) )
      {
	 currentdir = lock;
	 stream = findinput(file);
	 currentdir = save;
	 freeobj ( lock );
      }
   }

   if ( ! doget(stream) )
   {
      selectoutput ( sysout );
      writef ( "\027***  Cannot open \"%S\": ", file );
      fault(result2);
      selectoutput ( liststream );
      error( 128 );
   }
}
#endif

#ifdef AMI
amigaget ( file )
string file;
{
   FILE *stream = NULL;
   int pos;
   
#ifdef SUN
   unsigned char fname[256];
#endif
#ifdef IBM
   char fname[256];
#endif
   int i;

   dir_incl->file = current_directory->fname;

   if ( stream == NULL )
   {
      struct DIRNODE *ptr = dir_incl;

      while ( (stream == NULL) && (ptr != NULL) )
      {
	string dirname = ptr->file;
	int dirl = dirname[0];
        pos = 0;

	for ( i = 1 ; i <= dirl ; i++ )
	   fname[pos++] = dirname[i];

#ifdef IBM
        if ( (dirname[dirl] != '\'))
	   fname[pos++] = '\\';
#endif
#ifdef SUN
        if ( (dirname[dirl] != '/'))
	   fname[pos++] = '/';
#endif

	for ( i = 1 ; i <= file[0] ; i++ )
	   fname[pos++] = file[i];

	fname[pos] = 0;

	stream = fopen(fname, "r");
	/* printf("Open of %s gave res %d\n", fname, stream); */
	ptr    = ptr->link;
     }
   }

   /* When we get here, 'fname' holds the name of the file
      which we've just opened (if we managed to open one) */

   push_cd(fname,pos);

   if ( ! doget(stream, file) )
   {
      selectoutput( sysout );
      writef( "\026*** Cannot open \"%S\"\n", (long)file );
      selectoutput( liststream );
      error( 128 );
   }

   pop_cd();
}
#endif


push_cd(name,namelength)
char *name;
int namelength;
{
   struct FNAMENODE *temp = (struct FNAMENODE *)newvec (
                (sizeof(struct FNAMENODE))/BYTESPERWORD );
   string space;
   int i;

   /* printf("Push CD1: %s [%d]\n", name, namelength ); */  

   /* Search backwards until we hit a '/' => directory */

   while (namelength >= 0)
   {
#ifdef SUN
      if (name[namelength] == '/') break;
#else
      if (name[namelength] == '\\') break;
#endif
      namelength--;
   }

   space = (string)newvec( namelength/BYTESPERWORD );

   /* printf("Push CD2: %s [%d]\n", name, namelength ); */

   temp->link  = current_directory;
   temp->fname = space;
           
   for (i=0; i<namelength; i++)
     space[i+1] = name[i];
   space[0] = namelength;
 
   current_directory = temp;

   /* printf("Push CD3: %s [%d]\n", (current_directory->fname)+1, 
				 (current_directory->fname)[0] ); */
}

pop_cd()
{
   current_directory = current_directory->link;
}

int doget ( inputstream, filename )
FILE *inputstream;
string filename;
{
   if ( inputstream == NULL )
     return no;
   else
     {
	FILE *oldinput = sourcestream;
	int  oldgl     = getlevel;
	int  oldln     = sourcelinenumber;

	if ( getlevel == maxgetlevel )
	{
	   fclose(inputstream);
	   warning(130);
	   return yes;
	}

	commentline = yes;
	listline();

        /* Put filename on top of stack */
        {
           struct FNAMENODE *temp = (struct FNAMENODE *)newvec (
                (sizeof(struct FNAMENODE))/BYTESPERWORD );
           int length = filename[0];
           string space = (string)newvec( length/BYTESPERWORD );
           int i;

           temp->link  = current_filename;
           temp->fname = space;
           
           for (i=0; i<=length; i++)
             space[i] = filename[i];
 
           current_filename = temp;
        }

	getlevel++;
	sourcelinenumber = 0;
        sourcestream = inputstream;

	while ( ! ( ( getlevel == oldgl ) || ended ) )
	{
	   resetflags (  );
	   doline (  );
	}

	fclose(inputstream);
	sourcestream = oldinput;

	if ( inmacro )
	{
	   inmacro = no;
	   warning ( 132 );
	}

	sourcelinenumber = oldln;

        /* Pop filename from the top of the stack */
        current_filename = current_filename->link;

	return yes;
     };
}

/* OK */
settitle ( str )
string str;
{
   int i;
   int l = str[ 0 ];
   int m = ((titlecharsmax - l) / 2) - 1;

   for ( i = 0 ; i <= ( titlecharsmax - 1 ) ; i++ )
      titlevec[ i ] = ' ';

   for ( i = 1 ; i <= l ; i++ )
      titlevec[ m + i ] = str[ i ];
}

/* OK */
setlabel(type, value, alterable)
int type, alterable;
long value;
{
   int savesymb = symb;
   struct SYMNODE *savest = symbtype;
   int muldef	= no;
   undefined	= no;

   lookup(labelvec);

   if (type == s_abs)
      type = wordsized(value) ? s_abs16 : s_abs32;
   else
      if (type == s_rel)
	 type = type | ( sectnumber << 8 );
   if ( symb != s_new )
      if ( alterable )
      {
	 if ( ((symbtype->st_flags) & stb_equ) != 0 )
	    complain(104);
      }
      else
	 if ( ((symbtype->st_flags) & stb_set) != 0 )
	    complain ( 105 );

   switch ( s_mask & symb )
   {
   case s_Ar:
   case s_Dr:
      if ( directive != d_equr )
	 complain ( 52 );
   case s_abs16:
   case s_abs32:
   case s_rel:
      no_labels = no;
      if ( alterable ) goto labelsnew;
   case s_reg:
      if ( pass2 )
      {
	 int	   otype = (symbtype->st_type) & st_type_mask;
	 long ovalue = symbtype->st_value;

	 if ( otype == s_rel )
	    otype = symbtype->st_type;
	 if ( errors <= 0 )
	    if ( ! ((otype == type) && (ovalue == value)) )
	       complain(51);
	 symbtype->st_flags = symbtype->st_flags | stb_setnow;
      }
      else
      {
	 int  otype = (symbtype->st_type) & st_type_mask;
	 long ovalue = symbtype->st_value;

	 if ( otype == s_rel )
	    otype = symbtype->st_type;
	 if ( ! ( (otype == type)   &&
		  (ovalue == value) &&
		  (symbtype->st_definition == 0)) )
	 {
	    symbtype->st_flags = symbtype->st_flags | stb_muldef;
	    muldef = yes;
	 }
      }
      break;

   case s_macro:
      if ( pass2 )
	 symbtype->st_flags = symbtype->st_flags | stb_setnow;
      else
      {
	 symbtype->st_flags = symbtype->st_flags | stb_muldef;
	 muldef = yes;
      }
      break;

   case s_new:
labelsnew:
      symbtype->st_type  = ((symbtype->st_type) - symb ) | type;
      symbtype->st_flags = (symbtype->st_flags)       |
			   (stb_setnow + stb_setever) |
			   (alterable ? stb_set : stb_equ);
      symbtype->st_number = sectnumber;
      symbtype->st_value  = value;

      if ( type == s_macro )
	(symbtype->uval).st_macro = (struct MACRONODE *)value;

      if ( (type == s_Ar) || (type == s_Dr) )
      {
	int i;
	string name = symbtype->st_name;
	symbtype->st_value_low = value;
	for (i=1; i<=name[i]; i++)
	  name[i] = capitalch( name[i] );
      }
      break;

   case s_ext:
      {
	 struct XREFNODE *s = extrnsymbols;
	 while ( s != NULL )
	 {
	    if (symbtype == s->e_symbol)
	       complain(160);
	    s = s->e_link;
	 }
      }
      break;

   case s_instr:
      complain(53);

   case s_dir:
      complain(54);

   default:
      complain(55);

   }
   if ( undefined && pass2 )
      complain(95);
   if ( pass1 &&  ! systemwords  )
      symbtype->st_definition = muldef ? cr_multiple :
      alterable ? cr_setsymbol : sourcelinenumber;
   symb     = savesymb;
   symbtype = savest;
}

/* OK */
resetflags ( )
{
   error_found = no;
   listed = no;
   commentline = no;
   undefined = no;
   fwdref = no;
   extrnref = no;
   op1_extrnref = no;
   relocation_needed = no;
   op1_relocation_needed = no;
   expvecp = expvec + expsize;
   codewords = 0;
   bytesonline = 0;
   nitems = 0;
   op_ea = 0;
   op1_ea = 0;
}

