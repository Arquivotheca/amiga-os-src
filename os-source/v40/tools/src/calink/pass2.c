/* ALINKP2 : Automatically translated from BCPL to C */
/* ***********************************************************      */ 
/*                ##         ##        ##  ##     ##  ##    ## */
/*               ####        ##        ##  ###    ##  ##   ## */
/*              ##  ##       ##        ##  ####   ##  ##  ## */
/*             ########      ##        ##  ## ##  ##  #### */
/*            ##      ##     ##        ##  ##  ## ##  ## ## */
/*           ##        ##    ##        ##  ##   ####  ##  ## */
/*          ##          ##   ########  ##  ##    ###  ##   ## */
/*         ##            ##  ########  ##  ##     ##  ##    ## */
/*      */
/* ***********            March   1985      **************************/

#include "ALINKHDR.h"

F0052_dop2 ( )
{
   /*  This routine: */
   /*    1. Outputs the hunk size table */
   /*    2. Outputs the overlay supervisor */
   /*    3. Outputs the root files */
   /*    4. Outputs the library hunks (if any) */
   /*    5. Outputs the common blocks */
   /*    6. Outputs the overlay table */
   /*    7. Outputs the rest of the tree */
   pass1 = no;
   pass2 = yes;

   gcodevec = F0023_vec_get(maxsize); /* AC fix */

   currentfile = 0L;
   F0059_o_hunk_header ( hunklist, total_root_count );
   /*  stop output hunk header outputting resident info each go */
   /*  for overlays */
   resident_hunk_count = 0L;
   if ( overlaying )
      F0054_out_ovs (  );
   F0053_output_root_hunks (  );
   /*  Output the overlay information */
   if ( ovly_count >= 0L )
   {
      word j; /* FOR loop variables */
      word optr [ ( mark_size - 1L ) + 1 ];
      word size = ( ovly_count + 1L ) * ovly_entry_size;
      F0020_putword ( hunk_overlay );
      F0020_putword ( ( max_level + size ) + 1L );
      F0020_putword ( max_level + 2L );
      for ( j = 1L ; j <= ( max_level + 1L ) ; j++ )
         F0020_putword ( 0L );
      F0057_mark_file_position ( optr );
      for ( j = 1L ; j <= size ; j++ )
         F0020_putword ( 0L );
      F0056_out_ovly_hunks (  );
      F0058_set_file_position ( optr );
      /*  Output the overlay table information */
      for ( j = 0L ; j <= ovly_count ; j++ )
      {
         word m, k; /* FOR loop variables */
         word *s = (word*)ovly_symbol_table[ j ];
         word *h = (word*)s[ symbol_hunk ];
         word *n = (word*)h[ hunk_node ];
         /*  Output the file mark */
         for ( m = 0L ; m <= ( mark_size - 1L ) ; m++ )
            F0020_putword ( n[ node_mark + m ] );
         F0020_putword ( h[ hunk_level ] );
         F0020_putword ( h[ hunk_ordinate ] );
         F0020_putword ( ((word*)n[ node_hunks ])[ hunk_num ] );
         F0020_putword ( h[ hunk_num ] );
         F0020_putword ( s[ symbol_value ] + 4L );   /*  addrinc) */
         /*  Fill up entry with zeroes */
         for ( k = mark_size + 6L ; k <= ovly_entry_size ; k++ )
            F0020_putword ( 0L );
      }
   }
   F0022_deplete_output (  );
}

F0053_output_root_hunks ( )
{
   word i; /* FOR loop variables */
   word *hunk = hunklist;
   for ( i = 1L ; i <= total_root_count ; i++ )
   {
      word type = hunk[ hunk_type ];
      word memtype = type & memmask;
      switch ( (int)(type & typemask))
      {
      default:
         error ( err_int_hunk, type );
         break;
      case (int) hunk_code:
      case (int) hunk_data:
      case (int) hunk_bss:
         F005B_pass2_read_hunk ( hunk, "\004ROOT" );
         break;
      case (int) ext_common:
         F0055_out_bss_hunk ( hunk );
         break;
      }
      F0020_putword ( hunk_end );
      hunk = (word*)hunk[ hunk_link ];
      if ( hunk == 0L )
         break;
   }
   hunklist = hunk;
}

