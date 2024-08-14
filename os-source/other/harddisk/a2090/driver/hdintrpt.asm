	LLEN	130
	TTL	AMIGA Hard Disk Interrupt Server
	NOLIST
	INCLUDE 'exec/types.i'
	INCLUDE	"exec/interrupts.i"
	INCLUDE 'exec/lists.i'
	INCLUDE 'exec/nodes.i'
	INCLUDE 'exec/ports.i'
	INCLUDE 'exec/libraries.i'
	INCLUDE 'exec/io.i'
	INCLUDE 'exec/devices.i'
	INCLUDE 'exec/tasks.i'
	INCLUDE 'exec/memory.i'
	INCLUDE 'exec/execbase.i'
	INCLUDE 'exec/ables.i'
	INCLUDE 'exec/errors.i'
	INCLUDE 'exec/alerts.i'
	INCLUDE	"hardware/custom.i"
	INCLUDE	"hardware/intbits.i"

	INCLUDE 'internal.i'
	INCLUDE 'messages.i'
	INCLUDE "mymacros.asm"
	LIST
	PAGE
	CODE
	XDEF	hdintr
	XREF	intPort
	XREF	_intreq
	EXTERN_LIB Signal
	EXTERN_LIB PutMsg

*	A1 should be pointing to the device structure upon entry!

hdintr:		MOVE.L	HD_SYSLIB(A1),A6	Get pointer to System
		MOVE.L	HD_BASE(A1),A0		Point to board base address

*LCE		BTST.B	#HDB_NO512,HD_FLAGS(A1)	; KONAN HARDWARE PRESENT ?
*LCE		BNE.S	NoKONAN		

		BTST.B	#7,HDSTAT(A0)		See if INT2 Pending
		BEQ.S	hdexnm			If not set, exit, not mine
		MOVE.B	#0,HDINTACK(A0)		Toggle controller's INT2 bit
		BTST.B	#HDB_SCSI,HD_FLAGS(A1)	; SCSI active?
		BEQ.S	SignalTASK
		MOVE.B	#SC_STATUS,SCSIADDR(A0)	; Point to SCSI Status reg
		TST.B	SCSIIND(A0)		; Read stat to clear int

*LCE		BRA.S	SignalTASK
*LCENoKONAN:	
*LCE		BTST.B	#HDB_SCSI,HD_FLAGS(A1)	; SCSI active?
*LCE		BEQ.S	hdexnm
*LCE		BTST.B	#7,SCSIAUX(A0)		; Read stat to clear int
*LCE		BEQ.S	hdexnm			; Not SCSI interrupt
*LCE		MOVE.B	#SC_STATUS,SCSIADDR(A0)	; Point to SCSI Status reg
*LCE		TST.B	SCSIIND(A0)		; Read stat to clear int

*		;------ Signal the task that an interrupt has occured
*		;---!!! Need to parameterize the signal number and TCB ptr
SignalTASK:	MOVE.L	#HDF_CMDDONE,D0
		LEA	HD_TCB(A1),A1
		SYS	Signal(A6)

*
*		Now clear the ZERO condition code so that
*		the interrupt handler doesn't call the next
*		interrupt server.
*
		MOVEQ	#1,D0			Clear ZERO flag
		RTS				Now exit

*		This exit point sets the ZERO condition code
*		so the interrupt handler will try the next server
*		in the interrupt chain
*
hdexnm:		MOVEQ	#0,D0			Set ZERO condition code
		RTS
	END
