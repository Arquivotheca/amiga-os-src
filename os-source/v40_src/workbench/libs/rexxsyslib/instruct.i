* === rexx/instruct.i ==================================================
*
* Copyright (C) 1986-1989 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
* Definitions for REXX instruction codes

         IFND  INSTRUCT_I
INSTRUCT_I SET 1

         IFND  REXXENV_I
         INCLUDE  "rexxenv.i"
         ENDC

* Instruction Coding
* All instruction words are 32 bits long and are composed of several
* sub-fields, some fixed and some dependent on the sub-keyword options.
* Bits 0-8:   the secondary action codes, defined by the keywords given
*             with the instruction (or implicit for the instruction).
* Bits 9-15:  the secondary flag bits, which are determined by the keywords
*             present.
* Bits 16-31: the primary action flags.  Bits 24-29 of this field are the
*             actual instruction opcode.

* Secondary action flag bits (bits 9-15)
ISB_SKIP1ST    EQU   9                 ; skip first token?
ISB_NOBLANK    EQU   10                ; remove all blanks?
ISB_SYMLIST    EQU   11                ; process list of symbols?
ISB_EVAL       EQU   12                ; evaluate expression?
ISB_FIRST      EQU   13                ; must be first?
ISB_NOEVAL     EQU   14                ; suppress evaluation?
ISB_NOSCAN     EQU   15                ; suppress scanning?

* Primary action flag bits (bits 16-31)
IPB_EVAL       EQU   16                ; evaluate expression
IPB_SELECT     EQU   17                ; valid in SELECT?
IPB_EMPTY      EQU   18                ; empty clause?
IPB_MULTEVAL   EQU   19                ; scan through expressions?
IPB_FIRSTSYM   EQU   20                ; symbol/string after keyword?
IPB_REQDKEYW   EQU   21                ; must be followed by keywords?
IPB_SCANSUPP   EQU   22                ; suppress if in SCAN mode
IPB_SKIPSUPP   EQU   23                ; suppress if in SKIP mode

* Instruction opcode is in bits 24-29
IPB_PAUSE      EQU   30                ; pause if interactive debug?
IPB_CLEAR      EQU   31                ; clears 'skip'? (macro-inst)

* The flag form of the action flags ...
ISF_SKIP1ST    EQU   1<<ISB_SKIP1ST
ISF_NOBLANK    EQU   1<<ISB_NOBLANK
ISF_SYMLIST    EQU   1<<ISB_SYMLIST
ISF_EVAL       EQU   1<<ISB_EVAL
ISF_FIRST      EQU   1<<ISB_FIRST
ISF_NOEVAL     EQU   1<<ISB_NOEVAL
ISF_NOSCAN     EQU   1<<ISB_NOSCAN

IPF_EVAL       EQU   1<<IPB_EVAL
IPF_SELECT     EQU   1<<IPB_SELECT
IPF_EMPTY      EQU   1<<IPB_EMPTY
IPF_MULTEVAL   EQU   1<<IPB_MULTEVAL
IPF_FIRSTSYM   EQU   1<<IPB_FIRSTSYM
IPF_REQDKEYW   EQU   1<<IPB_REQDKEYW
IPF_SCANSUPP   EQU   1<<IPB_SCANSUPP
IPF_SKIPSUPP   EQU   1<<IPB_SKIPSUPP

IPF_PAUSE      EQU   1<<IPB_PAUSE
IPF_CLEAR      EQU   1<<IPB_CLEAR

         ; Keywords for 'ADDRESS' instruction

NUM_KT1        EQU   1
KT1_VALUE      EQU   ($0001!ISF_FIRST!ISF_NOSCAN)

         ; Keywords for the 'DO' instruction

NUM_KT4        EQU   6
KT4_TO         EQU   ($0001)
KT4_BY         EQU   ($0002)
KT4_FOR        EQU   ($0004)
KT4_FOREVER    EQU   ($0088)
KT4_WHILE      EQU   ($0110!ISF_NOSCAN)
KT4_UNTIL      EQU   ($0120!ISF_NOSCAN)
KT4_INIT       EQU   ($00C0)           ; implicit (no keyword)
KT4_OPTMASK    EQU   ($007F)           ; option mask
KT4_EXPRMASK   EQU   ($0017)           ; expressions required

         ; Keywords for 'IF' instruction

