/* ALINKRES : Automatically translated from BCPL to C */
/* **************************************************************/
/*	      ##	 ##	   ##	 ##  ########  ######	*/
/*	     ####	 ##	   ##	##   ########  #######	*/
/*	    ##	##	 ##	   ##  ##    ##        ##    ## */
/*	   ########	 ##	   ####      ######    ##    ## */
/*	  ##	  ##	 ##	   ## ##     ##        ##    ## */
/*	 ##	   ##	 ##	   ##  ##    ##        ##    ## */
/*	##	    ##	 ########  ##	##   ########  #######	*/
/*     ##	     ##  ########  ##	 ##  ########  ######	*/
/* ******************	Sept 12 1985	*********************** */

#include "ALINKHDR.h"

#ifdef DALE
#include "defaulthn.c"
#endif


/*  This first section contains all the routines that are */
/*   required to be resident (ie. Those that are called from */
/*   more than one of the other sections). */
start ( )
{
   word* argv [ ( argsupb ) + 1 ];   /*  Argument space */
   /*  First initialise everything, and read all the arguments. */
   F0000_initialise_and_read_arguments ( argv );
   /*  tidyup := tidy.up.and.stop */
   /*  Do the first pass */
   F002F_dop1 (  );
   /*  Output the map and cross reference, if required. */
   F0048_o_map_and_xref (  );
   /*  Open the output file, and perform the second pass. */
   if ( to_file != 0L )
   {
      tostream = F000B_open(to_file, "\002TO", no,yes);
      selectoutput ( tostream );
      F0052_dop2 (  );
      endwrite (  );
      tostream = 0L;
      selectoutput ( verstream );
   }
   /*  Output final message and finish */
   writef ( "\067Linking complete - maximum code size = %n ($%X8) bytes\n", max_total_size
       << 2L , max_total_size << 2L);
   F000A_tidy_up_and_stop (  );
}

/*  END of start() */
/*  Some general utility routines, including error handling. */
error ( code, a1, a2, a3, a4 )
word code, a1, a2, a3, a4;
{
   selectoutput ( verstream );
   writes ( "\017\nLinker error: " );
   F0029_mess ( code, a1, a2, a3, a4 );
   writes ( "\025 - linking abandoned\n" );
   rcode = return_severe;
   F000A_tidy_up_and_stop (  );
}

warn ( code, a1, a2, a3, a4 )
word code, a1, a2, a3, a4;
{
   word o = output();
   selectoutput ( verstream );
   writes ( "\021\nLinker warning: " );
   F0029_mess ( code, a1, a2, a3, a4 );
   newline (  );
   selectoutput ( o );
   if ( rcode == 0L )
      rcode = return_soft;
}

F000A_tidy_up_and_stop ( )
{
   /*  This routine tidies up everything, and calls 'stop' */
   while ( ! ( vector_chain == 0L ) )
   {
      word* v = vector_chain;
      vector_chain = (word*)(*v);
      freevec ( v );
   }
   if ( fromstream != 0L )
   {
      selectinput ( fromstream );
      endread (  );
   }
   if ( tostream != 0L )
   {
      selectoutput ( tostream );
      endwrite (  );
   }
   if ( ( verstream != 0L ) && ( verstream != sysprint ) )
   {
      selectoutput ( verstream );
      endwrite (  );
   }
   if ( withstream != 0L )
   {
      selectinput ( withstream );
      endread (  );
   }
   if ( ( mapstream != 0L ) && ( mapstream != sysprint ) )
   {
      selectoutput ( mapstream );
      endwrite (  );
   }
   if ( ( xrefstream != 0L ) && ( xrefstream != sysprint ) )
   {
      selectoutput ( xrefstream );
      endwrite (  );
   }
   stop ( rcode );
}

word F000B_open ( file, name, input ,rawflag)
word input, rawflag;
string name, file;
{
   /*  Opens file 'file', for input or output, depending on the */
   /*	third parameter. 'name' is used in the error message. */
   word s = input ? findinput(file,rawflag) : findoutput(file,rawflag);
   if ( s == 0L )
      error ( err_open, name, file );
   return s;
}

/*  This group of routines is concerned with reading binary files. */
/*  The routines are called from both passes. */
F000C_read_descendents ( tree, ov_level, total_size, number )
word *tree, ov_level, total_size, number;
{
   /*  This routine reads all the 'daughters' of 'tree'. */
   /*  The maximum size is computed, and, if in pass2, output */
   /*	file positions are remembered, and 'hunk.break's are output. */
   word* d = (word*)tree[ node_daughter ];
   if ( d == 0L )
   {
      /*  'tree' is a leaf - check size. */
      if ( total_size > max_total_size )
	 max_total_size = total_size;
   }
   else
   {
      word ord = 1L;
      while ( ! ( d == 0L ) )
      {
	 word count = 0L;
	 word size = total_size;
	 word* hptr = e_hunklist;
	 count = F000D_files_rd(d[ node_files ], "\007overlay", ov_level
	    , ord, d,  & size ,  & e_hunklist , number);
	 d[ node_hunks ] = (*hptr);
	 d[ node_count ] = count;
	 F000C_read_descendents ( d, ov_level + 1L, size, count + number
	     );
	 ord = ord + 1L;
	 d = (word*) d[ node_sibling ];
      }
   }
}

