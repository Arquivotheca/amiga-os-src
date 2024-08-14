*******************************************************************************
*
* $Id: pack.asm,v 39.7 93/02/10 14:57:18 vertex Exp $
*
* $Log:	pack.asm,v $
* Revision 39.7  93/02/10  14:57:18  vertex
* Cleaned up docs
* 
* Revision 39.6  92/04/20  11:59:57  mks
* Autodoc cleanup
*
* Revision 39.5  92/04/09  03:04:02  mks
* Added the tag exists bit true hack for Peter
*
* Revision 39.4  92/04/09  00:52:13  mks
* Oops, missed a tst of the boolean...
*
* Revision 39.3  92/04/07  08:18:53  mks
* Cleaned up docs...
*
* Revision 39.2  92/04/04  16:31:44  mks
* Forgot the V39 in the autodocs...
*
* Revision 39.1  92/04/04  16:27:09  mks
* Initial release (untested)
*
*******************************************************************************
*
* PackStructureTags() and UnpackStructureTags(), two very important routines
* for 3.0...
*
*******************************************************************************
*
	include	"exec/types.i"
	include	"exec/macros.i"
*
	include	"tagitem.i"
	include	"pack.i"
*
******* utility.library/PackStructureTags *************************************
*
*   NAME
*	PackStructureTags -- pack a structure with values from taglist. (V39)
*
*   SYNOPSIS
*	num = PackStructureTags(pack,packTable,tagList);
*	D0                      A0   A1        A2
*
*	ULONG PackStructureTags(APTR,ULONG *,struct TagItem *);
*
*   FUNCTION
*	For each table entry, a FindTagItem() will be done and if the
*	matching tag is found in the taglist, the data field will be
*	packed into the given structure based on the packtable
*	definition.
*
*   INPUTS
*	pack - a pointer to the data area to fill in.
*	packTable - a pointer to the packing information table.
*		    See <utility/pack.h> for definition and macros.
*	tagList - a pointer to the taglist to pack into the structure
*
*   RESULTS
*	num - the number of tag items packed
*
*   SEE ALSO
*	<utility/pack.h>, FindTagItem(), UnpackStructureTags()
*
*******************************************************************************
*
PackStructureTags:	xdef	PackStructureTags
*
* Registers:
*	a2 - Tag list (as passed in)
*	a3 - Structure (a0 parameter)
*	a4 - Current table location
*	d2 - Base tag value
*	d3 - Data offset (from table)
*	d4 - Count of packed values
*
		movem.l	a2-a4/d2-d4,-(sp)	; Save these
		moveq.l	#0,d4			; Clear count
		move.l	a0,a3			; Get structure
		move.l	a1,a4			; Get table...
*
pst_BaseTag:	move.l	(a4)+,d2		; Base tag value...
*
pst_Main:	move.l	(a4)+,d3		; Get next table entry
		beq.s	pst_Done		; branch to exit if NULL
		addq.l	#1,d3			; Check for -1
		beq.s	pst_BaseTag		; If -1, get new base tag
		subq.l	#1,d3			; back up again...
*
		btst	#PSTB_PACK,d3		; Check if we can pack this
		bne.s	pst_Main		; Nope, so skip it...
*
		move.l	d3,d0			; Get table value into d0
		swap	d0			; Get high word at bottom
		and.l	#$000003FF,d0		; Mask out unused bits
		add.l	d2,d0			; Add in base tag
		move.l	a2,a0			; Get tag list
		JSRLIB	FindTagItem		; Search for the tag
		tst.l	d0			; Did we get it?
		beq.s	pst_Main		; No tag in list, so next...
*
		addq.l	#1,d4			; Count match...
		move.l	d0,a0			; Pointer to tag item...
		addq.l	#4,a0			; Point at ti_Data...
		move.l	(a0),d1			; Get ti_Data...
		move.l	d3,d0			; Get table value again
		rol.l	#6,d0			; Move up and around into low
		and.w	#6,d0			; Mask everything else
		jmp	pst_Table(pc,d0.w)	; Do jumptable...
*
* Table of size operators in PackStructureTags
*
pst_Table:	bra.s	pst_Byte	; 2 bytes (00 0)
		bra.s	pst_Word	; 2 bytes (01 0)
		bra.s	pst_Long	; 2 bytes (10 0)
;		bra.s	pst_Bit		; fall-in (11 0)
*
pst_Bit:	move.w	d3,d0		; Get value...
		rol.w	#3,d0		; Get bit number into bits 0-2
	; 	and.w	#7,d0		; Mask it (ignore this, CPU does it)
		and.w	#$1FFF,d3	; Mask out bit number
		btst.l	#PSTB_EXISTS,d3	; Check for EXISTS hack thingy
		bne.s	pst_BSET	; If EXISTS hack, assume TRUE...
		tst.l	d1		; False=clear bit (inverted if signed)
		beq.s	pst_BCLR
pst_BSET:	bset.b	d0,0(a3,d3.w)	; Set the bit
		bra.s	pst_BitInvert	; Check for inverted boolean
pst_BCLR:	bclr.b	d0,0(a3,d3.w)	; Clear the bit
pst_BitInvert:	tst.l	d3		; Check for SIGNED bit
		bpl.s	pst_Main	; If signed we will invert the meaning
		bchg.b	d0,0(a3,d3.w)	; Invert the bit
		bra.s	pst_Main
