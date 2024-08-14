*************************************************************************
*                                                                       *
* Copyright (C) 1985-1991 Commodore Amiga Inc.  All rights reserved.    *
*                                                                       *
*************************************************************************

*************************************************************************
*
* $Id: disks.asm,v 39.3 92/07/01 20:13:01 mks Exp $
*
* $Log:	disks.asm,v $
* Revision 39.3  92/07/01  20:13:01  mks
* Fixed autodoc typos
* 
* Revision 39.2  92/03/12  11:22:29  mks
* Fixed branches that could be short to save ROM space
*
* Revision 39.1  91/11/08  14:23:56  mks
* Major autodoc cleanup
*
* Revision 39.0  91/10/28  16:25:49  mks
* First release of native build V39 expansion code
*
*************************************************************************

	NOLIST
	include "exec/types.i"
	include "exec/lists.i"
	include "exec/interrupts.i"
	include "exec/libraries.i"
	include "exec/nodes.i"
	include "exec/execbase.i"
	include "exec/ables.i"
	include "exec/memory.i"

	include "libraries/dos.i"
	include "libraries/dosextens.i"
	include "libraries/filehandler.i"
;	include "libraries/romboot_base.i"

	include "expansion.i"
	include "private_base.i"

	include "asmsupp.i"
	include "messages.i"
	LIST


	XLIB	AllocEntry
	XLIB	AllocMem
	XLIB	OpenLibrary
	XLIB	FreeMem
	XLIB	CloseLibrary
	XLIB	Enqueue
	XLIB	Open

	XLIB	Debug

	XDEF	MakeDosNode
	XDEF	AddDosNode
	XDEF	AddBootNode

	TASK_ABLES



******* expansion.library/MakeDosNode *********************************
*
*   NAME
*	MakeDosNode -- construct dos data structures that a disk needs
*
*   SYNOPSIS
*	deviceNode = MakeDosNode( parameterPkt )
*	D0			  A0
*
*	struct DeviceNode * MakeDosNode( void * );
*
*   FUNCTION
*	This routine manufactures the data structures needed to enter
*	a dos disk device into the system.  This consists of a DeviceNode,
*	a FileSysStartupMsg, a disk environment vector, and up to two
*	bcpl strings.  See the libraries/dosextens.h and
*	libraries/filehandler.h include files for more information.
*
*	MakeDosNode will allocate all the memory it needs, and then
*	link the various structure together.  It will make sure all
*	the structures are long-word aligned (as required by the DOS).
*	It then returns the information to the user so he can
*	change anything else that needs changing.  Typically he will
*	then call AddDosNode() to enter the new device into the dos
*	tables.
*
*   INPUTS
*	parameterPkt - a longword array containing all the information
*	    needed to initialize the data structures.  Normally I
*	    would have provided a structure for this, but the variable
*	    length of the packet caused problems.  The two strings are
*	    null terminated strings, like all other exec strings.
*
*	    longword	description
*	    --------	-----------
*	    0		string with dos handler name
*	    1		string with exec device name
*	    2		unit number (for OpenDevice)
*	    3		flags (for OpenDevice)
*	    4		# of longwords in rest of environment
*	    5-n 	file handler environment (see libraries/filehandler.h)
*
*   RESULTS
*	deviceNode - pointer to initialize device node structure, or
*	    null if there was not enough memory.  You may need to change
*	    certain fields before passing the DeviceNode to AddDosNode().
*
*   EXAMPLES
*	/* set up a 3.5" Amiga format floppy drive for unit 1 */
*
*	char execName[] = "trackdisk.device";
*	char dosName[] = "df1";
*
*	ULONG parmPkt[] = {
*	    (ULONG) dosName,
*	    (ULONG) execName,
*	    1,			/* unit number */
*	    0,			/* OpenDevice flags */
*
*	    /* here is the environment block */
*	    16, 		/* table upper bound */
*	    512>>2,		/* # longwords in a block */
*	    0,			/* sector origin -- unused */
*	    2,			/* number of surfaces */
*	    1,			/* secs per logical block -- leave as 1 */
*	    11, 		/* blocks per track */
*	    2,			/* reserved blocks -- 2 boot blocks */
*	    0,			/* ?? -- unused */
*	    0,			/* interleave */
*	    0,			/* lower cylinder */
*	    79, 		/* upper cylinder */
*	    5,			/* number of buffers */
*	    MEMF_CHIP,		/* type of memory for buffers */
*	    (~0 >> 1),          /* largest transfer size (largest signed #) */
*	    ~1, 		/* addmask */
*	    0,			/* boot priority */
*	    0x444f5300, 	/* dostype: 'DOS\0' */
*	};
*
*	struct Device Node *node, *MakeDosNode();
*
*	node = MakeDosNode( parmPkt );
*
*
*   SEE ALSO
*	AddDosNode, libraries/dosextens.h, libraries/filehandler.h
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*
*/


