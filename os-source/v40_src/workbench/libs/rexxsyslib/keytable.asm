* == keytable.asm ======================================================
*
* Copyright (c) 1986-1989 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
*
* =========================     KeyTable     ===========================
* Checks whether the specified string is an instruction keyword.
* Registers:   A0 -- test string 
* Return:      D0 -- keyword code or 0
KTPMAX   EQU      9                    ; maximum primary length
KeyTable 
         movem.l  d2/d3/a3,-(sp)
         lea      ns_Hash(a0),a3       ; test string (hash byte)
         move.w   ns_Length(a0),d3     ; test string length

         moveq    #KTPMAX,d0           ; maximum string
         cmp.w    d0,d3                ; too long?
         bhi.s    4$                   ; yes ...

         ; Search the primary table ... length cohorts only.

         move.b   KTPCount(pc,d3.w),d0 ; keyword count
         moveq    #0,d2                ; clear index
         move.b   KTPIndex(pc,d3.w),d2 ; starting index
         bra.s    3$                   ; jump in

         ; Compare the keyword strings including the hash byte.

1$:      lea      KTPTable(pc,d2.w),a0 ; keyword string
         movea.l  a3,a1                ; test string
         move.w   d3,d1                ; characters to check

2$:      cmpm.b   (a0)+,(a1)+          ; matched?
         dbne     d1,2$                ; loop back
         beq.s    5$                   ; ... match found

         add.w    d3,d2                ; increment index
         addq.w   #1,d2                ; skip hash
3$:      dbf      d0,1$                ; loop back

         ; Keyword not found ... clear return.

4$:      moveq    #0,d0                ; clear return
         bra.s    6$                   ; failure ...

         ; Scale the offset for 4-byte entries and load the code.

5$:      move.b   KTPTotal(pc,d3.w),d3 ; load offset
         sub.w    d0,d3                ; back up
         lsl.w    #2,d3                ; scale for 4 bytes
         lea      (KTPCodes-4)(pc),a0  ; code table
         adda.w   d3,a0                ; index to code
         move.l   (a0),d0              ; load code

6$:      movem.l  (sp)+,d2/d3/a3
         rts

         ; Counts of primary keywords by keyword length.

KTPCount dc.b     0,0,2,4,9,7,3,4,0,3

         ; Cumulative counts by keyword length.

KTPTotal EQU      *-2                  ; prestart
         dc.b     2,6,15,22,25,29,29,32

         ; Offsets to keyword strings, indexed by name length.

KTPIndex EQU      *-2                  ; prestart
         dc.b     KTP2-KTPTable        ; 2
         dc.b     KTP3-KTPTable        ; 3
         dc.b     KTP4-KTPTable        ; 4
         dc.b     KTP5-KTPTable        ; 5
         dc.b     KTP6-KTPTable        ; 6
         dc.b     KTP7-KTPTable        ; 7
         dc.b     0                    ; 8 (none)
         dc.b     KTP9-KTPTable        ; 9

         ; REXX instruction keywords, grouped by length.  Each string is
         ; preceded by a hash byte.

KTPTable
KTP2     dc.b     147,'DO'
         dc.b     143,'IF'

KTP3     dc.b     218,'ARG'
         dc.b     215,'END'
         dc.b     237,'NOP'
         dc.b     237,'SAY'

KTP4     dc.b     28,'CALL'
         dc.b     53,'DROP'
         dc.b     31,'ECHO'
         dc.b     41,'ELSE'
         dc.b     58,'EXIT'
         dc.b     61,'PULL'
         dc.b     64,'PUSH'
         dc.b     47,'THEN'
         dc.b     50,'WHEN'

KTP5     dc.b     101,'BREAK'
         dc.b     109,'LEAVE'
         dc.b     123,'PARSE'
         dc.b     133,'QUEUE'
         dc.b     120,'SHELL'
         dc.b     111,'TRACE'
         dc.b     140,'UPPER'

KTP6     dc.b     224,'RETURN'
         dc.b     192,'SELECT'
         dc.b     190,'SIGNAL'

KTP7     dc.b     6,'ADDRESS'
         dc.b     14,'ITERATE'
         dc.b     19,'NUMERIC'
         dc.b     44,'OPTIONS'

