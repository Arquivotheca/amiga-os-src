/* ALINKMX : Automatically translated from BCPL to C */
/* ***********************************************************  */   /*            ##         ##        ##    ##  ########  ###### */
/*           ####        ##        ##   ##   ########  ####### */
/*          ##  ##       ##        ##  ##    ##        ##    ## */
/*         ########      ##        ####      ######    ##    ## */
/*        ##      ##     ##        ## ##     ##        ##    ## */
/*       ##        ##    ##        ##  ##    ##        ##    ## */
/*      ##          ##   ########  ##   ##   ########  ####### */
/*     ##            ##  ########  ##    ##  ########  ###### */
/* ********************************            March   1985      ************************************************************************ */

#include "ALINKHDR.h"

/*  This section is concerned with the output of */
/*   the link map and the cross reference table. */
F0048_o_map_and_xref ( )
{
   /*  Output MAP if required */
   if ( mapping )
   {
      if ( map_file != 0L )
      {
         mapstream = findoutput(map_file,no);
         if ( mapstream == 0L )
            warn ( err_map, map_file );
      }
      if ( mapstream != 0L )
         /*  (Note that 'output.map' closes the stream) */
         F0049_output_map (  );
   }
   /*  Ditto for XREF */
   if ( xrefing )
   {
      if ( xref_file != 0L )
      {
         xrefstream = findoutput(xref_file,no);
         if ( xrefstream == 0L )
            warn ( err_xref, xref_file );
      }
      if ( xrefstream != 0L )
         /*  (Stream is closed as in comment above) */
         F004E_output_xref (  );
   }
   /*  Repair damage done to symbol table, if necessary */
   if ( mangled )
      F0051_unmangle_symbol_table (  );
}

F0049_output_map ( )
{
   word *hunk = completelist;
   word map_entries = width < 44L ? 1L : width / 44L;
   word *h = hunk;
   mangle_symbol_table (  );
   selectoutput ( mapstream );
   if ( mapstream == verstream )
      F004D_newlines ( 3L );
   /*  Begin loop through hunks. */
   while ( ! ( hunk == 0L ) )
   {
      word *s = (word*)h[ hunk_symbols ];
      word c = 0L;
      word *clinkptr = h + hunk_clink;
      F004A_print_header ( h );
      do
      {
         word base = F004B_print_header2(h);
         if ( s != 0L )
         {
            word j; /* FOR loop variables */
            for ( j = 1L ; j <= map_entries ; j++ )
               writes ( "\054 Symbol                           Value     " );
            F004D_newlines ( 2L );
            /*  Loop through symbols */
            while ( ! ( s == 0L ) )
            {
               word t = F0016_symtype(s[ symbol_name ]);
               if ( c == map_entries )
               {
                  c = 0L;
                  newline (  );
               }
               writef ( "\013 %O2 %X8%C ", s, s[ symbol_value ] + ( t == ext_abs
                   ? 0L : base ), t == ext_abs ? '*' : ' ' );
               c = c + 1L;
               s = (word*)s[ symbol_hunk ];
            }
            F004D_newlines ( 2L );
         }
         /*  do the clinks */
         if ( (*clinkptr) == 0L )
            break;
         h = (word*)(((word*)(*clinkptr))[ clink_hunk ]);
         clinkptr = ((word*)(*clinkptr)) + clink_link;
         s = (word*)h[ hunk_symbols ];
      } while ( true );
      newline (  );
      hunk = (word*)hunk[ hunk_link ];
      h = hunk;
   }
   if ( mapstream != verstream )
   {
      endwrite (  );
      mapstream = 0L;
      selectoutput ( verstream );
   }
}

