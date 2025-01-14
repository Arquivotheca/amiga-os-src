* == oper.i ============================================================
*
* Copyright (c) 1986, 1989 by William S. Hawes  All Rights Reserved.
*
* ======================================================================
* REXX operator code definitions

         IFND  REXX_OPER_I
REXX_OPER_I SET   1

OPERMASK EQU      $E0                  ; mask for priority
OPERSEQ  EQU      $1F                  ; mask for sequence
OPERSHFT EQU      5

* Prefix operators
OPLEVEL7 EQU      7<<OPERSHFT
OP_NOT   EQU      OPLEVEL7!27          ; prefix '~'
OP_SAME  EQU      OPLEVEL7!26          ; prefix '+'
OP_NEG   EQU      OPLEVEL7!25          ; prefix '-'

* Dyadic operators
OPLEVEL6 EQU      6<<OPERSHFT
OP_EXPN  EQU      OPLEVEL6!24          ; exponentiation '**'

OPLEVEL5 EQU      5<<OPERSHFT
OP_DIV3  EQU      OPLEVEL5!23          ; remainder '/'
OP_DIV2  EQU      OPLEVEL5!22          ; integer division'%'
OP_DIV1  EQU      OPLEVEL5!21          ; division '//'
OP_MULT  EQU      OPLEVEL5!20          ; multiplication '*'

OPLEVEL4 EQU      4<<OPERSHFT
OP_ADD   EQU      OPLEVEL4!19          ; addition '+'
OP_SUB   EQU      OPLEVEL4!18          ; subtraction '-'

OPLEVEL3 EQU      3<<OPERSHFT
OP_CCBL  EQU      OPLEVEL3!17          ; concatenation ' ' (with blank)
OP_CC    EQU      OPLEVEL3!16          ; concatenation '||' (or abuttal)

OPLEVEL2 EQU      2<<OPERSHFT
OP_EXEQ  EQU      OPLEVEL2!11          ; exactly equal '=='
OP_EXNE  EQU      OPLEVEL2!10          ; not exactly equal '~=='
OP_EQ    EQU      OPLEVEL2!9           ; equal '='
OP_NE    EQU      OPLEVEL2!8           ; not equal '~='
OP_GT    EQU      OPLEVEL2!7           ; greater than '>'
OP_GE    EQU      OPLEVEL2!6           ; greater or equal '>=', '~<'
OP_LT    EQU      OPLEVEL2!5           ; less than '<'
OP_LE    EQU      OPLEVEL2!4           ; less than or equal '>=', '~>'

OPLEVEL1 EQU      1<<OPERSHFT
OP_AND   EQU      OPLEVEL1!3           ; AND '&'

OPLEVEL0 EQU      0<<OPERSHFT
OP_OR    EQU      OPLEVEL0!2           ; OR '|'
OP_XOR   EQU      OPLEVEL0!1           ; exclusive OR '&&'

         ENDC
