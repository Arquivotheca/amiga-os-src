; To be assembled with casm.
; The sas assembler chokes big-time on this file for some reason.
; Ah, well.
; casm -a myOpenLib.asm -o myOpenLib.o
; then link away with the C...

; handy C prototype:
; extern ULONG __saveds __asm myOpenLib(register __a1 STRPTR, register __d0 ULONG);
;                                       LIBRARY NAME,          LIBRARY VERSION
; returns library base in d0
; See LibMon.c for docs.  This is the replacement function for OpenLibrary.
;

    INCLUDE "include:exec/types.i"
    INCLUDE "include:exec/macros.i"
    

    XREF _LVOStackSwap       ; amiga.lib
    XREF _LVOForbid          ; amiga.lib
    XREF _LVOPermit          ; amiga.lib
    XREF _myStackP           ; libmon.c
    XREF _closeLibGuts       ; libmon.c
    XREF _SysBase            ; cback.o

    XDEF _myCloseLib
    XDEF myCloseLib


    _myCloseLib:
    myCloseLib:

    movem.l     a2-a3/a6,-(sp)     ; save registers
    move.l      a1,a2              ; lib name
    lea         3*4(sp),a3         ; ptr to stack of caller

    ; Swap the stack:  If Locale is present, it turns KPrintF into a hog!
    move.l      _SysBase,a6         ; Exec calls forthcoming
    jsr         _LVOForbid(a6)      ; be safe :-)
    move.l      _myStackP,a0        ; global stackswap struct 
    jsr         _LVOStackSwap(a6)   ; swap out

    jsr         _closeLibGuts(pc)    ; Where The Work Is Done.
                                     ; a2=libName, a3=Stack

    ; Note: if closeLibGuts() ever grows a return value, save it here!

    ; Un-Swap the stack
    unSwap:
    move.l      _SysBase,a6         ; Exec calls forthcoming
    move.l      _myStackP,a0        ; global stackswap struct
    jsr         _LVOStackSwap(a6)   ; swap out
    jsr         _LVOPermit(a6)      ; Finally break the forbid.

    ; restore registers
    movem.l     (sp)+,a2-a3/a6

    ; farewell
    rts

end