KTP9     dc.b     189,'INTERPRET'
         dc.b     186,'OTHERWISE'
         dc.b     169,'PROCEDURE'
         CNOP     0,2

         ; The primary opcodes (full word length)

KTPCodes dc.l     KTF_DO
         dc.l     KTF_IF

         dc.l     KTF_ARG
         dc.l     KTF_END
         dc.l     KTF_NOP
         dc.l     KTF_SAY

         dc.l     KTF_CALL
         dc.l     KTF_DROP
         dc.l     KTF_SAY              ; ('ECHO' ... synonym for SAY)
         dc.l     KTF_ELSE
         dc.l     KTF_EXIT
         dc.l     KTF_PULL
         dc.l     KTF_PUSH
         dc.l     KTF_THEN
         dc.l     KTF_WHEN

         dc.l     KTF_BREAK
         dc.l     KTF_LEAVE
         dc.l     KTF_PARSE
         dc.l     KTF_QUEUE
         dc.l     KTF_ADDR             ; ('SHELL' ... synonym for ADDRESS)
         dc.l     KTF_TRACE
         dc.l     KTF_UPPER

         dc.l     KTF_RETRN
         dc.l     KTF_SELCT
         dc.l     KTF_SIGNL

         dc.l     KTF_ADDR             ; 'ADDRESS'
         dc.l     KTF_ITER             ; 'ITERATE'
         dc.l     KTF_NUMER            ; 'NUMERIC'
         dc.l     KTF_OPT              ; 'OPTIONS'

         dc.l     KTF_INTRP            ; 'INTERPRET'
         dc.l     KTF_OTHRW            ; 'OTHERWISE'
         dc.l     KTF_PROC             ; 'PROCEDURE'

         ; Secondary index by instruction opcode.

KTSOps   dc.b     1                    ; ADDRESS
         dc.b     0                    ; BREAK
         dc.b     0                    ; CALL
         dc.b     2                    ; DO
         dc.b     0                    ; DROP
         dc.b     0                    ; ELSE
         dc.b     0                    ; END
         dc.b     0                    ; ERROR
         dc.b     3                    ; IF
         dc.b     0                    ; INTERPRET
         dc.b     0                    ; ITERATE
         dc.b     0                    ; LEAVE
         dc.b     0
         dc.b     4                    ; NUMERIC
         dc.b     0                    ; OTHER
         dc.b     5                    ; PARSE
         dc.b     6                    ; PROCEDURE
         dc.b     0                    ; PUSH
         dc.b     0                    ; QUEUE
         dc.b     0                    ; RETURN
         dc.b     0                    ; SAY
         dc.b     0                    ; SELECT
         dc.b     7                    ; SIGNAL
         dc.b     0                    ; THEN
         dc.b     8                    ; TRACE
         dc.b     0                    ; UNTIL
         dc.b     0                    ; UPPER
         dc.b     9                    ; WHEN
         dc.b     10                   ; OPTIONS
         CNOP     0,2

* ===========================     SecCode     ==========================
* Searches for secondary keywords for the specified primary opcode.
* Registers:   A0 -- string structure
*              D0 -- primary instruction code
* Return:      D0 -- code or 0
KTSMAX   EQU      11                   ; maximum secondary length
SecCode
         movem.l  d2/d3/a2/a3,-(sp)
         lea      ns_Hash(a0),a3       ; start of test string
         move.w   ns_Length(a0),d3     ; test string length

         ; Use the primary opcode to load the secondary index ...

         moveq    #KTF_CODEMASK>>24,d1 ; opcode mask
         rol.l    #8,d0                ; rotate code
         and.b    d0,d1                ; extract index
         move.b   KTSOps(pc,d1.w),d1   ; load index
         beq.s    3$                   ; ... no sub-keywords

         moveq    #KTSMAX,d0           ; maximum length
         cmp.w    d0,d3                ; too long?
         bhi.s    3$                   ; yes ...

         move.b   KTSCount(pc,d1.w),d0 ; loop count
         moveq    #0,d2                ; clear index
         move.b   KTSIndex(pc,d1.w),d2 ; load index
         lea      KTSTable(pc),a2      ; secondary table

