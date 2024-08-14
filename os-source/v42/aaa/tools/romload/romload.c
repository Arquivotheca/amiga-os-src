/*
*          ************************************************ 
*          **                                            ** 
*          **        --equ  AMIGA HOME COMPUTER  equ--   ** 
*          **                                            ** 
*          ************************************************ 
*          **                                            ** 
*          **      --------------------------------      ** 
*          **           Absolute Memory Loader           ** 
*          **      --------------------------------      ** 
*          **                                            ** 
*          ************************************************ 


***********************************************************************
**                                                                    *
**   Copyright 1985, Amiga Computer Inc.   All rights reserved.       *
**   No part of this program may be reproduced, transmitted,          *
**   transcribed, stored in retrieval system, or translated into      *
**   any language or computer language, in any form or by any         *
**   means, electronic, mechanical, magnetic, optical, chemical,      *
**   manual or otherwise, without the prior written permission of     *
**   Amiga Computer Incorporated, 3350 Scott Blvd, Bld #7,            *
**   Santa Clara, CA 95051                                            *
**                                                                    *
***********************************************************************

This program was initially based on linker.b (the BCPL version by 
Tim J. King) and the "AMIGA Binary File Structure" Version 1.2,
February 16, 1985.

*/

/*
  0.0   Feb 14, 1985  Keith Stobie,  Initial Version
  0.1   Apr  8, 1985  Keith Stobie,  Revised to allow ROM addresses.
  0.1.1 Apr  9, 1985  Keith Stobie,  Correct suppress S2 rec len.
  0.1.2 Apr  9, 1985  Keith Stobie,  Correct check sum S2 rec len.
  0.2   Apr 10, 1985  Keith Stobie,  Correctly handl Symbol hunks
  0.3   Apr 23, 1985  Keith Stobie,  Lattice C IBM PC support.
  0.4	Jan 20, 1992  Randell Jesup, remove IBM garbage, make useful for A3090
*/
#define VERSION "0.4"


#include <exec/types.h>           /* Amiga standard types */
#undef NULL
#undef EOF

#include <stdio.h>         /* FILE type            */
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MIN(a,b)           (a<b)?a:b
#define MAX(a,b)           (a>b)?a:b

#ifdef DEFAULT_EXPAND
#define CLI_DEFAULTBLOCKSIZE 127  /* Size for a BSS if none (0) is provided */
#endif

/* Hunk Types */
#define HUNK_NAME       1000
#define HUNK_CODE       1001
#define HUNK_DATA       1002
#define HUNK_BSS        1003
#define HUNK_RELOC32    1004
#define HUNK_RELOC16    1005
#define HUNK_RELOC8     1006
#define HUNK_EXT        1007
#define HUNK_SYMBOL     1008
#define HUNK_DEBUG      1009
#define HUNK_END        1010
#define HUNK_HEADER     1011
#define HUNK_CONT       1012
#define HUNK_OVERLAY    1013
#define HUNK_BREAK      1014


BOOL  debug = FALSE;          /* Print debugging info to StdErr    */
BOOL  write_map = FALSE;      /* Print hunk map on standard output */
BOOL  exclude_map = FALSE;    /* Don't place hunk map in memory    */
BOOL  zero_pad = TRUE;        /* Pad last S2 record with zeroes    */
FILE  *in_file, *srec_file;   /* Input (loader file), Output( s records ) */
FILE  *raw_file = NULL;
BOOL  write_raw = FALSE;	/* write file as raw binary, not srec */
char  *prog_name;             /* Name via which program was run    */

#define MEM_SIZE 0x20000      /* 128K long words, 1/2 megabyte */
ULONG *memory;       /* Array in which a memory image is formed for srecs */

ULONG first_long_word = 0x8000L;   /* Where memory starts              */
/* next_long_word is number of long words into memory                 */
ULONG next_long_word = 0;         /* Next part of memory to fill in   */

union long_char {
    ULONG    value;
    char     bytes[ sizeof(ULONG) ];
};


