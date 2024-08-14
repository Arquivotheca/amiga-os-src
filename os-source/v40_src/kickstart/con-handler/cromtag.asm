*******************************************************************************
*
* $Id: cromtag.asm,v 1.9 92/02/13 17:50:58 mks Exp $
*
*******************************************************************************

        SECTION section

        NOLIST
        INCLUDE "exec/types.i"
        INCLUDE "exec/nodes.i"
        INCLUDE "exec/lists.i"
        INCLUDE "exec/libraries.i"
        INCLUDE "exec/initializers.i"
        INCLUDE "exec/resident.i"
        LIST

	INCLUDE "con-handler_rev.i"

	XREF @start
	XREF EndCode

	XDEF @XCEXIT

        ; The first executable location.  This should return an error
        ; in case someone tried to run you as a program (instead of
        ; loading you as a library).
START:
        jmp @start

;-----------------------------------------------------------------------
; A romtag structure.  Both "exec" and "ramlib" look for
; this structure to discover magic constants about you
; (such as where to start running you from...).
;-----------------------------------------------------------------------

initDDescrip:
                                        ;STRUCTURE RT,0
          DC.W    RTC_MATCHWORD         ; UWORD RT_MATCHWORD
          DC.L    initDDescrip          ; APTR  RT_MATCHTAG
          DC.L    EndCode		; APTR  RT_ENDSKIP
          DC.B    0  		        ; UBYTE RT_FLAGS
          DC.B    VERSION               ; UBYTE RT_VERSION
          DC.B    NT_UNKNOWN            ; UBYTE RT_TYPE
          DC.B    -121                  ; BYTE  RT_PRI
          DC.L    myName                ; APTR  RT_NAME
          DC.L    idString              ; APTR  RT_IDSTRING
          DC.L    @start                ; APTR  RT_INIT
                                        ; LABEL RT_SIZE

myName: dc.b 'con-handler',0
idString:   VSTRING
	ds.w	 0         ; force word allignment

* EndCode:
Init:
@XCEXIT:
	rts

	END
