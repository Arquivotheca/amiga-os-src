**
**	mmu - A tiny bonus rom module that holds the MMU tables.
**


*************************************************************************
*									*
*   Copyright 1990 Commodore-Amiga, Inc.				*
*   All rights reserved.  No part of this program may be reproduced,	*
*   transmitted, transcribed, stored in retrieval system, or		*
*   translated into any language or computer language, in any form	*
*   or by any means, electronic, mechanical, magnetic, optical, 	*
*   chemical, manual or otherwise, without the prior written		*
*   permission of Commodore-Amiga, Incorporated.			*
*									*
*************************************************************************
		INCLUDE "exec/types.i"
		INCLUDE "exec/macros.i"
		INCLUDE "exec/resident.i"
		INCLUDE "exec/memory.i"
		INCLUDE "exec/ables.i"
		INT_ABLES

		XREF	_LVOSupervisor


;Gary control block.  One bit per byte sized memory location (Bit 7).
A3000_BusTimeoutMode		EQU $00de0000	;0=DSACK 1=BERR|0=ok 1=timed out
A3000_BusTimeoutEnable		EQU $00de0001	;Bit 7	0=enable 1=disable
A3000_PUD			EQU $00de0002	;Power-Up-Detect. Bit 7, 1=cold

ABSEXECBASE	equ 4

END_MEM 	EQU	$08000000
K_SIZE		EQU	256*1024
K_MEM		EQU	END_MEM-K_SIZE-K_SIZE
CHIP_WRAP	EQU	1024*1024		;Magic
;----------------------------------------------------------------------------


;
;   POSITION INDEPENDENT CODE
;
;
StartCode:
		dc.w	$0000	    ;fake Exec 0 degree Kelvin capture
		dc.w	$feed
		dc.l	$c0edbabe

		dc.l	0	    ;Magik longword for Bill Koester to use

enablemmu:	move.l	ABSEXECBASE,a6
		DISABLE
		lea.l	supermode(pc),a5
		JMP	_LVOSupervisor(a6)

supermode:
		;
		;   Quick memory test.	If we are a 1MB chip system, fold
		;   the other 1MB chip area on top.
		;
		;   Run before MMU is enabled.	Must modify MMU table with
		;   real addresses.
		;
		lea.l	CHIP_WRAP,a0	    ;poke at this address
		move.l	(a0),d1
		not.l	d1
		not.l	(a0)
		move.l	$10,a1		    ;magic (confuse bus)
		cmp.l	(a0),d1
		bne.s	tm_OneMeg
		not.l	d1
		not.l	(a0)                ;restore longword
		move.l	$10,a1		    ;magic (confuse bus)
		cmp.l	(a0),d1
		beq.s	tm_TwoMeg	    ;Two megs of chip? Do nothing.

		;
		;   Ok, wrap 'em.
		;
tm_OneMeg:	lea.l	LevelB(pc),a0
		lea.l	LevelB+16(pc),a1
		move.l	(a0)+,(a1)+
		move.l	(a0)+,(a1)+
		move.l	(a0)+,(a1)+
		move.l	(a0)+,(a1)+
tm_TwoMeg:




		;
		;   Turn on MMU.  This write-protects ourself
		;
		lea.l	mmu_CRP(pc),a0
		dc.w	$f010			; PMOVE (a0),CRP [with flush]
		dc.w	$4c00
		lea.l	mmu_TC(pc),a0
		dc.w	$f010			; PMOVE (a0),TC
		dc.w	$4000
		lea.l	mmu_TT(pc),a0
		dc.w	$f010			; PMOVE (a0),TT0
		dc.w	$0800
		dc.w	$f010			; PMOVE (a0),TT1
		dc.w	$0c00
		suba.l	a0,a0
		dc.w	$4e7b,$8801		; movec.l   a0,vbr


		;
		;   Turn on BERR termination so we don't confuse
		;   dumb Zorro II cards.  Permissor will keep the software
		;   from puking.
		;
		move.b	#$80,A3000_BusTimeoutMode
		move.b	#$00,A3000_BusTimeoutEnable

		;
		;   WARNING: MAGIC STARTS HERE.  TALK TO BRYCE BEFORE
		;   EVEN _THINKING_ ABOUT CHANGING THIS REBOOT CODE!
		;
		move.l	#$00004ef9,0 ;in case the 8520 is not quick enough
		move.l	#$00fc0002,4 ;in case the 8520 is not quick enough
		lea.l	$00fc0002,a0 ;Point to JMP instruction in V1.3 ROM
		bra	MagicResetCode
