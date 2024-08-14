	LLEN	130
	TTL	Routine to add hard disk node to the Amiga Dos device list
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
	INCLUDE 'libraries/dosextens.i'

	INCLUDE 'hddisk.i'
	INCLUDE	'asmsupp.i'
	LIST
	INCLUDE 'internal.i'
	PAGE
	CODE
	XDEF	HDAddDevice
	EXTERN_LIB OpenLibrary
	EXTERN_LIB CloseLibrary

	SECTION section

*	A1 should be pointing to the device structure upon entry!

HDAddDevice	MOVEM.L	A0/A1/A4,-(A7)		Preserve registers
*		;------ Initialize structures for Amiga Dos
		LEA.L	devSizeInfo,A0		; Get addr. of device info
		MOVE.L	A0,D0			; Convert to BPTR
		ROR.L	#2,D0
		MOVE.L	D0,HD_SI+SI_DS_INFO(A2)	; Store BPTR to device size info

		LEA.L	driverName,A0		; Get addr. of driver name
		MOVE.L	A0,D0			; Convert to BPTR
		ROR.L	#2,D0
		MOVE.L	D0,HD_SI+SI_DEV_NAME(A2); Store BPTR to driver name

		LEA.L	HD_SI(A2),A0		; Get addr. startup info
		MOVE.L	A0,D0			; Convert to BPTR
		ROR.L	#2,D0
		MOVE.L	D0,HD_DEV_NODE+AD_STARTUP(A2); Store BPTR to startup

		LEA.L	devName,A0		; Get device name address
		MOVE.L	A0,D0			; Convert to BPTR
		ROR.L	#2,D0
		MOVE.L	D0,HD_DEV_NODE+AD_DEV_NAME(A2); Store BPTR to startup

		LEA.L	HDDosName,A1		; Get library name
		CLR	D0
		CALLSYS	OpenLibrary		; Open the Amiga Dos library
		MOVE.L	D0,A4

		LEA.L	DF0name,A0		; Point to "DF0:"
		MOVE.L	A0,D1
		LINKLIB	_LVODeviceProc,A4	; Find filesystem task
		MOVE.L	D0,A0
		LEA.L	-TC_SIZE(A0),A0
		MOVE.L	pr_SegList(A0),D0	; Get seglist BPTR
		ROL.L	#2,D0			; Convert to APTR
		MOVE.L	D0,A0
		MOVE.L	12(A0),D0		; Get required seglist BPTR
		MOVE.L	D0,AD_SEGLIST+HD_DEV_NODE(A2); and store in device node

		MOVE.L	dl_Root(A4),A0		; Point to RootNode
		MOVE.L	rn_Info(A0),D0		; Get BPTR to DosInfo structure
		ROL.L	#2,D0			; Convert to APTR
		MOVE.L	D0,A0
		LEA.L	di_DevInfo(A0),A0	; Point to Device Info list

FindEnd:	MOVE.L	(A0),D0			; Find end of list
		BEQ.S	EndFound		; If 0, end of list found
		ROL.L	#2,D0			; Convert to APTR
		MOVE.L	D0,A0
		BRA.S	FindEnd

EndFound:	LEA.L	HD_DEV_NODE(A2),A1	; Get address of device node
		MOVE.L	A1,D0			; Convert to BPTR
		ROR.L	#2,D0
		MOVE.L	D0,(A0)			; Add to device info list

*LCE		MOVE.L	(A0),D0			; Get BPTR to list
*LCE		LEA.L	HD_DEV_NODE(A2),A1	; Get address of device node
*LCE		MOVE.L	D0,AD_NEXT(A1)		; Have this node point to list
*LCE		MOVE.L	A1,D1			; Convert to BPTR
*LCE		ROR.L	#2,D1
*LCE		MOVE.L	D1,(A0)			; Put this node in list

		MOVE.L	A4,A1			; Now close the library
		CALLSYS	CloseLibrary

		MOVEM.L	(A7)+,A0/A1/A4		Restore registers
		RTS

HDDosName	DOSNAME
		CNOP	0,4	; LONG WORD ALIGN

*-- Disk size information structure

devSizeInfo:
	DS.L	11		; Number of longs in structure
	DS.L	128		; Number of longs in block
	DS.L	0		; Sector origin
	DS.L	4		; Number of surfaces
	DS.L	1		; Number of sectors per block
	DS.L	17		; Number of blocks per track
	DS.L	3		; Number of reserved blocks at start
	DS.L	0		; Pre-allocation factor
	DS.L	0		; Interleave factor
	DS.L	0		; Starting cylinder number
	DS.L	100		; Ending cylinder number
	DS.L	5		; Number of cache blocks

		CNOP	0,4	; LONG WORD ALIGN
driverName:	DC.B	13	; 14 characters in name, including trailing 0
		DC.B	'hddisk.device',0

		CNOP	0,4	; LONG WORD ALIGN
devName:	DC.B	3	; 4 characters in name, including trailing 0
		DC.B	'DH1',0	;

		CNOP	0,4	; LONG WORD ALIGN
DF0name:	DC.B	'DF0:',0

	END
