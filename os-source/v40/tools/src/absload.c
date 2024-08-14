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
  -DIBM   to generate for IBM PC
  -DAMIGA to generate for the Amiga
    default  generate for Sun
  -DBBLOAD to generate version for SUN that writes to billboard 
           instead of srecords.

  0.0   Feb 14, 1985  Keith Stobie,  Initial Version
  0.1   Apr  8, 1985  Keith Stobie,  Revised to allow ROM addresses.
  0.1.1 Apr  9, 1985  Keith Stobie,  Correct suppress S2 rec len.
  0.1.2 Apr  9, 1985  Keith Stobie,  Correct check sum S2 rec len.
  0.2   Apr 10, 1985  Keith Stobie,  Correctly handl Symbole hunks
  0.3   Apr 23, 1985  Keith Stobie,  Lattice C IBM PC support.
  0.4   Jul  5, 1985  Keith Stobie,  Native Amiga support.
                                     Removed DEFAULT expansion (TRIPOS)
  0.5   Jul 13, 1985  Keith Stobie,  -b <size>K option & checksum bug fix.
  1.0   Jul 13, 1985  Keith Stobie,  Added write to billboard.
*/
#define VERSION "1.0"

/*  Developer's Notes:
   The IBM version has never been made to work completely.
   It always dies near the end.

   The AMIGA version was done without using Lattice C, instead
   METACC and custom startup.obj, etc. were used.
*/
#ifndef AMIGA
#ifndef IBM
#define SUN                   /* Default if neight AMIGA or IBM */
#endif
#endif

#ifdef IBM
#define BINARY_OPEN 0x8000
#define TEXT_OPEN   0
int _fmode = TEXT_OPEN;    /* treat all files as text unless set otherwise */
#endif 


#ifdef AMIGA
#include <exec/types.h>        /* Amiga standard types */
#include <exec/nodes.h>        /* memory.h wants it */
#include <exec/memory.h>       /* MEMF_PUBLIC     */
#include <libraries/dos.h>     /* BPTR            */
#define EOF       -1
typedef BPTR      FILE;        /* DOS file handle */ 
extern FILE *stdin, *stdout;     /* From startup.obj            */
#define stderr stdout          /* There is no stderr on Amiga */
char *strcpy();
ULONG get_hex();

#else       /* SUN & IBM */

#undef NULL
#undef EOF
#include <stdio.h>         /* FILE type            */
typedef long		LONG;	    /* signed 32-bit quantity */
typedef unsigned long	ULONG;	    /* unsigned 32-bit quantity */
typedef short		WORD;	    /* signed 16-bit quantity */
typedef char		BYTE;	    /* signed 8-bit quantity */
typedef unsigned char	UBYTE;	    /* unsigned 8-bit quantity */
typedef short		BOOL;
#define TRUE		1
#define FALSE		0
#endif



#define MIN(a,b)           (a<b)?a:b
#define MAX(a,b)           (a>b)?a:b

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


BOOL  debug = FALSE;          /* Print debugging info to StdErr         */
BOOL  write_map = FALSE;      /* Print hunk map on standard output      */
char  file_name[ 1024 ];      /* Name of an input or output file        */
FILE  *in_file = NULL;        /* Input (loader file)                    */
FILE  *srec_file = NULL;      /* Output( s records )                    */
char  *prog_name;             /* Name via which program was run         */
#ifdef BBLOAD
int   ofd = -1;               /* Output File descriptor for billbaord   */
char  *ofname = "/dev/bb0";   /* Name of output file. Default is billboard */
BOOL  exclude_map = TRUE;     /* Don't place hunk map in memory (-x)    */
BOOL  zero_pad = FALSE;       /* Pad last S2 record with zeroes (-s)    */
#else
char  *ofname = "a.srec";     /* Name of output file. Default is a.srec */
BOOL  exclude_map = FALSE;    /* Don't place hunk map in memory (-x)    */
BOOL  zero_pad = TRUE;        /* Pad last S2 record with zeroes (-s)    */
#endif