;-------------- MagicResetCode ---------CHANGED FOR A GOOD REASON-----------

		CNOP	0,4	;IMPORTANT!  Longword align!  Do not change!
MagicResetCode: RESET		;all RAM goes away now!
		jmp	(a0)    ;Rely on prefetch to execute this instruction
;-------------- MagicResetCode ---------CHANGED FOR A GOOD REASON-----------



mmu_CRP 	dc.l	$80000002,(LevelA-StartCode+K_MEM+K_SIZE)
mmu_TC		dc.l	$80f08630	    ;32K pages-8 bits-6 bits-3 bits
mmu_TT		dc.l	$00000000,$00000000


A_DEFAULT	EQU (LevelB-StartCode+K_MEM+K_SIZE)!2
FAST_MEM	EQU (LevelBB-StartCode+K_MEM+K_SIZE)!2

		CNOP 0,16
LevelA		dc.l A_DEFAULT	;  0, 00000000
		dc.l A_DEFAULT	;  1, 01000000
		dc.l A_DEFAULT	;  2, 02000000
		dc.l A_DEFAULT	;  3, 03000000
		dc.l A_DEFAULT	;  4, 04000000
		dc.l A_DEFAULT	;  5, 05000000
		dc.l A_DEFAULT	;  6, 06000000
		dc.l FAST_MEM	;  7, 07000000	    fast memory
		dc.l A_DEFAULT	;  8, 08000000
		dc.l A_DEFAULT	;  9, 09000000
		dc.l A_DEFAULT	; 10, 0a000000
		dc.l A_DEFAULT	; 11, 0b000000
		dc.l A_DEFAULT	; 12, 0c000000
		dc.l A_DEFAULT	; 13, 0d000000
		dc.l A_DEFAULT	; 14, 0e000000
		dc.l A_DEFAULT	; 15, 0f000000
		dc.l A_DEFAULT	; 16, 10000000
		dc.l A_DEFAULT	; 17, 11000000
		dc.l A_DEFAULT	; 18, 12000000
		dc.l A_DEFAULT	; 19, 13000000
		dc.l A_DEFAULT	; 20, 14000000
		dc.l A_DEFAULT	; 21, 15000000
		dc.l A_DEFAULT	; 22, 16000000
		dc.l A_DEFAULT	; 23, 17000000
		dc.l A_DEFAULT	; 24, 18000000
		dc.l A_DEFAULT	; 25, 19000000
		dc.l A_DEFAULT	; 26, 1a000000
		dc.l A_DEFAULT	; 27, 1b000000
		dc.l A_DEFAULT	; 28, 1c000000
		dc.l A_DEFAULT	; 29, 1d000000
		dc.l A_DEFAULT	; 30, 1e000000
		dc.l A_DEFAULT	; 31, 1f000000
		dc.l A_DEFAULT	; 32, 20000000
		dc.l A_DEFAULT	; 33, 21000000
		dc.l A_DEFAULT	; 34, 22000000
		dc.l A_DEFAULT	; 35, 23000000
		dc.l A_DEFAULT	; 36, 24000000
		dc.l A_DEFAULT	; 37, 25000000
		dc.l A_DEFAULT	; 38, 26000000
		dc.l A_DEFAULT	; 39, 27000000
		dc.l A_DEFAULT	; 40, 28000000
		dc.l A_DEFAULT	; 41, 29000000
		dc.l A_DEFAULT	; 42, 2a000000
		dc.l A_DEFAULT	; 43, 2b000000
		dc.l A_DEFAULT	; 44, 2c000000
		dc.l A_DEFAULT	; 45, 2d000000
		dc.l A_DEFAULT	; 46, 2e000000
		dc.l A_DEFAULT	; 47, 2f000000
		dc.l A_DEFAULT	; 48, 30000000
		dc.l A_DEFAULT	; 49, 31000000
		dc.l A_DEFAULT	; 50, 32000000
		dc.l A_DEFAULT	; 51, 33000000
		dc.l A_DEFAULT	; 52, 34000000
		dc.l A_DEFAULT	; 53, 35000000
		dc.l A_DEFAULT	; 54, 36000000
		dc.l A_DEFAULT	; 55, 37000000
		dc.l A_DEFAULT	; 56, 38000000
		dc.l A_DEFAULT	; 57, 39000000
		dc.l A_DEFAULT	; 58, 3a000000
		dc.l A_DEFAULT	; 59, 3b000000
		dc.l A_DEFAULT	; 60, 3c000000
		dc.l A_DEFAULT	; 61, 3d000000
		dc.l A_DEFAULT	; 62, 3e000000
		dc.l A_DEFAULT	; 63, 3f000000
		dc.l A_DEFAULT	; 64, 40000000
		dc.l A_DEFAULT	; 65, 41000000
		dc.l A_DEFAULT	; 66, 42000000
		dc.l A_DEFAULT	; 67, 43000000
		dc.l A_DEFAULT	; 68, 44000000
		dc.l A_DEFAULT	; 69, 45000000
		dc.l A_DEFAULT	; 70, 46000000
		dc.l A_DEFAULT	; 71, 47000000
		dc.l A_DEFAULT	; 72, 48000000
		dc.l A_DEFAULT	; 73, 49000000
		dc.l A_DEFAULT	; 74, 4a000000
		dc.l A_DEFAULT	; 75, 4b000000
		dc.l A_DEFAULT	; 76, 4c000000
		dc.l A_DEFAULT	; 77, 4d000000
		dc.l A_DEFAULT	; 78, 4e000000
		dc.l A_DEFAULT	; 79, 4f000000
		dc.l A_DEFAULT	; 80, 50000000
		dc.l A_DEFAULT	; 81, 51000000
		dc.l A_DEFAULT	; 82, 52000000
		dc.l A_DEFAULT	; 83, 53000000
		dc.l A_DEFAULT	; 84, 54000000
		dc.l A_DEFAULT	; 85, 55000000
		dc.l A_DEFAULT	; 86, 56000000
		dc.l A_DEFAULT	; 87, 57000000
		dc.l A_DEFAULT	; 88, 58000000
		dc.l A_DEFAULT	; 89, 59000000
		dc.l A_DEFAULT	; 90, 5a000000
		dc.l A_DEFAULT	; 91, 5b000000
		dc.l A_DEFAULT	; 92, 5c000000
		dc.l A_DEFAULT	; 93, 5d000000
		dc.l A_DEFAULT	; 94, 5e000000
		dc.l A_DEFAULT	; 95, 5f000000
		dc.l A_DEFAULT	; 96, 60000000
		dc.l A_DEFAULT	; 97, 61000000
		dc.l A_DEFAULT	; 98, 62000000
		dc.l A_DEFAULT	; 99, 63000000
		dc.l A_DEFAULT	;100, 64000000
		dc.l A_DEFAULT	;101, 65000000
		dc.l A_DEFAULT	;102, 66000000
		dc.l A_DEFAULT	;103, 67000000
		dc.l A_DEFAULT	;104, 68000000
		dc.l A_DEFAULT	;105, 69000000
		dc.l A_DEFAULT	;106, 6a000000
		dc.l A_DEFAULT	;107, 6b000000
		dc.l A_DEFAULT	;108, 6c000000
		dc.l A_DEFAULT	;109, 6d000000
		dc.l A_DEFAULT	;110, 6e000000
		dc.l A_DEFAULT	;111, 6f000000
		dc.l A_DEFAULT	;112, 70000000
		dc.l A_DEFAULT	;113, 71000000
		dc.l A_DEFAULT	;114, 72000000
		dc.l A_DEFAULT	;115, 73000000
		dc.l A_DEFAULT	;116, 74000000
		dc.l A_DEFAULT	;117, 75000000
		dc.l A_DEFAULT	;118, 76000000
		dc.l A_DEFAULT	;119, 77000000
		dc.l A_DEFAULT	;120, 78000000
		dc.l A_DEFAULT	;121, 79000000
		dc.l A_DEFAULT	;122, 7a000000
		dc.l A_DEFAULT	;123, 7b000000
		dc.l A_DEFAULT	;124, 7c000000
		dc.l A_DEFAULT	;125, 7d000000
		dc.l A_DEFAULT	;126, 7e000000
		dc.l A_DEFAULT	;127, 7f000000
		dc.l A_DEFAULT	;128, 80000000
		dc.l A_DEFAULT	;129, 81000000
		dc.l A_DEFAULT	;130, 82000000
		dc.l A_DEFAULT	;131, 83000000
		dc.l A_DEFAULT	;132, 84000000
		dc.l A_DEFAULT	;133, 85000000
		dc.l A_DEFAULT	;134, 86000000
		dc.l FAST_MEM	;135, 87000000 bit 31 mirror of fast memory
		dc.l A_DEFAULT	;136, 88000000
		dc.l A_DEFAULT	;137, 89000000
		dc.l A_DEFAULT	;138, 8a000000
		dc.l A_DEFAULT	;139, 8b000000
		dc.l A_DEFAULT	;140, 8c000000
		dc.l A_DEFAULT	;141, 8d000000
		dc.l A_DEFAULT	;142, 8e000000
		dc.l A_DEFAULT	;143, 8f000000
		dc.l A_DEFAULT	;144, 90000000
		dc.l A_DEFAULT	;145, 91000000
		dc.l A_DEFAULT	;146, 92000000
		dc.l A_DEFAULT	;147, 93000000
		dc.l A_DEFAULT	;148, 94000000
		dc.l A_DEFAULT	;149, 95000000
		dc.l A_DEFAULT	;150, 96000000
		dc.l A_DEFAULT	;151, 97000000
		dc.l A_DEFAULT	;152, 98000000
		dc.l A_DEFAULT	;153, 99000000
		dc.l A_DEFAULT	;154, 9a000000
		dc.l A_DEFAULT	;155, 9b000000
		dc.l A_DEFAULT	;156, 9c000000
		dc.l A_DEFAULT	;157, 9d000000
		dc.l A_DEFAULT	;158, 9e000000
		dc.l A_DEFAULT	;159, 9f000000
		dc.l A_DEFAULT	;160, a0000000
		dc.l A_DEFAULT	;161, a1000000
		dc.l A_DEFAULT	;162, a2000000
		dc.l A_DEFAULT	;163, a3000000
		dc.l A_DEFAULT	;164, a4000000
		dc.l A_DEFAULT	;165, a5000000
		dc.l A_DEFAULT	;166, a6000000
		dc.l A_DEFAULT	;167, a7000000
		dc.l A_DEFAULT	;168, a8000000
		dc.l A_DEFAULT	;169, a9000000
		dc.l A_DEFAULT	;170, aa000000
		dc.l A_DEFAULT	;171, ab000000
		dc.l A_DEFAULT	;172, ac000000
		dc.l A_DEFAULT	;173, ad000000
		dc.l A_DEFAULT	;174, ae000000
		dc.l A_DEFAULT	;175, af000000
		dc.l A_DEFAULT	;176, b0000000
		dc.l A_DEFAULT	;177, b1000000
		dc.l A_DEFAULT	;178, b2000000
		dc.l A_DEFAULT	;179, b3000000
		dc.l A_DEFAULT	;180, b4000000
		dc.l A_DEFAULT	;181, b5000000
		dc.l A_DEFAULT	;182, b6000000
		dc.l A_DEFAULT	;183, b7000000
		dc.l A_DEFAULT	;184, b8000000
		dc.l A_DEFAULT	;185, b9000000
		dc.l A_DEFAULT	;186, ba000000
		dc.l A_DEFAULT	;187, bb000000
		dc.l A_DEFAULT	;188, bc000000
		dc.l A_DEFAULT	;189, bd000000
		dc.l A_DEFAULT	;190, be000000
		dc.l A_DEFAULT	;191, bf000000
		dc.l A_DEFAULT	;192, c0000000
		dc.l A_DEFAULT	;193, c1000000
		dc.l A_DEFAULT	;194, c2000000
		dc.l A_DEFAULT	;195, c3000000
		dc.l A_DEFAULT	;196, c4000000
		dc.l A_DEFAULT	;197, c5000000
		dc.l A_DEFAULT	;198, c6000000
		dc.l A_DEFAULT	;199, c7000000
		dc.l A_DEFAULT	;200, c8000000
		dc.l A_DEFAULT	;201, c9000000
		dc.l A_DEFAULT	;202, ca000000
		dc.l A_DEFAULT	;203, cb000000
		dc.l A_DEFAULT	;204, cc000000
		dc.l A_DEFAULT	;205, cd000000
		dc.l A_DEFAULT	;206, ce000000
		dc.l A_DEFAULT	;207, cf000000
		dc.l A_DEFAULT	;208, d0000000
		dc.l A_DEFAULT	;209, d1000000
		dc.l A_DEFAULT	;210, d2000000
		dc.l A_DEFAULT	;211, d3000000
		dc.l A_DEFAULT	;212, d4000000
		dc.l A_DEFAULT	;213, d5000000
		dc.l A_DEFAULT	;214, d6000000
		dc.l A_DEFAULT	;215, d7000000
		dc.l A_DEFAULT	;216, d8000000
		dc.l A_DEFAULT	;217, d9000000
		dc.l A_DEFAULT	;218, da000000
		dc.l A_DEFAULT	;219, db000000
		dc.l A_DEFAULT	;220, dc000000
		dc.l A_DEFAULT	;221, dd000000
		dc.l A_DEFAULT	;222, de000000
		dc.l A_DEFAULT	;223, df000000
		dc.l A_DEFAULT	;224, e0000000
		dc.l A_DEFAULT	;225, e1000000
		dc.l A_DEFAULT	;226, e2000000
		dc.l A_DEFAULT	;227, e3000000
		dc.l A_DEFAULT	;228, e4000000
		dc.l A_DEFAULT	;229, e5000000
		dc.l A_DEFAULT	;230, e6000000
		dc.l A_DEFAULT	;231, e7000000
		dc.l A_DEFAULT	;232, e8000000
		dc.l A_DEFAULT	;233, e9000000
		dc.l A_DEFAULT	;234, ea000000
		dc.l A_DEFAULT	;235, eb000000
		dc.l A_DEFAULT	;236, ec000000
		dc.l A_DEFAULT	;237, ed000000
		dc.l A_DEFAULT	;238, ee000000
		dc.l A_DEFAULT	;239, ef000000
		dc.l A_DEFAULT	;240, f0000000
		dc.l A_DEFAULT	;241, f1000000
		dc.l A_DEFAULT	;242, f2000000
		dc.l A_DEFAULT	;243, f3000000
		dc.l A_DEFAULT	;244, f4000000
		dc.l A_DEFAULT	;245, f5000000
		dc.l A_DEFAULT	;246, f6000000
		dc.l A_DEFAULT	;247, f7000000
		dc.l A_DEFAULT	;248, f8000000
		dc.l A_DEFAULT	;249, f9000000
		dc.l A_DEFAULT	;250, fa000000
		dc.l A_DEFAULT	;251, fb000000
		dc.l A_DEFAULT	;252, fc000000
		dc.l A_DEFAULT	;253, fd000000
		dc.l A_DEFAULT	;254, fe000000
		dc.l A_DEFAULT	;255, ff000000


		CNOP 0,16
