StartJFHS	DC.L	(EndJFHS-StartJFHS)/4
StartCode	MOVE.W	#$00,$DFF180		make screen black
		BRA.S	StartCode		and just keep doing it
;------ look like a BCPL module ------
		CNOP	0,4
		DC.L	0
		DC.L	1			; index of start entry
		DC.L	StartCode-StartJFHS	; offset of start entry
		DC.L	0			; no globals referenced
EndJFHS:

		END
