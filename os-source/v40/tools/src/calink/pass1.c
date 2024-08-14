/* ALINKP1 : Automatically translated from BCPL to C */
/* ***********************************************************  */  
/*            ##         ##        ##    ##  ########  ###### */
/*           ####        ##        ##   ##   ########  ####### */
/*          ##  ##       ##        ##  ##    ##        ##    ## */
/*         ########      ##        ####      ######    ##    ## */
/*        ##      ##     ##        ## ##     ##        ##    ## */
/*       ##        ##    ##        ##  ##    ##        ##    ## */
/*      ##          ##   ########  ##   ##   ########  ####### */
/*     ##            ##  ########  ##    ##  ########  ###### */
/* ********    March   1985      *******************************/

#include "ALINKHDR.h"

/*  This section contains all the routines used only in */
/*   the first pass1. */
F002F_dop1 ( )
{
   word i; /* FOR loop variables */
   /*  This routine does the following things: */
   /*   1. Read root files */
   /*   2. Read descendent files */
   /*   3. Read libraries */
   /*   4. Generate COMMON hunks and the overlay symbol table */
   /*   5. Computes the maximum hunk number */
   word messageout = no;
   word charsprinted = 0L;
   word * commonliste;
   word increment;
   word * h;
   pass1 = yes;
   pass2 = no;
   /*  Read the root, resident libraries go onto completelist */
   completelist = 0L;
   e_completelist = (word*)(&completelist) ;
   root_hunk_count = F000D_files_rd(root_files, "\004FROM", 0L, 0L, ovly_tree
      ,  & max_total_size ,  & e_hunklist ) + ( overlaying ? 1L : 0L );
   if ( hunklist == 0L )
      error ( err_empty1, from_file );
   /*  when we have got this far we remember the end of the root list so that we */
   /*  can add the library hunks into it at this point */
   rootliste = e_hunklist;
   F000C_read_descendents ( ovly_tree, 1L, max_total_size, 0L );
   /*  Now read the library these are added from rootlist */
   if ( refcount != 0L )
      F0032_pass1_read_library (  );
   else
      lib_files = 0L;
   commonliste = rootliste;
   commonlistptr = rootliste;
   if ( overlaying && ( ovly_count < 0L ) )
      warn ( err_ovs_ref3 );
   /*  Allocate the overlay symbol table */
   if ( ovly_count >= 0L )
      ovly_symbol_table = F0025_getblk(ovly_count + 1L);
   /*  Generate COMMON hunks, etc. */
   /*  Note that the exact number of resident hunks is */
   /*   not yet known. (they could be in the library scan) */
   F0017_scan ( F0031_tidy_after_pass1,  & messageout ,  & commonliste 
      ,  & charsprinted  );
   /*  adds overlay supervisor at begging of rootlist if overlaying */
   if ( overlaying )
      F0043_read_ovly_supervisor (  );
   (*e_completelist) = (word)hunklist;
   h = completelist;   /*  ptr to 1st resident (if there) */
   increment = 0L;
   total_root_count = ( root_hunk_count + comm_count ) + lib_count;
   /*   trundle down the whole list excluding resident hunks and overlays */
   /*  putting in hunk number and base  */

   /* work out max size of any part of a hunk AC fix 12/09/85 */

   maxsize = 0L;
   for ( i = 1L ; i <= ( total_root_count + resident_hunk_count ) ; i++ )
   {
      h[ hunk_num ] = increment;
      if ( h[hunk_size] > maxsize) maxsize = h[hunk_size]; /* AC fix */
      F0030_setclinknumber ( h, increment, h[ hunk_size ] );
      increment = increment + 1L;
      h = (word*)h[ hunk_link ];
   }
   max_hunk_number = total_root_count - 1L;
   /*  do the overlays if any */
   while ( ! ( h == 0L ) )
   {
      word n = h[ hunk_num ] + increment;
      h[ hunk_num ] = n;
      F0030_setclinknumber ( h, n, h[ hunk_size ] );
      if ( h[hunk_size] > maxsize) maxsize = h[hunk_size]; /* AC fix */
      if ( n > max_hunk_number )
         max_hunk_number = n;
      h = (word*)h[ hunk_link ];
   }
}