word F000D_files_rd ( files, name, ov_level, ov_ord, files_node, lv_size_total, lv_list_end, number )
word *files, *name, ov_level, ov_ord, *files_node, *lv_size_total, *lv_list_end, number;
{
   word n = 0L;
   while ( ! ( files == 0L ) )
   {
      n = n + F0010_file_rd(files + 1L, ov_level, ov_ord, files_node, no
	 , lv_size_total, lv_list_end, name, number + n);
      files =(word*) (*files);
   }
   return n;
}

/*  only used on pass1 NOW          */

word F000E_handle_hunk_name ( )
{
   word i; /* FOR loop variables */
   word namesize = getword();
   word hunktype;
   word type;
   word memtype;
#ifdef DALE
	if (romlink)
	{
		char *p;
		for (i = 0L ; i <= ( namesize - 1L ); i++)
			getword();	/* just loose it */
		namesize = 2;
		hunkname = F0023_vec_get(namesize);
		p = (char *)hunkname;
		strcpy(p,"romhunk");
	}
	else
	{
   		hunkname = F0023_vec_get(namesize);
   		for ( i = 0L ; i <= ( namesize - 1L ) ; i++ )
      		hunkname[ i ] = getword();
	}
#else
   hunkname = F0023_vec_get(namesize);
   for ( i = 0L ; i <= ( namesize - 1L ) ; i++ )
      hunkname[ i ] = getword();
#endif
   hunktype = getword();   /*  get type */
   memtype = hunktype & memmask;
   myresult2 = hunktype;

   /* check for the illeagal memory case */

   if (memtype == memmask) return t_err ; 
   switch ( (int)(hunktype & typemask) )
   {
   case (int) hunk_bss:
      type = t_bss | memtype;
      break;
   case (int) hunk_code:
      type = t_code | memtype;
      break;
   case (int) hunk_data:
      type = t_data | memtype ;
      break;
   case (int) hunk_res:
      type = t_res;
      break;
   default: return t_err ;
      break;
   }
   if ( type == t_err ) 
      return t_err;
   {
      word* s = F0025_getblk(size_hname);
      s[ hname_len ] = namesize;
      s[ hname_ptr ] = (word)hunkname;
      s[ hname_type ] = type;
      s[ hname_zero ] = 0L;
      {
	 word* t = F0012_lookup(s, type);
	 if ( t == 0L )
	 {
	    /*	new name */
	    s[ hname_base ] = 0L;
	    /*		 hname.parent ! s	     is filled in later */
	    s[ hname_zero ] = 0L;
	    s[ hname_size ] = 0L;
	    s[ hname_link ] = 0L;
	    F0013_insert ( s );   /*  add to symbol table */
	    hunkname = s;
	 }
	 else
	 {
	    /*	old hunkname */
	    F0026_freesymbol ( s );   /*  free space */
	    hunkname = t;   /*	ptr to hunkname */
	    conthunk = yes;
	 }
      }
   }
   return type == t_res ? type : hunktype;
}   /*	return type */

F000F_read_resident_library ( file, library )
word file, library;
{
   /*  This routine, only called in pass1, reads the library */
   /*	files which will be resident when the program is loaded. */
   /*	constructs a library hunk and errors if two defs of */
   /*	same library given */
   word h,*hp;
   if ( conthunk )
   {
      warn ( err_res_lib, hunkname, file );
      h = getword();
      if ( ! ( h == hunk_ext ) )
	 error ( err_object, h, file );
      do
      {
	 /*  skip the info so as not to mess symbol table */
	 word w = getword();
	 word n = 0L;
	 if ( w == 0L )
	    break;
	 switch ((int) F0016_symtype(w) )
	 {
	 case (int) ext_abs:
	 case (int) ext_res:
	 case (int) ext_def:
	    F001F_discard_words ( ( w & symbol_start_mask ) + 1L );
	    break;
	 case (int) ext_common:
	    n = 1L;   /*  as common has 'extra size slot' */
	 case (int) ext_ref32:
	 case (int) ext_ref16:
	 case (int) ext_ref8:
	    F001F_discard_words ( ( w & symbol_start_mask ) + n );
	    F001F_discard_words ( getword() );
	    break;
	 }
      } while ( true );
      return;
   }
   hp = F0033_pass1_read_hunk(hunk_res, 0L, 0L, 0L, file, library, 0L);
   resident_hunk_count = resident_hunk_count + 1L;
   /*  now add hunk to list */
   hp[ hunk_link ] = (*e_completelist);   /*  new link points to rootlist */
   (*e_completelist) = (word)hp;   /*  old link points to hp */
   e_completelist = hp + hunk_link;
}   /*	address off last res link */

