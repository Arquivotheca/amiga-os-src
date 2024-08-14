/* ALINKINIT : Automatically translated from BCPL to C */ 
/* ***************************************************************/
/*            ##         ##        ##    ##  ########  ######    */
/*           ####        ##        ##   ##   ########  #######   */
/*          ##  ##       ##        ##  ##    ##        ##    ##  */
/*         ########      ##        ####      ######    ##    ##  */
/*        ##      ##     ##        ## ##     ##        ##    ##  */
/*       ##        ##    ##        ##  ##    ##        ##    ##  */
/*      ##          ##   ########  ##   ##   ########  #######   */
/*     ##            ##  ########  ##    ##  ########  ######    */
/* ******************* Sept 13 1985 **************************** */

#include "ALINKHDR.h"

/*  This section contains the routines for initialising all the */
/*   data structures, and for reading and decoding the parameters. */

F0000_initialise_and_read_arguments ( argv )

word* argv;
{
   /*  Initialises the data structures, and reads arguments, both */
   /*   from the command line, and from any WITH files specified. */
   /*  First do the initialisation. */
   F0001_initialise (  );
   /*  Now decode the command line parameters */
#ifdef DALE
   if ( rdargs("\115FROM=ROOT,TO/K,WITH/K,VER/K,LIBRARY=LIB/K,MAP/K,XREF/K,WIDTH/K,FASTER/S,ROM/S"
#else
   if ( rdargs("\107FROM=ROOT,TO/K,WITH/K,VER/K,LIBRARY=LIB/K,MAP/K,XREF/K,WIDTH/K,FASTER/S"
#endif
      , argv, argsupb) == 0L )
      error ( err_bad_args );
   /*  Check for VER parameter. */
   if ( argv[ args_ver ] != 0L )
      verstream = F000B_open(argv[ args_ver ], "\003VER", no, no);
   selectoutput ( verstream );
   /*  Write out the initial message */
   writes ( "\155Amiga Linker Version 3.30         \nCopyright (C) 1985 by Tenchstar Ltd., T/A Metacomco.\nAll rights reserved.\n"
       );
   /*  Now check for FROM parameter */
   if ( argv[ args_from ] != 0L )
   {
      root_files = F0003_makefilelist(argv[ args_from ]);
      root_given = yes;
   }
   /*  TO */
   to_file = (string) argv[ args_to ];
   /*  LIBRARY */
   if ( argv[ args_library ] != 0L )
   {  
      lib_files = F0003_makefilelist(argv[ args_library ]);
      lib_given = yes;
   }
   /*  MAP */
   map_file = (string)argv[ args_map ];
   /*  XREF */
   xref_file = (string)argv[ args_xref ];

#ifdef SUN
	dummyfree = 1L;
#else
   dummyfree = 0L;  

   if (argv[args_faster] != 0L) dummyfree = 1L; 
#endif
#ifdef DALE
   romlink = 0L;  
   if (argv[args_rom] != 0L) romlink = 1L; 
#endif
 
   /*  WIDTH */
   width_string = argv[ args_width ];
   if ( width_string != 0L )
      width = F0004_read_number(width_string, "\005WIDTH", default_width
         , yes);
   /*  Now read the WITH files. */
   if ( argv[ args_with ] != 0L )
   {
      word* fl = F0003_makefilelist(argv[ args_with ]);
      while ( fl != 0L )
      {
         F0002_read_with_file ( fl + 1L );
         fl = (word*)(*fl);
      }
   }
   /*  Ensure that ROOT has been given. */
   if (  ! root_given  )
      error ( err_no_input );
   /*  Set the flags from the parameter values */
   overlaying = ovly_tree[ node_daughter ] != 0L;
   mapping = ( mapstream != 0L ) | ( map_file != 0L );
   xrefing = ( xrefstream != 0L ) | ( xref_file != 0L );
   /*  Set the 'root' files into the tree */
   ovly_tree[ node_files ] = (word)root_files;
}

F0001_initialise ( )
{
   word j; /* FOR loop variables */
   /*  Initialises everything it can think of! */
   /*  First clear all the streams */
   sysprint = output();
   verstream = sysprint;
   fromstream = 0L;
   tostream = 0L;
   withstream = 0L;
   mapstream = 0L;
   xrefstream = 0L;
   /*  Now the parameters which may be set */
   root_files = 0L;
   lib_files = 0L;
   width = default_width;
   root_given = no;
   lib_given = no;
   ovly_given = no;
   vector_chain = 0L;
   rcode = 0L;
   heapptr = work_vector_size + 1L;
   free_reference_chain = 0L;
   free_symbol_chain = 0L;
   pass1 = no;
   pass2 = no;
   hunklist = 0L;
   e_hunklist =  (word*)(&hunklist) ;
   lib_count = 0L;
   resident_hunk_count = 0L;
   comm_count = 0L;
   max_total_size = 0L;
   max_level = 0L;
   ovly_tree = F0025_getblk(size_node);
   ovly_tree[ node_daughter ] = 0L;
   ovly_tree[ node_sibling ] = 0L;
   symbol_table = F0023_vec_get(symbol_table_size);
   any_relocatable_symbols = no;
   ovly_count =  - 1L ;
   refcount = 0L;
   mangled = no;
   for ( j = 0L ; j <= symbol_table_upb ; j++ )
      symbol_table[ j ] = 0L;
   i_buffer = F0023_vec_get(i_buffer_size);
   i_ptr =  - 1L ;
   i_end = 0L;
   o_buffer = F0023_vec_get(o_buffer_size);
   o_ptr =  - 1L ;
   loading_ovly_supervisor = no;
   pulist = 0L;
   pu_current =  (word*)(&pulist) ;
   freehunkchain = 0L;
}

/*  This group of routines is concerned with decoding the command */
/*   line, and interpreting any WITH files given. */
F0002_read_with_file ( file )
string file;
{
   /*  Reads directives from the file 'file'. */
   /*  The directives that can be given are as follows: */
   /*   FROM files */
   /*   ROOT files */
   /*   TO file */
   /*   LIBRARY files */
   /*   MAP [file] */
   /*   XREF [file] */
   /*   OVERLAY */
   /*   tree description */
   /*   #                     (This is part of the OVERLAY directive) */
   /*   WIDTH n */
   line_number = 1L;
   with_file = file;
   withstream = F000B_open(file, "\004WITH", yes, no);
   selectinput ( withstream );
   do
   {
      word itemv [ ( 20L ) + 1 ];   /*  The vector for the directive */
      word item = rditem(itemv, 20);
      word ch; /*  = rdch();  AC fiddle */
      /*  Check for end of file */
      if ( item == 0L )
      {
         ch = getch();
         F0006_ungetch (  );
         if ( ch == endstreamch )
            break;
      }
      /*  Analyse the item */
      if ( item != 0L )
      {
         if ( item != 1L )
            goto comerr;
         switch ( findarg("\053FROM=ROOT,TO,LIBRARY,MAP,XREF,OVERLAY,WIDTH"
            , itemv) )
         {
         default:
            goto comerr;
         case 0:
            /*  FROM files    or   ROOT files */
            if ( root_given )
               F0003_makefilelist ( 0L, yes );
            else
            {
               root_files = F0003_makefilelist(0L, no);
               root_given = yes;
            }
            break;
         case 1:
            {
               /*  TO file */
               word* f = F0003_makefilelist(0L, no);
               if ( ( f == 0L ) || ( (*f) != 0L ) )
                  goto comerr;
               if ( to_file == 0L )
                  to_file = (string)(f + 1L);
               break;
            }
         case 2:
            /*  LIBRARY files */
            if ( lib_given )
               F0003_makefilelist ( 0L, yes );
            else
            {
               lib_files = F0003_makefilelist(0L, no);
               lib_given = yes;
            }
            break;
         case 3:
            {
               /*  MAP [file] */
               word* f = F0003_makefilelist(0L, no);
               if ( ( f != 0L ) && ( (*f) != 0L ) )
                  goto comerr;
               if ( map_file == 0L )
                  if ( f == 0L )
                     mapstream = verstream;
                  else
                     map_file = (string)(f + 1L);
               break;
            }
         case 4:
            {
               /*  XREF [file] */
               word* f = F0003_makefilelist(0L, no);
               if ( ( f != 0L ) && ( (*f) != 0L ) )
                  goto comerr;
               if ( xref_file == 0L )
                  if ( f == 0L )
                     xrefstream = verstream;
                  else
                     xref_file = (string)(f + 1L);
               break;
            }
         case 5:
            {
               /*  OVERLAY */
               /*  Xfiles    X is null or is one or more *'s */
               /*  ..... */
               /*  #         or eof. */
               /*  Eg. */
               /*  OVERLAY */
               /*  a */
               /*  *b */
               /*  *c */
               /*  d */
               /*  # */
               /*  This specifies the structure root(a(b,c),d) */
               word* parent;
               word* last_sibling = ovly_tree;
               word level =  - 1L ;
               do
               {
                  /*  Loop starts here.  Reads a line each time round */
                  word count = 0L;
                  word* fl = 0L;
                  word* node = 0L;
                  do
                     /*  Skip to end of last line */
                     ch = getch(); while ( ! ( ( ch == '\n' ) || ( ch == endstreamch
                      ) ) );
                  /*  Read first character of line */
                  ch = getch();
                  /*  Check for end of directive */
                  if ( ( ch != '#' ) && ( ch != endstreamch ) )
                  {
                     while ( ch == '*' )
                     {
                        count = count + 1L;
                        ch = getch();
                     }
                     F0006_ungetch (  );
                     /*  Check for valid count */
                     if ( count > ( level + 1L ) )
                        goto comerr;
                     /*  Read the files */
                     fl = F0003_makefilelist(0L, no);
                     if ( fl == 0L )
                        continue;
                     if ( ovly_given )
                     {
                        level = count;
                        continue;
                     }
                     /*  Allocate the node */
                     node = F0025_getblk(size_node);
                     node[ node_files ] = (word)fl;
                     node[ node_daughter ] = 0L;
                     node[ node_sibling ] = 0L;
                  }
                  /*  Check the count */
                  if ( count <= level )
                  {
                     word j; /* FOR loop variables */
                     /*  Unwind to previous (or current) level */
                     /*  'count' is zero during the final loop, */
                     /*  when the end of the directive has been */
                     /*  reached. */
                     for ( j = count + 1L ; j <= level ; j++ )
                     {
                        last_sibling = parent;
                        parent = (word*)last_sibling[ node_sibling ];
                        last_sibling[ node_sibling ] = 0L;
                     }
                     last_sibling[ node_sibling ] = (word)node;
                  }
                  else
                  {
                     /*  Store back pointer in 'sibling' */
                     /*  field of new parent. */
                     last_sibling[ node_sibling ] = (word)parent;
                     last_sibling[ node_daughter ] = (word)node;
                     parent = last_sibling;
                  }
                  last_sibling = node;
                  level = count;
               } while ( ! ( last_sibling == 0L ) );
               ovly_given = yes;
               break;
            }
         case 6:
            /*  WIDTH n */
            if ( width_string == 0L )
            {
               width = F0004_read_number(0L, "\005WIDTH", default_width
                  , yes);
               width_string = 1L;
            }
            break;
         }
      }
         /*  END of SWITCHON findarg..... */
         /*  Skip to end of line */
      do
         ch = getch(); while ( ! ( ( ch == '\n' ) || ( ch == endstreamch
          ) ) );
   } while ( true );
   /*  Tidy up */
   endread (  );
   withstream = 0L;
   return;
comerr:
   /*  Here on syntax error */
   error ( err_bad_with, file, line_number );
}

word* F0003_makefilelist ( s, ignore )
word ignore;
string s;
{
   /*  This routine constructs a file list from the string */
   /*   given as the first parameter, or if this is zero, */
   /*   from the input file.  In the latter case, if the */
   /*   parameter 'ignore' is true, the list is discarded. */
   /*  File names in the input may be separated by spaces, */
   /*   commas or +'s.  The list is terminated by end of */
   /*   line or ; if reading from a file, and by end of */
   /*   string otherwise.  If, when reading from a file, */
   /*   an asterisk is encountered, the rest of the line */
   /*   is ignored, and a new line taken.  This enables */
   /*   long lists to be split across several lines. */
   char fvec [ ( 127L ) + 1 ];
   word len = 0L;
   word ptr = 0L;
   word* chain = 0L;
   word* chaine =  (word*)(&chain) ;
   word started = no;
   word finished = no;
   word read = s == 0L;
   word discard = read & ignore;
   while ( ! finished )
   {
      word ch;
      if ( read )
      {
         ch = getch();
         if ( ( ch == '\n' ) || ( ch == ';' ) )
         {
            F0006_ungetch (  );
            ch = endstreamch;
         }
      }
      else
      {
         ptr = ptr + 1L;
         if ( ptr > s[ 0L ] )
            ch = endstreamch;
         else
            ch = s[ ptr ];
      }
      if ( ch == endstreamch )
      {
         ch = ',';
         finished = yes;
      }
      if ( ( ( ( ch == ',' ) || ( ch == '+' ) ) || ( ch == ' ' ) ) || ( read
          && ( ch == '*' ) ) )
      {
         if ( ( len != 0L ) &&  ! discard  )
         {
            word j; /* FOR loop variables */
            word* f = F0025_getblk(( len / bytesperword ) + 2L);
            (*chaine) = (word)f;
            chaine = f;
            (*f) = 0L;
            for ( j = 1L ; j <= len ; j++ )
            ((string) ( f + 1L ))[ j ] = fvec[ j ];
            ((string)( f + 1L) )[ 0L ] = len;
         }
         len = 0L;
         if ( ch == '*' )
            do
               /*  Ignore rest of line */
               ch = getch(); while ( ! ( ( ch == '\n' ) || ( ch == endstreamch
                ) ) );
      }
      else
      {
         /*  Character in file name */
         len = len + 1L;
         fvec[ len ] = ch;
      }
   }
   return chain;
}

word F0004_read_number ( s, name, default_value, warn_only )
word default_value, warn_only;
string s, name;
{
   /*  This routines generates an integer from a string, or */
   /*   from the input file if the string is zero. */
   /*  Spaces are ignored before and after the optional sign. */
   /*  The number is terminated by the first non-digit if */
   /*   reading from a file, or by the end of string or space */
   /*   otherwise. */
   /*  In the event of an error, either 'error' or 'warn' is */
   /*   called, depending on the value of the fourth parameter. */
   /*  In the latter case, 'default.value' is returned as the result. */
   word n = 0L;
   word ptr = 0L;
   word ch = 0L;
   word okay = no;
   word sign = no;
   word neg = no;
   word started = no;
   word read = s == 0L;
#ifdef IBM
   int (*routine)() ;
   if (warn_only) { routine = warn;} else { routine = error; }
#endif

#ifdef SUN 
   word (*routine)() = warn_only ? warn : error ;
#endif

   do
   {
      if ( read )
      {
         ch = getch();
         if ( ( ch == '\n' ) || ( ch == ';' ) )
         {
            ch = endstreamch;
            F0006_ungetch (  );
            break;
         }
      }
      else
      {
         ptr = ptr + 1L;
         if ( ptr > s[ 0L ] )
         {
            ch = endstreamch;
            break;
         }
         else
            ch = s[ ptr ];
      }
      if ( ( ( ch == '+' ) || ( ch == '-' ) ) && ( sign == 0L ) )
      {
         sign = yes;
         neg = ch == '-';
         continue;
      }
      if ( ( ch == ' ' ) &&  ! started  )
         continue;
      started = yes;
      if ( ( '0' <= ch ) && ( ch <= '9' ) )
      {
         okay = yes;
         n = ( ( n * 10L ) + ch ) - '0';
      }
      else
      {
         if ( read )
            F0006_ungetch (  );
         break;
      }
   } while ( true );
   /*  Check for correct string termination */
   if ( ( ch != ' ' ) && ( ch != endstreamch ) )
      okay = no;
   if ( okay )
      return neg ?  - n  : n;
   if ( read )
      (*routine) ( err_readnum1, name, line_number, with_file );
   else
      (*routine) ( err_readnum2, name, s );
   return default_value;
}

word getch ( )
{
   word ch = rdch();

   if ( ch == '\n' )
      line_number = line_number + 1L;
   return ch;
}

F0006_ungetch ( )
{
   unrdch (  );
   if ( rdch() == '\n' )
      line_number = line_number - 1L;
   unrdch (  );
}

