
*P-UNITI can also be incorretly set if input is in error eg AA(9\0 or AA (-\0

	TTL	'PARSE.ASM'


*************************************************************************	
*                                                                    	*
*   Copyright 1990, Joseph Katz/Mark Barton.  All rights reserved.	*
*   No part of this program may be reproduced, transmitted, or stored   *
*   in any language or computer system, in any form, whatsoever,	*
*   without the prior written permission of the authors.   		*
*                                                                    	*
*************************************************************************


          SECTION      speech

*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
*                                                                     	;
*                        PHONEME PARSER                               	;
*                                                                     	;
*    INPUT IS ASCII PHONEME STRING.  OUTPUT IS PHON, STRESS, DUR.     	;
*    ERRORS IN INPUT ARE NOTED BY RETURNING THE -IVE POSITION IN      	;
*    D3.                                                              	;
*                                                                     	;
*                                                                     	;
*    A0 - Address of ASCII string    A1 - PHON array			;
*    A2 - STRESS array		     A3 - DUR array			;
*    A4 - Features matrix            A5 - Globals area			;
*    A6 - Graphemes table            A7 - Stack				;
*                                                                     	;
*    D0 - ASCII char		     D1 - Scratch			;
*    D2 - Scratch		     D3 - Output count			;
*    D4 - Phoneme features           D5 - Last phon features		;
*    D6 - Internal phoneme code	     D7 - Previous phon			;
*                                                                     	;
*    Update history
*
*	8/27/90  -  Narrator had a problem dealing with input not 
*		    terminated by a '\0'.  Added code at ParChkLen
*		    and modified branches to ParMain (they now branch
*		    to ParChkLen) in section ParTestWordBreak to check
*		    for input length.
*    
*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


*	  ;--------- A few handy equates

PARSTEND  EQU       '#'       			;Alternate end-of-input marker
						;        (Macintosh relic)


*	  ;-------- Syllable bits contained in the STRESS array

SYLSTART  EQU       $80				;Start of syllable
SYLPOLY   EQU       $40       			;Poly-syllablic
SYLSTRD   EQU       $20       			;Stressed phoneme
SYLSGMT   EQU       $10       			;Stressed segment


*	  ;-------- Syllable bits contained in the DUR array

SYLWORDF  EQU       $80       			;Word final
SYLPHRSF  EQU       $40       			;Phrase final
SYLPUNTI  EQU	    $20	      			;P-UNIT initiator
SYLPUNTT  EQU	    $10	      			;P-UNIT terminator


	  INCLUDE   'exec/types.i'
          INCLUDE   'featequ.i'
          INCLUDE   'pcequs.i'
          INCLUDE   'gloequs.i'
	  INCLUDE   'narrator.i'
	  INCLUDE   'parse.i'


          XREF      FEAT
          XDEF      PARSE


*	  ;-------- Initialize.

PARSE	  MOVE.L    PARSTART(A5),A0		;Get beginning of ASCII string
	  MOVE.L    PHON(A5),A1
          MOVE.L    STRESS(A5),A2
          MOVE.L    DUR(A5),A3
          LEA       FEAT,A4


          CLR.W     (A1)+      			;Insert two blanks at beginning
          CLR.W     (A2)+			;   of output arrays
          CLR.W     (A3)+

          MOVE.B    #PHCQX,(A1)+		;Move in silent phon
	  CLR.B	    (A2)+			;Clear STRESS
	  CLR.B	    (A3)+			;   and DUR

          CLR.B     (A1)+        		;Move in a blank
	  CLR.B	    (A2)+			;Clear STRESS
	  CLR.B	    (A3)+			;   and DUR

          MOVEQ     #4,D3        		;Start output count at 4

          MOVEQ     #0,D0			;Input char
	  MOVEQ	    #0,D1
          MOVEQ     #0,D2
          MOVEQ     #0,D4
	  MOVEQ	    #0,D5
	  MOVEQ	    #0,D6
          MOVEQ     #0,D7			;Phoneme code



*	  ;------  PARSE main loop.  Get a character from the input array.
	  ;	   If it is a number (range 0-9 allowed), check that it
	  ;	   follows a vowel, and if so, store in stress array.
	  ;  	   If it is in the range A-Z or a special character, form
	  ;	   an index into the Grapheme pointer table, and use this
	  ;	   index to get the offset to the allowable graphemes that
	  ;	   begin with this letter.

ParMain   MOVEQ	    #0,D0
	  MOVE.B    (A0)+,D0       		;Get a char from input
	  BEQ	    ParDone	  		;Null terminates input string
          CMP.B     #PARSTEND,D0  		;'#' also terminates input
          BEQ       ParDone			;   (relic of Macintosh days)


	  LEA	    GraphemePointers,A6		;Grapheme table pointers
	  MOVE.W    D0,D1			;Input char
	  SUB.W	    #' ',D1			;Form table index
	  BLT	    ParError			;Char less than SPACE, error
	  CMP.W	    #MAXGRAPHEME,D1		;Check against max table entry
	  BGT	    ParError			;Char not in table, error
	  LSL.W	    #2,D1			;Table entries are longwords
	  MOVE.L    0(A6,D1),A6			;Pointer to allowable graphemes

ParSearch MOVE.B    (A6)+,D2			;Second char of digraph
	  BEQ.S	    ParEOList			;End of digraph list
	  MOVE.B    (A6)+,D6			;Internal phoneme code
	  CMP.B	    (A0),D2			;Second char matches?
	  BNE.S	    ParSearch			;   No, continue searching
	  ADDQ.L    #1,A0			;   Yes, bump input string ptr
	  BRA.S	    ParProcess			;      and process phoneme

ParEOList MOVE.B    (A6),D6			;Get internal phoneme code
	  CMP.B	    #PHCNULL,D6			;If PHCNULL,
	  BEQ.S	    ParError			;   Digraph error
						;   Else process phoneme


*	  ;------   Check for a stress number in the range 0-9 and, if so,
	  ;	    check previous phon for vowel feature.  If number and
	  ;	    previous phon is a vowel, store number in stress array.
	  ;	    If previous phon is not a vowel, mark as error.

ParProcess:
	  MOVEQ	    #0,D4			;Clear for use as FM index
	  MOVE.B    D6,D4			;Copy phoneme code
	  LSL.W     #2,D4			;Create FM index
          MOVE.L    0(A4,D4),D4         	;Get phoneme features

          MOVE.B    -1(A1),D7 		       	;Get previous phoneme code
	  MOVEQ	    #0,D5			;Clear for use as FM index
	  MOVE.B    D7,D5			;Copy phon
          LSL.W     #2,D5               	;Convert it to FM index
          MOVE.L    0(A4,D5),D5         	;Get phoneme features

	  BTST	    #NUMBERBIT,D4		;A number?
	  BEQ.S	    ParTestWordBreak		;No, skip
          BTST      #VOWELBIT,D5        	;Is previous phon a vowel?
          BEQ       ParError            	;No, mark as error and return

          AND.B     #$0F,D0             	;Convert ASCII number to binary
          OR.B      #SYLSTRD|SYLSGMT,D0        	;Set stressed seg and syl flags
          MOVE.B    D0,-1(A2)        		;Store in stress array
          BRA  	    ParMain			;Continue parsing



*	  ;------  Process word breaks.  If adjacent word breaks, overwrite the 
	  ; 	   old phoneme code with the new one.  If D3 <= 4, then there 
	  ;        is no real text preceeding the wordbreak.  In this case, 
	  ; 	   just ignore word break.  Also, don't store a blank over any
	  ;   	   previous punctuation.

ParTestWordBreak:
	  BTST      #WORDBRKBIT,D4      	;Word break?
          BEQ.S     ParTestPUnit		;  No, test for P-UNIT 
          BTST      #WORDBRKBIT,D5      	;  Yes, is prev phon WORDBREAK?
          BEQ.S     ParStore            	;    No, store phoneme
          CMP.W     #4,D3               	;Any previous text?
          BLE       ParChkLen             	;  No, dont store anything
          CMP.B	    #PHCBL,D6			;Is this phoneme a blank?
          BEQ       ParChkLen             	;  Yes, don't overwrite
	  MOVE.B    D6,-1(A1)			;  No, overwrite old phon
          BRA       ParChkLen	            	;Store new WORDBRK phon


*	  ;-------- Check for P-Unit initiators or terminators

ParTestPUnit:
	  CMP.B     #PHCLP,D6           	;P-UNIT initiator?
          BNE.S     ParPUTerm             	;  No, try P-UNIT terminator
          OR.B      #SYLPUNTI,(A3)    		;  Yes, set P-UNIT initiator bit
          BRA       ParMain             	;     and continue parsing

ParPUTerm:
	  CMP.B     #PHCRP,D6           	;P-UNIT terminator?
          BNE.S     ParPhonemeCheck		;  No, process phoneme codes
          OR.B      #SYLPUNTT,-1(A3)   		;  Yes, set P-UNIT term bit
          BRA       ParMain			;     and continue parsing



*	  ;------   Do some special phoneme parsing.  Replace certain 
	  ;	    VOWEL+R forms with new MITALK type diphthongs, and
	  ;	    replace OH with OW.

ParPhonemeCheck:
	   CMP.B     #PHCRR,D6			;Is this phon an R?
	   BNE.S     ParOHCheck			;  No, skip 

	   CMP.B     #PHCIY,D7			;Last phon IY?
	   BNE.S     ParEHCheck			;  No, try EH
	   MOVE.B    #PHCIXR,-1(A1)		;  Yes, replace IY with IXR
	   BRA       ParMain			;Continue parsing

ParEHCheck:
	  CMP.B      #PHCEH,D7			;Last phon EH?
	  BNE.S      ParAACheck			;  No, try AA
	  MOVE.B     #PHCEXR,-1(A1)		;  Yes, replace EH with EXR
	  BRA	     ParMain			;Continue parsing

ParAACheck:
	  CMP.B      #PHCAA,D7			;Last phon AA?
	  BNE.S      ParOWCheck			;  No, try OW
	  MOVE.B     #PHCAXR,-1(A1)		;  Yes, replace AA with AXR
	  BRA	     ParMain			;Continue parsing

ParOWCheck:
	  CMP.B      #PHCOW,D7			;Last phon OW?
	  BNE.S      ParUWCheck			;  No, try UW
	  MOVE.B     #PHCOXR,-1(A1)		;  Yes, replace OW with OXR
	  BRA	     ParMain			;Continue parsing

ParUWCheck:
	  CMP.B      #PHCUW,D7			;Last phon UW?
	  BNE.S      ParStore			;  No, store R phon
	  MOVE.B     #PHCUXR,-1(A1)		;  Yes, replace UW with UXR
	  BRA	     ParMain			;Continue parsing

ParOHCheck:
	  CMP.B      #PHCOH,D6			;Is this phon OH?
	  BNE.S      ParStore			;  No, branch
	  MOVE.B     #PHCOW,D6			;  Yes, replace with OW

ParStore  MOVE.B    D6,(A1)+			;Store phoneme code
	  ADDQ.L    #1,A2			;Bump STRESS pointer
	  ADDQ.L    #1,A3			;Bump DUR pointer
          ADDQ.W    #1,D3               	;Increment output count

*          CMP.B     #PHCPE,D6           	;This phon a period?
*          BEQ.S     ParDone             	;  Yes, speak to me
*          CMP.B     #PHCQM,D6           	;This phon a question mark?
*          BEQ.S     ParDone             	;  Yes, speak to me


*	;------	Check how far we've parsed.  Stop if the amount parsed
*		is equal to Io_Length in IORB.  This code added 8/27/90.
*		(except for the "BRA   ParMain" which was always there).

ParChkLen:
	  MOVE.L   A0,D0			;Get ptr to next phoneme
	  SUB.L	   PARSTART(A5),D0		;Compute length we've parsed
	  CMP.L	   PARLENTH(A5),D0		;Compare it to max length
	  BEQ.S	   ParDone			;Same, reached end of input
	  BRA	   ParMain			;Continue parsing.



*	  ;-------- Found an error in input string.  Since the synthesizer may
	  ;	    have already completed previous sentences, we must compute
	  ;	    the position of the error with reference to the beginning
	  ;	    of the original input string.

ParError  MOVE.L    A0,D3               	;Current pos in string
	  MOVE.L    USERIORB(A5),A1		;Get user IORB
          SUB.L     IO_DATA(A1),D3     		;Cur pos - very beginning of string
	  SUBQ.L    #1,D3			;        - 1
	  NEG.W	    D3				;Negative length ==> input error
	  MOVE.W    D3,PHONLEN(A5)		;Store in globals area
	  MOVEQ	    #ND_PhonErr,D3		;Set return code
          BRA	    ParRTS			;And go away



*	  ;-------- Done processing input.  Insure that there is some form
	  ;	    of punctuation at end of utterance.  Also if D3 <= 4,
	  ;	    then there is nothing to say.

ParDone   CMP.W     #4,D3               ;Do we have something real to say?
          BGT.S     ParCheckPunc        ;   Yes, check terminated by punctuation
	  MOVEQ	    #0,D3		;   No, set output length to zero
	  BRA.S	    ParReturn		;      and return

ParCheckPunc:
	  MOVEQ     #0,D0		;Clear for FM index
          MOVE.B    -1(A1),D0        	;Get last code in PHON array
	  BNE.S	    ParNotASpace	;Not a space, branch
	  MOVE.B    #PHCDA,-1(A1)	;Its a space, overwrite with a dash
	  CLR.B	    -1(A2)		;Clear STRESS
	  CLR.B	    -1(A3)		;Clear DUR
	  BRA.S	    ParCleanup		;Fixup end of arrays and go away

ParNotASpace:
	  LSL.W     #2,D0               ;Convert phon to FM index
          MOVE.L    0(A4,D0),D0         ;Get phoneme features
          BTST      #PAUSEBIT,D0        ;Is last phon a pause?
          BNE.S     ParCleanup          ;Yes, don't need to add anything
          MOVE.B    #PHCDA,(A1)+     	;No, insert dash at end of PHON array
          CLR.B     (A2)+            	;Clear STRESS
          CLR.B     (A3)+            	;Clear DUR
          ADDQ.W    #1,D3               ;Increase length of output arrays



*	  ;-------- Time to cleanup PHON, STRESS, and DUR arrays and return
	  ;	    to main routine.  We currently add two QX'x to PHON array,
	  ;	    clearing STRESS and DUR, and terminate all three arrays
	  ;	    with an X'FF'.

ParCleanup:
	  MOVE.B    #PHCQX,(A1)+	;Add 'QX' to end of PHON
	  CLR.B	    (A2)+		;Clear STRESS
	  CLR.B	    (A3)+		;Clear DUR
	  ADDQ.W    #1,D3		;Bump length of output arrays

	  MOVE.B    #PHCQX,(A1)+	;Add 'QX' to end of PHON
	  CLR.B	    (A2)+		;Clear STRESS
	  CLR.B	    (A3)+		;Clear DUR
	  ADDQ.W    #1,D3		;Bump length of output arrays

          MOVE.B    #$FF,(A1)       	;Insert X'FF' in PHON,
          MOVE.B    #$FF,(A2)       	;                STRESS, and 
          MOVE.B    #$FF,(A3)	        ;                DUR arrays
          ADDQ.W    #1,D3               ;Bump length of output arrays



*	  ;-------- Back to main we go.  D3 contains the output length of
	  ;	    the PHON, STRESS, and DUR arrays (or if there was an
	  ;	    input error, D3 contains ND_PhonErr and PHONLEN(A5) 
	  ;	    contains the -ive of the position in the ASCII string 
	  ;	    of the error).  Leave the "TST.W    D3" just above the
	  ;	    RTS to insure the CC's are set properly.

ParReturn MOVE.W    D3,PHONLEN(A5)	;Save length in globals

ParRTS	  TST.W	    D3			;Set CC's
	  RTS

          END 


