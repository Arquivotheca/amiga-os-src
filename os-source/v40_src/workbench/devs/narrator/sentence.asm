
	TTL	'SENTENCE.ASM'


*************************************************************************	
*                                                                    	*
*   Copyright 1990, 1991 Joseph Katz/Mark Barton.  All rights reserved.	*
*   No part of this program may be reproduced, transmitted, or stored   *
*   in any language or computer system, in any form, whatsoever,	*
*   without the prior written permission of the authors.   		*
*   									*
*   Modification History                                               	*
*									*
*************************************************************************


		SECTION speech
_AbsExecBase	EQU	4


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

io_Size	EQU	IO_SIZE
        INCLUDE	'featequ.i'
        INCLUDE	'gloequs.i'
	INCLUDE	'devices/audio.i'
	INCLUDE	'narrator.i'
	INCLUDE	'private.i'


******* Imported Functions *******************************************

	EXTERN_SYS AllocMem
	EXTERN_SYS FreeMem
	EXTERN_SYS PutMsg
	EXTERN_SYS ReplyMsg
	EXTERN_SYS Signal
	EXTERN_SYS Wait
	EXTERN_SYS DoIO
	EXTERN_SYS FindTask
	EXTERN_SYS SetTaskPri

	XDEF	FormSentence


FormSentence:
*		;------	Scan input string for PERIOD, QUESTION MARK, or
		;	end of input.

		MOVE.L	  PARLENTH(A5),D0	;Get previous length
		ADD.L	  D0,PARSTART(A5) 	;Update starting position
		MOVE.L	  PARSTART(A5),A0	;Get current pos of input str
		MOVE.L	  LFT2PARS(A5),D0	;Get amount left to parse
		MOVE.L	  D0,D3			;Copy to return code register
		BEQ.S	  FormExit		;Nothing left to say, just return
		SUBQ.L	  #1,D0			;Adjust for DBF
FormLoop	CMP.B	  #'.',(A0)		;Got a PERIOD?
		BEQ.S	  FormEOS		;  Yes, end of sentence
		CMP.B	  #'?',(A0)		;  No, got a QM?
		BEQ.S	  FormEOS		;    Yes, end of sentence
		CMP.B	  #'#',(A0)		;    No, end of input?
		BEQ.S	  FormEOS		;	Yes, end of sentence
		TST.B	  (A0)			;       Other end of input test
		BEQ.S	  FormEOS		;	
		ADDQ.L	  #1,A0			;Found nothing interesting, bump ptr
		DBF	  D0,FormLoop		;  and keep looking

*		;------	We've reached the end of the input string without 
		;	having found a . or ?.

		MOVE.L	  A0,D0			;Compute length of sentence
		SUB.L	  PARSTART(A5),D0
		MOVE.L	  D0,PARLENTH(A5)	;Save in globals area
		CLR.L	  LFT2PARS(A5)		;No more to say
		BRA.S	  FormAlloc		;Allocate PHON, STRESS, DUR buffers


		;-----	We've reached a real end of sentence.  If the char
		;	after the . or ? is a string terminator, then set
		;	LFT2PARS to zero.

FormEOS		ADDQ.L	  #1,A0			;Bump input pointer
		MOVE.L	  A0,D0			;Compute length of sentence
		SUB.L	  PARSTART(A5),D0
		MOVE.L	  D0,PARLENTH(A5)	;Save in globals area
		SUB.L	  D0,LFT2PARS(A5)	;Reduce amount remaining to parse

		TST.B	  (A0)			;Test for null term string
		BEQ.S	  FormEOS1		;End of input
		CMP.B	  #'#',(A0)		;Alternate end of input marker
		BNE.S	  FormAlloc		;No, time to alloc buffers
FormEOS1	CLR.L	  LFT2PARS(A5)		;No more to say.
		





*		;------ Allocate memory for PHON, STRESS, and DUR arrays and
		;	store pointers in globals area.  The length of the
		;	arrays is computed as twice the length of the input 
		;	string plus 10 (for the blanks and QX's inserted by
		;	PARSE).  This should insure that no overflow can 
		;	occur, either in PARSE or in PHONOL.  The memory is
		;	cleared during allocation to make life easier in PARSE.

FormAlloc	CLR.L	  PSDLEN(A5)		;Clear alloc length in globals
		CLR.L	  D0			;Initialize alloc length
		CLR.L	  D2			;Initialize offset
		MOVE.L	  PARLENTH(A5),D0	;Get length of sentence
		ADD.L	  D0,D0			;Each output array is 2*input +
		ADD.L	  #10,D0		; 		10 bytes long
		MOVE.W	  D0,D2			;Save for later
		ADD.L	  D0,D0
		ADD.L	  D2,D0			;D0 is now 6*input size+30 long
		MOVE.L	  D0,D3			;Copy alloc length
		MOVE.L	  #MEMF_PUBLIC+MEMF_CLEAR,D1
		CALLSYS	  AllocMem
		TST.L	  D0
		BNE.S	  FormGotMem
		MOVEQ	  #ND_NoMem,D3
		BRA.S	  FormExit
FormGotMem	MOVE.L	  D0,PHON(A5)		;Save start of PHON array
		ADD.L	  D2,D0			;Add in offset
		MOVE.L	  D0,STRESS(A5)		;Save start of STRESS array
		ADD.L	  D2,D0			;Add in offset
		MOVE.L	  D0,DUR(A5)		;Save start of DUR array
		MOVE.L	  D3,PSDLEN(A5)		;Save alloc length


*		;-----	Time to return to main loop.  D3 contains return code.
		; 	-ive ==> error
		;	   0 ==> nothing more to say
		;	+ive ==> something to say

FormExit	TST.W	  D3
		RTS

		END
