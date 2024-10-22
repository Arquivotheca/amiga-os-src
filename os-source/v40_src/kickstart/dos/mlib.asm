
	TTL "** MC68000 Machine code library **"
	SPC 4
******************************************************************
*                                                                *
*           (C) Copyright 1980 Tripos Research Group             *
*                                                                *
*               University of Cambridge                          *
*               Computer Laboratory                              *
*                                                                *
******************************************************************
	SPC 14
******************************************************************
*                                                                *
*   This is the Machine library for BCPL on the MC68000          *
*   This version still has multiply/divide in it pending change  *
*                                                                *
******************************************************************
	SPC 4
	TTL "** MC68000 Machine code library - Control Blocks **"
*
*
* Symbol definitions and data structures
*
*
	IFND KLUDGE
LIBWORD   EQU    23456               Marks library routines
SECWORD   EQU    12345               Marks a BCPL section
*
*
* Register definition
*
Z         EQUR    A0                 Constant Zero
P         EQUR    A1                 BCPL frame ptr
G         EQUR    A2                 BCPL global ptr
L         EQUR    A3                 Temp reg in save/rtn routine
B         EQUR    A4                 Addr of called routine
S         EQUR    A5                 Save routine addr
R         EQUR    A6                 Return addr
** SP        EQUR    A7                 System stack ptr

*
*
* Coroutine symbols
*
C_LINK    EQU        0               Link to next co-routine
C_CLLR    EQU        4               Caller coroutine
C_SEND    EQU        8               Stack end - 50
C_RESP    EQU       12               Resumption ptr
C_FUNC    EQU       16               Function
*                   20               PC dump - dummy
C_RTRN    EQU       24               Return link for stop

	PAGE
*
* Macro definitions
*
SECT    MACRO  arg1
**        CNOP   0,4
**        DC.L   LIBWORD
**        DC.B   7,\1
	ENDM

	ENDC


	PAGE
*
*	XDEFS
*
	XDEF	@comparedate
	XREF	@CompareDates

*	XDEF	MLIB
*	XDEF	MLIB_SEG

	PAGE
	TTL "** MC68000 Machine code library - routines **"

	IFND KLUDGE
	INCLUDE	"libhdr.i"
	ENDC
*
*
*MLIB_SEG  DC.L	     0			; no next segment
*MLIB      DC.L       (MEND-MLIB)/4
*
* First the global interface routines to MULTIPLY, DIVIDE and REMAINDER
* Evil fixed offsets from S! REJ
* these are pure BCPL routines
*
MULTIPLY
	JSR	$10(S)
	JMP	(R)
*
DIVIDE	JSR	$12(S)
	JMP	(R)
*
REMAINDER
	JSR	$12(S)
	MOVE.L	D2,D1
	JMP	(R)

	 SPC  4
******************************************************************
*                  Aptovec( Function, Upperbound )               *
* Strange funk BCPL magic, I won't touch - REJ                   *
*								 *
******************************************************************
*
	SECT        <'aptovec'>
*
APTOVEC	MOVEA.L	D1,B		; Save routine entry point
*
	MOVE.L	P,D1		; Vector base is D1
	MOVE.L	P,D0		; save byte version of sddress
	LSR.L	#2,D1
	SUBQ.L	#3,D1		; allow for aptovec's savespace
*
	MOVE.L	D2,D3
	ASL.L	#2,D3		; Size of required vector in bytes
	LEA	4(P,D3.L),P	; reset P pointer to leave space for vector
*
	MOVEM.L	D1/D2,(P)	; args for callee on stack as well as regs
*
*
	MOVE.L	B,-4(P)		; Write in the base
	MOVE.L	-8(Z,D0.L),-8(P)	; return address
	MOVE.L	-12(Z,D0.L),-12(P)	; old p pointer
*
	JMP	(B)		; enter the called function
*
	PAGE
******************************************************************
*                                                                *
*                  Getbyte(vector, offset)                       *
*                                                                *
******************************************************************

	SECT        <'getbyte'>

