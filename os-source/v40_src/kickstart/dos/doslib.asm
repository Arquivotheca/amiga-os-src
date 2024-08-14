	PLEN 55
	TTL    "*** TRIPOS Kernel for the CAPC ***"
******************************************************************
*                                                                *
* This version a major rewrite for the CAPC by TJK Jan 85        *
* (C) Copyright 1985 Metacomco, 26 Portland Square, BRISTOL, UK  *
* All rights reserved                                            *
*                                                                *
* This is based to a small extent on the original TRIPOS kernel  *
* for the 68000 written by Martin Richards at the University of  *
* Cambridge                                                      *
*                                                                *
******************************************************************
	SPC 4
******************************************************************
******************************************************************
*****        **       *****  ****       ****      ****     *******
********  *****  ***   ****  ****  ***   **   **   **   **  ******
********  *****  ****  ****  ****  ****  **  ****  **   **********
********  *****  ***   ****  ****  ***   **  ****  ***   *********
********  *****       *****  ****       ***  ****  ****   ********
********  *****  **  ******  ****  ********  ****  *****   *******
********  *****  ***  *****  ****  ********  ****  ******   ******
********  *****  ****  ****  ****  ********   **   **  **   ******
********  *****  *****  ***  ****  *********      ****     *******
******************************************************************
******************************************************************
	SPC 4
******************************************************************
*                                                                *
*        A 32 bit kernel for TRIPOS for the MC68000 system       *
*                                                                *
*                                                                *
******************************************************************
	PAGE
	SPC 4
* Register usage when in BCPL compiled code.
*
*      D0       work reg - used to hold the stack increment
*                          in a procedure call
*      D1       the first argument and result register
*      D2-D4    the second to fourth argument registers
*      D5-D7    general work registers
*      Z (=A0)  hold the constant 0 (to allow n(Z,Di.L) addresses)
*      P (=A1)  BCPL P pointer (MC address of first arg)
*      G (=A2)  BCPL G pointer (MC address of global 0)
*      L (=A3)  work reg - (hold the return address in the
*                           entry and return sequences)
*      B (=A4)  base reg - (hold the MC address of entry to the
*                           current procedure. It is necessary
*                           for position independent addressing
*                           of data in program code.  Note that
*                           instruction and data space MUST be
*                           the same)
*      S (=A5)  MC address of the save S/R:        Timings Totals
*                  MOVE.L	(SP)+,L                  6
*                  MOVEM.L   P/L/B,-12(P,D0.L)       19
*                  ADDA.L    D0,P                     7
*                  MOVEM.L   D1-D4,(P)               20
*                  JMP       (B)                      4      32
*      R (=A6)  MC address of the return code:
*                  MOVEM.L   -12(P),P/L              16
*                  MOVE.L	-4(P),B                  8
*                  JMP       (L)                      4      28
*      SP(=A7)the system stack pointer
*
* The (typical) calling sequence is:
*
*         MMOVEQ    #36,D0     Stack increment         2
*         MOVE.L   n(G),B     (calling a global)      8
*         JSR      (S)        Call the save routine   4      14
*
* To return from a procedure:
*
*         JMP      (R)                                4       4
*
*
	PAGE
	INCLUDE "exec/types.i"
	INCLUDE "exec/nodes.i"
	INCLUDE "exec/lists.i"
	INCLUDE "exec/memory.i"
	INCLUDE "exec/ports.i"
	INCLUDE "exec/libraries.i"
	INCLUDE "exec/devices.i"
	INCLUDE "exec/io.i"
	INCLUDE "exec/tasks.i"
	INCLUDE "exec/resident.i"
	INCLUDE "exec/execbase.i"
	INCLUDE "exec/alerts.i"
	INCLUDE "exec/ables.i"
	INCLUDE "exec/errors.i"
	INCLUDE "exec/initializers.i"

	INCLUDE "libraries/expansion.i"
	INCLUDE "libraries/expansionbase.i"

	INCLUDE "intuition/preferences.i"

	INCLUDE "internal/librarytags.i"

	INCLUDE "dos/dos.i"
	INCLUDE "dos/dosextens.i"
	INCLUDE "dos/filehandler.i"

	INCLUDE "libhdr.i"
	INCLUDE "fault.i"
	INCLUDE "calldos.i"

	INCLUDE "dos_rev.i"

	INT_ABLES

FUNCDEF MACRO
	XREF _LVO\1
	ENDM

	INCLUDE "exec/exec_lib.i"

	XREF	_LVOSetPrefs
	XREF	_LVOSDivMod32
	XREF	_LVOUDivMod32
	XREF	_LVOSMult32  
	XREF	_LVOCacheClearE
	XREF	_LVOTaggedOpenLibrary

*
* blib, etc routines
*
BLIB	MACRO	;\1 - symbolname
	XREF	_\1
@\1	EQU	_\1
	ENDM

*
* external defs
*
	XDEF	_doslibname
	XDEF	_consolename
	XDEF	_ramname
	XDEF	_shellname
	XDEF	_SEGLIST
	XDEF	@ClearICache
	XDEF	get_ubase	; hits a6!
	XDEF	@multiply
	XDEF	@udiv32
	XDEF	_udiv32
	XDEF	@divrem32
	XDEF	_divrem32
	XDEF	@div32
	XDEF	_div32
	XDEF	@rem32
	XDEF	_rem32
*	XDEF	@createtask
*	XDEF	_createtask
*	XDEF	@createproc
*	XDEF	_createproc
	XDEF	_activecode
	XDEF	@activecode
	XDEF	_deactcode
	XDEF	@deactcode
	XDEF	_CNP_ActiveCode
	XDEF	@CNP_ActiveCode
	XDEF	@MakeProc
	XDEF	@AddSegArray
	XDEF	@AllocVecPub
	XDEF	_AllocVecPub
	XDEF	@AllocVecPubClear
	XDEF	_AllocVecPubClear
	XDEF	_freeVec
	XDEF	@bcpl_qpkt
	XDEF	_bcpl_qpkt
	XDEF	@qpkt
	XDEF	_qpkt
	XDEF	@qpkt_task
	XDEF	_qpkt_task
	XDEF	@returnpkt
	XDEF	_returnpkt
	XDEF	@taskwait
	XDEF	@taskid
	XDEF	@myproc
	XDEF	@GetProgramDir
	XDEF	@ArgStr
	XDEF	_CurrentDir
	XDEF	@SetProgramDir
	XDEF	@SetArgStr
	XDEF	@seglist
	XDEF	@consoletask
	XDEF	@setconsole
	XDEF	_setconsole
	XDEF	@getresult2
	XDEF	@input
	XDEF	@output
	XDEF	@cli
	XDEF	@filesystemtask
	XDEF	@setfilesys
	XDEF	_setfilesys
	XDEF	@setresult2
	XDEF	@SetIoErr
	XDEF	@result2
	XDEF	@currentdir
	XDEF	_currentdir
	XDEF	@selectinput
	XDEF	_selectinput
	XDEF	@selectoutput
	XDEF	_selectoutput
	XDEF	@rootstruct
	XDEF	@abort
	XDEF	_abort
	XDEF	@InternalRunCommand
	XDEF	_InternalRunCommand
	XDEF	@exit
	XDEF	@opendosbase
	XDEF	@dosbase
	XDEF	@store_IBase
	XDEF	@MySetPrefs
	XDEF	doslib_createproc
	XDEF	_deletetask

	XDEF	CALLDOS
	XDEF	SAVE
	XDEF	RET

	XDEF	J_OPEN
	XDEF	J_CLOSE
	XDEF	J_NOP
*
* requester handler routine
*
	XREF	@requester
	XREF	@requester2
	XREF	@strlen

	XREF	@getstring
	BLIB	createtask
	BLIB	createproc
	BLIB	freeobj
	BLIB	endstream
	XREF	@freevars
	BLIB	UnLoadSeg
	XREF	@CleanupProc
	BLIB	RunCommand
*
* dos errors
*
	XREF	_dos_errors

*
* BCPL stuff
*
	XREF	MLIB_SEG
	XREF	FAKEBLIB_SEG
	XREF	FAKECLI_SEG
	XREF	CONHANDLER
	XREF	FILEHANDLER

	XREF	JTAB		; dos jump table init
	XREF	_end		; end of dos module

*
* debug stuff
*

	XREF	_kprintf

*DEBUG	EQU	1

DBUG	macro
	ifd	DEBUG

	; save all regs
	movem.l	d0/d1/a0/a1,-(a7)

; first stack up to eight arguments for the printf routine
	IFGE	NARG-9
		move.l	\9,-(sp)		; stack arg8
	ENDC
	IFGE	NARG-8
		move.l	\8,-(sp)		; stack arg7
	ENDC
	IFGE	NARG-7
		move.l	\7,-(sp)		; stack arg6
	ENDC
	IFGE	NARG-6
		move.l	\6,-(sp)		; stack arg5
	ENDC
	IFGE	NARG-5
		move.l	\5,-(sp)		; stack arg4
	ENDC
	IFGE	NARG-4
		move.l	\4,-(sp)		; stack arg3
	ENDC
	IFGE	NARG-3
		move.l	\3,-(sp)		; stack arg2
	ENDC
*	IFGE	NARG-2
*		move.l	\2,-(sp)		; stack arg1
*	ENDC

STKOFF	SET	(NARG-1)<<2			; actual stack space used
	pea.l	n1\@
	jsr	_kprintf
	lea.l	STKOFF(sp),sp			; scrap stuff on stack
	movem.l	(a7)+,d0/d1/a0/a1
	bra.s	n2\@

n1\@	dc.b	\1,10,0
	cnop	0,2
n2\@
	endc
	endm

* KLUDGE is because of a bug in CAPE
KLUDGE	 EQU	1

* to allow InitResident for testing
*NOT_REAL EQU	1

	IFND	KLUDGE
	XREF	BCPLCtoB
	XREF	BCPLBtoC
	XREF	CLEARVEC
	XREF	COPYSTRING
	XREF	COPYVEC
	XREF	UNPACKSTRING
	XREF	PACKSTRING
	ENDC

	PAGE
	TTL "** TRIPOS Kernel  -   Equates **"
*
*
* Symbol definitions and data structures
*
*
LIBWORD   EQU    23456               Marks library routines
SECWORD   EQU    12345               Marks a BCPL section
*NOTINUSE  EQU       -1               Link word for dequeued packets
NEGSIZE   EQU	NEGATIVE_GLOBAL_SIZE Number of negative globals
GLOBSIZE  EQU	UG                   NUmber of positive globals
ROOTSIZE  EQU	rn_SIZEOF	     size of rootnode
SEGSIZE   EQU	3 		     Size of SegVec for CreateProc
GVSIZE    EQU	(NEGSIZE+GLOBSIZE+1)*4
*CLI_UPB   EQU       15               Upper bound of CLI structure
CLISTK    EQU   800		     Stack used for initial CLI (was 400)
				     ; Changed to match Execute/Run/System.
				     ; note - in longwords

*DOSSTK    EQU     1500		     Stack used by DOS when called

*
* Absolute areas
*
SysBase	    EQU	    4
*

SIGMASK   EQU     $100               Signal bit 8 mask
MSGSIG    EQU        8               Signal 8 used for message handling
FLGSIG    EQU       12               Signals 12-15 used for testflag/setflag

*
* Register definition
*

Z         EQUR    A0                 Constant Zero
P         EQUR    A1                 BCPL P ptr
G         EQUR    A2                 BCPL G ptr
L         EQUR    A3                 Work reg
B         EQUR    A4                 Addr of current procedure
S         EQUR    A5                 Save routine addr
R         EQUR    A6                 Return addr

*
*
* Coroutine symbols
*

C_LINK    EQU        0               Link to next co-routine
C_CLLR    EQU        4               Caller coroutine
C_SEND    EQU        8               Stack end - 32
C_RESP    EQU       12               Resumption ptr
C_FUNC    EQU       16               Function
*                   20               PC dump - dummy
C_RTRN    EQU       24               Return link for stop

*
* Global vector symbols
*
* Not used here, but the BCPL c: commands (via bcplstartup.asm) set all unused
* globals to this.  This is worse than silly on amigados, they should point to
* a guru/alert with a specific number.  - REJ
*
*UNGLOB    EQU $474C0001              (Unset global n is UNGLOB + 2*n)
*
*
* Nucleus primitives
*
	; moved to libhdr.i


	PAGE
	TTL      "** TRIPOS Kernel - Macro definitions"
*
* Macro definitions
*
* Module header
*
SECT    MACRO
**        CNOP   0,4
**        DC.L   LIBWORD
**        DC.B   7,\1
	ENDM
*
* Call EXEC with A6 already set
*
CALL    MACRO
	JSR          _LVO\1(A6)
	ENDM
*
* Call EXEC with A6 not set
*
CALLS   MACRO
	MOVE.L	SysBase,A6
	CALL	\1
	ENDM

	PAGE
******************************************************************
*                                                                *
*         Kernel initialisation                                  *
*                                                                *
******************************************************************
	SECTION doslib,CODE

KLIB_SEG  DC.L	     0			; no next section
KLIB      DC.L       (KEND-KLIB)/4	; The size of the module for segment
*
* We are entered at KSTART from bootstrap.
* This Tripos derivative does not have any
* fixed locations within it, and most primitive calls are handled by EXEC.
*
* Where TRIPOS uses task numbers we use here EXEC ports so that we can use
* the EXEC message passing mechanism.
*
	BRA.S	KINIT	; Jump past segment list (keep lword aligned)
