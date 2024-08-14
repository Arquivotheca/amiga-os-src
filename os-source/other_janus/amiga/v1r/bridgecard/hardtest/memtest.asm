******* Included Files ***********************************************

	include "exec/types.i"
	include "memtest.i"

	XREF	_NextTest

******* Exported Functions *******************************************

	XDEF	_DestructiveMemTest
    
	XDEF	_ExpectedPattern
	XDEF	_ActualPattern
	XDEF	_BadAddress

	SECTION section,data
_ExpectedPattern ds.l	1
_ActualPattern	ds.l	1
_BadAddress	ds.l	1

	SECTION section,code

******* DestructiveMemTest *******************************************
*
*  SYNOPSIS
*	results = DestructiveMemTest( startAddr, endAddr )
*
*
*  FUNCTION
*	This test is a very exhaustive, intensive, memory test.
*	Returning success as a zero in D0.  Memory will be left
*	0 filled.
*
*
*  INPUTS
*	start_addr	- the address at which to begin testing
*	end_addr	- the last address to be tested
*		
*	These addresses will delimit an even number of longwords:
*	(f00000,f0000f) will test the four longwords f0000x, and
*	(f00000,f00008) will test f00000-f0000b: be careful.
*
*  OUTPUTS
*	results 	- D0 is set according to the results of the test.
*
*	A zero result is returned in D0 if the test reached a successful
*	conclusion.  A non-zero result will contain the test number,
*	and if the error occured during a write, the write bit will also
*	be set.
*
*	Occurance of any error will terminate this test.  The address in
*	error will be contained in the long variable BadAddress.
*
*	Additionally the caller may examine the long variables
*	ExpectedPattern and ActualPattern for the pattern in error that
*	was expected, and actually found.
*	
**********************************************************************
*
*	REGISTERS
*	d0 -	   contains the test results.
*	d1 -	   the expected pattern.
*	d2 -	   the encountered pattern, previous contents are lost.
*	d3 -	   permanant loop count initializer.
*	d4 -	   low loop counter
*	d5 -	   high loop counter		       
*	d6 -	   byte offset in spot test
*	d7 -	   old pattern in spot test
*
*	a0 -	   contains the current test ( failing ) address.
*	a2 -	   the starting address, this register is returned unchanged.
*	a3 -	   the ending address, this register is returned unchanged.
*
**********************************************************************
_DestructiveMemTest:
		movem.l d2-d7/a2-a3,-(a7)
		move.l	36(a7),a2
		move.l	40(a7),a3

* This enhancement was added to give use some more speed
		move.l	a3,d3		* capture the high address
		sub.l	a2,d3		* subtract the low address

		lsr.l	#2,d3		* divide by 4 to get LONGWORDS to test
*
*	BITCHECK -- FIRST TEST
*
*	The first test to be executed will be alternate ones and zeros
*	This test is designed to catch adjacent bits "pulling" on their 
*	neighbors, and is especially usefull on these very fast strobes!
*
		moveq	#MT_AAAA,d0
		bsr	SetupTest

		move.l	#$AAAAAAAA,d1	* our pattern of all a's
		bsr	ConstantTest

*
*	BITCHECK -- SECOND TEST
*
*	The second test to be executed will be alternate ones and zeros
*	except on the alternates from the test above.  This test is also
*	designed to catch adjacent bits "pulling" on their neighbors.
*
		moveq	#MT_5555,d0
		bsr	SetupTest

		move.l	#$55555555,d1	* our pattern of all 5's
		bsr	ConstantTest

*
*	ONEs  -- THIRD TEST
*
		moveq	#MT_FFFF,d0
		bsr	SetupTest

		moveq.l #-1,d1		* our pattern of all F's
		bsr	ConstantTest

*
*	ZEROs
*
		moveq	#MT_0000,d0
		bsr	SetupTest

		moveq	#0,d1		* our pattern of all 0's
		bsr	ConstantTest

*
*	BYTE SPOT TEST
*
*	This test will write a single byte and verify that it does not
*	appear at any other byte.  For this test, BadAddress contains
*	the write address, and ActualPattern contains the (offending)
*	address where the data is also found 
*	
		move.l	#MT_SPOTBYTE,d0
		bsr	SetupTest

		move.l	#$A5,d1
		bsr	SpotTest

*	SLIDING ZEROs -- FOURTH TEST
*
*	The fourth test to be executed will fill memory with 1's, overlaid
*	with a nibblewide field of zeros to be slid through the ones.  This
*	is the first test to be executed on the entire block of memory.
*	This means that we will no longer check each write operation.
*
		moveq	#MT_7777,d0
		bsr	SetupTest

		move.l	#$77777777,d1	* our pattern of 0111011101110111