;------- we use an MemList structure to actually allocate our
;------- memory in one fell swoop
MDN_DEVNODE	EQU	0
MDN_STARTUP	EQU	1
MDN_ENVIRON	EQU	2
MDN_DOSNAME	EQU	3
MDN_EXECNAME	EQU	4
MDN_NUMENTRIES	EQU	5

;------- parameterPkt
O_DOSNAME	EQU	0
O_EXECNAME	EQU	4
O_UNITNUM	EQU	8
O_OPENFLAGS	EQU	12
O_BLOCKSIZE	EQU	16

MakeDosNode:
		movem.l a2/a3/a4/a6,-(sp)
		move.l	ABSEXECBASE,a6
		move.l	a0,a2

		lea	-(ML_SIZE+(ME_SIZE*MDN_NUMENTRIES))(sp),sp

		;------ allocate all the memory we will need.  This
		;------ needs to be in two parts, because the device name
		;------ may be separately freed.
		move.w	#MDN_NUMENTRIES,ML_NUMENTRIES(sp)
		lea	ML_ME(sp),a1
		move.l	#MEMF_PUBLIC!MEMF_CLEAR,d1

		move.l	d1,(a1)+        ; MDN_DEVNODE reqs
		move.l	#DevList_SIZEOF,(a1)+

		;------ only allocate the startup message if he asked for it
		move.l	d1,(a1)+        ; MDN_STARTUP reqs
		move.l	#FileSysStartupMsg_SIZEOF,(a1)+


		move.l	d1,(a1)+        ; MDN_ENVIRON reqs
		move.l	O_BLOCKSIZE(a2),d0      ; compute size on the fly
		addq.l	#1,d0			; convert from upper bound
		lsl.l	#2,d0			;    to size
		move.l	d0,(a1)+

		;------ allocate enough room for the strings
		move.l	d1,(a1)+        ; MDN_DOSNAME reqs
		move.l	(a2),a0         ; O_DOSNAME(a2)
		bsr	slen
		move.l	d0,(a1)+

		move.l	d1,(a1)+        ; MDN_EXECNAME reqs
		move.l	O_EXECNAME(a2),a0
		bsr	slen
		move.l	d0,(a1)+

		;------ now try and actually get the memory
		move.l	sp,a0
		CALLSYS AllocEntry

		;------ clean up the stack
		lea	(ML_SIZE+(ME_SIZE*MDN_NUMENTRIES))(sp),sp

		;------ check for error
		btst	#31,d0
		bne.s	mdn_err

		;------ base of MemList in a4, DosNode struct in a3
		move.l	d0,a4
		move.l	ML_ME+(ME_SIZE*MDN_DEVNODE)+ME_ADDR(a4),a3

		;------ lets take care of the two names
		move.l	ML_ME+(ME_SIZE*MDN_DOSNAME)+ME_ADDR(a4),a1
		move.l	(a2),a0                 ; O_DOSNAME(a2)
		bsr.s	ctobcopy
		move.l	d0,dn_Name(a3)		;BSTR with NULL after it.
		IFGE	INFOLEVEL-200
		  MOVE.L  A0,-(SP)
		  ADDQ.L  #1,A0
		  MOVE.L  A0,-(SP)
		  INFOMSG 200,<'MakeDosNode dn_Name %s (%lx)'>
		  ADDQ.L  #8,SP
		ENDC

		move.l	ML_ME+(ME_SIZE*MDN_EXECNAME)+ME_ADDR(a4),a1
		move.l	O_EXECNAME(a2),a0
		bsr.s	ctobcopy
		;-- fudge fssm_Device BSTR to include NULL in count (Bryce)
		IFNE	OnePointFourDOS	;bump fssm_Device BSTR count blindly
		addq.b	#1,(a0)         ;bump fssm_Device BSTR count blindly
		ENDC			;Kodiak made me do it :-)
		IFGE	INFOLEVEL-200
		  MOVE.L  A0,-(SP)
		  ADDQ.L  #1,A0
		  MOVE.L  A0,-(SP)
		  INFOMSG 200,<'MakeDosNode fssm_Device %s (%lx)'>
		  ADDQ.L  #8,SP
		ENDC

		;------ now do startup message
		move.l	ML_ME+(ME_SIZE*MDN_STARTUP)+ME_ADDR(a4),a0
		move.l	d0,fssm_Device(a0) ;BSTR with NULL included in count(!)

		move.l	O_UNITNUM(a2),fssm_Unit(a0)
		move.l	O_OPENFLAGS(a2),fssm_Flags(a0)
		move.l	ML_ME+(ME_SIZE*MDN_ENVIRON)+ME_ADDR(a4),a1
		move.l	a1,d0
		lsr.l	#2,d0
		move.l	d0,fssm_Environ(a0)
		move.l	a0,d0
		lsr.l	#2,d0
		move.l	d0,dn_Startup(a3)

		;------ copy in environment vector
		lea	O_BLOCKSIZE(a2),a0
		move.l	(a0),d0