/* Allocate num_words of space out of memory */
ULONG get_memory( num_words )
   ULONG num_words;
{
   ULONG new_address = next_long_word;

   next_long_word += num_words;
   if (next_long_word >= MEM_SIZE)
      {
      quit_error( "Attempt to allocate more words than allocated for memory" );
      }
   return new_address;
}  /* get_memory() */



/* Print message and quit */
quit_error( message )
   char *message;
{
   fprintf( stderr, "**** %s: %s\n", prog_name, message );
   my_exit( 1 );
}  /* quit_error() */


/* Return a words worth of data from input stream */
BOOL eof_get_word( word )
   ULONG *word;
{
   int   c;
   union long_char val;
   SHORT i;
 
   for (i = 0; i < sizeof(ULONG); i++)
      {
      if ((c = getc( in_file)) == EOF) 
         { return TRUE; }
      val.bytes[i] = c;
      }
   *word = val.value;

   return FALSE;
}  /* get_word_and_eof */

/* Return a word, quit with error if at EOF */
ULONG getword()
{
   ULONG word;

   if (eof_get_word( &word )) { quit_error( "Unexpected EOF!" ); }
   return word;
}  /* getword() */

/* Add addend to the word starting at byte_offset bytes from hunk_base */
long_add( hunk_base, byte_offset, addend )
   ULONG *hunk_base;    /* memory[] long word address of hunk */
   ULONG byte_offset;
   ULONG addend;
{
   char *byte_base = (char *) (hunk_base);
   ULONG *value = (ULONG *) &byte_base[ byte_offset ];
   if (debug) {
      fprintf( stderr,"  byte_base=0x%lx, addend=0x%lx &value=0x%lx,*=0x%lx\n"
            , byte_base, addend, value, *value );  
   }
   *value += addend;
}  /* long_add */


