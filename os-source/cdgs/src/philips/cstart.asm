$CASE
$DEBUG
; %'@(#)cstart.asm      1.12    8/31/92'
;       Header file for 8051 type processors for C8051
;       -----------------------------------------------
;
;       When using the reentrant model, and interrupt protection of updating
;       the virtual stackpointer is required. Define PROTECT as YES
;
        NAME    _CSTART 
;
        EXTRN   CODE(_main)     ; start label user program
        PUBLIC  __STKSTART      ; start of stack (also used by XRAY51)
        PUBLIC  __EXIT          ; address to jump to on 'exit()'

;       -----------------------------------------------
;       IMPORTANT
;
;       Next values must be set correctly to get correct execution of programs
;
;       'HEAP' determines the maximum space that can be 'malloc()'ed, when the
;       function 'malloc' is not used in a program, this value should be '0'
;       When 'HEAP' has a size 0, No HEAP is allocated. When using 'malloc()'
;       will result in a linker error
;
;       'STACKLENGTH' value will create the possibility to create a minimum
;       stacklength. The stack is placed at the end of internal RAM. Filling
;       'STACKLENGTH' with a value will enable the linker to produce an error
;       if the stack does not fit in RAM anymore next to all other variables.
;
;       'RAMSIZE' determines the amount of internal (on chip) RAM that should
;       be cleared at startup
;       'XDATSTART' and 'XDATEND' determine the location and size of external
;       RAM shat should be cleared at startup
;       Clearance of the memory is done to give all non initialized 'static'
;       variables the value '0' at startup (defined by ANSI)
;
;       'REGBANK' determines the registerbank used at startup (Normally '0').
;       This value should be equal to the registerbank used by the 'main()'
;       routine.
;
;       'VIRT_STACK' is only used with the 'REENTRANT' model of C-51. This
;       space is used to place all Function parameters, Function locals,
;       Function return addresses, etc. during program execution.
;
;       'PROTECT' should be set to 'YES' only when the C-51 Program is
;       compiled with the 'REENTRANT' model, AND when interrupts may occur
;       during program execution (only when the interrupt procedure needs to
;       use the virtual stack in any way)
;
;       'FLOATMEM' should be set to the number of floats that should be
;       allowed on the 'floating point stack'. When floating point is
;       used within the application, this value MUST be set to at least
;       '2'. Any action with a floating point variable will use this 'stack'
;       The stack should be configured as large as needed to do the most
;       complex expression used in the application (for routines from the
;       'math.h' library, the given size ('5') will do).
;
;       'MFLOAT' should be set to 'XDATA' when floating point is used and the library
;       'float.lib' is linked. 'MFLOAT' should be set to 'IDATA' when the small library
;       'floats.lib' is linked. Which library can be used in what circumstances is explained
;       in the C-51 manual.
;
;       When the program does not make use of these features, values as
;       presented here do not have to be changed
;       -----------------------------------------------
%DEFINE(HEAP)           (0H)            ; define a heap (room for malloc in
                                        ; xdat) of 0 bytes
%DEFINE(STACKLENGTH)    (20H)           ; define a stacklength of 32 bytes
%DEFINE(RAMSIZE)        (255)           ; size of IDAT memory (in the range
                                        ; from 128 to 256)
%DEFINE(XDATSTART)      (0H)            ; start address of XDAT
%DEFINE(XDATEND)        (0H)            ; end address of XDAT (size is 0 Bytes)
%DEFINE(REGBANK)        (0)             ; default register bank
%DEFINE(VIRT_STACK)     (0H)            ; define a virtual stack in xdat of
                                        ; 1Kbyte (Reentrant model only)

%DEFINE(PROTECT)        (NO)            ; default no protection is done on
                                        ; updating the virtual stack pointer
                                        ; (reentrant model only)

%DEFINE(FLOATMEM)       (0)             ; define floating point stack members to allocate
%DEFINE(MFLOAT)         (IDATA)         ; Floating point stack in XDATA (normal library 'float.lib' linked)
                                        ; When changed to 'IDATA', 'floats.lib' can be linked (See the manual).

;
; the following code is needed for initialization of static variables
;
;       MODEL can have values (SMALL), (AUX), (LARGE) or (REENTRANT)
;       it defines in which area the strings are located.
;
;       %DEFINE(MODEL)(LARGE)
%DEFINE(MODEL)(SMALL)

        EXTRN   CODE(%MODEL)            ; Linker will give error when memory
                                        ; models are mixed up
;
; Definition of some names used by XRAY
;
        DSEG AT 0
__simulated_output      DATA    $+99H   ; Used by XRAY, output to SBUF will
                                        ; be done to standard I/O device.
