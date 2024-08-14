	TTL	'F0INIT.ASM'


*************************************************************************	
*                                                                    	*
*   Copyright 1990, Joseph Katz/Mark Barton.  All rights reserved.	*
*   No part of this program may be reproduced, transmitted, or stored   *
*   in any language or computer system, in any form, whatsoever,	*
*   without the prior written permission of the authors.   		*
*                                                                    	*
*************************************************************************

*************************************************************************
*                                                                    	*
*                        F0 CONTOUR INITIALIZATION                   	*
*                                                                    	*
*									*
*	All registers destroyed.					*
*									*
*   5/1/89  --  JK	PARSE inserts an initial QX into the phon,	*
*			stress, and dur arrays.  This first QX as well	*
*			as the next phon is marked as start of syllable *
*			which causes the count of number of syllables	*
*			to be 1 too high.  F0INIT used to just ignore	*
*			the first two blanks that PARSE inserted, now	*
*			it also ignores the initial QX.  See Note 1.	*
*									*
* 10/14/89  --  JK	Modified to dynamically allocate memory for 	*
*			the various F0 arrays.  If overflow, return	*
*			error ND_NoMem.					*
*									*
*************************************************************************

          SECTION      speech



	INCLUDE 'assembly.i'
 	INCLUDE 'exec/types.i'
	INCLUDE 'exec/nodes.i'
	INCLUDE 'exec/lists.i'
	INCLUDE 'exec/strings.i'
	INCLUDE 'exec/initializers.i'
	INCLUDE 'exec/memory.i'
	INCLUDE 'exec/interrupts.i'
	INCLUDE 'exec/ports.i'
	INCLUDE 'exec/libraries.i'
	INCLUDE 'exec/devices.i'
	INCLUDE 'exec/io.i'
	INCLUDE 'exec/tasks.i'
	INCLUDE 'exec/resident.i'
	INCLUDE 'hardware/custom.i'
	INCLUDE 'hardware/dmabits.i'
	INCLUDE 'exec/execbase.i'
	INCLUDE 'exec/ables.i'
	INCLUDE 'exec/errors.i'
	INCLUDE 'devices/audio.i'

          INCLUDE   'gloequs.i'
          INCLUDE   'featequ.i'
          INCLUDE   'pcequs.i'
          INCLUDE   'f0equs.i'
	  INCLUDE   'narrator.i'
	  INCLUDE   'private.i'

	  EXTERN_SYS AllocMem

          XREF      FEAT
          XDEF      F0INIT

_AbsExecBase	EQU	4



F0INIT	  MOVE.L    _AbsExecBase,A6		;Setup Exec base address

*	  ;-------- Allocate memory for the F0 arrays.  If no memory
	  ;	    available, return ND_NoMem error to main.

	  MOVEQ	    #0,D0			;Initialize count of syllables
	  CLR.W     F0TOTSYL(A5)		;   in D0 and globals area
	  MOVE.L    STRESS(A5),A0		;Get pointer to stress array

F0CountSyls:
	  MOVE.B    (A0)+,D1			;Get STRESS element
	  BPL.S	    F0CountSyls			;Nothing interesting
	  ADDQ.W    #1,D0			;Increment number of syllables
	  CMP.B	    #$FF,D1			;End of input?
	  BNE.S	    F0CountSyls			;No, keep looking
	  ADDQ.W    #1,D0			;Don't count the $FF as a syl,
						;  but add 2 bytes so F0HLS
						;  doesn't need to check for
						;  beginning of array while
						;  playing with syl to the left
						;  of the current syllable.
	  LSL.L	    #3,D0			;There are eight F0 arrays
	  MOVE.L    D0,D7			;Save (AllocMem destroys D0)
	  MOVE.L    #MEMF_PUBLIC+MEMF_CLEAR,D1	;Alloc flags
	  CALLSYS   AllocMem			;Get the memory
	  TST.L	    D0				;Got it?
	  BNE.S	    F0AllocOK			;  Yes
	  MOVEQ	    #ND_NoMem,D3		;  No, mark as error and return
  	  RTS


*	  ;-------- Got our memory, now we must initialize the various F0
	  ;	    array pointers.  The arrays that begin with "F0" are
	  ;	    "clause local" (ie they track the syllables on a clause
	  ; 	    by clause basis.  The other F0 arrays pointers store 
	  ;	    the absolute beginning of the arrays.  Each F0 array
	  ;	    starts 2 bytes in from the beginning.  This eliminates
	  ;	    the need to boundary check array indices in F0HLS and LLS.
F0AllocOK:
	  MOVE.W    D7,F0TOTSYL(A5)			;Length of F0 arrays

	  MOVE.L    PHON(A5),F0PHON(A5)			;Copy PHON pointer
	  MOVE.L    STRESS(A5),F0STRESS(A5)		;     STRESS
	  MOVE.L    DUR(A5),F0DUR(A5)			;     DUR

	  LSR.L	    #3,D7				;Length of each F0 array
	  ADDQ.L    #2,D0
	  MOVE.L    D0,F0START(A5)
	  MOVE.L    D0,START(A5)
	  ADD.L	    D7,D0
	  MOVE.L    D0,F0PEAK(A5)
	  MOVE.L    D0,PEAK(A5)
	  ADD.L	    D7,D0
	  MOVE.L    D0,F0END(A5)
	  MOVE.L    D0,FEND(A5)
	  ADD.L	    D7,D0
	  MOVE.L    D0,F0RISE(A5)
	  MOVE.L    D0,RISE(A5)
	  ADD.L	    D7,D0
	  MOVE.L    D0,F0FALL(A5)
	  MOVE.L    D0,FALL(A5)
	  ADD.L	    D7,D0
          MOVE.L    D0,F0CRBRK(A5)
          MOVE.L    D0,CRBRK(A5)
	  ADD.L	    D7,D0
          MOVE.L    D0,F0TUNELV(A5)
          MOVE.L    D0,TUNELV(A5)
	  ADD.L	    D7,D0
	  MOVE.L    D0,F0ACCENT(A5)
	  MOVE.L    D0,ACCENT(A5)



*	  ;-------- Ignore 1st two blanks and initial QX.

          ADDQ.L    #3,F0PHON(A5)				;*** Note 1
          ADDQ.L    #3,F0STRESS(A5)
          ADDQ.L    #3,F0DUR(A5)			



*	  ;-------- Clear syllable variables.

          CLR.W     F0NUMCLS(A5)        ;CLAUSE NUMBER
          CLR.W     F0NUMSYL(A5)        ;NUMBER OF SYLLABLES IN THIS CLAUSE
          CLR.W     F0NUMAS(A5)         ;NUMBER OF ACCENTED SYLLABLES
          CLR.W     F01STAS(A5)         ;POSITION OF 1ST ACCENTED SYLLABLE
          CLR.W     F0LASTAS(A5)        ;POSITION OF LAST ACCENTED SYLLABLE
          CLR.W     F0NUMPHS(A5)        ;PHRASE NUMBER

	  MOVEQ	    #0,D3		;Good return
          RTS

          END 
