; ------------------------------------------------------------------------------
; sslib_def.i           assembler defines for ss.libray
;
; $Locker$
;
; $Id$
;
; $Revision$
;
; $Header$
;
;-------------------------------------------------------------------------------

        IFND    SSLIB_DEF_I
SSLIB_DEF_I     SET 1

        INCLUDE "exec/lists.i"

; -- library base offsets from base structure

        LIBINIT
        LIBDEF  FUNC0
        LIBDEF  FUNC1
        LIBDEF  FUNC2
        LIBDEF  FUNC3
        LIBDEF  FUNC4
        LIBDEF  FUNC5
        LIBDEF  FUNC6
        LIBDEF  FUNC7

; -- sslib data area structure - this is appended to the library structure
;    at offset LIB_SIZE

        STRUCTURE       SSLib,LIB_SIZE
                ULONG   ss_SysLib
                ULONG   ss_DosLib
                ULONG   ss_InetLib
                ULONG   ss_SegList
                UBYTE   ss_Flags
                UBYTE   ss_pad
                LABEL   SSLib_Sizeof

; name of the library
SSLIBNAME       MACRO
        dc.b    'ss.library',0
        ENDM

INETLIBNAME       MACRO
        dc.b    'inet:libs/inet.library',0
        ENDM


; ============== end file sslib_def.i ============

        ENDC    ; SSLIB_DEF_I

;   ***** END *******
