/* ****************************************************************************** */
/* *   Assembler for the Motorola MC68000 Microprocessor:  Section 6		* */
/* *	      (c) Copyright 1985, Tenchstar Ltd., Bristol, UK.			* */
/* *										* */
/* *		    Last Modified :  11-APR-85	   (PJF)			* */
/* ****************************************************************************** */

/* M68KASM6 : Automatically translated from BCPL to C on 26-Feb-85 */


#include  "libhdr"
#include  "asmhdr"


/******************************************/
/*  External functions returning a value  */
/******************************************/

extern int evalexp();			   /*  asm5  */
extern struct EXPRESSNODE *expression();   /*  asm5  */
extern struct RELOCNODE *heap4();	   /*  asm7  */
extern int *newvec();			   /*  asm7  */

/*****************************************/
/* Local fuctions returning a value	 */
/*****************************************/

int wordsized();
int bytesized();
int absolute();
int relocatable();
int checkregister();
int bytesize();
int sizefield();
int eafield();

/* OK */
defineconstants ( size )
int size;
{
   if ( ( secttype == d_offset ) || ( secttype == d_bss ) )
      complain ( 194 );
   skiplayout (  );

   if ( size == ts_byte )
      do
	 definebyte (  ); while ( symb == s_comma );
   else
   {
      align(bytesize(ts_word));
      nitems = 0;
      do
	 defineword ( size, bytesize(size) ); while ( symb == s_comma );
   }

   if (labelset)
     setlabel(locmode,location,no);

   checkfor ( s_none, 77 );
}


/* OK */
definebyte ( )
{
   extrnref = no;
   if ( ch == '\'' )
   {
      do
      {
	 rch (	);
	 if ( ch == '\'' )
	 {
	    rch (  );
	    if ( ! ( ch == '\'' ) )
	       break;
	 }
	 if ( ch == '\n' )
	    complain ( 57 );
	 stackvalue ( s_abs16, 1, (long)ch, no, NULL );
      } while ( true );
      readsymb (  );
   }
   else
   {
      readsymb (  );
      if ( ! absolute(evalexp(expression())) )
	 complain ( 123 );
      if ( pass2 &&  ! bytesized(value)  )
	 warning ( 176 );
      stackvalue ( s_abs16, 1, value, extrnref, extrnlsymb );
   }
}

/* OK */
defineword ( size, bs )
int size, bs;
{
   int restype;
   extrnref = no;
   fwdref = no;
   readsymb (  );
   restype = evalexp(expression());
   if ( pass2 )
      if ( ( size == ts_word ) &&  ! wordsized(value)  )
	 warning ( 175 );
   stackvalue ( restype, bs, value, extrnref, extrnlsymb );
}

/* OK */
definestorage ( size )
int size;
{
   long incr = 0;
   int	restype = 0;
   int	bs = bytesize(size);

   nextsymb();
   restype = evalexp(expression());

   if ( fwdref )
      complain ( 79 );
   else
      if ( extrnref )
	 complain ( 164 );
      else
	 if ( symb != s_none )
	    complain ( 47 );
	 else
	    if (  ! absolute(restype)  )
	       complain ( 71 );

   if ( value < 0 )
   {
      complain ( 178 );
      value = 0;
   }

   incr = value * (long)bs;
   align ( bytesize(size == ts_byte ? size : ts_word) );
   if ( labelset )
      setlabel ( locmode, location, no );
   if ( pass2 && listing )
   {
      preparedefaultbuffer (  );
      linepos = 10;
      writechar ( '=' );
      writehexvalue ( incr, 4 );
      printbuffer (  );
      error_found = no;
      listed = yes;
   }
   setloc ( location + incr );
}

