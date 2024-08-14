* == ctable.asm ========================================================
*
* Copyright (c) 1986-1989 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
* Static data tables used by the REXX interpreter.

CF0      SET      CTF_SPACE
CF1      SET      CTF_DIGIT
CF2      SET      CTF_ALPHA
CF3      SET      CTF_REXXSYM
CF4      SET      CTF_REXXOPR
CF5      SET      CTF_REXXSPC
CF6      SET      CTF_UPPER
CF7      SET      CTF_LOWER

         ; Character attribute table for the REXX interpreter.  Sets the
         ; attribute flags associated with each character (range 0-255).

CTable   dc.b     CF0                  ; $00 (null)
         dc.b     CF0                  ; $01
         dc.b     CF0                  ; $02
         dc.b     CF0                  ; $03
         dc.b     CF0                  ; $04
         dc.b     CF0                  ; $05
         dc.b     CF0                  ; $06
         dc.b     CF0                  ; $07
         dc.b     CF0                  ; $08
         dc.b     CF0                  ; $09 (horizontal tab)
         dc.b     CF0                  ; $0A (line feed)
         dc.b     CF0                  ; $0B (vertical tab)
         dc.b     CF0                  ; $0C (form feed)
         dc.b     CF0                  ; $0D (carriage return)
         dc.b     CF0                  ; $0E
         dc.b     CF0                  ; $0F

         dc.b     CF0                  ; $10
         dc.b     CF0                  ; $11
         dc.b     CF0                  ; $12
         dc.b     CF0                  ; $13
         dc.b     CF0                  ; $14
         dc.b     CF0                  ; $15
         dc.b     CF0                  ; $16
         dc.b     CF0                  ; $17
         dc.b     CF0                  ; $18
         dc.b     CF0                  ; $19
         dc.b     CF0                  ; $1A
         dc.b     CF0                  ; $1B
         dc.b     CF0                  ; $1C
         dc.b     CF0                  ; $1D
         dc.b     CF0                  ; $1E
         dc.b     CF0                  ; $1F

         dc.b     CF0                  ; $20 (space)
         dc.b     CF3                  ; $21 !
         dc.b     0                    ; $22 "
         dc.b     CF3                  ; $23 #
         dc.b     CF3                  ; $24 $
         dc.b     CF4                  ; $25 %
         dc.b     CF4                  ; $26 &
         dc.b     0                    ; $27 '
         dc.b     CF5                  ; $28 (
         dc.b     CF5                  ; $29 )
         dc.b     CF4                  ; $2A *
         dc.b     CF4                  ; $2B +
         dc.b     CF5                  ; $2C ,
         dc.b     CF4                  ; $2D -
         dc.b     CF3                  ; $2E .
         dc.b     CF4                  ; $2F /

         dc.b     CF1!CF3              ; $30 0
         dc.b     CF1!CF3              ; $31 1
         dc.b     CF1!CF3              ; $32 2
         dc.b     CF1!CF3              ; $33 3
         dc.b     CF1!CF3              ; $34 4
         dc.b     CF1!CF3              ; $35 5
         dc.b     CF1!CF3              ; $36 6
         dc.b     CF1!CF3              ; $37 7
         dc.b     CF1!CF3              ; $38 8
         dc.b     CF1!CF3              ; $39 9
         dc.b     CF5                  ; $3A :
         dc.b     CF5                  ; $3B ;
         dc.b     CF4                  ; $3C <
         dc.b     CF4                  ; $3D =
         dc.b     CF4                  ; $3E >
         dc.b     CF3                  ; $3F ?

         dc.b     CF3                  ; $40 @
         dc.b     CF2!CF3!CF6          ; $41 A
         dc.b     CF2!CF3!CF6          ; $42 B
         dc.b     CF2!CF3!CF6          ; $43 C
         dc.b     CF2!CF3!CF6          ; $44 D
         dc.b     CF2!CF3!CF6          ; $45 E
         dc.b     CF2!CF3!CF6          ; $46 F
         dc.b     CF2!CF3!CF6          ; $47 G
         dc.b     CF2!CF3!CF6          ; $48 H
         dc.b     CF2!CF3!CF6          ; $49 I
         dc.b     CF2!CF3!CF6          ; $4A J
         dc.b     CF2!CF3!CF6          ; $4B K
         dc.b     CF2!CF3!CF6          ; $4C L
         dc.b     CF2!CF3!CF6          ; $4D M
         dc.b     CF2!CF3!CF6          ; $4E N
         dc.b     CF2!CF3!CF6          ; $4F O

         dc.b     CF2!CF3!CF6          ; $50 P
         dc.b     CF2!CF3!CF6          ; $51 Q
         dc.b     CF2!CF3!CF6          ; $52 R
         dc.b     CF2!CF3!CF6          ; $53 S
         dc.b     CF2!CF3!CF6          ; $54 T
         dc.b     CF2!CF3!CF6          ; $55 U
         dc.b     CF2!CF3!CF6          ; $56 V
         dc.b     CF2!CF3!CF6          ; $57 W
         dc.b     CF2!CF3!CF6          ; $58 Z
         dc.b     CF2!CF3!CF6          ; $59 Y
         dc.b     CF2!CF3!CF6          ; $5A Z
         dc.b     0                    ; $5B [
         dc.b     0                    ; $5C \
         dc.b     0                    ; $5D ]
         dc.b     CF4                  ; $5E ^  (XOR operator)
         dc.b     CF3                  ; $5F _

         dc.b     0                    ; $60
         dc.b     CF2!CF3!CF7          ; $61 a
         dc.b     CF2!CF3!CF7          ; $62 b
         dc.b     CF2!CF3!CF7          ; $63 e
         dc.b     CF2!CF3!CF7          ; $64 d
         dc.b     CF2!CF3!CF7          ; $65 e
         dc.b     CF2!CF3!CF7          ; $66 f
         dc.b     CF2!CF3!CF7          ; $67 g
         dc.b     CF2!CF3!CF7          ; $68 h
         dc.b     CF2!CF3!CF7          ; $69 i
         dc.b     CF2!CF3!CF7          ; $6A j
         dc.b     CF2!CF3!CF7          ; $6B k
         dc.b     CF2!CF3!CF7          ; $6C l
         dc.b     CF2!CF3!CF7          ; $6D m
         dc.b     CF2!CF3!CF7          ; $6E n
         dc.b     CF2!CF3!CF7          ; $6F o

         dc.b     CF2!CF3!CF7          ; $70 p
         dc.b     CF2!CF3!CF7          ; $71 q
         dc.b     CF2!CF3!CF7          ; $72 r
         dc.b     CF2!CF3!CF7          ; $73 s
         dc.b     CF2!CF3!CF7          ; $74 t
         dc.b     CF2!CF3!CF7          ; $75 u
         dc.b     CF2!CF3!CF7          ; $76 v
         dc.b     CF2!CF3!CF7          ; $77 w
         dc.b     CF2!CF3!CF7          ; $78 x
         dc.b     CF2!CF3!CF7          ; $79 y
         dc.b     CF2!CF3!CF7          ; $7A z
         dc.b     0                    ; $7B {
         dc.b     CF4                  ; $7C |
         dc.b     0                    ; $7D }
         dc.b     CF4                  ; $7E ~ (NOT operator)
         dc.b     CF0                  ; $7F

         ; Extended ASCII characters ($80-$FF)

         dc.b     CF0                  ; $80
         dc.b     CF0                  ; $81
         dc.b     CF0                  ; $82
         dc.b     CF0                  ; $83
         dc.b     CF0                  ; $84
         dc.b     CF0                  ; $85
         dc.b     CF0                  ; $86
         dc.b     CF0                  ; $87
         dc.b     CF0                  ; $88
         dc.b     CF0                  ; $89
         dc.b     CF0                  ; $8A
         dc.b     CF0                  ; $8B
         dc.b     CF0                  ; $8C
         dc.b     CF0                  ; $8D
         dc.b     CF0                  ; $8E
         dc.b     CF0                  ; $8F

         dc.b     CF0                  ; $90
         dc.b     CF0                  ; $91
         dc.b     CF0                  ; $92
         dc.b     CF0                  ; $93
         dc.b     CF0                  ; $94
         dc.b     CF0                  ; $95
         dc.b     CF0                  ; $96
         dc.b     CF0                  ; $97
         dc.b     CF0                  ; $98
         dc.b     CF0                  ; $99
         dc.b     CF0                  ; $9A
         dc.b     CF0                  ; $9B
         dc.b     CF0                  ; $9C
         dc.b     CF0                  ; $9D
         dc.b     CF0                  ; $9E
         dc.b     CF0                  ; $9F

         dc.b     CF0                  ; $A0 (non-breaking space)
         dc.b     0                    ; $A1
         dc.b     0                    ; $A2
         dc.b     0                    ; $A3
         dc.b     0                    ; $A4
         dc.b     0                    ; $A5
         dc.b     0                    ; $A6
         dc.b     0                    ; $A7
         dc.b     0                    ; $A8
         dc.b     0                    ; $A9
         dc.b     0                    ; $AA
         dc.b     0                    ; $AB
         dc.b     0                    ; $AC
         dc.b     0                    ; $AD
         dc.b     0                    ; $AE
         dc.b     0                    ; $AF

         dc.b     0                    ; $B0
         dc.b     0                    ; $B1
         dc.b     0                    ; $B2
         dc.b     0                    ; $B3
         dc.b     0                    ; $B4
         dc.b     0                    ; $B5
         dc.b     0                    ; $B6
         dc.b     0                    ; $B7
         dc.b     0                    ; $B8
         dc.b     0                    ; $B9
         dc.b     0                    ; $BA
         dc.b     0                    ; $BB
         dc.b     0                    ; $BC
         dc.b     0                    ; $BD
         dc.b     0                    ; $BE
         dc.b     0                    ; $BF

         dc.b     CF2!CF6              ; $C0
         dc.b     CF2!CF6              ; $C1
         dc.b     CF2!CF6              ; $C2
         dc.b     CF2!CF6              ; $C3
         dc.b     CF2!CF6              ; $C4
         dc.b     CF2!CF6              ; $C5
         dc.b     CF2!CF6              ; $C6
         dc.b     CF2!CF6              ; $C7
         dc.b     CF2!CF6              ; $CC
         dc.b     CF2!CF6              ; $C9
         dc.b     CF2!CF6              ; $CA
         dc.b     CF2!CF6              ; $CB
         dc.b     CF2!CF6              ; $CC
         dc.b     CF2!CF6              ; $CD
         dc.b     CF2!CF6              ; $CE
         dc.b     CF2!CF6              ; $CF

         dc.b     CF2!CF6              ; $D0
         dc.b     CF2!CF6              ; $D1
         dc.b     CF2!CF6              ; $D2
         dc.b     CF2!CF6              ; $D3
         dc.b     CF2!CF6              ; $D4
         dc.b     CF2!CF6              ; $D5
         dc.b     CF2!CF6              ; $D6
         dc.b     0                    ; $D7 (unassigned)
         dc.b     CF2!CF6              ; $D8
         dc.b     CF2!CF6              ; $DD
         dc.b     CF2!CF6              ; $DA
         dc.b     CF2!CF6              ; $DB
         dc.b     CF2!CF6              ; $DC
         dc.b     CF2!CF6              ; $DD
         dc.b     CF2!CF6              ; $DE
         dc.b     CF2!CF6              ; $DF

         dc.b     CF2!CF7              ; $E0
         dc.b     CF2!CF7              ; $E1
         dc.b     CF2!CF7              ; $E2
         dc.b     CF2!CF7              ; $E3
         dc.b     CF2!CF7              ; $E4
         dc.b     CF2!CF7              ; $E5
         dc.b     CF2!CF7              ; $E6
         dc.b     CF2!CF7              ; $E7
         dc.b     CF2!CF7              ; $E8
         dc.b     CF2!CF7              ; $E9
         dc.b     CF2!CF7              ; $EA
         dc.b     CF2!CF7              ; $EB
         dc.b     CF2!CF7              ; $EC
         dc.b     CF2!CF7              ; $ED
         dc.b     CF2!CF7              ; $EE
         dc.b     CF2!CF7              ; $EF

         dc.b     CF2!CF7              ; $F0
         dc.b     CF2!CF7              ; $F1
         dc.b     CF2!CF7              ; $F2
         dc.b     CF2!CF7              ; $F3
         dc.b     CF2!CF7              ; $F4
         dc.b     CF2!CF7              ; $F5
         dc.b     CF2!CF7              ; $F6
         dc.b     0                    ; $F7 (unassigned)
         dc.b     CF2!CF7              ; $F8
         dc.b     CF2!CF7              ; $F9
         dc.b     CF2!CF7              ; $FA
         dc.b     CF2!CF7              ; $FB
         dc.b     CF2!CF7              ; $FC
         dc.b     CF2!CF7              ; $FD
         dc.b     CF2!CF7              ; $FE
         dc.b     CF2!CF7              ; $FF

         ; Static string structure definitions.  Strings are limited to
         ; seven characters (plus a null) in the structure definition.

         CNOP     0,4
