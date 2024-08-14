* ========================================================================
* ToEnglish.asm -- convert character to upper-case english if possible
* Written Oct 1990 by Talin.
* ========================================================================

			section	toenglish.asm,code

			xdef	_ToEnglish,ToEnglish

_ToEnglish:
			move.w	6(sp),d0
ToEnglish:
			cmp.b	#$c0,d0
			blo.s	1$
			lea		foreign_chars-$c0(pc),a0
			move.b	0(a0,d0.w),d0
			rts
1$			cmp.b	#'a',d0
			blo.s	2$
			cmp.b	#'z',d0
			bhi.s	2$
			sub.b	#32,d0
2$			rts

foreign_chars
			dc.b	"AAAAAAÆCEEEEIIIIDNOOOOO×OUUUUYPS"
			dc.b	"AAAAAAÆCEEEEIIIIDNOOOOO÷OUUUUYPY"

			end