GETBYTE	ASL.L	#2,D1
	ADD.L	D1,D2		; D2th byte on MC vector
	MOVEQ	#0,D1
	MOVE.B	0(Z,D2.L),D1	; Fetch byte
	JMP	(R)

*****************************************************************
*                                                               *
*                   Putbyte(vector,offset,byte)                 *
*                                                               *
*****************************************************************
*
	SECT         <'putbyte'>
*
PUTBYTE	ASL.L	#2,D1
	ADD.L	D1,D2
	MOVE.B	D3,0(Z,D2.L)	; Put the given byte there
	JMP	(R)

	PAGE
******************************************************************
*                                                                *
*                  Get2bytes(vector, wordoffset)                 *
*                                                                *
******************************************************************

	SECT        <'get2byt'>

GET2BYTES
	ASL.L	#2,D1
	ASL.L	#1,D2
	ADD.L	D1,D2		; D2th byte on MC vector
	MOVEQ	#0,D1
	MOVE.W	0(Z,D2.L),D1	; Fetch word
	JMP	(R)

*****************************************************************
*                                                               *
*                   Put2bytes(vector,wordoffset,word)           *
*                                                               *
*****************************************************************
*
	SECT         <'put2byt'>
*
PUT2BYTES
	ASL.L	#2,D1
	ASL.L	#1,D2
	ADD.L	D1,D2
	MOVE.W	D3,0(Z,D2.L)	; Put the given word there
	JMP	(R)

******************************************************************
*                                                                *
*                  Gbytes(ba, size)                              *
*                                                                *
******************************************************************

	SECT        <'gbytes '>

GBYTES	MOVEA.L	D1,B		; B = MC addr of next byte
	MOVEQ	#0,D1
*
GBYTES1	LSL.L	#8,D1
	ADD.B	(A4)+,D1	; Add in next byte
	SUBQ.B	#1,D2
	BNE.S	GBYTES1
*
	JMP	(R)

*****************************************************************
*                                                               *
*                   Pbytes(ba,size,word)                        *
*                                                               *
*****************************************************************
*
	SECT         <'pbytes '>
*
PBYTES	MOVEA.L	D1,B		; B = MC addr of first byte
*
PBYTES1	MOVE.B	D3,-1(B,D2.L)	; Move a byte (last first)
	LSR.L	#8,D3		; Position word for next
	SUBQ.B	#1,D2
	BNE.S	PBYTES1		; Continue if necessary
*
	JMP	(R)


	PAGE
****************************************************************
*                                                              *
*                   Level()                                    *
*                                                              *
****************************************************************
*
	 SECT         <'level  '>
*
LEVEL	MOVE.L	-12(P),D1
	JMP	(R)
*
****************************************************************
*                                                              *
*                   Longjump(stackp,label)                     *
*                                                              *
****************************************************************
*
	SECT          <'longjum'>
*
LONGJUMP
	MOVEA.L	D1,P
	MOVEA.L	-4(P),B
	JMP	0(Z,D2.L)

	PAGE
	TTL "** MC6800 Machine code library - Co-routine mechanism **"
***************************************************************
*                                                             *
*                     Createco( Fn, Stsize )                  *
*                                                             *
***************************************************************
*
	SECT          <'creatco'>
*
CREATECO
	MOVE.L	D2,D1
	MOVEA.L	G_GETVEC(G),B
	MOVEQ	#28,D0		; Standard BCPL call
	JSR	(S)		; Get the store
	TST.L	D1
	BEQ.S	CONOS		; No store - return with zero
	MOVE.L	D1,D2
	ADD.L	4(P),D2		; BCPL stack end - ARG2
	ASL.L	#2,D2		; D2 = MC end
	MOVE.L	D1,D0
	ASL.L	#2,D1		; D1 = MC stack base
	MOVEA.L	D1,B
*
CR1	CLR.L	(A4)+		; = B
	CMP.L	B,D2		; Have we zeroed the stack?
	BGE.S	CR1
