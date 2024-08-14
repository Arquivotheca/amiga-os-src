        OPTIMON

;---------------------------------------------------------------------------

        NOLIST

        INCLUDE "exec/types.i"
        INCLUDE "exec/libraries.i"
        INCLUDE "exec/execbase.i"
        INCLUDE "exec/initializers.i"
        INCLUDE "exec/resident.i"

        INCLUDE "syscheck_rev.i"

        LIST

;---------------------------------------------------------------------------

	XDEF	SYSCHECKTAG

;---------------------------------------------------------------------------

	XREF	@SysCheckInit
	XREF	ENDCODE

;---------------------------------------------------------------------------

SYSCHECKTAG:

	IFND MACHINE_A600
        DC.W    RTC_MATCHWORD              ; UWORD RT_MATCHWORD
        DC.L    SYSCHECKTAG                ; APTR  RT_MATCHTAG
        DC.L    ENDCODE	                   ; APTR  RT_ENDSKIP
        DC.B    RTF_COLDSTART		   ; UBYTE RT_FLAGS
        DC.B    VERSION                    ; UBYTE RT_VERSION
        DC.B    NT_UNKNOWN                 ; UBYTE RT_TYPE
        DC.B    -35                        ; BYTE  RT_PRI
        DC.L    SysCheckName               ; APTR  RT_NAME
        DC.L    SysCheckName               ; APTR  RT_IDSTRING
        DC.L    @SysCheckInit              ; APTR  RT_INIT

SysCheckName DC.B 'syscheck',0

	ENDC

;-----------------------------------------------------------------------

        END