/*     F0045.print.hunk.list() */
/*  procedure to set the hunknumber slot of continuations */
/*  and the hunk.base */
F0030_setclinknumber ( h, n, size )
word *h, n, size;
{
   word *clink = hunk_clink + h;
   while ( ! ( (*clink) == 0L ) )
   {
      word *chunk = (word*)(((word*)(*clink))[ clink_hunk ]);
      chunk[ hunk_num ] = n;
      chunk[ hunk_base ] = size;
      size = size + chunk[ hunk_size ];
      if ( chunk[hunk_size] > maxsize) maxsize = chunk[hunk_size]; /* AC fix */
      clink = ((word*)(*clink)) + clink_link;
   }
}

F0031_tidy_after_pass1 ( symbol, lv_messageout, lv_commonliste, lv_charsprinted )
word *symbol, *lv_messageout, *lv_commonliste, *lv_charsprinted;
{
   /*  For each symbol in the table: */
   /*   1. If it is a COMMON symbol, a hunk.bss hunk is generated, containing */
   /*        the symbol and added */
   /*        to the end of the 'commonlist', which comes after the list of */
   /*        root hunks and library hunks */
   /*   2. If the symbol is undefined, a message is produced, and the symbol */
   /*        is changed to a definition of absolute zero. */
   /*   3. If the symol has an overlay reference, it is inserted in the */
   /*        overlay symbol table. */
   /*   'lv.messageout'   points to a boolean, initially false, which is */
   /*       set true once an undefined symbol has been printed. */
   /*   'lv.commonliste'  points to a variable which contains a pointer */
   /*       to the end of the rootlist. */
   /*   'lv.charsprinted' points to an integer, initially zero, which */
   /*       contains the number of characters printed on the current */
   /*       line of undefined symbols. */
   /*   Note that 'scan' calls the routine with a zero argument as its */
   /*    final action. */
   if ( symbol == 0L )
   {
      if ( (*lv_messageout) )
         /*  Finish off undefined symbols list. */
         writes ( "\002\n\n" );
   }
   else
   {
      /*  Check for COMMON symbol. */
      word t = F0016_symtype(symbol[ symbol_name ]);
      if ( t == ext_common )
      {
         /*  Allocate new hunk */
         word *ch = F0025_getblk(size_hunk);
         word hunknum;
         word *lv_liste;
         comm_count = comm_count + 1L;
         hunknum = comm_count + root_hunk_count;
         lv_liste = lv_commonliste;
         ch[ hunk_link ] = (*((word *)(*lv_liste)));
         (*((word *)(*lv_liste))) = (word)ch;   /*  hunk end pointer */
         (*lv_liste) = (word)(ch + hunk_link );   /*  ok */
         ch[ hunk_level ] = 0L;
         ch[ hunk_ordinate ] = 0L;
         ch[ hunk_size ] = symbol[ symbol_value ] + 3L;   /*  long word */
         /*  align ?? */
         ch[ hunk_type ] = ext_common;
         ch[ hunk_file ] = 0L;
         ch[ hunk_node ] = 0L;
         ch[ hunk_hunkname ] = 0L;
         ch[ hunk_pu ] = 0L;
         ch[ hunk_clink ] = 0L;
         ch[ hunk_base ] = 0L;
         max_total_size = max_total_size + ch[ hunk_size ];
         /*  all this does is turn the symbol into a ext.def */
         symbol[ symbol_name ] = symbol[ symbol_name ] + ( ( ext_def - t
             ) << first_byte_shift );
         symbol[ symbol_hunk ] = (word)ch;
         symbol[ symbol_value ] = 4L;
      }
      else
      {   
         /*  Not COMMON - check for undefined and overlay symbols. */
         if ( t >= 128L )
         {
            /*  Undefined symbol */
            if (  !( (*lv_messageout) ) )
            {
               (*lv_messageout) = yes;
               writes ( "\052\nLinker: unresolved external references:\n\n"
                   );
            }
 /*           if ( (*lv_charsprinted) > 71L ) */
 /*           {                               */
 /*              newline (  );                */
 /*              (*lv_charsprinted) = 0L;     */
 /*           }                               */

             writef ( "\021%O1 in file \"%S\"\n", symbol,((word*)(symbol[symbol_hunk]))[hunk_file] ); 
 /*           (*lv_charsprinted) = (*lv_charsprinted) + 9L; */ 
            rcode = return_hard;
            /*  all this line does is sets the symbol as undefined value 0 */
            symbol[ symbol_name ] = symbol[ symbol_name ] + ( ( ext_abs
                - t ) << first_byte_shift );
           /*  symbol[ symbol_hunk ] = 0L; ++++++ don't lose it */
            symbol[ symbol_value ] = 0L;
            F0027_freereferences ( symbol[ symbol_reflist ] );
         }
         if ( symbol[ symbol_overlaynumber ] >= 0L )
            ovly_symbol_table[ symbol[ symbol_overlaynumber ] ] = (word)symbol
               ;
      }
   }
}