NUM_KT7        EQU   1
KT7_THEN       EQU   ($0001!ISF_NOBLANK)

         ; Keywords for 'NUMERIC' instruction

NUM_KT12       EQU   5
KT12_FORM      EQU   ($0010!ISF_FIRST)
KT12_SCI       EQU   ($0001!ISF_NOSCAN)
KT12_ENG       EQU   ($0002!ISF_NOSCAN)
KT12_DIGITS    EQU   ($0014!ISF_FIRST!ISF_EVAL!ISF_NOSCAN)
KT12_FUZZ      EQU   ($0018!ISF_FIRST!ISF_EVAL!ISF_NOSCAN)
KT12_OPTMASK   EQU   ($000F)

         ; Keywords for 'PARSE' instruction

NUM_KT13       EQU   10
KT13_ARG       EQU   ($0001!ISF_NOEVAL!ISF_NOBLANK)
KT13_EXTERN    EQU   ($0002!ISF_NOEVAL!ISF_NOBLANK)
KT13_NUMER     EQU   ($0003!ISF_NOEVAL!ISF_NOBLANK)
KT13_PULL      EQU   ($0004!ISF_NOEVAL!ISF_NOBLANK)
KT13_SOURCE    EQU   ($0005!ISF_NOEVAL!ISF_NOBLANK)
KT13_VAR       EQU   ($0006!ISF_NOEVAL!ISF_NOBLANK)
KT13_VALUE     EQU   ($0007!ISF_EVAL)
KT13_VERS      EQU   ($0008!ISF_NOEVAL!ISF_NOBLANK)
KT13_WITH      EQU   ($0010!ISF_NOBLANK)
KT13_UPPER     EQU   ($0020!ISF_FIRST)
KT13_OPTMASK   EQU   ($000F)
KT13B_UPPER    EQU   5                 ; bit for 'UPPER'

         ; Keywords for 'PROCEDURE' instruction

NUM_KT14       EQU   1
KT14_EXPOSE    EQU   ($0002!ISF_SYMLIST!ISF_FIRST)

NUM_KT16       EQU   0                 ; 'RETURN' instruction
KT16_EXIT      EQU   ($0001)           ; (implicit)

         ; Secondary keyords for 'SIGNAL' instruction.

NUM_KT21       EQU   13                ; 'SIGNAL' instruction
KT21_ON        EQU   ($0180!ISF_FIRST!ISF_NOEVAL)
KT21_OFF       EQU   ($0100!ISF_FIRST!ISF_NOEVAL)
KT21_VALUE     EQU   ($0100!ISF_FIRST!ISF_NOSCAN)
KT21_BREAK_C   EQU   ($0040!INB_BREAK_C!ISF_NOSCAN)
KT21_BREAK_D   EQU   ($0040!INB_BREAK_D!ISF_NOSCAN)
KT21_BREAK_E   EQU   ($0040!INB_BREAK_E!ISF_NOSCAN)
KT21_BREAK_F   EQU   ($0040!INB_BREAK_F!ISF_NOSCAN)
KT21_ERROR     EQU   ($0040!INB_ERROR!ISF_NOSCAN)
KT21_HALT      EQU   ($0040!INB_HALT!ISF_NOSCAN)
KT21_IOERR     EQU   ($0040!INB_IOERR!ISF_NOSCAN)
KT21_NOVALUE   EQU   ($0040!INB_NOVALUE!ISF_NOSCAN)
KT21_SYNTAX    EQU   ($0040!INB_SYNTAX!ISF_NOSCAN)
KT21_FAILURE   EQU   ($0040!INB_FAILURE!ISF_NOSCAN)
KT21_OPTMASK   EQU   ($007F)
KT21_BITMASK   EQU   ($000F)           ; signal bit
KT21B_ON       EQU   7                 ; 'ON' bit

         ; Keywords for 'TRACE' instruction

NUM_KT22       EQU   1
KT22_VALUE     EQU   ($0001!ISF_FIRST!ISF_NOSCAN)

         ; Keywords for 'WHEN' instruction

NUM_KT25       EQU   1
KT25_THEN      EQU   ($0001!ISF_NOBLANK)

         ; Keywords for 'OPTIONS' instruction