F0054_out_ovs ( )
{
   loading_ovly_supervisor = yes;
   F005B_pass2_read_hunk ( hunklist );
   loading_ovly_supervisor = no;
   total_root_count = total_root_count - 1L;
   F0020_putword ( hunk_end );
   hunklist = (word*)hunklist[ hunk_link ];
}

F0055_out_bss_hunk ( hunk )
word *hunk;
{
   F0020_putword ( hunk_bss );
   F0020_putword ( hunk[hunk_hunkname]? ((word*)(hunk[hunk_hunkname]))[hname_size]: hunk[ hunk_size ] );
   F0020_putword ( hunk_end );
}

F0056_out_ovly_hunks ( )
{
   word *hunk = hunklist;
   word oord = 0L;   /*  different to nord */
   word nord = hunk[ hunk_ordinate ];
   word nlevel = hunk[ hunk_level];
   word olevel = 0L;  
   word first = yes;
   do
   {
      if (( nord != oord ) || ( olevel != nlevel))
      {
         word *node = (word*)hunk[ hunk_node ];
         if ( ! first )
            F0020_putword ( hunk_break );
         first = no;
         F0057_mark_file_position ( node + node_mark ); 
         F0059_o_hunk_header ( node[ node_hunks ], node[ node_count ] ); 
         oord = nord;
	 olevel = nlevel;

      }

      F005B_pass2_read_hunk ( hunk, "\007overlay" );
      F0020_putword ( hunk_end );
      hunk = (word*)hunk[ hunk_link ];
      if ( hunk == 0L )
         break;
      nord = hunk[ hunk_ordinate ];
      nlevel = hunk[ hunk_level ];
   } while ( true );
   F0020_putword ( hunk_break );

}

F0057_mark_file_position ( v )
word v;
{
   F0022_deplete_output (  );
   note ( tostream, v );
}

F0058_set_file_position ( v )
word v;
{
   F0022_deplete_output (  );
   point ( tostream, v );
}

F0059_o_hunk_header ( list, count )
word *list, count;
{
   if ( count != 0L )
   {
      word j; /* FOR loop variables */
      F0020_putword ( hunk_header );
      F005A_output_resident_bits (  );
      F0020_putword ( max_hunk_number + 1L );
      F0020_putword ( list[ hunk_num ] );
      F0020_putword ( ( list[ hunk_num ] + count ) - 1L );
      for ( j = 1L ; j <= count ; j++ )
      {  word * hname = (word*)(list[ hunk_hunkname]);
	 word size = hname ? hname[hname_size] : list[ hunk_size];
	 word memtype = (list[hunk_type]) & memmask;
         F0020_putword ( size | memtype );
         list = (word*)list[ hunk_link ];
      }
   }
}

/*  note this only puts out info for root block as resident.hunk.count set to 0 */
/*  after this */
F005A_output_resident_bits ( )
{
   word i; /* FOR loop variables */
   word *list = completelist;
   for ( i = 1L ; i <= resident_hunk_count ; i++ )
   {
      word len = ((word*)(list[ hunk_hunkname ]))[ hname_len ] & symbol_start_mask
         ;
      F0020_putword ( len );
      F0021_putwrds ( ((word*)(list[ hunk_hunkname ]))[ hname_ptr ], len );
      list = (word*)list[ hunk_link ];
   }
   F0020_putword ( 0L );
}