*
	MOVE.L	G_STACKBASE(G),D3	; Get current coroutine caller
	MOVE.L	D3,C_CLLR(Z,D1.L)	; Set the caller
	ASL.L	#2,D3			; MC current coroutine ptr
	MOVE.L	0(Z,D3.L),0(Z,D1.L)	; Link it in
	MOVE.L	D0,0(Z,D3.L)		; After the current
	MOVE.L	P,C_RESP(Z,D3.L)	; Resumption ptr
	MOVE.L	(P),C_FUNC(Z,D1.L)	; Set Fn
*
	SUBI.L	#160,D2			; Allow 40 words safety margin
	MOVE.L	D2,C_SEND(Z,D1.L)	; Set the stack end (as byte address)
	MOVE.L	D0,G_STACKBASE(G)	; STACKBASE
	MOVEA.L	D1,P			; New stack
	MOVE.L	D0,D1			; D1 = BCPL ptr to new cortn
*
CR2	MOVEQ	#32,D0			; $(
	MOVEA.L	G_COWAIT(G),B
	JSR	(S)			; D1 := COWAIT(D1))
	MOVEA.L	C_FUNC(P),B
	MOVEQ	#32,D0
	JSR	(S)			; D1 := Fn(D1)
	BRA.S	CR2			; $) REPEAT
*
*
CONOS	JMP	(R)
*
	PAGE
*****************************************************************
*                                                               *
*                    Deleteco( cptr )                           *
*                                                               *
*****************************************************************
*
*
	SECT          <'deletco'>
*
DELETECO
	MOVE.L	D1,D0
	ASL.L	#2,D1			; MC Coroutine ptr
	TST.L	C_CLLR(Z,D1.L)		; Still active
	BNE.S	COERR			; Yes
	MOVE.L	G_STACKBASE(G),D3	; Locate the coroutine chain
*
* Descend to the root coroutine
*
DC1	MOVE.L	D3,D2
	ASL.L	#2,D3
	MOVE.L	C_CLLR(Z,D3.L),D3	; Get caller
	BGT.S	DC1			; Root = -1
*
* Locate the coroutine on the chain
*
DC2	ASL.L	#2,D2
	MOVE.L	D2,D3
	MOVE.L	0(Z,D2.L),D2		; Next coroutine
	BEQ.S	COERR			; End of chain
	CMP.L	D2,D0			; Found it?
	BNE.S	DC2			; Keep going
*
	MOVE.L	0(Z,D1.L),0(Z,D3.L)	; Unlink it
	MOVE.L	D0,D1			; BCPL coroutine ptr
	MOVEA.L	G_FREEVEC(G),B
	MOVEQ	#12,D0			; BCPL call
	JSR	(S)			; Free the coroutine
	MOVEQ	#-1,D1			; Return TRUE
DC3	JMP	(R)			; Leave
*
COERR	MOVEQ	#0,D1			; Return FALSE
	BRA.S	DC3
*
	PAGE
******************************************************************
*                                                                *
*                         Callco( Cptr, Arg )                    *
*                                                                *
******************************************************************
*
*
	SECT           <'callco '>
*
CALLCO	MOVE.L	D1,D3
	ASL.L	#2,D3
*++       TST.L	C_CLLR(Z,D3.L)      Already active?
*++       BNE            COERR               Can't call recursivly
	MOVE.L	G_STACKBASE(G),D4
	MOVE.L	D4,C_CLLR(Z,D3.L)	; Activate new coroutine
	ASL.L	#2,D4			; MC old coroutine ptr
*
COENTER	MOVE.L	D1,G_STACKBASE(G)	; Set up stackbase
	MOVE.L	D2,D1			; Arg in D1
	MOVE.L	P,C_RESP(Z,D4.L)	; Save resumption ptr
	MOVEA.L	C_RESP(Z,D3.L),P	; Get new RESP
	JMP	(R)
*
*
	PAGE
