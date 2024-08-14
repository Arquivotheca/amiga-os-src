/************************************************************************
 *
 * JFTRANS.C -  Subroutine to read/write files to/from PC/Amiga.
 *
 *
 * 01-25-88 by Bill Koester (CATS) West Chester
 * Copyright 1988 Commodore International Ltd.
 *
 ************************************************************************/

/************************************************************************
 *
 * FUNCTION:
 *
 * j_file_transfer()
 * 
 * SYNTAX:
 * 
 * Error = j_file_transfer(Infile,Outfile,Direction,Mode,Transtable,Convert);
 * 
 * INPUTS:
 * 
 * char *Infile      - Pointer to NULL terminated string which is the name of
 *                     the input file to open. If Direction==JFT_AMIGA_PC Infile
 *                     will point to string containing a valid AmigaDos
 *                     filespec as documented in AREAD.DOC. If Direction==
 *                     JFT_PC_AMIGA Infile will point to a string containing
 *                     a valid PC filespec as documented in AWRITE.DOC.      
 * char *Outfile     - Pointer to NULL terminated string which is the name of
 *                     the output file to open. If Direction==JFT_AMIGA_PC
 *                     Infile will point to a string containing a valid MS-DOS
 *                     filespec as documented in AREAD.DOC. If Direction==
 *                     JFT_PC_AMIGA Infile will point to a valid AmigaDOS
 *                     filespec as documented in AWRITE.DOC.
 *           
 * int  Direction    - Direction flag from jftrans.h
 *                     JFT_AMIGA_PC - Indicates that the transfer is from the
 *                     Amiga to the PC so infile will be
 *                     an Amiga filespec and Outfile will be
 *                     an MS-DOS filespec.
 *                     JFT_PC_AMIGA - Indicates that the transfer is from the
 *                     PC to the Amiga. Infile will be a MS-DOS
 *                     filespec and Outfile will be an Amiga
 *                     filespec.
 * int  Mode         - Mode flag from jftrans.h
 *                     JFT_CRLF   - Causes CRLF conversions to take place.
 *                     JFT_BINARY - Suppresses CRLF conversions.
 * char *Transtable  - Pointer to an optional character array to be used for
 *                     character translations. The format for the array is
 *                     char table[256] = {
 *                        0xnn,      Entry 0 
 *                        0xnn,      Entry 1 
 *                          .        .   
 *                          .        .   
 *                          .        .   
 *                        0xnn,      Entry 254 
 *                        0xnn };    Entry 255 
 *                     j_file_transfer() uses the following line to perform
 *                     its translations:
 *                     writechar = transtable[readchar];
 *                     so the input character is used as an index into the
 *                     table and the and the character contained in that entry
 *                     is the character sent as output. In this way all 255
 *                     ASCII characters can be converted. If a NULL is provided
 *                     for this field the default translation tables are used.
 *                     The default translations are documented in AREAD.DOC and
 *                     AWRITE.DOC.
 * int  Convert      - Conversion flag from jftrans.h
 *                     JFT_CONVERT   - Convert characters according to
 *                     Transtable provided or use defaults if NULL given as
 *                     Transtable.
 *                     JFT_NO_CONVERT - Perform no conversion of characters.
 *                     Character value written will be the
 *                     same as that read.
 * 
 * OUTPUT:
 * 
 * int  Error Error flag from jftrans.h
 *       JFT_ERR_NOERR                   - Indicates a successful transfer.
 *       JFT_ERR_INVALID_MODE            - An invalid mode was specified
 *                                         Valid modes are JFT_CRLF and
 *                                         JFT_BINARY.
 *       JFT_ERR_INVALID_DIRECTION       - An invalid direction was
 *                                         specified. Valid directions
 *                                         are JFT_PC_AMIGA and
 *                                         JFT_AMIGA_PC.
 *       JFT_ERR_NO_SERVER               - The Amiga file server, PCDisk was
 *                                         not running.
 *       JFT_ERR_PC_OPEN                 - The PC file could not be opened.        
 *       JFT_ERR_AMIGA_OPEN              - The Amiga file could not be
 *                                         opened.
 *       JFT_ERR_AMIGA_READ              - There was an error while reading
 *                                         from the Amiga.
 *       JFT_ERR_AMIGA_WRITE             - There was an error while writing
 *                                         to the Amiga.     
 *       JFT_ERR_INVALID_CONVERSION_MODE - An invalid conversion mode was
 *                                         specified, Valid modes are
 *                                         JFT_CONVERT and JFT_NO_CONVERT.
 * 
 * 
 * 
 ************************************************************************/
                                                                                