/* Builds an absolute image of memory, returns starting address */
ULONG build_code_image()
{
   ULONG hunk_type;     /* Type of hunk block about to be read          */
   ULONG num_words;     /* Number of long words in data following       */
   ULONG hunk_table_size;  /* Number of hunks that should be kept       */
   ULONG *hunk_table;   /* Table of addresses (indexes) in memory       */
   ULONG first_hunk;    /* Index of first hunk into hunk_table          */
   ULONG last_hunk;     /* Index of last hunk into hunk_table           */
   ULONG cur_hunk_num;  /* The next CODE, DATA, or BSS hunk to process  */
   ULONG hunk_num;      /* Which hunk in the table is being dealt with  */
   ULONG first_code_hunk_num /* first cost code hunk we saw (> 0)       */
               = -1;    /* Presumably the starting addr                 */
   ULONG hunk_base;     /* index in memory  where data for hunk starts  */
   ULONG *hunk;         /* A word in current hunk being filled in       */

   ULONG  offset;       /* Number of words from base, a continuation starts */

   ULONG num_offset;    /* Number of relocation offsets                 */
   ULONG byte_offset;   /* Number of bytes into current hunk, relocation 
                         * begins                                       */
   ULONG ext_hunk_addr; /* Base of a hunk that current hunk uses as an
                         * external                                     */

   hunk_type = getword();
   if (debug) { fprintf( stderr, "hunk_type= %lx\n", hunk_type );}
   if (hunk_type != HUNK_HEADER)
      { quit_error( "First hunk not a hunk header!" ); }

   num_words = getword();
   if (debug) { fprintf( stderr, "num_words=%ld", num_words ); }
   if (num_words != 0)
      { quit_error( "Can't handle resident libraries!" ); }

   hunk_table_size = getword();
   first_hunk  = getword();
   last_hunk   = getword();
   if (debug) {
      fprintf( stderr, "  table_size=%ld  first_hunk=%ld  last_hunk=%ld\n"
         , hunk_table_size, first_hunk, last_hunk );
   }

   /* Read in the table */
   if (exclude_map) 
      { if ((hunk_table = (ULONG *) calloc( hunk_table_size, sizeof( ULONG )))
             == NULL)
           { quit_error( "Unable to allocate memory for hunk_table!" ); }
      }
   else
      { hunk_table  = &memory[ get_memory( hunk_table_size ) ];}

   if (debug && write_map) 
      { 
      printf( "deci  hex      hex \n" );
      printf( "hunk  base    size in \n" );
      printf( "num  address   bytes \n" );
      }
   for (hunk_num = first_hunk; hunk_num < hunk_table_size; hunk_num++)
      {
      if (hunk_num <= last_hunk)
         {
         num_words = getword();
#ifdef DEFAULT_EXPAND
         if (num_words == 0)
            num_words = CLI_DEFAULTBLOCKSIZE;   /* in case it is BSS. */
#endif
         hunk_base = first_long_word + get_memory( num_words );
         if (debug) {
            fprintf( stderr
                  , " hunk_num=%ld, hunk_base=0x%lx, num_words=%ld        &memory=0x%lx\n"
                  , hunk_num, hunk_base, num_words, hunk_base + &memory[0] );
         }
         if (write_map)
            {
            printf( "%2ld   %06lX     %4lX\n"
                  , hunk_num, hunk_base*sizeof(ULONG)
                  , num_words*sizeof(ULONG) );
            }
         /* $$ BCLP loader.b has chain together? */
         }
      else 
         {
         hunk_base = NULL;
         }
      hunk_table[ hunk_num ] = hunk_base;
      }

   /* Now load the hunks */
   cur_hunk_num = first_hunk;
   while (! eof_get_word( &hunk_type ))
      {
      if (debug) {
         fprintf( stderr, "hunk_type=%ld ", hunk_type );
      }
      switch (hunk_type) 
         {
      case HUNK_DEBUG:
      case HUNK_NAME:   num_words = getword();
                        if (debug) {
                           fprintf( stderr, " num_words=%ld \n", num_words );
                        }
                        /* Throw away the name or data */
                        while (num_words > 0)
                           {getword(); num_words--;}
                        break;

      case HUNK_BSS:
      case HUNK_DATA:
      case HUNK_CODE:   num_words = getword();
#ifdef DEFAULT_EXPAND
                        if ((num_words == 0) && (hunk_type == HUNK_BSS))
                           num_words = CLI_DEFAULTBLOCKSIZE; 
#endif
                        if (cur_hunk_num >= hunk_table_size)
                           { quit_error( "More hunks given then specified in header!" );}
                        
                        if ((hunk_type == HUNK_CODE) 
                        &&  (first_code_hunk_num == -1))
                           { /* This is the first_code_hunk! */
                           first_code_hunk_num = cur_hunk_num;
                           }
                        hunk_base = hunk_table[ cur_hunk_num ];
                        if (debug) {
                           fprintf( stderr," hunk_base=0x%lx, num_words=%ld\n"
                                 , hunk_base, num_words );
                        }
                        cur_hunk_num++;   /* Current next time we come thru */

                        hunk = &memory[ hunk_base - first_long_word ];
                        for ( ; num_words > 0; num_words-- )
                           {
                           *hunk++ = (hunk_type == HUNK_BSS) ? 0 : getword();
                           }
                        break;            

      case HUNK_CONT:   hunk_num = getword();
                        offset   = getword();
                        num_words = getword();
                        if (debug) {
                           fprintf( stderr
                              , "  hunk_num=%ld, offset=%ld, num_words=%ld\n"
                              , hunk_num, offset, num_words );
                        }

                        hunk = &memory[ hunk_table[ hunk_num ] 
                                        - first_long_word + offset ];
                        for ( ; num_words > 0; num_words-- )
                           {
                           *hunk++ = getword();
                           }
                        break;

      case HUNK_RELOC32: while ((num_offset = getword()) != 0)
                           {
                           hunk_num = getword();
                           if ((hunk_num < 0) || (hunk_num > last_hunk))
                              { 
                              fprintf( stderr
                                 , "**** %s: Bad hunk num, %ld, for reloc!  Not between %ld and %ld\n"
                                 , prog_name, hunk_num, 0, last_hunk );
                              fprintf( stderr, "     Skipping %ld words of offsets\n", num_offset ); 
                              for ( ; num_offset > 0; num_offset--)
                                 byte_offset = getword();
                              break;
                              }

                           ext_hunk_addr = hunk_table[ hunk_num ];
                           hunk = &memory[ hunk_base - first_long_word ];
                           if (debug) {
                              fprintf( stderr
                                    , "  hunk_num=%ld, addr=0x%lx\n"
                                    , hunk_num, ext_hunk_addr );
                           }
                           for ( ; num_offset > 0; num_offset--)
                              {
                              byte_offset = getword();
                              if (debug) {
                                 fprintf( stderr, "  byte_offset=0x%lx"
                                       , byte_offset );
                              }

                              long_add( hunk, byte_offset
                                      , (LONG) ext_hunk_addr * sizeof(LONG) );
                              }
                           if (debug) {
                              fprintf( stderr, "\n" );
                           }
                           }
                        break;

      case HUNK_END:    
                        if (debug) {
                           fprintf( stderr, "\n" );
                        }
                        break;      /* No overlays to worry about */

      case HUNK_SYMBOL: while((num_words = getword()) != 0)
                           {
                           num_words &= 0xffffff; /* Ignore top byte */
                           num_words++;
                           if (debug) {
                              fprintf( stderr, "symbols, num_words=%ld\n"
                                     , num_words );
                              }

                            /* Ignore name, and symbol value word */
                            while (num_words > 0)
                               {getword(); num_words--;}
                           }
                        break;
      
      case HUNK_BREAK:  
      case HUNK_OVERLAY: quit_error( "Can't handle overlay hunk!" );

      default:           quit_error( "Unrecognized hunk type!" );
         }  /* end switch */
      }  /* end while */

   /* Calculate starting code byte address */
   byte_offset = hunk_table[ first_code_hunk_num ] * sizeof( ULONG );

   if (exclude_map) 
      { free( hunk_table ); }
   else
      { /* Convert word addresses to byte addresses in hunk_table */
      for (hunk_num = first_hunk; hunk_num < hunk_table_size; hunk_num++)
         { hunk_table[ hunk_num ] *= sizeof( ULONG ); }
      }

   return byte_offset;    /* Starting code address */

}  /* build_code_image() */