1$:      movea.l  a2,a0                ; start of table
         adda.w   0(a2,d2.w),a0        ; add offset
         movea.l  a3,a1                ; test string
         move.l   d3,d1                ; test length

2$:      cmpm.b   (a0)+,(a1)+          ; matched?
         dbne     d1,2$                ; loop back
         beq.s    4$                   ; ... match found

         addq.w   #2,d2                ; next index
         dbf      d0,1$                ; loop back

3$:      moveq    #0,d0                ; not found ...
         bra.s    5$

         ; Match found ... load the secondary keyword code.

4$:      move.w   (KTSCodes-KTSTable)(a2,d2.w),d0

5$:      movem.l  (sp)+,d2/d3/a2/a3
         rts

         ; Counts of the number of secondary strings to check.

KTSCount EQU      *-1
         dc.b     NUM_KT1-1            ; 'ADDRESS'
         dc.b     NUM_KT4-1            ; 'DO'
         dc.b     NUM_KT7-1            ; 'IF'
         dc.b     NUM_KT12-1           ; 'NUMERIC'
         dc.b     NUM_KT13-1           ; 'PARSE'
         dc.b     NUM_KT14-1           ; 'PROCEDURE'
         dc.b     NUM_KT21-1           ; 'SIGNAL'
         dc.b     NUM_KT22-1           ; 'TRACE'
         dc.b     NUM_KT25-1           ; 'WHEN'
         dc.b     NUM_KT28-1           ; 'OPTIONS'

         ; The starting indices for the search strings.

KTSIndex EQU      *-1
         dc.b     KTS1-KTSTable        ; 'ADDRESS'
         dc.b     KTS4-KTSTable        ; 'DO'
         dc.b     KTS7-KTSTable        ; 'IF'
         dc.b     KTS12-KTSTable       ; 'NUMERIC'
         dc.b     KTS13-KTSTable       ; 'PARSE'
         dc.b     KTS14-KTSTable       ; 'PROCEDURE'
         dc.b     KTS21-KTSTable       ; 'SIGNAL'
         dc.b     KTS22-KTSTable       ; 'TRACE'
         dc.b     KTS25-KTSTable       ; 'WHEN'
         dc.b     KTS28-KTSTable       ; 'OPTIONS'

         ; Offsets to the keyword strings.

KTSTable
KTS1     dc.w     KTD_VALUE-KTSTable   ; secondary for 'ADDRESS'

KTS4     dc.w     KTD_TO-KTSTable      ; secondary for 'DO'
         dc.w     KTD_BY-KTSTable
         dc.w     KTD_FOR-KTSTable
         dc.w     KTD_WHILE-KTSTable
         dc.w     KTD_UNTIL-KTSTable
         dc.w     KTD_FOREVER-KTSTable

KTS7     dc.w     KTD_THEN-KTSTable    ; secondary for 'IF'

KTS12    dc.w     KTD_FUZZ-KTSTable    ; secondary for 'NUMERIC'
         dc.w     KTD_FORM-KTSTable
         dc.w     KTD_DIGITS-KTSTable
         dc.w     KTD_SCIENTFIC-KTSTable
         dc.w     KTD_ENGINRING-KTSTable

KTS13    dc.w     KTD_ARG-KTSTable     ; secondary for 'PARSE'
         dc.w     KTD_VAR-KTSTable
         dc.w     KTD_PULL-KTSTable
         dc.w     KTD_WITH-KTSTable
         dc.w     KTD_UPPER-KTSTable
         dc.w     KTD_VALUE-KTSTable
         dc.w     KTD_SOURCE-KTSTable
         dc.w     KTD_NUMERIC-KTSTable
         dc.w     KTD_VERSION-KTSTable
         dc.w     KTD_EXTERNAL-KTSTable

KTS14    dc.w     KTD_EXPOSE-KTSTable  ; secondary for 'PROCEDURE'
       
