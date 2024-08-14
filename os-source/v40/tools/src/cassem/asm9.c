
/* ****************************************************************************** */
/* *   Assembler for the Motorola MC68000 Microprocessor:  Section 9		* */
/* *	      (c) Copyright 1985, Tenchstar Ltd., Bristol, UK.			* */
/* *										* */
/* *		    Last Modified :  11-APR-85	   (PJF)			* */
/* ****************************************************************************** */

/* ****************************************************************************** */
/*	   This section contains all the output module formats			* */
/* ****************************************************************************** */

/* M68KASM9 : Automatically translated from BCPL to C on 26-Feb-85 */


#include  "libhdr"
#include  "asmhdr"



/******************************************/
/*  External functions returning a value  */
/******************************************/

extern int absolute();			  /*  asm6  */
extern int relocatable();		  /*  asm6  */
extern int wordsized(); 		  /*  asm6  */


/*******************************************/
/*  Local procedures returning a value	   */
/*******************************************/

int countrelocs();
int amigaxdefs();
int amigaxrefs();
int amigasymboldump();


#ifndef AMI
coagulatesections ( p_erel32, p_eabs32, p_erel16, p_erel8 )
word *p_erel32, *p_eabs32, *p_erel16, *p_erel8;
{
   updaterelocinfo ( reloc32info, 4 );
   updaterelocinfo ( reloc16info, 2 );
   updatexrefs ( p_erel32, p_eabs32, p_erel16, p_erel8 );
   updaterelsymbols ();
}

updaterelocinfo ( ptr, size )
word *ptr, size;
{
   while ( ! ( ptr == NULL ) )
   {
      word snum = ptr[ rel_snum ];
      word wrt	= ptr[ rel_wrt ];
      word type = typevec[ snum ];
      word cvec = cvecs[ snum ];
      word offset = ptr[ rel_address ];

      pbytes ( cvec+offset, size, bavec[wrt] + gbytes(cvec + offset, size) );

      ptr[ rel_address ] = offset + bavec[ snum ];
      ptr = (word*)ptr[ rel_link ];
   }
}

updatexrefs ( p_erel32, p_eabs32, p_erel16, p_erel8 )
word *p_erel32, *p_eabs32, *p_erel16, *p_erel8;
{
   word *ptr = extrnsymbols;

   while ( ! ( ptr == NULL ) )
   {
      word count8  = ptr[ e_count8 ];
      word count16 = ptr[ e_count16 ];
      word count32 = ptr[ e_count32 ];

      (*p_erel8)  =  (*p_erel8) + modifyxrefs ( count8, ptr[ e_refs8 ] );
      (*p_erel16) = (*p_erel16) + modifyxrefs ( count16, ptr[ e_refs16 ] );
      (*p_erel32) = (*p_erel32) + modifyxrefs(count32, ptr[ e_refs32 ]);
      (*p_eabs32) = (*p_eabs32) + result2;

      ptr = (word*)ptr[ e_link ];
   }
}

word modifyxrefs ( count, ptr )
word count, *ptr;
{
   word relcount = 0;
   word abscount = 0;

   if ( count == 0 )
      goto label;

   while ( ! ( ptr == NULL ) )
   {
      word address = ptr[ 1 ];
      word snum    = ptr[ 2 ];
      word type    = typevec[ snum ];

      switch ( type )
      {
	case d_text:
	case d_data:
	case d_bss:
	   relcount = relcount + 1;
	   ptr[ 1 ] = address + bavec[ snum ];
	   break;
	case d_abs:
	   abscount = abscount + 1;
	   break;
      }

      ptr = (word*)ptr[ 0 ];
   }

label:
   result2 = abscount;
   return relcount;
}

updaterelsymbols ( )
{
   int i;

   for ( i = 0 ; i <= ( tagtablesize - 1 ) ; i++ )
   {
      word *tag = (word*)tagtable[ i ];

      while ( ! ( tag == NULL ) )
      {
	 word type = tag[ st_type ] & st_type_mask;

	 if ( type == s_rel )
	 {
	    word snum  = tag[ st_snumber ];
	    tag[ st_value ] += bavec[ snum ];
	 }

	 tag = (word*)tag[ st_link ];
      }
   }
}

word countextrnsymbols ( offset )
word offset;
{
   word *ptr = extrnsymbols;
   word count = 0;

   while ( ! ( ptr == NULL ) )
   {
      count = count + ptr[ offset ];
      ptr   = (word*)ptr[ e_link ];
   }

   return count;
}