word F0010_file_rd ( file, ov_level, ov_ord, files_node, library, lv_size_total, lv_list_end, name, number )
word  ov_level, ov_ord, files_node, library, *lv_size_total, *lv_list_end, name, number;
string file;
{
   /*  This routine reads all the hunks in a file.  If 'library' is set */
   /*	in the first pass, or 'libtab' is non-zero in the second, then */
   /*	some or all of the hunks will be discarded.  The routine returns */
   /*	either the number of relocatable hunks read, if 'library' is */
   /*	false, or the number of relocatable AND absolute hunks required */
   /*	otherwise. */
   /*  Note the distinctions between the following variables: */
   /*	 hunk.number:	The number inserted into the hunk block in the list. */
   /*	 hunk.count:	The total number of hunks read so far. */
   /*	 hunks.used:	The number of both kinds of hunks actually used */
   /*			 so far.  This will be different from 'hunk.count' */
   /*			 for library files. */
   word size;
   word list = 0L;
/*   word* liste =  & (word)list ; */
   word* liste = &list ; 
   word hunk_count = 0L;
   word hunk_read = no;
   word pufound = no;
   word pucount =  - 1L ;
   word* oldhunk = 0L;
   filemark = 0L;   /*	so can pointwords into a file on pass2 */
   simplepoint = 1L;
   pos_in_pu =	- 1L ;
   fromstream = F000B_open(file, name, yes, yes);
   from_file = file;
   selectinput ( fromstream );
   F001E_replenish_input (  );
   /*  Give a warning message for an empty file */
   if ( pass1 && F001C_exhausted() )
      warn ( err_empty2, name, file );
   /*  Read through the hunks in the file */
   while ( ! (F001C_exhausted()==1L) )
   {
      word type = getword();
      hunkname = 0L;   /*  usefull flags */
      conthunk = no;
      switch ( (int)(type & typemask) )
      {
      default:
	 error ( err_object, type, file );
      /*  Handle program unit (TJK 20Feb85) */
      case (int) hunk_unit:
	 pucount = pucount + 1L;
	 F0028_handle_pu ( pucount, library );
	 pos_in_pu =  - 1L ;
	 hunk_read = no;
	 pufound = yes;
	 break;
      case (int) hunk_name:
	 type = F000E_handle_hunk_name();
      case (int) hunk_code:
      case (int) hunk_data:
      case (int) hunk_bss:
#ifdef DALE
	if (romlink)
		if (hunkname == 0) type = default_hunkname(type);
#endif
	 if ( ! pufound )
	    error ( err_no_pu, file );
	 if ((type & memmask) == memmask) 
	 { myresult2 = type;
	   type = t_err;
	 }
	 if ( type == t_err )
	    error ( err_object, myresult2, file );
	 if ( type == t_res )
	 {
	    F000F_read_resident_library ( file, library );
	    continue;
	 }
	 pos_in_pu = pos_in_pu + 1L;
	 if ( ! conthunk )
	    hunk_count = hunk_count + 1L;
	 /*  These are only allowed as the first actual */
	 /*   hunk in a collection. */
	 if ( hunk_read )
	    error ( err_no_end, file );
	 size = getword();
	 /*  as we record the size in the hunk node we might as well mark after it */
	 mark = filemark;
	 (*lv_size_total) = (*lv_size_total) + size;
	 /*  increment/set counters if a continuation to a hunk */
	 if ( hunkname != 0L )
	 {
	    hunkname[ hname_base ] = hunkname[ hname_size ];
	    hunkname[ hname_size ] = hunkname[ hname_size ] + size;
	 }
	 {
	    /*	Read the hunk. */
	    /*	Note also that the number of a new relocatable */
	    /*	hunk will be 'hunk.number+1', while for a */
	    /*	cont block it is its parents number */
	    word* h = F0033_pass1_read_hunk(type, size, ov_level, ov_ord
	       , files_node, library, number);
	    /*	Insert the new hunk block into the list. */
	    /*	Record the position of the hunk within the */
	    /*	 file in the 'gnum' field */
	    /*	only add real hunks not continuations */
	    if ( ! conthunk )
	    {
	       (*liste) =(word) h;
	       liste = hunk_link + h;
	       h[ hunk_gnum ] = hunk_count;
	       number = number + 1L;
	    }
	    if ( pos_in_pu == 0L )
	       pu_current[ pu_first ] = (word)h;
	    else
	       oldhunk[ hunk_nextpuhunk ] = (word)h;
	    oldhunk = h;
	 }
	 hunk_read = yes;
	 break;
      case (int) hunk_end:
	 hunk_read = no;
	 break;
      }
   }
   /*  Check that the file was terminated by a 'hunk.end' */
   if ( hunk_read )
      error ( err_bad_end, file );
   if ( list != 0L )
   {
      /*  Insert into parameter hunk list */
      /*   and compute maximum level. */
      if ( max_level < ov_level )
	 max_level = ov_level;
      (*liste) = (*((word*)(*lv_list_end)));
      (*((word*)(*lv_list_end))) = (word)list;
      (*lv_list_end) = (word)liste;
   }
   /*  Tidy up the input */
   endread (  );
   fromstream = 0L;
   return hunk_count;
}

