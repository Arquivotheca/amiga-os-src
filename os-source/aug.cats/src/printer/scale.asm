**********************************************************************
*                                                                    *
*   Copyright 1985, Commodore-Amiga Inc.   All rights reserved.      *
*   No part of this program may be reproduced, transmitted,          *
*   transcribed, stored in retrieval system, or translated into      *
*   any language or computer language, in any form or by any         *
*   means, electronic, mechanical, magnetic, optical, chemical,      *
*   manual or otherwise, without the prior written permission of     *
*   Commodore-Amiga Incorporated, 983 University Ave. Building #D,   *
*   Los Gatos, California, 95030                                     *
*                                                                    *
**********************************************************************
*
**********************************************************************

	xdef _Scale

; Scale(mult, mod, dim, et)
;		4		8	12	16
; UWORD mult, mod, dim
; WORD et
; modified Bresenham scaling.  Returns an integer >= mult.

_Scale
	move.l	4(a7),d0	; put mult in return register
	move.l	16(a7),a0	; get address of error term
	move.l	8(a7),d1	; get mod
	sub.w	d1,(a0)		; subtract mod from error term. gone too far?
	bgt.s	scale_exit	; no, so exit
	addq.w	#1,d0		; bump return value
	move.l	12(a7),d1	; get dim
	add.w	d1,(a0)		; add dim to error term
scale_exit:
	rts

	end
