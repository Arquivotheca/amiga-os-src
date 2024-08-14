      IFND  MY_MACROS_ASM
MY_MACROS_ASM  SET   1
*==========================================================================
*=====  MACROS I LIKE TO USE A LOT ========================================
*==========================================================================

* For declaring external system calls
EXT_SYS     MACRO
      XREF  _LVO\1
      ENDM

* For declaring external data
EXT_DATA     MACRO
      XREF  _LVO\1
      ENDM

* For calling system routines by name (eg. SYS  OpenWindow(A6) )
SYS         MACRO
      JSR   _LVO\1
      ENDM

*==========================================================================

* For fetching the exec pointer into A6
GET_EXECPTR MACRO
      MOVEA.L     _AbsExecBase,A6
      ENDM

* For fetching the intuition pointer into A6 (Requires IntPtr to be defined)
GET_INTPTR  MACRO
      MOVEA.L     IntPtr,A6
      ENDM

* For fetching the graphics pointer into A6 (Requires GfxPtr to be defined)
GET_GFXPTR  MACRO
      MOVEA.L     GfxPtr,A6
      ENDM

* For fetching the dos pointer into A6 (Requires DosPtr to be defined)
GET_DOSPTR  MACRO
      MOVEA.L     DosPtr,A6
      ENDM

*=========================================================================

* For fetching a structure member (GET_MEMBER .W,screen,sc_Width,A0,D0)
* in this example, screen is a longword containing the address of the struct
* A0 is the address register to use and D0 is the destination (can be addr)
GET_MEMBER  MACRO
      MOVEA.L     \2,\4
      MOVE\1      \3(\4),\5
      ENDM

* For fetching address of a structure member (GET_ADDR  screen,sc_BitMap,A0)
* in this example, screen is a longword containing the address of the struct
* A0 is the address register to use for the final address
GET_ADDR    MACRO
      MOVEA.L     \1,\3
      ADDA.L      #\2,\3
      ENDM

*==========================================================================
      ENDC