*
* Initialize the pattern
Loop4A:
		rol.l	#1,d1		* roll me over.. in the clover..
		movea.l a2,a0		* recover our starting address
		move.w	d3,d4		* recover test length
		move.l	d3,d5		* length into 2 words for long counts!
		swap	d5		* puts the high word within reach

*
* Write Pass
Loop4B:
		move.l	d1,(a0)+	* the write
		dbf	d4,Loop4B	* loop until done
		dbf	d5,Loop4B

*
* Read Pass
		movea.l a2,a0		* recover our starting address
		move.w	d3,d4		* recover test length
		move.l	d3,d5		* length into 2 words for long counts!
		swap	d5		* puts the high word within reach

Loop4C:
		move.l	(a0),d2 	* now read it back
		cmp.l	d1,d2		* are they = ?
		bne	Error		* the only way out!
		dbf	d4,Loop4C	* loop until done
		dbf	d5,Loop4C

		tst.l	d1		* have we rolled the zero to end yet?
		bmi.s	Loop4A		* nope, just keep on shifting!


*
*	SLIDING ONEs
*
*	The sixth test to be executed will fill memory with 0's, overlaid
*  with a nibblewide field of ones to be slid through the zeros.
*
		moveq	#MT_8888,d0
		bsr	SetupTest

		move.l	#$88888888,d1	* our pattern of 1000100010001000
*
* Initialize the pattern
Loop6A:
		rol.l	#1,d1		* roll me over.. in the clover..
		movea.l a2,a0		* recover our starting address
		move.w	d3,d4		* recover test length
		move.l	d3,d5		* length into 2 words for long counts!
		swap	d5		* puts the high word within reach

*
* Write Pass
Loop6B:
		move.l	d1,(a0)+	* the write
		dbf	d4,Loop6B	* loop until done
		dbf	d5,Loop6B

*
* Read Pass
		movea.l a2,a0		* recover our starting address
		move.w	d3,d4		* recover test length
		move.l	d3,d5		* length into 2 words for long counts!
		swap	d5		* puts the high word within reach

Loop6C:
		move.l	(a0),d2 	* now read it back
		cmp.l	d1,d2		* are they = ?
		bne	Error		* the only way out!
		dbf	d4,Loop6C	* loop until done
		dbf	d5,Loop6C

		tst.l	d1		* have we rolled the one to end yet?
		bpl.s	Loop6A		* nope, just keep on shifting!

*
*	ADDRESSes -- SEVENTH TEST
*
*	The seventh test to be executed will fill memory longwords with
*  its address.
*
		move.l	#MT_ADDR,d0
		bsr	SetupTest


*
* Write Address Pass
Loop7A:
		move.l	a0,(a0)+	* write current address and increment
		dbf	d4,Loop7A	* loop until done
		dbf	d5,Loop7A

*
*Read Address Pass
		movea.l a2,a0		* recover our starting address
		move.w	d3,d4		* recover test length
		move.l	d3,d5		* length into 2 words for long counts!
		swap	d5		* puts the high word within reach

Loop7B:
		move.l	a0,d1		* the pattern
		move.l	(a0)+,d2	* the actual value

		cmp.l	d1,d2		* are they = ?
		bne	Error		* the only way out!
		dbf	d4,Loop7B	* loop until done
		dbf	d5,Loop7B

*
*	INVERT ADDRESSes -- EIGHTH TEST
*
*	The eighth test to be executed will fill memory longwords with
*  its complemented address.
*
		move.l	#MT_NADDR,d0
		bsr	SetupTest

*
* Write NOT Address Pass
Loop8A:
		move.l	a0,d1		* get the current address
		not.l	d1		* invert it
		move.l	d1,(a0)+	* write out and increment to next cell
		dbf	d4,Loop8A	* loop until done
		dbf	d5,Loop8A

*
*Read NOT Address Pass
		move.w	d3,d4		* recover test length
		move.l	d3,d5		* length into 2 words for long counts!
		swap	d5		* puts the high word within reach
		move.l	a2,a0		* recover our starting address

Loop8B:
		move.l	a0,d1		* the address
		not.l	d1		* invert it
		move.l	(a0)+,d2	* the actual value

		cmp.l	d1,d2		* are they = ?
		bne	Error		* the only way out!
		dbf	d4,Loop8B	* loop until done
		dbf	d5,Loop8B


