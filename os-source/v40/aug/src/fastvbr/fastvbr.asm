		TTL		FastVBR
*
*******************************************************************************
*									      *
*	FastVBR 1.0							      *
*									      *
*	Copyright (c) 1989 by Michael Sinz    MKSoft Development	      *
*									      *
*	AmigaDOS EXEC release 1.2 or greater...				      *
*									      *
*******************************************************************************
*									      *
*	To assemble, I used HX68 (ADAPT)				      *
*									      *
*	# HX68  68k MakeFile rule...					      *
*	.asm:								      *
*		@HX68 -a $*.asm -i include: -cqsy -o $*.o		      *
*		@BLink $*.o lib lib:amiga.lib to $* sc sd nd		      *
*		@List $*						      *
*									      *
*	It should assemble without any problems on most 680x0 assemblers      *
*	The code is position independant and thus needs an assembler that     *
*	can produce such files.						      *
*									      *
*******************************************************************************
*									      *
*	This program moves the VBR to FAST memory if it is not there	      *
*									      *
*******************************************************************************
*									      *
*	Reading legal mush can turn your brain into guacamole!		      *
*									      *
*		So here is some of that legal mush:			      *
*									      *
* Permission is hereby granted to distribute this program's source	      *
* executable, and documentation for non-comercial purposes, so long as the    *
* copyright notices are not removed from the sources, executable or	      *
* documentation.  This program may not be distributed for a profit without    *
* the express written consent of the author Michael Sinz.		      *
*									      *
* This program is not in the public domain.				      *
*									      *
* Fred Fish is expressly granted permission to distribute this program's      *
* source and executable as part of the "Fred Fish freely redistributable      *
* Amiga software library."						      *
*									      *
* Permission is expressly granted for this program and it's source to be      *
* distributed as part of the Amicus Amiga software disks, and the	      *
* First Amiga User Group's Hot Mix disks.				      *
*									      *
*******************************************************************************
*
* The following INCLUDE files are needed to make this program assemble.
* They come with the Amiga Macro-Assembler Package.
*
	NOLIST					; No need to list these
	INCLUDE	"exec/types.i"
	INCLUDE	"exec/memory.i"
	INCLUDE	"exec/execbase.i"
	INCLUDE	"libraries/dosextens.i"
	LIST
*
*******************************************************************************
*
* This is the only fixed address in the system and it even "floats"...
*
		xref	_AbsExecBase
*
*******************************************************************************
*
* Some macros that make calling the system routines easier...
*
CALLSYS		MACRO
		xref	_LVO\1		; Set the external reference
		jsr	_LVO\1(a6)
		ENDM
*
JUMPSYS		MACRO
		xref	_LVO\1		; Set the external reference
		jmp	_LVO\1(a6)
		ENDM
*
* Now, for the start of the code...
*
FastVBR:	move.l	_AbsExecBase,a6		; Get the EXEC library base
*
		clr.l	d7		; Clear d7...
*
		move.l	d7,a1		; Clear a1
		CALLSYS	FindTask	; Get our task pointer...
		move.l	d0,a4		; Now, move it to a1 for addressing use
		lea	pr_MsgPort(a4),a3
		tst.l	pr_CLI(a4)	; Check if this is a CLI task...
		bne.s	QuickCLI	; If so, skip the next section
*
*******************************************************************************
*
* This section deals with starting from the WorkBench...
* It is just here for completeness...
*
QuickWorkBench:	move.l	a3,a0		; Get message port
		CALLSYS	WaitPort	; Wait for the WorkBench message...
		move.l	a3,a0		; ...it's here, so let's get it
		CALLSYS	GetMsg		; off the message port...
		bra.s	QuickCont	; ...and go to the continue routine
*
*******************************************************************************
*
* The routine was started from the CLI  (prefered)
* Let's store a NULL pointer so that there is no WB message...
*
QuickCLI:	move.l	d7,d0		; No reply message...
*
*******************************************************************************
*
* Continue with the Quick initialization
*
QuickCont:	move.l	d0,-(sp)	; Save the message pointer...
*
*******************************************************************************
*
* Check if we are running already...
*
		btst.b	#AFB_68010,AttnFlags+1(a6)
		beq.s	Bail_Out	; If no 68010 or better, no VBR...
*
		lea	GetVBR(pc),a5	; Routine...
		CALLSYS	Supervisor	; Call it...
		tst.l	d0		; Check if we are still at location 0
		bne.s	Bail_Out	; If not, bail out...
*
* Ok, so we need to allocate 1K of RAM for the vectors to be placed into...
*
		move.l	#$400,d0	; Number of bytes...
		move.l	#MEMF_PUBLIC|MEMF_FAST,d1	; Type of memory
		CALLSYS	AllocMem	; Get the memory
		tst.l	d0		; Check for result...
		beq.s	Bail_Out	; If no fast memory, no need to move...
		move.l	d0,a0		; Get the register set up...
		move.w	#$100-1,d1	; Number of vectors-1  (256 vectors)
		lea	MoveVBR(pc),a5	; Get routine address
		CALLSYS	Supervisor	; Do the copy...
*
* If we wanted to, we could state that the VBR was moved at this point...
*
		; If you want some output...
*
*******************************************************************************
*
* Now, exit as needed...
*
Bail_Out:	move.l	(sp)+,d0	; Get that message back
		beq.s	abortNoWB	; If none, we are done
		move.l	d0,a1		; Get the pointer ready
		JUMPSYS	ReplyMsg	; ...when returning WB message
abortNoWB:	rts			; RTS out of this task...
*
*******************************************************************************
*
		OPT	p=68010		; 68010 processor assembly
*
*******************************************************************************
*
* Supervisor mode get the vbr...
*
GetVBR:		movec.l	vbr,a0		; Get current VBR
		move.l	a0,d0		; And store it...
		rte			; Return...
*
*******************************************************************************
*
* Supervisor mode - Tricky moving of the vbr...
*
MoveVBR:	or.w	#$0700,sr	; DISABLE, rte will restore SR...
*
* We need to do all of this in full disable to be safe
*
		movec.l	vbr,a1		; Get old VBR (it may have changed)
		movec.l	a0,vbr		; Save new VBR (hope for no NMI ;^)
copy_loop:	move.l	(a1)+,(a0)+	; Copy it up...
		dbra.s	d1,copy_loop	; Do all of them...
*
* Ok, now we may need to flush the caches...
*
		btst.b	#AFB_68040,AttnFlags+1(a6)	; Check for 68040
		beq.s	MoveVBR_done	; If not 68040, exit...
*
* We need to make sure that the data cache has been pushed into physical RAM
*
		OPT	p=68040		; 68040 assembly
		cpusha	DC		; push the data cache (68040 or better)
		OPT	p=68010		; Back to 68010 assembly...
*
MoveVBR_done:	rte			; And we are done.
*
*******************************************************************************
*
* For the VERSION command...
*
		include	"FastVBR_rev.i"
		VERSTAG
*
*******************************************************************************
*
* And, with that we come to the end of another full-length feature staring
* that wonderful MC680x0 and the Amiga system.  Join us again next time
* for more Wonderful World of Amiga...  Until then, keep boinging...
*
		end
