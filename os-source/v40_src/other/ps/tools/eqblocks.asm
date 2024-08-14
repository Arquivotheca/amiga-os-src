****************************************************************************
**
** BOOL equal_blocks (char *block1, char *block2, int size)
**
****************************************************************************

				SECTION	code,data

				xdef	_equal_blocks

_equal_blocks:	move.l	4(sp),a0
				move.l	8(sp),a1
				move.l	12(sp),d0

				subq.w	#1,d0					;adjust for DBRA

compare_bytes	cmpm.b	(a0)+,(a1)+				;compare one 64K block MAX
				dbne	d0,compare_bytes
				bne.s	blocks_differ			;done on first difference
				
				swap	d0						;any more 64K blocks ?
				subq.w	#1,d0
				bmi		blocks_eq
				swap	d0						;yes: reconstruct long counter
				bra.s	compare_bytes			;and continue

blocks_eq		moveq	#-1,d0
				rts

blocks_differ	moveq	#0,d0
				rts

				END