/* Write out as many S2 records as necessary to write:
 * format of S2 record is:
 *   S2<len><addr><data><checksum>
 * <len>  is two ascii digits representing a hexadecimal numbers.
 *         Thus maximum length is 255; Length of addr + data + checksum.
 * <addr>  six ascii hex digits of where data is to go.
 * <data>   ascii hex digits of data.
 * <checksum> 2 ascii hex digits which when summed with all the data leaves
 *             a FF in the lowest byte.
 */

write_s_rec( data, len, address )
   UBYTE *data;
   ULONG len;  /* Number of bytes of data to write */
   ULONG address;
{
   ULONG rec_len; /* Number of bytes of data being written to S2 record */
   SHORT i;
#define MAX_REC_LEN 32

 if (!write_raw)
 {
   /* Write 128 hex digits (MAX_REC_LEN bytes) at a time */
   while (len > 0)
      {
      rec_len = MIN( len, MAX_REC_LEN );
      fprintf( srec_file, "S2%02lX%06lX"
             , (zero_pad?MAX_REC_LEN:rec_len)+4, address );
      for (i = 0; i<rec_len; i++)
         { fprintf( srec_file, "%02lX", *data++ ); }

      if ((rec_len < MAX_REC_LEN) && (zero_pad))
         { /* Kludge: Zero pad last record. (Zeros don't change checksum) */
         for (i = 0; i<(MAX_REC_LEN-rec_len); i++)
            { fprintf( srec_file, "00" );}
         }

      fprintf( srec_file, "%02lX\n"
             , (LONG) checksum( address, data - rec_len
                       , (zero_pad?MAX_REC_LEN:rec_len)+4 ) );
      address += rec_len;
      len -= rec_len;
      }
 } else {
	fwrite(data,len,1,srec_file);
 }
}  /* write_s_rec( ) */