triposmodule ( )
{
   word o      = output();
   word *ptr   = extrnsymbols;
   word erel32 = 0;
   word erel16 = 0;
   word erel8  = 0;
   word eabs32 = 0;

   coagulatesections( &erel32, &eabs32, &erel16, &erel8 );

   if ( (erel16 != 0) || (erel8 != 0) )
   {
      selectoutput ( sysout );
      writes ( "\077TRIPOS module cannot deal with 8 or 16 bit external references\n" );
      selectoutput ( o );
      return;
   }

   selectoutput ( codestream );

   triposhunk ( t_hunk, relcodewords, 167, yes, no );
   triposrelocinfo ( relrp16, relrp32, 168, 169, yes, no );

   if ( ! ( ( entrysymbols == 0 ) && ( erel32 == 0 ) ) )
   {
      systemword ( t_ext );
      if ( ! ( entrysymbols == NULL ) )
	 triposxdefs ();
      if ( ! ( erel32 == 0 ) )
	 triposxrefs ( yes, no );
      systemword ( 0 );
   }

   if ( dumpsymbols )
      dumpsymboltable ( relocatable );

   triposhunk ( t_abshunk, abscodewords, 170, no, yes );
   triposrelocinfo ( absrp16, absrp32, 171, 172, no, yes );

   if ( ! ( eabs32 == 0 ) )
   {
      systemword ( t_ext );
      triposxrefs ( no, yes );
      systemword ( 0 );
   }

   if ( dumpsymbols )
      dumpsymboltable ( absolute );

   systemword ( t_end );

   selectoutput ( o );
}

triposhunk ( sword, csize, messno, relflag, absflag )
word sword, csize, messno, relflag, absflag;
{
   if ( ! ( csize == 0 ) )
   {
      word r	= csize;
      word *ptr = sectionlist;

#ifdef TRIPOS
      if ( toobig(r) )
	 error ( messno );
#endif

      systemword ( sword );
      if ( relflag ) writeword( r );

      while ( ! ( ptr == NULL ) )
      {
	 word type = ptr[ sect_type ];
	 word r    = ptr[ sect_max ] / bytesper68000word;
	 word *v   = (word*)ptr[ sect_codevec ];

	 switch ( type )
	 {
	   case d_text:
	   case d_data:
	      if ( relflag ) writewords( v, r );
	      break;
	   case d_bss:
	      if ( relflag )
	      {
		 word i;
		 for ( i = 1 ; i <= r ; i++ )
		    writeword ( 0 );
	      }
	      break;
	   case d_abs:
	      if ( absflag )
	      {
		 word am = ptr[ sect_min ] / bytesper68000word;
		 writeword ( am );
		 writeword ( abscodewords );
		 writewords ( v + am, abscodewords );
	      }
	   default:
	      break;
	 }
	 ptr = (word*)ptr[ sect_link ];
      }
   }
}

triposrelocinfo ( count16, count32, msg1, msg2, relflag, absflag )
word count16, count32, msg1, msg2, relflag, absflag;
{
   triposrelocblock ( count16, t_reloc16, msg1, reloc16info, relflag, absflag );
   triposrelocblock ( count32, t_reloc32, msg2, reloc32info, relflag, absflag );
}

triposrelocblock ( count, sysword, errorcode, ptr, relflag, absflag )
word count, sysword, errorcode, *ptr, relflag, absflag;
{
   if ( count == 0 ) return;

#ifdef TRIPOS
   if ( toobig(count) ) error( errorcode );
#endif

   systemword ( sysword );
   writeword ( count );

   while ( ! ( ptr == NULL ) )
   {
      word snum = ptr[ rel_snum ];
      word type = typevec[ snum ];

      switch ( type )
      {
	case d_text:
	case d_data:
	case d_bss:
	   if ( relflag )
	      writeword ( ptr[ rel_address ] );
	   break;
	case d_abs:
	   if ( absflag )
	      writeword ( ptr[ rel_address ] );
	   break;
	default:
	   complain ( 1014 );
      }

      ptr = (word*)ptr[ rel_link ];
   }
}

triposxdefs ( )
{
   word *ptr = entrysymbols;

   while ( ! ( ptr == NULL ) )
   {
      word i;
      word *symbol = (word*)ptr[ e_symbol ];
      word type    = symbol[ st_type ] & st_type_mask;
      word value   = symbol[ st_value ];
      string name  = (string)(symbol + st_name);
      word l	   = name[ 0 ];
      word length  = l > maxextlength ? maxextlength : l;
      word size    = maxextlength == 7 ? 2 : 4;

      unsigned char buff[16];

      buff[ 0 ] = relocatable(type) ? ext_defrel : ext_defabs;

      for ( i = 1 ; i <= length ; i++ )
	 buff[ i ] = name[ i ];

      for ( i = length + 1 ; i <= maxextlength ; i++ )
	 buff[ i ] = ' ';

      writewords ( buff, size );
      writeword ( value );

      ptr = (word*)ptr[ e_link ];
   }
}