/*  This routine is used for reading (and outputting) hunks in the */
/*   second pass. */
word F005B_pass2_read_hunk ( hunk, name )
word *hunk, name;
{
   /*  This routine reads a hunk, and writes it, together */
   /*   with any relocation information to the output file. */
   /*  It resolves external symbol references, and generates */
   /*   references to the overlay supervisor symbols. */
   word codevec = (word) gcodevec; /* AC fix^2 */
   word *relocvec = 0L;
   word relocsize = 0L;
   word t = 0L;
   word *symbols = 0L;
   word *symbolchain = 0L;
   word bytebase = 0L;
   word base = 0L;
   word notfirst = no;
   word *clink = (word*)hunk[ hunk_clink ];
   word size = hunk[ hunk_size ];
   word bytetop = size << 2L;
   word type = hunk[ hunk_type ];
   word ov_level = hunk[ hunk_level ];
   word debugvec [ ( 2L ) + 1 ];
   word *symbolvec = 0L;
   word currentpos;
   word codevecsize = clink != 0L ? ((word *)(hunk[ hunk_hunkname ]))[ hname_size ]
       : size;
   string file = (string)hunk[ hunk_file ];
   string cfile = file;
   word num = hunk[ hunk_num ];
   word *currenthunk = (word*)hunk;
   word myinstream;
   word totalsize = codevecsize << 2L;
   word myfilemark = hunk[ hunk_mark ];
   from_file = cfile;
   hunkname = (word*)hunk[ hunk_hunkname ];
   if ( ! ( ( currentfile == 0L ) || ( file == currentfile ) ) )
   {
      endread (  );
      fromstream = 0L;
   }
   if ( ! ( file == currentfile ) )
   {
      fromstream = F000B_open(file, name, yes, yes);
      filemark = 0L;
      simplepoint  = 0L;
      selectinput ( fromstream );
   }
   myinstream = input();
   currentfile = file;
   mypointword ( myfilemark );
   debugvec[ 0L ] = 0L;
   debugvec[ 1L ] = 0L;

   F0020_putword ( type );
   F0020_putword ( codevecsize );
   type = type & typemask;

   do
   {
      /*  this bit must be repeated for coagulation */
      /* ========================================================================== */
      if ( notfirst )
      {
         word *chunk = (word*)clink[ clink_hunk ];
         string ofile = cfile;
         currenthunk = chunk;
         cfile = (string)chunk[ hunk_file ];
         from_file = cfile;
         /*  do an endread if new file */
         /*  or old file not parent file */
         if ( ! ( ( ofile == file ) || ( ofile == cfile ) ) )
         {
            endread (  );
            fromstream = 0L;
         }
         if ( ! ( ofile == cfile ) )
            { simplepoint = 0L ;
            if ( cfile == file )
            {
               fromstream = myinstream;
               selectinput ( myinstream );
            }
            else
            {
               fromstream = F000B_open(cfile, name, yes, yes);
               filemark = 0L;
               simplepoint = 0L;
               selectinput ( fromstream );
            };
            }
         myfilemark = chunk[ hunk_mark ];
         mypointword ( myfilemark );
         base = chunk[ hunk_base ];
         bytebase = base << 2L;
         size = chunk[ hunk_size ];
         bytetop = bytebase + ( size << 2L );
         clink = (word*)clink[ clink_link ];
      }
      if ( ( type == hunk_data ) || ( type == hunk_code ) )
         F001D_getwrds ( codevec + bytebase, size );  /* AC fix */

      t = F001A_getoptword();
      /*  Check for relocation information NOW 1 piece for each hunk */
      while ( ( ( t == hunk_reloc32 ) || ( t == hunk_reloc16 ) ) || ( t
          == hunk_reloc8 ) )
      {
         if ( type == hunk_bss )
            error ( err_bss_rel2, currenthunk );
         switch ( (int)t )
         {
         case (int) hunk_reloc32:
            F0062_do32bitreloc (  & relocvec , codevec, bytebase, bytetop
               , currenthunk );
            break;
         case (int) hunk_reloc16:
            F0063_dorelocation ( codevec, 16L, 2L, 32767L,  - 32768L ,0xFFFF0000L
				 ,0x8000L , bytebase , bytetop, num, currenthunk );
            break;
         case (int) hunk_reloc8:
            F0063_dorelocation ( codevec, 8L, 1L, 127L,  - 128L , 0xFFFFFF00L, 0x80L,
				 bytebase , bytetop, num, currenthunk );
            break;
         }
         t = F001A_getoptword();
      }
      if ( t == hunk_ext )
      {
         symbols = F0011_read_extblock(no,  & symbolchain , bytebase);
         if ( ! ( symbols == 0L ) )
            F005C_do_symbol_relocation ( symbols, codevec, num, type, ov_level
               , bytebase, bytetop, currenthunk );
         t = F001A_getoptword();
      }
      if ( t == hunk_symbol )
      {
         F0067_read_symbols (  & symbolvec , base );
         t = F001A_getoptword();
      }
      if ( t == hunk_debug )
         F0065_read_debug ( debugvec );
      else
         F001B_ungetword (  );
      if ( ! notfirst )
      {
         currentpos = filemark;
         notfirst = yes;
      }

      codevec = codevec - (size<<2); /* AC fix */
 
      if ( ( type == hunk_data ) || ( type == hunk_code ) )
      F0021_putwrds ( gcodevec, size ); /* AC fix */
      
   } while ( ! ( clink == 0L ) );
   /*  end of repeat */
   /* ========================================================================== */
   if ( ! ( cfile == file ) )
   {
      endread (  );
      fromstream = 0L;
      simplepoint = 0L;
   }
   fromstream = myinstream;
   selectinput ( fromstream );
   mypointword ( currentpos );

   /*  Now output the relocation information, if any. */
   /*  needs changing as only reloc32 at present */
   if ( ! ( relocvec == 0L ) )
   {
      F0064_handle_relocvec (  & relocvec  );
      /*  Bob's fix for ifs theres no xrefs  */
      if ( symbolchain == 0L | loading_ovly_supervisor )
         F0020_putword ( 0L );
   }
   /*  Now output the condensed external relocation information */
   if ( loading_ovly_supervisor )
      symbolchain = 0L;
   symbols = symbolchain;
   /*  if all the symbols are 16/8 refs then no reloc made */
   if ( symbols != 0L )
   {
      word started = relocvec != 0L;   /*  only put header if relocvec 0 */
      /*  Loop through the symbols */
      while ( ! ( symbols == 0L ) )
      {
         word *h = (word*)symbols[ symbol_hunk ];
         /*  If absolute, or not referenced, then nothing to do */
         if ( ( h == 0L ) || ( symbols[ symbol_reflist ] == 0L ) )
         {
            word *s = symbols;
            symbols = (word*)symbols[ symbol_link ];
            F0026_freesymbol ( s );
         }
         else
         {
            word j; /* FOR loop variables */
            word count = 0L;
            for ( j = 1L ; j <= 2L ; j++ )
            {
               word *lvs = (word*)&symbols ;
               if ( j == 2L )
                  if ( count > 0L )
                  {
                     if (  ! started  )
                     {
                        F0020_putword ( hunk_reloc32 );
                        started = yes;
                     }
                     F0020_putword ( count );
                     F0020_putword ( h[ hunk_num ] );
                  }
               while ( ! ( (word*)(*lvs) == 0L ) )
                  if ( ((word*)(*lvs))[ symbol_hunk ] == (word)h )
                  {
                     word *s = (word*)(*lvs);
                     word *refs = (word*)(s[ symbol_reflist ]);
                     /*  Loop through references */
                     while ( ! ( refs == 0L ) )
                     {
                        if ( refs[ ref_type ] == ext_ref32 )
                           if ( j == 2L )
                              F0020_putword ( refs[ ref_offset ] );
                           else
                              count = count + 1L;
                           refs = (word*)refs[ ref_link ];
                     }
                     if ( j == 2L )
                     {
                        (*lvs) = s[ symbol_link ];
                        F0026_freesymbol ( s );
                     }
                     else
                        lvs = symbol_link + s;
                  }
                  else
                     /*  Not the same hunk */
                     lvs = symbol_link + (word*)(*lvs);
            }
         }
      }
      /*  Terminate the table */
      if ( started )
         F0020_putword ( 0L );
   }
   /*  handle symbol and debug hunks */
   if ( ! ( symbolvec == 0L ) )
      F0068_handlesymbol (  & symbolvec  );
   if ( ! ( debugvec[ 0L ] == 0L ) )
      F0066_handledebug ( debugvec );
   return 0L;
}