NUMSTAT  EQU      8                    ; number of static strings
gnNULL   dc.l     0
         dc.w     0
         dc.b     KEEPSTR,0,0,0,0,0,0,0,0,0
gnFALSE  dc.l     0
         dc.w     1
         dc.b     KEEPNUM,48,'0',0,0,0,0,0,0,0
gnTRUE   dc.l     1
         dc.w     1
         dc.b     KEEPNUM,49,'1',0,0,0,0,0,0,0
gnREXX   dc.l     0
         dc.w     4
         dc.b     KEEPSTR,71,'REXX',0,0,0,0
gnCOMMAND dc.l    0
         dc.w     7
         dc.b     KEEPSTR,255,'COMMAND',0
gnSTDIN  dc.l     0
         dc.w     5
         dc.b     KEEPSTR,130,'STDIN',0,0,0
gnSTDOUT dc.l     0
         dc.w     6
         dc.b     KEEPSTR,227,'STDOUT',0,0
gnSTDERR dc.l     0
         dc.w     6
         dc.b     KEEPSTR,212,'STDERR',0,0

         ; Static strings for the special variable names

gnRESULT dc.l     0
         dc.w     6
         dc.b     KEEPSTR,223,'RESULT',0,0

gnRC     dc.l     0
         dc.w     2
         dc.b     KEEPSTR,149,'RC',0,0,0,0,0,0

gnSIGL   dc.l     0
         dc.w     4
         dc.b     KEEPSTR,47,'SIGL',0,0,0,0

         ; Static token array for the special variables

gtRESULT dc.l     0                    ; 'RESULT' symbol token
         dc.l     0
         dc.b     T_SYMVAR
         dc.b     0
         dc.w     0
         dc.l     gnRESULT

gtRC     dc.l     0                    ; 'RC' symbol token
         dc.l     0
         dc.b     T_SYMVAR
         dc.b     0
         dc.w     0
         dc.l     gnRC

gtSIGL   dc.l     0                    ; 'SIGL' symbol token
         dc.l     0
         dc.b     T_SYMVAR
         dc.b     0
         dc.w     0
         dc.l     gnSIGL

         ; Miscellaneous constant data

CALLCode dc.l     KC_CALL              ; 'CALL' instruction
DOCode   dc.l     KC_DO                ; 'DO' instruction
PRIMask  dc.l     KTF_PRIMASK          ; instruction mask
EVALMask dc.l     IPF_EVAL!ISF_EVAL    ; EVAL flags
MaxLen   dc.l     MAXLEN               ; maximum string length