*
* Initialisation consists of starting up the initial CLI task. We create
* a new task and set it up so that when it starts executing it will initialise
* registers in a suitable fashion. We also need to build an initial segment
* list for this task. The Tripos image loaded into memory is built by SYSL
* so that the code segment table is patched to contain the correct values.
*
* We can now link with external symbols, however BPTRs are a pain - REJ
* This is (effectively) a segarray structure, is used in pr_SegList
* This will have to be copied into real memory and turned into BPTRs.
*

	CNOP	0,4
_SEGLIST
	DC.L	4<<2            ; Size of segment list*4: init will undo *4
	DC.L	KLIB_SEG        ; segment - CPTR! KLIB
	DC.L	FAKEBLIB_SEG    ; segment - CPTR! blib stubs
	DC.L	0		; MLIB_SEG	     segment - CPTR! mlib
	DC.L	FAKECLI_SEG     ; segment - CPTR! cli stubs
	DC.L	0		; CONHANDLER  syssegl[5]
	DC.L	0		; FILEHANDLER syssegl[6]
	DC.L	0		; restart     syssegl[7] (not used)
	DC.L	0		; cli c entrypoint syssegl[8]
	DC.L	0		; ram seglist syssegl[9]
SEGLISTEND

INITSEGSIZE	  EQU	     (SEGLISTEND-_SEGLIST)/4	; for initialization

*
* Create initial task after setting up suitable registers
*

* put in a rom tag, so we can find it later.
* not coldstart, we're started by the boot code of a device

romtag:
	DC.W	RTC_MATCHWORD		;(50) UWORD RT_MATCHWORD
	DC.L	romtag			;(52) APTR  RT_MATCHTAG
	DC.L	_end			;(56) APTR  RT_ENDSKIP
	DC.B	0; RTW_COLDSTART	;(5A) UBYTE RT_FLAGS
	DC.B	VERSION			;(5B) UBYTE RT_VERSION
	DC.B	NT_LIBRARY		;(5C) UBYTE RT_TYPE
	DC.B	-120			;(5D) BYTE  RT_PRI
	DC.L	LIBNAME			;(5E) APTR  RT_NAME
	DC.L	idtag			;(62) APTR  RT_IDSTRING
	DC.L	KINIT			;(66) APTR  RT_INIT
						;(6A) LABEL RT_SIZE

idtag:	VSTRING
	ds.w	0


KINIT:
	DBUG <"in dos init code">

    IFD NOT_REAL
	MOVEM.L	d2-d7/a2-a6,-(sp)
    ENDC
* sysbase in a6 is NOT guaranteed because we're NOT inited by InitResident
*
	MOVE.L	SysBase,A6		; Library base register
*
* Initialise DOS library. This involves copying the node to real RAM
* and allocating space for rootnode and shared GV
*
* Allocate everything with DOSBase, to save code/frags.
* We get the base, a timerreq, the RootNode, and the shared global vector.
* Also space for a seglist for the console-handler (and the filesystem)
*
CONSEGSIZE	EQU	3*4
RAMSEGSIZE	EQU	3*4
BASESIZE	EQU	dl_SIZEOF+2+ROOTSIZE+GVSIZE+CONSEGSIZE+IOTV_SIZE+RAMSEGSIZE

	MOVE.L	#BASESIZE,D0		; size of data area everything
	LEA	JTAB,A0			; vector table
	SUB.L	A1,A1			; no initstruct table
	MOVE.L	A1,A2			; no init routine (return to me)
					; with no init routine, d1 is spurious
	CALL	MakeLibrary
	TST.L	D0
	BEQ	DISASTER
	MOVE.L	D0,A5			; A5 points to library base
	OR.B	#LIBF_SUMUSED!LIBF_CHANGED,LIB_FLAGS(a5)
*
* Turn on the data cache if it exists
*
*USE_CACHE_CALLS	EQU	1
*
* &$#%^$%*(*#&$ Hurricane boards crash if data cache is turned on without
* MMU tables to disable caching for chip mem and IO registers.
* #$#(*&%*&^%^@(*&$
*
	IFD	USE_CACHE_CALLS
DBITS	EQU   CACRF_WriteAllocate!CACRF_ClearD!CACRF_EnableD
CBITS	EQU   CACRF_IBE!CACRF_ClearI!CACRF_EnableI

	MOVE.L	#DBITS!CBITS,D0
	MOVEQ	#-1,D1			; affect all bits
	CALL	CacheControl
	ENDC
*
* Setup the dl_Errors pointer.
*
	LEA	_dos_errors,A0
	MOVE.L	A0,dl_Errors(A5)
*
* Open utility.library
*
	MOVEQ	#OLTAG_UTILITY,d0	; utility library, any version
	CALL	TaggedOpenLibrary
	MOVE.L	D0,dl_UtilityBase(A5)
	BEQ	DISASTER
*
* Now build the rootnode (MUST be cleared - Makelibrary guarantees this!)
* Used as an exec message later!!!!!
* +2 is because a library structure is odd-sized, and the root must be on
* a longword boundary!
*
	LEA	dl_SIZEOF+2(a5),A0
	MOVE.L	A0,dl_Root(A5)
*
* and now the global vector (pointer into middle of GV - see negsize)
* we want it in A2 for later
*
	LEA	dl_SIZEOF+2+ROOTSIZE+(NEGSIZE*4)(a5),A2
	MOVE.L	A2,dl_GV(A5)
*
* Set up the timer request pointer in dosbase
* Note you don't need a port for OpenDevice!
* I put it last because I'm not certain it will always be a longword size
*
	LEA	dl_SIZEOF+2+ROOTSIZE+GVSIZE+CONSEGSIZE(a5),A1
	MOVE.L	A1,dl_TimeReq(a5)
	sub.l	a0,A0			; name of NULL (0) is timer.device-V40
	MOVEQ	#UNIT_VBLANK,D0
	MOVEQ	#0,D1
	CALL	OpenDevice

*
* Other doslib base stuff
*
	LEA.L	SAVE(PC),A3	Save routine address
	LEA.L	RET(PC),A4	Return routine address
	MOVEM.L	A2-A4,dl_A2(A5)	Save registers

*
* Set up things MakeLibrary doesn't do for us
*
	MOVE.B	#NT_LIBRARY,LN_TYPE(A5)
	LEA.L	LIBNAME(PC),A1
	MOVE.L	A1,LN_NAME(A5)
	MOVEQ	#0,d0
	MOVE.B	D0,LN_PRI(A5)
	MOVE.W	#VERSION,LIB_VERSION(A5)
	MOVE.W	#REVISION,LIB_REVISION(A5)

	MOVE.L	A5,A1	

	CALL	AddLibrary		; add the library to the exec list
*
* Removed ramlib init from here, and moved it to cli_init.  REJ
*
* Create initial CLI (A2 contains globvec)
*
* Must build a seglist (segarray) in alloced memory
* Note size in segarray is in bytes, and is converted into longwords
*
	LEA.L	_SEGLIST(PC),A3		; Segment list ptr
	MOVEQ	#(INITSEGSIZE*4)+4,D0	; # of bytes
	BSR	@AllocVecPub		; Since Deact undoes this
	TST.L	D0
	BEQ	DISASTER

	MOVE.L	D0,A0
	MOVE.L	D0,D1			; this is arg1 for lower down!
	MOVEQ	#INITSEGSIZE,D0		; # of entries + size
	BRA.S	2$			; make count correct
1$
	MOVE.L	(A3)+,D2	; copy entries while converting to BPTRs!
	LSR.L	#2,D2
	MOVE.L	D2,(A0)+
2$	DBRA	D0,1$
*
* Now find the console-handler and add it to the seglist
*
	MOVE.L	D1,A3		; Save segarray ptr

*
* find the shell
*
	LEA	SHELLNAME(pc),a1
	CALL	FindResident
	MOVE.L	D0,A1
	TST.L	D0
	BEQ	DISASTER		; very bad, no shell/cli
	MOVE.L	RT_INIT(A1),a0		; table of entries, BCPL entry first

	MOVE.L	(a0),d0			; bcpl entrypoint (for createtask)
	LSR.L	#2,d0
	MOVE.L	D0,4*4(a3)		; replace syssegl[4]

	MOVE.L	4(a0),d0		; c entrypoint (for create(new)proc)
	LSR.L	#2,d0
	MOVE.L	D0,8*4(a3)		; replace syssegl[8]

* now console-handler
	LEA	CONSOLENAME(PC),A1
	MOVE.W	#dl_SIZEOF+2+ROOTSIZE+GVSIZE,d3
	MOVEQ	#5*4,d2			; offset in segarray - syssegl[5]
	BSR	FindResSeg

* now ram-handler...
	LEA	RAMNAME(PC),A1
	MOVE.W	#dl_SIZEOF+2+ROOTSIZE+GVSIZE+CONSEGSIZE+IOTV_SIZE,d3
	MOVEQ	#9*4,d2			; offset in segarray - syssegl[9]
	BSR	FindResSeg
*
* Now the fs - it's funny, and has to look like BCPL (ARGH).
*
	LEA	FILESYSNAME(PC),A1
	CALL	FindResident
	MOVE.L	D0,A1
	TST.L   D0
	BEQ     DISASTER

	;-- fs has rt_init point to the actual seglist
	MOVE.L	RT_INIT(A1),D0
	LSR.L   #2,D0			; we want a BPTR
	MOVE.L	D0,6*4(a3)		; syssegl[6] = fs
	
*
* Create the initial process
*
 DBUG <"creating initial process">
	MOVE.L	A3,D1		; get segarray ptr back
	LSR.L	#2,D1		; SegArray as BCPL ptr (arg1)
	MOVE.L	#CLISTK,D2	; Stack size
	MOVEQ   #0,D3		; Priority zero
	LEA.L	CLINAME(PC),A3	; Name
	MOVE.L	A3,D0		; master segarray as arg4 - in d0
	ASR.L	#2,D0		; BCPL address
	MOVE.L	#GLOBSIZE,(A2)	; Insert size of GV at (G)
	MOVE.L	A2,D5		; Globvec(bptr)
	ASR.L	#2,D5
	MOVE.L	D5,A0		; createtask wants 5th param in A0
	BSR	@createtask	; Causes most of the globvec to be built
 DBUG <"created initial process">
	MOVE.L	D0,D7
	BEQ.S	DISASTER        ; Failed

* Extract the mount list node from the expansion base slot

	MOVEQ	#OLTAG_EXPANSION,D0	; expansion library, any version
	CALL	TaggedOpenLibrary	; NOTE: not a process yet!!!!!
	MOVE.L	D0,A2			; Pointer to expbase
	LEA.L	eb_MountList(A2),A0

*	  ; BootNode struct now declared in /libraries/romboot_base.i
*
* STRUCTURE BootNode,LN_SIZE
*    UWORD	bn_Flags
*    CPTR	bn_DeviceNode
*    LABEL	BootNode_SIZEOF
*
* Link the remaining eb_MountList entries to the boot devicenode for AmigaDos
*
* Save d0! (holds expansionbase!)
	MOVEM.L	D0/A0/A1,-(SP)
	MOVE.L	LH_HEAD(A0),A0
	TST.L	LN_SUCC(A0)		; list empty ?
	BEQ.S	BootLink_Done
	TST.L	bn_DeviceNode(A0)	; this bootnode has current devnode ?
	BEQ.S	BootLink_Done

Boot_Link:
	MOVE.L	LN_SUCC(A0),A1		
	TST.L	LN_SUCC(A1)		; nothing more to link ?
	BEQ.S	BootLink_Done

	MOVE.L	bn_DeviceNode(A0),A0

	TST.L	dn_Next(A0)		; current node already linked ?
	BNE.S	BootLink_Done		; won't mess with circularities for now.

	TST.L	bn_DeviceNode(A1)	; next bootnode has subsequent
					; devnode ?
	BEQ.S	BootLink_Done
	MOVE.L	bn_DeviceNode(A1),D0	; cptr to next node
	LSR.L	#2,D0			; convert to bptr
	MOVE.L	D0,dn_Next(A0) 		; link to next of current node

	MOVE.L	A1,A0			; next becomes current node
	BRA.S	Boot_Link

BootLink_Done:
	MOVEM.L	(SP)+,D0/A0/A1

	CALL	RemHead			; (a0) - a6 still has execbase
*
* D7 contains the message port for that task which is waiting for a message
* Use rootnode space as initial message area, and send message to
* start it up. D0 contains the device node pointer for the boot device.
*
* EVIL!!!!!!! FIX!!!!!!!!!
* The 4 bytes corresponding to days+2 to minutes+1 in the rootnode MUST
* be 0 (they correspond to ln_Name of a node).
*
	BSR	GETDATA		; Get library ptr into L (A3)
	MOVE.L	D7,A0		; Port
	CLR.L	pr_WindowPtr-pr_MsgPort(A0) ; Zero window ptr (use workbench)
	MOVE.L	dl_Root(A3),A1	; Message area

* Pass the boot device pointer in the startup packet, and release
* the node space used for it.

	TST.L	D0
	BEQ.S	EndL
	MOVE.L	D0,A3
	MOVE.L	bn_DeviceNode(A3),rn_FileHandlerSegment(A1)