F005C_do_symbol_relocation ( symbols, codevec, num, type, ov_level, bytebase, bytetop, currenthunk )
word *symbols, *codevec, num, type, ov_level, bytebase, bytetop, *currenthunk;
{
   word *s = symbols;
   /*  Check for valid relocation */
   if ( ( s != 0L ) && ( type == hunk_bss ) )
      error ( err_bss_rel1, s, currenthunk );
   /*  First loop through the symbols and do the relocation in the */
   /*   code vector. */
   while ( ! ( s == 0L ) )
   {
      word *l = F0012_lookup(s, t_sym);
      word ov = no;
      word *refs = (word*)(s[ symbol_reflist ]);
      word t = 200L;
      word levl = 0L;
      /*  If the symbol exists, but its hunk is zero, */
      /*   it was undefined, and the level must be */
      /*   zero. */
      if ( l != 0L )
      {
         word *h = (word*)l[ symbol_hunk ];
         if ( h != 0L )
            levl = h[ hunk_level ];
      }
      /*  Check for overlay reference  */
      if ( ( l != 0L ) && ( F0016_symtype(l[ symbol_name ]) < 128L ) )
         if ( levl > ov_level )
         {
            F005F_construct_ovly_symbol ( s, l[ symbol_overlaynumber ] );
            l = F0012_lookup(s, t_sym);
            ov = yes;
         }
      if ( l != 0L )
         t = F0016_symtype(l[ symbol_name ]);
      /*  Check that the symbol is defined */
      if ( t >= 128L )
         error ( err_int_bug );
      /*    if symbol is absolute set hunk POINTER to 0   */
      s[ symbol_hunk ] = t == ext_abs ? 0L : l[ symbol_hunk ];
      /*  Satisfy the references */
      while ( ! ( refs == 0L ) )
      {
         word o = refs[ ref_offset ];
         /*  Check for valid overlay reference */
         if ( ov && ( F005D_getw(codevec, o) != 0L ) ) 
            error ( err_ovs_ref2, l, refs ); 

	 refs[ ref_hunk ] = (word)currenthunk;

         switch ( (int)(refs[ ref_type ]) )
         {
         case (int) ext_ref32:
            F0060_do32ref ( codevec, l, refs, o, bytebase, bytetop );
            break;
         case (int) ext_ref16:
            F0061_doref ( codevec, ext_ref16, l, refs, o, num, 16L, 32767L
               ,  - 32768L , 2L, 0xFFFF0000L,0x8000L, s[ symbol_hunk ] 
	       ,bytebase, bytetop );
            break;
         case (int) ext_ref8:
            F0061_doref ( codevec, ext_ref8, l, refs, o, num, 8L, 127L,  - 128L
                , 1L, 0xFFFFFF00L, 0x80L, s[ symbol_hunk ], bytebase, bytetop );
            break;
         }

         refs = (word*)(refs[ ref_link ]);
      }
      s =(word*)( s[ symbol_link ]);
   }
}