LevelB		dc.l $00000041	 ;  0, 00000000
		dc.l $00040041	 ;  1, 00040000
		dc.l $00080041	 ;  2, 00080000
		dc.l $000c0041	 ;  3, 000c0000
		dc.l $00100041	 ;  4, 00100000
		dc.l $00140041	 ;  5, 00140000
		dc.l $00180041	 ;  6, 00180000
		dc.l $001c0041	 ;  7, 001c0000
		dc.l $00200001	 ;  8, 00200000
		dc.l $00240001	 ;  9, 00240000
		dc.l $00280001	 ; 10, 00280000
		dc.l $002c0001	 ; 11, 002c0000
		dc.l $00300001	 ; 12, 00300000
		dc.l $00340001	 ; 13, 00340000
		dc.l $00380001	 ; 14, 00380000
		dc.l $003c0001	 ; 15, 003c0000
		dc.l $00400001	 ; 16, 00400000
		dc.l $00440001	 ; 17, 00440000
		dc.l $00480001	 ; 18, 00480000
		dc.l $004c0001	 ; 19, 004c0000
		dc.l $00500001	 ; 20, 00500000
		dc.l $00540001	 ; 21, 00540000
		dc.l $00580001	 ; 22, 00580000
		dc.l $005c0001	 ; 23, 005c0000
		dc.l $00600001	 ; 24, 00600000
		dc.l $00640001	 ; 25, 00640000
		dc.l $00680001	 ; 26, 00680000
		dc.l $006c0001	 ; 27, 006c0000
		dc.l $00700001	 ; 28, 00700000
		dc.l $00740001	 ; 29, 00740000
		dc.l $00780001	 ; 30, 00780000
		dc.l $007c0001	 ; 31, 007c0000
		dc.l $00800001	 ; 32, 00800000
		dc.l $00840001	 ; 33, 00840000
		dc.l $00880001	 ; 34, 00880000
		dc.l $008c0001	 ; 35, 008c0000
		dc.l $00900001	 ; 36, 00900000
		dc.l $00940001	 ; 37, 00940000
		dc.l $00980001	 ; 38, 00980000
		dc.l $009c0001	 ; 39, 009c0000
		dc.l $00a00001	 ; 40, 00a00000
		dc.l $00a40001	 ; 41, 00a40000
		dc.l $00a80001	 ; 42, 00a80000
		dc.l $00ac0001	 ; 43, 00ac0000
		dc.l $00b00001	 ; 44, 00b00000
		dc.l $00b40001	 ; 45, 00b40000
		dc.l $00b80001	 ; 46, 00b80000
		dc.l $00bc0051	 ; 47, 00bc0000
		dc.l $00c00001	 ; 48, 00c00000
		dc.l $00c40001	 ; 49, 00c40000
		dc.l $00c80001	 ; 50, 00c80000
		dc.l $00cc0001	 ; 51, 00cc0000
		dc.l $00d00001	 ; 52, 00d00000
		dc.l $00d40001	 ; 53, 00d40000
		dc.l $00d80001	 ; 54, 00d80000
		dc.l $00dc0051	 ; 55, 00dc0000
		dc.l K_MEM!5	    ; 56, 00f80000  (write protected)
		dc.l K_MEM!5	    ; 57, 00fc0000  (write protected)
		dc.l $00e80011	    ; 58, 00e80000
		dc.l $00ec0011	    ; 59, 00ec0000
		dc.l K_MEM+K_SIZE!5 ; 60, 00f00000  (write protected)
		dc.l K_MEM+K_SIZE!5 ; 61, 00f40000  (write protected)
		dc.l K_MEM!5	    ; 62, 00f80000  (write protected)
		dc.l K_MEM!5	    ; 63, 00fc0000  (write protected)


		CNOP 0,16