#define LINT_ARGS                         /* Enable type checking       */
#include <stdio.h>
#include <dos.h>
#undef NULL
#include <janus/janus.h>
#include <janus/jftrans.h> 
#include "januslib.h"
                                               
#define   FALSE      0

unsigned char * buffer;            /* Initialised as a far ptr   */
                                          /* by amigainit in jint.c     */
struct AmigaDiskReq *paramptr;

static	short File,Off_h,Off_l; 
static unsigned char tbuff[PCDISK_BUFFER_SIZE];

int jft_writeblock(char *,int,FILE *,int,int *);
int jft_readblock(char *,int,FILE *,int,int *);
int jft_translate(unsigned char *,int,unsigned char *);
unsigned char amigainit();
short CAmiga();
                     
/************************************************************************
 *
 * j_file_transfer(infile, outfile, direction, mode, transtable)
 *
 * Transfer a file from PC->Amiga or Amiga->PC
 *
 *   Infile    -  File to read
 *   Outfile   -  File to write
 *   Direction -  JFT_PC_AMIGA or JFT_AMIGA_PC
 * Mode        -  JFT_BINARY or JFT_CR_LF
 * Transtable  -  One dimensional array of unsigned chars where input
 *                char is used as index and char from that entry is sent
 *                as output char. Allows user defined translation tables.
 *                If Transtable == NULL default table is used.
 *
 ************************************************************************/
