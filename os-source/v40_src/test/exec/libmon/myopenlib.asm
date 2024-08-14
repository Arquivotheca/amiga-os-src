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
    XREF _openLibGuts        ; libmon.c
    XREF _SysBase            ; cback.o

;   Needed if you use the OLD CODE section.
;   XREF _LVOFreeVec         ; amiga.lib
;   XREF _KPrintF            ; debug.lib
;   XREF _oldOpenLib         ; libmon.c
;   XREF _myFmtStr           ; libmon.c
;   XREF _blankString        ; libmon.c
;   XREF @getTaskName        ; getTaskName.c

    XDEF _myOpenLib
    XDEF myOpenLib


    _myOpenLib:
    myOpenLib:

    movem.l     a2-a3/a6/d2,-(sp)   ; save registers
    move.l      a1,a2               ; lib name
    move.l      d0,d2               ; version
    lea         4*4(sp),a3          ; ptr to stack of caller  (4 or 5??)

    ; Swap the stack:  If Locale is present, it turns KPrintF into a hog!
    move.l      _SysBase,a6         ; Exec calls forthcoming
    jsr         _LVOForbid(a6)      ; be safe :-)
    move.l      _myStackP,a0        ; global stackswap struct 
    jsr         _LVOStackSwap(a6)   ; swap out

    jsr         _openLibGuts(pc)    ; Where The Work Is Done.
                                    ; a2=libName, a3=Stack, d2=LibVer
    move.l      d0,a2               ; save result

    ; Un-Swap the stack
    unSwap:
    move.l      _SysBase,a6         ; Exec calls forthcoming
    move.l      _myStackP,a0        ; global stackswap struct
    jsr         _LVOStackSwap(a6)   ; swap out
    jsr         _LVOPermit(a6)      ; Finally break the forbid.

    move.l      a2,d0               ; result of openLibGuts()

    ; restore registers
    movem.l     (sp)+,a2-a3/a6/d2

    ; farewell
    rts

; OLD CODE ----------------------------------------------------
;    ; Get string describing the task, or NULL for failure
;    jsr         @getTaskName(pc) ; C function with VOID args.
;    move.l      d0,d3            ; save task/proc string to d3.  Must FreeVec later.
;    bne.s       pushTask         ; if not null, goto push
;    pea         _blankString     ; in libmon.c
;    bra.s       printIt
;
;    pushTask:
;    move.l      d3,-(sp)
;
;                                 ; either d3 or _blankString is pushed.
;
;    ; Print the info
;    printIt:
;    move.l      d2,-(sp)         ; library version
;    move.l      a2,-(sp)         ; library name
;    pea         _myFmtStr        ; kprintf format string
;    jsr         _KPrintF(pc)
;    add.l       #16,sp           ; fix stack -- add (4 ptrs * 4 bytes/ptr)
;
;    tst.l       d3               ; must we FreeVec it?
;    beq.s       unSwap           ; not if it's null
;    move.l      d3,a1            ; load for freevec
;    jsr         _LVOFreeVec(a6)  ; free up the string
;
;    ; Un-Swap the stack
;    unSwap:
;    move.l      _myStackP,a0
;    jsr         _LVOStackSwap(a6)
;
;    ; put libname/ver back to prep for OpenLibrary call
;    move.l      d2,d0
;    move.l      a2,a1
;
;    ; call original OpenLibrary
;    move.l      _oldOpenLib,a2
;    jsr         (a2)
;
;    ; restore registers
;    movem.l     (sp)+,a2-a3/a6/d2-d3
;
;    ; farewell
;    rts
;
;
; this is now dealt with in the C module...
;myFmtStr:
;    dc.b "OpenLibrary('%s', %lu);',10,0

end

