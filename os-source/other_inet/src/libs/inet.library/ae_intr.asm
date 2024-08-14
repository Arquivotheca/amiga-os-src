;
; Lance interrupt stub.
;

	xref	_aeintrC
	xdef	_ae_intr

	xref	_aaintrC
	xdef	_aa_intr

_ae_intr:

        move.l  a1,a6         ; Set library pointer into a6...
        jsr     _aeintrC      ; Call the code...
        moveq	#0,d0
	rts

_aa_intr:

        move.l  a1,a6         ; Set library pointer into a6...
        jsr     _aaintrC      ; Call the code...
        moveq	#0,d0
	rts