triposxrefs ( relflag, absflag )
word relflag, absflag;
{
   word *ptr = extrnsymbols;

   while ( ! ( ptr == NULL ) )
   {
      word *symbol = (word*)ptr[ e_symbol ];
      word *refs   = (word*)ptr[ e_refs32 ];
      word count = countxrefs(refs, relflag, absflag);
      string name = (string)(symbol + st_name);
      word l = name[ 0 ];
      word length = l > maxextlength ? maxextlength : l;
      word size = maxextlength == 7 ? 2 : 4;
      unsigned char buff[16];

      if ( ! ( count == 0 ) )
      {
	 word i;

	 buff[ 0 ] = ext_ref;

	 for ( i = 1 ; i <= length ; i++ )
	    buff[ i ] = name[ i ];

	 for ( i = length + 1 ; i <= maxextlength ; i++ )
	    buff[ i ] = ' ';

	 writewords ( buff, size );
	 writeword ( count );

	 while ( ! ( refs == NULL ) )
	 {
	    word snum = refs[ 2 ];
	    word type = typevec[ snum ];

	    switch ( type )
	    {
	      case d_text:
	      case d_data:
	      case d_bss:
		 if ( relflag )
		    writeword ( refs[ 1 ] );
		 break;
	      case d_abs:
		 if ( absflag )
		    writeword ( refs[ 1 ] );
		 break;
	    }

	    refs = (word*)refs[ 0 ];
	 }
      }

      ptr = (word*)ptr[ e_link ];
   }
}


#ifdef TRIPOS
word toobig ( v )
word v;
{
    return v > ( 32767 / BYTESPERWORD );
}
#endif


word countxrefs ( refs, relflag, absflag )
word *refs, relflag, absflag;
{
   word count = 0;

   while ( ! ( refs == NULL ) )
   {
      word snum = refs[ 2 ];
      word type = typevec[ snum ];

      switch ( type )
      {
	case d_text:
	case d_data:
	case d_bss:
	   if ( relflag )
	      count = count + 1;
	   break;
	case d_abs:
	   if ( absflag )
	      count = count + 1;
	   break;
      }

      refs = (word*)refs[ 0 ];
   }

   return count;
}

dumpsymboltable ( criterion )
word (*criterion)();
{
   if ( symbolspresent(criterion) )
   {
      word i; /* FOR loop variables */
      systemword ( t_symbols );

      for ( i = 0 ; i <= ( tagtablesize - 1 ) ; i++ )
      {
	 word *tag = (word*)tagtable[ i ];

	 while ( ! ( tag == NULL ) )
	 {
	    word type = tag[ st_type ] & st_type_mask;

	    if ( criterion(type) )
	    {
	       word i, j; /* FOR loop variables */
	       string name = (string)(tag + st_name);
	       word value = tag[ st_value ];
	       word length = name[ 0 ];
	       word nwords = ( ( length - 1 ) / bytesper68000word ) + 1;
	       word buffer[tagsize];

	       for ( i = 0 ; i <= ( tagsize - 1 ) ; i++ )
		  buffer[ i ] = 0;

	       writeword ( length );

	       for ( j = 1 ; j <= length ; j++ )
		  ((string)buffer)[j - 1] = name[ j ];

	       writewords ( buffer, nwords );
	       writeword ( value );
	    }

	    tag = (word*)tag[ st_link ];
	 }
      }

      systemword ( 0 );
   }
}


word symbolspresent ( criterion )
word (*criterion)();
{
   word i; /* FOR loop variables */

   for ( i = 0 ; i <= ( tagtablesize - 1 ) ; i++ )
   {
      word *tag = (word*)tagtable[ i ];

      while ( ! ( tag == NULL ) )
      {
	 word type = tag[ st_type ] & st_type_mask;

	 if ( criterion(type) )
	    return true;

	 tag = (word*)tag[ st_link ];
      }
   }
   return false;
}
#endif


#ifdef TRIPOS
systemword ( word )
word word;
{
   writeword ( word );
}

writeword ( word )
word word;
{
   writewords (  & word , 1 );
}

writewordvec ( wordvec, words )
word wordvec, words;
{
   writewords ( wordvec, words );
}
#endif


#ifdef U42
systemword ( word )
word word;
{
   writeword ( word );
}

writeword ( word )
word word;
{
   wbytes (  & word , bytesper68000word );
}

writewordvec ( wordvec, words )
word wordvec, words;
{
   writewords ( wordvec, words );
}
#endif


#ifdef AMI
systemword ( w )
int w;
{
#ifdef DBUG
   mydebug( "systemword", "w = %X8", (long)w );
#endif
   writeword ( (long)w );
}

#ifdef SUN
writeword ( w )
long w;
{
   wbytes (  & w , bytesper68000word );
}
#endif

#ifdef IBM
/* longs are stored backwards on the IBM so we must
   invert the number before writing it out */
writeword ( w )
long w;
{
   char buffer[4];
   int i;
   int result;

   for (i=3; i >= 0; i-- )
     buffer[3-i] = ((char *)(&w))[i];

   result = wbytes ( buffer, bytesper68000word );

#ifdef DBUG
   mydebug( "writeword","w = %X8; result = %n", w, (long)result);
#endif
}
#endif