KTS21    dc.w     KTD_ON-KTSTable      ; secondary for 'SIGNAL'
         dc.w     KTD_OFF-KTSTable
         dc.w     KTD_HALT-KTSTable
         dc.w     KTD_ERROR-KTSTable
         dc.w     KTD_IOERR-KTSTable
         dc.w     KTD_VALUE-KTSTable
         dc.w     KTD_SYNTAX-KTSTable
         dc.w     KTD_BREAK_C-KTSTable
         dc.w     KTD_BREAK_D-KTSTable
         dc.w     KTD_BREAK_E-KTSTable
         dc.w     KTD_BREAK_F-KTSTable
         dc.w     KTD_FAILURE-KTSTable
         dc.w     KTD_NOVALUE-KTSTable

KTS22    dc.w     KTD_VALUE-KTSTable   ; secondary for 'TRACE'

KTS25    dc.w     KTD_THEN-KTSTable    ; secondary for 'WHEN'

KTS28    dc.w     KTD_NO-KTSTable      ; secondary for 'OPTIONS'
         dc.w     KTD_CACHE-KTSTable
         dc.w     KTD_FAILAT-KTSTable
         dc.w     KTD_PROMPT-KTSTable
         dc.w     KTD_RESULTS-KTSTable
         dc.w     KTD_RESIDENT-KTSTable

         ; Secondary keyword opcodes (half-word length).  These must be
         ; in the same order as the string offsets above.

KTSCodes dc.w     KT1_VALUE            ; secondary for 'ADDRESS'

         dc.w     KT4_TO               ; secondary for 'DO'
         dc.w     KT4_BY
         dc.w     KT4_FOR
         dc.w     KT4_WHILE
         dc.w     KT4_UNTIL
         dc.w     KT4_FOREVER

         dc.w     KT7_THEN             ; secondary for 'IF'

         dc.w     KT12_FUZZ            ; secondary for 'NUMERIC'
         dc.w     KT12_FORM
         dc.w     KT12_DIGITS
         dc.w     KT12_SCI
         dc.w     KT12_ENG

         dc.w     KT13_ARG             ; secondary for 'PARSE'
         dc.w     KT13_VAR
         dc.w     KT13_PULL
         dc.w     KT13_WITH
         dc.w     KT13_UPPER
         dc.w     KT13_VALUE
         dc.w     KT13_SOURCE
         dc.w     KT13_NUMER
         dc.w     KT13_VERS
         dc.w     KT13_EXTERN
    
         dc.w     KT14_EXPOSE          ; secondary for 'PROCEDURE'

         dc.w     KT21_ON              ; secondary for 'SIGNAL'
         dc.w     KT21_OFF
         dc.w     KT21_HALT
         dc.w     KT21_ERROR
         dc.w     KT21_IOERR
         dc.w     KT21_VALUE
         dc.w     KT21_SYNTAX
         dc.w     KT21_BREAK_C
         dc.w     KT21_BREAK_D
         dc.w     KT21_BREAK_E
         dc.w     KT21_BREAK_F
         dc.w     KT21_FAILURE
         dc.w     KT21_NOVALUE
       
         dc.w     KT22_VALUE           ; secondary for 'TRACE'

         dc.w     KT25_THEN            ; secondary for 'WHEN'

         dc.w     KT28_NO              ; secondary for 'OPTIONS'
         dc.w     KT28_CACHE
         dc.w     KT28_FAILAT
         dc.w     KT28_PROMPT
         dc.w     KT28_RESULTS
         dc.w     KT28_RESIDENT

         ; Array of error label strings.  These are the SIGNAL condition
         ; keywords.

         CNOP     0,4                  ; force longword alignment
ErrLabel

         ; Control 'C' break ... load error if not set.

gnBREAK_C dc.l    ERR10_002            ; 'HALT' error
         dc.w     7
         dc.b     KEEPSTR!(1<<LOADNOT)
         dc.b     7,'BREAK_C',0

gnBREAK_D dc.l    0                    ; clear if not set
         dc.w     7
         dc.b     KEEPSTR!(1<<LOADNOT)
         dc.b     8,'BREAK_D',0

gnBREAK_E dc.l    0                    ; clear if not set
         dc.w     7
         dc.b     KEEPSTR!(1<<LOADNOT)
         dc.b     9,'BREAK_E',0

gnBREAK_F dc.l    0                    ; clear if not set
         dc.w     7
         dc.b     KEEPSTR!(1<<LOADNOT)
         dc.b     10,'BREAK_F',0

         ; 'ERROR' condition ... set 'RC' and clear if set.

