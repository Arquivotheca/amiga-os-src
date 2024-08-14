#ifndef JANUS_JANUSVAR_H
#define JANUS_JANUSVAR_H

/************************************************************************
 * (Amiga side file)
 *
 * janusvar.h -- the software data structures for the janus board
 *
 * Copyright (c) 1986, Commodore Amiga Inc.,  All rights reserved.
 *
 * Date       Name
 * --------   -------------   -------------------------------------------
 * 07-15-88 - Bill Koester  - Modified for self inclusion of required files
 * 07-26-88 - Bill Koester  - Added ja_Reserved to JanusAmiga
 *                            Added ja_AmigaState, ja_PCState and related
 *                            flags to JanusAmiga
 * 10-05-88 - Bill Koester  - Added Ver/Rev fields to JanusAmiga struc
 * 10-06-88 - Bill Koester  - Added HandlerLoaded field to JanusAMiga
 * 11-08-88 - Bill Koester  - Added AMIGA_PC_READY flags
 * 07-09-88 - Bill Koester  - Added variables for software locking
 ************************************************************************/

#ifndef JANUS_MEMORY_H
#include <janus/memory.h>
#endif

/*
 * all bytes described here are described in the byte order of the 8088.
 * Note that words and longwords in these structures will be accessed from
 * the word access space to preserve the byte order in a word -- the 8088
 * will access longwords by reversing the words : like a 68000 access to the
 * word access memory
 */

struct JanusAmiga {

   UBYTE    ja_Lock;              /* also used to handshake at 8088 reset */
   UBYTE    ja_8088Go;            /* unlocked to signal 8088 to init      */
   struct JanusMemHead ja_ParamMem;
   struct JanusMemHead ja_BufferMem;
   RPTR     ja_Interrupts;
   RPTR     ja_Parameters;
   UWORD    ja_NumInterrupts;

   /* This field is used by janus.library to communicate Amiga states 
    * to the PC.  The bits of this field may be read by anyone, but 
    * may be written only by janus.library.  
    */
   USHORT   ja_AmigaState;

   /* This field is used by the PC to communicate PC states 
    * to the Amiga.  The bits of this field may be read by anyone, but 
    * may be written only by the PC operating system.  
    */
   USHORT   ja_PCState;

   /* These fields are set by janus.library and the PC janus handler so
    * they can read each others version numbers.
    */
   UWORD    ja_JLibRev;
   UWORD    ja_JLibVer;
   UWORD    ja_JHandlerRev;
   UWORD    ja_JHandlerVer;

   /* This field is zero before the PC is running and is set to nonzero
    * When the PC's JanusHandler has finished initializing.
    */
   UWORD    ja_HandlerLoaded;

   UBYTE    ja_PCFlag;
   UBYTE    ja_AmigaFlag;
   UBYTE    ja_Turn;
   UBYTE    ja_Pad;

   ULONG    ja_Reserved[4];
   };


/* === AmigaState Definitions === */
#define  AMIGASTATE_RESERVED  0xFFF8
#define  AMIGA_NUMLOCK_SET    0x0001
#define  AMIGA_NUMLOCK_SETn   0
#define  AMIGA_NUMLOCK_RESET  0x0002
#define  AMIGA_NUMLOCK_RESETn 1
#define  AMIGA_PC_READY       0x0004
#define  AMIGA_PC_READYn      2

/* === PCState Definitions === */
#define  PCSTATE_RESERVED     0xFFFF

/*------ constant to set to indicate a pending software interrupt       */

#define JSETINT  0x7f

#endif /* End of JANUS_JANUSVAR_H conditional compilation               */