/* OK */
defineconstantblock ( size )
int size;
{
   int	length = 0;
   int	restype = 0;
   int	bs = bytesize(size);

   if ( ( secttype == d_bss ) || ( secttype == d_offset ) )
      complain ( 194 );

   nextsymb (  );
   restype = evalexp(expression());
   if ( fwdref )
      complain ( 179 );
   else
      if ( extrnref )
	 complain ( 164 );
      else
	 if (  ! absolute(restype)  )
	    complain ( 180 );
	 else
	    if ( value <= 0 )
	    {
	       complain ( 181 );
	       length = 0;
	    }
	    else
	       length = (int)value;

   if ( symb != s_comma )
      complain ( 10 );

   if ( size == ts_byte )
      definebyte (  );
   else
   {
      align(bytesize(ts_word));
      defineword ( size, bs );
   }

   if (labelset)
     setlabel(locmode,location,no);

   checkfor ( s_none, 77 );

   codentimes = length;
}

/* OK */
checktagsize ( )
{
   if ( tagsize_given != ts_none )
      complain ( 80 );
}

/* OK */
int wordsized ( operand )
long operand;
{
    return ( (-32768 <= operand) & (operand <= 32767)) |
	     ( (operand & 0xFFFF) == operand );
}

/* OK */
int bytesized ( operand )
long operand;
{
    return ( (-128 <= operand) & (operand <= 127)) |
	     ((operand & 0xFF) == operand);
}

/* OK */
int absolute ( ea )
int ea;
{
    return ( ea == s_abs16 ) | ( ea == s_abs32 );
}

/* OK */
int relocatable ( ea )
int ea;
{
    return ( ea & s_mask ) == s_rel;
}

int checkregister ( reg )
struct EXPRESSNODE *reg;
{
   int rnum  = (reg->uvalleft).intvalue;
   int rsize = (reg->uvalright).value;

   if ( rsize == ts_none )
      return rnum;
   else
      complain ( 81 );
}

/* OK */
checklabel ( possible )
int possible;
{
   if ( labelset != possible )
      complain ( possible ? 82 : 83 );
}

/* OK */
nextsymb ( )
{
   skiplayout();
   readsymb();
}

/* OK */
spacelines ( n )
int n;
{
   if ( pass2 && listing )
   {
      int i; /* FOR loop variables */
      clearbuffer (  );
      for ( i = 1 ; i <= n ; i++ )
	 printbuffer (	);
      listed = yes;
   }
}

/* OK */
printbuffer ( )
{
   if ( pass2 && ( error_found || listing ) )
   {
      register int i; /* FOR loop variables */
      register int linelength = 0;

      for ( i = charsperline - 1 ; i >= 0 ; i-- )
	 if ( ! ( outbuff[ i ] == ' ' ) )
	 {
	    linelength = i + 1;
	    break;
	 }
      if ( ( ( onpage % ( linesperpage - 5 ) ) == 0 ) && paging )
	 pageheading (	);

#ifdef TRIPOS
      for ( i = 0 ; i <= ( linelength - 1 ) ; i++ )
	 wrch ( outbuff[ i ] );
      newline (  );
#endif

#ifdef U42
      wbytes ( outbuff, linelength );
      newline (  );
#endif

#ifdef AMI
      wbytes ( outbuff, linelength );
      newline (  );
#endif

      onpage++;
   }
}

/* OK */
wch ( ch )
int ch;
{
   if ( ch == '\n' )
   {
      (*wrchsave)( '\n' );
      linepos = 0;
   }
   else
      if ( ! ( linepos >= charsperline ) )
      {
	 (*wrchsave)( ch );
	 linepos++;
      }
}

/* OK */
pageheading ( )
{
   if ( pass2 && paging )
   {
      int i;
      wrchsave = wrch_fn;
      wrch_fn  = wch;
      linepos  = 0;
      pagenumber += 1;

      writef ("\043\fMC68000 ASSEMBLER VERSION %N.%N   ",
		   (long)version, (long)release );
      for ( i = 0 ; i <= ( titlecharsmax - 1 ) ; i++ )
	 (*wrch_fn)( titlevec[ i ] );
      writef ( "\024 %S %S     PAGE %N\n\n", (long)datestring, (long)timestring,
	     (long)pagenumber );
      if ( crossreference )
  writes ( "\105            SYMBOL               DEFN   VALUE             REFERENCES\n" );
      else
	 if ( ! errormessages )
  writes ( "\110   LOC              OBJECT             STMT            SOURCE STATEMENT\n" );
      writes ( "\002\n\n" );
      wrch_fn = wrchsave;
      onpage = 0;
   }
}