writewordvec ( wordvec, n )
long *wordvec;
int n;
{
   writewords ( wordvec, n );
}
#endif

#ifndef AMI
motorolamodule ( )
{
   word o = output();
   word absmin = 0;
   word absmax = 0;
   word *absvec = 0;
   word *ptr = sectionlist;

   if ( ! ( relcodewords == 0 ) )
   {
      selectoutput ( sysout );
      writes ( "\057MOTOROLA module cannot handle Relocatable code\n" );
      selectoutput ( o );
      return;
   }

   if ( ! ( ( extrnsymbols == NULL ) && ( entrysymbols == NULL ) ) )
   {
      selectoutput ( sysout );
      writes ( "\057MOTOROLA module cannot handle External Symbols\n" );
      selectoutput ( o );
      return;
   }

   if ( dumpsymbols )
   {
      selectoutput ( sysout );
      writes ( "\055MOTOROLA module cannot handle SYMBOL dumping\n" );
      selectoutput ( o );
      return;
   }

   if ( ! ( ( reloc_16count == 0 ) && ( reloc_32count == 0 ) ) )
   {
      selectoutput ( sysout );
      writes ( "\070MOTOROLA module cannot deal with Relocation within code\n"
	  );
      selectoutput ( o );
      return;
   }

   selectoutput ( codestream );

   while ( ! ( ptr == NULL ) )
   {
      if ( ptr[ sect_type ] == d_abs )
	 break;
      ptr = (word*)ptr[ sect_link ];
   }

   if ( ptr == NULL )
   {
      selectoutput ( sysout );
      writes ( "\045No absolute code for MOTOROLA module\n" );
      selectoutput ( o );
      return;
   }

   absmin = ptr[ sect_min ];
   absmax = ptr[ sect_max ];
   absvec = (word*)ptr[ sect_codevec ];

   {
      word addr;
      word cs = 0;

      for ( addr = absmin ; addr <= ( absmax - 1 ) ; addr += 32 )
      {
	 word i; /* FOR loop variables */
	 word left = absmax - addr;
	 word nbytes = left > 32 ? 32 : left;
	 word length = 4 + nbytes;
	 cs = ( ( length + ( addr & 0xFF ) ) + ( ( addr >> 8 ) & 0xFF )
	     ) + ( ( addr >> 16 ) & 0xFF );
	 writef ( "\005S2%X2", length );
	 writehex ( addr, 6 );
	 for ( i = addr ; i <= ( ( addr + nbytes ) - 1 ) ; i++ )
	 {
	    word byte = absvec[ i ];
	    cs = cs + byte;
	    writehex ( byte, 2 );
	 }
	 writef ( "\004%X2\n",  ~ cs  );
      }
      cs = ( ( length + ( absmin & 0xFF ) ) + ( ( absmin >> 8 ) & 0xFF )
	  ) + ( ( absmin >> 16 ) & 0xFF );
      writef ( "\013S804%X6%X2\n", absmin,  ~ cs  );
   }
   selectoutput ( o );
}


intelhexmodule ( )
{
   word o = output();
   word ptr;
   word absmin = 0;
   word absmax = 0;
   word absvec = 0;
   word erel32 = 0;
   word erel16 = 0;
   word erel8  = 0;
   word eabs32 = 0;

   if ( ! ( ( relcodewords > 0 ) && ( abscodewords > 0 ) ) )
   {
      selectoutput ( sysout );
      writes ( "\106INTEL HEX module cannot deal with mixed Absolute and Relocatable code\n"
	  );
      selectoutput ( o );
      return;
   }

   if ( ! ( reloc_32count == 0 ) )
   {
      selectoutput ( sysout );
      writes ( "\064INTEL HEX module cannot deal with 32-bit relocation\n"
	  );
      selectoutput ( o );
      return;
   }

   if ( ! ( ( extrnsymbols == 0 ) && ( entrysymbols == 0 ) ) )
   {
      selectoutput ( sysout );
      writes ( "\060INTEL HEX module cannot handle External symbols\n" );
      selectoutput ( o );
      return;
   }

   if ( dumpsymbols )
   {
      selectoutput ( sysout );
      writes ( "\056INTEL HEX module cannot handle SYMBOL dumping\n" );
      selectoutput ( o );
      return;
   }

   if ( abscodewords > 0 )
   {
      ptr = sectionlist;
      while ( ! ( ptr == 0 ) )
      {
	 if ( ptr[ sect_type ] == d_abs )
	    break;
	 ptr = ptr[ sect_link ];
      }
      if ( ptr == 0 )
	 complain ( 0 );
      absmin = ptr[ sect_min ];
      absmax = ptr[ sect_max ];
      absvec = ptr[ sect_codevec ];
   }
   else
      coagulatesections ( &erel32, &eabs32, &erel16, &erel8  );
   selectoutput ( codestream );
   writes ( absm ? "\016$      0500FE\n" : "\016$      0501FD\n" );
   while ( ! ( ptr == 0 ) )
      intelcodeblock ( ptr );
   intelrelocblock ( ptr );
   selectoutput ( o );
}