__simulated_input       DATA    $+99H   ; Used by XRAY, input from SBUF will
                                        ; be done from standard I/O device.

;
; Definition of names needed for initialization of static variables
;
        PUBLIC __START_ROM_XD
        PUBLIC __START_ROM_ID
        PUBLIC __START_ROM_PD
        PUBLIC __START_ROM_BA
        PUBLIC __START_ROM_BI
        PUBLIC __START_ROM_DA
        PUBLIC __START_ROM_ST
        PUBLIC __START_RAM_XD
        PUBLIC __START_RAM_ID
        PUBLIC __START_RAM_PD
        PUBLIC __START_RAM_BA
        PUBLIC __START_RAM_BI
        PUBLIC __START_RAM_DA
        PUBLIC __START_RAM_ST

CINIT_ROM_XD    SEGMENT CODE ROMDATA            ; define labels needed for
                                                ; initialization
        RSEG    CINIT_ROM_XD
__START_ROM_XD:
CINIT_ROM_ID    SEGMENT CODE ROMDATA
        RSEG    CINIT_ROM_ID
__START_ROM_ID:
CINIT_ROM_PD    SEGMENT CODE ROMDATA
        RSEG    CINIT_ROM_PD
__START_ROM_PD:
CINIT_ROM_BA    SEGMENT CODE ROMDATA
        RSEG    CINIT_ROM_BA
__START_ROM_BA:
CINIT_ROM_BI    SEGMENT CODE ROMDATA
        RSEG    CINIT_ROM_BI
__START_ROM_BI:
CINIT_ROM_DA    SEGMENT CODE ROMDATA
        RSEG    CINIT_ROM_DA
__START_ROM_DA:
CINIT_ROM_ST    SEGMENT CODE ROMDATA
        RSEG    CINIT_ROM_ST
__START_ROM_ST:

CINIT_RAM_XD    SEGMENT XDATA
        RSEG    CINIT_RAM_XD
__START_RAM_XD:
CINIT_RAM_ID    SEGMENT IDATA
        RSEG    CINIT_RAM_ID
__START_RAM_ID:
CINIT_RAM_PD    SEGMENT XDATA
        RSEG    CINIT_RAM_PD
__START_RAM_PD:
CINIT_RAM_BA    SEGMENT DATA    BITADDRESSABLE
        RSEG    CINIT_RAM_BA
__START_RAM_BA:
CINIT_RAM_BI    SEGMENT BIT
        RSEG    CINIT_RAM_BI
__START_RAM_BI:
CINIT_RAM_DA    SEGMENT DATA
        RSEG    CINIT_RAM_DA
__START_RAM_DA:
%IF(%EQS(%MODEL,SMALL)) THEN (
CINIT_RAM_ST    SEGMENT IDATA           ; This is the small memory model, so
                                        ; strings are allocated in idata
)FI
%IF(%EQS(%MODEL,AUX)) THEN (
CINIT_RAM_ST    SEGMENT XDATA           ; This is the auxiliary memory model,
                                        ; so strings are allocated in pdata
)FI
%IF(%EQS(%MODEL,LARGE)) THEN (
CINIT_RAM_ST    SEGMENT XDATA           ; This is the large memory model, so
                                        ; strings are allocated in xdata
)FI
%IF(%EQS(%MODEL,REENTRANT)) THEN (
CINIT_RAM_ST    SEGMENT XDATA           ; This is the reentrant memory model,
                                        ; so strings are allocated in xdata
)FI
        RSEG    CINIT_RAM_ST
__START_RAM_ST:

;
?STACK  SEGMENT IDATA
        RSEG    ?STACK
__STKSTLAB:
        DS      %STACKLENGTH            ; reserve space for the stack
__STKSTART      IDATA   __STKSTLAB - 1
;
;       No 'HEAP' will result in linker error when using a 'malloc' routine
;
%IF( %HEAP GT 00H ) THEN (
        PUBLIC  __HEAPSTART     ; start of heap area
        PUBLIC  __HEAPLENGTH    ; end of heap area

?HEAP   SEGMENT XDATA
        RSEG    ?HEAP
__HEAPSTART:
        DS      %HEAP           ; reserve space for the heap
__HEAPEND:
__HEAPLENGTH    XDATA   __HEAPEND - __HEAPSTART ; length of the heap
)FI

%IF(%EQS(%MODEL,REENTRANT)) THEN (
PUBLIC  __TOP_OF_VIRT_STACK
?VIRT_STACK     SEGMENT XDATA
        RSEG    ?VIRT_STACK
        DS      %VIRT_STACK     ; reserve space for the virtual stack in
                                ; external RAM
__TOP_OF_VIRT_STACK:
) FI