/* Checksum the data in sum, len, and data */
int checksum( sum, data, len )
   ULONG sum;     /* Starting set of four bytes to checksum */
   UBYTE *data;   /* Array of data to checksum              */ 
   ULONG len;     /* Length to be checsummed in, and length of data+4 */
{
   sum = (sum>>24) + (sum>>16 & 0xFF) + (sum>>8 & 0xFF) + (sum & 0xFF);
   sum = sum  + (len>>16 & 0xFF) + (len>>8 & 0xFF) + (len & 0xFF);
   len -= 4;   /* Discount the 4 bytes of address */

   for (; len > 0; len--)
       { sum += *data++; }

   return (0xFF - (int)(sum & 0xFF));
}  /* checksum() */  

void
add_raw (ULONG raw_addr)
{
	FILE *save_in = in_file;
	ULONG i = raw_addr+1;
	ULONG val;
	BOOL eof;

	if (!raw_file)
		return;

	in_file = raw_file;
	while (!(eof = eof_get_word(&val)) &&
	       (i + first_long_word < MEM_SIZE))
	{
		memory[ i - first_long_word ] = val;
		i++;
	}
	/* save size in longword before data */
	memory[raw_addr - first_long_word] = (i - (raw_addr+1)) * 4;
	next_long_word = max(next_long_word,i + first_long_word);
	in_file = save_in;
}

/* Describe usage of this program */
print_help()
{
   fprintf( stderr, "%s Version %s \n", prog_name, VERSION );
   fprintf(stderr, "Usage: %s -dmsx -o <sfile> -T <addr> <lfile> \n"
          , prog_name );
#define fprint(msg) fprintf(stderr,"   %s\n",msg)
fprint("-d         Write debugging info to stderr.        (default is none)");
fprint("-m         Write map of hunk addresses to stdout. (default is none)");
fprint("           Output: <decimal hunk num> <hex base addr> <hex length>");
fprint("-s         Write last S2 record short (Suppress zero fill)" );
fprint("-x         Exclude writing map into memory (best for ROM code)" );
fprint("-o <sfile> Writes srecords to <sfile>.          (default is a.srec)");
fprint("-F <rfile> <addr> Puts file <rfile> into image at <addr>, raw      ");
fprint("-T <addr>  Byte address in hexadecimal to start segment at."); 
fprint("           Address must be long word aligned.    (default is 20000)");
fprint("<lfile>    Opens and reads loader <lfile>.  (default is read stdin)");
}  /* print_help() */


/* Convert an Amiga binary load format file (from the linker) 
 * to an absolute memory image Motorala srecord file  
 * Bugs: Doesn't handle overlays.
 */

