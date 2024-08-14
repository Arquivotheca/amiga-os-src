   IFND  JANUS_JANUSREG_I
JANUS_JANUSREG_I  SET   1

;****************************************************************************
; (Amiga side file)
;
; janusreg.i -- janus hardware registers (from amiga point of view)
;
; Copyright (c) 1986, Commodore Amiga Inc., All rights reserved.
;
; 7-15-88 - Bill Koester - Modified for self inclusion of required files
;10-23-89 - Bill Koester - Added DISKTO[PC|AMIGA] bits for floppy switch
;****************************************************************************

   IFND  EXEC_TYPES_I
   INCLUDE  "exec/types.i"
   ENDC

; hardware interrupt bits (all bits are active low)

   BITDEF   JINT,MINT,0       ; mono video ram written to
   BITDEF   JINT,GINT,1       ; color video ram written to
   BITDEF   JINT,CRT1INT,2    ; mono video control registers changed
   BITDEF   JINT,CRT2INT,3    ; color video control registers changed
   BITDEF   JINT,ENBKB,4      ; keyboard ready for next character
   BITDEF   JINT,LPT1INT,5    ; parallel control register
   BITDEF   JINT,COM2INT,6    ; serial control register
   BITDEF   JINT,SYSINT,7     ; software int request


; These are the definitions for the io registers.  All the registers are byte
; wide and the address are for Byte Access addresses

jio_1000KeyboardData    EQU   $061f ; data that keyboard will read in a1000
jio_2000KeyboardData    EQU   $1fff ; data that keyboard will read in a2000

jio_SystemStatus        EQU   $003f ; pc only register
jio_NmiEnable           EQU   $005f ; pc only register

jio_Com2XmitData        EQU   $007d
jio_Com2ReceiveData     EQU   $009d
jio_Com2IntEnableW      EQU   $00bd
jio_Com2IntEnableR      EQU   $00dd
jio_Com2DivisorLSB      EQU   $007f
jio_Com2DivisorMSB      EQU   $009f
jio_Com2IntID           EQU   $00ff
jio_Com2LineCntrl       EQU   $011f
jio_Com2ModemCntrl      EQU   $013f
jio_Com2LineStatus      EQU   $015f
jio_Com2ModemStatus     EQU   $017f

jio_Lpt1Data            EQU   $019f ; data byte
jio_Lpt1Status          EQU   $01bf ; see equates below
jio_Lpt1Control         EQU   $01df ; see equates below

jio_MonoAddressInd      EQU   $01ff ; current index into crt data regs
jio_MonoData            EQU   $02a1 ; every other byte for 16 registers
jio_MonoControlReg      EQU   $02ff

jio_ColorAddressInd     EQU   $021f ; current index into crt data regs
jio_ColorData           EQU   $02c1 ; every other byte for 16 registers
jio_ColorControlReg     EQU   $023f
jio_ColorSelectReg      EQU   $025f
jio_ColorStatusReg      EQU   $027f

jio_DisplaySystemReg    EQU   $029f

jio_IntReq              EQU   $1ff1 ; read clears, pc -> amiga ints
jio_PcIntReq            EQU   $1ff3 ; r/o, amiga -> pc ints
jio_ReleasePcReset      EQU   $1ff5 ; r/o, strobe release pc's reset
jio_PCconfiguration     EQU   $1ff7 ; r/w, give/set PC config.
jio_IntEna              EQU   $1ff9 ; r/w, enables pc int lines
jio_PcIntGen            EQU   $1ffb ; w/o, bit == 0 -> cause pc int
jio_Control             EQU   $1ffd ; w/o, random control lines

;now the magic bits in each register (and boy, are there a lot of them!)

; bits for Lpt1Status register

   BITDEF   JPCLS,STROBE,0
   BITDEF   JPCLS,AUTOFEED,1
   BITDEF   JPCLS,INIT,2
   BITDEF   JPCLS,SELECTIN,3
   BITDEF   JPCLS,IRQENABLE,4    ; active 1

; bits for Lpt1Control register

   BITDEF   JPCLC,ERROR,3
   BITDEF   JPCLC,SELECT,4
   BITDEF   JPCLC,NOPAPER,5
   BITDEF   JPCLC,ACK,6
   BITDEF   JPCLC,BUSY,7

; bits for PcIntReq, PcIntGen registers

   BITDEF   JPCINT,IRQ1,0        ; active high
   BITDEF   JPCINT,IRQ3,1        ; active low
   BITDEF   JPCINT,IRQ4,2        ; active low
   BITDEF   JPCINT,IRQ7,3        ; active low

; pc side interrupts

JPCKEYINT   EQU   $ff            ; keycode available
JPCSENDINT  EQU   $fc            ; system request
JPCLPT1INT  EQU   $f6            ; printer acknowledge


; bits for RamSize

   BITDEF   JRAM,EXISTS,0        ; unset if there is any ram at all
   BITDEF   JRAM,2MEG,1          ; set if 2 meg, clear if 1/2 meg

; bits for control register

   BITDEF   JCNTRL,ENABLEINT,0   ; enable amiga interrupts
   BITDEF   JCNTRL,DISABLEINT,1  ; disable amiga interrupts
   BITDEF   JCNTRL,RESETPC,2     ; reset the pc. remember to strobe
                                 ;   ReleasePcReset afterwards  
   BITDEF   JCNTRL,CLRPCINT,3    ; turn off all amiga->pc ints (except
                                 ;   keyboard
   BITDEF   JCNTRL,CLRBUSY,4     ; interfaces parallel busy bit
   BITDEF   JCNTRL,DISKTOAMIGA,5 ; Switch disk to Amiga for floppy switch
                                 ; Active low
   BITDEF   JCNTRL,DISKTOPC,6    ; Switch disk to PC for floppy switch
                                 ; Active low



   ENDC  ;End of JANUS_JANUSREG_I conditional assembly

