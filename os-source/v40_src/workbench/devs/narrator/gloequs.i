
	TTL	'GLOEQUS.I'


*************************************************************************	
*                                                                    	*
*   Copyright 1990, Joseph Katz/Mark Barton.  All rights reserved.	*
*   No part of this program may be reproduced, transmitted, or stored   *
*   in any language or computer system, in any form, whatsoever,	*
*   without the prior written permission of the authors.   		*
*                                                                    	*
*************************************************************************


     
         NOLIST


*		;------ CPU Type

MAC      	EQU     0
AMIGA    	EQU	1


*		;------	"Parms" structure equates

OF_DIGRAPHS	EQU	 0
OF_FEATURES	EQU	 2
OF_F1		EQU	 6
OF_F2		EQU	 7
OF_F3		EQU	 8
OF_A1		EQU	 9
OF_A2		EQU	10
OF_A3		EQU	11
OF_MINDUR	EQU	12
OF_INHDUR	EQU	13
OF_AV		EQU	14
OF_FRICID	EQU	15
OF_HEIGHT	EQU	16
OF_WIDTH	EQU	17
PARMSIZE	EQU	18



*		;------ Buffer size equates

MAXPHONL	EQU     512             ;MAX NUMBER OF PHONEMES
F0MAXLEN 	EQU	128             ;MAX NUM OF SYLS (MUST BE MULT OF 4)



*		;------ Globals area equates

    STRUCTURE	GLOBALS,0
	ULONG	F1REC				;Formants
	ULONG	F2REC
	ULONG	F3REC
	UWORD	SYNC				;Word/syl sync has occured flag
	UWORD	PITCH    	    		;Samples/glot pulse
	UWORD	FRICLEN   	     		;Fricative length counter
	UWORD	FRIDEX   	    		;Frics index
	UWORD	FRINC    			;Reversable fric increment
	UWORD	SAMPCNT  			;Sample count
	UWORD	BUFFLAG  			;Use buffer 1 or 2 indicator
	UWORD	UPDATE   			;Frame update is needed flag
	UWORD	RECLEN				;Counts down 128 audio samples
	UWORD	NOISEREGISTER			;3 Hz LPF noise register
	UWORD	FILTER1DELAY			;Noise delay element 1
	UWORD	FILTER2DELAY			;Noise delay element 2
	UWORD	FREQ     	    		;Input parm for pitch in HZ
	UWORD	RATE     	 		;Input parm for rate in wpm
	ULONG	KHZ12    	 		;Sampling rate for TAG
	LABEL	SAMPERFRAME  			;Another name for `SPEED'
	UWORD	SPEED    			;Samples / frame
	UWORD	SEX      			;Sex of voice
	ULONG	COEFPTR  			;Coef buffer pointer
	ULONG	MOTHPTR  			;Mouth buffer pointer
	UWORD	F0MODE   			;F0 mode indicator
	UWORD	BANWID   			;Working bandwidth
	UWORD	BW       			;Bandwidth counter
	ULONG	COEFSIZE 			;Size of coef buffer
	ULONG	MOTHSIZE 			;Size of mouth buffer
	ULONG	READERADDR   			;Pointer to READER routine
	ULONG	PARVERYB     			;Very beginning of ASCII input
	ULONG	CURMOUTH     			;Current mouth buffer position


*         	;------	Audio DMA control block

	LABEL	AudArgs	   			;AudArgs == AudChan
	UBYTE	AudChan
	UBYTE	AudVol
	ULONG	AudBuff
	UWORD	AudLen
	UWORD	AudPer
	UWORD	AudCyc
	ULONG	AudTask
	UBYTE	AudBit
	UBYTE	AudSync
	UWORD	AudPad



*       	;------	F0 area

	ULONG	F0START   			;Current pos in F0 arrays
	ULONG	F0PEAK
	ULONG	F0END
	ULONG	F0CRBRK
	ULONG	F0ACCENT
	ULONG	F0TUNELV
	ULONG	F0RISE
	ULONG	F0FALL
	ULONG	F0PHON				;Current pos in P,S,D arrays
	ULONG	F0DUR
	ULONG	F0STRESS
	UWORD	F0NUMCLS			;Number of clauses
	UWORD	F0NUMAS				;Number of accented syllables
	UWORD	F0NUMSYL			;Number of syllables in clause
	UWORD	F0TOTSYL			;Number of syls in utterance
	UWORD	F01STAS				;Position of 1st accented syl
	UWORD	F0LASTAS			;Position of last accented syl
	UWORD	F0NUMPHS			;Number of phrases


*		;------ Variables used by PARSE

	ULONG	PARSTART			;Start of ASCII input
	ULONG	PARLENTH			;Length of ASCII input
	UWORD	PHONLEN				;Length of PHON input ????
	UWORD	MAX2PARS			;Max allowed to parse
	ULONG	LFT2PARS			;Amount left over to parse

	STRUCT	FRADDR,32			;Fric addr table (32 bytes)
	UWORD	MALEPITCH 			;Current male pitch ????
	UWORD	MALERATE  			;Current male rate  ????
	UWORD	MALEF0MODE 			;Current male F0 mode ????
	UWORD	FEMPITCH  			;Current female pitch ???
	UWORD	FEMRATE   			;Current female rate ????
	UWORD	FEMF0MODE 			;Current female F0 mode ????
	ULONG	CentralPct			;Central percentage
	ULONG	CentralPC			;Central phoneme code


*		;-------- Random stuff

	UWORD	MySig     			;Number of signal bit
	ULONG	MySigMask 			;Long word with signal bit set
	UWORD	SampFreq  			;Output sampling freq, typ 22200
	UBYTE	Reson     			;Resonance parm, typ 6
	UBYTE	Mouth     			;Non-zero ==> generate mouths
	UWORD	Volume				;Volume
	UWORD	Channel				;Channel info
	ULONG	AudLib 				;Pointer to audio library
	ULONG	USERIORB  			;Pointer to user's IORB
	ULONG	SYNTHBFR 			;SYNTH output buffer

	ULONG	PHON	  			;Pointer to PHON array
	ULONG	STRESS				;Pointer to STRESS array
	ULONG	DUR				;Pointer to DUR array
	ULONG	PSDLEN				;Length of PHON,STRESS,DUR 

	ULONG	START				;Abs beginning of F0 arrays
	ULONG	PEAK
	ULONG	FEND
	ULONG	CRBRK
	ULONG	ACCENT
	ULONG	TUNELV
	ULONG	RISE
	ULONG	FALL

	STRUCT	OUTBUFF1,512			;Audio output bfr 1 (512 bytes)
	STRUCT	OUTBUFF2,512			;Audio output bfr 2 (512 bytes)

	LABEL	GLOSIZE				;Size of globals area

FIXBFRSZ  	EQU     GLOSIZE
VARLEN   	EQU     (NOISEREGISTER+2)/2    ;NUMBER OF VARS TO CLEAR IN SYNTH


          LIST