******************************************************************
*                                                                *
*                         Resumeco( Cptr, Arg )                  *
*                                                                *
******************************************************************
*
	SECT            <'resumco'>
*
RESUMECO
	MOVE.L	G_STACKBASE(G),D4	; Get the current cortn
	CMP.L	D4,D1			; Compare with given cortn
	BEQ.S	RC1			; J if resuming current cortn
*
	MOVE.L	D1,D3
	ASL.L	#2,D3			; MC new coroutine ptr
	ASL.L	#2,D4
**+       TST.L	C_CLLR(Z,D3.L)    New - already active?
**+       BNE             COERROR
**+       TST.L	C_CLLR(Z,D4.L)    Old = root?
**+       BLT             COERROR
	MOVE.L	C_CLLR(Z,D4.L),C_CLLR(Z,D3.L)	; Activate new cortn.
	CLR.L	C_CLLR(Z,D4.L)			; Deactivate old
	BRA.S	COENTER
*
RC1	MOVE.L	D2,D1		; D1 = arg
	JMP	(R)		; Return from RESUMECO
*
*
******************************************************************
*                                                                *
*                         Cowait( Arg )                          *
*                                                                *
******************************************************************
*
	SECT          <'cowait '>
*
COWAIT	MOVE.L	G_STACKBASE(G),D4
	ASL.L	#2,D4			; MC old coroutine
	MOVE.L	C_CLLR(Z,D4.L),D3	; Get caller
**+       BLE           COERROR
	MOVE.L	D3,G_STACKBASE(G)	; Set up stackbase
	ASL.L	#2,D3			; MC new coroutine ptr
	CLR.L	C_CLLR(Z,D4.L)		; Deactivate old coroutine
	MOVE.L	P,C_RESP(Z,D4.L)	; Save resumption point
	MOVEA.L	C_RESP(Z,D3.L),P	; Get new resumption ptr
*
	JMP	(R)			; Resume execution of new cortn
*
* CompareDate( Adate, Bdate )
*
* Compares the dates pointed at by the two arguments. Returns
* <0, 0 or >0 depending on how the dates compare.
* C version - BCPL is in fakeblib.asm
*

@comparedate:
	bsr	@CompareDates	; returns backwards returncode
	neg.l	d0		; should we compress to 1/-1? 
	rts			; No, checked BCPL source, it uses < & >.

*          PAGE
*          TTL "** MC68000 Machine code library - Global list **"
* Global initialisation list
*
*          CNOP          0,4
*          DC.L	0                               The end of the list
*          DC.L	G_MULTIPLY/4,(MULTIPLY-MLIB)
*          DC.L	G_DIVIDE/4,(DIVIDE-MLIB)
*          DC.L	G_REMAINDER/4,(REMAINDER-MLIB)
*          DC.L	G_GBYTES/4,(GBYTES-MLIB)
*          DC.L	G_PBYTES/4,(PBYTES-MLIB)
*          DC.L	G_GETBYTE/4,(GETBYTE-MLIB)
*          DC.L	G_PUTBYTE/4,(PUTBYTE-MLIB)
*          DC.L	G_GET2BYTES/4,(GET2BYTES-MLIB)
*          DC.L	G_PUT2BYTES/4,(PUT2BYTES-MLIB)
*          DC.L	G_LEVEL/4,(LEVEL-MLIB)
*          DC.L	G_LONGJUMP/4,(LONGJUMP-MLIB)
*          DC.L	G_APTOVEC/4,(APTOVEC-MLIB)
*          DC.L	G_CREATECO/4,(CREATECO-MLIB)
*          DC.L	G_DELETECO/4,(DELETECO-MLIB)
*          DC.L	G_COWAIT/4,(COWAIT-MLIB)
*          DC.L	G_RESUMECO/4,(RESUMECO-MLIB)
*          DC.L	G_CALLCO/4,(CALLCO-MLIB)
*          DC.L	99                           Highest referenced global
			; FIX!
MEND      END