LevelBB 	dc.l $07000001	 ;  0, 00000000
		dc.l $07040001	 ;  1, 00040000
		dc.l $07080001	 ;  2, 00080000
		dc.l $070c0001	 ;  3, 000c0000
		dc.l $07100001	 ;  4, 00100000
		dc.l $07140001	 ;  5, 00140000
		dc.l $07180001	 ;  6, 00180000
		dc.l $071c0001	 ;  7, 001c0000
		dc.l $07200001	 ;  8, 00200000
		dc.l $07240001	 ;  9, 00240000
		dc.l $07280001	 ; 10, 00280000
		dc.l $072c0001	 ; 11, 002c0000
		dc.l $07300001	 ; 12, 00300000
		dc.l $07340001	 ; 13, 00340000
		dc.l $07380001	 ; 14, 00380000
		dc.l $073c0001	 ; 15, 003c0000
		dc.l $07400001	 ; 16, 00400000
		dc.l $07440001	 ; 17, 00440000
		dc.l $07480001	 ; 18, 00480000
		dc.l $074c0001	 ; 19, 004c0000
		dc.l $07500001	 ; 20, 00500000
		dc.l $07540001	 ; 21, 00540000
		dc.l $07580001	 ; 22, 00580000
		dc.l $075c0001	 ; 23, 005c0000
		dc.l $07600001	 ; 24, 00600000
		dc.l $07640001	 ; 25, 00640000
		dc.l $07680001	 ; 26, 00680000
		dc.l $076c0001	 ; 27, 006c0000
		dc.l $07700001	 ; 28, 00700000
		dc.l $07740001	 ; 29, 00740000
		dc.l $07780001	 ; 30, 00780000
		dc.l $077c0001	 ; 31, 007c0000
		dc.l $07800001	 ; 32, 00800000
		dc.l $07840001	 ; 33, 00840000
		dc.l $07880001	 ; 34, 00880000
		dc.l $078c0001	 ; 35, 008c0000
		dc.l $07900001	 ; 36, 00900000
		dc.l $07940001	 ; 37, 00940000
		dc.l $07980001	 ; 38, 00980000
		dc.l $079c0001	 ; 39, 009c0000
		dc.l $07a00001	 ; 40, 00a00000
		dc.l $07a40001	 ; 41, 00a40000
		dc.l $07a80001	 ; 42, 00a80000
		dc.l $07ac0001	 ; 43, 00ac0000
		dc.l $07b00001	 ; 44, 00b00000
		dc.l $07b40001	 ; 45, 00b40000
		dc.l $07b80001	 ; 46, 00b80000
		dc.l $07bc0001	 ; 47, 00bc0000
		dc.l $07c00001	 ; 48, 00c00000
		dc.l $07c40001	 ; 49, 00c40000
		dc.l $07c80001	 ; 50, 00c80000
		dc.l $07cc0001	 ; 51, 00cc0000
		dc.l $07d00001	 ; 52, 00d00000
		dc.l $07d40001	 ; 53, 00d40000
		dc.l $07d80001	 ; 54, 00d80000
		dc.l $07dc0001	 ; 55, 00dc0000
		dc.l $07e00001	 ; 56, 00e00000
		dc.l $07e40001	 ; 57, 00e40000
		dc.l $07e80001	 ; 58, 00e80000
		dc.l $07ec0001	 ; 59, 00ec0000
		dc.l $07f00001	 ; 60, 00f00000
		dc.l $07f40001	 ; 61, 00f40000
		dc.l $07f80005	 ; 62, 00f80000     write protected
		dc.l $07fc0005	 ; 63, 00fc0000     write protected


IDString:	dc.b	'mmu',0
		CNOP	0,4
mmu_EndMarker:

		END
