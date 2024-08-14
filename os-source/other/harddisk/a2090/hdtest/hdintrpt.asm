	LLEN	75
	TTL	AMIGA Hard Disk Interrupt Server
	IDNT	"HDInt"
	NOLIST
	INCLUDE "exec/types.i"
	INCLUDE "exec/nodes.i"
	INCLUDE	"exec/ports.i"
	INCLUDE	"hardware/custom.i"
	INCLUDE	"hardware/intbits.i"
	INCLUDE "my-macros.asm"
	LIST
	PAGE
	CODE
		XDEF	_hdintr
		XREF	_intPort
		XREF	_AbsExecBase
		XREF	_intreq
		EXT_SYS	PutMsg
*		EXT_DATA _intreq

*_hdintr	MOVE.L	$0(A1),A4		Get hd board base address
_hdintr		MOVEM.L	A0/A1/A4,-(A7)		Preserve registers
		MOVE.L	#$E80000,A0		Point to board base address
		BTST.B	#7,$20(A0)		See if INT2 Pending
		BNE.S	hdexnm			If not set, exit, not mine

		MOVE.B	#$F7,$20(A0)		Toggle controller's INT2 bit

*		Send back message indicating interrupt has occured
		LEA.L	_intPort,A0
		LEA.L	hdmsg,A1
		MOVEA.L	_AbsExecBase,A4
		SYS	PutMsg(A4)
*
		MOVE.W	#INTF_PORTS,_intreq	Clear PORTIA's INT2 bit
*
*		Now clear the ZERO condition code so that
*		the interrupt handler doesn't call the next
*		interrupt server.
*
		ANDI	#$FB,CCR		Clear ZERO flag
		BRA.S	hdexit			Now exit
*
*		This exit point sets the ZERO condition code
*		so the interrupt handler will try the next server
*		in the interrupt chain
*
hdexnm		ORI	#$04,CCR		Set ZERO condition code
*
hdexit		MOVEM.L	(A7)+,A0/A1/A4		Restore registers
		RTS
		DATA
hdmsg		DC.L	0,0
		DC.B	NT_MESSAGE,0,0
		DC.L	0
		DC.L	0
		DC.W	MN_SIZE