/*  These routines are used for reading libraries in pass1. */
F0032_pass1_read_library ( )
{
   /*  This routine: */
   /*    1. Reads the library files. */
   /*    2. does things to pu list */
   /*    3. Alters hunk numbers in the hunk list */
   word *lv_fl = (word*)(&lib_files);
   word nhunks = 0L;
   word nfiles = 0L;
   word *libptr = rootliste;
   word *firstlibpu = pu_current + pu_link;
   if ( (*lv_fl) == 0L )
      return;
   while ( ! ( (*lv_fl) == 0L ) )
   {
      word* file = (word*)(((word*)(*lv_fl)) + 1L);
      word nh = F0010_file_rd(file, 0L, 0L, ovly_tree, yes,  & max_total_size
          ,  & rootliste , "\007LIBRARY", 0L);
      lv_fl =(word*) (*lv_fl);
   }
   /*  trundle around the pulist and down respective xref list */
   F0039_scan_pu_list ( (*firstlibpu) );
   /*  trundle down hunks seeing if required if not then delete */
   nhunks = F003C_scan_hunk_list(libptr);
   /*  trundle down pu list deleting unused nodes */
   /*  also tidy up xrefs xdefs etc */
   /*  not quite correct for continuations */
   F003F_scan_pu_list_again ( firstlibpu );
   while ( ! ( freehunkchain == 0L ) )
   {
      word temp = freehunkchain[ hunk_link ];
      word clink = freehunkchain[ hunk_clink];   /* AC fix */
      if ( !( clink == 0L )) F0024_vec_free(clink);    /* AC fix */
      F0024_vec_free ( freehunkchain );
      freehunkchain = (word*)temp;
   }
   lib_count = nhunks;
}