NUM_KT28       EQU   6
KT28_NO        EQU   ($0040!ISF_FIRST)
KT28_CACHE     EQU   ($0001)
KT28_FAILAT    EQU   ($0042!ISF_EVAL!ISF_NOSCAN)
KT28_PROMPT    EQU   ($0044!ISF_EVAL!ISF_NOSCAN)
KT28_RESULTS   EQU   ($0008)
KT28_RESIDENT  EQU   ($0010)
KT28_OPTMASK   EQU   ($001F)
KT28B_NO       EQU   6                 ; 'NO' bit

         ; Primary instruction codes ...

KC_ADDR        EQU   ($00000000!IPF_CLEAR!IPF_PAUSE)
KC_BREAK       EQU   ($01000000!IPF_CLEAR!IPF_PAUSE)
KC_CALL        EQU   ($02000000!IPF_CLEAR)
KC_DO          EQU   ($03000000!IPF_CLEAR)
KC_DROP        EQU   ($04000000!IPF_CLEAR!IPF_PAUSE)
KC_ELSE        EQU   ($05000000!IPF_CLEAR)
KC_END         EQU   ($06000000!IPF_CLEAR)
KC_ERROR       EQU   ($07000000)
KC_IF          EQU   ($08000000!IPF_CLEAR!IPF_PAUSE)
KC_INTRP       EQU   ($09000000!IPF_CLEAR!IPF_PAUSE)
KC_ITER        EQU   ($0A000000!IPF_CLEAR!IPF_PAUSE)
KC_LEAVE       EQU   ($0B000000!IPF_CLEAR!IPF_PAUSE)
KC_NOP         EQU   ($0C000000!IPF_CLEAR!IPF_PAUSE)
KC_NUMER       EQU   ($0D000000!IPF_CLEAR!IPF_PAUSE)
KC_OTHRW       EQU   ($0E000000!IPF_CLEAR)
KC_PARSE       EQU   ($0F000000!IPF_CLEAR!IPF_PAUSE)
KC_PROC        EQU   ($10000000!IPF_CLEAR!IPF_PAUSE)
KC_PUSH        EQU   ($11000000!IPF_CLEAR!IPF_PAUSE)
KC_QUEUE       EQU   ($12000000!IPF_CLEAR!IPF_PAUSE)
KC_RETRN       EQU   ($13000000!IPF_CLEAR)
KC_SAY         EQU   ($14000000!IPF_CLEAR!IPF_PAUSE)
KC_SELCT       EQU   ($15000000!IPF_CLEAR!IPF_PAUSE)
KC_SIGNL       EQU   ($16000000!IPF_CLEAR)
KC_THEN        EQU   ($17000000!IPF_CLEAR)
KC_TRACE       EQU   ($18000000!IPF_CLEAR!IPF_PAUSE)
KC_UNTIL       EQU   ($19000000!IPF_CLEAR)
KC_UPPER       EQU   ($1A000000!IPF_CLEAR!IPF_PAUSE)
KC_WHEN        EQU   ($1B000000!IPF_CLEAR!IPF_PAUSE)
KC_OPT         EQU   ($1C000000!IPF_CLEAR!IPF_PAUSE)