intelcodeblock ( ptr )
word ptr;
{
   word addr; /* FOR loop variables */
   word base = p;
   word size = absm ? absmax - absmin : relcodewords;
   word top = base + size;
   word bvec = absm ? absvec : relvec;
   for ( addr = base ; addr <= ( top - 1 ) ; addr += 32 )
   {
      word i; /* FOR loop variables */
      word left = top - addr;
      word nbytes = left > 32 ? 32 : left;
      word lbyte = addr & 0xFF;
      word hbyte = ( addr >> 8 ) & 0xFF;
      word cs = ( nbytes + lbyte ) + hbyte;
      if ( ! wordsized(addr) )
      {
	 selectoutput ( sysout );
	 writes ( "\060INTEL HEX module cannot handle 24-bit addresses\n"
	     );
	 selectoutput ( o );
	 return;
      }
      writef ( "\014:%X2%X2%X200", nbytes, hbyte, lbyte );
      for ( i = addr ; i <= ( ( addr + nbytes ) - 1 ) ; i++ )
      {
	 word byte = bvec[ i ];
	 cs = cs + byte;
	 writehex ( byte, 2 );
      }
      writef ( "\004%X2\n",  - cs  );
   }
}

intelrelocblock ( absm )
word absm;
{
   word i; /* FOR loop variables */
   word rvec = absm ? absrvec16 : relrvec16;
   word rvecp = absm ? absrp16 : relrp16;
   for ( i = 0 ; i <= ( rvecp - 1 ) ; i += 16 )
   {
      word j; /* FOR loop variables */
      word nwords = rvecp - i;
      word nbytes = ( nwords > 16 ? 16 : nwords ) * 2;
      word cs = nbytes + 4;
      writef ( "\012$%X2000004", nbytes );
      for ( j = 0 ; j <= ( ( nbytes / 2 ) - 1 ) ; j++ )
      {
	 word reladdr = rvec[ 1 ];
	 word wrt = rvec[ 2 ];
	 word lbyte = reladdr & 0xFF;
	 word hbyte = ( reladdr >> 8 ) & 0xFF;
	 if ( ! wordsized(reladdr) )
	 {
	    selectoutput ( sysout );
	    writes ( "\072INTEL HEX module cannot handle 24-bit relocation addresses"
		);
	    selectoutput ( o );
	    return;
	 }
	 if ( ( wrt == s_data ) || ( wrt == s_bss ) )
	 {
	    selectoutput ( sysout );
	    writes ( "\057INTEL HEX module cannot deal with DATA and BSS\n"
		);
	    selectoutput ( o );
	    return;
	 }
	 writehex ( hbyte, 2 );
	 writehex ( lbyte, 2 );
	 cs = ( cs + hbyte ) + lbyte;
	 rvec = rvec[ 0 ];
      }
      writef ( "\004%X2\n",  - cs  );
   }
   writes ( "\014:00000001FF\n" );
}
#endif

#ifndef AMI
unix4_2module ( )
{
   word oldout = output();
   word erel32 = 0;
   word erel16 = 0;
   word erel8  = 0;
   word eabs32 = 0;
   word offset = 4;

   coagulatesections ( &erel32, &eabs32, &erel16, &erel8 );

   if ( ! ( (abscodewords == 0) &&
	    (eabs32 == 0) &&
	    (absrp16 == 0) &&
	    (absrp32 == 0)    )  )
   {
      selectoutput ( sysout );
      writes ( "\050UNIX module cannot handle Absolute code\n" );
      selectoutput ( oldout );
      return;
   }

   selectoutput ( codestream );

   unixheader ( erel32+erel16+erel8 );

   unixcodehunk (  );

   unixrelocinfo ( reloc16info, r_length16 );
   unixrelocinfo ( reloc32info, r_length32 );

   unixextrelocinfo ( e_refs32, e_count32, r_length32 );
   unixextrelocinfo ( e_refs16, e_count16, r_length16 );
   unixextrelocinfo ( e_refs8, e_count8, r_length8 );

   offset = unixsymboltable(offset);
   unixnametable ( offset );

   selectoutput ( oldout );
}

unixcodehunk ( )
{
   word *ptr = sectionlist;
   while ( ! ( ptr == 0 ) )
   {
      word type = ptr[ sect_type ];
      word words = ptr[ sect_max ] / bytesper68000word;
      word v = ptr[ sect_codevec ];
      if ( ( type == d_text ) || ( type == d_data ) )
	 writewords ( v, words );
      ptr = (word*)ptr[ sect_link ];
   }
}

