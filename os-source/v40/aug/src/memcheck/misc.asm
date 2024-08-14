;*****************************************************************************/
; misc.asm
;*****************************************************************************/

	INCLUDE	"exec/types.i"
	INCLUDE "exec/tasks.i"
	INCLUDE	"exec/memory.i"
	INCLUDE "dos/dos.i"
	INCLUDE	"dos/dosextens.i"
	INCLUDE	"globaldata.i"

;*************************************************************************************
; void ASM MemoryCorrupt (REG (a2) struct MemHeader *mh);
;*************************************************************************************
	XDEF	_MemoryCorrupt
;*****************************************************************************/
	XREF	_kprintf
	XREF	corrupt
;*****************************************************************************/
_MemoryCorrupt:
	; Memory list is corrupt
	move.l	MH_FREE(a2),-(a7)
	move.l	MH_UPPER(a2),-(a7)
	move.l	MH_LOWER(a2),-(a7)
	pea	corrupt(pc)
	jsr	_kprintf
	lea	16(a7),a7		; restore the stack
	rts

;*************************************************************************************
; void ASM MungMem (REG (a0) char *ptr, REG (d0) ULONG size, REG (d1) ULONG value);
;*************************************************************************************
	XDEF	_MungMem
;*****************************************************************************/
_MungMem:
	addq.l	#7,d0			; round to the nearest
	lsr.l	#3,d0			; double-longword
	bra.s	__MungMem_2
__MungMem_1:
	move.l	d1,(a0)+
	move.l	d1,(a0)+
__MungMem_2:
	dbra	d0,__MungMem_1
	sub.l	#$10000,d0
	bpl.s	__MungMem_1
	rts

;*****************************************************************************/
; ULONG ASM TestMem (REG (a0) VOID *memb, REG (a6) struct Library *SysBase);
;*****************************************************************************/
;	1 = Odd
;	2 = Invalid
;*****************************************************************************/
	XDEF	_TestMem
;*****************************************************************************/
	XREF	_LVOTypeOfMem
;*****************************************************************************/
_TestMem:
	move.l	a0,d1				; Move it to a register we can test
	btst.l	#0,d1				; Is the address odd?
	bne	_IsOdd				; It is, so we are invalid
	move.l	a0,a1				; TypeOfMem needs the address in A1
	jsr	_LVOTypeOfMem(a6)		; Now see if the address is in any memory list
	tst.l	d0
	bne.b	_Good
	moveq.l	#2,d0				; It's invalid memory
	rts
_IsOdd:
	moveq.l	#1,d0				; It's odd memory
	rts
_Good:
	moveq.l	#0,d0
	rts

;*****************************************************************************/
; STRPTR ASM GetProcessName (REG (a0) struct Task *tc);
;*****************************************************************************/
	XDEF	_GetProcessName
;*****************************************************************************/
_GetProcessName:
	movea.l	a0,a1				; Make a copy of the task pointer
	cmpi.b	#NT_PROCESS,LN_TYPE(a1)		; if (tc->tc_Node.ln_Type == NT_PROCESS)
	bne.s	justtask			; just a task
	move.l	pr_CLI(a1),d0			; if (pr->pr_CLI)
	beq.s	justtask			; just a task

	move.l  d0,d1				; Make sure we have...
	andi.l	#$C0000000,d1			; a valid BPTR
	bne.s	justtask			; not, so treat like a task

	lsl.l	#2,d0				; Convert BPTR to CLI pointer
	move.l	d0,a1				; copy the CLI pointer to a1
	move.l	cli_CommandName(a1),d0		; if (cli->cli_CommandName)
	beq.s	justtask			; not, so treat like a task

	move.l  d0,d1				; Make sure we have...
	andi.l	#$C0000000,d1			; a valid BPTR
	bne.s	justtask			; not, so treat like a task

	lsl.l	#2,d0				; CommandName bptr to ptr
	movea.l	d0,a1				; ptr command name bstr in a1
	moveq.l #0,d0				; clear out d0
	move.b	(a1)+,d0			; len to d0 & bump a1 to 1st char
	beq.s	justtask			; name is empty
	clr.b	0(a1,d0.l)			; null terminate command name
	bra.s	tailpath2			; try using the tail of a path name

tailpath:
	cmpi.b	#'/',0(a1,d0.w)
	beq.s	tailpath3
	cmpi.b	#':',0(a1,d0.w)
	beq.s	tailpath3
tailpath2:
	dbra.s	d0,tailpath
	bra.s	tailpath4

tailpath3:
	addq.l	#1,d0				; result 0 if no / or :
	adda.l	d0,a1				; tailpath
tailpath4:
	move.l	a1,d0				; commandname to d0
	bra.s   gotname

justtask:
	move.l	LN_NAME(a0),d0			; Use the task name
gotname:
	move.l	d0,a0				; move name to a0
	tst.l	d0
	bne.s	notnull
	lea.l	nullname(pc),a0			; if null, supply <null> name
notnull:
	cmpi.l	#$DEADBEEF,(a0)			; if points to beef, show this
	bne.s	notdead1
	lea.l	deadname(pc),a0
	bra.s	gotdone
notdead1:
	cmpi.l	#$ADBEEFDE,(a0)			; show BPTR's to deadbeef
	bne.s	gotdone
	lea.l	adbename(pc),a0
gotdone:
	move.l	a0,d0
	rts

;*****************************************************************************/

nullname	dc.b	'<NULL>',0
deadname	dc.b	'<0xDEADBEEF>',0
adbename	dc.b	'<0xADBEEFDE>',0

;*****************************************************************************/

;STRPTR ASM GetTaskName (REG (a0) struct Task * tc, REG (a1) STRPTR buffer, REG (d0) ULONG len)
;{
;    struct Process *pr = (struct Process *) tc;
;    STRPTR name = tc->tc_Node.ln_Name;
;    struct CommandLineInterface *cli;
;    STRPTR tmp;
;
;    /* Prepare the CLI name for printing */
;    if (tc->tc_Node.ln_Type == NT_PROCESS)
;    {
;	if (pr->pr_CLI)
;	{
;	    if (cli = BADDR (pr->pr_CLI))
;	    {
;		if (cli->cli_CommandName)
;		{
;		    /* Try getting command name */
;		    tmp = BADDR (cli->cli_CommandName);
;		    if (tmp[0] > 0)
;			name = ++tmp;
;		}
;	    }
;	}
;    }
;    strncpy (buffer, name, len);
;    return (name);
;}
