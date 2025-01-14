;****** Included Files ***********************************************

    NOLIST
    INCLUDE     "exec/types.i"
    INCLUDE     "exec/nodes.i"
    INCLUDE     "exec/lists.i"
    INCLUDE     "exec/ables.i"
    LIST

;****** Exported Functions *******************************************

	XDEF        _FindNameI

;struct Node * ASM FindNameI (REG(a0) struct List *, REG(a1) STRPTR);

_FindNameI:
	MOVEM.L	D2/A2,-(SP)
	MOVE.L	A0,A2
	MOVE.L	A1,D1
	MOVE.L	(A2),D2
	BEQ.S	fnc_exit
fnc_next:
	MOVE.L  D2,A2               ; current node
	MOVE.L  (A2),D2             ; next node
	BEQ.S   fnc_exit            ; end of list
	;---- found a node
	MOVE.L  LN_NAME(A2),A0
	MOVE.L  D1,A1

	;[A0-candidate A1-master D0-scratch]
fnc_char:
	MOVE.B  (A0)+,D0            ; candidate character
	BEQ.S   fnc_endsource       ; check end...
	CMP.B   (A1)+,D0            ; compare with master
	BEQ.S   fnc_char            ; match...
	BCHG.B  #5,D0               ; toggle case bit
	CMP.B   -1(A1),D0
	BNE.S   fnc_next            ; mismatch...

	;Check that character is capsable.  Valid ranges are
	;A-Z a-z and all extended characters except multiply,
	;divide, sharp S and umlaut y.
	BCLR.B  #5,D0               ; cut case tests by two
	CMP.B   #'A',D0
	BLO.S   fnc_next            ; mismatch (below A / a)...
	CMP.B   #'[',D0
	BLO.S   fnc_char            ; match (below [ / {)...
	CMP.B   #$C0,D0
	BLO.S   fnc_next            ; mismatch (below special A)...
	CMP.B   #$D7,D0
	BEQ.S   fnc_next            ; mismatch (multiply / divide)...
	CMP.B   #$DF,D0
	BEQ.S   fnc_next	; mismatch (sharp s / umlaut y)...
	BRA.S   fnc_char

fnc_endsource:
	TST.B   (A1)
	BNE.S   fnc_next
	MOVE.L  A2,D2		; result

fnc_exit:
	;[D2-return value]
	MOVE.L  D1,A1
	MOVE.L	D2,D0		; return the results in D0
	MOVEM.L	(SP)+,A2/D2
	RTS

;FindNameI:
;	MOVEM.L D2/A2,-(SP)
;	MOVE.L  A0,A2               ; Start to A2
;	MOVE.L  A1,D1               ; Name to D1
;	MOVE.L  (A2),D2
;	BEQ.S   fnc_exit            ; list tail

	END