EndL

 DBUG <"Sending initial message...">
	CALL	PutMsg		; A6 still has execbase

*
* Close expansion library
*
	MOVE.L	A2,A1		; expbase (a6 == execbase)
	CALL	CloseLibrary

* Free the boot node memory
*	  MOVE.L 	 L,A1
*	  MOVEQ		 #BootNode_SIZEOF,D0
*	  CALLS		 FreeMem
*
* We have nothing else to do now, so delete myself.
*
 DBUG <"bye bye">
	SUBA.L	A1,A1		; Myself
	CALL	RemTask		; Bye bye (a6 = execbase)
*
* Here if a disaster whilst booting
*
DISASTER
	MOVE.L	#AT_DeadEnd!AN_DOSLib,D7
	CALLS	Alert

*
* Subroutine used by above code to set up console/fs
* a1 = name of module, a3 = ptr to segarray, a5 = library ptr,
* d2 = offset in segarray to store seglist, d3 = offset from library for seglist
* (d2 and d3 are words) a6 = execbase
*
FindResSeg:
	CALL	FindResident
	MOVE.L	D0,A1		; Save and test
	TST.L	D0
	BEQ.S	DISASTER	; very, very bad
*
* build handler seglist in dosbase
*
	LEA	0(a5,d3.w),A0	; address of built seglist
	MOVE.L	A0,D0		; save
	CLR.L	(A0)+		; no next segment
	MOVE.W	#$4ef9,(A0)+	; JMP abs.l
	MOVE.L	RT_INIT(A1),(A0) ; Address to jump to

	MOVE.L	D0,A0		; for clearicache
	
	LSR.L	#2,D0		;BPTR to seglist
	MOVE.L	D0,0(A3,d2.w)	;syssegl[d2] = seglist

	MOVEQ	#10,d0		; length of modified code
	bsr.s	@ClearICache

	RTS
*
* Stuff to clear cache after modifying code
* void __regargs ClearICache(UBYTE *addr, LONG len)
*
@ClearICache:
	move.l	a6,-(sp)
	move.l	SysBase,a6
	moveq	#CACRF_ClearI,d1	; clear ICache only
	jsr	_LVOCacheClearE(a6)
	move.l	(sp)+,a6
	rts

*
* Strings for above and below
*
	CNOP    0,4

CLINAME   DC.B       11,'Initial CLI',0		; BSTR!
_doslibname:
LIBNAME   DC.B       'dos.library',$00		; CSTR
*EXPNAME   DC.B       'expansion.library',$00	; CSTR
*UTILITY   DC.B       'utility.library',$00	; CSTR
*TIMER     DC.B	     'timer.device',$00		; CSTR
_consolename:
CONSOLENAME
	DC.B	     'con-handler',$0		; CSTR
FILESYSNAME
	DC.B	     'filesystem',$0		; CSTR
_shellname:
SHELLNAME DC.B	     'shell',$0			; CSTR
_ramname:
RAMNAME	  DC.B	     'ram-handler',$0		; CSTR
	CNOP	0,2

	PAGE
	TTL        "Library support"
*
*
* The dos jump table init used to be here.
*
* Initialisation code
*
J_OPEN  
	ADDQ.W	#1,LIB_OPENCNT(A6)	; Increment use count
	MOVE.L	A6,D0			; And return A6
	RTS

J_CLOSE
	SUBQ.W	#1,LIB_OPENCNT(A6)	; Optimized REJ
J_NOP   MOVEQ	#0,D0			; Return with zero (no segment)
	RTS

*
* Now the standard call for calling the DOS, using BCPL globals.
*
* Entered with D0        Global number
*              D1-D4     Args
*
* Note A6 does not have to point to library entry.
*
* We may trash D0/D1/A0/A1 and nothing else.
*
*CALLGLOB
*        MOVEM.L D2-D7/A2-A6,-(SP)       Save all registers we must
*        SUBA.L	A0,A0                   Zero A0
*	MOVE.L	SP,D5                   Get SP
*	SUBI.L	#DOSSTK,D5              Make BCPL stack ptr
*	BCLR    #1,D5                   Ensure long word aligned
*	MOVE.L	D5,A1                   And into BCPL stack ptr reg
*        MOVEM.L BCPLregs,A2/A5/A6       Set up other BCPL regs
*        ASL.L	#2,D0                   Make global number byte offset
*        MOVE.L	0(A2,D0.L),A4           Get entry point required
*        MOVEQ   #12,D0                  Mimimal stack frame
*        JSR     (A5)                    Call BCPL routine
*        MOVE.L	D1,D0                   Result to D0
*        MOVEM.L (SP)+,D2-D7/A2-A6       Restore registers
*        RTS
*
* Routine for BCPL to call C routines via global vector.
* Entered with normal BCPL environment (see above).
* Takes mask for converting params in B (A4) (pushed by stub rtn)
* Has address of C-style routine in L (A3) (pushed by stub rtn)
* Called from the Save Rtn (A5) after pushing/saving regs.
* P (A1) has ptr to the contents of D1-D4, B has calldos, L has temp.
* Must exit by calling return code (A6).  Can modify D0/1,A3,A4 (and maybe
* D5-D7).
*
* Using semi-custom routines works better in some cases.
*

	; this version stuffs bstr's on the stack
CALLDOS
	MOVEM.L	D2-D4/A0-A2/A6,-(A7)
	LINK	A5,#0			; saves a5, and allows us to play
					; stack games without unbalancing it.
	; handle argument conversion
	; arguments are at (p) and args 1-4 are in regs D1-D4 as well
	; codes in mask are:
	;		00 - no change/unused,
	;		01 - BPTR -> CPTR
	;		10 - BSTR -> CSTR
	;		11 - end of args
	;
	; mask is laid out as follows: bits 0-1 -> 1st arg, 2-3 -> 2nd arg, etc
	; top bit of mask == 0 for no shift of result, 1 for cptr->bptr
	;
	; BSTR->CSTR conversion is by copying it onto the stack, because of
	; the resident command.
	;
	MOVE.L	A4,D0			; get mask
	MOVE.L	A1,A0			; pointer to args

loop:	MOVE.L	D0,D6			; get current mask bits
	LSR.W	#2,D0			; shift mask bits for next loop
	ANDI.W	#3,D6			; get code (0-3)
	SUBQ.W	#1,D6			; start switch code
	BMI.S	code00			; code 00 - ignore (d6 = -1 to 2)
	BNE.S	code10			; not code 01 - try 10

	; we know it's 01 - BPTR->CPTR
	MOVE.L	(A0),D7			; convert to cptr
	ASL.L	#2,D7
	MOVE.L	D7,(A0)			; store cptr back in arg
	; fall thru into code00

code00: ADDQ.L	#4,A0			; code 00 - do nothing -
	BRA.S	loop			;                  increment arg ptr

code10:	SUBQ.W	#1,D6			; d6 now 0 to 1
	BNE.S	loadargs		; code 11 - done

	; must be code 10 - BSTR to CSTR - copy onto stack!
	; FIX!!!! we should check stack limits here!!!!!!
	;
	MOVE.L	A5,D7			; SAVE A5!

	MOVE.L	(A0),A5			; get BSTR
	ADD.L	A5,A5
	ADD.L	A5,A5			; change to CPTR

	; get space from stack - make sure it's word-alligned!
	MOVEQ	#0,d6
	MOVE.B	(a5)+,d6		; get string length
	MOVE.L	d6,d1			; save
	ADDQ.W	#2,d1			; space for null and 1 for word allign
	ANDI.B	#$FE,d1			; force word allignment-faster than BCLR
	SUB.W	d1,a7			; get length bytes from stack
	MOVE.L	a7,(a0)+		; store cstr pointer and increment arg

	; now copy the string onto the stack
	MOVE.L	a7,a6			; pointer to new string
	BRA.S	2$
1$	MOVE.B	(a5)+,(a6)+
2$	DBRA	d6,1$
	CLR.B	(a6)			; null-terminate

	MOVE.L	D7,A5			; RESTORE A5!
	BRA.S	loop

	; load the args into the registers they'll be expected in
	; openwindow counts on D5 being loaded, etc.
loadargs:
	MOVEM.L	(A1),D1-D7		; read args 1-7 into regs

	; MUST leave a1 intact for use by called routines!!!!

	MOVE.L	G_DOSBASE(A2),A6	; get DOSBase from global vector
	JSR	(A3)			; call the actual routine
	; returns value in D0

	; d2-d4 regs assumed trashed by interface routines that need
	; to play with args in funny ways - see fakeblib.asm (in addition to
	; the normal d0/d1/a0/a1).

done:   ; now check return for shift needed (cptr->bptr)
	MOVE.L	A4,D1
	BTST.L	#RETURN_BPTR_BIT,D1
	BEQ.S	not_bptr
	LSR.L	#2,D0

not_bptr:				; finally!
	UNLK	A5			; restore stack pointer and a5
	MOVEM.L	(SP)+,D2-D4/A0-A2/A6	; restore BCPL environ
	MOVE.L	D0,D1			; return code in D1 for bcpl
	JMP	(R)			; BCPL return

*
*
* Various return sequences used by the kernel
*
CERROR
	MOVE.L	D1,-(SP)
	MOVE.L	SysBase,A0
	MOVE.L	ThisTask(A0),A0
	MOVE.L	(SP)+,pr_Result2(A0)	; TRICKY!!!!!
	MOVEQ	#0,D0
	RTS
*
* BCPL error rtns
*
BCPLERROR
	BSR	GETID
	MOVE.L	D1,pr_Result2-pr_MsgPort(L)
ER_RET  CLR.L	D1			; Set zero result
	JMP	(R)			; Return

	PAGE
	TTL         "Subroutines at fixed offset from S register"
*
* Subroutines which are called relative to the S register
*
******************************************************************
* The standard return sequence                                   *
******************************************************************

RET       MOVEM.L	-12(P),P/L			[-0C]
	  MOVE.L	-4(P),B				[-06]
	  JMP		(L)				[-02]
*
******************************************************************
*  The standard SAVE routine which is used to call a routine     *
*         Entered with B pointing to routine address             *
******************************************************************

SAVE
	MOVE.L	(SP)+,L		 Standard save sequence [00]
	MOVEM.L	P/L/B,-12(P,D0.L)			[02]
	ADDA.L	D0,P					[08]
	MOVEM.L	D1-D4,(P)				[0A]
	JMP	(B)					[0E]

********************************************************************************
*  Branches to arithmetic routines                                             *
********************************************************************************
MULT      BRA.S     MULT32                                    [10]
DIV       BRA.S     DIV32                                     [12]
DIVX      BRA.S     DIV32                                     [14]
********************************************************************************
*       Stack Check                                                            *
*                 Entered with D0 = Stack frame size of this routine           *
* Can't totally remove, due to fixed offsets from above
********************************************************************************
STACKCHECK
*            ADD.L	P,D0                New top of stack      [16]
*            MOVE.L	G_STACKBASE(G),D5       Current stackbase as BCPL address
*            LSL.L	#2,D5               MC address
*            CMP.L	C_SEND(Z,D5.L),D0   Compare stack limit with stack top
*            BGE.S   STACKOVER           J if top > limit
	  RTS                         OK
*STACKOVER   MOVEQ   #84,D0              Stack overflow error
*            TRAP    #0
*            RTS
********************************************************************************
*       Multiply                32 bit multiply routine                        *
*                 Performs D1 := D1 * D2                                       *
********************************************************************************
@multiply:
	movem.l	a0-a2/a6,-(sp)
	bsr.s   get_ubase
	bra.s	mult_common
MULT32
	movem.l	a0-a2/a6,-(sp)
	move.l	G_UTILITYBASE(a2),a6
mult_common:
	move.l	d1,d0
	move.l	d2,d1
	jsr	_LVOSMult32(a6)
	move.l	d0,d1
	movem.l	(sp)+,a0-a2/a6
	rts

	; returns a6=utilitybase, not touching any other regs
get_ubase:				; shared with divide - hits a6 only!
	move.l	SysBase,a6
	move.l	ThisTask(a6),a6
	cmp.b	#NT_TASK,LN_TYPE(a6)	; is this a process
	beq.s	open_dos		; got to util with a6 = globvec,
	move.l	pr_GlobVec(a6),a6	; no other regs are touched
util:	move.l	G_UTILITYBASE(a6),a6
	rts

open_dos:	;-- we're a task.  Open dos and find the shared globvec
	movem.l	d0-d1/a0-a1,-(sp)	; preserve regs!
	bsr	@opendosbase		; uses openlibrary
	move.l	d0,a6			; dosbase
	move.l	dl_GV(a6),a6		; shared global vector
	movem.l	(sp)+,d0-d1/a0-a1
	bra.s	util


	PAGE
********************************************************************************
*       Divide                  32 bit division routine                        *
*           Performs D1 := D1/D2; D2 := D1 REM D2                              *
*									       *
********************************************************************************
*
*     Note that both D1 and D2 can be as big as one greater than the
*     largest ordinary positive integer held in a 32-bit (signed) word.
* If D2=0 the 68000 will take a 'divide by zero' trap.
* The only case that can lead to an answer that is out of the range
* of normal (signed) numbers is #X80000000/1 which can arise when
* Divide1 was called on #X80000000/(-1)

@rem32:
_rem32:
	bsr.s	@div32
	move.l	d1,d0
	rts

