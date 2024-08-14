	TTL	'FEATMAT.ASM'


*************************************************************************	
*                                                                    	*
*   Copyright 1990, Joseph Katz/Mark Barton.  All rights reserved.	*
*   No part of this program may be reproduced, transmitted, or stored   *
*   in any language or computer system, in any form, whatsoever,	*
*   without the prior written permission of the authors.   		*
*                                                                    	*
*************************************************************************


        SECTION    speech



*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
*                                     ;
*         FEATURE   MATRIX            ;
*                                     ;
*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

      XDEF   	FEAT

      INCLUDE   'featequ.i'


FEAT  dc.l   IGNORE+WORDBRK  				;BLANK  0
      dc.l   BOUND+SILENT+PAUSE+WORDBRK+TERM 		;. 	1
      dc.l   BOUND+SILENT+PAUSE+WORDBRK+TERM  		;?	2
      dc.l   BOUND+SILENT+PAUSE+WORDBRK  		;,	3
      dc.l   SILENT+PAUSE+WORDBRK  			;-	4
      dc.l   IGNORE  					;(	5
      dc.l   IGNORE  					;)	6
      dc.l   IGNORE					;RESERVED 7
      dc.l   IGNORE  					;RESERVED 8
      dc.l   VOWEL+SONOR+FRONT+VOICED 	 		;IY     9
      dc.l   VOWEL+SONOR+FRONT+VOICED  			;IY'	10
      dc.l   VOWEL+SONOR+FRONT+VOICED  			;IH	11
      dc.l   VOWEL+SONOR+FRONT+VOICED  			;IH'
      dc.l   VOWEL+SONOR+VOICED  			;EH	13
      dc.l   VOWEL+SONOR+VOICED  			;EH'
      dc.l   VOWEL+SONOR+VOICED	  			;AE	15
      dc.l   VOWEL+SONOR+VOICED  			;AE'
      dc.l   VOWEL+SONOR+MID+VOICED  			;AA	17
      dc.l   VOWEL+SONOR+BACK+VOICED  			;AH	18
      dc.l   VOWEL+SONOR+BACK+VOICED  			;AO	19
      dc.l   VOWEL+SONOR+BACK+VOICED  			;AO'	20
      dc.l   VOWEL+SONOR+BACK+VOICED	  		;UH	21
      dc.l   VOWEL+SONOR+BACK+VOICED  			;UH'	22
      dc.l   VOWEL+SONOR+BACK+VOICED  			;AX	23
      dc.l   VOWEL+SONOR+FRONT+VOICED  			;IX	24
      dc.l   VOWEL+SONOR+MID+VOICED			;ER	25
      dc.l   VOWEL+SONOR+MID+VOICED  			;ER'	26
      dc.l   PAUSE+SONOR+MID+VOICED+SILENT		;QX 	27
      dc.l   VOICED		  			;AXP	28
      dc.l   VOWEL+SONOR+MID+VOICED+LIQUID  		;RX	29
      dc.l   VOWEL+SONOR+MID+VOICED+LIQUID  		;LX	30
      dc.l   VOWEL+SONOR+DIPH+VOICED	  		;EY	31
      dc.l   VOWEL+SONOR+VOICED  			;EY'	32
      dc.l   VOWEL+SONOR+MID+DIPH+VOICED 		;AY	33
      dc.l   VOWEL+SONOR+VOICED  			;AY'	34
      dc.l   VOWEL+SONOR+BACK+DIPH+VOICED  		;OY	35
      dc.l   VOWEL+SONOR+VOICED  			;OY'	36
      dc.l   VOWEL+SONOR+MID+DIPH+VOICED  		;AW	37
      dc.l   VOWEL+SONOR+BACK+ROUND+VOICED  		;AW'	38
      dc.l   VOWEL+SONOR+BACK+DIPH+VOICED  		;OW 	39
      dc.l   VOWEL+SONOR+BACK+ROUND+VOICED  		;OW'	40
      dc.l   VOWEL+SONOR+BACK+ROUND+DIPH+VOICED  	;UW	41
      dc.l   VOWEL+SONOR+BACK+ROUND+VOICED  		;UW'	42
      dc.l   VOWEL+SONOR+FRONT+DIPH+VOICED		;IXR	43
      dc.l   VOWEL+SONOR+MID+VOICED			;IXR'	44
      dc.l   VOWEL+SONOR+DIPH+VOICED			;EXR	45
      dc.l   VOWEL+SONOR+MID+VOICED			;EXR'	46
      dc.l   VOWEL+SONOR+MID+DIPH+VOICED		;AXR	47
      dc.l   VOWEL+SONOR+MID+VOICED			;AXR'	48
      dc.l   VOWEL+SONOR+BACK+DIPH+VOICED		;OXR	49
      dc.l   VOWEL+SONOR+MID+VOICED			;OXR'	50
      dc.l   VOWEL+SONOR+BACK+ROUND+DIPH+VOICED		;UXR	51
      dc.l   VOWEL+SONOR+MID+VOICED			;UXR'	52
      dc.l   CONS+SONOR+BACK+ROUND+VOICED+GLIDE  	;WH	53
      dc.l   CONS+SONOR+BACK+VOICED+LIQUID  		;R	54
      dc.l   CONS+SONOR+BACK+VOICED+LIQUID  		;L	55
      dc.l   CONS+SONOR+BACK+ROUND+VOICED+GLIDE  	;W	56
      dc.l   CONS+SONOR+FRONT+VOICED+GLIDE  		;Y	57
      dc.l   VOWEL+SONOR+FRONT+DIPH+VOICED		;YU	58
      dc.l   VOWEL+SONOR+BACK+ROUND+VOICED		;YU'	59
      dc.l   CONS+SONOR+VOICED+STOPP+NASAL+LABIAL  	;M	60
      dc.l   CONS+SONOR+VOICED+STOPP+NASAL+DENTAL  	;N	61
      dc.l   CONS+SONOR+VOICED+FRONT+STOPP+NASAL+PALATE ;NX	62
      dc.l   CONS+VOICED+STOPP+PLOS+DENTAL 		;DX	63
      dc.l   CONS+STOPP+GLOTTAL	  			;Q	64
      dc.l   CONS+MID+FRIC+DENTAL			;S	65
      dc.l   CONS+FRIC+PALATE  				;SH	66
      dc.l   CONS+FRIC+LABIAL  				;F	67
      dc.l   CONS+FRIC+DENTAL  				;TH	68
      dc.l   CONS+VOICED+FRIC+DENTAL	  		;Z	69
      dc.l   CONS+VOICED+FRIC+PALATE  			;ZH	70
      dc.l   CONS+VOICED+FRIC+LABIAL  			;V	71
      dc.l   CONS+VOICED+FRIC+DENTAL  			;DH	72
      dc.l   CONS+STOPP+PLOSA+AFFRIC+PALATE  		;CH	73
      dc.l   CONS+VOICED+STOPP+PLOS+AFFRIC+FRIC+PALATE 	;J	74
      dc.l   CONS+ASPIR  				;/H	75
      dc.l   CONS+ASPIR+VOICED				;HX     76
      dc.l   CONS+VOICED+STOPP+PLOS+LABIAL  		;B	77
      dc.l   CONS+VOICED+STOPP+PLOS+DENTAL  		;D	78
      dc.l   CONS+VOICED+STOPP+PLOS+PALATE  		;G	79
      dc.l   CONS+VOICED+STOPP+PLOS+PALATE  		;GX	80
      dc.l   CONS+STOPP+PLOSA+LABIAL  			;P	81
      dc.l   CONS+STOPP+PLOSA+DENTAL  			;T	82
      dc.l   CONS+STOPP+VOICED+GLOTTAL			;TQ	83
      dc.l   CONS+STOPP+PLOSA+PALATE  			;K	84
      dc.l   CONS+STOPP+PLOSA+PALATE  			;KX	85
      dc.l   VOWEL+SONOR+MID+VOICED  			;UL 	86
      dc.l   VOWEL+SONOR+MID+VOICED  			;UM	87
      dc.l   VOWEL+SONOR+MID+VOICED  			;UN	88
      dc.l   VOWEL+SONOR+VOICED  			;IL	89
      dc.l   VOWEL+SONOR+VOICED  			;IM	90
      dc.l   VOWEL+SONOR+VOICED  			;IN	91
      dc.l   0          				;OH     92
      dc.l   0          				;/C     93
      dc.l   NUMBER					;0	94
      dc.l   NUMBER					;1	95
      dc.l   NUMBER					;2	96
      dc.l   NUMBER					;3	97
      dc.l   NUMBER					;4	98
      dc.l   NUMBER					;5	99
      dc.l   NUMBER					;6     100
      dc.l   NUMBER					;7     101
      dc.l   NUMBER					;8     102
      dc.l   NUMBER					;9     103

      END 