#ifdef IBM
LONG mem_size = 0x20000L;     /* 128K bytes */
#else
#ifdef  AMIGA
LONG mem_size =  0x4000L;     /* 16K bytes (assume fragmented memory)*/
#else
LONG mem_size = 0x80000L;     /* 1/2 megabyte (use virtual memory) */
#endif
#endif
LONG mem_words;               /* Size of memory in LONG words      */
ULONG *memory = NULL;  /* Array in which a memory image is formed for srecs */

ULONG first_long_word = 0x8000L;   /* Where memory starts              */
/* next_long_word is number of long words into memory                 */
ULONG next_long_word = 0;         /* Next part of memory to fill in   */

union long_char {
    ULONG    value;
    char     bytes[ sizeof(ULONG) ];
};

#ifdef IBM
 char *getml();        /* Dynamic memory allocator for memory */ 
#else
#ifdef AMIGA
 char *AllocMem();
#else
 char *calloc();      /* Dynamic memory allocator for memory */ 
#endif
#endif

/* Allocate num_words of space out of memory */
ULONG get_memory( num_words )
   ULONG num_words;
{
   ULONG new_address = next_long_word;

   next_long_word += num_words;
   if (next_long_word >= mem_words)
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
   my_exit( 10 );
}  /* quit_error() */

#ifdef IBM
/* Convert MC6800 binary long words to 8086 */
ULONG swap(val)
    union long_char *val;
{
    union long_char temp;
    temp.bytes[0]=val->bytes[3];
    temp.bytes[1]=val->bytes[2];
    temp.bytes[2]=val->bytes[1];
    temp.bytes[3]=val->bytes[0];
    return((long)temp.value);
} /* swap() */
#endif