F004A_print_header ( h )
word *h;
{
   /*  Prints a header to a hunk for the map or cross reference. */
   word level = h[ hunk_level ];
   word t = h[ hunk_type ];
   word memtype = t & memmask;
   word *hunkn = (word*)h[ hunk_hunkname ];
   word size = hunkn == 0L ? h[ hunk_size ] : hunkn[ hname_size ];
   t = t & typemask;
   writef ( "\005%I2. ", h[ hunk_gnum ] );
   {
      string s = t == hunk_data ? "\007DATA.  " : t == hunk_code ? "\007CODE.  "
          : t == hunk_bss ? "\007BSS.   " : t == ext_common ? "\007COMMON."
          : t == hunk_res ? "\011RESIDENT." : "\007BUGBUG.";

      string s1 = (memtype == chipmem) ? "\006CHIP  " 
		: (memtype == fastmem) ? "\006FAST  "
		: (memtype == pubmem ) ? "\006PUBLIC"
		: "\006BADMEM";
    
      writef("\022%S Memory Type %S ",s,s1);

      if ( ( h[ hunk_size ] != 0L ) || ( t != hunk_bss ) )
         writef ( "\017Total Size %x6.", size << 2L );
   }
   /*  need bit for hunkname here (TO DO) */
   newline (  );
   /*      WHILE alist \= 0 DO */
   if ( hunkn != 0L )
      writef ( "\017 Hunkname: %o1\n", hunkn );
   if ( level != 0L )
      writef ( "\040 Overlay level %N, ordinate %N.\n", level, h[ hunk_ordinate
          ] );
   newline (  );
}

/*  printheader2 prints out the file name */
/*  base */
/*  pu name */
word F004B_print_header2 ( h )
word *h;
{
   word file = h[ hunk_file ];
   word size = h[ hunk_size ] << 2L;
   word *pu = (word*)h[ hunk_pu ];
   word punamelen = pu[ pu_namelen ];
   char *nameptr = (char*)pu[ pu_name ];
   word base = h[ hunk_base ] << 2L;
   /*  write the file name */
   writef ( "\012 File: %S\n", file );
   /*  write out program units name ( if any) */
   writes ( "\017 Program Unit: " );
   if ( punamelen == 0L )
      writes ( "\007No Name" );
   else
   {
      word i; /* FOR loop variables */
      for ( i = 0L ; i <= ( ( punamelen << 2L ) - 1L ) ; i++ )
         wrch ( nameptr[ i ] == 0L ? ' ' : nameptr[ i ] );
   }
   wrch ( '\n' );
   /*  write out the size in bytes  */
   writef ( "\026 Base: %X6  Size: %X6\n", base, size );
   newline (  );
   return base;
}

F004C_spaces ( n )
word n;
{
   word j; /* FOR loop variables */
   for ( j = 1L ; j <= n ; j++ )
      wrch ( ' ' );
}

F004D_newlines ( n )
word n;
{
   word j; /* FOR loop variables */
   for ( j = 1L ; j <= n ; j++ )
      wrch ( '\n' );
}