/*  This routine, called in both passes, read a 'hunk.ext' block */
word* F0011_read_extblock ( library, asymbols, bytebase )
word library, *asymbols, bytebase;
{
   /*  This is called after reading a 'hunk.ext'.  It reads all the */
   /*	symbols, delivering a chain of them.  If in pass2, then */
   /*	only the references are read.  If 'library' is true, then */
   /*  the symbols are given the type t.lib  as opposed to t.sym */
   word chain = 0L;
   word* chaine =  & chain ;
   word w1 = getword();
   /*  Loop through all the symbols in the block */
   while ( w1 != 0L )
   {
      word namesize = w1 & symbol_start_mask;
      word* name = F0023_vec_get(namesize);   /*  space for name */
      word value;
      word t = F0016_symtype(w1);
      /*  Read the rest of the name */
      F001D_getwrds ( name, namesize );
      /*  and the value...... */
      value = getword();
      if ( pass2 && ( t == ext_common ) )
      {
	 /*  Treat as simple references */
	 value = getword();
	 t = ext_ref32;
      }
      /*  Only return references in pass2. */
      if ( ( pass1 || ( t >= 128L ) ) || loading_ovly_supervisor )
      {
	 word* s = F0025_getblk(size_symbol);
	 /*  Initialise the symbol fields */
	 s[ symbol_name ] = w1;
	 s[ symbol_nameptr ] = (word)name;
	 (*chaine) = (word)s;
	 chaine = s + symbol_link;
	 (*chaine) = 0L;
	 s[ symbol_value ] = 0L;
	 s[ symbol_overlaynumber ] =  - 1L ;
	 s[ symbol_reflist ] = 0L;
	 s[ symbol_type ] = library ? t_lib : t_sym;
	 switch ((int) t )
	 {
	 case (int) ext_common:
	    s[ symbol_value ] = value;
	    value = getword();
	 case (int) ext_ref32:
	 case (int) ext_ref16:
	 case (int) ext_ref8:
	    /*	Only keep references if they will be needed. */
	    if ( ( pass2 || xrefing ) || ( overlaying && ( t != ext_common
		) ) )
	    {
	       word j; /* FOR loop variables */
	       word* refs = 0L;
	       for ( j = 1L ; j <= value ; j++ )
	       {
		  word* r = F0025_getblk(size_ref);
		  word offset = getword();
		  /*  bytebase is only used in pass2 */
		  r[ ref_offset ] = offset + bytebase;
		  r[ ref_link ] = (word)refs;
		  r[ ref_type ] = t;
		  refs = r;
	       }
	       s[ symbol_reflist ] = (word)refs;
	    }
	    else
	       F001F_discard_words ( value );
	    break;
	 case (int) ext_res:
	 case (int) ext_abs:
	 case (int) ext_def:
	    s[ symbol_value ] = value;
	    break;
	 default:
	    error ( err_object2, t, s, from_file );
	 }
      }
      w1 = getword();
   }
   while ( ! ( (*asymbols) == 0L ) )
      asymbols = (word*)(*asymbols);
   (*asymbols) = chain;
   return (word*)chain;
}

/*  The following routines are used to manage the symbol table. */
/*  Look up s in table return 0 if not there */
word* F0012_lookup ( s, type )
word *s, type;
{
   word* t = (word*)symbol_table[ F0015_hashval(s) ];
   word len_s = s[ symbol_name ] & symbol_start_mask;	/*  length of name  words */
   while ( ! ( t == 0L ) )
   {
      word len_t = t[ symbol_name ] & symbol_start_mask;
      word same = yes;
      if ( ! ( type == t[ symbol_type ] ) )
	 goto fail;
      if ( ! ( len_t == len_s ) )
	 goto fail;
      {
	 word i; /* FOR loop variables */
	 word* name_t = (word*)t[ symbol_nameptr ];
	 word* name_s = (word*)s[ symbol_nameptr ];
	 for ( i = len_t - 1L ; i >= 0L ; i-- )
	    if ( name_t[ i ] != name_s[ i ] )
	    {
	       same = no;
	       break;
	    }
      }
      if ( same )
	 return t;
   fail:
      t = (word*)(t[ symbol_link ]);
   }
   return 0L;
}

F0013_insert ( s )
word* s;
{
   word* a = symbol_table + F0015_hashval(s);
   s[ symbol_link ] = (*a);
   (*a) = (word)s;
}

F0014_delete ( s )
word* s;
{
   word* p = symbol_table + F0015_hashval(s);
   while ( ! ( (*p) == 0L ) )
   {
      if ( (*p) ==(word) s )
      {
	 (*p) = s[ symbol_link ];
	 s[ symbol_link ] = 0L;
	 return;
      }
      p = (word*)((word*)(*p) + symbol_link);
   }
}

/* s ->     ============ value */
/*	    ------------ hunk */
/*	    ------------ reflist */
/*	    ------------ overlay number */
/*	    ------------ link */
/*	    ------------ name	  =   I symtype + length I */
/*	    ------------ nameptr ->   I FRED padded to longword boundary I */
/*	    */
word F0015_hashval ( s )
word* s;
{
   word i; /* FOR loop variables */
   word len = s[ symbol_name ] & symbol_start_mask;
   word hash = len;   /*  symbol.type ! s */
   /*  dont use symbol type so we can */
   /*  do a library fiddle */
   word *name = (word*)s[ symbol_nameptr ];
   for ( i = 0L ; i <= ( len - 1L ) ; i++ )
      hash = hash ^ name[ i ];
   return (word) abs( hash % symbol_table_size );
}