/* OK */
int bytesize ( size )
int size;
{
   switch ( size )
   {
   case ts_long:
      return 4;
   case ts_word:
      return 2;
   case ts_byte:
      return 1;
   case ts_none:
      return bytesize(ts_default);
   default:
      complain ( 1010 );
   }
}

/* OK */
checkexpression ( type )
int type;
{
   if ( fwdref )
      complain ( 79 );
   else
      if ( extrnref &&	!( ( directive == d_equ ) || ( directive == d_set
	  ) ) )
	 complain ( 164 );
      else
	 checkfor ( s_none, 47 );
   switch ( directive )
   {
   case d_equr:
      if ( ! ( ( type == s_Ar ) || ( type == s_Dr ) ) )
	 complain ( 84 );
      break;
   case d_ifeq:
   case d_ifne:
   case d_iflt:
   case d_ifle:
   case d_ifgt:
   case d_ifge:
   case d_org:
      if ( ! absolute(type) )
	 complain ( 71 );
      break;
   default:
      if ( ! ( absolute(type) || relocatable(type) ) )
	 complain ( 85 );
      break;
   }
}

/* OK */
listline ( )
{
   if ( pass2 && ( listing || error_found ) )
      printline (  );
   else
      codeline (  );
}

/* OK */
printline ( )
{
   if ( ! ( listed &&  ! error_found  ) )
   {
      preparedefaultbuffer (  );
      codeline (  );
      printbuffer (  );
   }
}

/* OK */
preparedefaultbuffer ( )
{
   register int i;
   clearbuffer();
   linepos = 0;
   if ( ! commentline )
   {
      writehexvalue ( location, locmode == s_rel ? 4 : 6 );
      if ( locmode == s_rel )
	 writechar ( '\'' );
   }
   linepos = 38;
   writenumber ( sourcelinenumber, 5 );
   if ( ( macrodepth > 0 ) &&  ! inmacro  )
   {
      linepos = 43;
      writechar ( '+' );
   }
   linepos = 44;
   for ( i = 0 ; i <= ( length - 1 ) ; i++ )
      writechar ( inputbuff[ i ] );
   if ( error_found )
   {
      linepos = 35;
      writechar ( 'E' );
   }
}

/* OK */
codeline ( )
{
   register int i;
   for ( i = 1 ; i <= codentimes ; i++ )
   {
      int itemsprinted;
      for ( itemsprinted = 0 ; itemsprinted <= ( nitems - 1 ) ; itemsprinted++ )
      {
	 int offset = itemsprinted * (sizeof(struct SVNODE)/BYTESPERWORD);
	 struct SVNODE *cb = (struct SVNODE *)(codebuff + offset);
	 int dtype   = cb->type;
	 int dsize   = cb->size;
	 long dvalue = cb->value;
	 int dext    = cb->extref;
	 struct SYMNODE *dsymb = cb->extsymb;

	 if ( dext )
	    addextrnref ( dsymb, location, dsize );
	 if ( relocatable(dtype) )
	    relocate ( location, dsize, (dtype >> 8) & 0xFF );
	 if ( pass2 && ( listing || error_found ) )
	    writebytes ( dsize, dvalue );

	 codebytes ( dsize, dvalue );
      }
   }
}

/* OK */
writebytes ( dsize, dvalue )
int dsize;
long dvalue;
{
   int i;
   for ( i = dsize - 1 ; i >= 0 ; i-- )
      writebyte ( (char)( (dvalue >> (i*8)) & 0xFF ) );
}

/* OK */
codebytes ( dsize, dvalue )
int dsize;
long dvalue;
{
   int i;
   for ( i = dsize - 1 ; i >= 0 ; i-- )
      codebyte ( (char)((dvalue >> (i*8)) & 0xFF) );
}

/* OK */
writebyte ( b )
char b;
{
   static int positions[] = { 11, 13, 17, 19, 23, 25, 29, 31 };
   if ( bytesonline == 8 )
   {
      printbuffer (  );
      clearbuffer (  );
      commentline = yes;
      bytesonline = 0;
   }
   linepos = positions[ bytesonline ];
   writehexvalue( (long)b, 2 );
   bytesonline++;
}

