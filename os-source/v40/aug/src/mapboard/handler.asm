*******************************************************************************
*
* Ok, so here is the really tricky part of MapBoard
*
*******************************************************************************
*
		include	"exec/types.i"
		include	"exec/lists.i"
		include	"exec/nodes.i"
		include	"exec/tasks.i"
		include	"exec/macros.i"
		include	"exec/execbase.i"
		include	"exec/ables.i"
		include	"libraries/configvars.i"
*
		include	"MapBoard_rev.i"
*
*******************************************************************************
*
* This is the amount of space that the registers take on the stack...
*
STACK		equ	4*(8+7)
*
*******************************************************************************
*
		opt	p=68040			; Use 68030 instructions
*
*******************************************************************************
*
* void CacheMap(ExecBase, ConfigDev, offset, size)
*               a6        a0         d0      d1
*
* This routine is to modify the MMU table to mark the address offset/size
* in the hardware device.  Note that you should not call this if you
* are not running on a 68040.  It does not double check for you.
*
_CacheMap:	xdef	_CacheMap
		movem.l	d2-d7/a2-a5,-(sp)	; Save everything...
		lea	CacheMap(pc),a5		; Code to run...
		JSRLIB	Supervisor		; Do it in supervisor state
		movem.l	(sp)+,d2-d7/a2-a5	; Restore as needed
		rts
*
cm_RTE:		rte
*
* Now, this is the real CacheMap code...  It runs in Supervisor mode
*
CacheMap:	movec.l	tc,d7			; Get MMU setting...
		tst.w	d7			; Is it on?
		bpl.s	cm_RTE			; If not, we can't MMU map
		btst.l	#14,d7			; Check for 4K pages
		bne.s	cm_RTE			; If not, we exit...
*
* Now, lets figure out what 4K pages we can mark.  Basically, we need to
* make sure that the addresses that are asked for are completely within
* the 4K pages.  This means that any address that are only part of a 4K
* page are ignored.
*
		move.l	cd_BoardAddr(a0),d4	; Get board address...
		add.l	d0,d4			; Add Offset...
		move.l	d4,d5			; Store in end pointer...
		add.l	d1,d5			; Add size to end pointer...
		move.l	#$00000FFF,d0		; Get 4K-1
		add.l	d0,d4			; start + (4K-1)
		not.l	d0			; Invert (mask to 4K)
		and.l	d0,d4			; mask start to 4K
		and.l	d0,d5			; make end to 4K
*
* d4 is the address of the start page
* d5 is the address of the page just past the end.
* (That is d5 points 1 past the end of the page)
*
* Now, loop through pages marking each page as cachable...
*
mmu_Loop:	cmp.l	d4,d5			; Are we at the end?
		bls.s	cm_RTE			; If so, exit...
		move.l	d4,d0			; Get page address
		add.l	#$1000,d4		; Bump to the next page...
*
* Ok, so now d0 is the page, so find the silly MMU entry for it...
*
		movec.l	urp,a0			; Get ROOT pointer...
		bfextu	d0{0:7},d1		; Get the root index...
		add.l	d1,d1			; *2
		add.l	d1,d1			; *4
		add.l	d1,a0			; Add to root pointer...
		move.l	(a0),d1			; Get page entry
		and.l	#$FFFFFE00,d1		; Mask into the page table
		move.l	d1,a0			; Store pointer...
		bfextu	d0{7:7},d1		; Get the pointer index...
		add.l	d1,d1			; *2
		add.l	d1,d1			; *4
		add.l	d1,a0			; Add to table pointer...
		move.l	(a0),d1			; Get page entry...
		and.l	#$FFFFFF00,d1		; Mask to the pointer...
		move.l	d1,a0			; Put into address register...
		bfextu	d0{14:6},d1		; Get index into page table
		add.l	d1,d1			; *2
		add.l	d1,d1			; *4
		add.l	d1,a0			; a0 now points at the page...
		move.l	(a0),d1			; Get page entry...
		btst.l	#0,d1			; Check if bit 0 is set...
		beq.s	mmu_Loop		; If not, invalid page...
		bclr.l	#6,d1			; Clear no-cache bit...
		bset.l	#5,d1			; Set Copyback bit...
		pflusha				; Flush the ATC
		move.l	d1,(a0)			; Store new MMU entry
		cpushl	dc,(a0)			; Flush to RAM
		bra.s	mmu_Loop		; Do next one...
*
*******************************************************************************
*
		VERSTAG	; Version string...
*
*******************************************************************************
*
* "A master's secrets are only as good as the
*  master's ability to explain them to others."  -  Michael Sinz
*
*******************************************************************************
*
		END