word F0016_symtype ( w )
#ifdef SUN
unsigned word w;
{
    return w >> first_byte_shift;
}
#endif

#ifdef IBM
word w;
{ return ((w >> first_byte_shift) & 0x000000FFL);
}
#endif

F0017_scan ( f, a1, a2, a3 )
word (*f)(), a1, a2, a3;
{
   word j; /* FOR loop variables */
   /*  Calls f with arguments (symbol, a1, a2, a3) for every symbol */
   /*	in the table, followed by a call with a zero first parameter. */
   for ( j = 0L ; j <= symbol_table_upb ; j++ )
   {
      word* s = (word*)symbol_table[ j ];
      while ( ! ( s == 0L ) )
      {
	 if ( s[ symbol_type ] == t_sym )
	    (*f) ( s, a1, a2, a3 );
	 s = (word*)(s[ symbol_link ]);
      }
   }
   (*f) ( 0L, a1, a2, a3 );
}

/*  fiddle for now */
writeoct ( s, n )
word *s;
int n;
{
   word  i; /* FOR loop variables */
   word len = s[ symbol_name ] & symbol_start_mask;
   string name = (string)s[ symbol_nameptr ];
   word maxlen = len;
   if ( ! ( n == 1L ) )
   { len = (len > 8L) ? 8L : len%9L;
     maxlen = 8;
   }
   { word letters = (len << 2) -1L;
     word spaces  = ((maxlen - len) << 2);
     for ( i = 0L ; i <= letters ; i++ )
        wrch ( name[ i ] == 0L ? ' ' : name[ i ] );
     for ( i = 1L ; i <= spaces ; i++ )
        wrch ( ' ' );
    }
}

/*  The  routines used for binary input and output */
word getword ( )
{
   i_ptr = i_ptr + 1L;
   filemark = filemark + 1L;
   if ( i_ptr < i_end )
#ifdef SUN
      return i_buffer[ i_ptr ];
#endif

#ifdef IBM
      return swap(&(i_buffer[ i_ptr ]));
#endif

   /*  Buffer empty - try to refill */
   if (  ! F001E_replenish_input()  )
      error ( err_finish1, from_file );
   i_ptr = 0L;
#ifdef SUN
   return i_buffer[ 0L ];
#endif

#ifdef IBM
   return swap(&(i_buffer[ 0L ]));
#endif
}

word F001A_getoptword ( )
{
   /*  Gets the next input word, if there was one, returning */
   /*	zero if file is exhausted.  This routine is only used */
   /*	to read object types, and so it is safe to return zero. */
   i_ptr = i_ptr + 1L;
   filemark = filemark + 1L;
   if ( i_ptr < i_end )
#ifdef SUN
      return i_buffer[ i_ptr ];
#endif

#ifdef IBM
      return swap(&(i_buffer[ i_ptr ]));
#endif

   /*  Buffer empty - can it be refilled? */
   if (  ! F001E_replenish_input()  )
      i_buffer[ 0L ] = 0L;

   i_ptr = 0L;
#ifdef SUN
   return i_buffer[ 0L ];
#endif

#ifdef IBM
   return swap(&(i_buffer[ 0L ]));
#endif
}

F001B_ungetword ( )
{
   i_ptr = i_ptr - 1L;
   filemark = filemark - 1L;
}

word F001C_exhausted ( )
{
   if ( ( i_ptr + 1L ) < i_end )
      return no;
   return  (F001E_replenish_input()==1L ? 0L : 1L) ;
}

F001D_getwrds ( v, n )
word *v, n;
{
   word j; /* FOR loop variables */
   word got = ( i_end - i_ptr ) - 1L;
   filemark = filemark + n;
   if ( got > n )
      got = n;
   for ( j = 0L ; j <= ( got - 1L ) ; j++ )
      v[ j ] = i_buffer[ ( i_ptr + j ) + 1L ];
   i_ptr = i_ptr + got;
   if ( got < n )
   {
      word left = n - got;
#ifdef SUN
      if ( (word) abs( readwords(v + got, left))  != left )
#endif

#ifdef IBM
      if (  readwords(v + got, left)  != left )
#endif
	 error ( err_finish2, from_file, n );
   }
}

word F001E_replenish_input ( )
{
   i_ptr =  - 1L ;
   /*  IF testflags(1) THEN */
   /*	error(err.break) */
#ifdef SUN
   i_end = (word) abs( readwords(i_buffer, i_buffer_size) );
#endif

#ifdef IBM
   i_end =  readwords(i_buffer, i_buffer_size) ;
#endif
   return (word)(i_end > 0L);
}

F001F_discard_words ( n )
word n;
{
 mypointword(filemark+n) ;
}

F0020_putword ( w )
word w;
{
   if ( ( o_ptr + 1L ) == o_buffer_size )
      F0022_deplete_output (  );
   o_ptr = o_ptr + 1L;
#ifdef IBM
   w = swap(&w);
#endif
   o_buffer[ o_ptr ] = w;
}