/* OK */
codebyte ( b )
char b;
{
   setloc ( location );
   if ( pass2 )
      ((char*)codevec)[ location ] = b;
   setloc ( location + 1 );
}

/* OK */
align ( boundary )
int boundary;
{
   long try = (location + (long)boundary) - 1;
   long decr = try % boundary;
   setloc ( (long)(try - decr) );
}

/* OK */
writehexvalue ( h, d )
long h;
int d;
{
   static char chvec[] =   { '0', '1', '2', '3', '4', '5', '6', '7',
			   '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
   if ( d > 1 )
      writehexvalue ( h >> 4, d - 1 );
   writechar ( chvec[ h & 0xF ] );
}

/* OK */
writenumber ( n, d )
int n, d;
{
   if ( d > 1 )
      writenumber ( n / 10, d - 1 );
   writechar ( (char)(n == 0 ? ' ' : ( n % 10 ) + '0') );
}

/* OK */
writestring ( s )
string s;
{
   int i;
   for ( i = 1 ; i <= s[ 0 ] ; i++ )
      writechar ( s[ i ] );
}

/* OK */
writechar ( c )
char c;
{
   if ( linepos >= charsperline )
      return;
   outbuff[ linepos++ ] = c;
}

/* OK */
clearbits ( )
{
   int i;

   for ( i = 0 ; i <= ( tagtablesize - 1 ) ; i++ )
   {
      struct SYMNODE *ptr = tagtable[ i ];

      while ( ptr != NULL )
      {
	 if ( ptr->st_definition != 0 )
	    ptr->st_flags = ptr->st_flags & ~stb_setnow ;
	 ptr = ptr->st_link;
      }
   }
}

/* OK */
relocate ( address, size, wrt )
long address;
int size, wrt;
{
   if ( pass2 )
      switch ( size )
      {
      case 4:
	 reloc32info = heap4(reloc32info, address, sectnumber, wrt);
	 reloc_32count++;
	 updatecount( &relrp32, &absrp32);
	 break;

      case 2:
	 reloc16info = heap4(reloc16info, address, sectnumber, wrt);
	 reloc_16count++;
	 updatecount( &relrp16, &absrp16 );
         warning(195);
	 break;

      default:
	 complain(192);
	 break;
      }
}

/* OK */
updatecount( p_relrcount, p_relacount)
int *p_relrcount, *p_relacount;
{
   switch ( secttype )
   {
   case d_text:
   case d_data:
      (*p_relrcount)++;
      break;
   case d_abs:
      (*p_relacount)++;
      break;
   default:
      warning(186);
      break;
   }
}

/* OK */
generate ( masktype )
int masktype;
{
   switch ( masktype )
   {
   case 1:
      swapoperands (  );
      codeword ( instr_mask			 |
		 ((int)(op1_exp) << 9)		 |
		 (sizefield(instr_size)  << 6)	 |
		 eafield() );
      genea();
      break;

   case 2:
      codeword ( instr_mask			|
		 (sizefield(instr_size) << 6)	|
		 eafield() );
      if ( source_ea != 0 )
      {
	 swapoperands();
	 genea();
	 swapoperands();
      }

      genea();
      break;

   case 4:
      if ( (op1_ea == am_Ar) || (op1_ea == am_Dr) )
	 swapoperands();
      codeword(instr_mask | exp);
      if ( source_ea != 0 )
	 codeword ( (int)(op1_exp & 0xFFFF) );
      break;

   case 5:
      codeword (   instr_mask |
		   ((int)(op1_exp & 0x7) << 9)	 |
		   (sizefield(instr_size) << 6)  |
		   eafield() );
      genea();
      break;

   case 6:
      codeword ( instr_mask	  |
		 (source_ea << 8) |
		 eafield() );
      genea();
      break;

   case 7:
      swapoperands();
      codeword ( instr_mask	  |
		 (op1_exp << 9)   |
		 eafield()	);
      genea();
      break;

   case 9:
      codeword( instr_mask | eafield() );
      genea ();
      break;

   case 10:
      codeword ( instr_mask				|
		 ((instr_size == ts_long ? 1 : 0) << 6) |
		 (int)exp );
      break;

   case 15:
      codeword ( instr_mask );
      if ( dest_ea != 0 )
	 codeword ( (int)(exp & 0xFFFF) );
      break;

   default:
      complain(1011);
   }
}

/* OK */
int sizefield ( size )
int size;
{
   switch ( size )
   {
   case ts_long:
      return 0x2;
   case ts_word:
      return 0x1;
   case ts_byte:
      return 0x0;
   case ts_none:
      return sizefield(ts_default);
   case ts_short:
      complain ( 86 );
   default:
      complain ( 1012 );
   }
}

/* OK */
int eafield ( )
{
   switch ( op_ea )
   {
   case am_Dr:
      return 0x0 + (int)exp;
   case am_Ar:
      return 0x8 + (int)exp;
   case am_Ar_ind:
      return 0x10 + (int)exp;
   case am_Ar_pi:
      return 0x18 + (int)exp;
   case am_Ar_pd:
      return 0x20 + (int)exp;
   case am_abs16:
      return 0x38;
   case am_abs32:
      return 0x39;
   case am_PC_disp:
      return 0x3A;
   case am_PC_index:
      return 0x3B;
   default:
      return 0x3C;
   case am_Ar_disp:
      return 0x28 + (registers->uvalleft).intvalue;
   case am_Ar_index:
      return 0x30 + (((registers->uvalleft).eptr)->uvalleft).intvalue;
   }
}

/* OK */
genea ( )
{
   switch ( op_ea )
   {
   case am_Ar:
      if ( instr_size == ts_byte )
	 complain ( 29 );
   case am_Dr:
   case am_Ar_ind:
   case am_Ar_pi:
   case am_Ar_pd:
      return;
   case am_Ar_disp:
      if ( pass1 )
	 codeword(0);
      else
	 if ( registers->type == s_Ar )
	    if ( (registers->uvalright).value == ts_none )
	    {
	       if ( pass2 && extrnref )
		  addextrnref ( extrnlsymb, location + ( codewords
		      * 2 ), 2 );
	       codeword ( (int)exp );
	    }
	    else
	       complain ( 81 );
	 else
	    complain ( 134 );
      break;

   case am_Ar_index:
      {
	 struct EXPRESSNODE *Ar = (registers->uvalleft).eptr;
	 struct EXPRESSNODE *Ir = (registers->uvalright).eptr;
	 if (pass1)
	    codeword(0);
	 else
	 {
	    int l = (Ir->uvalright).value == ts_long ? 1 :
		    (Ir->uvalright).value == ts_word ? 0 :
		    (Ir->uvalright).value == ts_none ? 0 : complain(87);
	    int r = Ir->type == s_Ar ? 1 : 0;

	    if ( ! ((-128 <= exp) && (exp <= 127)) )
	       complain(72);

	    if ( (Ar->uvalright).value == ts_none )
	    {
	       if ( extrnref && pass2 )
		  addextrnref ( extrnlsymb,
				(location + (codewords* 2)) + 1, 1 );

	       codeword (  (r << 15)			     |
			   (((Ir->uvalleft).intvalue) << 12) |
			   (l << 11)			     |
			   (int)(exp & 0xFF)	);
	    }
	    else
	       complain(81);
	 }
      }
      break;

   case am_abs32:
      if ( extrnref && pass2 )
	 addextrnref ( extrnlsymb, location + (codewords*2), 4 );
      if ( relocation_needed && pass2 )
	 relocate ( location + (codewords*2), 4, reloc_wrt );
      codeword ( (int)(exp >> 16) );

   case am_abs16:
      codeword ( (int)(exp & 0xFFFF) );
      break;

   case am_PC_disp:
      if ( pass1 )
	 codeword(0);
      else
      {
	 long pc = location + ( codewords * 2 );
	 long o = exp - pc;

	 if ( extrnref )
	 {
	    o = exp;
	    addextrnref ( extrnlsymb, pc, 2 );
	 }
	 else
	    if ( ! ( ((locmode == s_abs) && absolute(exptype)) ||
		     ((locmode == s_rel) &&
			    (sectnumber == (exptype >> 8) & 0xFF))) )
	       complain ( 88 );
	 if ( ! ((-32768 <= o) && (o <= 32767)) )
	    complain(177);
	 codeword( (int)(o & 0xFFFF) );
      }
      break;

   case am_PC_index:
      if ( pass1 )
	 codeword(0);
      else
      {
	 struct EXPRESSNODE *Ir = registers;
	 int l = (Ir->uvalright).value == ts_long ? 1 :
		 (Ir->uvalright).value == ts_word ? 0 :
		 (Ir->uvalright).value == ts_none ? 0 : complain(87);
	 int r = Ir->type == s_Ar ? 1 : 0;

	 if ( extrnref )
	    addextrnref ( extrnlsymb, (location + (codewords*2)) + 1, 1 );
	 else
	 {
	    exp = exp - (location + 2);
	    if ( ! ( ((locmode == s_abs) && absolute(exptype)) ||
		     ((locmode == s_rel) &&
			 (sectnumber == (exptype >> 8) & 0xFF))))
	       complain ( 88 );
	 }
	 if ( ! ((-128 <= exp) && (exp <= 127)) )
	    complain(72);
	 codeword(   (r << 15)			      |
		     ((Ir->uvalleft).intvalue << 12 ) |
		     (l << 11)			      |
		     (int)(exp & 0xFF)	);
      }
      break;

   default:
      {
	 int bs = bytesize(instr_size);
	 if (extrnref && pass2)
	    addextrnref( extrnlsymb, location+(codewords*2), bs );
	 if (relocation_needed && pass2)
	    relocate ( location + (codewords*2), bs, reloc_wrt );
	 if ( bs == 4 )
	    codeword ( (int)(exp >> 16) );
	 if ( bs == 1 )
	    codeword ( (int)(exp & 0xFF) );
	 else
	    codeword ( (int)(exp & 0xFFFF) );
      }
      break;
   }
}

/* OK */
addextrnref ( symbol, address, size )
struct SYMNODE *symbol;
long address;
int size;
{
   if ( pass2 )
   {
      int flag = no;
      struct XREFNODE *s;
      int e_refsxx;
      int e_countxx;

   startlabel:
      s = extrnsymbols;

      while ( s != NULL )
	 if ( s->e_symbol == symbol )
	 {
	    switch ( size )
	    {
	    case 4:
	       (s->e_count32)++;
	       s->e_refs32 = heap4( s->e_refs32, address, sectnumber, 0 );
	       break;
	    case 2:
	       (s->e_count16)++;
	       s->e_refs16 = heap4( s->e_refs16, address, sectnumber, 0 );
	       break;
	    case 1:
	       (s->e_count8)++;
	       s->e_refs8 = heap4( s->e_refs8, address, sectnumber, 0 );
	       break;
	    }
	    return;
	 }
	 else
	    s = s->e_link;

      if ( flag == no )
      {
	 flag = yes;
	 symbol = (symbol->uval).st_xref;
	 goto startlabel;
      }

      complain ( 1013 );
   }
}

/* OK */
codeword ( w )
int w;
{
   codewords++;
   stackvalue( s_abs16, 2, (long)w, no, NULL );
}

/* OK */
stackvalue ( a, b, c, eref, esymb )
int a, b, eref;
long c;
struct SYMNODE *esymb;
{
   int offset = nitems*(sizeof(struct SVNODE)/bytesperword);

   struct SVNODE *cb = (struct SVNODE *)(codebuff + offset);

   cb->type    = a;
   cb->size    = b;
   cb->value   = c;
   cb->extref  = eref;
   cb->extsymb = esymb;

   if ( ++nitems > codesize )
   {
      selectoutput(sysout);
      writes ( "\040***Error:  Code Buffer Overflow\n" );
      abortassembly();
   }
}

/* OK */
clearbuffer ( )
{
   int i; /* FOR loop variables */
   for ( i = 0 ; i <= ( maxllen - 1 ) ; i++ )
      outbuff[ i ] = ' ';
}

/* OK */
swapoperands ( )
{
   int t1		  = op_ea;
   int t2		  = exptype;
   long t3		  = exp;
   struct EXPRESSNODE *t4 = registers;
   int t5		  = extrnref;
   struct SYMNODE *t6	  = extrnlsymb;
   int t7		  = relocation_needed;
   int t8		  = reloc_wrt;

   op_ea		  = op1_ea;
   exptype		  = op1exptype;
   exp			  = op1_exp;
   registers		  = op1_registers;
   extrnref		  = op1_extrnref;
   extrnlsymb		  = op1extrnlsymb;
   relocation_needed	  = op1_relocation_needed;
   reloc_wrt		  = op1reloc_wrt;

   op1_ea		  = t1;
   op1exptype		  = t2;
   op1_exp		  = t3;
   op1_registers	  = t4;
   op1_extrnref 	  = t5;
   op1extrnlsymb	  = t6;
   op1_relocation_needed  = t7;
   op1reloc_wrt 	  = t8;
}

/* OK */
setloc ( newloc )
long newloc;
{
   if ( newloc > maxloc )
      maxloc = newloc;
   if ( newloc < minloc )
      minloc = newloc;
   if ( ! ((newloc & 0xFF000000) == 0) )
      if ( ! error_found )
      {
	 ended = yes;
	 aborted = yes;
	 complain ( 138 );
      }
   location = newloc;
}

/* OK */
changesection ( flag, ptr, type )
int flag, type;
struct SECTNODE  *ptr;
{
   int newsectnumber =	-1 ;
   int count = 0;

   if ( flag == no )
   {
      string name = (string)ptr;
      struct SECTNODE *p;
      ptr = (struct SECTNODE *)(& sectionlist) ;
      p = ptr->sect_link;

      while ( p != NULL )
      {
	 string sname = p->sect_name;
	 int	stype = p->sect_type;
	 if ( (type == stype) && ( compstring(name, sname) == 0 ) )
	    break;
	 count++;
	 ptr = p;
	 p = ptr->sect_link;
      }

      if ( p == NULL )
      {
	 int i;
	 string temp;

#ifdef DBUG
	 mydebug("Changesection","location = %N; no_labels = %N",
			location, (long)no_labels );
#endif
	 /* Don't create a new section table unless we really have
	    to. Use the last one if that section is "empty" */

	 if ( !( no_labels && (location == 0) &&
		 (sectptr != (struct SECTNODE *)-1) ) )
	 {
	   if ( maxsectnumber == 255 )
	      complain ( 185 );

#ifdef DBUG
	   mydebug( "Changesection","New Section: ln = %N", (long)sourcelinenumber );
#endif
	   ptr->sect_link      = (struct SECTNODE *)newvec(
				      (sizeof(struct SECTNODE)-1)/BYTESPERWORD );
	   ptr = ptr->sect_link;
	   ptr->sect_link     = NULL;
	   ptr->sect_min      = MAXINT;
	   ptr->sect_max      = MININT;
	   ptr->sect_loc      = 0;
	   ptr->sect_codevec  = 0;
	   ptr->sect_number   = count;

	   maxsectnumber = count;
	 }

	 ptr->sect_type = type;

         /* name length could be zero, so beware when 'newvec'ing */
	 temp = (string)newvec( name[0]/BYTESPERWORD ) ;
	 for ( i = 0 ; i <= name[ 0 ] ; i++ )
	    temp[ i ] = name[ i ];
	 ptr->sect_name = temp;
      }
      else
	 ptr = p;
   }

   newsectnumber = ptr->sect_number;
   if ( sectptr != (struct SECTNODE *)-1 )
   {
      sectptr->sect_min = minloc;
      sectptr->sect_max = maxloc;
      sectptr->sect_loc = location;
   }

   minloc   = ptr->sect_min;
   maxloc   = ptr->sect_max;
   location = ptr->sect_loc;
   codevec  = ptr->sect_codevec;
   sectptr  = ptr;
   secttype = ptr->sect_type;
   sectnumber = ptr->sect_number;
   locmode = (secttype == d_text) ||
	     (secttype == d_data) ||
	     (secttype == d_bss) ? s_rel : s_abs;
}