%IF( %FLOATMEM GT 00H ) THEN (
        PUBLIC __FLOATSTART
        PUBLIC __FLOATEND
FLOAT_STACK SEGMENT %MFLOAT
        RSEG FLOAT_STACK
__FLOATSTART:
        DS      4 * %FLOATMEM   ; reserve space for 'floating point stack'
__FLOATEND:
)FI

;
        CSEG    AT 0H
        LCALL   __START         ; Do NOT change to 'LJMP', because a reference
                                ; to the 'STARTUP' segment is needed. The return
                                ; address will be removed from stack, because
                                ; the stackpointer is initialized.
;
;       Call the user program
;
;       DS      2BH - 3H        ; allocate all non used interrupt vectors.

STARTUP SEGMENT CODE
        RSEG STARTUP
__START:
        ; First we have to do some initialisation. Disable the interrupts
        ; (because an 'exit' will jump to this point also) and clear the
        ; memory. Also make sure which register bank is the current used one.
        CLR     EA                      ; disable interrupts

        CLR     A
        MOV     PSW,    A               ; select register bank 0

        ; First initialize all IDATA to '0'
%IF( %RAMSIZE GT 08H ) THEN (
        MOV     R0,   #08               ; one position after registerbank 0
__CLR_IDAT:
        MOV     @R0,  A
        INC     R0
        CJNE    R0,   #%RAMSIZE, __CLR_IDAT     ; RAMSIZE must be 128 or 256
)FI

%IF( %XDATEND GT %XDATSTART ) THEN (
        MOV     DPTR,   #%XDATSTART             ; Initialise all XDATA to '0'
__CLR_XDAT:
        CLR     A
        MOVX    @DPTR,  A
        INC     DPTR
        MOV     A,      DPL
        CJNE    A,      #LOW(%XDATEND+1), __CLR_XDAT
        MOV     A,      DPH
        CJNE    A,      #HIGH(%XDATEND+1), __CLR_XDAT
)FI

        MOV     SP,     #LOW(__STKSTART)        ; initialise stack pointer
;       MOV     P2,     #00                     ; needed for models using pdat

; For reentrant model, initialise 'virtual stack' pointer
%IF( %NES(%PROTECT,YES) AND %EQS(%MODEL,REENTRANT) ) THEN (
        EXTRN   DATA( __SP,__BP )
        MOV     __SP,   #HIGH( __TOP_OF_VIRT_STACK )
        MOV     __SP+1, #LOW( __TOP_OF_VIRT_STACK )
        MOV     __BP,__SP
        MOV     __BP+1,__SP+1
) FI
%IF( %EQS(%PROTECT,YES) AND %EQS(%MODEL,REENTRANT) ) THEN (
        EXTRN   DATA( __SP,__BP )
        PUSH    0A8H                    ; save interrupt flag (IE)
        CLR     0AFH                    ; No interrupts allowed
        MOV     __SP,   #HIGH( __TOP_OF_VIRT_STACK )
        MOV     __SP+1, #LOW( __TOP_OF_VIRT_STACK )
        POP     0A8H                    ; restore interrupt flag
        MOV     __BP,__SP
        MOV     __BP+1,__SP+1
) FI

%IF( %FLOATMEM GT 00H AND %EQS(%MFLOAT,XDATA) ) THEN (
        EXTRN DATA( __FLOATSP )
        MOV     __FLOATSP,   #HIGH( __FLOATSTART ) ; initialise floating point stackpointer
        MOV     __FLOATSP+1, #LOW( __FLOATSTART )
) FI
%IF( %FLOATMEM GT 00H AND %EQS(%MFLOAT,IDATA) ) THEN (
        EXTRN DATA( __FLOATSP )
        MOV     __FLOATSP, #__FLOATSTART
) FI

        LCALL   __INITS                 ; initialise needed data segments

        MOV     PSW,    #(%REGBANK * 8) ; select correct registerbank

        USING   %REGBANK
        LJMP    _main           ; Jump to C-program (23 March 1993 LCALL changed in LJMP)
__EXIT:
__STOP:
        ; DB    0A5H            ; illegal instruction halts simulator
                                ; (but will loop some emulators)
        SJMP    __STOP

__INITS:                        ; all neccessary initialisation code will
                                ; automatically be linked here
                                ; Do NOT remove these lines and do NOT place
                                ; it somewhere else in the startup code
                                ; (must be the last lines in the startup code)
        EXTRN CODE( __EINITS )  ; Leave this external here

        END