F0021_putwrds ( v, n )
word *v, n;
{
   word left = ( o_buffer_size - o_ptr ) - 1L;
   if ( n > left )
   {
      F0022_deplete_output (  );
      writewords ( v, n );
   }
   else
   {
      word j; /* FOR loop variables */
      for ( j = 0L ; j <= ( n - 1L ) ; j++ )
	 o_buffer[ ( o_ptr + j ) + 1L ] = v[ j ];
      o_ptr = o_ptr + n;
   }
}

F0022_deplete_output ( )
{
   if ( o_ptr >= 0L )
      writewords ( o_buffer, o_ptr + 1L );
   o_ptr =  - 1L ;
}

/*  These are the storage management routines. */
word* F0023_vec_get ( size )
word size;
{
   word* v;
   if (dummyfree == 1L) return (word *)(F0025_getblk(size));
   v= getvec(size);
   if ( v == 0L )
      error ( err_no_store );
   (*v) = (word)vector_chain;
   vector_chain = v;
   return v + 1L;
}

word* vec_get1 ( size )
word size;
{
   word* v;
   v= getvec(size);
   if ( v == 0L )
      error ( err_no_store );
   (*v) = (word)vector_chain;
   vector_chain = v;
   return v + 1L;
}

F0024_vec_free ( v )
word* v;
{
   word* c = (word*)&vector_chain ;
   if (dummyfree == 1L) return;
   v = v - 1L;
   while ( ! ( (*c) == 0L ) )
   {
      if ( (*c) ==(word) v )
      {
	 (*c) = (*v);
	 freevec ( v );
	 return;
      }
      c = (word*)(*c);
   }
   error ( err_int_free );
}


word* F0025_getblk ( size )
word size;
{
   if ( ( size == size_ref ) && ( free_reference_chain != 0L ) )
   {
      word* result = free_reference_chain;
      free_reference_chain = (word*)(result[ ref_link ]);
      return result;
   }
   if ( ( size == size_symbol ) && ( free_symbol_chain != 0L ) )
   {
      word* result = free_symbol_chain;
      free_symbol_chain = (word*)(result[ symbol_link ]);
      return result;
   }
   {
      word h = heapptr;
      word nh = heapptr + size;
      if ( nh > work_vector_size )
      {
	 /*  Current vector is exhausted */
	 if ( size > work_vector_size )
	    return vec_get1(size);
	 work_vector = vec_get1(work_vector_size);
	 heapptr = size;
	 return work_vector;
      }
      heapptr = nh;
      return (word*)(h + work_vector);
   }
}

F0026_freesymbol ( s )
word* s;
{
   F0027_freereferences ( s[ symbol_reflist ] );
   F0024_vec_free(s[symbol_nameptr]);
   s[ symbol_link ] = (word)free_symbol_chain;
   free_symbol_chain = s;
}

F0027_freereferences ( r )
word* r;
{
   if ( r != 0L )
   {
      word* r1 = r;
      while ( ! ( r[ ref_link ] == 0L ) )
	 r = (word*)( r[ ref_link ]);
      r[ ref_link ] =(word) free_reference_chain;
      free_reference_chain = r1;
   }
}

/*  needs globals pulist = 0 initially */
/*  pu.current = @pulist initially */
/*  need to put the address of current pu into the current hunk */
/*  library = true if a libray */
F0028_handle_pu ( count, library )
word count, library;
{
   word len = getword();   /*  length of name */
   word* name = 0L;
   if ( ! ( len == 0L ) )
   {
      name = F0023_vec_get(len);
      F001D_getwrds ( name, len );
   }
   {   /*  read name */
      word* temp = pu_current;
      pu_current = F0025_getblk(size_pu);
      temp[ pu_link ] = (word)pu_current;
   }
   /*  ****** a pu node ******* */
   /*	 pu.current!pu.first	   // ptr to first hunk of a pu - set later */
   pu_current[ pu_link ] = 0L;
   pu_current[ pu_pif ] = count;   /*	 pos in file */
   pu_current[ pu_xdef ] = 0L;	 /*    list of xrefs */
   pu_current[ pu_xref ] = 0L;	 /*    list of xdefs */
   pu_current[ pu_namelen ] = len;   /*    length of name */
   pu_current[ pu_name ] =(word) name;	 /*    name */
   pu_current[ pu_required ] = library ? 0L : 2L;
}

