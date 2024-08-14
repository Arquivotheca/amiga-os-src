
	xref	_netPROCESS
	xdef	_NetSegment

        CNOP    0,4             	; longword align
        DC.L    0               	; size (unknown)
_NetSegment:
        DC.L    0               	; bptr to next seg (none)
        ;------ open inet library
        moveq.l #1,d0 			;V1 inet.library...
        lea     MyName(pc),a1
        move.l  4,a6			; AbsExecBase
        jsr     -$228(a6)     		; OpenLibrary
                                        ; We know we are open...
;
        move.l  a6,-(sp)        ; Save a6
        move.l  d0,-(sp)        ;   and the library pointer...
        move.l  d0,a6           ; Set library pointer into a6...
        jsr     _netPROCESS     ; Call the code...
        move.l  (sp)+,a1        ; Get library pointer ready for CloseLibrary
        move.l  (sp)+,a6        ; Get execbase again...
;
        jmp     -$19E(a6)    	; Close library...
                                ; CloseLibrary does the RTS for us...


MyName	dc.b	'inet.library',0