DIV32:
	move.l	d1,d0
	move.l	d2,d1
	bsr.s	@div32
	move.l	d1,d2
	move.l	d0,d1
	rts

@udiv32:
_udiv32:
	move.l	a6,-(sp)
	bsr.s	get_ubase
	JSR	_LVOUDivMod32(a6)
	move.l	(sp)+,a6
	rts

*
* return remainder in a variable passed in a0
*
@divrem32:
_divrem32:
	bsr.s	@div32		; preserves a0!
	move.l	d1,(a0)
	rts
@div32:
_div32:
	; save a0-a1 for BCPL and divrem32!
	movem.l	a0-a1/a6,-(sp)
	bsr.s	get_ubase
	JSR	_LVOSDivMod32(a6)
	movem.l	(sp)+,a0-a1/a6
	rts
	
	PAGE
	TTL     "*** Tripos Kernel - Primitive Routines ***"
******************************************************************
*                                                                *
* CREATETASK( SEGLIST, STACK, PRIORITY, NAME, GVEC )             *
*                                                                *
* This creates a new task with code specified by seglist. The    *
* stack and priority also passed. The task structure is          *
* initialised and the task added to the queue.                   *
* This is a BCPL task and a global vector (shared or             *
* private) is passed as GVEC. In this case the task is started   *
* so that suitable BCPL initialisation is performed (the global  *
* vector is initialised and registers set up). The initialisation*
* will then call TASKWait to wait for the first message.         *
*                                                                *
* struct MsgPort *createtask (BPTR segl,LONG stacksize,LONG pri, *
*			      BSTR name,BPTR globvec);           *
* called with C conventions!					 *
* Does not modify BSTR name, ok to be in ROM			 *
******************************************************************
	SECT       <'Createt'>

CRTASK
	MOVEM.L	A0/A1/A6,-(SP)
	MOVE.L	D4,D0		; Lattice can't pass args in D4 or D5
	MOVE.L	16(A1),A0	; get globvec off BCPL stack (BPTR)
	BSR	@createtask
	bra	restore_bcpl_exit

*          CNOP       0,4
****************************************************************
*                                                              *
* ProcID := CreateProc( Name, Pri, Seg, Stacksize )            *
*                                                              *
* Creates a new process with specified (C format) name, pri    *
* and stacksize (in bytes). The segment list is one returned   *
* from LoadSeg, and other entries in the Segment vector are    *
* copied from the current environment.                         * 
*                                                              *
* struct MsgPort *createproc (char *name,LONG pri,BPTR seg,    *
* 			      LONG stacksize);		       *
****************************************************************
*

CRPROC
	movem.l	a0/a1/a6,-(sp)
	bsr.s	doslib_createproc
	bra	restore_bcpl_exit

doslib_createproc:
	move.l	d4,d0
	jmp	@createproc

*
* Get memory via AllocEntry.  Entered with D0 = size.  Returns Entry or high
* bit set.  Requires a6=sysbase!
*
AllocEntry:
	MOVE.L	D0,-(SP)			; Size onto stack
	MOVE.L	#MEMF_PUBLIC+MEMF_CLEAR,-(SP)	; Requirements
	MOVE.W	#1,-(SP)			; Number of entries
	LEA.L	-LN_SIZE(SP),SP			; Leave space for node
	MOVE.L	SP,A0				; Arg into A0
	CALL	AllocEntry			; Get Memory
	LEA.L	LN_SIZE+10(SP),SP		; Adjust stack
	RTS

*
* Get memory for task structure and stack. Place it into
* the task field so that it gets deleted properly on exit.
* Entered with D0 = size
*
AllocProc:
	MOVEM.L	a2/a6,-(SP)
	MOVE.L	SysBase,A6			; Get library pointer
	BSR	AllocEntry			; Get Memory
	TST.L	D0
	BMI.S   AllPErr				; AllocEntry sets bit 31 on err
	MOVE.L	D0,A2				; A2 is mem block
	MOVE.L	ME_ADDR+ML_ME(A2),A1		; A1 is task space
	LEA	TC_MEMENTRY(A1),A0
	NEWLIST	A0
	EXG.L	A2,A1	
	CALL	AddHead
	MOVE.L	A2,D0				; Return memory
AllocPr2
	MOVEM.L	(SP)+,a2/a6
	RTS
AllPErr
	MOVEQ      #0,D0			; Error
	BRA.S      AllocPr2

*
* Create process and initialize
* a0 - pointer to longword stacksize
* a1 - pointer to name - NOT TERMINATED (maybe)
* d0 - length of name
*
@MakeProc:
 DBUG <"In MakeProc, stack = %ld, 0x%lx, %ld">,(a0),a1,d0
	MOVEM.L	D2-D5/A2-A3,-(SP)
	MOVE.L	(a0),d4			; get stacksize
	ADDQ.L	#3,D4			; We want to round stack size up to
	AND.W	#$FFFC,D4		; the next multiple of 4 bytes
	MOVE.L	D4,(a0)			; put stacksize back

* Find length of Task name and allocate everything using AllocEntry
	MOVE.L	D0,D5			; save original length
	ADDQ.L	#4,D0			; space for null + 3 for alignment
	AND.W	#$FFFC,D0		; make length an even number of LWs
	MOVE.L	D0,D3			; save length
	ADD.L	D4,D0			; form length of block (add length)
					; BCPL dos merely made it even
* Get space for Task struct and process base to stacksize (in bytes)
	ADD.L	#pr_SIZEOF,D0
	MOVE.L	D0,D2			; save it
	MOVE.L	A1,A2			; save
	BSR.s	AllocProc
*					; also adds all memory to tc_mementry
	TST.L	D0
	BEQ	CRPERR			; no memory!
	MOVE.L	D0,A0			; process ptr
	MOVE.L	D0,A1

	;-- now copy name above stack
	ADD.L	d2,a1			; ptr past last word block
	SUB.L	d3,a1			; subtract length of name (inc null)
	MOVE.L	a1,LN_NAME(a0)
	BRA.S	2$			; d5 still has original length
1$	MOVE.B	(a2)+,(a1)+
2$	DBRA	d5,1$			; may not be null-terminated
	CLR.B	(a1)			; null-terminate

*
* Initialise task structure slots.
* a0 Space for task
* d4 Stacksize in bytes

	MOVE.B	#NT_PROCESS,LN_TYPE(A0)		; Set type of new task
	MOVE.L	#SIGMASK,TC_SIGWAIT(A0)		; Wait for this signal
	LEA	AbortHandler(PC),A1
	MOVE.L	A1,TC_TRAPCODE(A0)		; Install trap handler
	LEA	pr_SIZEOF(A0),A1		; A1 is low stack
	MOVE.L	A1,TC_SPLOWER(A0)		; Fill in SPLower
	MOVE.L	A1,D1
	LSR.L	#2,D1				; Stack as BCPL ptr
	MOVE.L	D1,pr_StackBase(A0)		; Stored in process base
	MOVE.L	D4,pr_StackSize(A0)		; Stack size
	ADD.L	D4,A1				; Make SPUpper
	MOVE.L	A1,TC_SPUPPER(A0)		; also a return value!
	MOVE.L	D4,-(A1)			; push stacksize onto stack
	MOVE.L	A1,TC_SPREG(A0)			; initial stack value
	MOVE.L	A1,pr_ReturnAddr(A0)		; ..and also returnaddr

	MOVE.L	A0,MP_SIGTASK+TC_SIZE(A0)	; Initialise message port slot
	LEA	MP_MSGLIST+TC_SIZE(A0),A1	; Point to msg list
	NEWLIST	a1				; Initialise this
	MOVE.B	#MSGSIG,MP_SIGBIT+TC_SIZE(A0)	; ..and signal to be issued

* Initialise process structure
* Can't call @dosbase here - may be from a task!
	BSR	GETDATA				; returns dosbase in A3
						; leaves all other regs
	MOVE.L	dl_GV(a3),pr_GlobVec(A0)	; shared global vector ptr

* Initialize the local env variable list
	LEA     pr_LocalVars(a0),a1		; list ptr
	NEWLIST	a1

	MOVE.L	A0,D0				; return value
CRPERR:
	MOVEM.L	(SP)+,D2-D5/A2-A3
	RTS

*
* Make segment ptr (segarray), and add to process.  Allocate and build
* fake seglist if needed.  Must be callable from a Task.  Puts the memory
* on the tc_MemEntry so it can be freed by RemTask().
*
* a0 - new process
* d0 - seglist (BPTR) or NULL
* d1 - entry (CPTR) or NULL - mutually exclusive with seglist!
*
@AddSegArray:
	movem.l	d2-d4/a2-a3/a6,-(sp)
	move.l	a0,a2				; new proc
	move.l	d0,d2				; seglist
	moveq	#5*4,d0				; size of segarray
	move.l	d1,d3				; entry
	beq.s	1$

	;-- need to build fake seglist
	moveq	#5*4+10,d0			; $00000000 JMP xxxx.L
1$:
	;-- allocate with AllocEntry so we can add it to the tc_MemEntry list
	;-- This means it will be freed by RemTask...
	move.l	SysBase,a6			; Get library pointer
	bsr	AllocEntry			; so we can add it to tc_MemEntry
	tst.l	d0
	bmi.s	adderr				; AE() returns bit 31 for error
	move.l	d0,a1
	move.l	ME_ADDR+ML_ME(a1),a3		; the actual memory
	lea	TC_MEMENTRY(a2),a0
	CALL	AddHead				; add it to the tc_MemEntry

	move.l	a3,d0				; segarray (list of seglists)
	lsr.l	#2,d0				; make into bptr

	move.l	d0,d4
	tst.l	d3				; did we need a seglist?
	beq.s	got_seglist
						; first longword already is 0
	move.w	#$4ef9,5*4+4(a3)		; JMP xxx.L
	move.l	d3,5*4+6(a3)			; address to call
	move.l	d4,d2				; save seglist ptr
	addq.l	#(5*4)>>2,d2			; this is our seglist (BPTR!!)

	;-- Flush Icache - a bit more flushed than we have to
	move.l	a3,a0				; flush for the whole alloc
	moveq	#5*4+10,d0			; size of allocation
	bsr	@ClearICache

got_seglist:
	;-- d2 has seglist (BPTR) a2 - new process
	;-- a3 has segarray, d4 has segarray BPTR
	move.l	d4,pr_SegList(a2)	; put in new proc
	lea	_SEGLIST+4(pc),a1	; addr of default segarray

	moveq	#2,d0			; C code we assume, not BCPL
	move.l	d0,(a3)+		; C procs don't use globin
	move.l	(a1)+,d0
	lsr.l	#2,d0
	move.l	d0,(a3)+		; BPTR to klib segment
	move.l	(a1)+,d0
	lsr.l	#2,d0
	move.l	d0,(a3)+		; BPTR to blib segment
	move.l	d2,(a3)+		; our seglist
					; cli entry already NULL due to alloc

	;-- d0 is TOBPTR(blib addr), which means success (!0)
addexit:
	movem.l	(sp)+,d2-d4/a2-a3/a6
	rts
adderr: moveq	#0,d0
	bra.s	addexit

*          CNOP       0,4
****************************************************************
*                                                              *
*  This code is called when a task is activated or deactivated *
*                                                              *
****************************************************************
*
* D0-D7, L, S and R may be used during activation, and P and G are
* used until they are set to the stack and global vector.
*
*
* First find the required global vector size after locating my own
* task structure
*
* Only referenced for starting BCPL tasks from CreateTask - REJ
*
_activecode:
@activecode:
ACTIV
 DBUG <"in new task">
	MOVE.L	SysBase,A6
	MOVE.L	ThisTask(A6),A4
	ADDA.L	#TC_SIZE,A4		; B (A4) is now process ID
					; This means it is the MsgPort - REJ
*
* Set up BCPL registers
*
	SUBA.L	Z,Z
	MOVE.L	TC_SPLOWER-pr_MsgPort(A4),P	; Set up BCPL stack (A1)
	MOVE.L	pr_GlobVec-pr_MsgPort(A4),G	; And global vector (A2)
*
* Now clear BCPL stack
*
	MOVE.L	P,A3			; L (A3) = starting address
	MOVE.L	#$ABABABAB,D3		; Stack pattern
	MOVE.L	SP,D2			; Top of stack
	SUBQ.L	#2,D2			; Less two bytes for safety
ACT10
	MOVE.L	D3,(A3)+		; Clear the BCPL stack
	CMPA.L	D2,A3			; Works for any size stack
	BLT.S	ACT10
*
	MOVE.L	#-1,C_CLLR(P)		; -1 -> root coroutine
	MOVE.L	D2,C_SEND(P)		; Stack end
	MOVE.L	P,D2			; Stack base
	ASR.L	#2,D2			; BCPL address
	MOVE.L	D2,G_STACKBASE(G)	; Patch local GV
*
	MOVE.L	pr_SegList-pr_MsgPort(B),D7	; D7 = BCPL ptr to SEGL
	MOVE.L	D7,D6
	ASL.L	#2,D6			; D6 = MC addr to SEGL
	ADD.L	0(Z,D6.L),D7
	ASL.L	#2,D7			; D7 = MC addr of last entry
	LEA.L	ACT11(PC),R		; Return addr for GLOBIN (A6)
*
ACT11   ADDQ.L	#4,D6			; D6 = MC addr of next entry
	CMP.L	D6,D7
	BLT.S	ACT12			; J if all entries processed