/*  *********************** */
/*  generalised error / warning message writer */
F0029_mess ( code, arg1, arg2, arg3, arg4 )
word code, arg1, arg2, arg3, arg4;
{
   /*  ** user errors ** */
   string s;
   word a1 = 0L;
   word a2 = 0L;
   word a3 = 0L;
   word a4 = 0L;
#ifdef SUN
   word (*f1)() = 0L;
   word (*f2)() = 0L;
   word (*f3)() = 0L;
   word (*f4)() = 0L;
#endif

#ifdef IBM
   int (*f1)() = 0;
   int (*f2)() = 0;
   int (*f3)() = 0;
   int (*f4)() = 0;
#endif
   word f1a = 0L;
   word f2a = 0L;
   word f3a = 0L;
   word f4a = 0L;
   switch ( (int)code )
   {
   default:
      a1 = code;
      s = "\025Unknown error code %n";
      break;
   case (int) err_hunk_rel:   /*  arg1 = bitsize arg2 = thishunk arg3 = hunk | 0 */
      a1 = arg1;
      f1 = hunkinfo;
      f1a = arg2;
      if (arg3){ f2 = hunkinfo ;}
      f2a = arg3;
      s = "\111%n bit relocation only allowed between named sections \nwith the same name"
	 ;
      break;
   case (int) err_hunk_ref:   /*  arg1 = bitsize arg2 = l arg3 = ref */
      a1 = arg1;
      f1 = symbol_info;
      f1a = arg2;
      f2 = ref_info;
      f2a = arg3;
      s = "\111%n bit references only allowed between named sections \nwith the same name"
	 ;
      break;
   case (int) err_value_rel:   /*  arg1 = bitsize arg2 = thishunk arg3 = hunk  arg4 = value */
      a1 = arg1;
      a2 = arg4;
      f1 = hunkinfo;
      f1a = arg2;
      f2 = hunkinfo;
      f2a = arg3;
      s = "\065Value for %n bit relocation out of range. Value = %X8";
      break;
   case (int) err_value_ref:   /*  arg1 = bitsize arg2 = l arg3 = ref arg4 = value  */
      a1 = arg1;
      a2 = arg4;
      f1 = symbol_info;
      f1a = arg2;
      f2 = ref_info;
      f2a = arg3;
      s = "\063Value out of range for %n bit reference. Value = %X8";
      break;
   case (int) err_no_input:
   case (int) err_bad_args:
      s = "\143Usage is: alink [[from] <file>[[,|+]<file>]] [with <file>][library <file>[[+|,]<file>]] [to <file>]"
	 ;
      break;
   /*	 CASE err.no.input : s:= "no primary input specified" */
   /*			   ENDCASE */
   case (int) err_bad_with:
      a1 = arg1;
      a2 = arg2;
      s = "\044error in WITH file \"%S\" near line %N";
      break;
   case (int) err_open:
      a1 = arg1;
      a2 = arg2;
      s = "\027can\'t open %S file \"%S\"";
      break;
   case (int) err_break:
      a1 = pass2 ? (word)"\006second" : (word) "\005first";
      s = "\024BREAK during %S pass";
      break;
   case (int) err_empty1:
      a1 = arg1;
      s = "\035empty primary input file \"%S\"";
      break;
   case (int) err_no_pu:
      a1 = arg1;
      s = "\033No Program unit in file %S\n";
      break;
   case (int) err_object:
      a1 = arg1;
      a2 = arg2;
      s = "\044invalid object type %X8 in file \"%S\"";
      break;


   case (int) err_ovs_ref1:
      a1 =arg1;
      a2 =((word *)arg2)[hunk_file];
      a3 =((word *)arg3)[hunk_file];
      s = "\124invalid overlay reference of symbol %O1 \n              ref in file %s def in file %s";
      break;


   /*  *** compiler / assembler errors *** */
   case (int) err_ovs_sym:   /*  "invalid use of overlay symbol %o1",symbols */
   case (int) err_sym_use:   /*  "invalid use of symbol *"%O1*" in file *"%S*"", */
   case (int) err_com_use:   /*  "invalid use of common *"%O1*" * in file etc */
   case (int) err_ovs_ref2:   /*  "non-zero overlay reference in file *"%S*"", */
   case (int) err_bss_rel1:   /*  "invalid external block relocation in file *"%S*"", from.file */
   case (int) err_bss_rel2:   /*  "invalid bss relocation in file *"%S*"", from.file */
   case (int) err_pu_rel:   /*	"bad pu relocation" */
   case (int) err_off_rel1:   /*  "32reloc bad offset" */
   case (int) err_off_rel2:   /*  "doreloc bad offset" */
   case (int) err_off_ref1:   /*  "32ref %o2 bad offset o = %X8",symbol,o */
   case (int) err_off_ref2:   /*  "ref bad offset" */
   case (int) err_ended:   /*  "file *"%S*" ended while discarding %N words", from.file, n */
   case (int) err_no_end:   /*	"end block missing in file *"%S*"", file */
   case (int) err_bad_end:   /*  "file *"%S*" terminated invalidly", file */
   case (int) err_finish1:   /*  "file *"%S*" ended prematurely", from.file */
   case (int) err_finish2:   /*  "file *"%S*" ended during input of %N words", from.file, n */
      a1 = code;
      s = "\035Invalid Object Module code %n";
      break;
   /*  ***   internal errors  shouldn't happen my fault */
   case (int) err_int_hunk:   /*  "bad type %X8 in hunklist",type */
   case (int) err_int_lib:   /*  "internal error in lib scan" */
   case (int) err_int_free:   /*  "invalid argument for vec.free during %S", pass1 -> "pass1", */
   case (int) err_int_bug:   /*  "bug - symbol *"%O1*" not defined in pass2*N", s */
      a1 = code;
      s = "\021Internal Error %n";
      break;
   /*	other internal errors that might occur */
   case (int) err_no_store:
      a1 = pass1 ? (word)"\005pass1" : pass2 ? (word)"\005pass2" : (word)"\016initialisation"
	 ;
      s = "\034insufficient store during %S";
      break;
   case (int) err_bad_ovs:
      a1 = arg1;
      s = "\047overlay supervisor file \"%S\" is invalid";
      break;
   /*  warnings usually */
   case (int) err_res_lib:
      a1 = arg1;
      a2 = arg2;
      s = "\055Resident Library %o1 defined again in file %s";
      break;
   case (int) err_empty2:
      a1 = arg1;
      a2 = arg2;
      s = "\025%S file \"%S\" is empty";
      break;
   case (int) err_ovs_ref3:
      s = "\033no overlay references found";
      break;
   case (int) err_mul_def:
      f1 = symbol_info;
      f1a = arg1;
      f2 = symbol_info;
      f2a = arg2;
      s = "\035multiple definition of symbol";
      break;
   case (int) err_map:
      a1 = arg1;
      s = "\050can\'t open MAP file \"%S\" - map abandoned";
      break;
   case (int) err_xref:
      a1 = arg1;
      s = "\065can\'t open XREF file \"%S\" - cross reference abandoned"
	 ;
      break;
   case (int) err_readnum1:
      a1 = arg1;
      a2 = arg2;
      a3 = arg3;
      s = "\060invalid %S value found near line %N in file \"%S\"";
      break;
   case (int) err_readnum2:
      a1 = arg1;
      a2 = arg2;
      s = "\025invalid %S value (%S)";
      break;
   case (int) err_object2:
      a1 = arg1;
      a2 = arg2;
      a3 = arg3;
      s = "\047invalid type %X8 for \"%O1\" in file \"%S\"";
      break;
   }
   writef ( s, a1, a2, a3, a4 );
   newline (  );
   if ( ! ( f1 == 0L ) )
      { (*f1) ( f1a );
	newline(); };
   if ( ! ( f2 == 0L ) )
      { (*f2) ( f2a );
	newline(); };
   if ( ! ( f3 == 0L ) )
      { (*f3) ( f3a );
	newline(); };
   if ( ! ( f4 == 0L ) )
      { (*f4) ( f4a );
	newline(); };
}

