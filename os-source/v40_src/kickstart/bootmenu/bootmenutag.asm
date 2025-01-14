        OPTIMON

;---------------------------------------------------------------------------

        NOLIST

        INCLUDE "exec/types.i"
        INCLUDE "exec/libraries.i"
        INCLUDE "exec/execbase.i"
        INCLUDE "exec/initializers.i"
        INCLUDE "exec/resident.i"

        INCLUDE "bootmenu_rev.i"

        LIST

;---------------------------------------------------------------------------

        XREF    SYSCHECKTAG
        XREF	@BootMenuInit

;---------------------------------------------------------------------------

BOOTMENUTAG:
        DC.W    RTC_MATCHWORD              ; UWORD RT_MATCHWORD
        DC.L    BOOTMENUTAG                ; APTR  RT_MATCHTAG
        DC.L    SYSCHECKTAG                ; APTR  RT_ENDSKIP
        DC.B    RTF_COLDSTART		   ; UBYTE RT_FLAGS
        DC.B    VERSION                    ; UBYTE RT_VERSION
        DC.B    NT_UNKNOWN                 ; UBYTE RT_TYPE
        DC.B    -50                        ; BYTE  RT_PRI
        DC.L    BootMenuName               ; APTR  RT_NAME
        DC.L    BootMenuId                 ; APTR  RT_IDSTRING
        DC.L    @BootMenuInit	           ; APTR  RT_INIT

BootMenuName DC.B 'bootmenu',0
BootMenuId   VSTRING

;-----------------------------------------------------------------------

        END