unixheader ( ecount )
word ecount;
{
   word headervec [ 8 ];
   word symtabsize = getsymbolnumber( - 1 ) * bytes_per_symbol;
   word rtextsize = ( relrp16+relrp32+ecount ) * bytes_per_reloc_symbol;

   headervec[ 0 ] = omagic;
   headervec[ 1 ] = relcodewords * bytesper68000word;
   headervec[ 2 ] = 0;
   headervec[ 3 ] = bsswords * bytesper68000word;
   headervec[ 4 ] = symtabsize;
   headervec[ 5 ] = 0;
   headervec[ 6 ] = rtextsize;
   headervec[ 7 ] = 0;

   wbytes ( headervec, 8 * bytesper68000word );
}

unixrelocinfo ( ptr, r_length )
word *ptr, r_length;
{
   while ( ! ( ptr == 0 ) )
   {
      word info;
      word wrt = ptr[ 2 ];
      word wrttype = typevec[ wrt ];
      wrt = wrttype == d_text ? n_text : wrttype == d_data ? n_data : wrttype
	  == d_bss ? n_bss : complain(1015);
      info = ( wrt << 8 ) + r_length;
      wbytes ( ptr + 1, 4 );
      wbytes (	& info , 4 );
      ptr = (word*)ptr[ 0 ];
   }
}

unixextrelocinfo ( e_refsx, e_countx, r_length )
word e_refsx, e_countx, r_length;
{
   word *ptr = extrnsymbols;
   while ( ! ( ptr == 0 ) )
   {
      word *symbol = (word*)ptr[ e_symbol ];
      word *refs = (word*)ptr[ e_refsx ];
      word count = ptr[ e_countx ];
      string name = (string)(symbol + st_name);
      word idnumber = getsymbolnumber(name);
      word info = ( ( idnumber << 8 ) + r_length ) + r_extern;
      if ( ! ( count == 0 ) )
      {
	 word i; /* FOR loop variables */
	 for ( i = 1 ; i <= count ; i++ )
	 {
	    wbytes ( refs + 1, 4 );
	    wbytes (  & info , 4 );
	    refs = (word*)refs[ 0 ];
	 }
      }
      ptr = (word*)ptr[ 0 ];
   }
}

word unixsymboltable ( offset )
word offset;
{
   word i; /* FOR loop variables */
   word endstring = 0;
   for ( i = 0 ; i <= ( tagtablesize - 1 ) ; i++ )
   {
      word *t = (word*)tagtable[ i ];
      while ( ! ( t == 0 ) )
      {
	 word type = t[ st_type ];
	 word snum = type >> 8;
	 string name = (string)(t + st_name);
	 word info;
	 word name_type;

	 type = type & st_type_mask;

	 switch ( type )
	 {
	   case s_rel:
	    type = typevec[ snum ];
	    name_type = (type == d_text) ? n_text :
			(type == d_data) ? n_data : n_bss;
	    break;
	   case s_ext:
	    name_type = n_ext;
	    break;
	   case s_abs16:
	   case s_abs32:
	    name_type = n_abs;
	    break;
	   default:
	    goto endloop;
	 }

	 if ( ( type != s_ext ) )
	    if ( xdefsymbol(name) )
	       name_type = name_type + n_ext;

	 info = ( ( name_type << 24 ) + ( n_other << 16 ) ) + n_desc;
	 wbytes (  & offset , 4 );
	 wbytes (  & info , 4 );
	 wbytes ( t + st_value, 4 );
	 offset = ( offset + name[ 0 ] ) + 1;

      endloop:
	 t = (word*)t[ st_link ];
      }
   }
   return offset;
}

unixnametable ( offset )
word offset;
{
   word i; /* FOR loop variables */
   word zero = 0;

   wbytes (  & offset , 4 );
   for ( i = 0 ; i <= ( tagtablesize - 1 ) ; i++ )
   {
      word *t = (word*)tagtable[ i ];
      while ( ! ( t == 0 ) )
      {
	 word type = t[ st_type ] & st_type_mask;
	 if ( (type == s_rel) ||
	      (type == s_ext) ||
	      (absolute(type))	)
	 {
	    word i; /* FOR loop variables */
	    string name = (string)(t + st_name);
	    word length = name[ 0 ];
	    string buff = tagv;
	    for ( i = 1 ; i <= length ; i++ )
	       buff[ i - 1 ] = name[ i ];
	    buff[ length ] = 0;
	    wbytes ( buff, length + 1 );
	 }

	 t = (word*)t[ st_link ];
      }
   }
   wbytes (  & zero , 1 );
}

word getsymbolnumber ( name )
string name;
{
   word i; /* FOR loop variables */
   word number = 0;

   for ( i = 0 ; i <= ( tagtablesize - 1 ) ; i++ )
   {
      word *t = (word*)tagtable[ i ];

      while ( ! ( t == NULL ) )
      {
	 word type = t[ st_type ] & st_type_mask;

	 if ( (type == s_rel)	||
	      (type == s_ext)	||
	      (absolute(type))	 )
	 {
	    if ( name == ( (string)(t + st_name) ) )
	       return number;
	    number = number + 1;
	 }

	 t = (word*)t[ st_link ];
      }
   }
   return number;
}