/*  This routine is used for reading hunks in the first pass. */
word *F0033_pass1_read_hunk ( type, size, ov_level, ov_ord, files_node, library, number )
word type, size, ov_level, ov_ord, files_node, library, number;
{
   /*  This routine is called after the type and */
   /*   size of a hunk (hunk.data,hunk.code, hunk.bss ) have been read. */
   /*   It reads and discards the code and relocation information, and */
   /*   checks the external symbol information, if present. */
   /*  Note the following special parameters: */
   /*  library: true if this is part of a library */
   /*           if it is then we must add symbols to the library table and */
   /*           not the symbol table */
   /*  The routine delivers: */
   /*    1. A ptr to a new hunk block */
   /*  ** number is only really used by overlays via read.descendents  */
   word t;
   word type1 = type & typemask;
   word relhunk = ( type1 == hunk_data ) || ( type1 == hunk_code );
   word *symbols = 0L;
   word hashsym = library ? t_lib : t_sym;
   /*  First throw away the code if a hunk */
   if ( ! ( type1 == hunk_bss ) )
      F001F_discard_words ( size );
   /*  Check for relocation block */
   t = F001A_getoptword();
   while ( ( ( t == hunk_reloc32 ) || ( t == hunk_reloc16 ) ) || ( t == hunk_reloc8
       ) )
   {
      do
      {
         /*    discard the reloc hunk */
         word n = getword();
         if ( n == 0L )
            break;
         F001F_discard_words ( n + 1L );
      } while ( true );
      t = F001A_getoptword();
   }
   /*  Check for hunk.ext */
   if ( t == hunk_ext )
   {
      F0011_read_extblock ( library,  & symbols , 0L );
      t = F001A_getoptword();
   }
   if ( t == hunk_symbol )
   {
      word n = getword() + 1L;
      while ( ! ( n == 1L ) )
      {
         F001F_discard_words ( n );
         n = getword() + 1L;
      }
      t = F001A_getoptword();
   }
   if ( t == hunk_debug )
      F001F_discard_words ( getword() );
   else
      F001B_ungetword (  );
   {
      /*  Allocate and initialise the hunk */
      word *h;
      /*  Must allocate hunk block */
      /*  note that we use vec.get for library hunks so */
      /*  that they may be given back if not used */
#ifdef SUN
      h =  library ? F0023_vec_get(size_hunk) : F0025_getblk(size_hunk);
#endif
#ifdef IBM
      h =  F0025_getblk(size_hunk);
#endif    
      h[ hunk_link ] = 0L;
      h[ hunk_type ] = type;
      h[ hunk_size ] = size;
      h[ hunk_num ] = number;
      h[ hunk_node ] = files_node;
      h[ hunk_level ] = ov_level;
      h[ hunk_ordinate ] = ov_ord;
      h[ hunk_file ] = (word)from_file;
      h[ hunk_hunkname ] = (word)hunkname;   /*  ptr to HUNKNAME block */
      h[ hunk_pu ] = (word)pu_current;
      h[ hunk_mark ] = mark;
      h[ hunk_clink ] = 0L;
      h[ hunk_nextpuhunk ] = 0L;   /*  this stops a linked list */
      h[ hunk_base ] = 0L;   /*  for now */
      /*  if first named hunk remember its address */
      if ( ( hunkname != 0L ) &&  ! conthunk  )
         hunkname[ hname_parent ] = (word)h;
      if ( conthunk )
      {
#ifdef SUN
         word *ch = library ? F0023_vec_get(size_clink) : F0025_getblk(size_clink
#endif
#ifdef IBM
         word *ch = F0025_getblk(size_clink
#endif

            );
         word *parent = (word*)hunkname[ hname_parent ];
         word *chain = parent + hunk_clink;
         ch[ clink_hunk ] = (word) h;
         ch[ clink_link ] = 0L;
         h[ hunk_clink ] = 0L;
         /*  add info to chain of parent hunk */
         while ( ! ( (*chain) == 0L ) )
            chain = ((word*)(*chain)) + clink_link;
         (*chain) = (word) ch;
      } 
/*  Now check the symbols associated with this hunk */
      while ( ! ( symbols == 0L ) )
      {
         word *ns = (word*)symbols[ symbol_link ];
         word nt = F0016_symtype(symbols[ symbol_name ]);
         word *r = (word*)symbols[ symbol_reflist ];
         word *l = F0012_lookup(symbols, t_sym);
         word already_a_symbol = (word)l;
         word library_xdef = 1L;  /* AC fix */
         /*  if a library then look for symbols in library table as well */
         /*  but only if not found elsewhere */
         if ( library && ( l == 0L ) )
            l = F0012_lookup(symbols, t_lib);
         if ( loading_ovly_supervisor && ( l != 0L ) )
            error ( err_ovs_sym, symbols, l );
         /*  Insert new hunk into references */
         while ( ! ( r == 0L ) )
         {
            r[ ref_hunk ] = (word)h;
            r = (word*)r[ ref_link ];
         }
         /*  remember symbols is not in the hunk table */
         symbols[ symbol_hunk ] = (word)h;
         /*  See if it was a new symbol */
         if ( l == 0L )
         {
            if ( ( nt >= 128L ) && ( nt != ext_common ) )
               refcount = refcount + 1L;
            F0013_insert ( symbols );
            /*  add library 'nodes' if ness */
            if ( library )
               if ( nt >= 128L )
                  F0038_add_library_xref ( symbols );
               else {
                  F0037_add_library_xdef ( symbols );
                  library_xdef = 0L;   /* AC fix */
                    }
            any_relocatable_symbols = yes;
         }
         else
         {   /*  ? */
            /*  The symbol has already been inserted. */
            /*  First check the types for consistency. */
            word ot = F0016_symtype(l[ symbol_name ]);
            /*  check for two bad refs i.e common and something else */
            if ( ( ot != nt ) && ( ( ( ot == ext_common ) && ( nt >= 128L
                ) ) || ( ( nt == ext_common ) && ( ot >= 128L ) ) ) )
               error ( err_sym_use, symbols, l, from_file );
            else
               if ( ( ( ot & 128L ) == 0L ) && ( 0L == ( nt & 128L ) ) )
               {
                  /*  Both are definitions  only fist is used */
                  if (  ! library  &&  !( type == hunk_res ) )
                     warn ( err_mul_def, l, symbols );
               }
               else
                  /*  this test may let a few sillies slip through but I can't */
                  /*  spot them at this stage e.g. a 16 and 8 bit ref to a def */
                  if ( ( ot >= 128L ) && ( nt >= 128L ) )
                  {   /*  both references */
                     /*  Both are references */
                     word v = symbols[ symbol_value ];
                     if ( ot == ext_common )
                        /*  First find larger size. */
                        if ( v > l[ symbol_value ] )
                           l[ symbol_value ] = v;
                     F0038_add_library_xref ( symbols );
                     F0036_add_references ( symbols, l );
                  }
                  else
                  {
                     /*  One is a definition, the other is a reference */
                     /*  If the reference is to a common block, and */
                     /*  the definition is a relative symbol that is */
                     /*  far enough from the end of the hunk, then */
                     /*  the symbol becomes the definition. */
                     /*  Note also that the symbol value might be */
                     /*  a byte offset, while common block and */
                     /*  hunk sizes are in words.  In this case, */
                     /*  the symbol is required to be on a word */
                     /*  boundary for it to be a valid common */
                     /*  definition. */
                     word otisref = ot >= 128L;
                     word *ref = otisref ? l : symbols;
                     word *def = otisref ? symbols : l;
                     word reftype = otisref ? ot : nt;
                     word deftype = otisref ? nt : ot;
                     word *defhunk = (word*)def[ symbol_hunk ];
                     /*  Check for a common definition. */
                     if ( reftype == ext_common )
                     {
                        /*  Check sizes and offsets */
                        word o = def[ symbol_value ];
                        word ow = o >> 2L;
                        word ht = (defhunk[ hunk_type ]) & typemask;
                        word hs = defhunk[ hunk_size ];
                        word cs = ref[ symbol_value ];
                        if ( ( ( ( ( ( deftype != ext_def ) || ( ht != hunk_data
                            ) ) || ( ht != hunk_code ) ) || ( ( hs - ow
                            ) < cs ) ) || ( defhunk[ hunk_level ] != 0L
                            ) ) || ( ( ow << 2L ) != o ) )
                           error ( err_com_use, symbols, l, from_file );
                     }
                     /*  Check for overlay references */
                     F0034_check_references ( ref, def );
                     /*  The references will not be required again if */
                     /*   a cross reference is not being produced. */
                     if ( xrefing )
                        F0036_add_references ( ref, def );
                     else
                     {
                        F0027_freereferences ( ref[ symbol_reflist ] );
                        ref[ symbol_reflist ] = 0L;
                        def[ symbol_reflist ] = 0L;
                     }
                     /*  Set the new symbol values */
                     /*  old symbol is a ref new is a def */
                     if ( ref == l )
                     {
                        word j; /* FOR loop variables */
                        word link = l[ symbol_link ];
                        word nameptr = l[symbol_nameptr]; /* AC fix */
                        for ( j = 0L ; j <= ( size_symbol - 1L ) ; j++ )
                           l[ j ] = symbols[ j ];
                        l[ symbol_nameptr ] = nameptr ; /* AC fix */
                        l[ symbol_link ] = link;
                        if ( ot != ext_common )
                           refcount = refcount - 1L;
                        /*  old symbol ref new library def */
                        if ( library )
                        {
                           F0037_add_library_xdef ( l );
                           /*  a library def has satisfied a symbol ref */
                           if ( already_a_symbol )
                              pu_current[ pu_required ] = 1L;
                        }
                     }
                     else
                        /*  old symbol def new library symbol ref */
                        if ( library )
                           F0038_add_library_xref ( symbols );
                  }
            symbols[ symbol_reflist ] = 0L;
            if (library_xdef = 1L) F0026_freesymbol( symbols ); /* AC fix */
         }
         symbols = ns;
      }
      return h;
   }
}

/*  These routines are concerned with symbol reference chains. */
/*  They are only called in the first pass. */
F0034_check_references ( ref, def )
word *ref, *def;
{
   /*  Checks for valid overlay references. */
   word *r = (word*)ref[ symbol_reflist ];
   word *hdef = (word*)def[ symbol_hunk ];
   word ldef = hdef[ hunk_level ];
   word ndef = hdef[ hunk_node ];
   word ov = no;
   if ( ldef == 0L )
      return;
   /*  Must be OK */
   /*  Loop through all the references */
   while ( ! ( r == 0L ) )
   {
      word *href = (word*)r[ ref_hunk ];
      word lref = href[ hunk_level ];
      word nref = href[ hunk_node ];
      r = (word*)r[ ref_link ];
      if ( ndef == nref )
         continue;
      /*  OK - in the same overlay segment */
      if ( ( ldef < lref ) && F0035_descends_from(nref, ndef, lref - ldef
         ) )
         continue;
      /*  OK - legal reference to point higher in tree */
      if ( ( ldef == ( lref + 1L ) ) && F0035_descends_from(ndef, nref, 1L
         ) )
      {
         /*  OK - overlay reference to next lower level */
         ov = yes;
         continue;
      }
      error ( err_ovs_ref1, def,href,hdef);
   }
   /*  See if symbol is an overlay reference */
   if ( ov && ( def[ symbol_overlaynumber ] ==  - 1L  ) )
   {
      ovly_count = ovly_count + 1L;
      def[ symbol_overlaynumber ] = ovly_count;
   }
}

word F0035_descends_from ( high, low, count )
word *high, *low, count;
{
   /*  Returns true if 'high' is a descendent of 'low' within */
   /*   'count' generations. */
   if ( count == 0L )
      return no;
   else
   {
      word *d = (word*)low[ node_daughter ];
      word found = no;
      while ( ! ( ( d == 0L ) || found ) )
      {
         if ( ( d == high ) || F0035_descends_from(high, d, count - 1L)
             )
            found = yes;
         d = (word*)d[ node_sibling ];
      }
      return found;
   }
}

F0036_add_references ( sfrom, sto )
word *sfrom, *sto;
{
   /*  Adds the reference chain in 'sfrom' to that in 'sto'. */
   word *r1 = symbol_reflist + sto;
   while ( ! ( (*r1) == 0L ) )
      r1 = ((word*)(*r1)) + ref_link;
   (*r1) = sfrom[ symbol_reflist ];
   sfrom[ symbol_reflist ] = 0L;
}

/*  add a library def to the pu */
F0037_add_library_xdef ( symbol )
word *symbol;
{
   symbol[ symbol_pulink ] = pu_current[ pu_xdef ];
   pu_current[ pu_xdef ] = (word)symbol;
}

/*  adds a library ref node to the PU */
F0038_add_library_xref ( symbol )
word *symbol;
{
   word i; /* FOR loop variables */
   word *sym = F0025_getblk(size_symbol);
   word len = (symbol[symbol_name]) & symbol_start_mask; /* AC fix */
   word *name = F0023_vec_get(len); /* AC fix */
   for ( i = 0L ; i <= ( len - 1L ) ; i++ )
      name[ i ] = ((word*)symbol[ symbol_nameptr ])[ i ];
   sym[ symbol_name ] = symbol[ symbol_name ];
   sym[ symbol_nameptr ] = (word)name;
   sym[ symbol_pulink ] = pu_current[ pu_xref ];
   sym[ symbol_type ] = t_lib;
   sym[ symbol_reflist ] = 0L;
   pu_current[ pu_xref ] = (word)sym;
}

F0039_scan_pu_list ( pu )
word *pu;
{
   while ( ! ( pu == 0L ) )
   {
      if ( pu[ pu_required ] == 1L )
         F003A_setpunode ( pu );
      pu = (word*)(pu[ pu_link ]);
   }
}

F003A_setpunode ( pu )
word *pu;
{
   if ( pu[ pu_required ] == 2L )
      return;
   pu[ pu_required ] = 2L;
   /* **** this bit that coagulated section now removed */
   /*    // set all the pu's on conts */
   /*    $( let hunk = pu!pu.first */
   /*       until hunk = 0 do */
   /*       // for each hunk need to set */
   /*       // all the pus on the chain */
   /*       // if it a whole hunk then we have already set the pu */
   /*       $( unless hunk!hunk.clink = 0 do */
   /*          $( let parent = (hunk!hunk.hunkname)!hname.parent */
   /*             let clink = parent!hunk.clink */
   /*             let chunk = parent */
   /*             $( F003A.setpunode(chunk!hunk.pu) */
   /*                if clink = 0 then break */
   /*                chunk := clink!clink.hunk */
   /*                clink := clink!clink.link */
   /*             $) repeat */
   /*          $) */
   /*          hunk := hunk!hunk.nextpuhunk */
   /*       $) */
   /*    $) */
   /*  **** end of removal */
   F003B_scan_pu_xrefs ( pu );
}

/*  set pu node true if a good xref */
F003B_scan_pu_xrefs ( punode )
word *punode;
{
   word *xrefs = (word*)punode[ pu_xref ];
   while ( ! ( xrefs == 0L ) )
   {
      /*  lookup the reference to find it def */
      word *defsym = F0012_lookup(xrefs, t_lib);
      word *newpunode;
      /*  next line catches a ref that has become a def */
      if ( defsym == 0L )
         defsym = F0012_lookup(xrefs, t_sym);
      if ( ! ( defsym == 0L ) )
      {
         newpunode =(word*)( ((word*)defsym[ symbol_hunk ])[ hunk_pu]) ;
         F003A_setpunode ( newpunode );
      }
      xrefs = (word*)(xrefs[ symbol_pulink ]);
   }
}

/*  make hunkptr be the pointer to the first lib hunk */
/*  scan hunks see if they go to a good xref */
word F003C_scan_hunk_list ( hunkptr )
word *hunkptr;
{
   word count = 0L;
   word * stopptr = (word*)(*rootliste);

/* AC fix for Dale's libray scan problem */

   word * list = hunklist; 
   while (! (list == (word *) (*hunkptr))) 
   { 
     F003D_is_hunk_required(&list); 
     list = (word *)(list[hunk_link]); 
   } 

/* AC fix end */

   while ( ! ( (word*)(*hunkptr) == stopptr ) )
   {  
      word *nexthunk = (word*)(((word*)(*hunkptr))[ hunk_link ]);
      if ( F003D_is_hunk_required(hunkptr) )
      {
         /*  part of hunk is required */
         count = count + 1L;
         hunkptr = ((word*)(*hunkptr)) + hunk_link;
      }
      else
         /*  discard hunk from chain */
         (*hunkptr) = (word)nexthunk;
   }
   e_hunklist = hunkptr;
   return count;
}

word F003D_is_hunk_required ( hunkptr )
word *hunkptr;
{
   word * hunk = (word*)(*hunkptr);
   word *clinkptr = hunk + hunk_clink;
   word *nexthunk = (word*)hunk[ hunk_link ];
   word *hunkn = (word*)hunk[ hunk_hunkname ];
   /*  trundle around hunk until we find a node that is required */
   while ( ! ( ((word*)hunk[ hunk_pu ])[ pu_required ] == 2L ) )
   {  
      F003E_freehunk ( hunk );   /*  can do this as freehunk doesn't */
      /*  if no hunks left then free hunk and return false */
      if ( (*clinkptr) == 0L )
         return no;
      /*  otherewise trundle */
      hunk = (word*)(((word*)(*clinkptr))[ clink_hunk ]);
      clinkptr = ((word*)(*clinkptr)) + clink_link;
   }
   /*       F0024.vec.free(clink) */
   /*  must have a required hunk so set the result */
   /*  hunk points to a required hunk */
   /*  clinkptr points to a clink node  (clever) */
   (*hunkptr) = (word)hunk;
   hunk[ hunk_clink ] = (*clinkptr);   /*  incase not parent */
   clinkptr = hunk + hunk_clink; /* bug fix AC 19 Apr 85 */
   hunk[ hunk_link ] = (word)nexthunk;   /*  ditto */
   /*  reset hunkname parent pointer */
   if ( ! ( hunkn == 0L ) )
      hunkn[ hname_parent ] = (word)hunk;
   /*  now want to see if there are any continuautions to add */
   while ( ! ( (*clinkptr) == 0L ) )
   {
      word *clink = (word*)(*clinkptr);
      if ( ((word*)((word*)clink[ clink_hunk ])[ hunk_pu ])[ pu_required ] == 2L )
         /*  on to next link */
         clinkptr = clink + clink_link;
      else
      {
         /* delete the clink */
         (*clinkptr) = clink[ clink_link ];   /*  remove node */
         F003E_freehunk ( clink[ clink_hunk ] );
      }
   }
   /*          F0024.vec.free(clink) */
   return yes;
}

/*  if a resident then decrease count */
/*  or a continuation */
F003E_freehunk ( hunk )
word *hunk;
{
   word *hunkn = (word*)hunk[ hunk_hunkname ];
   word size = hunk[ hunk_size ];
   max_total_size = max_total_size - size;
   /*  rejecting a resident library decrement count */
   if ( hunk[ hunk_type ] == hunk_res )
      resident_hunk_count = resident_hunk_count - 1L;
   /*  a hunkname then decrement total size */
   if ( ! ( hunkn == 0L ) )
      hunkn[ hname_size ] = hunkn[ hname_size ] - size;
#ifdef SUN
   /*  add to chain to free later */
   hunk[ hunk_link ] = (word)freehunkchain;
   freehunkchain = hunk;
#endif
}

/*  tidy pu list i.e take out unused bits */
F003F_scan_pu_list_again ( punode )
word* punode;
{
   word *oldpunode = punode;
   punode = (word*)(*punode);
   while ( ! ( punode == 0L ) )
   {
      word *temp = (word*)punode[ pu_link ];
      if ( punode[ pu_required ] == 1L )
         error ( err_int_lib );
      if ( punode[ pu_required ] == 2L )
      {
         oldpunode[ pu_link ] = (word)punode;
         oldpunode = punode;
         punode[ pu_link ] = 0L;
         F0040_tidypunode ( punode );
      }
      else
         F0041_freepunode ( punode );
      punode = temp;
   }
}

/*  after doing this our hunk list should be ok */
/*  but the symbol table will be full of dross unless */
/*  we tidy it remember that the symbol table will */
/*  only have an entry of type t.sym or t.lib NOT both */
F0040_tidypunode ( punode )
word *punode;
{
   word *xrefs = (word*)punode[ pu_xref ];
   word *xdefs = (word*)punode[ pu_xdef ];
   while ( ! ( xdefs == 0L ) )
   {
      /*  change type of the xdefs */
      word *temp = xdefs;
      xdefs[ symbol_type ] = t_sym;
      xdefs = (word*)(xdefs[ symbol_pulink ]);
      temp[ symbol_pulink ] =  - 1L ;
   }
   while ( ! ( xrefs == 0L ) )
   {
      /*  change type of xrefs */
      word *temp = xrefs;
      word *xdef = F0012_lookup(xrefs, t_lib);
      if ( xdef != 0L )
      {
         xdef[ symbol_type ] = t_sym;
         /*  tidy an undefined lib ref */
         if ( ! ( F0016_symtype(xdef[ symbol_name ]) < 128L ) )
            xdef[ symbol_pulink ] =  - 1L ;
      }
      xrefs = (word*)xrefs[ symbol_pulink ];
      F0026_freesymbol ( temp );
   }
}

/*  this is difficult because we do not know whether the xref node */
/*  is pointed to by someone else */
/*  e.g. 1st library not used but references fred */
/*       2nd library is used and references fred */
/*  do a lookup to see if it is defined in a true pu */
F0041_freepunode ( punode )
word *punode;
{
   word *xrefs = (word*)punode[ pu_xref ];
   word* xdefs = (word*)punode[ pu_xdef ];
   while ( ! ( xdefs == 0L ) )
   {
      word *temp = xdefs;
      /*  this is ok as a multiple def does not get put on the chain */
      /*  also by deleting the symbol we shouldn't find it on a lookup */
      F0027_freereferences ( xdefs[ symbol_reflist ] );
      F0014_delete ( xdefs );
      xdefs = (word*)( temp[ symbol_pulink ]);
   }
   while ( ! ( xrefs == 0L ) )
   {
      word i; /* FOR loop variables */
      word *temp = xrefs;
      word *sym = F0012_lookup(xrefs, t_sym);
      for ( i = 1L ; i <= 2L ; i++ )
      {
         if ( ! ( sym == 0L ) )
         {
            word *ref = sym + symbol_reflist;
            if ( ! ( ref == 0L ) )
               F0042_delete_from_reflist ( ref, punode );
         }
         sym = F0012_lookup(xrefs, t_lib);
      }
      xrefs = (word*)xrefs[ symbol_pulink ];
      F0026_freesymbol ( temp );
   }
}

/*  function to delete all the refs to a duff pu */
/*  the duff hunks have not been freed yet */
F0042_delete_from_reflist ( list, pu )
word *list, *pu;
{
   word *ptr = list;
   while ( ! ( (*ptr) == 0L ) )
   {
      word *nextref = (word*)(((word*)(*ptr))[ ref_link ]);
      if ( ((word*)(((word*)(*ptr))[ ref_hunk ]))[ hunk_pu ] == (word)pu )
      {
         (*ptr) = (word)nextref;
         continue;
      }
      ptr = ((word*)(*ptr)) + ref_link;
   }
}

/*  do we really need the code that looks for different files ? */
/*  depending on number of overlay refs */
F0043_read_ovly_supervisor ( )
{
   word j; /* FOR loop variables */
   string file = "\007ovs-###";
   string v = (string)F0023_vec_get(30L);
   word n = ovly_count < 0L ? 20L : ( ( ovly_count + 20L ) / 20L ) * 20L
      ;
   word c;
   for ( j = file[ 0L ] ; j >= 1L ; j-- )
   {
      char cc = file[ j ];
      if ( cc == '#' )
      {
         cc = ( n % 10L ) + '0';
         n = n / 10L;
      }
      v[ j ] = cc;
   }
   v[ 0L ] = file[ 0L ];
   /*  Read the file and add to list */
   loading_ovly_supervisor = yes;
   {
      word *temp = 0L;
      word *tempe = (word*) &temp ;
      c = F0010_file_rd(v, 0L, 0L, 0L, 0L,  & max_total_size ,  & tempe
          , "\022overlay supervisor", 0L);
      (*tempe) = (word)hunklist;
      hunklist = temp;
   }
   loading_ovly_supervisor = no;
   /*   the overlay supervisor consists of 1 hunk */
   if ( c != 1L )
      error ( err_bad_ovs, v );
}
