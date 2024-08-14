/***********************************************************************
****************                                        ****************
****************        -=   CD FILE SYSTEM   =-        ****************
****************                                        ****************
************************************************************************
***                                                                  ***
***   Written by Carl Sassenrath for Commodore International, Ltd.   ***
*** Copyright (C) 1991 Commodore Electronics Ltd. All rights reserved***                                                                  ***
***                                                                  ***
***         Confidential and Proprietary System Source Code          ***
***                                                                  ***
***********************************************************************/

#include "main.h"

/***********************************************************************
***
***  UpperChars
***
*** Default character set for string compares.  All alpha chars
*** including European chars are in uppercase.
***
***********************************************************************/
UCHAR dUpperChars[256] =
{
#asm
    cseg        ; MANX 3.6 does not have const arrays...
    public  _UpperChars
_UpperChars:        ; So this is needed to put array in code seg.
#endasm
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 
    0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 
    0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 
    0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f, 
    0x60, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 
    0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 
    0x58, 0x59, 0x5a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f, 
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 
    0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f, 
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 
    0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f, 
    0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 
    0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf, 
    0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 
    0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf, 
    0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 
    0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, 
    0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 
    0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf, 
    0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 
    0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, 
    0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xf7, 
    0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xff, 
};

#asm
clear   MACRO
        moveq.l #0,\1
    ENDM

************************************************************************
***
***  CopyStr(target,source,maxlength)
***
*** Copy a string. Terminate copy when:
***
***     1. a MAX length (64K max)
***     2. a NULL character
***
*** Return the string length.
***
************************************************************************
    public  _CopyStr
_CopyStr:
        move.l  1*4(sp),a1  ; target
        move.l  2*4(sp),a0  ; source
        move.l  3*4(sp),d0  ; max length
        bra.s   6$
2$:
        move.b  (a0)+,(a1)+ ; move char
6$:     dbeq    d0,2$       ; repeat for max len or terminated

        move.l  1*4(sp),a0
        sub.l   a0,a1       ; calculate length
        move.l  a1,d0
        subq.l  #1,d0
        rts         ; return length