word F005D_getw ( vector, offset )
word vector, offset;
{
    return gbytes( vector + offset, 4);
}

word F005E_add ( vector, offset, value )
word vector, offset, value;
{
    return (word)pbytes(vector + offset, 4, value + gbytes(vector + offset, 4));
}


   /*  this routine handles 16/8 bit addition to code vec etc */
word add16_8 ( vector, offset, value, n_bytes, max, min, ormask, andmask )
word vector, offset, value, max, min, ormask, andmask, n_bytes;
{
   word ovalue = gbytes(vector + offset,(int)n_bytes);   /*  fish out old value */
   if ( ! ( ( ovalue & andmask ) == 0L ) )   /*  is it negative */
      ovalue = ovalue | ormask;   /*  yes, so sign extend */
   value = value + ovalue;   /*  add together */
   if ( ! ( ( min <= value ) && ( value <= max ) ) )
   {
      myresult2 = value;
      return no;
   }
   pbytes ( vector + offset, (int)n_bytes, value );
   return yes;
}

/*  These routines are concerned with overlaying */
F005F_construct_ovly_symbol ( s, n )
word *s, n;
{
   word i; /* FOR loop variables */
   char *ovlyname = (char *)F0023_vec_get(2L);
   s[ symbol_name ] = 2L + ( ext_def << first_byte_shift );
   for ( i = 0L ; i <= 3L ; i++ )
      ovlyname[ i ] = "\004OVLY"[ i + 1L ];
   for ( i = 7L ; i >= 4L ; i-- )
   {
      char c = ( n % 10L ) + '0';
      ovlyname[ i ] = c;
      n = n / 10L;
   }
   s[ symbol_nameptr ] = (word)ovlyname;
}