int j_file_transfer(infile,outfile,direction,mode,transtable,convert)
char    *infile, *outfile;
int     direction,mode;
unsigned char    *transtable;
int convert;
{
   FILE           *localfile;
   int             rc;
   char             modestr[3];
   int            status;

/* Conversion table translates from Amiga to IBM character set       */
/* PC char is index,    entry is char to send to Amiga                */

static unsigned char pc_amiga_table[256] = {
   0x00,   /* Entry 00   */
   0x01,   /* Entry 01   */
   0x02,   /* Entry 02   */
   0x03,   /* Entry 03   */
   0x04,   /* Entry 04   */
   0x05,   /* Entry 05   */
   0x06,   /* Entry 06   */
   0x07,   /* Entry 07   */
   0x08,   /* Entry 08   */
   0x09,   /* Entry 09   */
   0x0a,   /* Entry 0a   */
   0x0b,   /* Entry 0b   */
   0x0c,   /* Entry 0c   */
   0x0d,   /* Entry 0d   */
   0x0e,   /* Entry 0e   */
   0x0f,   /* Entry 0f   */
   0x10,   /* Entry 10   */
   0x11,   /* Entry 11   */
   0x12,   /* Entry 12   */
   0x13,   /* Entry 13   */
   0x14,   /* Entry 14   */
   0x15,   /* Entry 15   */
   0x16,   /* Entry 16   */
   0x17,   /* Entry 17   */
   0x18,   /* Entry 18   */
   0x19,   /* Entry 19   */
   0x1a,   /* Entry 1a   */
   0x1b,   /* Entry 1b   */
   0x1c,   /* Entry 1c   */
   0x1d,   /* Entry 1d   */
   0x1e,   /* Entry 1e   */
   0x1f,   /* Entry 1f   */
   0x20,   /* Entry 20   */
   0x21,   /* Entry 21   */
   0x22,   /* Entry 22   */
   0x23,   /* Entry 23   */
   0x24,   /* Entry 24   */
   0x25,   /* Entry 25   */
   0x26,   /* Entry 26   */
   0x27,   /* Entry 27   */
   0x28,   /* Entry 28   */
   0x29,   /* Entry 29   */
   0x2a,   /* Entry 2a   */
   0x2b,   /* Entry 2b   */
   0x2c,   /* Entry 2c   */
   0x2d,   /* Entry 2d   */
   0x2e,   /* Entry 2e   */
   0x2f,   /* Entry 2f   */
   0x30,   /* Entry 30   */
   0x31,   /* Entry 31   */
   0x32,   /* Entry 32   */
   0x33,   /* Entry 33   */
   0x34,   /* Entry 34   */
   0x35,   /* Entry 35   */
   0x36,   /* Entry 36   */
   0x37,   /* Entry 37   */
   0x38,   /* Entry 38   */
   0x39,   /* Entry 39   */
   0x3a,   /* Entry 3a   */
   0x3b,   /* Entry 3b   */
   0x3c,   /* Entry 3c   */
   0x3d,   /* Entry 3d   */
   0x3e,   /* Entry 3e   */
   0x3f,   /* Entry 3f   */
   0x40,   /* Entry 40   */
   0x41,   /* Entry 41   */
   0x42,   /* Entry 42   */
   0x43,   /* Entry 43   */
   0x44,   /* Entry 44   */
   0x45,   /* Entry 45   */
   0x46,   /* Entry 46   */
   0x47,   /* Entry 47   */
   0x48,   /* Entry 48   */
   0x49,   /* Entry 49   */
   0x4a,   /* Entry 4a   */
   0x4b,   /* Entry 4b   */
   0x4c,   /* Entry 4c   */
   0x4d,   /* Entry 4d   */
   0x4e,   /* Entry 4e   */
   0x4f,   /* Entry 4f   */
   0x50,   /* Entry 50   */
   0x51,   /* Entry 51   */
   0x52,   /* Entry 52   */
   0x53,   /* Entry 53   */
   0x54,   /* Entry 54   */
   0x55,   /* Entry 55   */
   0x56,   /* Entry 56   */
   0x57,   /* Entry 57   */
   0x58,   /* Entry 58   */
   0x59,   /* Entry 59   */
   0x5a,   /* Entry 5a   */
   0x5b,   /* Entry 5b   */
   0x5c,   /* Entry 5c   */
   0x5d,   /* Entry 5d   */
   0x5e,   /* Entry 5e   */
   0x5f,   /* Entry 5f   */
   0x60,   /* Entry 60   */
   0x61,   /* Entry 61   */
   0x62,   /* Entry 62   */
   0x63,   /* Entry 63   */
   0x64,   /* Entry 64   */
   0x65,   /* Entry 65   */
   0x66,   /* Entry 66   */
   0x67,   /* Entry 67   */
   0x68,   /* Entry 68   */
   0x69,   /* Entry 69   */
   0x6a,   /* Entry 6a   */
   0x6b,   /* Entry 6b   */
   0x6c,   /* Entry 6c   */
   0x6d,   /* Entry 6d   */
   0x6e,   /* Entry 6e   */
   0x6f,   /* Entry 6f   */
   0x70,   /* Entry 70   */
   0x71,   /* Entry 71   */
   0x72,   /* Entry 72   */
   0x73,   /* Entry 73   */
   0x74,   /* Entry 74   */
   0x75,   /* Entry 75   */
   0x76,   /* Entry 76   */
   0x77,   /* Entry 77   */
   0x78,   /* Entry 78   */
   0x79,   /* Entry 79   */
   0x7a,   /* Entry 7a   */
   0x7b,   /* Entry 7b   */
   0x7c,   /* Entry 7c   */
   0x7d,   /* Entry 7d   */
   0x7e,   /* Entry 7e   */
   0x7f,   /* Entry 7f   */
   0xc7,   /* Entry 80      */
   0xfc,   /* Entry 81      */
   0xe9,   /* Entry 82      */
   0xe2,   /* Entry 83      */
   0xe4,   /* Entry 84      */
   0xe0,   /* Entry 85      */
   0xe5,   /* Entry 86      */
   0xe7,   /* Entry 87      */
   0xea,   /* Entry 88      */
   0xeb,   /* Entry 89      */
   0xe8,   /* Entry 8a      */
   0xef,   /* Entry 8b      */
   0xee,   /* Entry 8c      */
   0xec,   /* Entry 8d      */
   0xc4,   /* Entry 8e      */
   0xc5,   /* Entry 8f      */
   0xc9,   /* Entry 90      */
   0xe6,   /* Entry 91      */
   0xc6,   /* Entry 92      */
   0xf4,   /* Entry 93      */
   0xf6,   /* Entry 94      */
   0xf2,   /* Entry 95      */
   0xfb,   /* Entry 96      */
   0xf9,   /* Entry 97      */
   0xff,   /* Entry 98      */
   0xd6,   /* Entry 99      */
   0xdc,   /* Entry 9a      */
   0xa2,   /* Entry 9b      */
   0xa3,   /* Entry 9c      */
   0xa5,   /* Entry 9d      */
   0x7f,   /* Entry 9e      */
   0x7f,   /* Entry 9f      */
   0xe1,   /* Entry a0      */
   0xed,   /* Entry a1      */
   0xf3,   /* Entry a2      */
   0xfa,   /* Entry a3      */
   0xf1,   /* Entry a4      */
   0xd1,   /* Entry a5      */
   0xaa,   /* Entry a6      */
   0xba,   /* Entry a7      */
   0xbf,   /* Entry a8      */
   0x7f,   /* Entry a9      */
   0xac,   /* Entry aa      */
   0xbd,   /* Entry ab      */
   0xbc,   /* Entry ac      */
   0xa1,   /* Entry ad      */
   0xab,   /* Entry ae      */
   0xbb,   /* Entry af      */
   0x7f,   /* Entry b0      */
   0x7f,   /* Entry b1      */
   0x7f,   /* Entry b2      */
   0x7f,   /* Entry b3      */
   0x7f,   /* Entry b4      */
   0x7f,   /* Entry b5      */
   0x7f,   /* Entry b6      */
   0x7f,   /* Entry b7      */
   0x7f,   /* Entry b8      */
   0x7f,   /* Entry b9      */
   0x7f,   /* Entry ba      */
   0x7f,   /* Entry bb      */
   0x7f,   /* Entry bc      */
   0x7f,   /* Entry bd      */
   0x7f,   /* Entry be      */
   0x7f,   /* Entry bf      */
   0x7f,   /* Entry c0      */
   0x7f,   /* Entry c1      */
   0x7f,   /* Entry c2      */
   0x7f,   /* Entry c3      */
   0x7f,   /* Entry c4      */
   0x7f,   /* Entry c5      */
   0x7f,   /* Entry c6      */
   0x7f,   /* Entry c7      */
   0x7f,   /* Entry c8      */
   0x7f,   /* Entry c9      */
   0x7f,   /* Entry ca      */
   0x7f,   /* Entry cb      */
   0x7f,   /* Entry cc      */
   0x7f,   /* Entry cd      */
   0x7f,   /* Entry ce      */
   0x7f,   /* Entry cf      */
   0x7f,   /* Entry d0      */
   0x7f,   /* Entry d1      */
   0x7f,   /* Entry d2      */
   0x7f,   /* Entry d3      */
   0x7f,   /* Entry d4      */
   0x7f,   /* Entry d5      */
   0x7f,   /* Entry d6      */
   0x7f,   /* Entry d7      */
   0x7f,   /* Entry d8      */
   0x7f,   /* Entry d9      */
   0x7f,   /* Entry da      */
   0x7f,   /* Entry db      */
   0x7f,   /* Entry dc      */
   0x7f,   /* Entry dd      */
   0x7f,   /* Entry de      */
   0x7f,   /* Entry df      */
   0x7f,   /* Entry e0      */
   0xdf,   /* Entry e1      */
   0x7f,   /* Entry e2      */
   0x7f,   /* Entry e3      */
   0x7f,   /* Entry e4      */
   0x7f,   /* Entry e5      */
   0xb5,   /* Entry e6      */
   0x7f,   /* Entry e7      */
   0x7f,   /* Entry e8      */
   0x7f,   /* Entry e9      */
   0x7f,   /* Entry ea      */
   0xf0,   /* Entry eb      */
   0x7f,   /* Entry ec      */
   0xf8,   /* Entry ed      */
   0x7f,   /* Entry ee      */
   0x7f,   /* Entry ef      */
   0x7f,   /* Entry f0      */
   0xb1,   /* Entry f1      */
   0x7f,   /* Entry f2      */
   0x7f,   /* Entry f3      */
   0x7f,   /* Entry f4      */
   0x7f,   /* Entry f5      */
   0xf7,   /* Entry f6      */
   0x7f,   /* Entry f7      */
   0xb0,   /* Entry f8      */
   0xb7,   /* Entry f9      */
   0x7f,   /* Entry fa      */
   0x7f,   /* Entry fb      */
   0x7f,   /* Entry fc      */
   0xb2,   /* Entry fd      */
   0xaf,   /* Entry fe      */
   0x7f    /* Entry ff      */
};

/* Conversion table translates from Amiga to IBM character set       */
/* Amiga char is index, entry is char to send to PC                   */

static unsigned char amiga_pc_table[256] = {
   0x00,   /* Entry 00   */
   0x01,   /* Entry 01   */
   0x02,   /* Entry 02   */
   0x03,   /* Entry 03   */
   0x04,   /* Entry 04   */
   0x05,   /* Entry 05   */
   0x06,   /* Entry 06   */
   0x07,   /* Entry 07   */
   0x08,   /* Entry 08   */
   0x09,   /* Entry 09   */
   0x0a,   /* Entry 0a   */
   0x0b,   /* Entry 0b   */
   0x0c,   /* Entry 0c   */
   0x0d,   /* Entry 0d   */
   0x0e,   /* Entry 0e   */
   0x0f,   /* Entry 0f   */
   0x10,   /* Entry 10   */
   0x11,   /* Entry 11   */
   0x12,   /* Entry 12   */
   0x13,   /* Entry 13   */
   0x14,   /* Entry 14   */
   0x15,   /* Entry 15   */
   0x16,   /* Entry 16   */
   0x17,   /* Entry 17   */
   0x18,   /* Entry 18   */
   0x19,   /* Entry 19   */
   0x1a,   /* Entry 1a   */
   0x1b,   /* Entry 1b   */
   0x1c,   /* Entry 1c   */
   0x1d,   /* Entry 1d   */
   0x1e,   /* Entry 1e   */
   0x1f,   /* Entry 1f   */
   0x20,   /* Entry 20   */
   0x21,   /* Entry 21   */
   0x22,   /* Entry 22   */
   0x23,   /* Entry 23   */
   0x24,   /* Entry 24   */
   0x25,   /* Entry 25   */
   0x26,   /* Entry 26   */
   0x27,   /* Entry 27   */
   0x28,   /* Entry 28   */
   0x29,   /* Entry 29   */
   0x2a,   /* Entry 2a   */
   0x2b,   /* Entry 2b   */
   0x2c,   /* Entry 2c   */
   0x2d,   /* Entry 2d   */
   0x2e,   /* Entry 2e   */
   0x2f,   /* Entry 2f   */
   0x30,   /* Entry 30   */
   0x31,   /* Entry 31   */
   0x32,   /* Entry 32   */
   0x33,   /* Entry 33   */
   0x34,   /* Entry 34   */
   0x35,   /* Entry 35   */
   0x36,   /* Entry 36   */
   0x37,   /* Entry 37   */
   0x38,   /* Entry 38   */
   0x39,   /* Entry 39   */
   0x3a,   /* Entry 3a   */
   0x3b,   /* Entry 3b   */
   0x3c,   /* Entry 3c   */
   0x3d,   /* Entry 3d   */
   0x3e,   /* Entry 3e   */
   0x3f,   /* Entry 3f   */
   0x40,   /* Entry 40   */
   0x41,   /* Entry 41   */
   0x42,   /* Entry 42   */
   0x43,   /* Entry 43   */
   0x44,   /* Entry 44   */
   0x45,   /* Entry 45   */
   0x46,   /* Entry 46   */
   0x47,   /* Entry 47   */
   0x48,   /* Entry 48   */
   0x49,   /* Entry 49   */
   0x4a,   /* Entry 4a   */
   0x4b,   /* Entry 4b   */
   0x4c,   /* Entry 4c   */
   0x4d,   /* Entry 4d   */
   0x4e,   /* Entry 4e   */
   0x4f,   /* Entry 4f   */
   0x50,   /* Entry 50   */
   0x51,   /* Entry 51   */
   0x52,   /* Entry 52   */
   0x53,   /* Entry 53   */
   0x54,   /* Entry 54   */
   0x55,   /* Entry 55   */
   0x56,   /* Entry 56   */
   0x57,   /* Entry 57   */
   0x58,   /* Entry 58   */
   0x59,   /* Entry 59   */
   0x5a,   /* Entry 5a   */
   0x5b,   /* Entry 5b   */
   0x5c,   /* Entry 5c   */
   0x5d,   /* Entry 5d   */
   0x5e,   /* Entry 5e   */
   0x5f,   /* Entry 5f   */
   0x60,   /* Entry 60   */
   0x61,   /* Entry 61   */
   0x62,   /* Entry 62   */
   0x63,   /* Entry 63   */
   0x64,   /* Entry 64   */
   0x65,   /* Entry 65   */
   0x66,   /* Entry 66   */
   0x67,   /* Entry 67   */
   0x68,   /* Entry 68   */
   0x69,   /* Entry 69   */
   0x6a,   /* Entry 6a   */
   0x6b,   /* Entry 6b   */
   0x6c,   /* Entry 6c   */
   0x6d,   /* Entry 6d   */
   0x6e,   /* Entry 6e   */
   0x6f,   /* Entry 6f   */
   0x70,   /* Entry 70   */
   0x71,   /* Entry 71   */
   0x72,   /* Entry 72   */
   0x73,   /* Entry 73   */
   0x74,   /* Entry 74   */
   0x75,   /* Entry 75   */
   0x76,   /* Entry 76   */
   0x77,   /* Entry 77   */
   0x78,   /* Entry 78   */
   0x79,   /* Entry 79   */
   0x7a,   /* Entry 7a   */
   0x7b,   /* Entry 7b   */
   0x7c,   /* Entry 7c   */
   0x7d,   /* Entry 7d   */
   0x7e,   /* Entry 7e   */
   0x7f,   /* Entry 7f   */
   0x7f,   /* Entry 80      */
   0x7f,   /* Entry 81      */
   0x7f,   /* Entry 82      */
   0x7f,   /* Entry 83      */
   0x7f,   /* Entry 84      */
   0x7f,   /* Entry 85      */
   0x7f,   /* Entry 86      */
   0x7f,   /* Entry 87      */
   0x7f,   /* Entry 88      */
   0x7f,   /* Entry 89      */
   0x7f,   /* Entry 8a      */
   0x7f,   /* Entry 8b      */
   0x7f,   /* Entry 8c      */
   0x7f,   /* Entry 8d      */
   0x7f,   /* Entry 8e      */
   0x7f,   /* Entry 8f      */
   0x7f,   /* Entry 90      */
   0x7f,   /* Entry 91      */
   0x7f,   /* Entry 92      */
   0x7f,   /* Entry 93      */
   0x7f,   /* Entry 94      */
   0x7f,   /* Entry 95      */
   0x7f,   /* Entry 96      */
   0x7f,   /* Entry 97      */
   0x7f,   /* Entry 98      */
   0x7f,   /* Entry 99      */
   0x7f,   /* Entry 9a      */
   0x7f,   /* Entry 9b      */
   0x7f,   /* Entry 9c      */
   0x7f,   /* Entry 9d      */
   0x7f,   /* Entry 9e      */
   0x7f,   /* Entry 9f      */
   0x7f,   /* Entry a0      */
   0xad,   /* Entry a1      */
   0x9b,   /* Entry a2      */
   0x9c,   /* Entry a3      */
   0x7f,   /* Entry a4      */
   0x9d,   /* Entry a5      */
   0x7f,   /* Entry a6      */
   0x7f,   /* Entry a7      */
   0x7f,   /* Entry a8      */
   0x7f,   /* Entry a9      */
   0xa6,   /* Entry aa      */
   0xae,   /* Entry ab      */
   0xaa,   /* Entry ac      */
   0x7f,   /* Entry ad      */
   0x7f,   /* Entry ae      */
   0xfe,   /* Entry af      */
   0xf8,   /* Entry b0      */
   0xf1,   /* Entry b1      */
   0xfd,   /* Entry b2      */
   0x7f,   /* Entry b3      */
   0x7f,   /* Entry b4      */
   0xe6,   /* Entry b5      */
   0x7f,   /* Entry b6      */
   0xf9,   /* Entry b7      */
   0x7f,   /* Entry b8      */
   0x7f,   /* Entry b9      */
   0xa7,   /* Entry ba      */
   0xaf,   /* Entry bb      */
   0xac,   /* Entry bc      */
   0xab,   /* Entry bd      */
   0x7f,   /* Entry be      */
   0xa8,   /* Entry bf      */
   0x7f,   /* Entry c0      */
   0x7f,   /* Entry c1      */
   0x7f,   /* Entry c2      */
   0x7f,   /* Entry c3      */
   0x8e,   /* Entry c4      */
   0x8f,   /* Entry c5      */
   0x92,   /* Entry c6      */
   0x80,   /* Entry c7      */
   0x7f,   /* Entry c8      */
   0x90,   /* Entry c9      */
   0x7f,   /* Entry ca      */
   0x7f,   /* Entry cb      */
   0x7f,   /* Entry cc      */
   0x7f,   /* Entry cd      */
   0x7f,   /* Entry ce      */
   0x7f,   /* Entry cf      */
   0x7f,   /* Entry d0      */
   0xa5,   /* Entry d1      */
   0x7f,   /* Entry d2      */
   0x7f,   /* Entry d3      */
   0x7f,   /* Entry d4      */
   0x7f,   /* Entry d5      */
   0x99,   /* Entry d6      */
   0x7f,   /* Entry d7      */
   0x7f,   /* Entry d8      */
   0x7f,   /* Entry d9      */
   0x7f,   /* Entry da      */
   0x7f,   /* Entry db      */
   0x9a,   /* Entry dc      */
   0x7f,   /* Entry dd      */
   0x7f,   /* Entry de      */
   0xe1,   /* Entry df      */
   0x85,   /* Entry e0      */
   0xa0,   /* Entry e1      */
   0x83,   /* Entry e2      */
   0x7f,   /* Entry e3      */
   0x84,   /* Entry e4      */
   0x86,   /* Entry e5      */
   0x91,   /* Entry e6      */
   0x87,   /* Entry e7      */
   0x8a,   /* Entry e8      */
   0x82,   /* Entry e9      */
   0x88,   /* Entry ea      */
   0x89,   /* Entry eb      */
   0x8d,   /* Entry ec      */
   0xa1,   /* Entry ed      */
   0x8c,   /* Entry ee      */
   0x8b,   /* Entry ef      */
   0xeb,   /* Entry f0      */
   0xa4,   /* Entry f1      */
   0x95,   /* Entry f2      */
   0xa2,   /* Entry f3      */
   0x93,   /* Entry f4      */
   0x7f,   /* Entry f5      */
   0x94,   /* Entry f6      */
   0xf6,   /* Entry f7      */
   0xed,   /* Entry f8      */
   0x97,   /* Entry f9      */
   0xa3,   /* Entry fa      */
   0x96,   /* Entry fb      */
   0x81,   /* Entry fc      */
   0x7f,   /* Entry fd      */
   0x7f,   /* Entry fe      */
   0x98   /* Entry ff      */
};

   if(mode!=JFT_BINARY && mode!=JFT_CR_LF)
      return(JFT_ERR_INVALID_MODE);

   if(direction!=JFT_PC_AMIGA && direction!=JFT_AMIGA_PC)
      return(JFT_ERR_INVALID_DIRECTION);

   if(convert!=JFT_CONVERT && convert!=JFT_NO_CONVERT)
      return(JFT_ERR_INVALID_CONVERSION_MODE);

   if(mode==JFT_BINARY)                   /* build string for fopen     */
      modestr[1]='b';                     /* wt, wb, rt, rb             */
   else
      modestr[1]='t';
   
   if(direction==JFT_AMIGA_PC)
      modestr[0]='w';
   else
      modestr[0]='r';

   modestr[2]='\000';                     /* NULL terminate string      */

   if(direction==JFT_AMIGA_PC)            /* open appropriate PC file   */
      localfile=fopen(outfile,modestr);
   else
      localfile=fopen(infile,modestr); 

     if(localfile==NULL)                  /* could not open PC file     */
      return(JFT_ERR_PC_OPEN);
                                                                             


 
    
   /* Locate the parameter buffer area */

   rc = amigainit();
   if (rc != JSERV_OK)
   {
      fclose(localfile);
      return(JFT_ERR_NO_SERVER);   
   }
    
 
   /* Place the filename in the buffer area and request an open */

   if(direction==JFT_AMIGA_PC)         /* open appropriate PC file      */
   {
      strcpy(buffer,infile);
      rc = CAmiga(ADR_FNCTN_OPEN_OLD,0);/* Request to PCDisk to open     */
                                       /* Amiga file                    */
   } else {
      strcpy(buffer,outfile);
      rc = CAmiga(ADR_FNCTN_OPEN_NEW,0);/* Request to PCDisk to open     */
                                       /* Amiga file                    */
     }
 
   if ( rc != JSERV_OK)
   {
      fclose(localfile);
      return(JFT_ERR_AMIGA_OPEN);
   }

	File  = paramptr->adr_File;
	Off_h = paramptr->adr_Offset_h;
	Off_l = paramptr->adr_Offset_l;

   /* Copy from infile to outfile */

   while((rc=jft_readblock(tbuff,PCDISK_BUFFER_SIZE,localfile,direction,&status))!=0)
   {
      if(status!=JFT_NOERROR)
         break;
      if(convert==JFT_CONVERT)
      {
         if(transtable)
            jft_translate(tbuff,rc,transtable);
         else
            jft_translate(tbuff,rc,direction ? amiga_pc_table : pc_amiga_table);
      }
      jft_writeblock(tbuff,rc,localfile,direction,&status);
      if(status!=JFT_NOERROR)
         break;
   }
	paramptr->adr_File = File;   
   CAmiga(ADR_FNCTN_CLOSE,0);              /* Request to PCDisk, close file   */
   fclose(localfile);
   return(status);
}                                       /* End main                        */