*
*	BYTEFILL -- NINTH TEST
*
*	The ninth test to be executed will fill memory by bytes with
*	zeros, and will then check, one more time, that all portions of
*	memory were tested.
*
		move.l	#MT_B0000,d0
		bsr	SetupTest

		moveq	#0,d1		* our data pattern

		move.l	d3,d4		* recover our starting length
		lsl.l	#2,d4		* X4, to get the byte count!
		addq.l	#3,d4
		move.l	d4,d5		* length into 2 words for long counts!
		swap	d5		* puts the high word within reach

*
* Write "zero" BYTE pass
Loop9A:
		move.b	d1,(a0)+	* write out the 0 to the byte address
		dbf	d4,Loop9A	* loop until done
		dbf	d5,Loop9A	* loop until all of it's done

*
* Read for the "zero" bytes by word increment

		move.l	d3,d4		* recover our starting length
		lsl.l	#1,d4		* X2, to get the word length
		addq.l	#1,d4
		move.l	d4,d5		* length into 2 words for long counts!
		swap	d5		* puts the high word within reach
		move.l	a2,a0		* recover our starting address

Loop9B:
		move.w	(a0)+,d2	* the actual value
		cmp.w	d1,d2		* are they = ?
		bne.s	Error		* the only way out!
		dbf	d4,Loop9B	* loop until done
		dbf	d5,Loop9B	* loop until all of it's done

		clr.l	d0		* no errors, clear the guy and return
		bra.s	Error

PopError:
		;------ an error from a subroutine
		addq.l	#4,a7		* pop the return address

Error:
		move.b	d0,d4
		and.b	#BYTEACCESS,d4
		bne.s	1$
		move.b	d0,d4
		and.b	#LONGACCESS,d4
		beq.s	Exit
1$
		subq.l	#1,a0		* realign to point to address in error
		bra.s	Exit
2$
		subq.l	#4,a0		* realign to point to address in error	
Exit:
		move.l	d1,_ExpectedPattern
		move.l	d2,_ActualPattern
		move.l	a0,_BadAddress
		movem.l (a7)+,d2-d7/a2-a3
		rts			; ".. good nite Gracie.."!

**********************************************************************

SetupTest:
		move.l	d0,-(a7)
		jsr	_NextTest
		addq.l	#4,a7

		move.w	d3,d4		* recover test length
		move.l	d3,d5		* length into 2 words for long counts!
		swap	d5		* puts the high word within reach
		movea.l a2,a0		* recover our starting address
		rts

ConstantTest:
		addq.b	#LONGACCESS,d0

LoopCTL:
		move.l	d1,(a0) 	* the write
		move.l	(a0)+,d2	* now read it back

		cmp.l	d1,d2		* are they = ?
		bne	PopError	* the only way out!
		dbf	d4,LoopCTL	* loop until done
		dbf	d5,LoopCTL

		subq.b	#LONGACCESS,d0
		addq.b	#BYTEACCESS,d0

		move.l	d3,d4		* recover test length
		lsl.l	#2,d4		* convert to bytes
		addq.l	#3,d4
		move.l	d4,d5		* length into 2 words for long counts!
		swap	d5		* puts the high word within reach
		movea.l a2,a0		* recover our starting address

LoopCTB:
		move.b	d1,(a0) 	* the write
		move.b	(a0)+,d2	* now read it back

		cmp.b	d1,d2		* are they = ?
		bne	PopError       * the only way out!
		dbf	d4,LoopCTB	* loop until done
		dbf	d5,LoopCTB
				  
		rts


SpotTest:
		addq.b	#BYTEACCESS,d0
		moveq	#0,d6
spTB1:
		move.l	a2,a0		* recover our starting address
		add.l	d6,a0		* get the next byte to test
		move.b	(a0),d7   
		move.b	d1,(a0)+

		move.l	d3,d4		* recover test length
		lsl.l	#2,d4		* convert to bytes
		addq.l	#3,d4
		move.l	d4,d5		* length into 2 words for long counts!
		swap	d5		* puts the high word within reach
		move.l	a2,a1		* recover our starting address

spTB2
		cmp.b	(a1)+,d1
		beq	spError
spTestAddr:
		dbf	d4,spTB2
		dbf	d5,spTB2

		move.b	d7,-(a0)
		add.l	d6,d6
		beq.s	spOneBit
		cmp.l	d3,d6
		blt.s	spTB1
		rts
 
spOneBit:
		moveq	#1,d6
		bra.s	spTB1

spError:
		cmp.l	a0,a1
		beq.s	spTestAddr
		move.l	a1,d2
		subq.l	#1,d2
		bra	PopError

	END