/*  do 32 bit relocation */
/*  therefore all we have to do is splat these locations with */
/*  providing relocation is in range */
/*  i.e 0 <= offset <= bsize */
F0060_do32ref ( codevec, symbol, ref, o, bytebase, bytetop )
word *codevec, *symbol, *ref, o, bytebase, bytetop;
{
   word type = F0016_symtype(symbol[ symbol_name ]);   /*  get type res abs or rel */
   /*  only add the bytebase if a ext.def */
   word value = symbol[ symbol_value ] + 
               ( type == ext_def ? ((word*)(symbol[ symbol_hunk ]))[ hunk_base ] << 2L : 0L );
   if ( ! ( ( bytebase <= o ) && ( o <= ( bytetop - 4L ) ) ) )
      error ( err_off_ref1, symbol, ref );
   F005E_add ( codevec, o, value );
}

/*  check ref + symbol have same hunk name */
/*  check ref.offset ! refs + (contname -> hunkname!hname.base,0 */
/*  in range 0 < x < FF 0 < x < FFFF */
/*  splat all locations with x - symbol.value */
F0061_doref ( codevec, type, l, ref, o, num, bitsize, max, min, bytesize
	      ,ormask, andmask , defhunk, bytebase, bytetop )
word codevec, type, *l, *ref, o, num, bitsize, max, min, bytesize,
     ormask, andmask,*defhunk, bytebase, bytetop;
{
   /*  check things unless absolute */
   word absolute = defhunk == 0L;
   word defbase = absolute ? 0L : ( defhunk[ hunk_base ] << 2L ) - ( type
       == ext_ref8 ? 1L : 0L );
   word value = l[ symbol_value ] + defbase;
   if ( ! absolute )
   {
      if ( hunkname == 0L )
         error ( err_hunk_ref, bitsize, l, ref );
/*      if ( ! ( (word)hunkname == ((word*)(ref[ ref_hunk ]))[ hunk_hunkname ] ) ) */
      if ( ! ( defhunk[hunk_hunkname] == ((word*)(ref[ ref_hunk ]))[ hunk_hunkname ] ) ) 
         error ( err_hunk_ref, bitsize, l, ref );
   }
   value = value - ( absolute ? 0L : o );
   if ( ! ( ( bytebase <= o ) && ( o <= ( bytetop - bytesize ) ) ) )
      error ( err_off_ref2, bitsize, l, ref );
   if ( ! add16_8(codevec, o, value, bytesize, max, min, ormask, andmask))
      error ( err_value_ref, bitsize, l, ref, myresult2 );
}

