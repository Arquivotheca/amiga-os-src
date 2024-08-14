$CASE
$INCLUDE (defs.asm)
;***************************************************************************
;
; Project:      Commodore
; File:         sonydriv.asm
; Date:         30 March 1993
; Status:       Draft
;
; Description:  CXD2500 decoder drivers and DSIC2 driver functions
;               this module contains the following drivers:
;               void cxd2500_wr(byte cxd2500_char)
;               void wr_dsic2(byte dsic2_char)
;               byte rd_dsic2(void)
;               bit sledge_switch(void)
;               void reset_dsic2_cd6(void)
;               bit cd6_read_subcode(void)
;               bit simulate_subcode_ready(void)
;               void stop_tray(void)
;               void pull_tray(void)
;               void push_tray(void)
;               bit tray_switch(void)
;
; Decisions:    -
;
; HISTORY               AUTHOR COMMENTS
; 05 March 1993         creation H.v.K.
;
;***************************************************************************
NAME    driver_module

PUBLIC  _cxd2500_wr, _cd6_read_subcode, _simulate_subcode_ready, _led_0_on, _led_0_off
PUBLIC  _wr_dsic2, _rd_dsic2, _sledge_switch, _reset_dsic2_cd6
PUBLIC  _wr_dsic2_BYTE, _cxd2500_wr_BYTE, _n1_speed, _mute_pin, _audio_cxd2500_BYTE
PUBLIC   _hf_present, _door_closed, _audio_cxd2500, _led_1_on, _led_1_off

EXTRN   CODE(_delay)
EXTRN   DATA(_delay_BYTE, _servo_timer, _Q_buffer)
EXTRN   IDATA(_audio_cntrl, _peak_level_low, _peak_level_high)

EXTRN   BIT    (_scor_edge)

SILD                    BIT     P1.1    ;load line to DSIC2
SICL                    BIT     P1.3    ;clock line to DSIC2
SIDA                    BIT     P1.4    ;data line to DSIC2
RST                     BIT     P1.5    ;reset line to DSIC2
QDA                     BIT     P2.6    ;subcode data line to CXD2500
QCL                     BIT     P2.5    ;subcode clock line to CXD2500
UDAT                    BIT     P2.3    ;user register data line to CXD2500
UCL                     BIT     P2.1    ;user register clock line to CXD2500
ULAT                    BIT     P2.2    ;user register latch line to CXD2500
HF_PRESENT              BIT     P3.3    ;hf detector input
_n1_speed               BIT     P1.7
_mute_pin               BIT     P2.0

door_switch             BIT     P2.4


SLEDGE_SWITCH           BIT     P1.2


driver          SEGMENT         CODE
driver_data     SEGMENT         DATA

RSEG            driver_data
_wr_dsic2_BYTE:
_cxd2500_wr_BYTE:
_audio_cxd2500_BYTE:    DS      1

RSEG            driver

;--------------------------------------------------------------------------
; Function:     cxd2500_wr
; Input:        _CXD2500_WR_BYTE: char to be sent to CXD2500
; Output        -
; Abstract:     Write char to CXD2500
;
; Decisions:    NO check on _CXD2500_WR_BYTE is done
;--------------------------------------------------------------------------
_cxd2500_wr:
        mov     R0,#3           ; give first 3 pulses with data=0
        clr     UDAT
pulse3:
        clr     UCL             ; ucl=0
        setb    UCL             ; ucl=1
        djnz    R0,pulse3
;
        mov     A,_cxd2500_wr_BYTE
        mov     R0,#8           ; setup loop counter on 8 bits
wr_loop:
        clr     UCL             ; ucl=0
        rrc     A               ; CF contains next data bit
        mov     UDAT,C          ; bit on port
        setb    UCL             ; ucl=1
        djnz    R0,wr_loop
        clr     ULAT            ; give latch pulse
        setb    ULAT
        setb    UDAT            ; data line high by default
        ret



;--------------------------------------------------------------------------
; Function:     audio_cxd2500
; Input:        _AUDIO_CXD2500_BYTE: char to be sent to CXD2500
; Output        -
; Abstract:     Write char to CXD2500 audio register
;
; Decisions:    NO check on _CXD2500_WR_BYTE is done
;--------------------------------------------------------------------------
_audio_cxd2500:
        mov     R0,#3           ; give first 3 pulses with data=0
        clr     UDAT
pulse_3:
        clr     UCL             ; ucl=0
        setb    UCL             ; ucl=1
        djnz    R0,pulse_3
