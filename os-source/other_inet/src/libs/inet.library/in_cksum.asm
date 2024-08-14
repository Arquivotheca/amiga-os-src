	OPTIMON

	XDEF	_in_cksum
	XREF	_printf

	SECTION	IN_CKSUM,CODE
;   1: /*
;   2:  * Copyright (c) 1988 Regents of the University of California.
;   3:  * All rights reserved.
;   4:  *
;   5:  * Redistribution and use in source and binary forms are permitted
;   6:  * provided that the above copyright notice and this paragraph are
;   7:  * duplicated in all such forms and that any documentation,
;   8:  * advertising materials, and other materials related to such
;   9:  * distribution and use acknowledge that the software was developed
;  10:  * by the University of California, Berkeley.  The name of the
;  11:  * University may not be used to endorse or promote products derived
;  12:  * from this software without specific prior written permission.
;  13:  * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
;  14:  * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
;  15:  * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
;  16:  *
;  17:  *	@(#)in_cksum.c	7.2 (Berkeley) 6/29/88
;  18:  */
;  19: 
;  20: #include <sys/types.h>
;  21: #include <sys/mbuf.h>
;  22: 
;  23: #include "proto.h"
;  24: 
;  25: /*
;  26:  * Checksum routine for Internet Protocol family headers (Portable Version).
;  27:  *
;  28:  * This routine is very heavily used in the network
;  29:  * code and should be modified for each CPU to be as fast as possible.
;  30:  */
;  31: 
;  32: #define ADDCARRY(x)  (x > 65535 ? x -= 65535 : x)
;  33: #define REDUCE {l_util.l = sum; sum = l_util.s[0] + l_util.s[1]; ADDCARRY(sum);}
;  34: 
;  35: in_cksum(m, len)
;  36: 	register struct mbuf *m;
;  37: 	register int len;
;  38: {
_in_cksum
	LINK      A5,#-28
	MOVEM.L   D4-D7/A2-A3,-(A7)
	MOVEA.L   $0008(A5),A2
	MOVE.L    $000C(A5),D7
;  39: 	register u_short *w;
;  40: 	register int sum = 0;
	MOVEQ     #00,D5
;  41: 	register int mlen = 0;
	MOVEQ     #00,D6
;  42: 	int byte_swapped = 0;
	MOVEQ     #00,D4
;  43: 
;  44: 	union {
;  45: 		char	c[2];
;  46: 		u_short	s;
;  47: 	} s_util;
;  48: 	union {
;  49: 		u_short s[2];
;  50: 		long	l;
;  51: 	} l_util;
;  52: 
;  53: 	for (;m && len; m = m->m_next) {
	BRA.W     foo

main_loop:
;  54: 		if (m->m_len == 0)

	MOVE.W    $0008(A2),D0
	BEQ.W     bar

;  55: 			continue;
;  56: 		w = mtod(m, u_short *);

	MOVE.L    A2,D0
	ADD.L     $0004(A2),D0
	MOVE.L    D0,A3

;  57: 		if (mlen == -1) {
	MOVEQ     #-1,D0
	CMP.L     D0,D6
	BNE.B     notneg

;  58: 			/*
;  59: 			 * The first byte of this mbuf is the continuation
;  60: 			 * of a word spanning between this mbuf and the
;  61: 			 * last mbuf.
;  62: 			 *
;  63: 			 * s_util.c[0] is already saved when scanning previous 
;  64: 			 * mbuf.
;  65: 			 */
;  66: 			s_util.c[1] = *(char *)w;
	MOVE.B    (A3),D0
	MOVE.B    D0,-21(A5)
;  67: 			sum += s_util.s;
	MOVEQ     #00,D0
	MOVE.W    -22(A5),D0
	ADD.L     D0,D5
;  68: 			w = (u_short *)((char *)w + 1);
	ADDQ.L    #1,A3
;  69: 			mlen = m->m_len - 1;
	MOVE.W    $0008(A2),D0
	EXT.L     D0
	MOVE.L    D0,D6
	SUBQ.L    #1,D6
;  70: 			len--;
	SUBQ.L    #1,D7
;  71: 		} else
	BRA.B     cont1

;  72: 			mlen = m->m_len;
notneg:
	MOVE.W    $0008(A2),D6
	EXT.L     D6

;  73: 		if (len < mlen)
cont1:
	CMP.L     D6,D7
	BGE.B     cont2

;  74: 			mlen = len;
	MOVE.L    D7,D6

;  75: 		len -= mlen;
cont2:
	SUB.L     D6,D7

;  76: 		/*
;  77: 		 * Force to even boundary.
;  78: 		 */
;  79: 		if ((1 & (int) w) && (mlen > 0)) {
	MOVE.L    A3,D0
	BTST      #0,D0
	BEQ.B     cont3
	TST.L     D6
	BLE.B     cont3

;  80: 			REDUCE;
	MOVE.L    D5,-26(A5)
	MOVEQ     #00,D0
	MOVE.W    -24(A5),D0
	MOVEQ     #00,D1
	MOVE.W    -26(A5),D1
	ADD.L     D0,D1
	MOVE.L    D1,D5
	CMPI.L    #$0000FFFF,D5
	BLE.B     foobar
	SUBI.L    #$0000FFFF,D5

;  81: 			sum <<= 8;
foobar:	ASL.L     #8,D5

;  82: 			s_util.c[0] = *(u_char *)w;
	MOVE.B    (A3),D0
	MOVE.B    D0,-22(A5)
;  83: 			w = (u_short *)((char *)w + 1);
	ADDQ.L    #1,A3
;  84: 			mlen--;
	SUBQ.L    #1,D6
;  85: 			byte_swapped = 1;
	MOVEQ     #01,D4
;  86: 		}
	BRA.B     cont3

;  87: 		/*
;  88: 		 * Unroll the loop to make overhead from
;  89: 		 * branches &c small.  
;  90: 		 */
;  91: 		while ((mlen -= 32) >= 0) {
;  92: 			sum += w[0]; sum += w[1]; sum += w[2]; sum += w[3];
while32:
	MOVEQ      #0,D0
	MOVE.W     (A3)+,D0
	ADD.L      D0,D5	
	MOVE.W     (A3)+,D0
	ADD.L      D0,D5	
	MOVE.W     (A3)+,D0
	ADD.L      D0,D5	
	MOVE.W     (A3)+,D0
	ADD.L      D0,D5	
;  93: 			sum += w[4]; sum += w[5]; sum += w[6]; sum += w[7];
	MOVE.W     (A3)+,D0
	ADD.L      D0,D5	
	MOVE.W     (A3)+,D0
	ADD.L      D0,D5	
	MOVE.W     (A3)+,D0
	ADD.L      D0,D5	
	MOVE.W     (A3)+,D0
	ADD.L      D0,D5	
;  94: 			sum += w[8]; sum += w[9]; sum += w[10]; sum += w[11];
	MOVE.W     (A3)+,D0
	ADD.L      D0,D5	
	MOVE.W     (A3)+,D0
	ADD.L      D0,D5	
	MOVE.W     (A3)+,D0
	ADD.L      D0,D5	
	MOVE.W     (A3)+,D0
	ADD.L      D0,D5	
;  95: 			sum += w[12]; sum += w[13]; sum += w[14]; sum += w[15];
	MOVE.W     (A3)+,D0
	ADD.L      D0,D5	
	MOVE.W     (A3)+,D0
	ADD.L      D0,D5	
	MOVE.W     (A3)+,D0
	ADD.L      D0,D5	
	MOVE.W     (A3)+,D0
	ADD.L      D0,D5	
;  96: 			w += 16;
;  97: 		}
cont3:
	MOVEQ     #$20,D0
	SUB.L     D0,D6
	BGE.B     while32

;  98: 		mlen += 32;
	MOVEQ     #$20,D0
	ADD.L     D0,D6
;  99: 		while ((mlen -= 8) >= 0) {
	BRA.B     cont4

; 100: 			sum += w[0]; sum += w[1]; sum += w[2]; sum += w[3];
while8:
	MOVEQ      #0,D0
	MOVE.W     (A3)+,D0
	ADD.L      D0,D5	
	MOVE.W     (A3)+,D0
	ADD.L      D0,D5	
	MOVE.W     (A3)+,D0
	ADD.L      D0,D5	
	MOVE.W     (A3)+,D0
	ADD.L      D0,D5	
; 101: 			w += 4;
; 102: 		}
cont4:
	SUBQ.L    #8,D6
	BGE.B     while8
; 103: 		mlen += 8;
	ADDQ.L    #8,D6
; 104: 		if (mlen == 0 && byte_swapped == 0)
	BNE.B     reduce
	TST.L     D4
	BEQ.W     bar
; 105: 			continue;
; 106: 		REDUCE;
reduce:
	MOVE.L    D5,-26(A5)
	MOVEQ     #00,D0
	MOVE.W    -24(A5),D0
	MOVEQ     #00,D1
	MOVE.W    -26(A5),D1
	ADD.L     D0,D1
	MOVE.L    D1,D5
	CMPI.L    #$0000FFFF,D5
	BLE.B     cont5
	SUBI.L    #$0000FFFF,D5
	BRA.B     cont5
; 107: 		while ((mlen -= 2) >= 0) {
; 108: 			sum += *w++;
while2:
	MOVEQ     #00,D0
	MOVE.W    (A3)+,D0
	ADD.L     D0,D5
; 109: 		}
cont5:
	SUBQ.L    #2,D6
	BGE.B     while2

; 110: 		if (byte_swapped) {
	TST.L     D4
	BEQ.B     no_byte

; 111: 			REDUCE;
	MOVE.L    D5,-26(A5)
	MOVEQ     #00,D0
	MOVE.W    -24(A5),D0
	MOVEQ     #00,D1
	MOVE.W    -26(A5),D1
	ADD.L     D0,D1
	MOVE.L    D1,D5
	CMPI.L    #$0000FFFF,D5
	BLE.B     cont6
	SUBI.L    #$0000FFFF,D5

; 112: 			sum <<= 8;
cont6:
	ASL.L     #8,D5

; 113: 			byte_swapped = 0;
	MOVEQ     #00,D4

; 114: 			if (mlen == -1) {
	MOVEQ     #-1,D0
	CMP.L     D0,D6
	BNE.B     neg_mlen
; 115: 				s_util.c[1] = *(char *)w;
	MOVE.B    (A3),D0
	MOVE.B    D0,-21(A5)
; 116: 				sum += s_util.s;
	MOVEQ     #00,D0
	MOVE.W    -22(A5),D0
	ADD.L     D0,D5
; 117: 				mlen = 0;
	MOVEQ     #00,D6
	BRA.B     bar
; 118: 			} else
; 119: 				mlen = -1;
neg_mlen:
	MOVEQ     #-1,D6

; 120: 		} else if (mlen == -1)
	BRA.B     bar
no_byte:
	MOVEQ     #-1,D0
	CMP.L     D0,D6
	BNE.B     bar
; 121: 			s_util.c[0] = *(char *)w;
	MOVE.B    (A3),-22(A5)
; 122: 	}
;--------------------------------------------------
bar:
	MOVEA.L   (A2),A2
foo:
	MOVE.L    A2,D0
	BEQ.B     jello
	TST.L     D7
	BNE.W     main_loop
; 123: 	if (len)
jello:
	TST.L     D7
	BEQ.B     cont7
; 124: 		printf("cksum: out of data\n");
	PEA       message(PC)
	JSR       _printf(PC)
	ADDQ.W    #4,A7
; 125: 	if (mlen == -1) {
cont7:
	MOVEQ     #-1,D0
	CMP.L     D0,D6
	BNE.B     cont8
; 126: 		/* The last mbuf has odd # of bytes. Follow the
; 127: 		   standard (the odd byte may be shifted left by 8 bits
; 128: 		   or not as determined by endian-ness of the machine) */
; 129: 		s_util.c[1] = 0;
	CLR.B     -21(A5)
; 130: 		sum += s_util.s;
	MOVEQ     #00,D0
	MOVE.W    -22(A5),D0
	ADD.L     D0,D5
; 131: 	}
; 132: 	REDUCE;
cont8:
	MOVE.L    D5,-26(A5)
	MOVEQ     #00,D0
	MOVE.W    -24(A5),D0
	MOVEQ     #00,D1
	MOVE.W    -26(A5),D1
	ADD.L     D0,D1
	MOVE.L    D1,D6
	CMPI.L    #$0000FFFF,D6
	BLE.B     cont9
	MOVE.L    #$0000FFFF,D0
	SUB.L     D0,D6
; 133: 	return (~sum & 0xffff);
cont9:
	MOVE.L    D6,D0
	NOT.L     D0
	ANDI.L    #$0000FFFF,D0
; 134: }
	MOVEM.L   (A7)+,D4-D7/A2-A3
	UNLK      A5
	RTS

message	DC.B	'cksum: out of data',10,0