*
pst_Byte:	move.b	d1,0(a3,d3.w)	; Write the byte
		bra.s	pst_Main
*
pst_Word:	move.w	d1,0(a3,d3.w)	; Write the word
		bra.s	pst_Main
*
pst_Long:	move.l	d1,0(a3,d3.w)	; Write the long
		bra.s	pst_Main
*
ust_Done:	; Same code, so save some ROM...
pst_Done:	move.l	d4,d0			; Get result
		movem.l	(sp)+,a2-a4/d2-d4	; Restore
		rts				; Exit!
*
******* utility.library/UnpackStructureTags ***********************************
*
*   NAME
*	UnpackStructureTags -- unpack a structure to values in taglist. (V39)
*
*   SYNOPSIS
*	num = UnpackStructureTags(pack,packTable,tagList);
*	D0                        A0   A1        A2
*
*	ULONG UnpackStructureTags(APTR,ULONG *,struct TagItem *);
*
*   FUNCTION
*	For each table entry, a FindTagItem() will be done and if the
*	matching tag is found in the taglist, the data in the structure
*	will be placed into the memory pointed to by the tag's ti_Data.
*	ti_Data *must* point to a LONGWORD.
*
*   INPUTS
*	pack - a pointer to the data area to be unpacked
*	packTable - a pointer to the packing information table.
*		    See <utility/pack.h> for definition and macros
*	tagList - a pointer to the taglist to unpack into
*
*   RESULTS
*	num - the number of tag items unpacked
*
*   SEE ALSO
*	<utility/pack.h>, FindTagItem(), PackStructureTags()
*
*******************************************************************************
*
UnpackStructureTags:	xdef	UnpackStructureTags
*
* Registers:
*	a2 - Tag list (as passed in)
*	a3 - Structure (a0 parameter)
*	a4 - Current table location
*	d2 - Base tag value
*	d3 - Data offset (from table)
*	d4 - Count of packed values
*
		movem.l	a2-a4/d2-d4,-(sp)	; Save these
		moveq.l	#0,d4			; Clear count
		move.l	a0,a3			; Get structure
		move.l	a1,a4			; Get table...
*
ust_BaseTag:	move.l	(a4)+,d2		; Get base tage value...
*
ust_Main:	move.l	(a4)+,d3		; Get table value...
		beq.s	ust_Done		; If NULL, we are done...
		addq.l	#1,d3			; Check for -1
		beq.s	ust_BaseTag		; If -1, get new base tag
		subq.l	#1,d3			; back to normal
*
		btst	#PSTB_UNPACK,d3		; Check if we can unpack this
		bne.s	ust_Main		; Nope, so skip it...
*
		move.l	d3,d0			; Get table value into d0
		swap	d0			; Get high word at bottom
		and.l	#$000003FF,d0		; Mask out unused bits
		add.l	d2,d0			; Add in base tag
		move.l	a2,a0			; Get tag list
		JSRLIB	FindTagItem		; Search for the tag
		tst.l	d0			; Did we get it?
		beq.s	ust_Main		; No tag in list, so next...
*
		addq.l	#1,d4			; Count match...
		move.l	d0,a0			; Get into address register
		move.l	ti_Data(a0),a0		; Get pointer to real answer
*
		moveq.l	#0,d1			; Clear d1
		move.l	d3,d0			; Get table value again
		rol.l	#6,d0			; Move up and around into low
		and.w	#6,d0			; Mask everything else
		jmp	ust_Table(pc,d0.w)	; Do jumptable...
*
* Table of size operators in PackStructureTags
*
ust_Table:	bra.s	ust_Byte	; 2 bytes (00 0)
		bra.s	ust_Word	; 2 bytes (01 0)
		bra.s	ust_Long	; 2 bytes (10 0)
;		bra.s	ust_Bit		; fall-in (11 0)
*
ust_Bit:	move.w	d3,d0		; Get value...
		rol.w	#3,d0		; Get bit number into bits 0-2
	;	and.w	#7,d0		; Mask it (ignore this, CPU does it)
		and.w	#$1FFF,d3	; Mask out bit number
		btst	d0,0(a3,d3.w)	; Check the bit
		beq.s	ust_False	; Bit off?
		not.l	d1		; Bit on...  Set result to TRUE
ust_False:	tst.l	d3		; Check for signed bit
		bpl.s	ust_Store	; All ok, so just store it
		not.l	d1		; Invert boolean
		bra.s	ust_Store	; Store result
*
ust_Byte:	move.b	0(a3,d3.w),d1	; Write the byte
		tst.l	d3		; Check for signed
		bpl.s	ust_Store	; If unsigned, just go
		ext.w	d1		; Sign extend
		ext.l	d1		; since this is signed
		bra.s	ust_Store
*
ust_Word:	move.w	0(a3,d3.w),d1	; Write the word
		tst.l	d3		; Check for signed
		bpl.s	ust_Store	; If unsigned, just go
		ext.l	d1		; Sign extend it
		bra.s	ust_Store
*
ust_Long:	move.l	0(a3,d3.w),d1	; Write the long
;		bra.s	ust_Store	; Fall into the store operation
*
ust_Store:	move.l	d1,(a0)		; Store the result
		bra.s	ust_Main
*
*******************************************************************************
*
		end