************************************************************************
***
***  DStrToBStr(target,source,maxlength)
***
*** Copy and convert an ISO-9660 'D' string to a DOS BCPL string.
*** ISO 'D' strings are normally found in Vol Descriptors.
*** Such ISO strings contain no length byte and are terminated by:
***
***     1. a MAX length
***     2. a SPACE character ($20)
***     3. a NULL character (safety Amiga Extension)
***
*** The resulting string area must be large enough to hold both
*** the BSTR length byte and a NULL terminator (for C's sake) in
*** addition to the string's characters.
***
*** No case or escape conversions are done.
***
*** Return the string length (string only).
***
************************************************************************
SPC     equ $20

    public  _DStrToBStr
_DStrToBStr:
        move.l  1*4(sp),a1  ; target
        move.l  2*4(sp),a0  ; source
        move.l  3*4(sp),d0  ; max length

        clr.b   (a1)+       ; room for len byte
        bra.s   6$
2$:
        move.b  (a0)+,d1    ; get char
        beq.s   8$
        cmp.b   #SPC,d1     ; is it a space?
        beq.s   8$
        move.b  d1,(a1)+    ; store char
6$:     dbf d0,2$       ; repeat for max len

8$:     clr.b   (a1)+       ; terminator
        move.l  1*4(sp),a0
        sub.l   a0,a1       ; calculate length
        move.l  a1,d0
        subq.l  #2,d0
        move.b  d0,(a0)     ; store in len byte

        rts         ; return length

************************************************************************
***
***  MakeFileName(name,ident)
***
*** Creates a BSTR string from an ISO directory entry identifer.
*** Strips out the ';' when found.  Leaves the case of the string
*** alone. Null terminates the final string (to be nice).
***
************************************************************************
    public  _MakeFileName
_MakeFileName:
        move.l  1*4(sp),a1  ; target name
        move.l  2*4(sp),a0  ; source ident
        moveq.l #0,d0
        move.b  (a0)+,d0    ; length from DIR entry
        clr.b   (a1)+       ; room for len byte
        bra.s   6$
2$:
        move.b  (a0)+,d1    ; get char
        beq.s   8$
        cmp.b   #';',d1     ; is it a version delimiter?
        beq.s   8$
        move.b  d1,(a1)+    ; store char
6$:     dbf d0,2$       ; repeat for max len

8$:     clr.b   (a1)+       ; terminator
        move.l  1*4(sp),a0
        sub.l   a0,a1       ; calculate length
        move.l  a1,d0
        subq.l  #2,d0       ; sub len and term bytes
        move.b  d0,(a0)     ; store in len byte

        rts         ; return length

************************************************************************
***
***  MakeISOName(name,ident)
***
*** Same as MakeFileName but uppercases letters to prep for
*** comparison.
***
************************************************************************
    public  _MakeISOName
    public  _CharSet
_MakeISOName:
        move.l  a2,-(sp)
        move.l  2*4(sp),a1  ; target name
        move.l  3*4(sp),a0  ; source ident
        moveq.l #0,d0
        move.b  (a0)+,d0    ; length from DIR entry
        clr.b   (a1)+       ; room for len byte
        move.l  _CharSet,a2
        moveq.l #0,d1
        bra.s   6$
2$:
        move.b  (a0)+,d1    ; get char
        beq.s   8$
        cmp.b   #';',d1     ; is it a version delimiter?
        beq.s   8$
        move.b  0(a2,d1.w),d1   ; translate char
        move.b  d1,(a1)+    ; store char
6$:     dbf d0,2$       ; repeat for max len

8$:     clr.b   (a1)+       ; terminator
        move.l  2*4(sp),a0
        sub.l   a0,a1       ; calculate length
        move.l  a1,d0
        subq.l  #2,d0       ; sub len and term bytes
        move.b  d0,(a0)     ; store in len byte

        move.l  (sp)+,a2
        rts         ; return length

************************************************************************
***
***  CmpISOStr
***
*** Compare to ISO strings BStr.  These strings MUST be NULL
*** terminated.  Return the difference of the last char that
*** differs.  This works for strings of unequal lengths.
*** This is a case sensitive compare.
***
***     return  < 0 if  ARG1 < ARG2
***         > 0 if  ARG1 > ARG2
***         = 0 if  ARG1 = ARG2
***
************************************************************************
    public  _CmpISOStr
_CmpISOStr:
        move.l  1*4(sp),a1  ; t
        move.l  2*4(sp),a0  ; s

        clear   d0
        clear   d1
        move.b  (a1)+,d0    ; length
        move.b  (a0)+,d1    ; length
        cmp.b   d0,d1
        bgt.s   1$      ; want smaller of the two
        move.b  d1,d0       ; d1 smaller
1$:     clear   d1      ; set CC's for dbne
                    ; (do one extra repeat)
2$:     cmp.b   (a0)+,(a1)+
6$:     dbne    d0,2$

        move.b  -1(a1),d0
        sub.b   -1(a0),d0   ; calculate difference
        ext.w   d0
        ext.l   d0
        rts         ; return difference

************************************************************************
***
***  MatchStr
***
*** Compare a C string with a fixed length array of chars (not
*** necessarily NULL terminated).  Return TRUE if the strings
*** are the same.  This is a case sensitive compare.
***
************************************************************************
    public  _MatchStr
_MatchStr:
        move.l  1*4(sp),a1  ; t
        move.l  2*4(sp),a0  ; s
        move.l  3*4(sp),d0  ; len
        clear   d1      ; set CC's for dbne
        bra.s   6$
2$:     cmp.b   (a0)+,(a1)+
6$:     dbne    d0,2$
        bne.s   8$      ; they differ
        moveq.l #-1,d0      ; return TRUE
        bra.s   9$
8$:     clear   d0      ; return FALSE
9$:     rts

************************************************************************
***
***  MatchBStr
***
*** Compare two BStrings.  Return TRUE if they are the same.
*** This is a case sensitive compare.
***
************************************************************************
    public  _MatchBStr
_MatchBStr:
        move.l  1*4(sp),a1  ; t
        move.l  2*4(sp),a0  ; s

        clear   d0
        move.b  (a0)+,d0
        move.b  (a1)+,d1
        cmp.b   d0,d1
        bne.s   8$
        bra.s   6$
2$:     cmp.b   (a0)+,(a1)+
6$:     dbne    d0,2$
        bne.s   8$      ; they differ
        moveq.l #-1,d0      ; return TRUE
        bra.s   9$
8$:     clear   d0      ; return FALSE
9$:     rts         ; return difference

************************************************************************
***
***  FlipBytes(long)
***
*** Convert a long word from 68000 to Intel format.
***
************************************************************************
    public  _FlipBytes
_FlipBytes:
        move.l  4(sp),d1
        move.l  d2,-(sp)
        moveq   #3,d2
1$:     rol.l   #8,d1
        move.b  d1,d0
        ror.l   #8,d0
        dbf d2,1$
        move.l  (sp)+,d2
        rts
#endasm
