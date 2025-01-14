* == biflist.asm =======================================================
*
* Copyright (c) 1986-1989 by William S. Hawes.  All Rights Reserved.
*
* ======================================================================
*
* ===========================     BIFList     ==========================
* Checks whether a name string is a REXX Built-In function.  If so, the 
* jump offset and argument data are returned.  Private to 'BIFunc'.
* Return:   D4 -- function offset or 0
*           D5 -- argument byte
*           D6 -- conversion byte
* Scratch:  D2-D6/A2-A3
BIFMAXLN SET      10                   ; maximum name length
BIFList
         movea.l  TSTRING(a5),a0       ; function name
         lea      ns_Hash(a0),a3       ; test string (including hash)
         move.w   ns_Length(a0),d3     ; test string length

         moveq    #BIFMAXLN,d0         ; maximum length
         cmp.w    d0,d3                ; too long?
         bhi.s    4$                   ; yes

         move.b   BLCount(pc,d3.w),d0  ; search count
         move.w   d3,d1                ; name length
         add.w    d1,d1                ; scale for 2 bytes
         move.w   BLIndex(pc,d1.w),d2  ; starting index
         bra.s    3$                   ; jump in

1$:      lea      BLTable(pc,d2.w),a0  ; function name
         movea.l  a3,a1                ; test string
         move.w   d3,d1                ; length (loop count + 1)

2$:      cmpm.b   (a0)+,(a1)+          ; strings match?
         dbne     d1,2$                ; loop back
         beq.s    5$                   ; function found ...

         add.w    d3,d2                ; advance by length
         addq.w   #3,d2                ; skip hash and tag bytes
3$:      dbf      d0,1$                ; loop back

         ; Function not found ... clear offset and return.

4$:      moveq    #0,d4                ; function not found
         rts

         ; Function name found ... load argument data and offset

5$:      move.b   (a0)+,d5             ; argument byte
         move.b   (a0)+,d6             ; conversion byte
         move.b   BLTotal(pc,d3.w),d3  ; total function count
         sub.w    d0,d3                ; back up
         add.w    d3,d3                ; scale for 2 bytes
         move.w   (BLTable-2)(pc,d3.w),d4
         rts

         ; Counts of function names indexed by length.

BLCount  dc.b     0,0,0,14,13,13,27,13,3,4,2

         ; Cumulative counts of function names.

BLTotal  dc.b     0,0,0,14,27,40,67,80,83,87,89

         ; Offsets to the starting string indexed by name length.  Only
         ; names of the same length as the function name are checked.

BLIndex  EQU      *-6
         dc.w     BL3-BLTable             ; 3
         dc.w     BL4-BLTable             ; 4
         dc.w     BL5-BLTable             ; 5
         dc.w     BL6-BLTable             ; 6
         dc.w     BL7-BLTable             ; 7
         dc.w     BL8-BLTable             ; 8
         dc.w     BL9-BLTable             ; 9
         dc.w     BL10-BLTable            ; 10

         ; The table of jump offsets to the actual function code.

