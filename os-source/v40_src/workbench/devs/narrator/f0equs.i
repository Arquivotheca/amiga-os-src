	TTL	'F0EQUS.I'


*************************************************************************	
*                                                                    	*
*   Copyright 1990, Joseph Katz/Mark Barton.  All rights reserved.	*
*   No part of this program may be reproduced, transmitted, or stored   *
*   in any language or computer system, in any form, whatsoever,	*
*   without the prior written permission of the authors.   		*
*                                                                    	*
*************************************************************************


*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
*                                                               ;
*             CONSTANTS USED IN THE F0 ALGORITHM                ;
*                                                               ;
*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


*-------- Percentages expressed in 128THS.  Each percentage is
*         separately calculated to avoid any round-off errors.

F0PCT5    EQU        (5*128+50)/100
F0PCT10   EQU       (10*128+50)/100
F0PCT15   EQU       (15*128+50)/100
F0PCT20   EQU       (20*128+50)/100
F0PCT30   EQU       (30*128+50)/100
F0PCT35   EQU       (35*128+50)/100
F0PCT40   EQU       (40*128+50)/100
F0PCT45   EQU       (45*128+50)/100
F0PCT50   EQU       (50*128+50)/100
F0PCT57   EQU       (57*128+50)/100
F0PCT60   EQU       (60*128+50)/100
F0PCT66   EQU       (67*128+50)/100
F0PCT70   EQU       (70*128+50)/100
F0PCT78   EQU       (78*128+50)/100
F0PCT80   EQU       (80*128+50)/100
F0PCT100  EQU       128


*-------- Bit positions in DUR and ACCENT arrays

F0WORDFB  EQU       7         ;WORD FINAL BIT
F0CLASFB  EQU       6         ;CLAUSE FINAL BIT
F0PUNITI  EQU       5         ;P-UNIT INITIATOR   (DUR ARRAY ONLY)
F0PUNITT  EQU       4         ;P-UNIT TERMINATOR  (DUR ARRAY ONLY)
F0STRSYB  EQU       5         ;STRESSED SYLLABLE  (ACCENT ARRAY ONLY)
F0INPUNB  EQU       4         ;INSIDE P-UNIT BIT   (ACCENT ARRAY ONLY)


*-------- Bit positions in STRESS array

F0POLY    EQU       6         ;MULTISYLLABIC BIT
F0SSBIT   EQU       5         ;STRESSED SYLLABLE BIT


*-------- Bit positions in CR/BREAK arrays

F0CRTYPE  EQU       7         ;CR TYPE BIT


*-------- CR/BREAK array masks

F0CRTYPM  EQU       $80       ;CR TYPE MASK
F0CRNUMM  EQU       $70       ;CR NUMBER MASK
F0CRMASK  EQU       $F0       ;CR TYPE AND NUMBER MASK
F0BRMASK  EQU       $0F       ;BREAK NUMBER MASK


*-------- ACCENT array masks and constants

F0WORDF   EQU       $80                 ;WORD FINAL MASK
F0CLAUSF  EQU       $40                 ;CLAUSE FINAL MASK
F0STRSYL  EQU       $20                 ;STRESSED SYL MASK
F0INPUN   EQU       $10                 ;INSIDE P-UNIT MASK
F0ACNTNO  EQU       $0F                 ;ACCENT NUMBER MASK (RELATED TO F0MAXSTR)

F0MAXSTR  EQU       15                  ;MAXIMUM STRESS (RELATED TO F0ACNTNO)
F0STRSCL  EQU       185			;Stress scale factor (13*128/9)


*-------- TUNE/LEVEL array masks and constants

F0TUNEMK  EQU       $0C       ;TUNE MASK
F0LEVMK   EQU       $03       ;LEVEL MASK

F0TUNEA   EQU       $04       ;TUNE A MARKER IN TUNE ARRAY
F0TUNEB   EQU       $08       ;TUNE B MARKER IN TUNE ARRAY


*-------- F0 frequency constants (all freqs in 2 Hz steps)

F0USE2HZ  EQU	      1				;USE 2 HZ STEPS FLAG
F05HZ	  EQU	(5+1)/2				;USED FOR 5HZ PERTURBATIONS
F010HZ	  EQU	  10/2				;USED TO ADJ F0 IN MANUAL MODE
F012HZ	  EQU	  12/2				;USED IN HEAD PK CALC (F0HDPEAK)
F0110HZ   EQU    110/2                   	;MIN BOR FREQ IN TUNE A
F0125HZ   EQU    115/2                   	;MIN BOR FREQ IN TUNE B
F0MAXHZ	  EQU	 400/2				;MAX F0 FREQUENCY ALLOWED
F0TERM    EQU     75/2                   	;TERMINAL FREQUENCY FOR TUNE A
F0MAXHPK  EQU    145/2                   	;MAXIMUM HEAD PEAK
F0MINHPK  EQU    125/2                   	;MINIMUM HEAD PEAK
F0BOR     EQU    110/2                   	;BOR
F0STDBOR  EQU    110/2                   	;"STANDARD" BOR
F0HDBASE  EQU    (F0BOR*123/F0STDBOR)/2    	;HEAD PEAK BASE FREQ



