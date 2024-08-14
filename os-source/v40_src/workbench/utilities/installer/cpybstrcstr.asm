*****************************************************
*		Parsec Soft Systems string functions		*
*****************************************************

			section	code

			include 'macros.i'

			DECLAREA	CpyBstrCstr

			move.l	4(sp),a0
			move.l	8(sp),a1
CpyBstrCstr
			adda.l	a0,a0			; BPTR -> CPTR
			adda.l	a0,a0
			moveq	#0,d0
			move.b	(a0),d0			; length
			exg.l	a0,a1			; make order right for str_ncpy
			addq.w	#1,a1			; string starts one above length
			move.l	a0,d1
			bra.s	2$

1$			move.b	(a1)+,(a0)+

		ifnd	BIGSTRING
2$			dbeq	d0,1$
			beq.s	3$
		endc
		ifd		BIGSTRING
			beq.s	3$
			subq.w	#1,d0
			bpl.s	2$
		endc
			clr.b	(a0)

3$			move.l	d1,d0
			rts

			end

#include "exec/types.h"

#define BTOC(bptr)    ((long)(bptr) << 2)

char *CpyBstrCstr(b,c) char *b,*c;
{	char *a = (char *)BTOC(b);
	return str_ncpy(c,a+1,*a);
}
