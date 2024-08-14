*
* C initial startup procedure under AmigaDOS
*
* Use the following command line to make c.o
* asm -u -iINCLUDE: c.a
*
        INCLUDE "exec/types.i"
        INCLUDE "exec/alerts.i"
        INCLUDE "exec/nodes.i"
        INCLUDE "exec/lists.i"
        INCLUDE "exec/ports.i"
        INCLUDE "exec/libraries.i"
        INCLUDE "exec/tasks.i"
        INCLUDE "exec/memory.i"
        INCLUDE "exec/execbase.i"
	INCLUDE "exec/resident.i"
	INCLUDE	"exec/macros.i"
        INCLUDE "libraries/dos.i"
        INCLUDE "libraries/dosextens.i"
        INCLUDE "workbench/startup.i"
        INCLUDE "dos/dos_lib.i"
        INCLUDE "dos/exall.i"
	INCLUDE "utility/hooks.i"

	INCLUDE	"internal/librarytags.i"

	INCLUDE "romdisk_rev.i"

MEMFLAGS	EQU	MEMF_CLEAR+MEMF_PUBLIC
AbsExecBase	EQU	4

        xref    _LinkerDB               * linker defined base value
        xref    __BSSBAS                * linker defined base of BSS
        xref    __BSSLEN                * linker defined length of BSS

        section text,code

	BASEREG a4

	XREF	_SysBase

        xref    @start                   * Name of C program to start with.
	xref	@dbwrite

	xdef	@do_match
*
	dc.l	8	; BPTR
Handler:
	dc.l	0	; Next
	; C version
	move.l	AbsExecBase.W,a6
	moveq	#pr_MsgPort,D0
	add.l	ThisTask(a6),D0
	move.l	d0,a0
	JSRLIB	GetMsg			* get initial packet
	move.l	d0,a0
	move.l	LN_NAME(a0),a3		* ptr to DosPacket

        move.l  #MEMF_CLEAR,d1
        move.l  #__BSSLEN,d0            * get length of BSS in longwords
	lsl.l	#2,d0			* into bytes!
	lea	_LinkerDB,a4
	lea	__BSSBAS,a0
	sub.l	a4,a0
	add.l	a0,d0			* add in initialized data space
	JSRLIB	AllocMem		* get global storage (All 0's!)
	move.l	d0,a4			* ptr to mem
	tst.l	d0
	beq.s	exit			* probably not safe!!!

        move.l  A6,_SysBase(a4)

	;-- pass parameter in a0
	move.l	a3,a0
        jsr     @start(PC)              * call C entrypoint
*        moveq.l #0,d1                   * set successful status (BCPL)
	moveq	#0,d0			* C
exit:
	rts

	;-- failed allocation - exit
error_exit:
	move.l	dp_Link(a3),a1		* message
	move.l	dp_Port(a3),a0		* "reply" port
	JSRLIB	PutMsg
	moveq	#-1,d1
	bra.s	exit

*
* Assembler routine for exall to call hook function
*
@do_match:
	movem.l	d0/a2/a3,-(a7)	; tricky!
	move.l	a7,a2		; points to data (from d0)
	move.l	eac_MatchFunc(a0),a0	; hook structure
	move.l	h_Entry(a0),a3
	jsr	(a3)		; a0=ptr to hook, a1=pointer to param,
				; a2=ptr to object
	movem.l	(a7)+,d1/a2/a3	; don't hit d0!
	rts
*

*
Entry:
	movem.l	d2-d7/a2-a6,-(sp)

	moveq.l	#OLTAG_DOS,d0
	JSRLIB	TaggedOpenLibrary
	move.l	d0,a6			; dos.library now in a6...

	lea	ROM(pc),a0		; Get name
	move.l	a0,d1

	move.l	#DLT_DEVICE,d2		; Device
	JSRLIB	MakeDosEntry		; Make the entry
	tst.l	d0			; Check it
	beq.s	error			; If not OK, exit...

	move.l	d0,a0

	lea	Handler(pc),a1		; Get handler
	move.l	a1,d0			; Get into d0
	lsr.l	#2,d0			; into a BPTR
	move.l	d0,dol_SegList(a0)	; My "seglist"

	move.l	#4000,dol_StackSize(a0)	; Stack
	move.l	#10,dol_Priority(a0)	; Priority
	move.l	#-1,dol_GlobVec(a0)	; GlobalVector (none)

	move.l	a0,d1
	JSRLIB	AddDosEntry

	lea	ROMDev(pc),a0		; Get device name
	move.l	a0,d1			; Set it up
	moveq.l	#0,d2
	JSRLIB	GetDeviceProc
	move.l	d0,d1
	beq.s	error
	JSRLIB	FreeDeviceProc

error:	move.l	a6,a1			; Close up again...
	JSRLIB	CloseLibrary

	movem.l	(sp)+,d2-d7/a2-a6
	moveq.l	#0,d0
	rts
*
	XREF	_end
romtag:
	DC.W	RTC_MATCHWORD		;(50) UWORD RT_MATCHWORD
	DC.L	romtag			;(52) APTR  RT_MATCHTAG
	DC.L	_end			;(56) APTR  RT_ENDSKIP
	DC.B	RTF_AFTERDOS		;(5A) UBYTE RT_FLAGS
	DC.B	VERSION			;(5B) UBYTE RT_VERSION
	DC.B	NT_UNKNOWN		;(5C) UBYTE RT_TYPE
	DC.B	-124			;(5D) BYTE  RT_PRI
	DC.L	RAMNAME			;(5E) APTR  RT_NAME
	DC.L	idtag			;(62) APTR  RT_IDSTRING
*	DC.L	Seglist			;(66) APTR  RT_INIT
	DC.L	Entry			;(66) APTR  RT_INIT
					;(6A) LABEL RT_SIZE

ROM:	dc.b	'ROM',0
ROMDev:	dc.b	'ROM:',0
RAMNAME: dc.b	'rom-handler',0
idtag:	 VSTRING

        END