lcopy_loop:
		move.l	(a0)+,(a1)+
		dbra	d0,lcopy_loop

		;------ finish off DeviceNode
		move.l	#600,dn_StackSize(a3)
		move.l	#10,dn_Priority(a3)

		;------ free the alloc'ed entry
		moveq	#ML_ME+(ME_SIZE*MDN_NUMENTRIES),d0
		move.l	a4,a1
		CALLSYS FreeMem

		;------ set up our return value
		move.l	a3,d0

mdn_end:
		movem.l (sp)+,a2/a3/a4/a6
		rts

mdn_err:
		moveq	#0,d0
		bra.s	mdn_end


****************** support routines ***************************


slen:		; ( a0: string ) -- return length including null + bcpl len

		move.l	a0,d0
		beq.s	slen_end

		moveq	#-1,d0

slen_loop:
		tst.b	(a0)+
		dbeq	d0,slen_loop

		not.l	d0	; strlen
		addq.l	#2,d0	; plus null & length byte

slen_end
		rts



ctobcopy:	; ( a0: cstring-source, a1: bstring-dest )
		; return D0=bptr to bstring-dest
		;	 A0=cptr to bstring-dest

		move.l	a0,d0
		beq.s	ctobcopy_end

		;------ reserve space for bcpl length
		addq.l	#1,a1

		moveq	#-1,d0
ctobcopy_loop:
		move.b	(a0)+,(a1)+
		dbeq	d0,ctobcopy_loop

		;------ get length in d0
		move.l	d0,d1
		not.l	d0		; strlen

		;------ get address of beginning of string & update length
		lea	-1(a1,d1.l),a0
		move.b	d0,(a0)

		;------ convert to bptr
		move.l	a0,d0
		lsr.l	#2,d0

ctobcopy_end:
		rts


