* startup code for C shell

        include "exec/types.i"
        include "exec/nodes.i"
        include "exec/lists.i"
        include "exec/libraries.i"
        include "exec/initializers.i"
        include "exec/resident.i"
	

	include "libhdr.i"
	include "shell_rev.i"

	XREF @startup
	XREF _startup

	SECTION	text,code

* remove the first 0 for ram use

*	DC.L	0			; no next segment
start:	DC.L	(end-start)/4

* startup is the C entry point for the C shell

Entry:	movem.l	A0/A1/A6,-(SP)
	lsl.l	#2,D1
	move.l D1,A0
	move.l #1,A0	;signal C startup
	jsr _startup
	movem.l (SP)+,A0/A1/A6
	rts
	jmp (A6)

; now the fun stuff
	
	CNOP 0,4
	DC.l	0

centry:	movem.l	A0/A1/A6,-(SP)
	move.l #1,A0	;signal C startup
	jsr _startup
	movem.l (SP)+,A0/A1/A6
	rts

	CNOP	0,4

	DC.L	0
	DC.L	G_START/4,(Entry-start)
	DC.L	G_CLISTART/4,(Entry-start)
	DC.L	G_GLOBMAX/4

end:


start_tab
          DC.L    start-4                  ; APTR  RT_INIT
	  DC.L	  centry-4

	XREF	_endcode
	
;-----------------------------------------------------------------------
; A romtag structure.  Both "exec" and "ramlib" look for
; this structure to discover magic constants about you
; (such as where to start running you from...).
;-----------------------------------------------------------------------

romtag:                                 ;STRUCTURE RT,0
          DC.W    RTC_MATCHWORD         ; UWORD RT_MATCHWORD
          DC.L    romtag                ; APTR  RT_MATCHTAG
          DC.L    _endcode              ; APTR  RT_ENDSKIP
          DC.B    0  		        ; UBYTE RT_FLAGS
          DC.B    36                    ; UBYTE RT_VERSION
          DC.B    NT_UNKNOWN            ; UBYTE RT_TYPE
          DC.B    -122                  ; BYTE  RT_PRI
          DC.L    myName                ; APTR  RT_NAME
          DC.L    idString              ; APTR  RT_IDSTRING
          DC.L    start_tab             ; APTR  RT_INIT
                                        ; LABEL RT_SIZE
	CNOP	0,2
myName: dc.b 'shell',0

        ; this is an identifier tag to help in supporting the library
        ; format is 'name version.revision (dd MON yyyy)',<cr>,<lf>,<null>
	CNOP	0,2
	dc.b '$VER: '
idString:   VSTRING
	    ds.w 0	; force word alignment


	section __MERGED,DATA

	CNOP	0,4

x:	DC.L	(y-x)/4
	DC.L	0
	DC.L	0
	DC.L	G_GLOBMAX/4
y:
	END