/* Return a words worth of data from input stream */
BOOL eof_get_word( word )
   ULONG *word;
{
   int   c;
   union long_char val;
   WORD i;
 
   for (i = 0; i < sizeof(ULONG); i++)
      {
      if ((c = getc( in_file)) == EOF) 
         { return TRUE; }
      val.bytes[i] = c;
      }
#ifdef AMIGA
   if (SetSignal(0,0) & (1<<12) /* CTRL-B */) {
      quit_error( "**BREAK detected" );
   }
#endif 
#ifdef IBM 
   /* Swap bytes for 8086! */
   *word = swap( &val.value );
#else
   *word = val.value;
#endif
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
   if (debug) { fprintf( stderr, "hunk_type= 0x%lx\n", hunk_type );}
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
#ifdef AMIGA
      { if ((hunk_table = (ULONG *)AllocMem( hunk_table_size * sizeof( ULONG)
                                           , MEMF_PUBLIC ))
#else
      { if ((hunk_table = (ULONG *) calloc( hunk_table_size, sizeof( ULONG )))
#endif
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
         hunk_base = first_long_word + get_memory( num_words );
         if (debug) {
            fprintf( stderr
                  , " hunk_num=%ld, hunk_base=0x%lx, num_words=%ld        &memory=0x%lx\n"
                  , hunk_num, hunk_base, num_words, hunk_base + &memory[0] );
         }
         if (write_map)
            {
#ifdef AMIGA
            printf( "%2ld   %06lx     %4lx\n"
#else
            printf( "%2ld   %06lX     %4lX\n"
#endif
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
#ifdef AMIGA
      { FreeMem( (char *)hunk_table, hunk_table_size * sizeof( ULONG) );}
#else
      { cfree( hunk_table ); }
#endif
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
#ifdef IBM
   char *data;
#else
   UBYTE *data;
#endif
   ULONG len;  /* Number of bytes of data to write */
   ULONG address;
{
   ULONG rec_len; /* Number of bytes of data being added to S2 record */
   WORD i;
#ifdef IBM
    unsigned int unsigned_char;        /* Value to hold unsigned char */
#endif
#define MAX_REC_LEN  32  /* Number of data bytes per S2 record */

   if (zero_pad && (len % MAX_REC_LEN)) {
      /* Fill out data with zeroes to make evenly divisible by MAX_REC_LEN */
      rec_len = MAX_REC_LEN - (len % MAX_REC_LEN);
      for ( i = 0; i < rec_len; i++ ) { 
          data[ len + i ] = 0; 
      }
      /* Adjust len to be evenly divisbile by MAX_REC_LEN */
      len += rec_len;
   }
 
   /* Write 64 hex digits (MAX_REC_LEN bytes) at a time */
   while (len > 0)
      {
#ifdef AMIGA
      if (SetSignal(0,0) & (1<<12) /* CTRL-B */) {
         quit_error( "** BREAK detected" );
      }
#endif 
      rec_len = MIN( len, MAX_REC_LEN );
#ifdef AMIGA
      fprintf( srec_file, "S2%02lx%06lx", rec_len+4, address );
#else
      fprintf( srec_file, "S2%02lX%06lX", rec_len+4, address );
#endif

      for (i = 0; i<rec_len; i++)
#ifdef IBM
         { 
         unsigned_char = ((int)*data++) && 0xFF;
         fprintf( srec_file, "%02lX", unsigned_char );
         }
#else
#ifdef AMIGA
         { fprintf( srec_file, "%02lx", *data++ ); }
#else
         { fprintf( srec_file, "%02lX", *data++ ); }
#endif
#endif

#ifdef AMIGA
      fprintf( srec_file, "%02lx\n" 
#else
      fprintf( srec_file, "%02lX\n"
#endif
             , (LONG) checksum( address, &data[ -rec_len ]
                              , rec_len+4 ) );
      address += rec_len;
      len -= rec_len;
      }
}  /* write_s_rec( ) */

/* Checksum the data in sum, len, and data */
int checksum( sum, data, len )
   ULONG sum;     /* Starting set of four bytes to checksum */
#ifdef IBM
   char *data;    /* Array of data to checksum              */
#else
   UBYTE *data;   /* Array of data to checksum              */ 
#endif
   ULONG len;     /* Length to be checsummed in, and length of data+4 */
{
   sum = (sum>>24) + (sum>>16 & 0xFF) + (sum>>8 & 0xFF) + (sum & 0xFF);
   sum = sum  + (len>>16 & 0xFF) + (len>>8 & 0xFF) + (len & 0xFF);
   len -= 4;   /* Discount the 4 bytes of address */

   for (; len > 0; len--)
#ifdef IBM
       { sum += ((int)*data++)&0xFF;}
#else
       { sum += *data++; }
#endif
   return (0xFF - (int)(sum & 0xFF));
}  /* checksum() */  



/* Describe usage of this program */
print_help()
{
   fprintf( stderr, "%s Version %s \n", prog_name, VERSION );
#define fprint(msg) fprintf(stderr,"   %s\n",msg)
#ifdef BBLOAD
fprintf(stderr, "Usage: %s -xdm -o <billboard> -T <addr> -b <Ksize> <lfile>\n"
          , prog_name );
fprint("-x         Write segment map at start of memory" );
#else
fprintf(stderr, "Usage: %s -sxdm -o <sfile> -R <addr> -T <addr> -b <Ksize> <lfile>\n"
          , prog_name );
fprint("-s         Write last S2 record short (Suppress zero fill)" );
fprint("-x         Exclude writing map into memory (best for ROM code)" );
#endif !BBLOAD
fprint("-d         Write debugging info to stderr.        (default is none)");
fprint("-m         Write map of hunk addresses to stdout. (default is none)");
fprint("           Output: <decimal hunk num> <hex base addr> <hex length>");
#ifdef BBLOAD
fprint("-o <billboard> Bill Board device to write to. (default is /dev/bb0)");
#else
fprint("-o <sfile> Writes srecords to <sfile>.          (default is a.srec)");
fprint("-R <offset>  Offset to subtract from address in S-Record."); 
#endif BBLOAD
fprint("-T <addr>  Byte address in hexadecimal to start segment at."); 
fprint("           Address must be long word aligned.    (default is 20000)");
   fprintf( stderr  
 , "   -b <Ksize> Memory buffer size in units of 1,024 bytes. (default %ld)\n"
     , mem_size/1024 );
#ifdef IBM
fprint("<lfile>    Opens and reads loader <lfile>. ");
#else
fprint("<lfile>    Opens and reads loader <lfile>.  (default is read stdin)");
#endif
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
   
   /* Starting byte address */
   ULONG start_addr = first_long_word * sizeof(ULONG); 
   ULONG rom_offset = 0;
   ULONG start_code;

#ifdef IBM
   prog_name = "absload";     /* Name via which we assume program is called */
#else
   prog_name = *argv;         /* Name via which we were run */
#endif

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
         case 'x': exclude_map = ! exclude_map;
                   break;
#ifndef BBLOAD
         case 's': zero_pad = ! zero_pad;
                   break;
#endif
         case 'o': /* Open srec output file */
                   if (--argc < 0) 
                      quit_help( "Missing object file name after -o option!");
                   ofname = *++argv;  /* Keep new name */
                   break;                  
#ifndef BBLOAD
	 case 'R': /* get rom address */
                   if (--argc < 0) 
                      quit_help( "Missing address param after -R option!" );
#ifdef AMIGA
                   rom_offset = get_hex( *++argv );
#else
                   sscanf( *++argv, "%x", &rom_offset );
#endif
#endif !BBLOAD
		   break; 
         case 'T': /* get starting address */
                   if (--argc < 0) 
                      quit_help( "Missing address param after -T option!" );
#ifdef AMIGA
                   start_addr = get_hex( *++argv );
#else
                   sscanf( *++argv, "%x", &start_addr );
#endif
                   first_long_word = start_addr / sizeof( ULONG );
                   if (first_long_word*sizeof(ULONG) != start_addr) 
                      quit_help("-T Address is not on a long word boundary!");
                   break;
         case 'b': /* get memory size in K */
                   if (--argc < 0) 
                      quit_help("Missing buffer size param after -b option!");
#ifdef AMIGA
                   mem_size = get_dec( *++argv );
#else
                   sscanf( *++argv, "%d", &mem_size );
#endif
                   if (mem_size >= 2048) /* not more then 2MB */ 
                      quit_help("-b size given is more than 2MB!");
                   mem_size = mem_size * 1024;  /* Convert K to bytes */
                   break;
         default:  print_help( prog_name );
                   fprintf( stderr, "**** %s: illegal option %lc\n"
                           , prog_name, *s );
                   my_exit( 10 );
            }
      }



   if (debug) 
      {
      fprintf( stderr, "Version %s \n", VERSION );
      fprintf( stderr, "start address = 0x%lx long words, mem size = %ldK\n"
             , first_long_word, mem_size/1024 );
      }

   if (argc > 1)
      { quit_help( "Too many parameters or bad syntax!" ); }

   /* Setup memory */
#ifdef IBM
   if ((memory = (ULONG *)getml( mem_size )) == NULL)
#endif
#ifdef AMIGA
   if ((memory = (ULONG *)AllocMem( mem_size, MEMF_PUBLIC )) == NULL )
#endif
#ifdef SUN
   if ((memory = (ULONG *)calloc( mem_size, sizeof( BYTE ))) == NULL)
#endif
      {
      fprintf( stderr, "**** %s: Unable to allocate %ld bytes of memory! \n"
             , prog_name, mem_size );
      exit( 1 );
      }
   mem_words = mem_size / sizeof( ULONG );

#ifdef BBLOAD
   if ((ofd = open (ofname, 2 )) == -1) {
      fprintf (stderr, "%s: open of ", prog_name );
      perror( ofname );
      my_exit( 2 );
   }
#else

#ifdef AMIGA
   if ((srec_file = (FILE *) Open( ofname, MODE_NEWFILE )) == NULL)
#else
   if ((srec_file = fopen( ofname, "w+" )) == NULL)
#endif
      { 
      fprintf( stderr, "**** %s: Unable to open file %s \n"
             , prog_name, ofname );
      my_exit( 10 );
      }
#endif !BBLOAD

   if (argc == 1)
      {
      strcpy( file_name, *argv );
#ifdef IBM
      /* Read the load file in binary format */
      if ((in_file = fopen( file_name, "rb" )) == NULL)
#endif
#if AMIGA
      if ((in_file = (FILE *)Open( file_name, MODE_OLDFILE )) == NULL)
#endif
#ifdef SUN
      if ((in_file = fopen( file_name, "r" )) == NULL)
#endif
         { 
         fprintf( stderr, "**** %s: Unable to open file %s \n"
                , prog_name, file_name );
         my_exit( 10 );
         }
      }
   else
      {
#ifdef IBM
      /* Can't reopen STDIN in binary mode, so ... */
      fprintf( stderr, "**** %s: A load file file name is required!\n"
             , prog_name );
#else
#ifdef AMIGA
      file_name[0] = '\0'; /* Indicate STDIN was used so don't close it */
#endif
      in_file = stdin;
#endif
      }



   /* Read MetaCompco loader format and create memory image */
   start_code = build_code_image(); /* Returns byte starting position */

#ifdef BBLOAD
   /* Write memory image through bill board */
   put_memory( (char *) memory 
             , next_long_word * sizeof( ULONG )
             , start_addr );
#else
   /* Write Motorla srecords from memory image */
   write_s_rec( (char *) memory
	       , next_long_word * sizeof( ULONG )
	       , start_addr-rom_offset );

   /* Write final S8 record */
#ifdef AMIGA
   fprintf( srec_file, "S8%02lx%06lx", 4, start_code-rom_offset );  
   fprintf( srec_file, "%02lx\n", checksum( start_code-rom_offset, 0, 4 ) );
#else
   fprintf( srec_file, "S8%02lX%06lX", 4, start_code-rom_offset );  
   fprintf( srec_file, "%02lX\n", checksum( start_code-rom_offset, 0, 4 ) );
#endif
#endif !BBLOAD

   my_exit( 0 );
}  /* main() */



my_exit( code )
   int code;
{
#ifdef AMIGA 
   if (srec_file != NULL) { Close( srec_file );}
   if ((in_file != NULL) && (file_name[0] != '\0')) {
      Close( in_file );  /* Must close if not STDIN */
   }
   if (memory) { FreeMem( (char *) memory, mem_size );}
#endif

#ifdef IBM
   if (memory) {
      if (rlsml( (char *) memory, mem_size ) != 0) {
         fprintf( stderr, "%s: Unable to release memory!\n", prog_name );
         if (code == 0) { code = 1;}
      }
   }
#endif

#ifdef SUN
   if (memory) { cfree( (char *) memory );}
#endif

   exit( code );
}   /* my_exit() */ 

#ifdef AMIGA
int getc( file ) 
   FILE *file;
{
   UBYTE buf;
   int len, error;

   if ((len = Read( file, &buf, 1 )) <= 0) { 
      if (len < 0) { printf( "Error reading file: %ld\n", IoErr() ); }
      return EOF; 
   }
   return buf;
} /* getc() */

char *strcpy(dest, source)
register char    *dest;
register char    *source;
{
   char *orig = dest;
   while (*dest++ = *source++);

  return( orig );
}

ULONG get_hex( s )
   char *s;    /* String with hex digits */
{
   long num=0;

   for (;*s != '\0'; s++) {
      switch( *s ) {
         case '0': case '1': case '2': case '3': case'4':
         case '5': case '6': case '7': case '8': case'9':
            num = num*16 + *s - '0'; break;
         case 'a': case 'b': case 'c': case 'd': case'e': case'f':
            num = num*16 + 10 + *s - 'a'; break;
         case 'A': case 'B': case 'C': case 'D': case'E': case'F':
            num = num*16 + 10 + *s - 'A'; break;
         default: return num;    /* return what we found */
      }
   }

   return num;
}  /* get_hex() */

int get_dec( s )
   char *s;    /* String with decimal digits */
{
   long num=0;

   for (;*s != '\0'; s++) {
      switch( *s ) {
         case '0': case '1': case '2': case '3': case'4':
         case '5': case '6': case '7': case '8': case'9':
            num = num*10 + *s - '0'; break;
         default: return num;    /* return what we found */
      }
   }

   return num;
}  /* get_dec() */

#endif !AMIGA

#ifdef BBLOAD
#include "bbload.c"
#endif BBLOAD