******* expansion.library/AddBootNode ********************************
*
*   NAME
*	AddBootNode -- Add a BOOTNODE to the system (V36)
*
*   SYNOPSIS
*	ok = AddBootNode( bootPri, flags, deviceNode, configDev )
*	D0		  D0	   D1	  A0	      A1
*
*	BOOL AddBootNode( BYTE,ULONG,struct DeviceNode *,struct ConfigDev * );
*
*   FUNCTION
*	This function will do one of two things:
*
*		1> If dos is running, add a new disk type device immediatly.
*		2> If dos is not yet running, save information for later
*		   use by the system.
*
*   FUNCTION
*	This routine makes sure that your disk device (or a device
*	that wants to be treated as if it was a disk...) will be
*	entered into the system.  If the dos is already up and
*	running, then it will be entered immediately.  If the dos
*	has not yet been run then the data will be recorded, and the
*	dos will get it later.
*
*	We try and boot off of each device in turn, based on priority.
*	Floppies have a hard-coded priority.
*
*	There is only one additional piece of magic done by AddBootNode.
*	If there is no executable code specified in the deviceNode
*	structure (e.g. dn_SegList, dn_Handler, and dn_Task are all
*	null) then the standard dos file handler is used for your
*	device.
*
*	Documentation note: a "task" as used here is a dos-task, not
*	an exec-task.  A dos-task, in the strictest sense, is the
*	address of an exec-style message port.	In general, it is
*	a pointer to a process's pr_MsgPort field (e.g. a constant
*	number of bytes after an exec task).
*
*	Autoboot from an expansion card before DOS is running requires
*	the card's ConfigDev structure.
*
*	Pass a NULL ConfigDev pointer to create a non-bootable node.
*
*   INPUTS
*	bootPri -- a BYTE quantity with the boot priority for this disk.
*	    This priority is only for which disks should be looked at:
*	    the actual disk booted from will be the first disk with
*	    a valid boot block.  If no disk is found then the "bootme"
*	    hand will come up and the bootstrap code will wait for
*	    a floppy to be inserted.  Recommend priority assignments are:
*
*		+5   -- unit zero for the floppy disk.	The floppy should
*			always be highest priority to allow the user to
*			abort out of a hard disk boot.
*		 0   -- the run of the mill hard disk
*		-5   -- a "network" disk (local disks should take priority).
*		-128 -- don't even bother to boot from this device.
*
*	flags -- additional flag bits for the call:
*	    ADNF_STARTPROC (bit 0) -- start a handler process immediately.
*		Normally the process is started only when the device node
*		is first referenced.  This bit is meaningless if you
*		have already specified a handler process (non-null dn_Task).
*
*	deviceNode -- a legal DOS device node, properly initialized.
*		Typically this will be the result of a MakeDosNode().
*		Special cases may require a custom-built device node.
*
*	configDev -- a valid board's ConfigDev structure.  This is required
*		for autoboot before DOS is running and if left NULL will
*		result in an non-bootable node.
*
*   RESULTS
*	ok - non-zero everything went ok, zero if we ran out of memory
*	    or some other weirdness happened.
*
*   NOTE
*	This function eliminates the need to manually Enqueue a BOOTNODE
*	onto an expansion.library list.  Be sure V36 expansion.library is
*	available before calling this function!
*
*   SEE ALSO
*	AddDosNode
*
**********************************************************************

******* expansion.library/AddDosNode *********************************
*
*   NAME
*	AddDosNode -- mount a disk to the system
*
*   SYNOPSIS
*	ok = AddDosNode( bootPri, flags, deviceNode )
*	D0		 D0	  D1	 A0
*
*	BOOL AddDosNode( BYTE,ULONG,struct DeviceNode *);
*
*   FUNCTION
*	This is the old (pre V36) function that works just like
*	AddBootNode().  It should only be used if you *MUST* work
*	in a 1.3 system and you don't need to autoboot.
*
*   RESULTS
*	ok - non-zero everything went ok, zero if we ran out of memory
*	    or some other weirdness happened.
*
*   EXAMPLE
*	/* enter a bootable disk into the system.  Start a file handler
*	** process immediately.
*	*/
*	if(  AddDosNode( 0, ADNF_STARTPROC, MakeDosNode( paramPacket ) )  )
*		...AddDosNode ok...
*
*   BUGS
*	Before V36 Kickstart, no function existed to add BOOTNODES.
*	If an older expansion.library is in use, driver code will need
*	to manually construct a BootNode and Enqueue() it to eb_Mountlist.
*	If you have a V36 or better expansion.library, your code should
*	use AddBootNode().
*
*   SEE ALSO
*	MakeDosNode, AddBootNode
*
**********************************************************************
*
*   IMPLEMENTATION NOTES
*
*
*   REGISTER USAGE
*		; D0 -> D2 = bootpri
*		; D1 -> D3 = flags
*		;       A1 = New bootnode
*		; A0 -> A2 = devicenode
*		; A1 -> A3 = configdev
*
*/