BLTable  dc.w     BIFabs-BIF_Jump
         dc.w     BIFarg-BIF_Jump
         dc.w     BIFb2c-BIF_Jump
         dc.w     BIFc2b-BIF_Jump
         dc.w     BIFc2d-BIF_Jump
         dc.w     BIFc2x-BIF_Jump
         dc.w     BIFd2c-BIF_Jump
         dc.w     BIFd2x-BIF_Jump
         dc.w     BIFeof-BIF_Jump
         dc.w     BIFmax-BIF_Jump
         dc.w     BIFmin-BIF_Jump
         dc.w     BIFpos-BIF_Jump
         dc.w     BIFx2c-BIF_Jump
         dc.w     BIFx2d-BIF_Jump

         dc.w     BIFdate-BIF_Jump
         dc.w     BIFfind-BIF_Jump
         dc.w     BIFform-BIF_Jump
         dc.w     BIFfuzz-BIF_Jump
         dc.w     BIFhash-BIF_Jump
         dc.w     BIFleft-BIF_Jump
         dc.w     BIFopen-BIF_Jump
         dc.w     BIFseek-BIF_Jump
         dc.w     BIFshow-BIF_Jump
         dc.w     BIFsign-BIF_Jump
         dc.w     BIFtime-BIF_Jump
         dc.w     BIFtrim-BIF_Jump
         dc.w     BIFword-BIF_Jump

         dc.w     BIFbitor-BIF_Jump
         dc.w     BIFclose-BIF_Jump
         dc.w     BIFindex-BIF_Jump
         dc.w     BIFlines-BIF_Jump
         dc.w     BIFrandu-BIF_Jump
         dc.w     BIFright-BIF_Jump
         dc.w     BIFspace-BIF_Jump
         dc.w     BIFstrip-BIF_Jump
         dc.w     BIFtrace-BIF_Jump
         dc.w     BIFtrunc-BIF_Jump
         dc.w     BIFupper-BIF_Jump
         dc.w     BIFvalue-BIF_Jump
         dc.w     BIFwords-BIF_Jump

         dc.w     BIFabbrev-BIF_Jump
         dc.w     BIFaddlib-BIF_Jump
         dc.w     BIFbitand-BIF_Jump
         dc.w     BIFbitchg-BIF_Jump
         dc.w     BIFbitclr-BIF_Jump
         dc.w     BIFbitset-BIF_Jump
         dc.w     BIFbittst-BIF_Jump
         dc.w     BIFbitxor-BIF_Jump
         dc.w     BIFcenter-BIF_Jump
         dc.w     BIFcenter-BIF_Jump
         dc.w     BIFcopies-BIF_Jump
         dc.w     BIFdelstr-BIF_Jump
         dc.w     BIFdigits-BIF_Jump
         dc.w     BIFexists-BIF_Jump
         dc.w     BIFexport-BIF_Jump
         dc.w     BIFimport-BIF_Jump
         dc.w     BIFinsert-BIF_Jump
         dc.w     BIFlength-BIF_Jump
         dc.w     BIFpragma-BIF_Jump
         dc.w     BIFrandom-BIF_Jump
         dc.w     BIFreadch-BIF_Jump
         dc.w     BIFreadln-BIF_Jump
         dc.w     BIFremlib-BIF_Jump
         dc.w     BIFsubstr-BIF_Jump
         dc.w     BIFsymbol-BIF_Jump
         dc.w     BIFverify-BIF_Jump
         dc.w     BIFxrange-BIF_Jump

         dc.w     BIFaddress-BIF_Jump
         dc.w     BIFbitcomp-BIF_Jump
         dc.w     BIFcompare-BIF_Jump
         dc.w     BIFdelword-BIF_Jump
         dc.w     BIFgetclip-BIF_Jump
         dc.w     BIFlastpos-BIF_Jump
         dc.w     BIFoverlay-BIF_Jump
         dc.w     BIFreverse-BIF_Jump
         dc.w     BIFsetclip-BIF_Jump
         dc.w     BIFstorage-BIF_Jump
         dc.w     BIFsubword-BIF_Jump
         dc.w     BIFwritech-BIF_Jump
         dc.w     BIFwriteln-BIF_Jump

         dc.w     BIFcompress-BIF_Jump
         dc.w     BIFdatatype-BIF_Jump
         dc.w     BIFgetspace-BIF_Jump

         dc.w     BIFerrortxt-BIF_Jump
         dc.w     BIFfreespce-BIF_Jump
         dc.w     BIFtranslat-BIF_Jump
         dc.w     BIFwordindx-BIF_Jump

         dc.w     BIFsourceln-BIF_Jump
         dc.w     BIFwordleng-BIF_Jump

         ; Names for the Built-In functions, preceded by the hash byte.
         ; Each name is followed by an argument byte containing the maximum
         ; and minimum number of arguments, and a conversion flags byte.

BL3      dc.b     214,'ABS',$11,$00
         dc.b     218,'ARG',$20,$01
         dc.b     183,'B2C',$11,$00
         dc.b     183,'C2B',$11,$00
         dc.b     185,'C2D',$21,$02
         dc.b     205,'C2X',$11,$00
         dc.b     185,'D2C',$21,$02
         dc.b     206,'D2X',$21,$02
         dc.b     218,'EOF',$11,$00
         dc.b     230,'MAX',$F2,$00
         dc.b     228,'MIN',$F2,$00
         dc.b     242,'POS',$32,$04
         dc.b     205,'X2C',$11,$00
         dc.b     206,'X2D',$21,$02

