	TTL	'PHONET.I'


*************************************************************************	
*                                                                    	*
*   Copyright 1990, Joseph Katz/Mark Barton.  All rights reserved.	*
*   No part of this program may be reproduced, transmitted, or stored   *
*   in any language or computer system, in any form, whatsoever,	*
*   without the prior written permission of the authors.   		*
*                                                                    	*
*************************************************************************


*		;------ Assembler version of the PhonetParms structure.

  STRUCTURE PP,0
    APTR	PP_CUMDUR		;Pointer to current segment
    APTR	PP_LOCAL 		;Pointer to start of local interp area
    UBYTE	PP_OLDDUR		;Duration of previous segment (frames)
    UBYTE	PP_SEGDUR		;Duration of current segment (frames)
    UBYTE	PP_TRANTYPE		;Transition type
    UBYTE	PP_PAD			;For alignment
    UWORD	PP_MINTIME		;Max back prop of smoothing (frames)
    WORD	PP_TARGET		;Final value at Cumdur+Segdur
    WORD	PP_OLDVAL		;Value at frame before Cumdur
    BYTE	PP_TCF			;Forward smoothing time (in frames)
    BYTE	PP_TCB			;Backward smoothing time (in frames)
    WORD	PP_BPER			;Boundry percentage
    WORD	PP_BVF			;Forward boundry target
    WORD	PP_BVB			;Backward boundry target
    LABEL	PP_SIZE			;Size of structure



*		;------ Trantype EQUs.  Must match those found in phonet.h.

DISCON	EQU	0
DISSMO	EQU	1
SMODIS	EQU	2
SETSMO	EQU	3