dosName:	DOSNAME
		ds.w	0

*		; BootNode structure now included from expansion_private.i
*
* STRUCTURE BootNode,LN_SIZE
*    UWORD	bn_Flags
*    CPTR	bn_DeviceNode
*    LABEL	BootNode_SIZEOF
*


AddDosNode:	sub.l	a1,a1	;Same as NewAddDosNode with no ConfigDev!
AddBootNode:

		IFGE	INFOLEVEL-200
		  MOVE.L   A1,-(SP)
		  MOVE.L   A0,-(SP)
		  MOVE.L   D1,-(SP)
		  MOVE.L   D0,-(SP)
		  INFOMSG  200,<'AddDosNode: Pri %lx Flags %lx DevNode %lx Configdev %lx'>
		  ADD.L    #4*4,SP
		ENDC

		movem.l d2/d3/a2/a3,-(sp)

		;------ save the arguments
		move.l	d0,d2
		move.l	d1,d3
		move.l	a0,a2
		move.l	a1,a3

		;------ test for null DeviceNode
		move.l	a0,d0
		beq.s	adn_end		;D0=NULL

		;------ try and add the request NOW
		bsr.s	enterDosNode
		tst.l	d0
		bne.s	adn_success

		;------ the dos wasn't there.  link the driver in
		move.l	#MEMF_CLEAR,d1	;So LN_TYPE will get cleard -Bryce
		moveq	#BootNode_SIZEOF,d0
		LINKEXE AllocMem
		move.l	d0,a1
		tst.l	d0
		beq.s	adn_end		;D0=NULL
		;[A1-BootNode]

		move.l	a3,d0		;Check configdev
		bne.s	adn_BuildBootNode



	;------ Build a regular non-bootable BootNode
		;clr.b	LN_TYPE(a1)
		move.b	d2,LN_PRI(a1)
		move.l	dn_Name(a2),d0
		lsl.l	#2,d0
		addq.l	#1,d0
		move.l	d0,LN_NAME(a1)
		move.w	d3,bn_Flags(a1)
		move.l	a2,bn_DeviceNode(a1)
		bra.s	adn_queueit
	;^^^^^^ End regular node


	;------ Build node with the silly LN_NAME kludge (a bootable BootNode)
adn_BuildBootNode:
		move.b	#NT_BOOTNODE,LN_TYPE(a1)
		move.b	d2,LN_PRI(a1)
		move.l	a3,LN_NAME(a1)          ;ConfigDev (Ugh! Kludge!)
		move.w	d3,bn_Flags(a1)
		move.l	a2,bn_DeviceNode(a1)
	;^^^^^^ End boot node



adn_queueit:	;------ link the node in [A1]
		lea.l	eb_MountList(a6),a0     ;List
		;This Enqueue() had no protection! -Bryce
		move.l	ABSEXECBASE,a3
		exg.l	a3,a6
		FORBID
		CALLSYS	Enqueue 		;Queue it
		PERMIT
		exg.l	a3,a6

adn_success:	;------ enter an OK return code
		moveq	#-1,d0
adn_end:	movem.l (sp)+,d2/d3/a2/a3
		rts


;******************** support routines *****************************