;
        mov     A,_audio_cxd2500_BYTE ; variup varidwn mute att pct1 pct2 0 0
        rrc     A               ;
        rrc     A               ; 0 0 variup varidwn mute att pct1 pct2
        mov     R0,#6           ; setup loop counter on 6 bits
wr_loop_ad:
        clr     UCL             ; ucl=0
        rrc     A               ; CF contains next data bit
        mov     UDAT,C          ; bit on port
        setb    UCL             ; ucl=1
        djnz    R0,wr_loop_ad

; now the data byte

        mov     A,#0AH
        mov     R0,#4           ; setup loop counter on 4 bits
wr_loop_da:
        clr     UCL             ; ucl=0
        rrc     A               ; CF contains next data bit
        mov     UDAT,C          ; bit on port
        setb    UCL             ; ucl=1
        djnz    R0,wr_loop_da

        clr     ULAT            ; give latch pulse
        setb    ULAT
        setb    UDAT            ; data line high by default
        ret


;--------------------------------------------------------------------------
; Function:     wr_dsic2
; Input:        _WR_DSIC2_BYTE
; Output        -
; Abstract:     write _WR_DSIC2_BYTE to DSIC2
;
; Decisions:    -
;--------------------------------------------------------------------------
_wr_dsic2:
        mov     A,_wr_dsic2_BYTE
        mov     R0,#8
next_bit:
        clr     SICL            ; sicl=0
        rlc     A
        mov     SIDA,C          ; data=(A).C
        setb    SICL            ; sicl=1
        djnz    R0,next_bit
; all 8 bits to DSCI2
        setb    SIDA
        clr     SILD
        nop
        nop
        nop
        setb    SILD
; command loaded, wait 150 usec
        mov     R0,#75                  ;dec + jnz = 2 usec at Fc = 12MHz
        djnz    R0,$
        ret


;--------------------------------------------------------------------------
; Function:     rd_dsic2
; Input:
; Output        A
; Abstract:     read  DSIC2 byte
;
; Decisions:    -
;--------------------------------------------------------------------------
_rd_dsic2:
        setb    SIDA                    ;data line input
        clr     SICL                    ;sicl=0
        mov     R0,#7                   ;last bit different
        clr     SILD                    ;sild=0
recbit:
        clr     SICL                    ;sicl=0
        mov     C,SIDA                  ;data => CF
        rlc     A                       ;CF => (ACC).7 ..(ACC).1
        setb    SICL                    ;sicl=1
        djnz    R0,recbit
; 7 bits received
        mov     C,SIDA                  ;data => CF
        rlc     A                       ;CF => (ACC).0
        setb    SILD                    ;sild=1
; byte read, wait 150 usec
        mov     R0,#75                  ;dec + jnz = 2 usec at Fc = 12MHz
        djnz    R0,$
        ret


;--------------------------------------------------------------------------
; Function:     sledge_switch
; Input:
; Output        A
; Abstract:     read sledge switch: CLOSED: CY=0; OPEN CY=1
;
; Decisions:    -
;--------------------------------------------------------------------------
_sledge_switch:
        mov     C,SLEDGE_SWITCH
        ret


;--------------------------------------------------------------------------
; Function:    reset_dsic2_cd6
; Input:       -
; Output       -
; Abstract:    make reset pulse
;
; Decisions:    -
;--------------------------------------------------------------------------
_reset_dsic2_cd6:
        clr     RST                     ; reset dsic2_cd6: 80 ms
        mov     _delay_BYTE,#160
        lcall   _delay
        setb    RST                     ; release dsic2_cd6: 20 ms
        mov     _delay_BYTE,#40
        lcall   _delay
        clr     RST                     ; reset 500 Us
        mov     _delay_BYTE,#1
        lcall   _delay
        setb    RST                     ; release dsic2_cd6
        mov     _delay_BYTE,#20         ; wait 10 ms for RESET line to get STEADY
        lcall   _delay

        setb    SILD                    ; default settings CXD2500 and DSIC2 lines
        setb    SIDA
        setb    SICL
        setb    QDA
        setb    QCL
        setb    UDAT
        setb    UCL
        setb    ULAT
        setb    IT0                     ; set SCOR interrupt falling edge sensitive
        clr     _scor_edge              ; clear flag falling edge detected
        ret


