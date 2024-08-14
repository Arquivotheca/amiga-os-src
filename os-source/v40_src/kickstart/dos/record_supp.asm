*
* record_supp.asm
* 
*
        INCLUDE "exec/types.i"
        INCLUDE "exec/nodes.i"
        INCLUDE "exec/lists.i"
        INCLUDE "exec/ports.i"
        INCLUDE "exec/libraries.i"
        INCLUDE "exec/tasks.i"
        INCLUDE "exec/memory.i"
        INCLUDE "libraries/dos.i"
        INCLUDE "libraries/dosextens.i"
        INCLUDE "exec/funcdef.i"
        INCLUDE "exec/exec_lib.i"
        INCLUDE "libraries/dos_lib.i"

	INCLUDE "libhdr.i"

callsys macro
        CALLLIB _LVO\1
        endm
        
*
* blib, etc routines
*
BLIB	MACRO	;\1 - symbolname
	XREF	_\1
@\1	EQU	_\1
	ENDM

**

	XREF	@sendpacket

**

	XDEF	@LockRecord,_LockRecord

*
* LONG ASM
* LockRecord (REG(d1) BPTR fh, REG(d2) LONG position, REG(d3) LONG length,
* 	    REG(d0) LONG mode, REG(a0) LONG timeout)  /* ran out of regs */
* {
* 	struct FileHandle *file = (struct FileHandle *) BADDR(fh);
* 
* 	return sendpkt5(file->fh_Type,ACTION_LOCK_RECORD,file->fh_Arg1,
* 			position,length,mode,timeout);
* }
*

*
* args come in regs d1-d5
*

@LockRecord:
_LockRecord:
	movem.l	d2-d7,-(a7)

	asl.l	#2,d1		; turn fh into CPTR
	move.l	d1,a0

	move.l	d5,d7		; shift args
	move.l	d4,d6
	move.l	d3,d5
	move.l	d2,d4

	move.l	fh_Arg1(a0),d3
	move.l	#ACTION_LOCK_RECORD,d2
	move.l	fh_Type(a0),d1
	bsr	@sendpacket

	movem.l	(a7)+,d2-d7
	rts


	END