/*  read in the relocation info and use */
/*  F0069.getnthhunkinpu(n) */
/*  to get actual hunk number */
/*  then for each bit of relocation vec.get some space */
/*  relocvec - > | link | len | hunk number | reloc info | */
/*                   | */
/*               | link | len | hunk number | reloc info | */
/*               |  0   | len | hunk number | reloc info | */
/*  remember if get nth hunk ditto is a cont hunk then add hunkbase to reloc */
/*  info */
F0062_do32bitreloc ( relocvec, codevec, bytebase, bytetop, thishunk )
word *relocvec, *codevec, bytebase, bytetop, *thishunk;
{
   word len = getword();   /*  length */
   word *hunk = F0069_getnthhunkinpu(getword(), thishunk);   /*  wrt hunk */
   word base = (hunk[ hunk_base ]) << 2L;
   word num = hunk[ hunk_num ];   /*  hunk number reloc wrt */
   word *list = F0023_vec_get(len + 3L);
   word *rvec = list + 3L;
   word *res = list;
   do
   {
      if ( ( bytebase == 0L ) && ( base == 0L ) )
         /*  niether hunk is a continuation  */
         /*  simple case  */
         F001D_getwrds ( rvec, len );
      else
      {
         word i; /* FOR loop variables */
         /*  else if a continuation need to patch offset */
         /*  and patch at offset with base of wrt hunk */
         for ( i = 0L ; i <= ( len - 1L ) ; i++ )
         {
            word offset = getword() + bytebase;
            if ( ! ( ( bytebase <= offset ) && ( offset <= ( bytetop - 4L
                ) ) ) )
               error ( err_off_rel1, thishunk, hunk, offset );
            F005E_add ( codevec, offset, base );
#ifdef SUN
            rvec[ i ] = offset;
#endif

#ifdef IBM
            rvec[ i ] = swap(&offset);
#endif
         }
      }
#ifdef SUN
      list[ 1L ] = len;
      list[ 2L ] = num;
#endif
#ifdef IBM
      list[ 1L ] = swap(&len);
      list[ 2L ] = swap(&num);
#endif
      len = getword();
      if ( len == 0L )
         break;
      hunk = F0069_getnthhunkinpu(getword(), thishunk);
      base = (hunk[ hunk_base ])<<2;
      num = hunk[ hunk_num ];
      list[ 0L ] = (word)F0023_vec_get(len + 3L);
      list = (word*)list[ 0L ];
      rvec = list + 3L;
   } while ( true );
   list[ 0L ] = 0L;
   while ( ! ( (*relocvec) == 0L ) )
      relocvec =(word*) (*relocvec);
   (*relocvec) = (word)res;
}

/*  check that relocation is wrt a hunk with same hunk name */
/*  also make sure that relocation is valid i.e the hunk base of the */
/*  hunk to relocate wrt is not greater than FF or FFFF */
/*  codevec = codevector */
/*  size = 16/8 */
/*  max = */
/*  n */
/*  bytebase = base of current hunk */
F0063_dorelocation ( codevec, bitsize, bytesize, max, min, ormask, andmask ,
		      bytebase, bytetop, thisnum, thishunk )
word codevec, bitsize, bytesize, max, min, ormask, andmask, 
       bytebase, bytetop, thisnum, *thishunk;
{
   if ( ! ( hunkname != 0L ) )
      error ( err_hunk_rel, bitsize, thishunk, 0L );
   {
      word len = getword();
      word *hunk = F0069_getnthhunkinpu(getword(), thishunk);
      word base = (hunk[ hunk_base ])<<2;
      word num = hunk[ hunk_num ];
      do
      {
         word i; /* FOR loop variables */
         if ( ! ( num == thisnum ) )
            error ( err_hunk_rel, bitsize, thishunk, hunk );
         /*  relocate things */
         for ( i = 1L ; i <= len ; i++ )
         {
            word offset = getword() + bytebase;
            word value = base + gbytes(codevec + offset, (int)bytesize);
            if ( ! ( ( bytebase <= offset ) && ( offset <= ( bytetop - bytesize
                ) ) ) )
               error ( err_off_rel2, bitsize, thisnum, thishunk, offset
                   );
            if ( !  add16_8(codevec,offset,value,bytesize,max,min,ormask,andmask)) 
               error ( err_value_rel, bitsize, thishunk, hunk, myresult2 );
         }
         len = getword();
         if ( len == 0L )
            return;
         hunk = F0069_getnthhunkinpu(getword(), thishunk);
         base = (hunk[ hunk_base ])<<2;
         num = hunk[ hunk_num ];
      } while ( true );
   }
}