main( argc, argv )
   int   argc;
   char  *argv[];
{
#define quit_help(msg) { print_help(); quit_error( msg );}
   char  *s;
   char  file_name[ 1024 ];
   char  raw_name[ 1024 ];
   char  addr_file_name[ 1024 ];
   FILE  *addr_file;		/* starting address file descriptor */

   /* Starting byte address */
   ULONG start_addr = first_long_word * sizeof(ULONG); 
   ULONG start_code, raw_addr;

   addr_file_name[0] = 0;
   raw_name[0] = 0;

   prog_name = *argv;         /* Name via which we were run */
   strcpy( file_name, "a.srec" );   /* Default srec name, override with -o */

   /* Setup memory */
   if ((memory = (ULONG *)calloc( MEM_SIZE, sizeof( ULONG ))) == NULL)
      {
      fprintf( stderr, "**** %s: Unable to allocate %ld bytes of memory! \n"
             , prog_name, (LONG)MEM_SIZE * sizeof( ULONG ) );
      exit( 1 );
      }

   /* Get - flags and process */
   while ((--argc > 0)  && ((*++argv)[0] == '-'))
      {
      for (s = argv[0]+1; *s != '\0'; s++)
         switch (*s) 
            {
         case 'd': debug = TRUE;       /* write Debugin info to stderr */
                   break;
         case 'm': write_map = TRUE;   /* write hunk Map to stdout */
                   break;
         case 'x': exclude_map = TRUE; /* eXclude placing hunk map in memory*/
                   break;
         case 's': zero_pad = FALSE;  /* Suppress zero pad */
                   break;
         case 'o': /* Open srec output file */
                   if (--argc < 0) 
                      quit_help( "Missing object file name after -o option!");
                   sscanf( *++argv, "%s", file_name );
                   break;                  
         case 'F': /* Open raw input file */
                   if (--argc < 0) 
                      quit_help( "Missing raw file name after -F option!");
                   sscanf( *++argv, "%s", raw_name );
		   if (--argc < 0) 
                      quit_help( "Missing raw address after -F option!");
                   sscanf( *++argv, "%x", &raw_addr );
		   /* convert byte to longword offset */
		   raw_addr /= sizeof(ULONG);
                   break;                  
	 case 'b': /* write output as binary file */
		   write_raw = TRUE;
		   break;

         case 'T': /* get starting address */
                   if (--argc < 0) 
                      quit_help( "Missing address param after -T option!" );
                   sscanf( *++argv, "%x", &start_addr );
                   first_long_word = start_addr / sizeof( ULONG );
                   if (first_long_word*sizeof(ULONG) != start_addr) 
                      quit_help("-T Address is not on a long word boundary!");
                   break;
	 case 'a': /* get starting address from a file */
                   if (--argc < 0) 
                      quit_help("Missing address file name after -a option!");
                   sscanf( *++argv, "%s", addr_file_name );
		   if (( addr_file = fopen (addr_file_name, "r")) == NULL){
		       quit_help("Address file will not open!");
		   }
		   else {
		       fscanf (addr_file, "%lx", &start_addr);
	               first_long_word = start_addr / sizeof( ULONG );
                       if (first_long_word*sizeof(ULONG) != start_addr) 
                          quit_help("Start address not long word aligned!");
		       fclose (addr_file);
		   }
                   break;                 
         default:  print_help( prog_name );
                   fprintf( stderr, "**** %s: illegal option %lc\n"
                           , prog_name, *s );
                   my_exit( 1 );
            }
      }



   if (debug) 
      {
      fprintf( stderr, "Version %s \n", VERSION );
      fprintf( stderr, "start address = 0x%lx long words\n", first_long_word);
      }

   if (argc > 1)
      { quit_help( "Too many paramaters or bad syntax!" ); }

   if ((srec_file = fopen( file_name, "w+" )) == NULL)
      { 
      fprintf( stderr, "**** %s: Unable to open file %s \n"
             , prog_name, file_name );
      my_exit( 1 );
      }

   if ((*raw_name) && (raw_file = fopen( raw_name, "r" )) == NULL)
      { 
      fprintf( stderr, "**** %s: Unable to open file %s \n"
             , prog_name, raw_name );
      my_exit( 1 );
      }

   if (argc == 1)
      {
      sscanf( *argv, "%s", file_name );
      if ((in_file = fopen( file_name, "r" )) == NULL)
         { 
         fprintf( stderr, "**** %s: Unable to open file %s \n"
                , prog_name, file_name );
         my_exit( 1 );
         }
      }
   else
      {
      in_file = stdin;
      }

   /* Read MetaCompco loader format and create memory image */
   start_code = build_code_image(); /* Returns byte starting position */

   /* add raw file, if any, to image */
   add_raw(raw_addr);

   /* Write Motorla srecords from memory image */
   write_s_rec( (char *) memory
               , next_long_word * sizeof( ULONG )
               , start_addr );

   /* Write final S8 record */
   if (!write_raw)
   {
	   fprintf( srec_file, "S8%02lX%06lX", 4, start_code );  
	   fprintf( srec_file, "%02lX\n"
                    , checksum( start_code, 0, 4 ) );
   }

   /* update starting address file */
   if (addr_file_name[0] != '\0') {
   	if (( addr_file = fopen (addr_file_name, "w+")) == NULL){
   	    	quit_help("Address file will not open for write!");
   	}
   	else {
       	    fprintf (addr_file, "%lx",
/*             (next_long_word*sizeof( ULONG ) +start_addr+15) & 0xfffff0 ); */
               (next_long_word*sizeof( ULONG ) +start_addr) );
       	    fclose (addr_file);
       }
   }

   exit(0);
}  /* main() */

my_exit( code )
{
   free( (char *) memory );
   exit( code );
}   /* my_exit() */ 
