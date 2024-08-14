
	TTL	'NARRATOR.ASM'


*************************************************************************	
*                                                                    	*
*   Copyright 1990, 1991 Joseph Katz/Mark Barton.  All rights reserved.	*
*   No part of this program may be reproduced, transmitted, or stored   *
*   in any language or computer system, in any form, whatsoever,	*
*   without the prior written permission of the authors.   		*
*   									*
*   Modification History                                               	*
*   									*
*	3/4/91	JK --	Added new subroutine FormSentence to break the	*
*			input into sentences.  Less pause before	*
*			speaking and smaller buffers need be allocated.	*
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

	XREF	PARSE,STRMARK,PHONOL,SYLLABLE,PROSOD,CONVERT
	XREF	F0INIT,F0HLS,F0LLS,COMPCOEF,FEAT,FormSentence
 	XREF	SYNTH,TermIO,AllocMultChan,FreeMultChan
	XREF	_Phonet,F0INTERP,F0ENTHUS,NARDBG_DebugMode,NARDBG_FreeCoef

	XDEF	Narrator

NUMCENTPHONS	EQU	11			;Size of allowed cent phons list


Narrator	JSR	  NARDBG_DebugMode		;Run in DEBUG mode?

	  	MOVEM.L   A1-A6/D2-D7,-(SP)		;Save registers
	  	MOVE.L    A1,A2            		;Copy IORequest ptr
	  	MOVE.L    ND_GLOBALS(A3),A5		;Get ptr to globals area
*	  	MOVE.L    ND_AUDIOLIB(A3),AudLib(A5) 	;Save ptr to Audio lib
	  	MOVE.L	  A2,USERIORB(A5)		;Save ptr to user's IORB

	 	MOVE.L    _AbsExecBase,A6		;SysLib pointer


*	  	;------ Allocate the audio channels and build the DMA IORBs

		JSR	  AllocMultChan
	  	MOVE.W	  D0,D3				;Check return code
	  	BNE	  MAINRTN			;Branch if error
		

*	  	;------ Copy mouth flag to globals

	  	MOVE.B	  NDI_MOUTHS(A2),Mouth(A5)


*	  	;------ Validate speaking rate against bounds

		MOVE.W    NDI_RATE(A2),D1		;Get rate from IORB
          	CMP.W     #MINRATE,D1
       	  	BGE.S     PAR1
PAR2	  	MOVEQ	  #ND_RateErr,D3
	  	BRA       MAINRTN                
PAR1      	CMP.W     #MAXRATE,D1
          	BGT.S     PAR2
          	MOVE.W    D1,RATE(A5)


*		;------ Validate pitch against bounds 

		MOVE.W	  NDI_PITCH(A2),D1		;Get pitch from IORB
	  	CMP.W     #MINPITCH,D1               
          	BGE.S     PAR3
PAR4		MOVEQ     #ND_PitchErr,D3
		BRA	  MAINRTN 
PAR3          	CMP.W     #MAXPITCH,D1
          	BGT.S     PAR4
		LSR.W	  #1,D1				;Convert to 2Hz steps
          	MOVE.W    D1,FREQ(A5)


*		;------ Validate F0 mode (Natural, Robotic, or Manual)

		MOVE.W	  NDI_MODE(A2),D1		;Get F0 mode from IORB
		CMP.W     #NATURALF0,D1
          	BEQ.S     PAR5
          	CMP.W     #ROBOTICF0,D1
          	BEQ.S     PAR5
		CMP.W	  #MANUALF0,D1
		BEQ.S	  PAR5
                MOVEQ	  #ND_ModeErr,D3
		BRA       MAINRTN
PAR5       	MOVE.W    D1,F0MODE(A5)


*		;------ Validate sex

		MOVE.W	  NDI_SEX(A2),D1
 		CMP.W     #MALE,D1
          	BEQ.S     PAR6
		CMP.W     #FEMALE,D1
		BEQ.S     PAR6
                MOVEQ	  #ND_SexErr,D3
		BRA	  MAINRTN
PAR6		MOVE.W    D1,SEX(A5)


*		;------ Validate sampling frequency

		MOVE.W	  NDI_SAMPFREQ(A2),D1
		CMP.W	  #MINFREQ,D1
		BGE.S     PAR7
PAR8		MOVEQ	  #ND_FreqErr,D3
		BRA       MAINRTN
PAR7		CMP.W     #MAXFREQ,D1
		BGT.S     PAR8
		MOVE.W	  D1,SampFreq(A5)


*		;------ Validate volume

		MOVE.W    NDI_VOLUME(A2),D1
		CMP.W     #MINVOL,D1
                BGE.S     PAR9