word xdefsymbol ( name )
string name;
{
   word *ptr = entrysymbols;
   while ( ! ( ptr == 0 ) )
   {
      if ( (string)(ptr[ e_symbol ] + st_name) == name )
	 return true;
      ptr = (word*)ptr[ e_link ];
   }
   return false;
}
#endif

#ifdef TRIPOS
wbytes ( ptr, n )
word ptr, n;
{
   word i; /* FOR loop variables */
   for ( i = 0 ; i <= ( n - 1 ) ; i++ )
      wrch ( ptr[ i ] );
}
#endif

/* OK */
amigamodule ( )
{
   SCBPTR o = output();
   struct SECTNODE *ptr = sectionlist;

   selectoutput(codestream);
   amigahunkname(PUname, hunk_unit);

   while ( ptr != NULL )
   {
      int type	= ptr->sect_type;
      int snum	= ptr->sect_number;
      string  s = ptr->sect_name;

      if ( type == d_abs )
      {
	 selectoutput ( sysout );
	 writes ( "\051AMIGA module cannot handle Absolute code\n" );
	 selectoutput ( o );
	 return;
      }

      amigahunkname ( ptr->sect_name, hunk_name );

      amigacodemodule ( ptr->sect_max, ptr->sect_codevec, type );

      amigarelocinfo ( snum );

      if ( (entrysymbols != NULL) || (extrnsymbols != NULL) )
      {
	 int flag = no;
	 flag = amigaxdefs(snum, yes);
	 flag = amigaxrefs(snum, 32, flag);
	 flag = amigaxrefs(snum, 16, flag);
	 flag = amigaxrefs(snum, 8,  flag);
	 if (flag)
	    systemword ( 0 );
      }

      if ( dumpsymbols )
	 amigasymboldump ( ptr->sect_number, yes );

      systemword ( hunk_end );

   endloop:
      ptr = ptr->sect_link;
   }

   selectoutput(o);
}

/* OK */
amigacodemodule(csize, cvec, type)
string cvec;
long csize;
int type;
{
   long r = csize/bytesper68000word;
   int sysword = (type == d_text) ? hunk_code :
		 (type == d_data) ? hunk_data :
		 (type == d_bss)  ? hunk_bss  : hunk_data;
#ifdef DBUG
   mydebug( "amigacodemodule", "nbytes = %n; nwords = %n", csize, r );
#endif

   systemword( sysword );

   writeword( (type == d_offset) ? 0L : r );

   if ( ! ((type == d_bss) || (type == d_offset)) )
      writewords(cvec, (int)(r) );
}

/* OK */
amigahunkname ( s, sysword )
string s;
int sysword;
{
   if ( s != NULL )
   {
      int i;

      char buff [((titlecharsmax + 3)/BYTESPERWORD)*BYTESPERWORD+1];
      int  length = s[0];
      int  size = (length + 3)/bytesper68000word;

      if ( ( length == 0 ) && ( sysword == hunk_name ) )
	 return;

      systemword( sysword );
      writeword((long)(size));

      for ( i = 1 ; i <= length ; i++ )
	 buff[i-1] = s[i];

      for ( i = length + 1 ; i <= (size*bytesper68000word) ; i++ )
	 buff[i-1] = 0;

      writewords (buff, size);
   }
}

/* OK */
amigarelocinfo ( snumber )
int snumber;
{
   if ( reloc_16count != 0 )
      amigarelocblock ( reloc16info, hunk_reloc16, snumber );
   if ( reloc_32count != 0 )
      amigarelocblock ( reloc32info, hunk_reloc32, snumber );
}

/* OK */
amigarelocblock ( relptr, sysword, snumber )
struct RELOCNODE *relptr;
int sysword, snumber;
{
   int count = countrelocs(relptr, no, snumber);
   struct SECTNODE *sectptr = sectionlist;

   if ( count == 0 )
      return;

   writeword ( (long)sysword );
   while ( sectptr != NULL )
   {
      count = countrelocs(relptr, yes, snumber, sectptr->sect_number);

      if ( count != 0 )
      {
	 struct RELOCNODE *ptr = relptr;
	 writeword ( (long)(count) );
	 writeword ( (long)(sectptr->sect_number) );

	 while ( ptr != NULL )
	 {
	    int sin = ptr->sectno;
	    int wrt = ptr->wrt;
	    long address = ptr->address;

	    if ( (sin == snumber) && (wrt == (int)(sectptr->sect_number)) )
	       writeword ( address );

	    ptr = ptr->link;
	 }
      }

      sectptr = sectptr->sect_link;
   }
   systemword( 0 );
}

/* OK */
int countrelocs ( relptr, flag, s_in, s_wrt )
struct RELOCNODE *relptr;
int  flag, s_in, s_wrt;
{
   int count = 0;
   while ( relptr != NULL )
   {
      int snum = relptr->sectno;
      int wrt  = relptr->wrt;

      if ( s_in == snum )
	 if ( ! ((flag == yes) && (wrt != s_wrt)) )
	    count++;

      relptr = relptr->link;
   }
   return count;
}