*
* At this point:
*
*      D6 = MC addr of the current entry of SEGL
*      D7 = MC address of the last entry of SEGL
*      Z  = 0 (A0)
*      R  = MC addr of ACT11 (the return addr for GLOBIN) (A6)
*      G  = Globvec pointer (CPTR) in A2
*
* GLOBIN only changes D0, D1, D2, D3, B and L (A3 and A4)
*
*
	MOVE.L	0(Z,D6.L),D1           D1 = BCPL ptr to segment
	BRA	GLOBIN                 GLOBIN(seg) rtn to ACT11
*
*
* We are now ready to call START(TASKWait())
*
ACT12	LEA.L	SAVE,S			; Addr of save routine
	LEA.L	RET,R			; Addr of return routine
*
	; wait for packet, used to call global vec
	; packet MUST be LW aligned!
	MOVEM.L	A0/A1,-(SP)		; save BCPL regs
	BSR	@taskwait		; wait for packet
	MOVEM.L	(SP)+,A0/A1		; restore BCPL regs
	MOVE.L	D0,D1			; BCPL want it in D1
	LSR.L	#2,D1			; BPTR not CPTR!
*
	; we will use BCPL routine to actually start the BCPL task
	MOVE.L	G_START(G),B	   	; call BCPL start code
	MOVEQ	#32,D0			; Stack increment
	JSR	(S)			; Now call START(pkt)
*
* Task has finished normally - free its seglist and globvec if new
* FinalPC code for both CreateTask and CreateProc - REJ
*
_deactcode:
@deactcode:
DEACT
	;-- returncode is in d0, all others assumed trashed
	MOVE.L	SysBase,A6
	MOVE.L	ThisTask(A6),A0		; faster than findtask()
	BSR	@CleanupProc		; do all the cleanup in C
					; returns returncode
	SUB.L	A1,A1
	CALL	RemTask			; kill self

*	RTS				; impossible

*
* CNP_ActiveCode: CreateNewProc startup code.  Sets up a0/d0 from
* pr_Arguments, and then calls the code we want to execute.
*
* No need to restore regs.
*
_CNP_ActiveCode:
@CNP_ActiveCode:
	MOVE.L	SysBase,A6
	MOVE.L	ThisTask(A6),A6
	MOVE.L	pr_Arguments(A6),A0	; argument parameter (NULL if no args)
	MOVE.L	A0,d0			; registers other than a0/d0 are free
	BEQ.S	noargs			; if a0 is NULL d0 is 0

	;-- warning! this uses 2x stack!!!!
	move.l	a0,d3			; save in d3
	BSR	@strlen			; (a0) - length returned in d0
	move.l	d0,d4			; length
					; d3 is already pr_Arguments
	move.l	pr_StackSize(A6),D2	; in bytes
	move.l	pr_SegList(A6),D1	; segarray (BPTR)
	asl.l	#2,d1
	move.l	d1,a0
	move.l	3*4(a0),d1		; seglist (bptr)
	bra	_RunCommand		; returns to DEACT

noargs:	MOVE.L	pr_SegList(A6),D1	; segarray BPTR
	ASL.L	#2,D1			; APTR
	MOVE.L	D1,A1
	MOVE.L	3*4(A1),D1		; Segarray[3], the seglist we should use
	ASL.L	#2,D1			; APTR to first segment
	ADDQ.L	#4,D1			; point to first instruction
	MOVE.L	D1,A1
	JMP	(A1)			; RTS will go to exec returncode (deact)
	
	PAGE
******************************************************************
* 								 *
*        DELETETASK( )                                           *
*                                                                *
* This deletes the current task                                  *
*                                                                *
******************************************************************
	SECT       <'Deletet'>
_deletetask	EQU	DEACT
DELTASK		EQU	DEACT

	PAGE
******************************************************************
*                                                                *
*        OPENDEVICE( IOB, NAME, UNIT, FLAGS )                    *
*                                                                *
* This opens the specified device and returns nonzero if OK      *
* NAME is a BCPL string but terminated with a null a la C        *
*                                                                *
* This will be maintained as a BCPL rtn since C/ASM routines call*
* exec directly.						 *
******************************************************************
	SECT       <'OpenDev'>
OPEND	ASL.L	#2,D2                  Move name to mc ptr
	ADDQ.L	#1,D2                  Avoid length field
	ASL.L	#2,D1                  Move IOB to mc ptr
	MOVE.L	D1,D5                  And save
	MOVEM.L	A0/A1/A6,-(SP)         Save regs
	MOVE.L	SysBase,A6
	MOVE.L	ThisTask(A6),D6
	ADD.L	#TC_SIZE,D6            Now process id
	MOVE.L	D2,A0                  Name to A0
	MOVE.L	D5,A1                  IOB to A1
	MOVE.L	D3,D0                  Unit to D0
	MOVE.L	D4,D1                  Flags to D1
	MOVE.L	D6,MN_REPLYPORT(A1)    Reply port
	MOVE.W	#IO_SIZE-MN_SIZE,MN_LENGTH(A1)  Message length

	CALLS	OpenDevice

	MOVEM.L	(SP)+,A0/A1/A6         Restore regs
	MOVEQ	#-1,D1		    ; assume ok
	TST.L	D0
	BEQ.S	OPEND1		    ; OK
	MOVEQ	#0,D1		    ; error
OPEND1  JMP	(R)		    ; BCPL routine!

	PAGE
******************************************************************
*                                                                *
*        SETIO( IOB, COMMAND, DATA, LENGTH, OFFSET )             *
*                                                                *
* This sets fields as specified in the IOB                       *
*                                                                *
* BCPL routine, not needed by C/asm language programmers	 *
******************************************************************
	SECT       <'SetIo  '>
SETIO	ASL.L	#2,D1                  Move IOB to mc ptr
	MOVE.L	D1,A3                  IOB to A3
	MOVE.W	D2,IO_COMMAND(A3)
	ASL.L	#2,D3                  Convert data ptr to mc
	MOVE.L	D3,IO_DATA(A3)
	MOVE.L	D4,IO_LENGTH(A3)
	MOVE.L	16(P),IO_OFFSET(A3)
	JMP	(R)		    ; BCPL routine!

	PAGE
******************************************************************
*                                                                *
*        error = DoIo( IOB )               			 *
*                                                                *
* This performs the specified IO and waits for it.       	 *
*                                                                *
* BCPL routine, not needed by C/asm language programmers	 *
******************************************************************
	SECT       <'DoIo '>
DOIO	move.l	#_LVODoIO,d0
	; fall through

*
* Takes a function offset in d0, and an argument in d1, and calls the exec
* library entry with the BADDR(d1) in a1.  Moves d0 to d1 on return.
*
bcpl_call_exec:
	movem.l	a0/a1/a6,-(sp)		; Save BCPL regs
	asl.l	#2,d1
	move.l	d1,a1			; turn param into APTR
	move.l	SysBase,A6
	jsr	0(a6,d0.l)		; call exec rtn at offset in d0

	;-- shared cleanup code
restore_bcpl_exit:
	move.l	d0,d1			; BCPL wants returns in d1

restore_bcpl_done:			; for routines that have set rtn or
					; have no return
	movem.l	(sp)+,a0/a1/a6		; Restore regs
	jmp	(R)

	PAGE
******************************************************************
*                                                                *
*        SendIO( IOB, pkt )                                      *
*                                                                *
* This performs the specified IO but does not wait for it.       *
* The pkt will be returned later with the ID field pointing to   *
* the IOB.                                                       *
*                                                                *
* BCPL routine, not needed by C/asm language programmers	 *
* Bizarre stuff occurs with pkt and IOB!			 *
******************************************************************
	SECT       <'SendIo '>
SEND_IO	MOVEM.L	A0/A1/A6,-(SP)         Save regs
	ASL.L	#2,D1                  IOB to mc ptr
	MOVE.L	D1,A1                  IOB to A1
	ASL.L	#2,D2                  Pkt to mc ptr
	MOVE.L	D2,LN_NAME(A1)         pkt address saved in name
	MOVE.L	D1,0(Z,D2.L)           ID field has IOB ptr
	CALLS	SendIO
	BRA.S	restore_bcpl_done

	PAGE
******************************************************************
*                                                                *
*        CLOSEDEVICE( IOB )                                      *
*                                                                *
* This closes the device associated with the IOB                 *
*                                                                *
* BCPL routine, not needed by C/asm language programmers	 *
******************************************************************
CLOSED	move.l	#_LVOCloseDevice,d0
	bra.s	bcpl_call_exec

	PAGE
******************************************************************
*                                                                *
*                 RES := GETMEM(UPPERBOUND,FLAGS)                *
*                 RES := GETVEC(UPPERBOUND)                      *
*                                                                *
* This function is BCPL callable.                                *
* Returns the BCPL pointer to a vector with at least the given   *
* upper bound. The word at offset -1 of the vector contains      *
* the length of the store block and should not be touched by the *
* user. It may be called                                         *
* from machine code using R a the return address.  If no global  *
* vector has been set up then G must be zero.                    *
*                                                                *
* GETVEC used to change: D0, D1, D2, D3, D4 and L                *
* It now changes D0/D1/A0/D1 for @getvec, D0/D1/D2 for GETVEC	 *
*                                                                *
* On return:                                                     *
*      RES ~= 0    OK                                            *
*      RES  = 0    Error                                         *
*                  RESULT2 = 103    Insufficient store           *
*                                                                *
* NOTE: seems to add _2_ longs to all allocations!!!!! REJ 	 *
******************************************************************

	SECT       <'getvec '>
*
GETVEC
	MOVEQ	#MEMF_PUBLIC,D2
GETM	MOVEM.L	A0/A1/A6,-(SP)	; not the same as GETMEM!!!
	MOVE.L	d1,d0
	MOVE.L	d2,d1		; args in d0/d1!
	ADDQ.L	#1,d0
	ASL.L	#2,d0		; add 1 longword and make into bytes
	CALLS	AllocVec	; allocate memory
	MOVE.L	D0,D1		; BCPL return reg
	LSR.L	#2,D1		; return a BPTR!
	BRA.S	restore_bcpl_done

@AllocVecPubClear:
_AllocVecPubClear:
	MOVE.L	#MEMF_PUBLIC+MEMF_CLEAR,d1
	BRA.S	alloc_common

@AllocVecPub:
_AllocVecPub:
	moveq	#MEMF_PUBLIC,d1

alloc_common
	MOVE.L	a6,-(sp)
	CALLS	AllocVec		; d0/d1
restore_a6:
	MOVE.L	(sp)+,a6
	rts

	PAGE
******************************************************************
*                                                                *
*                           FREEVEC(V)                           *
*                                                                *
* This BCPL callable routine frees the vector V, which should    *
* have been obtained from GETVEC. It aborts the task if an error *
* is detected. If the vector is zero the call has no effect      *
* No BCPL stack or Global vector are required.                   *
*                                                                *
* Called with C/asm calling conventions (CPTR)			 *
* Cannot handle vectors of more than 16M-4! FIX			 *
******************************************************************
*
*
	SECT       <'freevec'>
*
FREEVEC:
	move.l	#_LVOFreeVec,d0
	bra.s	bcpl_call_exec

*
* stub for calling freevec to save a few bytes
* takes param in a1
*
_freeVec:
	move.l	a6,-(sp)
	CALLS	FreeVec
	bra.s	restore_a6

	PAGE
******************************************************************
*                                                                *
*                         GLOBIN(SEG)                            *
*                                                                *
* This function initialises the globals defined in the given     *
* segment. It returns -1, or 0 if an error is detected - an      *
* attempt to initialise a global beyond the upperbound given in  *
* GLOBSIZE. GLOBIN is defined in KLIB since it is called from    *
* the scheduler in ACTIV.                                        *
*                                                                *
* GLOBIN must return via register R since this is assumed in     *
* ACTIV.                                                         *
*                                                                *
* GLOBIN changes:  D0, D1, D2, D3, B and L (A3 and A4)           *
*                                                                *
* BCPL routine not called by C/asm language programmers		 *
******************************************************************

	SECT       <'globin '>
*
GLOBIN
	MOVEQ.L	#-1,D0           Success flag initialized to TRUE
*
GLOBIN0	ASL.L	#2,D1
	BEQ.S	GLOBIN3          J if end of section list
	MOVE.L	D1,A4            B  = MC addr of the section
	MOVE.L	(A4)+,D1         D1 = BCPL ptr to next sect (or 0)
	MOVE.L	(A4),D2          D2 = the section length in words
	ASL.L	#2,D2            Now in bytes
	LEA.L	-4(A4,D2.L),A3   L(A3) = MC addr of MAXGLOB word
*
GLOBIN1	MOVE.L	-(A3),D2         D2 = entry relative ptr
	BEQ.S	GLOBIN0          J if end of Gn/Ln pairs
	MOVE.L	-(A3),D3         D3 = the global number (n say)
	CMP.L	(G),D3
	BGT.S	GLOBIN2          Error if n too big
	ADD.L	A4,D2            D2 = MC addr of entry
	ASL.L	#2,D3            D3 = Gvec subscript in bytes
	MOVE.L	D2,0(G,D3.L)     Put entry address in global n
	BRA.S	GLOBIN1          Deal with next Gn/Ln pair
*
GLOBIN2	CLR.L	D0               Indicate error in flag
	MOVE.L	#111,D1	      ; FIX! what error is this?!
	BRA	BCPLERROR