PAR10		MOVEQ     #ND_VolErr,D3
		BRA       MAINRTN
PAR9		CMP.W     #MAXVOL,D1
		BGT.S	  PAR10
		MOVE.B    D1,Volume(A5)


*		;------ Validate degree of centralization.

		BTST	  #NDB_NEWIORB,NDI_FLAGS(A2)	;New IORB?
		BEQ.S	  NarrCont			;No, skip central checks
		MOVEQ	  #0,D1
		MOVE.B    NDI_CENTRALIZE(A2),D1
		CMP.W     #MINCENT,D1
                BGE.S     ValCentPct
InvalCentPct	MOVEQ     #ND_DCentErr,D3
		BRA       MAINRTN
ValCentPct	CMP.W     #MAXCENT,D1
		BGT.S	  InvalCentPct
		MOVE.L    D1,CentralPct(A5)
		BEQ.S	  NarrCont


*		;------	Validate centralization phoneme (if any)

		TST.L	  NDI_CENTPHON(A2)		;Central phon ptr NULL?
		BEQ.S	  InvalCentPhon			;  Yes, return an error
		MOVE.L	  NDI_CENTPHON(A2),A0		;Central phon string ptr
		MOVE.W	  (A0),D0			;Get central phon
		LEA	  AllowedCentPhons,A0
		MOVEQ	  #NUMCENTPHONS-1,D1
Cent1		CMP.W	  (A0)+,D0			;Comp with allowed phons
		ADDQ.L	  #2,A0				;Bump past phon code
		DBEQ	  D1,Cent1			;Loop
		TST.W	  D1				;If D1 neg, not an 
		BPL.S	  Cent2				;  allowed cent phon
InvalCentPhon	MOVEQ	  #ND_CentPhonErr,D3
		BRA	  MAINRTN
Cent2		MOVEQ	  #0,D0
		MOVE.W	  -2(A0),D0
		MOVE.L	  D0,CentralPC(A5)		;Store p.c. in globals



NarrCont  CLR.L	    PARLENTH(A5)		;Initialize length to 0
	  MOVE.L    IO_DATA(A2),PARSTART(A5) 	;Initialize starting position
	  MOVE.L    IO_LENGTH(A2),LFT2PARS(A5)	;Initialize amount left to parse


*		;------- Main synthesizer loop

MAINLOOP  JSR	    FormSentence		;Find sentence and alloc bfrs
	  BEQ	    MAINRTN			;D3=0, nothing more to say
	  BMI	    MAINRTN			;Error (D3 has code)
	  JSR       PARSE
          BEQ       MAINRTN             	;Nothing more to say
          BMI       MAINRTN             	;PARSE error
          JSR       STRMARK
          JSR       PHONOL
          JSR       SYLLABLE
          JSR       F0INIT
	  BMI	    MAINRTN			;No memory for F0 arrays
F0LOOP    JSR       F0HLS
          BEQ.S     F0ADJ               	;Sentence completed
          JSR       F0LLS
          BRA.S     F0LOOP
F0ADJ     JSR	    F0ENTHUS			;Enthusiasm adjust
	  JSR       PROSOD
          JSR       COMPCOEF
	  BMI	    ReturnCOEF			;COMPCOEF cannot get memory
	  MOVE.L    CentralPct(A5),-(SP)	;Degree of centralization
	  MOVE.L    CentralPC(A5),-(SP)		;Centralize phoneme code
	  MOVE.L    USERIORB(A5),-(SP)		;Address of user IORB
	  MOVE.L    MOTHPTR(A5),-(SP)		;Address of mouth array
	  MOVE.L    CRBRK(A5),-(SP)		;F0 CRBRK array
	  MOVE.L    FEND(A5),-(SP)		;F0 END array
	  MOVE.L    PEAK(A5),-(SP)		;F0 PEAK array
	  MOVE.L    START(A5),-(SP)		;F0 START array
	  MOVE.L    COEFPTR(A5),-(SP)		;Coef buffer
	  MOVE.L    DUR(A5),-(SP)		;Duration
	  MOVE.L    STRESS(A5),-(SP)		;Stress
	  MOVE.L    PHON(A5),-(SP)		;Phon  
	  JSR	    _Phonet
	  LEA	    48(SP),SP			;Fix stack
	  JSR	    F0INTERP			;F0 interpolation
	  JSR	    CONVERT			;Convert step sizes to indeces


