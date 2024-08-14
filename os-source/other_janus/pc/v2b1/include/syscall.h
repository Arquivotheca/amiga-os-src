#ifndef JANUS_SYSCALL_H
#define JANUS_SYSCALL_H

/************************************************************************
 * (PC Side File)
 *
 * syscall.h -- interface definitions between amiga and commodore-pc
 *
 * Copyright (c) 1986, Commodore Amiga Inc., All rights reserved
 *
 * 7-15-88 - Bill Koester - Modified for self inclusion of required files
 ************************************************************************/

#ifndef EXEC_TYPES_H
#include "exec/types.h"
#endif


/*
 * All registers in this section are arranged to be read and written
 * from the WordAccessOffset area of the shared memory.   If you really
 * need to use the ByteAccessArea, all the words will need to be byte
 * swapped.
 */


/* Syscall86 -- how the 8086/8088 wants its parameter block arranged    */

struct Syscall86 {

   UWORD    s86_AX;
   UWORD    s86_BX;
   UWORD    s86_CX;
   UWORD    s86_DX;
   UWORD    s86_SI;
   UWORD    s86_DS;
   UWORD    s86_DI;
   UWORD    s86_ES;
   UWORD    s86_BP;
   UWORD    s86_PSW;
   UWORD    s86_INT;          /* 8086 int # that will be called         */
   };




/* Syscall68 -- the way the 68000 wants its parameters arranged         */

struct Syscall68 {

   ULONG    s68_D0;
   ULONG    s68_D1;
   ULONG    s68_D2;
   ULONG    s68_D3;
   ULONG    s68_D4;
   ULONG    s68_D5;
   ULONG    s68_D6;
   ULONG    s68_D7;
   ULONG    s68_A0;
   ULONG    s68_A1;
   ULONG    s68_A2;
   ULONG    s68_A3;
   ULONG    s68_A4;
   ULONG    s68_A5;
   ULONG    s68_A6;
   ULONG    s68_PC;        /* pc to start execution from                 */
   ULONG    s68_ArgStack;  /* array to be pushed onto stack              */
   ULONG    s68_ArgLength; /* number of bytes to be pushed (must be even)*/
   ULONG    s68_MinStack;  /* minimum necessary stack (0 = use default)  */
   ULONG    s68_CCR;       /* condition code register                    */
   ULONG    s68_Process;   /* ptr to process for this block.             */
   UWORD    s68_Command;   /* special commands: see below                */
   UWORD    s68_Status;
   UWORD    s68_SigNum;    /* internal use: signal to wake up process    */
   };



#define S68COM_DOCALL  0 /* normal case -- jsr to specified Program cntr */
#define S68COM_REMPROC 1 /* kill process                                 */
#define S68COM_CRPROC  2 /* create the proces, but do not call anything  */



#endif /* End of JANUS_SYSCALL_H conditional compilation                 */