*
GLOBIN3	BSR	     GETDATA	      ; get dosbase into A3
	MOVE.L	A3,G_DOSBASE(A2) ; put dosbase in all global vectors!
	MOVE.L	dl_UtilityBase(A3),G_UTILITYBASE(A2)
	MOVE.L	dl_IntuitionBase(A3),G_INTUITION(A2)
	MOVE.L	D0,D1            ; Return flag as result
	JMP	(R)	      ; BCPL routine!


	PAGE
******************************************************************
*                                                                *
*               Qpkt( pkt )                                      *
*                      d1                                        *
* This BCPL callable routine sends a packet by means of a        *
* message structure pointed at by machine ptr in pkt!pkt.link.   *
* The message is sent to the port identified by pkt!pkt.id, and  *
* this field is swapped to indicate the senders port.            *
*								 *
* In C terms, the packet is sent to the current value of dp_Port,*
* and the dp_Port is replaced with the current process's port.	 *
*                                                                *
* Called with C/asm conventions, pkt = CPTR			 *
* NOTE: packet can't be above $7fffffff!!			 *
* Evil: uses dp_Port, and does a swap with your own port.  Now   *
* also sets mn_ReplyPort.  No new programs should rely on the	 *
* swapping.							 *
******************************************************************
*
*
	SECT       <'qpkt   '>
*
@bcpl_qpkt:
_bcpl_qpkt:
QPKT
	LEA	@qpkt,A3	; a3 is scratch in BCPL
	BRA.S	pktcommon

RETURNPKT
	LEA	@returnpkt,A3

pktcommon:
	MOVEM.L A0/A1/A6,-(SP)
	ASL.L	#2,D1		; convert packet BPTR->cptr
	JSR	(A3)
	bra	restore_bcpl_exit	; qpkt sort of returns a value

;==============================================================================
; success = ReturnPkt( pkt, res1, res2 )
;    d0		       d1   d2    d3
;
; modified by REJ
; Send a packet back to the caller
;==============================================================================
@returnpkt:
_returnpkt:
	MOVE.L	D1,A0
	TST.L	D1		; make sure there is a packet
	BNE.S	10$		; nope, not there
	RTS
10$	MOVEM.L	D2/D3,dp_Res1(a0)	; stash return codes
	;--  fall through!

@qpkt:
_qpkt:
	MOVE.L	D1,A0		; Pkt into address register - MC ptr
	MOVE.L	dp_Link(A0),D0	; Extract message mc address
	BEQ.S	QPKT1		; Invalid message ptr
	MOVE.L	D0,A1		; msg pointer into A1 for putmsg
	MOVE.L	D1,LN_NAME(A1)	; Ptr to pkt in message name field
	MOVE.L	A6,-(SP)	; Save
	MOVE.L	SysBase,A6
	MOVEQ	#pr_MsgPort,D0
	ADD.L	ThisTask(A6),D0	; addr of pr_MsgPort
	MOVE.L	dp_Port(A0),D1	; Port ID into d1
	MOVE.L	D0,dp_Port(A0)	; Replace my port ID in pkt
*	MOVE.L	D0,MN_REPLYPORT(A1) ; Set up so ReplyMsg() works
	MOVE.L	D1,A0		; Move port into a0 for putmsg
	CALL	PutMsg		;  a6 == execbase, a0 = port, a1 = msg
	MOVEQ	#1,D0		; No error occurred
	MOVE.L	(SP)+,A6
	RTS

*
* a2 = pointer to port to return it to, a6 = sysbase
*
@qpkt_task:
_qpkt_task:
	MOVE.L	D1,A0		; Pkt into address register - MC ptr
	MOVE.L	dp_Link(A0),D0	; Extract message mc address
	BEQ.S	QPKT1		; Invalid message ptr
	MOVE.L	D0,A1		; msg pointer into A1 for putmsg
	MOVE.L	D1,LN_NAME(A1)	; Ptr to pkt in message name field
	MOVE.L	dp_Port(A0),D1	; Port ID into d1
	MOVE.L	A2,dp_Port(A0)	; Replace my port ID in pkt
*	MOVE.L	A2,MN_REPLYPORT(A1)	; Set up so ReplyMsg() works
	MOVE.L	D1,A0		; Move port into a0 for putmsg
	CALL	PutMsg		; a6 == execbase, a0 = port, a1 = msg
	MOVEQ	#1,D0		; No error occurred
	RTS

* Invalid port. Set error.
QPKT1	MOVEQ	#101,D1		; FIX! use equate!
	BRA	CERROR

	PAGE
******************************************************************
*                                                                *
*                   Packet := Taskwait()                         *
*                                                                *
* This BCPL callable routine waits for a message to arrive.      *
* It then dequeues the message and returns the packet pointed    *
* at by the node name field. The message can be retrieved if     *
* required by following the link field of the packet.            *
*                                                                *
* Converted to C/asm conventions				 *
* NOTE: funny use of ln_Name!					 *
*								 *
* If called from a task, assume that a1 is port address		 *
* should only happen from DoPkt() (sendpkt)			 *
******************************************************************
*
*
	SECT       <'taskwai'>
*
TASKWAI
	MOVE.L	R,-(sp)			; return addr
	; fall thru
@taskwait:
	MOVEM.L	D7/a0/a1/A6,-(SP)	; Save regs (incl BCPL regs)
	MOVE.L	SysBase,A6
	MOVE.L	ThisTask(A6),A0
	CMP.B	#NT_PROCESS,LN_TYPE(a0)
	BNE.S	is_task			; if not process skip PktWait
	ADD.W	#TC_SIZE,A0		; get pr_MsgPort addr

* See if pktwait function to be called instead
	MOVE.L	pr_PktWait-pr_MsgPort(A0),D1
	BEQ.S	TWait0			; Zero so no special action
	MOVE.L	D1,A0			; Address to call
	JSR	(A0)			; Call it, returns msg in D0
	BRA.S	TWait3

is_task:
	MOVE.L	a1,a0			; for call from task, a1 = port!
TWait0:
	MOVE.L	A0,D7			; Save port address

* Signal can be set from any point now. See if message waiting already.
TWait1:	MOVE.L	D7,A0			; Retrieve port address
	CALL	GetMsg			; a6 = execbase
	TST.L	D0 
	BEQ.S	TWait2			; No message this time

* Message received. Return packet ptr held in name field.
TWait3:	MOVE.L	D0,A0
	MOVE.L	LN_NAME(A0),D0		; Extract packet mc address
	MOVE.L	D0,D1			; For BCPL
	ASR.L	#2,D1			; Convert to BCPL address
	MOVEM.L	(SP)+,D7/a0/a1/A6	; Restore regs
	RTS

* Wait for message to arrive.
TWait2:	MOVE.L	#SIGMASK,D0		; Wait for signal
	CALL	Wait			; a6 = execbase
	BRA.S	TWait1			; Now go get message

	PAGE
*****************************************************************
*                                                               *
*                 RES := SETFLAGS(TASKID,FLAGS)                 *
*                                                               *
* Sets flags of the specified task. We use signal numbers       *
* 12, 13, 14, 15 for this purpose.                              *
*                                                               *
* BCPL routine, not for C/asm programmers                       *
*****************************************************************

	SECT       <'setflag'>
*
SETFLG	MOVEM.L	a0/A1/A6,-(SP)		; Save regs
	MOVE.L	MP_SIGTASK(Z,D1.L),A1	; Get Task Struct
	MOVEQ	#$0F,D0			; Ensure only 4 set
	AND.L	D2,D0			; Mask to D0
	MOVEQ	#12,D3			; Shift required
	LSL.L	D3,D0			; Shift flags up
	CALLS	Signal
SETFLG1
	bra	restore_bcpl_done	; restore regs, jmp (r)

	PAGE
*****************************************************************
*                                                               *
*               RES := TESTFLAGS(FLAGS)                         *
*                                                               *
* Tests and clears the flags of the current task.               *
* On return:                                                    *
*    RES  = FALSE (=0)  None of the specified flags were set    *
*    RES  = TRUE (-1)   At least one specified flag was set     *
*                                                               *
* BCPL routine, not for C/asm programmers			*
*****************************************************************

	SECT       <'testfla'>
*
TSTFLG	MOVEM.L	a0/A1/A6,-(SP)
	MOVEQ	#0,D0		; Signals are to be cleared
	MOVEQ	#12,D2		; Shift required
	MOVEQ	#$F,D3		; Ensure only 4 signals used
	AND.L	D3,D1
	MOVE.L	D1,D3		; Save mask
	LSL.L	D2,D1		; Move mask up
	CALLS	SetSignal	; Clear specified flags, return setting
	MOVEQ	#0,D1		; Anticipate answer FALSE
	LSR.L	D2,D0		; Shift old signals down
	AND.L	D3,D0		; Mask with request
	BEQ.S	TSTFLG1		; None set
	MOVEQ	#-1,D1		; Return TRUE if some set
TSTFLG1
	BRA.S	SETFLG1		; Restore & return

	PAGE
******************************************************************
*                                                                *
*           Res := EXEC( CODE, D0, D1, A0, A1, A2 )              *
*                                                                *
* This BCPL callable routine provides a general interface to the *
* ROM routines provided by EXEC.                                 *
*                                                                *
* BCPL routine, unneeded by C/asm programmers			 *
******************************************************************
*
*
	SECT       <'Exec   '>
*
EXEC    MOVEM.L	A0-A2/A6,-(SP)	; Save regs
	MOVE.L	D1,D7		; Save request code
	MOVE.L	D2,D0		; Arg2 is D0
	MOVE.L	D3,D1		; Arg3 is D1
	MOVE.L	D4,A0		; Arg4 is A0
	MOVE.L	20(P),A2	; Arg6 is A2
	MOVE.L	16(P),A1	; Arg5 is A1
	MOVE.L	SysBase,A6	; Library pointer
	JSR	0(A6,D7.W)	; Call routine
	MOVEM.L	(SP)+,A0-A2/A6	; Restore regs
	MOVE.L	D0,D1		; Result in D1
	JMP	(R)		; And return

	PAGE
******************************************************************
*                                                                *
*           Res := CALLRES( LIB, CODE, D0, D1, A0, A1 )          *
*                                                                *
* This BCPL callable routine provides a general interface to the *
* ROM routines provided by other libraries.                      *
*                                                                *
* BCPL only routine, unneeded by C/asm programmers		 *
******************************************************************
*
*
	SECT       <'Callres'>
*
CALLRES	MOVEM.L	A0/A1/A6,-(SP)		; Save regs
	MOVE.L	D1,A6			; Library pointer to A6
	MOVE.L	D2,D7			; Save request code
	MOVE.L	D3,D0			; Arg3 is D0
	MOVE.L	D4,D1			; Arg4 is D1
	MOVE.L	16(P),A0		; Arg5 is A0
	MOVE.L	20(P),A1		; Arg6 is A1
	JSR	0(A6,D7.W)		; Call it
	bra	restore_bcpl_exit

	PAGE
******************************************************************
*                                                                *
*         Res := GETLONG(Base, Byteoffset)                       *
*                PUTLONG(Base, Byteoffset,Val)                   *
*                                                                *
* These BCPL callable routines are used to access long sized     *
* values which are not long word aligned                         *
*                                                                *
* BCPL only routines, unneeded by C/asm programmers		 *
******************************************************************
*
*
	SECT       <'Getlong'>
*
GETLONG	ASL.L	#2,D1		; mc ptr
	ADD.L	D2,D1		; add byte offset
	MOVE.L	0(Z,D1.L),D1	; get lword
	JMP	(R)
*
	SECT       <'Putlong'>
*
PUTLONG	ASL.L	#2,D1		; mc ptr
	ADD.L	D2,D1		; add byte offset
	MOVE.L	D3,0(Z,D1.L)	; put lword
	JMP	(R)		; And return

	PAGE
*
* These routines provide BCPL io support with read only globals
* C/BCPL routines
*
_CurrentDir:
	MOVE.W	#pr_CurrentDir-pr_MsgPort,D0
	BRA.S	selcommon
*
@SetProgramDir:
_SetProgramDir:
	MOVE.W	#pr_HomeDir-pr_MsgPort,D0
	BRA.S	selcommon
*
@SetArgStr:
_SetArgStr:
	MOVE.W	#pr_Arguments-pr_MsgPort,D0
	BRA.S	selcommon
*
@setfilesys:
_setfilesys:
	MOVEQ	#pr_FileSystemTask-pr_MsgPort,D0
	BRA.S	selcommon
*
@setconsole:
_setconsole:
	MOVEQ	#pr_ConsoleTask-pr_MsgPort,D0
	BRA.S	selcommon
*
SELIN	MOVE.L	R,-(SP)
@selectinput:
_selectinput:
	MOVEQ	#pr_CIS-pr_MsgPort,D0
	BRA.S	selcommon
*
SELOUT	MOVE.L	R,-(SP)			; return addr
@selectoutput:
_selectoutput:
	MOVEQ	#pr_COS-pr_MsgPort,D0
selcommon
	BSR.S	@getid
	MOVE.L	0(a0,d0.w),-(sp)	; get old value
	MOVE.L	D1,0(a0,d0.w)		; set with new value
	MOVE.L	(sp)+,D0		; return old value
	MOVE.L	D0,D1			; BCPL return in D1
	SUB.L	A0,A0			; BCPL needs A0 == 0
	RTS

