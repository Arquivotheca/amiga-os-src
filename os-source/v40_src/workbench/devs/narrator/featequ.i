	TTL	'FEATEQU.I'


*************************************************************************	
*                                                                    	*
*   Copyright 1990, Joseph Katz/Mark Barton.  All rights reserved.	*
*   No part of this program may be reproduced, transmitted, or stored   *
*   in any language or computer system, in any form, whatsoever,	*
*   without the prior written permission of the authors.   		*
*                                                                    	*
*************************************************************************


            NOLIST
*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
*                                    ;
*    FEATURE  MATRIX  EQUATES        ;
*                                    ;
*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
*
VOWEL       EQU          1
VOWELBIT    EQU          0
CONS        EQU          2
CONSBIT     EQU          1
SONOR       EQU          4
SONORBIT    EQU          2
FRONT       EQU          8
FRONTBIT    EQU          3
MID         EQU          $10
MIDBIT      EQU          4
BACK        EQU          $20
BACKBIT     EQU          5
ROUND       EQU          $40
ROUNDBIT    EQU          6
DIPH        EQU          $80
DIPHBIT     EQU          7
STOPP       EQU          $100
STOPBIT     EQU          8
VOICED      EQU          $200
VOICEDBIT   EQU          9
PLOS        EQU          $400
PLOSBIT     EQU          10
PLOSA       EQU          $800
PLOSABIT    EQU          11
FRIC        EQU          $1000
FRICBIT     EQU          12
ASPIR       EQU          $2000
ASPIRBIT    EQU          13
AFFRIC      EQU          $4000
AFFRICBIT   EQU          14
LIQUID      EQU          $8000
LIQUIDBIT   EQU          15
NASAL       EQU          $10000
NASALBIT    EQU          16
GLIDE       EQU          $20000
GLIDEBIT    EQU          17
DENTAL      EQU          $40000
DENTALBIT   EQU          18
BOUND       EQU          $80000
BOUNDBIT    EQU          19
IGNORE      EQU          $100000
IGNOREBIT   EQU          20
PRIME       EQU          $200000
PRIMEBIT    EQU          21
SILENT      EQU          $400000
SILENTBIT   EQU          22
LABIAL      EQU          $800000
LABIALBIT   EQU          23
PALATE      EQU          $1000000
PALATEBIT   EQU          24
PAUSE       EQU          $2000000
PAUSEBIT    EQU          25
WORDBRK     EQU          $4000000
WORDBRKBIT  EQU          26
INVALID     EQU          $8000000
INVALIDBIT  EQU          27
TERM	    EQU		 $10000000
TERMBIT	    EQU		 28
GLOTTAL     EQU		 $20000000
GLOTTALBIT  EQU		 29
NUMBER	    EQU		 $40000000
NUMBERBIT   EQU		 30


* !!! CAUTION !!! Bit 31 can NEVER be used as a feature !!!

            LIST


