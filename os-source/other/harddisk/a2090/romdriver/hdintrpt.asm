*******************************************************************************
*
*	Source Control
*	--------------
*	$Header: hdintrpt.asm,v 34.13 88/01/21 18:05:04 bart Exp $
*
*	$Locker:  $
*
*	$Log:	hdintrpt.asm,v $
*   Revision 34.13  88/01/21  18:05:04  bart
*   compatible with disk based / binddriver useage
*   
*   Revision 34.12  87/12/04  19:14:13  bart
*   checkpoint
*   
*   Revision 34.11  87/12/04  12:08:55  bart
*   checkpoint before adding check for existing dosname on eb_mountlist
*   
*   Revision 34.10  87/10/26  16:31:33  bart
*   checkpoint
*   
*   Revision 34.9  87/10/25  13:20:21  bart
*   check if it is for THIS handler...
*   
*   Revision 34.8  87/10/14  15:02:57  bart
*   10-13 rev 1
*   
*   Revision 34.7  87/10/14  14:16:12  bart
*   beginning update to cbm-source.10.13.87
*   
*   Revision 34.6  87/07/08  14:22:23  bart
*   *** empty log message ***
*   
*   Revision 34.5  87/07/08  14:01:10  bart
*   y
*   
*   Revision 34.4  87/06/11  15:48:17  bart
*   working autoboot 06.11.87 bart
*   
*   Revision 34.3  87/06/03  10:59:16  bart
*   checkpoint
*   
*   Revision 34.2  87/05/31  16:36:01  bart
*   chickpoint
*   
*   Revision 34.1  87/05/29  19:39:37  bart
*   checkpoint
*   
*   Revision 34.0  87/05/29  17:39:57  bart
*   added to rcs for updating
*   
*
*******************************************************************************

	LLEN	130
	TTL	AMIGA Hard Disk Interrupt Server
	NOLIST

	SECTION	section

	IFND	EXEC_TYPES_I
	INCLUDE 'exec/types.i'
	ENDC
	IFND	EXEC_INTERRUPTS_I
	INCLUDE	"exec/interrupts.i"
	ENDC
	IFND	EXEC_LISTS_I
	INCLUDE 'exec/lists.i'
	ENDC
	IFND	EXEC_NODES_I
	INCLUDE 'exec/nodes.i'
	ENDC
	IFND	EXEC_PORTS_I
	INCLUDE 'exec/ports.i'
	ENDC
	IFND	EXEC_LIBRARIES_I
	INCLUDE 'exec/libraries.i'
	ENDC
	IFND	EXEC_IO_I
	INCLUDE 'exec/io.i'
	ENDC
	IFND	EXEC_DEVICES_I
	INCLUDE 'exec/devices.i'
	ENDC
	IFND	EXEC_TASKS_I
	INCLUDE 'exec/tasks.i'
	ENDC
	IFND	EXEC_MEMORY_I
	INCLUDE 'exec/memory.i'
	ENDC
	IFND	EXEC_EXECBASE_I
	INCLUDE 'exec/execbase.i'
	ENDC
	IFND	EXEC_ABLES_I
	INCLUDE 'exec/ables.i'
	ENDC
	IFND	EXEC_ERRORS_I
	INCLUDE 'exec/errors.i'
	ENDC
	IFND	EXEC_ALERTS_I
	INCLUDE 'exec/alerts.i'
	ENDC
	IFND	HARDWARE_CUSTOM_I
	INCLUDE	"hardware/custom.i"
	ENDC
	IFND	HARDWARE_intbits_I
	INCLUDE	"hardware/intbits.i"
	ENDC

	INCLUDE 'internal.i'
	INCLUDE 'messages.i'
	INCLUDE "mymacros.asm"
	LIST
	PAGE

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

*BART	-- IF NOT FOR THIS SPECIFIC HANDLER, PASS IT ON
*		BTST.B  #HDB_ACTIVE,HD_FLAGS(A1) check if it is for THIS handler
*		BEQ     hdexnm                   If not set, exit, not mine

		MOVE.B	#0,HDINTACK(A0)		Toggle controller's INT2 bit

*BART	-- ONLY HANDLE ONCE
*		BCLR.B  #HDB_ACTIVE,HD_FLAGS(A1) Signal unit not active

		BTST.B	#HDB_SCSI,HD_FLAGS(A1)	; SCSI active?
		BEQ.S	SignalTASK

		MOVE.B	#SC_STATUS,SCSIADDR(A0)	; Point to SCSI Status reg
		MOVE.B	SCSIIND(A0),D0		; Read stat to clear interrupt

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
		BRA.S	hdexit			Now exit
*
*		This exit point sets the ZERO condition code
*		so the interrupt handler will try the next server
*		in the interrupt chain
*
hdexnm		MOVEQ	#0,D0			Set ZERO condition code
*
hdexit		RTS

	END