******************************************************************
*                                                                *
* Routines to provide information from the process structure     *
* Returns task id (msgport) in L (A3)				 *
*                                                                *
* BCPL routine, C/asm use FindTask()				 *
******************************************************************
*
* id := taskid()
*
* Return process id
* BCPL routine
*
@taskid:
	BSR.S	@getid
	MOVE.L	A0,D0
	RTS

TASKID	BSR.S	GETID
	MOVE.L	A3,D1		; L
	JMP	(R)

*
* Service routine to locate my process id, and return it in L
* No other regs affected
*
GETID	MOVE.L	SysBase,A3
	MOVE.L	ThisTask(A3),A3
	ADD.W	#TC_SIZE,A3	; pointer to messageport
	RTS
*
* Faster than FindTask(0), smaller than define
*
@myproc	MOVE.L	SysBase,A0
	MOVE.L	ThisTask(A0),d0
	RTS
*
* C version: returns msgport in A0!
* Musn't modify d0/d1/a1!
*
@getid	MOVE.L	SysBase,A0
	MOVE.L	ThisTask(A0),A0
	ADD.W	#TC_SIZE,A0	; pointer to messageport
	RTS

*
* lock = GetProgramDir()
*
* Return lock on home directory of program - do NOT unlock!
*
@GetProgramDir
	MOVE.W	#pr_HomeDir-pr_MsgPort,D0
	BRA.S	ProcCommon
*
* string = ArgStr()
*
* Return pointer to initial argument string for process
*
@ArgStr
	MOVE.W	#pr_Arguments-pr_MsgPort,D0
	BRA.S	ProcCommon
*
* list := seglist()
*
* Return segment list
*
SEGL   	  MOVE.L	R,-(SP)		; return addr
@seglist:
	MOVE.W	#pr_SegList-pr_MsgPort,D0
	BRA.S	ProcCommon
*
* task := consoletask()
*
* Return console task id
*
CTASK	MOVE.L	R,-(SP)		; return addr
@consoletask:
	MOVE.W	#pr_ConsoleTask-pr_MsgPort,D0
	BRA.S	ProcCommon
*
* result2 = getresult2()
*
* Return result2 for a task
*
GETR2	MOVE.L	R,-(SP)		; return addr
@getresult2:
	MOVE.W	#pr_Result2-pr_MsgPort,D0
	BRA.S	ProcCommon
*
* input fh = input()
*
* Return pr_CIS for a task
*
INPUT	MOVE.L	R,-(SP)		; return addr
@input:
	MOVE.W	#pr_CIS-pr_MsgPort,D0
	BRA.S	ProcCommon
*
* output fh = output()
*
* Return pr_COS for a task
*
OUTPUT	MOVE.L	R,-(SP)		; return addr
@output:
	MOVE.W	#pr_COS-pr_MsgPort,D0
	BRA.S	ProcCommon
*
* cli pointer = cli()
*
* Return pr_CLI for a task (CPTR for C, BPTR for bcpl)
*
CLI	MOVE.L	R,-(SP)		; return addr
@cli:
	MOVE.W	#pr_CLI-pr_MsgPort,D0
	BSR.S	ProcCommon		; returns in both D0/D1
	LSL.L	#2,D0			; so C/asm get CPTR, BCPL get BPTR!
	RTS
*
* task := filesystemtask()
*
* Return filing system task id
*
FTASK	MOVE.L	R,-(SP)		; return addr
@filesystemtask:
	MOVE.W	#pr_FileSystemTask-pr_MsgPort,D0

*
* must preserve a0/a1 for BCPL
*
ProcCommon:
	BSR.S	@getid		; returns msgport addr in a0
	MOVE.L	0(A0,D0.W),D1	; get value out of process structure
	MOVE.L	D1,D0		; for C/asm
	SUB.L	A0,A0		; restore Z reg for BCPL
	RTS
*
* rc := RESULT2(alter,val)
*
* Returns value of RESULT2, and sets it if ALTER is true
* C/BCPL routine(s)
* MUST NOT TOUCH A1! 
*
@SetIoErr:				; for dos.library
	MOVE.L	D1,D0
@setresult2:				; param in D0!!!!!
	MOVE.L	D2,-(SP)
	MOVE.L	D0,D2
	MOVEQ	#-1,D1			; force set of result2
	BSR.S	@result2
	MOVE.L	(SP)+,D2
	RTS

RES2	BSR.S	@result2		; destroys A0

*
* common restore a0 and exit routine for bcpl
*
bcplcommon
	SUB.L	a0,a0
	JMP	(R)
	
@result2:
	BSR.S	@getid				; Get ID into a0
	LEA.L	pr_Result2-pr_MsgPort(a0),a0	; Point to val
	BRA.S	UPDTE
*
* rc := currentdir(alter,val)
*
* Returns value of currentdir, and alters it
* C/BCPL routine
* MUST NOT TOUCH A1! 
*
CDIR	BSR.S	@currentdir
	BRA.S	bcplcommon
@currentdir:
_currentdir:
	BSR	@getid
	LEA.L	pr_CurrentDir-pr_MsgPort(a0),a0

UPDTE	TST.L	D1		; Update?
	BNE.S	UPDTE1		; No, return value
	MOVE.L	(a0),D1
	MOVE.L	D1,D0
	RTS

UPDTE1	MOVE.L	(a0),D1
	MOVE.L	D2,(a0)		; return old value!
	MOVE.L	D1,D0		; for BCPL and C
	RTS

*
*
* This service routine returns with the address of the DOS
* data area in L (A3). All registers are saved.
*
* opendosbase can be called from tasks!
*
@opendosbase:
	move.l	a3,d0		; save a3 (GETDATA hits it, saves all others)
	bsr.s	GETDATA
	exg.l	a3,d0		; swap a3 and d0, restoring a3, setting return
	rts

GETDATA
	MOVEM.L	D0/D1/A0/A1/A6,-(SP)
	LEA	LIBNAME(PC),A1	; dos.library
	MOVEQ	#0,D0		; Version
	CALLS	OpenLibrary
	MOVE.L	D0,A3		; L
	MOVE.L	A3,A1
	CALL	CloseLibrary	; Keep use count correct
	MOVEM.L	(SP)+,D0/D1/A0/A1/A6
	RTS

*
* Uses the pr_Globvec from your process
* only touches a0/d0!!!!
*
@dosbase:
	MOVE.L	SysBase,A0
	MOVE.L	ThisTask(A0),A0
	MOVE.L	pr_GlobVec(A0),A0
	MOVE.L	G_DOSBASE(A0),D0
	RTS

*
* This routine stores a value in Intuitionbase in the globvec
* Needed by BCPL routines.  Takes parameter in A0!
*
@store_IBase:
	MOVE.L	SysBase,A1
	MOVE.L	ThisTask(A1),A1
	MOVE.L	pr_GlobVec(A1),A1
	MOVE.L	A0,G_INTUITION(A1) ; store it
	MOVE.L	G_DOSBASE(a1),a1
	MOVE.L	a0,dl_IntuitionBase(a1)
	RTS
*
* Call setprefs for me (gets ibase from globvec) (a0 = prefs, d0 = size)
*
@MySetPrefs:
	MOVE.L	A6,-(A7)
	MOVE.L	SysBase,A6
	MOVE.L	ThisTask(A6),A6
	MOVE.L	pr_GlobVec(A6),A6
	MOVE.L	G_INTUITION(A6),A6		; GET IBASE IN a6
	MOVEQ	#1,D1				; notify
	JSR	_LVOSetPrefs(A6)
	MOVE.L	(A7)+,A6
	RTS
*
* Routine to return pointer to root structure
* BCPL and C versions
*
ROOTN	BSR.S	@rootstruct
	LSR.L	#2,D0
	MOVE.L	D0,D1
	SUB.L	A0,A0
	JMP	(R)

@rootstruct:
	BSR.S	@dosbase
	MOVE.L	D0,A0
	MOVE.L	dl_Root(A0),D0   Pointer to rootnode
	RTS

	PAGE
	TTL        "Error Handler"
*************************************************************************
*
* This section of code gets called when an exception is encountered.
* It saves all the registers and then calls the routine Abort.
*
AbortHandler
	MOVE.L	a6,-(sp)		; save a reg so we can play
	MOVE.L	USP,a6
	MOVEM.L	D0-D7/A0-A6,-(a6)	; note: a6 we pushed is wrong
	MOVE.L	(sp)+,(8+6)*4(a6)	; skip 8 data regs, 6 addr regs
	MOVE.L	(SP)+,d1		; Code into arg

* update USP
	MOVE.L	a6,USP			; update the USP so we can switch

* we now are pointing the exception frame (@sp)  We want to drop it.
* Punt, it's too hard for the gain.  Punch the upper SSP bound into SSP
	move.l	$4,a6
	move.l	SysStkUpper(a6),SP

* switch back to user mode
	ANDI.W	#(~$2000),SR

* Get Exec's handler, and patch it in to stop runaway
	MOVE.L	TaskTrapCode(A6),A2
	MOVE.L	ThisTask(A6),A0
	MOVE.L	A2,TC_TRAPCODE(A0)

* Now call Abort( Code )
	BSET.L	#31,D1			; Add AT_DeadEnd bit
	BSR.S	@abort			; Call Abort
* abort should never come back!