KC_ASSGN       EQU   ($20000000!IPF_CLEAR!IPF_PAUSE)
KC_COMM        EQU   ($21000000!IPF_CLEAR!IPF_PAUSE)

         ; Micro-instructions ... (these don't clear the skip flag)

KC_FUNC        EQU   ($23000000)
KC_LABEL       EQU   ($24000000!IPF_PAUSE)
KC_PASS        EQU   ($25000000)

* Abbreviations for commonly-used combinations ...
IPF_S1         EQU   (IPF_SKIPSUPP)
IPF_S2         EQU   (IPF_SKIPSUPP!IPF_SCANSUPP)
IPF_EVALONLY   EQU   (IPF_EVAL!ISF_NOSCAN)
IPF_MULTIPLE   EQU   (IPF_EVAL!IPF_MULTEVAL)

         ; Primary Instruction Codes ...

KTF_ADDR       EQU   (KC_ADDR!IPF_S2!IPF_FIRSTSYM!ISF_EVAL)
KTF_ARG        EQU   (KC_PARSE!IPF_S1!KT13_ARG!KT13_UPPER)
KTF_BREAK      EQU   (KC_BREAK!IPF_S1!IPF_EMPTY!ISF_NOSCAN)
KTF_CALL       EQU   (KC_CALL!IPF_S2!IPF_FIRSTSYM!IPF_EMPTY!ISF_EVAL)
KTF_DO         EQU   (KC_DO!IPF_MULTIPLE)
KTF_DROP       EQU   (KC_DROP!IPF_S1!ISF_SYMLIST)
KTF_ELSE       EQU   (KC_ELSE!ISF_NOSCAN)
KTF_END        EQU   (KC_END!IPF_SELECT!ISF_SYMLIST)
KTF_ERROR      EQU   (KC_ERROR)
KTF_EXIT       EQU   (KC_RETRN!IPF_S2!IPF_EVALONLY!IPF_EMPTY!KT16_EXIT)
KTF_IF         EQU   (KC_IF!IPF_MULTIPLE)
KTF_INTRP      EQU   (KC_INTRP!IPF_S2!IPF_EVALONLY!IPF_EMPTY)
KTF_ITER       EQU   (KC_ITER!IPF_S1!ISF_SYMLIST)
KTF_LEAVE      EQU   (KC_LEAVE!IPF_S1!ISF_SYMLIST)
KTF_NOP        EQU   (KC_NOP!IPF_S2!IPF_EMPTY!ISF_NOSCAN)
KTF_NUMER      EQU   (KC_NUMER!IPF_S2!IPF_REQDKEYW!IPF_EMPTY)
KTF_OTHRW      EQU   (KC_OTHRW!IPF_SELECT!ISF_NOSCAN)
KTF_PARSE      EQU   (KC_PARSE!IPF_S1!IPF_REQDKEYW!IPF_MULTEVAL)
KTF_PROC       EQU   (KC_PROC!IPF_S1)
KTF_PULL       EQU   (KC_PARSE!IPF_S1!KT13_PULL!KT13_UPPER)
KTF_PUSH       EQU   (KC_PUSH!IPF_S2!IPF_EVALONLY!IPF_EMPTY)
KTF_QUEUE      EQU   (KC_QUEUE!IPF_S2!IPF_EVALONLY!IPF_EMPTY)
KTF_RETRN      EQU   (KC_RETRN!IPF_S2!IPF_EVALONLY!IPF_EMPTY)
KTF_SAY        EQU   (KC_SAY!IPF_S2!IPF_EVALONLY!IPF_EMPTY)
KTF_SELCT      EQU   (KC_SELCT!IPF_EMPTY!ISF_NOSCAN)
KTF_SIGNL      EQU   (KC_SIGNL!IPF_S1!IPF_EVAL!IPF_FIRSTSYM)
KTF_THEN       EQU   (KC_THEN!IPF_SELECT!ISF_NOSCAN)
KTF_TRACE      EQU   (KC_TRACE!IPF_S1!IPF_EVAL!IPF_FIRSTSYM)
KTF_UNTIL      EQU   (KC_UNTIL!IPF_S2!IPF_EVALONLY!IPF_EMPTY)
KTF_UPPER      EQU   (KC_UPPER!IPF_S1!ISF_SYMLIST)
KTF_WHEN       EQU   (KC_WHEN!IPF_SELECT!IPF_MULTIPLE)
KTF_OPT        EQU   (KC_OPT!IPF_S2!IPF_EMPTY)

         ; Instruction codes for other statements

KTF_ASSGN      EQU   (KC_ASSGN!IPF_S2!IPF_EVALONLY!IPF_EMPTY)
KTF_COMM       EQU   (KC_COMM!IPF_S2!IPF_EVALONLY!IPF_EMPTY)
KTF_LABEL      EQU   (KC_LABEL!IPF_S2!ISF_NOSCAN)
KTF_NULL       EQU   (KC_PASS!IPF_S2!ISF_NOSCAN)
KTF_FUNC       EQU   (KC_FUNC!IPF_S2!IPF_MULTIPLE!ISF_SKIP1ST!ISF_NOSCAN)

KTF_PRIMASK    EQU   $FF000000      ; mask for primary keyword
KTF_CODEMASK   EQU   $3F000000      ; mask for instruction code
KTF_SECMASK    EQU   $000001FF      ; mask for secondary identifier

         ENDC