*	;------	Set the task priority high
	MOVE.L	_AbsExecBase,A6
	SUB.L	A1,A1
	CALLSYS	FindTask
	MOVE.L	D0,A1
	MOVEQ	#100,D0				;Old style IORB uses pri=100
	MOVE.L	USERIORB(A5),A0			;Get user IORB
	BTST	#NDB_NEWIORB,NDI_FLAGS(A0)	;New style IORB?
	BEQ.S	SetPri				;No, use default priority of 100
	MOVE.B	NDI_PRIORITY(A0),D0		;Get speaking priority from IORB
SetPri	CALLSYS	SetTaskPri


*	;------	Call the synthesizer
        JSR       SYNTH


*	;------	Set the task priority low
	MOVE.L	_AbsExecBase,A6
	SUB.L	A1,A1
	CALLSYS	FindTask
	MOVE.L	D0,A1
	MOVEQ	#0,D0
	CALLSYS	SetTaskPri


*	;------ Release coef bfr if KEEPCOEF bit not set in flags field and
	;       if we were actually able to allocate it.

ReturnCOEF:
	JSR	  NARDBG_FreeCoef		;Free up the coef mem


*	;------	Release the Mouth shape buffer.

Check_Mouth:
	  TST.B	    Mouth(A5)
	  BEQ.S	    MAINRTN
	  MOVE.L    MOTHPTR(A5),A1
	  MOVE.L    MOTHSIZE(A5),D0
	  BEQ.S	    MAINRTN		;Nothing to return
	  CALLSYS   FreeMem


MAINRTN	  MOVE.L   _AbsExecBase,A6	;Restore SysBase
	  MOVE.L   PSDLEN(A5),D0	;Return PHON, STRESS, and DUR memory
	  BEQ.S	   MAINRTN1		;Nothing to return
	  MOVE.L   PHON(A5),A1		;Address of memory to return
	  CALLSYS  FreeMem		;Back it goes
	  MOVE.L   #0,PSDLEN(A5)	;Reset length of alloc'd P,S,D arrays

MAINRTN1  MOVEQ	   #0,D0
	  MOVE.W   F0TOTSYL(A5),D0	;Return F0 arrays
	  BEQ.S	   MAINRTN2		;Nothing to return
	  MOVE.L   START(A5),A1		;Address of memory to return
	  SUBQ.L   #2,A1		;F0 buffers actually start 2 to the left
	  CALLSYS  FreeMem		;Back it goes
	  MOVE.W   #0,F0TOTSYL(A5)	;Reset length of alloc'd F0 arrays

MAINRTN2  TST.W     D3			;Check return code
          BNE.S     MAINRTN3		;If error code
	  TST.L	    LFT2PARS(A5)	;Anything left to say?
	  BNE	    MAINLOOP		;Yep, keep going


MAINRTN3  MOVE.W    D3,D0			;Transfer return code to D0
	  MOVEM.L   (SP)+,A1-A6/D2-D7		;Restore important registers

	  JSR	    FreeMultChan		;Preserves all registers

	  CLR.L	    ND_USERIORB(A3)		;Clear the current user IORB

	  MOVE.L    IO_LENGTH(A1),IO_ACTUAL(A1)	;Assume everything worked ok
	  MOVE.B    D0,IO_ERROR(A1)		;Store return code in IORB
	  BEQ.S	    MainDone			;Everything ok, return


*	  ;-------- An error occured during processing.  If phoneme code
	  ;	    error, set IO_ACTUAL to -ive position of error in the
	  ;	    ASCII string (should just be position of error, but this
	  ;	    is a holdover from previous days).  If any other error,
	  ;	    zero IO_ACTUAL.

	  MOVE.L    #0,IO_ACTUAL(A1)		;Clear IO_ACTUAL
	  CMP.B	    #ND_PhonErr,D0		;Phoneme code error?
	  BNE.S     MainDone			;   No, just return
	  MOVE.L    ND_GLOBALS(A3),A0		;   Yes, Get ptr to globals area
	  MOVE.W    PHONLEN(A0),D0		;   	 Get position of error
	  EXT.L	    D0				;        Extend to longword
	  MOVE.L    D0,IO_ACTUAL(A1)		;        Put in user's IORB

MainDone  BSR	  TermIO			;Return msg to user
	  RTS

AllowedCentPhons:
	DC.B	"IY",0,9
	DC.B	"IH",0,11
	DC.B	"EH",0,13
	DC.B	"AE",0,15
	DC.B	"AA",0,17
	DC.B	"AH",0,18
	DC.B	"AO",0,19
	DC.B	"OW",0,39
	DC.B	"UH",0,21
	DC.B	"ER",0,25
	DC.B	"UW",0,41

	DC.B      'COPYRIGHT 1990, 1991  JOSEPH KATZ / MARK BARTON'
	CNOP 0,2

	END