/* OK */
int amigaxdefs ( snum, outflag )
int snum, outflag;
{
   int	result = no;
   struct XDEFNODE *ptr = entrysymbols;
   int count = 0;

   while ( ptr != NULL )
   {
      struct SYMNODE *symbol = ptr->symbol;

      if ( symbol->st_number == snum )
	 if (outflag)
	 {
	    int  i;
	    int  type	= (symbol->st_type) & st_type_mask;
	    long value	= symbol->st_value;
	    string name = symbol->st_name;
	    int length	= name[ 0 ];
	    int  size	= ( (length-1)/bytesper68000word ) + 1;
	    string buff = tagv;
	    long info	= absolute(type) ? ext_defabs : ext_def;

	    info = (info << 24) + size;

	    if ( result == no )
	    {
	       systemword ( hunk_ext );
	       result = yes;
	    }

	    writeword ( info );

	    for ( i = 1 ; i <= length ; i++ )
	       buff[i-1] = name[i];

	    for ( i = length + 1; i <= (size*bytesper68000word); i++ )
	       buff[i-1] = 0;

	    writewords ( buff, size );
	    writeword ( value );
	 }
	 else
	    count++;

      ptr = ptr->link;
   }

   return outflag ? result : count;
}


/* OK */
int amigaxrefs ( snum, size, result )
int snum, size, result;
{
   struct XREFNODE *ptr = extrnsymbols;
   int type;

   while ( ptr != NULL )
   {
      int count;
      struct RELOCNODE *refs;

      switch (size)
      {
	case 32 : count = ptr->e_count32;
		  refs	= ptr->e_refs32;
		  type	= ext_ref32;
		  break;
	case 16 : count = ptr->e_count16;
		  refs	= ptr->e_refs16;
		  type	= ext_ref16;
		  break;
	case  8 : count = ptr->e_count8;
		  refs	= ptr->e_refs8;
		  type	= ext_ref8;
		  break;
      }

      if ( count != 0 )
      {
	 struct RELOCNODE *p = refs;
	 int c = 0;

	 while ( p != NULL )
	 {
	   if ( p->sectno == snum )
	      c++;
	   p = p->link;
	 }
	 count = c;

	 if ( count != 0 )
	 {
	    int i;
	    struct SYMNODE *symbol = ptr->e_symbol;
	    string name  = symbol->st_name;
	    int length	 = name[0];
	    int size	 = ( (length-1)/bytesper68000word ) + 1;
	    string buff  = tagv;
	    long info	 = type;

	    info = (info << 24) + (long)size;

	    if ( result == no )
	    {
	       systemword ( hunk_ext );
	       result = yes;
	    }

	    writeword ( info );

	    for ( i = 1; i <= length; i++ )
	       buff[i-1] = name[i];

	    for ( i = length+1; i <= (size*bytesper68000word); i++ )
	       buff[i-1] = 0;

	    writewords(buff, size);
	    writeword((long)count);

	    while ( refs != NULL )
	    {
	       if ( snum == refs->sectno )
		  writeword ( refs->address );

	       refs = refs->link;
	    }
	 }
      }

      ptr = ptr->e_link;
   }

   return result;
}


/* OK */
int amigasymboldump ( snum, flag )
int snum, flag;
{
   int i;
   int count = 0;

   if (flag)
      systemword( hunk_symbol );

   for ( i = 0; i <= (tagtablesize-1); i++ )
   {
      struct SYMNODE *tag = tagtable[i];

      while ( tag != NULL )
      {
	 int type = (tag->st_type) & st_type_mask;

	 if ( type == s_rel )
	 {
	    int sn = (tag->st_type >> 8) & 0xFF;

	    if ( sn == snum )
	       if ( flag )
	       {
		  int i;
		  string name	= tag->st_name;
		  long value	= tag->st_value;
		  int length	= name[0];
		  int nwords	= ((length-1)/bytesper68000word) + 1;
		  string buffer = tagv;
		  long info	= ext_symb;

                  /* Don't output 'C compiler generated' local labels
                     and any other local labels if the user doesn't 
                     want them */

                  if ( !( (output_local_labels == no) &&
                          ((('0' <= name[1]) && (name[1] <= '9')) ||
                          (name[1] == '.')) ) )
                  {
		    info = (info << 24) + (long)nwords;

		    writeword ( (long)info );

		    for ( i = 1; i <= length; i++ )
		       buffer[i-1] = name[i];

		    for ( i = length+1; i <= (nwords*bytesper68000word); i++ )
		       buffer[i-1] = 0;

		    writewords (buffer, nwords);
		    writeword (value);
                  }
	       }
	       else
		  count++;
	 }
	 tag = tag->st_link;
      }
   }

   if ( flag )
      systemword ( 0 );

   return count;
}

