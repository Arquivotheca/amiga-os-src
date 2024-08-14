include	title.inc
subttl	 Filler Module  

include equates.inc

eproma	segment	public

	assume	cs:eproma

public aa_fill
aa_fill label byte

;	db	8000h dup (0)	;fill first 32kb with zero

IF BRIDGEBOARD
	db	05c0h dup (0ffh)	;adjust this size until top8k starts
					;at E000.
ELSE
	db	00a0h dup (0ffh)	;adjust this size until top8k starts
					;at E000.
ENDIF

public aa_adjust_to_e000

aa_adjust_to_e000 label byte

eproma	ends
	end