gnERROR  dc.l     0                    ; clear if set
         dc.w     5
         dc.b     KEEPSTR!(1<<LOADSET)!(1<<SETRC)
         dc.b     138,'ERROR',0,0,0

         ; 'HALT' condition ... load error if not set.

gnHALT   dc.l     ERR10_002            ; 'HALT' error
         dc.w     4
         dc.b     KEEPSTR!(1<<LOADNOT)
         dc.b     41,'HALT',0,0,0,0

         ; 'IOERR' condition ... set 'RC' and clear error.

gnIOERR  dc.l     0                    ; clear error
         dc.w     5
         dc.b     KEEPSTR!(1<<LOADNOT)!(1<<LOADSET)!(1<<SETRC)
         dc.b     129,'IOERR',0,0,0

         ; 'NOVALUE' condition ... clear error if not set

gnNOVALUE dc.l    0                    ; clear if not set
         dc.w     7
         dc.b     KEEPSTR!(1<<LOADNOT)
         dc.b     26,'NOVALUE',0

         ; 'SYNTAX' condition ... set 'RC' and clear if set.

gnSYNTAX dc.l     0                    ; clear if set
         dc.w     6
         dc.b     KEEPSTR!(1<<LOADSET)!(1<<SETRC)
         dc.b     231,'SYNTAX',0,0

         ; 'FAILURE' condition ... set 'RC' and clear if set.

gnFAILURE dc.l    0                    ; clear if set
         dc.w     7
         dc.b     KEEPSTR!(1<<LOADSET)!(1<<SETRC)
         dc.b     8,'FAILURE',0

         ; Secondary keyword strings

KTD_BY   dc.b     155,'BY'
KTD_ON   dc.b     157,'ON'
KTD_NO   dc.b     157,'NO'
KTD_TO   dc.b     163,'TO'

KTD_ARG  dc.b     218,'ARG'
KTD_FOR  dc.b     231,'FOR'
KTD_OFF  dc.b     219,'OFF'
KTD_VAR  dc.b     233,'VAR'

KTD_FORM dc.b     52,'FORM'
KTD_FUZZ dc.b     79,'FUZZ'
KTD_HALT EQU      gnHALT+ns_Hash
KTD_PULL dc.b     61,'PULL'
KTD_THEN dc.b     47,'THEN'
KTD_WITH dc.b     60,'WITH'

KTD_CACHE      dc.b  84,'CACHE'
KTD_ERROR      EQU   gnERROR+ns_Hash
KTD_IOERR      EQU   gnIOERR+ns_Hash
KTD_UNTIL      dc.b  140,'UNTIL'
KTD_UPPER      dc.b  140,'UPPER'
KTD_VALUE      dc.b  125,'VALUE'
KTD_WHILE      dc.b  121,'WHILE'

KTD_DIGITS     dc.b  196,'DIGITS'
KTD_EXPOSE     dc.b  212,'EXPOSE'
KTD_FAILAT     dc.b  177,'FAILAT'
KTD_PROMPT     dc.b  226,'PROMPT'
KTD_SOURCE     dc.b  209,'SOURCE'
KTD_SYNTAX     EQU   gnSYNTAX+ns_Hash

KTD_BREAK_C    EQU   gnBREAK_C+ns_Hash
KTD_BREAK_D    EQU   gnBREAK_D+ns_Hash
KTD_BREAK_E    EQU   gnBREAK_E+ns_Hash
KTD_BREAK_F    EQU   gnBREAK_F+ns_Hash
KTD_FAILURE    EQU   gnFAILURE+ns_Hash
KTD_FOREVER    dc.b  25,'FOREVER'
KTD_NOVALUE    EQU   gnNOVALUE+ns_Hash
KTD_NUMERIC    dc.b  19,'NUMERIC'
KTD_RESULTS    dc.b  50,'RESULTS'
KTD_VERSION    dc.b  38,'VERSION'

KTD_EXTERNAL   dc.b  99,'EXTERNAL'
KTD_RESIDENT   dc.b  94,'RESIDENT'

KTD_SCIENTFIC  dc.b  225,'SCIENTIFIC'

KTD_ENGINRING  dc.b  43,'ENGINEERING'
         CNOP     0,2