BL4      dc.b     30,'DATE',$30,$02
         dc.b     33,'FIND',$22,$00
         dc.b     52,'FORM',$00,$00
         dc.b     79,'FUZZ',$00,$00
         dc.b     36,'HASH',$11,$00
         dc.b     43,'LEFT',$32,$02
         dc.b     50,'OPEN',$32,$00
         dc.b     40,'SEEK',$32,$00
         dc.b     65,'SHOW',$31,$00
         dc.b     49,'SIGN',$11,$00
         dc.b     47,'TIME',$10,$00
         dc.b     60,'TRIM',$11,$00
         dc.b     60,'WORD',$32,$02

BL5      dc.b     128,'BITOR',$31,$00
         dc.b     118,'CLOSE',$11,$00
         dc.b     120,'INDEX',$32,$04
         dc.b     123,'LINES',$10,$00
         dc.b     122,'RANDU',$10,$01
         dc.b     126,'RIGHT',$32,$02
         dc.b     108,'SPACE',$31,$02
         dc.b     146,'STRIP',$31,$00
         dc.b     111,'TRACE',$10,$00
         dc.b     140,'TRUNC',$21,$02
         dc.b     140,'UPPER',$11,$00
         dc.b     125,'VALUE',$11,$00
         dc.b     143,'WORDS',$11,$00

BL6      dc.b     178,'ABBREV',$32,$04
         dc.b     160,'ADDLIB',$42,$08
         dc.b     178,'BITAND',$31,$00
         dc.b     177,'BITCHG',$22,$02
         dc.b     192,'BITCLR',$22,$02
         dc.b     203,'BITSET',$22,$02
         dc.b     218,'BITTST',$22,$02
         dc.b     216,'BITXOR',$31,$00
         dc.b     193,'CENTER',$32,$02
         dc.b     193,'CENTRE',$32,$02
         dc.b     195,'COPIES',$22,$02
         dc.b     206,'DELSTR',$32,$06
         dc.b     196,'DIGITS',$00,$00
         dc.b     224,'EXISTS',$11,$00
         dc.b     226,'EXPORT',$41,$04
         dc.b     219,'IMPORT',$21,$02
         dc.b     213,'INSERT',$52,$0C
         dc.b     194,'LENGTH',$11,$00
         dc.b     184,'PRAGMA',$21,$00
         dc.b     193,'RANDOM',$30,$07
         dc.b     167,'READCH',$21,$02
         dc.b     182,'READLN',$11,$00
         dc.b     187,'REMLIB',$11,$00
         dc.b     227,'SUBSTR',$42,$06
         dc.b     214,'SYMBOL',$11,$00
         dc.b     213,'VERIFY',$42,$08
         dc.b     197,'XRANGE',$20,$00

BL7      dc.b     6,'ADDRESS',$00,$00
         dc.b     14,'BITCOMP',$32,$00
         dc.b     7,'COMPARE',$32,$00
         dc.b     17,'DELWORD',$32,$06
         dc.b     8,'GETCLIP',$11,$00
         dc.b     38,'LASTPOS',$32,$04
         dc.b     34,'OVERLAY',$52,$0C
         dc.b     28,'REVERSE',$11,$00
         dc.b     20,'SETCLIP',$21,$00
         dc.b     21,'STORAGE',$40,$04
         dc.b     38,'SUBWORD',$32,$06
         dc.b     22,'WRITECH',$22,$00
         dc.b     37,'WRITELN',$22,$00

BL8      dc.b     108,'COMPRESS',$21,$00
         dc.b     92,'DATATYPE',$21,$00
         dc.b     76,'GETSPACE',$10,$01

BL9      dc.b     207,'ERRORTEXT',$11,$01
         dc.b     142,'FREESPACE',$20,$02
         dc.b     174,'TRANSLATE',$41,$00
         dc.b     180,'WORDINDEX',$22,$02

BL10     dc.b     249,'SOURCELINE',$10,$01
         dc.b     254,'WORDLENGTH',$32,$02
         CNOP     0,2
