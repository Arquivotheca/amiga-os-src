******************************************************************************
*
* ScaleBitMapLine - Copyright (c) 1989,90,91 by MKSoft Development
*
* This code is confidential and property of MKSoft Development
*
******************************************************************************
*
* void ScaleBitMapLine(source,dest,srcwidth,destwidth,startx)
*                      a0     a1   d0       d1		d7
*
* Scales the source bitmap line into the destination line.
* The source line width and destination line width must be
* more larger than 1.  They must also be less than 32,000.
* StartX is the bit into the word (0 - 31).
*
	XDEF _ScaleBitMapLine

_ScaleBitMapLine:
		movem.l	d2-d7/a2-a5,-(sp)	; Save these...
*
* First, calculate the full and partial bits...
*
		move.l	d1,d2			; Set for the divide...
		divu.w	d0,d2			; Divide the stuff...
*
* d2 is now:  (error term  :  full pixels)
*
		move.w	d0,a5			; Save it here for later
		move.w	d0,d3			; Start of error counter
		asr.w	#1,d3			; Divide by 2 to center error
		neg.w	d3			; Negative
		exg	d3,a2			; Store in a2
*
* a2 is the current error term counter...
*
		exg	a2,d2			; Put it where best kept...
*
* Now set up the destination longword and the bit count for it
*
		moveq.l	#32,d6			; Bit count for it...
*
* Ok, we have
*		D7	=destination long word (in progress)
*		D6	=destination long word free bits
*		A5	=original X size...
*		A2	=(error term : full pixels)
*		D2	=current error term
*		D1	=destination pixels left
*		D0	=source pixels left
*
* We will use
*		D5	=source long word
*		D4	=source bit count
*
* Now, start the loop.
*
		subq.l	#1,d0			; Addjust source count...
		move.l	(a0)+,d5		; Get source...
		asl.l	d7,d5			; Shift over to first bit
		moveq.l	#32,d4			; Number of bits
		sub.l	d7,d4			;... left after shift...
		bra.s	Scale_Main1		; Go to it...
*
*
Scale_Main:	add.l	d5,d5			; Next bit over
		subq.w	#1,d4			; Count the bit
		bne.s	Scale_Main1		; If not done, skip
*
* We are done with last long word, so get another
*
		move.l	(a0)+,d5		; Get next word
		moveq.l	#32,d4			; Set the count
Scale_Main1:	move.l	a2,d3			; Get error term
		swap	d3			; into d3...
		add.w	d3,d2			; Count it...
		bmi.s	No_Extra		; No extra bit this time
		swap	d3			; Get pixel size into d3
		addq.w	#1,d3			; Add one...
		sub.w	a5,d2			; Subtract width again...
		bra.s	Scale_Main2		; Go and do it...
No_Extra:	swap	d3			; Set D3 to pixel size
*
* Now, we know how many bits the current top bit of d5 needs to be in d7
*
Scale_Main2:	tst.w	d3			; Check if zero
		beq.s	Next_Source		; If zero, we go to next
		move.l	d0,a3			; Now, save d0 in A3
		move.w	d4,a4			; and save d4 in A4
*
* Now, do as many loops here as needed to store the given pixel
* by the magnification given
*
Mag_Loop:	move.w	d3,d0			; Get number of bits
		cmp.w	d0,d6			; Do we have the room?
		bpl.s	Have_Room		; If we have room for them...
		move.w	d6,d0			; We only have room for these
Have_Room:	asl.l	d0,d7			; Move destination over...
		tst.l	d5			; Check if negative
		bpl.s	No_Bit			; If no bit, we skip
		moveq.l	#1,d4			; Set the bit...
		asl.l	d0,d4			; Shift over...
		subq.l	#1,d4			; Set all of the bits needed
		or.l	d4,d7			; Set the new bits...
No_Bit:		sub.w	d0,d6			; Subtract bits we did...
		bne.s	No_Store		; We don't store
		move.l	d7,(a1)+		; Store the bits...
		moveq.l	#32,d6			; Set up the next one...
		sub.l	d6,d1			; Count them...
*
* We only need to go back if we have possible more bits left
*
		sub.w	d0,d3			; Count the ones we did
		bne.s	Mag_Loop		; Loop back for the rest
*
* We now finished the current bit, so test some more
*
No_Store:	move.w	a4,d4			; Restore D4
		move.l	a3,d0			; Restore D0
*
Next_Source:	dbra.s	d0,Scale_Main		; Count the bit and loop back
*
* Now, handle the last few bits...
*
		tst.l	d1			; Any left?
		beq.s	Scale_Done		; If not, end it...
		asl.l	d6,d7			; Get D7 set up...
		cmp.l	#17,d1			; Check how many are left
		bpl.s	Do_Long			; We have a long left...
		swap	d7			; Set it up...
		move.w	d7,(a1)+		; Store it
		bra.s	Scale_Done		; We are done
Do_Long:	move.l	d7,(a1)+		; Store it
*
* We are done now
*
Scale_Done:	movem.l	(sp)+,d2-d7/a2-a5	; Restore
		rts
*
******************************************************************************
	end