/*  just out put the relocation vector that we made up above */
/*  return TRUE if something there FALSE otherwise */
/*  pass address of relocve */
F0064_handle_relocvec ( relocvec )
word *relocvec;
{
   relocvec = (word*)(*relocvec);
   F0020_putword ( hunk_reloc32 );
   while ( ! ( relocvec == 0L ) )
   {
      word *orv = relocvec;
#ifdef SUN
      word len = relocvec[ 1L ];
#endif
#ifdef IBM
      word len = swap(&(relocvec[ 1L ]));
#endif
      F0021_putwrds ( relocvec + 1L, len + 2L );   /*  output len hunk info */
      relocvec = (word*)relocvec[ 0L ];
      F0024_vec_free ( orv );
   }
}

F0065_read_debug ( v )
word *v;
{
   word n = getword();
   word *nv = F0023_vec_get(n + 2L);
   word *chain = v;
   F001D_getwrds ( nv + 2L, n );
   (*nv) = 0L;
   nv[ 1L ] = n;
   while ( ! ( (*chain) == 0L ) )
      chain = (word*)(*chain);
   (*chain) = (word)nv;
   v[ 1L ] = v[ 1L ] + n;
}

F0066_handledebug ( v )
word *v;
{
   word len = v[ 1L ];
   word *nv = (word*)(*v);
   F0020_putword ( hunk_debug );
   F0020_putword ( len );
   while ( ! ( nv == 0L ) )
   {
      word *temp = nv;
      word len1 = nv[ 1L ];
      F0021_putwrds ( nv + 2L, len1 );
      F0024_vec_free ( temp );
      nv = (word*)(*nv);
   }
}

F0067_read_symbols ( v, base )
word *v, base;
{
   word len = getword();
   base = base << 2L;
   while ( ! ( len == 0L ) )
   {
      word *nv = F0023_vec_get(len + 3L);
      F001D_getwrds ( nv + 2L, len );   /*  name */
#ifdef SUN
      ( nv + 2L )[ len ] = getword() + base;   /*  offset */
#endif 
#ifdef IBM
      { word temp = getword() + base;
      ( nv + 2L )[ len ] = swap(&temp);   /*  offset */
      }
#endif 
      nv[ 1L ] = len;
      while ( ! ( (*v) == 0L ) )
         v = (word*)(*v);   /*  run down chain */
      (*v) = (word)nv;   /*  add to end */
      (*nv) = 0L;
      len = getword();
   }
}

/*  pass info straight through but if we are a cont hunk then add hunkname ! */
/*  symbol.hunbase to symbol value */
F0068_handlesymbol ( v )
word *v;
{
   word *nv = (word*)(*v);
   F0020_putword ( hunk_symbol );
   while ( ! ( nv == 0L ) )
   {
      word len = nv[ 1L ];
      word *temp = nv;
      F0020_putword ( len );
      F0021_putwrds ( nv + 2L, len + 1L );   /* len + value */
      nv = (word*)(*nv);
      F0024_vec_free ( temp );
   }
   F0020_putword ( 0L );
}

/*  get nth hunk  */
word *F0069_getnthhunkinpu ( n, hunk )
word n, *hunk;
{
   word i; /* FOR loop variables */
   word *pu = (word*)hunk[ hunk_pu ];
   word *res = (word*)pu[ pu_first ];
   for ( i = 1L ; i <= n ; i++ )
   {
      if ( res == 0L )
         break;
      res = (word*)res[ hunk_nextpuhunk ];
   }
   if ( res == 0L )
      error ( err_pu_rel, hunk, n );
   return res;
}