int jft_writeblock(buff,size,localfile,direction,status)
char *buff;
int    size;
FILE   *localfile;
int    direction;
int    *status;
{
   int writecount;

   *status=JFT_NOERROR;                  /* assume OK                     */

   if(direction==JFT_AMIGA_PC)
      return(fwrite(buff,1,size,localfile));

	paramptr->adr_File = File;
	paramptr->adr_Offset_h = Off_h;
	paramptr->adr_Offset_l = Off_l;

	{	int x;
		for(x=0;x<PCDISK_BUFFER_SIZE;x++)
			buffer[x]=buff[x];
	}
   writecount=CAmiga(ADR_FNCTN_WRITE,size);
   if(writecount<0)
   {
      *status=JFT_ERR_AMIGA_WRITE;
      return(0);
   }
	Off_h = paramptr->adr_Offset_h;
	Off_l = paramptr->adr_Offset_l;
   return(writecount);   
}
int jft_readblock(buff,size,localfile,direction,status)
char *buff;
int    size;
FILE   *localfile;
int    direction;
int    *status;
{
   int readcount;

   *status=JFT_NOERROR;                  /* assume OK                     */

   if(direction==JFT_PC_AMIGA)
      return(fread(buff,1,size,localfile));

	paramptr->adr_File = File;
	paramptr->adr_Offset_h = Off_h;
	paramptr->adr_Offset_l = Off_l;

   readcount=CAmiga(ADR_FNCTN_READ,size);
   if(readcount<0)
   {
      *status=JFT_ERR_AMIGA_READ;
      return(0);
   }
	{	int x;
		for(x=0;x<PCDISK_BUFFER_SIZE;x++)
			buff[x]=buffer[x];
	}
	Off_h = paramptr->adr_Offset_h;
	Off_l = paramptr->adr_Offset_l;
   return(readcount);   
}
int jft_translate(buf,count,table)
unsigned char *buf;
int count;
unsigned char *table;
{
   int x;

   for(x=0;x<count;x++)
      buf[x]=table[buf[x]];
} 
 
