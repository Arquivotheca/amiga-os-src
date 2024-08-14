#ifndef JANUS_MEMORY_H
#define JANUS_MEMORY_H

/************************************************************************
 * (PC Side File)
 *
 * Memory.h -- Structures and defines for Janus emeory
 *
 * Copyright (c) 1986, Commodore Amiga Inc.,  All rights reserved.
 *
 * 7-15-88 - Bill Koester - Modified for self inclusion of required files
 ************************************************************************/

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

typedef  UWORD RPTR;

/* magic constants for memory allocation                                */

#define MEM_TYPEMASK      0x00ff /* 8 memory areas                       */
#define MEMB_PARAMETER    (0)    /* parameter memory                     */
#define MEMB_BUFFER       (1)    /* buffer memory                        */

#define MEMF_PARAMETER    (1<<0) /* parameter memory                     */
#define MEMF_BUFFER       (1<<1) /* buffer memory                        */

#define MEM_ACCESSMASK    0x3000 /* bits that participate in access types*/
#define MEM_BYTEACCESS    0x0000 /* pointer to byte access address space */
#define MEM_WORDACCESS    0x1000 /* pointer to word access address space */
#define MEM_GRAPHICACCESS 0x2000 /* ptr to graphic access address space  */
#define MEM_IOACCESS      0x3000 /* pointer to i/o access address space  */

#define TYPEACCESSTOADDR  5      /* # of bits to turn access mask to addr*/

/*
 * The amiga side of the janus board has four sections of its address space.
 * Three of these parts are different arrangements of the same memory.  The
 * fourth part has the specific amiga accesible IO registers (jio_??).
 * The other three parts all contain the same data, but the data is arranged
 * in different ways: Byte Access lets the 68k read byte streams written
 * by the 8088, Word Access lets the 68k read word streams written by the
 * 8088, and Graphic Access lets the 68k read medium res graphics memory
 * in a more efficient manner (the pc uses packed two bit pixels ; graphic
 * access rearranges these data bits into two bytes, one for each bit plane).
 */

#define ByteAccessOffset    0x00000
#define WordAccessOffset    0x20000
#define GraphicAccessOffset 0x40000
#define IoAccessOffset      0x60000


/*
 * within each bank of memory are several sub regions.  These are the
 * definitions for the sub regions
 */

#define BufferOffset      0x00000
#define ColorOffset       0x10000
#define ParameterOffset   0x18000
#define MonoVideoOffset   0x1c000
#define IoRegOffset       0x1e000

#define BufferSize        0x10000
#define ParameterSize     0x04000

/* constants for sizes of various janus regions                            */

#define JANUSTOTALSIZE (512*1024) /* 1/2 megabyte                          */
#define JANUSBANKSIZE  (128*1024) /* 128K per memory bank                  */
#define JANUSNUMBANKS  (4)        /* four memory banks                     */
#define JANUSBANKMASK  (0x60000)  /* mask bits for bank region             */


/*
 * all bytes described here are described in the byte order of the 8088.
 * Note that words and longwords in these structures will be accessed from
 * the word access space to preserve the byte order in a word -- the 8088
 * will access longwords by reversing the words : like a 68000 access to the
 * word access memory
 *
 * JanusMemHead -- a data structure roughly analogous to an exec mem chunk.
 * It is used to keep track of memory used between the 8088 and the 68000.
 */


struct JanusMemHead {

   UBYTE    jmh_Lock;            /* lock byte between processors        */
   UBYTE    jmh_pad0;
   APTR     jmh_68000Base;       /* rptr's are relative to this         */
   UWORD    jmh_8088Segment;     /* segment base for 8088               */
   RPTR     jmh_First;           /* offset to first free chunk          */
   RPTR     jmh_Max;             /* max allowable index                 */
   UWORD    jmh_Free;            /* total number of free bytes -1       */
   };




struct JanusMemChunk {

   RPTR     jmc_Next;            /* rptr to next free chunk             */
   UWORD    jmc_Size;            /* size of chunk                       */
   };

/*
 * === ===================================================================== 
 * === JanusRemember Structure ============================================= 
 * === ===================================================================== 
 * This is the structure used for the JRemember memory management routines 
 */

struct JanusRemember {

   RPTR     jrm_NextRemember; /* Pointer to the next JanusRemember struct */
   RPTR     jrm_Offset;       /* Janus offset to this memory allocation   */
   USHORT   jrm_Size;         /* Size of this memory allocation           */
   USHORT   jrm_Type;         /* Type of this memory allocation           */
   };


#endif /* End of JANUS_MEMORY_H conditional compilation                   */