F004E_output_xref ( )
{
   word *hunk = completelist;
   word xref_entries = width < 51L ? 1L : ( width - 27L ) / 16L;
   word *h = hunk;
   if ( ! mangled )
      mangle_symbol_table (  );
   selectoutput ( xrefstream );
   if ( xrefstream == verstream )
   {
      F004D_newlines ( 3L );
      if ( xrefstream == mapstream )
         F004D_newlines ( 2L );
   }
   /*  Loop through hunks */
   while ( ! ( hunk == 0L ) )
   {
      word *s = (word*)h[ hunk_symbols ];
      word *clinkptr = h + hunk_clink;
      F004A_print_header ( h );
      do
      {
         F004B_print_header2 ( h );
         if ( s != 0L )
         {
            word j; /* FOR loop variables */
            writes ( "\012 Symbol   " );
	    for ( j = 1L ; j <= 24 ; j++) wrch(' ');

            for ( j = 1L ; j <= xref_entries ; j++ )
               writes ( "\020Offset   Hunk   " );
            F004D_newlines ( 2L );
            /*  Loop through symbols */
            while ( ! ( s == 0L ) )
            {
               word *r = (word*)s[ symbol_reflist ];
               word c = 0L;
               writef ( "\004 %O2", s );
               /*  Loop through references */
               while ( ! ( r == 0L ) )
               {
                  word n = ((word*)(r[ ref_hunk ]))[ hunk_gnum ];
                  if ( c == xref_entries )
                  {
                     newline (  );
                     F004C_spaces ( 33L );
                     c = 0L;
                  }
                  writef ( "\006 %X8%C", r[ ref_offset ] + ( ((word*)(r[ ref_hunk
                      ]))[ hunk_base ] << 2L ), ' ' );
                  if ( n < 0L )
                     writes ( "\006res   " );
                  else
                     writef ( "\006 %I3  ", n );
                  c = c + 1L;
                  r = (word*)r[ ref_link ];
               }
               newline (  );
               s = (word*)s[ symbol_hunk ];
            }
            newline (  );
         }
         if ( (*clinkptr) == 0L )
            break;
         h = (word*)(((word*)(*clinkptr))[ clink_hunk ]);
         clinkptr = ((word*)(*clinkptr)) + clink_link;
         s = (word*)h[ hunk_symbols ];
      } while ( true );
      newline (  );
      hunk = (word*)hunk[ hunk_link ];
      h = hunk;
   }
   if ( xrefstream != verstream )
   {
      endwrite (  );
      xrefstream = 0L;
      selectoutput ( verstream );
   }
}

mangle_symbol_table ( )
{
   /*  This routine first assigns 'global' numbers to each hunk in */
   /*   the list (-1 for resident hunks). */
   /*  Then, every symbol is linked onto a chain from its parent */
   /*   hunk, in ascending order of value. */
   word *h = completelist;
   word c =  - 1L ;
   mangled = yes;
   while ( ! ( h == 0L ) )
   {
      h[ hunk_symbols ] = 0L;
      c = c + 1L;
      h[ hunk_gnum ] = c;
      {
         word *clink = h + hunk_clink;
         while ( ! ( (*clink) == 0L ) )
         {
            word *chunk = (word*)((word*)(*clink))[ clink_hunk ];
            chunk[ hunk_gnum ] = c;
            chunk[ hunk_symbols ] = 0L;
            clink = ((word*)(*clink)) + clink_link;
         }
      }
      h = (word*)h[ hunk_link ];
   }
   F0017_scan ( F0050_make_symbol_lists );
}

F0050_make_symbol_lists ( s )
word *s;
{
   if ( ! ( s == 0L ) )
   {
      word *h = (word*)s[ symbol_hunk ];
      if ( h != 0L )
      {
         word v = s[ symbol_value ];
         word *c = hunk_symbols + h;
         while ( ! ( ( (*c) == 0L ) || ( ((word*)(*c))[ symbol_value ] >= v ) ) )
            c = ((word*)(*c)) + symbol_hunk;
         s[ symbol_hunk ] = (*c);
         (*c) = (word)s;
      }
   }
}

F0051_unmangle_symbol_table ( )
{
   /*  This routine restores the symbol table to its normal state. */
   word *hunk = completelist;
   word *h = hunk;
   while ( ! ( hunk == 0L ) )
   {
      word *clinkptr = hunk + hunk_clink;
      do
      {
         /*  trundle down each chain */
         word *s = (word*)h[ hunk_symbols ];
         while ( ! ( s == 0L ) )
         {
            word *t = (word*)s[ symbol_hunk ];
            s[ symbol_hunk ] = (word)h;
            s = t;
         }
         /*  if clinks do them */
         if ( (*clinkptr) == 0L )
            break;
         h = (word*)(((word*)(*clinkptr))[ clink_hunk ]);
         clinkptr = ((word*)(*clinkptr)) + clink_link;
      } while ( true );
      hunk = (word*)hunk[ hunk_link ];
      h = hunk;
   }
}