/***************************************************************************
 *																								  	
 *	JINT.C	-	Support routines for INT 0B Janus calls
 *
 * Tim King March 1987                                          
 *
 * MODIFIED for MICROSOFT-C 01-25-88 by Bill Koester (CATS) West Chester
 *
 ***************************************************************************/
																						  				
 

 
/***************************************************************************
 *
 *	amigainit()			-	Initialize the Amiga interface. Returns nonzero
 *								on error.
 *
 ***************************************************************************/ 
unsigned char amigainit()																 		
{
  int rc, i;
  unsigned int j_param_seg, j_param_offset, j_buffer_seg;
  union REGS r;
  struct SREGS sr;
  unsigned long temp;
	UBYTE	error;
 
  /* This is the first item to be called. Therefore call it twice
     because the handler may not be installed first time (!) */

  for (i=0;i<2;i++)
  {
	 error=GetBase(JSERV_PCDISK,&j_param_seg,&j_param_offset,&j_buffer_seg);
  }
 
  if (error != JSERV_OK)
  {
     return(JSERV_NOSERVICE); 					/* Error no Amiga service			*/
  }

 
  if (j_param_offset == -1)
  {
     return(JSERV_NOSERVICE);   					/* Error no Amiga service 			*/
  }

  /* Convert offset & segment into far pointer so we can use it 				*/
  /* WARNING - CAUTION - DANGER !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
  /* Braindead MICROSOFT-C implements far pointers as a segment/offset 		*/
  /* pair contained in a 32 bit long [segment ptr : offset ] so to get at	*/
  /* the real address F00A8 your far pointer must contain F00000A8 like the*/
  /* segment offset pair F000:00A8. It seems that MICROSOFT-C would rather	*/
  /* be efficient than compatible or correct!!! Arrgh.							*/

  paramptr = (struct AmigaDiskReq *)((unsigned long)
                                     (((unsigned long)j_param_seg) << 16) +
                                     (unsigned long)j_param_offset );

  buffer = (char *) ((unsigned long)
							((unsigned long)j_buffer_seg << 16) +
                     (unsigned long)(paramptr->adr_BufferOffset));
 
  /* Perform some initialisation */
  paramptr -> adr_Offset_h = 0;
  paramptr -> adr_Offset_l = 0;
  paramptr -> adr_Count_h  = 0;
 
  return (JSERV_OK);
}
 
/***************************************************************************
 *
 * CAmiga(code,count)	-	Perform Amiga service return:
 *																	-1			error
 *																	0			EOF
 *																	actual 	otherwise
 *
 ***************************************************************************/ 
short CAmiga(code,count)													 	
unsigned short code, count;
{
  unsigned char rc;

  paramptr->adr_Fnctn = code;
  paramptr->adr_Count_h = 0;
  paramptr->adr_Count_l = count;

  rc = CallAmiga(JSERV_PCDISK);
  if (rc) return(rc); /* Error - no service*/
														  		
  rc = WaitAmiga(JSERV_PCDISK);
/*
	printf("Error from J_WAIT = %d\n",rc);
*/
  if (rc!=JSERV_FINISHED) return(-1);  	/* Error - not terminated 			*/

  rc = (unsigned char)(paramptr->adr_Err & 0xff);
  if (rc)
  {
/*
	printf("adr_Err = %d\n",rc);
*/
    return( -1 );        						/* Error on Amiga 					*/
  }

  if (code==ADR_FNCTN_READ || code==ADR_FNCTN_WRITE)
  {
    return( paramptr->adr_Count_l );
  }

  return(JSERV_OK);
}