******************************************************************
*                                                                *
*           ABORT( Code )                                        *
*                                                                *
* This BCPL callable routine displays a requestor, then when the *
* user presses Continue it calls an Alert if it was called with  *
* a DOS code, and returns to caller otherwise. The message is    *
* slightly different depending on whether it was a disk error.   *
*                                                                *
* We want it to look like this:					 *
*	Software Failure					 *
*								 *
*	            Professional Page				 *
*	     has failed (error code #00000000)			 *
*	       Wait for disk activity to end			 *
*								 *
******************************************************************
*
*
ABORT
	MOVE.L	R,-(SP)
@abort
_abort
	;-- movem transfers a6 first, moving down.  leaves sp->d1
	MOVEM.L	D1-D7/A0/A1/A6,-(SP)	; save regs for below and BCPL
	move.l	d1,d5

	CMPI.L	#AN_KeyRange!AT_DeadEnd,D1	; Disk error?
	BNE.S	get_name			; No..

	MOVEQ	#STR_DISK_CORRUPT,d1
	BSR	@getstring
	move.l	d0,a1
	bra.s	gotname

	;-- get the program name (or process name)
get_name:
	move.l	$4,a6
	move.l	ThisTask(a6),a0
	move.l	LN_NAME(a0),a1
	move.l	a1,d0
	bne.s	goodname
	lea.l	corrupt_name(pc),a1
goodname:
	cmp.b	#NT_PROCESS,LN_TYPE(a0)
	bne.s	gotname

	;-- process
	move.l	pr_CLI(a0),d0
	asl.l	#2,d0
	beq.s	gotname			; no cli, use task name
	move.l	d0,a0
	move.l	cli_CommandName(a0),d0
	asl.l	#2,d0
	beq.s	gotname			; sanity check
	move.l	d0,a1
	moveq	#0,d0
	move.b	(a1)+,d0		; get length of BSTR
	clr.b	0(a1,d0)		; terminate it

gotname:	;-- name is in a1
	move.l	a1,d6			; will go in d1 later

	MOVEQ	#STR_TASK_FAILED,d1
	BSR	@getstring
	MOVE.L	D0,D2

	MOVEQ	#STR_WAIT_DISK,d1
	BSR	@getstring
	MOVE.L	D0,D3

	MOVE.L	D6,D1			; put it back

	MOVE.L	#1<<14,D4	  ; NEWPREFS - really means use Suspend|Reboot
	BSR	@requester2	  ; d5 is also a parameter - the error

* If the user presses Suspend, do Wait(0)

	TST.L	D0
	Beq.S	reboot
	MOVEQ	#0,d0
	CALLS	Wait		; never returns

* If the user presses Cancel, we call an Alert if DOS errors or
* return otherwise.
reboot:
	MOVE.L	d5,D7		; Code passed. If top bit set then Alert
	BPL.S	ABORT2

* non-recoverable: call alert
* setup of a5 unneeded in 2.0, but lets do it anyways (4 bytes)

	MOVE.L	SysBase,a6
	LEA	ThisTask(a6),a5		; pointer to task that errored
	CALL	Alert

ABORT2  MOVEM.L	(SP)+,D1-D7/A0/A1/A6
	RTS

corrupt_name:
	dc.b	0			; FIX! put something here
	ds.w	0

	PAGE
	SECT       <'RunComm'>
**************************************************************************
*
* rc := runcommand( seg, stacksize, paramptr, paramlen )
* BCPL:		    BPTR   LONGS      BPTR      LONG
* C:		    BPTR   BYTES      CPTR      LONG
*
* This code runs a command under TRIPOS in a language independent
* fashion. A stack of specified size is constructed, and the entry point
* of the code is assumed to be the first executable word of the first
* hunk in the segment list (which is a list of code blocks linked by BCPL
* pointers). All TRIPOS registers are saved, and the code is entered with
* the SP at the base of the new stack, having pushed a suitable return
* address, the size of the stack and the caller's stack onto it first of all.
* In addition the previous return address for STOP is saved, and a new
* one installed.
*
* Thus a program running under Tripos will find a stack as follows
*
* ReturnAddress    StackSize   CallerStack
*
* The registers are set up as follows (any register may be corrupted
* except SP).
* 
* D0  Size of parameter string in bytes
* A0  Pointer to parameter string area
*
* pr_Arguments is set to the arguments and restored afterwards
* NOTE: arguments are a CSTR!
* 
* The routine returns
* -1  if the program could not be loaded (result2() has more info)
* 0   if the program ran and did not set a return code
* >0  if the program set a return code (result2() normally has more info)
*

RUNC
	MOVE.L	R,-(SP)			; return addr
	ASL.L	#2,D2			; stacksize to bytes
	ASL.L	#2,D3			; BPTR to CSTR -> CPTR to CSTR
					; fall thru

_InternalRunCommand:
@InternalRunCommand:
	MOVEM.L	D2-D7/A0-A6,-(SP)	; save tripos regs too
*
* set up BCPL environment!!!!!!!!!!!!
* Actually, that would be hard.  People like Manx fexec WILL break, I'm afraid,
* just like they did for 1.2.
* I'll try though....
*
* Note that I put the BCPL a1 (P) on the NEW stack, not the old one,
* because I can't get a hold of the calling BCPL processes a1 to use here.
* Possible solution: seperate runcommands (and fault!) for BCPL and C.
* ARGH!!!!!!! (FIX)
*
*Manx code:
*	ret_val = _doexec(len, stksiz, stksiz+8, len, arg, (seg+1)<<2, stk.ll);
*...
*	link	a5,#0
*	movem.l	d1/a6,-(sp)
*	movem.l	d3-d7/a2-a5,-(sp)		;save registers
*	move.l	sp,__mysp				;save our sp
*	movem.l	8(a5),d0/d2/d3/d4/a0/a4/a7	;load params
*	move.l	4(sp),a3				;get old sp from CLI
*	movem.l	4(a3),a1/a2/a5/a6		;get BCPL environment
*	move.l	d0,12(a1)				;set length
*	move.l	a0,d1					;copy to dreg
*	lsr.l	#2,d1					;convert to BPTR
*	move.l	d1,8(a1)				;set ptr
*	move.l	a0,d1					;copy to d1 as well
*	jsr	(a4)
*
	BSR	GETDATA			; Get dosbase in a3
	MOVEM.L	dl_A2(a3),a2/a5/a6
	BSR	GETID			; Get process ID (MsgPort) in L (A3)

*
* Save pr_Arguments for restoration later, set new value
*
	MOVE.L	pr_Arguments-pr_MsgPort(a3),-(SP)
	MOVE.L	D3,pr_Arguments-pr_MsgPort(a3)

* NOTE that getvec allocated 2 LWs more than asked - 1 for len, 1 extra!
* VERY important: BCPL-startup.asm relies on this!!!

	MOVE.L	D1,D7		; save for later
	MOVE.L	D2,D0		; Stacksize in D2 (bytes)
	ADDQ.L	#4,D0		; since we now use Exec AllocVec
	BSR	@AllocVecPub	; get memory
	TST.L	D0		; Check worked
	BEQ.S	RUNERRL		; Error

* Save old stack boundaries on old stack
	move.l	TC_SPLOWER-pr_MsgPort(a3),-(sp)
	move.l	TC_SPUPPER-pr_MsgPort(a3),-(sp)
	move.l	a3,-(sp)		; so we can grab it later

* now finish setting up BCPL/Manx fexec environment...
* fixing fexec probably won't work, but worth a try.
*
	MOVE.L	d0,a1			; this must be maintained until jsr
	ADDQ.W	#4,a1			; For safety, I think it can be munged
					; otherwise.
	MOVEM.L	a1/a2/a5/a6,-(sp)	; Only for compatibility

* now finish setting up the new stack

	ADD.L	D2,D0		; D0 is now high stack address
	ADDQ.L	#4,D0		; Now top stack ptr (bytes) (extra LW)
	MOVE.L	D0,A4		; Into B (A4)

	MOVE.L	d7,d1
	MOVEM.L	d1-d4,(a1)	; Store params for EVIL programs!!!!
		
	MOVE.L	pr_ReturnAddr-pr_MsgPort(A3),-(SP) ; Save old return address
	MOVE.L	SP,-(A4)		; Push my old stack ptr onto new stack
	MOVE.L	D2,-(A4)		; Push stacksize onto new stack (bytes)
*
* Remember the base stack in the process base for STOP.
*
	MOVE.L	A4,pr_ReturnAddr-pr_MsgPort(A3)	; Store new returnaddr stack
; FIX! protection from here to sp_xxx swap!!!
	MOVE.L	A4,SP		; New stack ptr
	move.l	a1,TC_SPLOWER-pr_MsgPort(a3)
	lea	8(sp),a0	; old stack pointer and stack size
	move.l	a0,TC_SPUPPER-pr_MsgPort(a3)
	ASL.L	#2,D7		; Segment as mc ptr
	MOVE.L	D7,A3		; Save seglist in A3
	MOVE.L	D3,A0		; CPTR to params into A0
	MOVE.L	D4,D0		; Args size
	JSR	4(A3)		; Hold tight - may whomp registers
*
* The loaded command should return here. Convention indicates that D0
* contains the return code. SP should be set to point to the stacksize.
*
RETADD	MOVEM.L	(SP)+,D2/D3	; Extract stack size and my stack
	MOVE.L	SP,D1		; Save old stack ptr
; FIX! protection from here to sp_xxx swap!
	MOVE.L	D3,SP		; New stack now
	MOVE.L	(SP)+,D7	; Remember previous returnaddr
	ADD.W	#16,SP		; Dump 4 regs pushed on for compatibility
	move.l	(sp)+,a3	; get process msgport addr back
	move.l	(sp)+,TC_SPUPPER-pr_MsgPort(a3)
	move.l	(sp)+,TC_SPLOWER-pr_MsgPort(a3)
	MOVE.L	D7,pr_ReturnAddr-pr_MsgPort(A3)	; Restore saved return address
	MOVE.L	D0,-(SP)	; Save return code - TRICKY!
	SUB.L	D2,D1		; D1 = Low stack + 1 LW
	SUBQ.L	#4,D1		; Now Stack Base
	MOVE.L	D1,A1		; FreeVec is in exec
	CALLS	FreeVec		; free stack (a6 is scratch here)
	MOVE.L	(SP)+,D0	; return code
restore_exit:
*
* restore pr_Arguments
*
	MOVE.L	(SP)+,pr_Arguments-pr_MsgPort(a3)

	MOVEM.L	(SP)+,D2-D7/A0-A6	; restore all C/asm regs
	MOVE.L	D0,D1			; for BCPL
	RTS
*
* Here if out of memory
*
RUNERRL MOVEQ	#-1,D0
	BRA.S	restore_exit

	PAGE
**********************************************************************
*                                                                    *
*       STOP(RC)                                                     *
*                                                                    *
*    Returns from either a command called through RUNCOMMAND         *
*    or deactivates a process. It simply returns to the return       *
*    address saved on the stack pointed to by pr_ReturnAddr-pr_MsgPort-4 *
*                                                                    *
* Always has been a C/asm routine interface			     *
**********************************************************************
@exit:
NSTOP   BSR	@getid		; Get my ID
	MOVE.L	pr_ReturnAddr-pr_MsgPort(A0),D0	; New stack ptr
	SUBQ.L	#4,D0		; Point to return address
	MOVE.L	D0,SP
	MOVE.L	D1,D0		; Return code into D0
	SUB.L	A0,A0		; restore the Z reg for BCPL
	RTS			; Return to saved address (usually DEACT)

	PAGE
**********************************************************************
*		Globals to be initialised                            *
*                                                                    *
**********************************************************************

	IFD KLUDGE
	INCLUDE	"downc.asm"
	INCLUDE	"mlib.asm"
	ENDC

	CNOP	0,4		; Align
GLOBEND	DC.L	0		; End of the global list

; globals from downc.asm:

	dc.l	G_CTOB/4,(BCPLCtoB-KLIB)
	dc.l	G_BTOC/4,(BCPLBtoC-KLIB)
	dc.l	G_CLEARVEC/4,(CLEARVEC-KLIB)
	dc.l	G_COPYSTRING/4,(COPYSTRING-KLIB)
	dc.l	G_COPYVEC/4,(COPYVEC-KLIB)
	dc.l	G_UNPACKSTRING/4,(UNPACKSTRING-KLIB)
	dc.l	G_PACKSTRING/4,(PACKSTRING-KLIB)
	dc.l	G_CAPITALCH/4,(CAPITALCH-KLIB)
	dc.l	G_COMPCH/4,(COMPCH-KLIB)
	dc.l	G_COMPSTRING/4,(COMPSTR-KLIB)

; globals from mlib.asm:

	dc.l	G_MULTIPLY/4,(MULTIPLY-KLIB)
	dc.l	G_DIVIDE/4,(DIVIDE-KLIB)
	dc.l	G_REMAINDER/4,(REMAINDER-KLIB)
	dc.l	G_GBYTES/4,(GBYTES-KLIB)
	dc.l	G_PBYTES/4,(PBYTES-KLIB)
	dc.l	G_GETBYTE/4,(GETBYTE-KLIB)
	dc.l	G_PUTBYTE/4,(PUTBYTE-KLIB)
	dc.l	G_GET2BYTES/4,(GET2BYTES-KLIB)
	dc.l	G_PUT2BYTES/4,(PUT2BYTES-KLIB)
	dc.l	G_LEVEL/4,(LEVEL-KLIB)
	dc.l	G_LONGJUMP/4,(LONGJUMP-KLIB)
	dc.l	G_APTOVEC/4,(APTOVEC-KLIB)
	dc.l	G_CREATECO/4,(CREATECO-KLIB)
	dc.l	G_DELETECO/4,(DELETECO-KLIB)
	dc.l	G_COWAIT/4,(COWAIT-KLIB)
	dc.l	G_RESUMECO/4,(RESUMECO-KLIB)
	dc.l	G_CALLCO/4,(CALLCO-KLIB)

; globals from klib(doslib):

	dc.l	G_GLOBIN/4,(GLOBIN-KLIB)	; pure BCPL
	dc.l	G_GETMEM/4,(GETM-KLIB)		; BCPL
	dc.l	G_GETVEC/4,(GETVEC-KLIB)	; BCPL
	dc.l	G_FREEVEC/4,(FREEVEC-KLIB)	; BCPL
	dc.l	G_OPENDEVICE/4,(OPEND-KLIB)	; pure BCPL
	dc.l	G_SETIO/4,(SETIO-KLIB)		; pure BCPL
	dc.l	G_DOIO/4,(DOIO-KLIB)		; pure BCPL
	dc.l	G_SENDIO/4,(SEND_IO-KLIB)	; pure BCPL
	dc.l	G_CLOSEDEVICE/4,(CLOSED-KLIB)	; pure BCPL
	dc.l	G_CREATETASK/4,(CRTASK-KLIB)	; interface
	dc.l	G_ICREATEPROC/4,(CRPROC-KLIB)	; interface
	dc.l	G_DELETETASK/4,(DELTASK-KLIB)	; pure BCPL/C
	dc.l	G_SETFLAGS/4,(SETFLG-KLIB)	; pure BCPL
	dc.l	G_TESTFLAGS/4,(TSTFLG-KLIB)	; pure BCPL
	dc.l	G_ABORT/4,(ABORT-KLIB)		; BCPL
	dc.l	G_ROOTSTRUCT/4,(ROOTN-KLIB)	; BCPL
	dc.l	G_TASKWAIT/4,(TASKWAI-KLIB)	; BCPL
	dc.l	G_PKTWAIT/4,(TASKWAI-KLIB)	; BCPL (dup of taskwait)
	dc.l	G_QPKT/4,(QPKT-KLIB)		; BCPL
	dc.l	G_RETURNPKT/4,(RETURNPKT-KLIB)	; BCPL
	dc.l	G_EXEC/4,(EXEC-KLIB)		; pure BCPL
	dc.l	G_CALLRES/4,(CALLRES-KLIB)	; pure BCPL
	dc.l	G_RESULT2/4,(RES2-KLIB)		; BCPL
	dc.l	G_CURRENTDIR/4,(CDIR-KLIB)	; BCPL
	dc.l	G_TASKID/4,(TASKID-KLIB)	; BCPL
	dc.l	G_SEGLIST/4,(SEGL-KLIB)		; BCPL
	dc.l	G_SELECTINPUT/4,(SELIN-KLIB)	; BCPL
	dc.l	G_SELECTOUTPUT/4,(SELOUT-KLIB)	; BCPL
	dc.l	G_INPUT/4,(INPUT-KLIB)		; BCPL
	dc.l	G_OUTPUT/4,(OUTPUT-KLIB)	; BCPL
	dc.l	G_CONSOLETASK/4,(CTASK-KLIB)	; BCPL
	dc.l	G_FILESYSTEMTASK/4,(FTASK-KLIB)	; BCPL
	dc.l	G_GETLONG/4,(GETLONG-KLIB)	; pure BCPL
	dc.l	G_PUTLONG/4,(PUTLONG-KLIB)	; pure BCPL
	dc.l	G_RUNCOMMAND/4,(RUNC-KLIB)	; BCPL
	dc.l	G_STOP/4,(NSTOP-KLIB)		; pure
	dc.l	G_CLI/4,(CLI-KLIB)		; BCPL

	DC.L	G_GLOBMAX/4		; Highest referenced global

KEND      END