symbol_info ( symbol )
word* symbol;
{
   word type = F0016_symtype(symbol[ symbol_name ]);
   word* hunk =(word*) symbol[ symbol_hunk ];
   word value = symbol[ symbol_value ];
   newline (  );
   writef ( "\015symbol = %o1\n", symbol );
   writetype ( type );
   writef ( "\015value  = %X8\n", value );
   if ( ! ( hunk == 0L ) )
      hunkinfo ( hunk );
}

ref_info ( ref )
word* ref;
{
   word type = ref[ ref_type ];
   word* hunk =(word*) ref[ ref_hunk ];
   word offset = ref[ ref_offset ];
   writetype ( type );
   writef ( "\014offset = %n\n", offset );
   if ( ! ( hunk == 0L ) )
      hunkinfo ( hunk );
}

writetype ( type )
word type;
{
   string s;
   switch ( (int)type )
   {
   default:
      s = "\007Unknown";
      break;
   case (int) ext_def:
      s = "\026Relocatable definition";
      break;
   case (int) ext_abs:
      s = "\023Absolute definition";
      break;
   case (int) ext_res:
      s = "\033Resident library definition";
      break;
   case (int) ext_ref32:
      s = "\02032 bit reference";
      break;
   case (int) ext_ref16:
      s = "\02016 bit reference";
      break;
   case (int) ext_ref8:
      s = "\0178 bit reference";
      break;
   case (int) ext_common:
      s = "\020Common reference";
      break;
   }
   writef ( "\011Type: %s\n", s );
}

hunkinfo ( hunk )
word* hunk;
{
   word size = hunk[ hunk_size ] << 2L;
   word base = hunk[ hunk_base ] << 2L;
   word num = hunk[ hunk_num ];
   word* hname =(word*) hunk[ hunk_hunkname ];
   word* pu = (word*)hunk[ hunk_pu ];
   word file = hunk[ hunk_file ];
   writes ( "\023Program unit name: " );
   F002E_writepuname ( pu );
   if ( hname == 0L )
      writef ( "\020No section name\n" );
   else
      writef ( "\014Section %o1\n", hname );
   writef ( "\011File: %s\n", file );
   writef ( "\066Hunk Number: %n\nBase = %X8 (bytes)\nsize = %X8 (bytes)\n", num
      , base, size );
}

F002E_writepuname ( pu )
word* pu;
{
   word len = pu[ pu_namelen ];
   char* name = (char*)pu[ pu_name ];
   if ( len == 0L )
      writes ( "\004None" );
   else
   {
      word i; /* FOR loop variables */
      for ( i = 0L ; i <= ( len - 1L ) ; i++ )
	 wrch ( name[ i ] );
   }
   wrch ( '\n' );
}


union u {
    long    a;
    char    b[4];
};

word swap(val)
    union u *val;
{
    union u temp;
    temp.b[0]=val->b[3];
    temp.b[1]=val->b[2];
    temp.b[2]=val->b[1];
    temp.b[3]=val->b[0];
    return((word)temp.a);
}