;--------------------------------------------------------------------------
; Function:    cd6_read_subcode
; Input:       -
; Return:      CY = 0 -> no new subcode read, CY = 1 -> new subcode available in Q_buffer[10]
; Output       Q_buffer[10]
; Abstract:    this function reads the subcode from the CXD2500 if available
; Decisions:    -
;--------------------------------------------------------------------------
_cd6_read_subcode:
        jbc     _scor_edge,scor_detected        ;jump if SCOR edge detected
subcode_invalid:
        clr     C                       ;report no new subcode read
        ret
scor_detected:
        jnb     QDA,subcode_invalid
subcode_available:
        mov     R0,#_peak_level_low
        mov     @R0,#00
        mov     R0,#_peak_level_high
        mov     @R0,#00                 ;reset peak level information

        mov     R2,#10                  ;read 10 bytes, no peak value
        mov     R1,#8                   ;a byte has 8 bits
        mov     R0,#_Q_buffer           ;point to start in Q_buffer
next_cd6_subc_bit:
        clr     QCL
        setb    QCL                     ;data on data-line available
        mov     C,QDA                   ;read bit
        rrc     A                       ;add bit with previous read bits
        djnz    R1,next_cd6_subc_bit
; R1=0 -> store byte
        mov     @R0,A
        inc     R0                      ;point to next byte of Q_buffer
        mov     R1,#8                   ;reset bit counter to 8
        djnz    R2,next_cd6_subc_bit
; R2=0 -> all bytes read
        mov     R0,#_audio_cntrl
        mov     A,@R0                   ;fetch control information
        anl     A,#0FCH
        jz      subcode_ready
; A <> 0
        mov     R1,#8                   ;a byte has 8 bits
        mov     R0,#_peak_level_low     ;peak level low
next_level_peak_bit:
        clr     QCL
        setb    QCL                     ;data on data-line available
        mov     C,QDA                   ;read bit
        rrc     A                       ;add bit with previous read bits
        djnz    R1,next_level_peak_bit
; R1=0 -> store byte
        mov     @R0,A

        mov     R1,#8                   ;a byte has 8 bits
        mov     R0,#_peak_level_high    ;peak level high
next_level_peak_bit_1:
        clr     QCL
        setb    QCL                     ;data on data-line available
        mov     C,QDA                   ;read bit
        rrc     A                       ;add bit with previous read bits
        djnz    R1,next_level_peak_bit_1
; R1=0 -> store byte
        mov     @R0,A

subcode_ready:
        setb    C                       ;report new subcode read
        ret


;--------------------------------------------------------------------------
; Function:    simulate_subcode_ready
; Input:       -
; Return:      CY = 0 -> subcode ready, CY = 1 -> subcode invalid
; Output
; Abstract:    this function simulates the subcode ready status of the CD6
; Decisions:    -
;--------------------------------------------------------------------------
_simulate_subcode_ready:
        setb    C
        jnb     _scor_edge,sim_ex
        clr     _scor_edge
        mov     C,QDA
        cpl     C
sim_ex:
        ret


;--------------------------------------------------------------------------
; Function:   led_0_on
; Input:       -
; Output       -
; Abstract:   led_0 active
; Decisions:   -
;--------------------------------------------------------------------------
_led_0_on:
        clr     P1.0
        ret

;--------------------------------------------------------------------------
; Function:   led_0_off
; Input:       -
; Output       -
; Abstract:    led_0 not active 
; Decisions:   -
;--------------------------------------------------------------------------
_led_0_off:
        setb    P1.0
        ret

;--------------------------------------------------------------------------
; Function:   led_1_on
; Input:       -
; Output       -
; Abstract:   led_1 gives light
; Decisions:   -
;--------------------------------------------------------------------------
_led_1_on:
        clr     P0.0
        ret

;--------------------------------------------------------------------------
; Function:   led_1_off
; Input:       -
; Output       -
; Abstract:   led_1 does not give light
; Decisions:   -
;--------------------------------------------------------------------------
_led_1_off:
        setb    P0.0
        ret

;--------------------------------------------------------------------------
; Function:   _hf_present
; Input:       -
; Output      1: hf present 0: hf not present
; Abstract:   check hf detector
; Decisions:   -
;--------------------------------------------------------------------------

_hf_present:
        mov     C,HF_PRESENT            ;measure HF
        jnc     hf_present
        mov     b,#5
measure_again:
        mov     _delay_BYTE,#10
        lcall   _delay
        mov     C,HF_PRESENT            ;measure HF
        jnc     hf_present
        djnz    B,measure_again

no_hf_present:  
        clr     c
        ret

hf_present:
        setb    C
        ret

_door_closed:
        mov     C,door_switch
        ret

        END