enterDosNode:	; ( d2:bootpri, d3:flags, a2:deviceNode )
		movem.l	a2/a3/d2/d3,-(sp)

		;------ try and open the DOS
		lea	dosName(pc),a1
		CLEAR	d0
		LINKEXE OpenLibrary

		tst.l	d0
		beq.s	edn_fail

		;
		;	:TODO: Randell would like to see direct DOS fiddling
		;	removed eventually.
		;
		;------ now we're cooking!
		move.l	d0,a3

		;------ get the root node
		move.l	dl_Root(a3),a1

		;------ the rule was: if all of handler, task, and seglist
		;------ is null, then give him a file system seglist
		tst.l	dn_Task(a2)
		bne.s	edn_gotcode
		tst.l	dn_Handler(a2)
		bne.s	edn_gotcode
		tst.l	dn_SegList(a2)
		bne.s	edn_gotcode

		move.l	rn_FileHandlerSegment(a1),dn_SegList(a2)

edn_gotcode:
		FORBID	a0		;:OPTIMIZE:
		move.l	rn_Info(a1),d0
		lsl.l	#2,d0
		move.l	d0,a0

		;------ a0 has the base of the DosInfo structure. insert
		;------ the new device node.
		move.l	di_DevInfo(a0),(a2)
		move.l	a2,d1
		lsr.l	#2,d1
		move.l	d1,di_DevInfo(a0)

		PERMIT	a0		;:OPTIMIZE:

		;------ parse the flags
		btst	#ADNB_STARTPROC,d3
		beq.s	edn_close

		;
		;	d1 - BPTR to devicenode
		;	d2 - scratch
		;	a2 - APTR to devicenode
		;	a3 - dos.library
		;
		bsr.s	DosCall

edn_close
		;------ close the library
		move.l	a3,a1
		LINKEXE CloseLibrary

		moveq	#-1,d0	;Yo!  CloseLibrary does not guarantee a return.
				;The "no dos" path from this function was
				;NEVER TAKEN!
edn_end:	movem.l	(sp)+,a2/a3/d2/d3
		rts

edn_fail:	moveq	#0,d0	;:OPTIMIZE: (just remove it)
		bra.s	edn_end




	IFEQ	OnePointFourDOS
;
;	d1 - BPTR to devicenode
;	d2 - scratch
;	a2 - APTR to devicenode
;	a3 - dos.library
;
DosCall:	; ( call dos by global # (in d0), doslib in a3 )  !EVIL!
		; ( d1 has BPTR to device node)			   !EVIL!
		moveq	#112,d0 	;loaddevice global vector.  !EVIL!
		MOVEM.L D3/D4,-(SP)

		exg	a3,a6
		JSR	_LVOOpen+2(A6)	; magic -- two into dos call
		exg	a3,a6

		MOVEM.L (SP)+,D3/D4
		RTS
	ENDC


	IFNE	OnePointFourDOS
;
;DosCall- ask device load & start the specified device
;
;	d1 - BPTR to devicenode
;	d2 - scratch
;	a2 - APTR to devicenode
;	a3 - dos.library
;
	XLIB	GetDeviceProc
	XLIB	FreeDeviceProc

MAXDEVICENAME	EQU	32
STACKSAVE	EQU	MAXDEVICENAME+2

DosCall:	exg	a3,a6

		lea.l	-STACKSAVE(sp),sp

			move.l	dn_Name(a2),a0		;A0 = BPTR to name
			adda.l	a0,a0
			adda.l	a0,a0
			cmp.b	#MAXDEVICENAME,(a0)+	;A0 = APTR to cstring
			bhi.s	dc_toobig

			move.l	sp,a1
dc_next:		move.b	(a0)+,(a1)+
			bne.s	dc_next
			move.b  #':',-1(a1)
			clr.b	(a1)

			move.l	sp,d1		;D1 = APTR to "devicename:"
			moveq	#0,d2		;D2 = NULL
			jsr	_LVOGetDeviceProc(a6)
			move.l	d0,d1
			jsr	_LVOFreeDeviceProc(a6)
dc_toobig:


		lea.l	STACKSAVE(sp),sp

		exg	a3,a6
		rts
	ENDC


	END
