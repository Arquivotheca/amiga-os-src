******* Included Files ***********************************************

	include "exec/types.i"
	include "romdiag.i"

******* Exported Functions *******************************************

	XDEF	_MemTest
	XDEF	WarmStartTest


******* ColdStartTest ************************************************
*
*  NAME
*	ColdStartTest -- Begin testing from an unknown condition
*
*
*  SYNOPSIS
*	results = ColdStartTest( start_addr , end_addr , return )
*	D0			 A2	      A3	 A5
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
*	end_addr		- the last address to be tested
*	return		- the address through which to return
*
*
*	OUTPUTS
*	results 	- D0 is set according to the results of the test.
*
*	A zero result is returned in D0 if the test reached a successful
*	conclusion.  A non-zero result will contain the test number,
*	and if the error occured during a write, the write bit will also
*	be set.
*
*	Occurance of any error will terminate this test.  The address in
*	error will be contained in A0.
*
*	Additionally the caller may examine D1 for the expected pattern,
*	and D2 for the actual pattern found.
*	
*
*	REGISTERS
*	D0 -	   contains the test results.
*	D1 -	   the expected pattern.
*	D2 -	   the encountered pattern, previous contents are lost.
*	D3 -	   scratch, previous contents are lost.
*	D4 -	   scratch, previous contents are lost.
*	A0 -	   contains the current test ( failing ) address.
*	A2 -	   the starting address, this register is returned unchanged.
*	A3 -	   the ending address, this register is returned unchanged.
*	A5 -	   the return address, this register is returned unchanged.
*
*
*  EXCEPTIONS
*
*  SEE ALSO
*
*  MISCELLANY
*
**********************************************************************
ColdStartTest:

* This enhancement was added to give use some more speed
	move.l	a3,d3		    * capture the high address
	sub.l	a2,d3		    * subtract the low address

	lsr.l	#2,d3		    * now divide by 4 to get LONGWORDS to test
	move.l	d3,d4		    * make a copy for reuse!
	clr.l	d0		    * clear out the error

*
*	BITCHECK -- FIRST TEST
*
*	The first test to be executed will be alternate ones and zeros
*	This test is designed to catch adjacent bits "pulling" on their 
*	neighbors, and is especially usefull on these very fast strobes!
*
	move.w	#ERROR_TEST1,d0 * the first test
	addq.b	#BITCHECK,d0	* we'll begin with the bit check test

	move.l	#$AAAAAAAA,d1	* our pattern of all a's
	movea.l a2,a0		* copy our starting address for use

Loop1:
	move.l	d1,(a0) 	    * the write
	move.l	(a0)+,d2	    * now read it back

	cmp.l		d1,d2				* are they = ?
	bNE		Error				* the only way out!
	dbF		d3,Loop1			* loop until done


*
*
*	BITCHECK -- SECOND TEST
*
*	The second test to be executed will be alternate ones and zeros
*	except on the alternates from the test above.  This test is also
*	designed to catch adjacent bits "pulling" on their neighbors.
*
	move.w	#ERROR_TEST2,d0 * the second test
	addq.b	#BITCHECK,d0	* define it as another bit check test

	move.l	#$55555555,d1	* our pattern of all 5's
	movea.l a2,a0				* recover our starting address
	move.l	d4,d3				* recover our starting length

Loop2:
	move.l	d1,(a0) 		* the write
	move.l	(a0)+,d2			* now read it back

	cmp.l		d1,d2				* are they = ?
	bNE		Error				* the only way out!
	dbF		d3,Loop2			* loop until done

*
*	ONEs  -- THIRD TEST
*	This test has been removed for speed & time considerations.  If
*	subsequent testing should show that this is a product liability
*	this test may easily be reinserted here.
*
*
*	SLIDING ZEROs -- FOURTH TEST
*
*	The fourth test to be executed will fill memory with 1's, overlaid
*	with a nibblewide field of zeros to be slid through the ones.  This
*	is the first test to be executed on the entire block of memory.
*	This means that we will no longer check each write operation.
*
	move.w	#ERROR_TEST4,d0 * the fourth test
	move.l	#$77777777,d1	* our pattern of 0111011101110111
*
* Initialize the pattern
Loop4A:
	rol.l		#1,d1		* roll me over.. in the clover..
	movea.l a2,a0			* recover our starting address
	move.l	d4,d3			* recover our starting length

*
* Write Pass
Loop4B:
	move.l	d1,(a0)+			* the write
	dbF		d3,Loop4B		* loop until done

*
* Read Pass
	movea.l a2,a0				* recover our starting address
	move.l	d4,d3				* recover our starting length

Loop4C:
	move.l	(a0),d2 		* now read it back
	cmp.l		d1,d2				* are they = ?
	bNE		Error				* the only way out!
	dbF		d3,Loop4C		* loop until done

	tst.l	d1		* have we rolled the zero around to end yet?
	bMI.s	Loop4A			* nope, just keep on shifting!

*
*	ZEROs -- FIFTH TEST
*	This test has been removed for speed & time considerations.  If
*	subsequent testing should show that this is a product liability
*	this test may easily be reinserted here.
*
*	SLIDING ONEs -- SIXTH TEST
*
*	The sixth test to be executed will fill memory with 0's, overlaid
*  with a nibblewide field of ones to be slid through the zeros.
*
	move.w	#ERROR_TEST6,d0 * the sixth test
	move.l	#$88888888,d1	* our pattern of 1000100010001000
*
* Initialize the pattern
Loop6A:
	rol.l	#1,d1			    * roll me over.. in the clover..
	movea.l a2,a0			    * recover our starting address
	move.l	d4,d3			    * recover our starting length

*
* Write Pass
Loop6B:
	move.l	d1,(a0)+			* the write
	dbF		d3,Loop6B		* loop until done

*
* Read Pass
	movea.l a2,a0				* recover our starting address
	move.l	d4,d3				* recover our starting length

Loop6C:
	move.l	(a0),d2 		* now read it back
	cmp.l		d1,d2				* are they = ?
	bNE		Error				* the only way out!
	dbF		d3,Loop6C		* loop until done

	tst.l	d1		* have we rolled the one around to end yet?
	bPL.s	Loop6A		* nope, just keep on shifting!



*
*	ADDRESSes -- SEVENTH TEST
*
*	The seventh test to be executed will fill memory longwords with
*  its address.
*
QuickMemTest:
	move.l	a3,d3				* capture the high address
	sub.l		a2,d3				* subtract the low address

	lsr.l		#2,d3				* now divide by 4 to get LONGWORDS to test
	move.l	d3,d4				* make a copy for reuse!
	clr.l		d0					* clear out the error


Test7A:
	move.w	#ERROR_TEST7,d0 * the seventh test
	movea.l a2,a0				* recover our starting address
	move.l	d4,d3				* recover our starting length

*
* Write Address Pass
Loop7A:
	move.l	a0,(a0)+			* write out the current address and increment
	dbF		d3,Loop7A			* loop until done

*
*Read Address Pass
	movea.l a2,a0				* recover our starting address
	move.l	d4,d3				* recover our starting length

Loop7B:
	move.l	a0,d1				* the pattern
	move.l	(a0)+,d2			* the actual value

	cmp.l		d1,d2				* are they = ?
	bNE		Error				* the only way out!
	dbF		d3,Loop7B		* loop until done

*
*	INVERT ADDRESSes -- EIGHTH TEST
*
*	The eighth test to be executed will fill memory longwords with
*  its complemented address.
*
	move.w	#ERROR_TEST8,d0 * the eighth test
	movea.l a2,a0				* recover our starting address
	move.l	d4,d3				* recover our starting length

*
* Write NOT Address Pass
Loop8A:
	move.l	a0,d1				* get the current address
	not.l		d1					* invert it
	move.l	d1,(a0)+			* write it out and increment to next cell
	dbF		d3,Loop8A		* loop until done

*
*Read NOT Address Pass
	movea.l a2,a0				* recover our starting address
	move.l	d4,d3				* recover our starting length

Loop8B:
	move.l	a0,d1				* the address
	not.l		d1					* invert it
	move.l	(a0)+,d2			* the actual value

	cmp.l		d1,d2				* are they = ?
	bNE		Error				* the only way out!
	dbF		d3,Loop8B		* loop until done


*
*	BYTEFILL -- NINTH TEST
*
*	The ninth test to be executed will fill memory by bytes with
*  zeros, and will then check, one more time, that all portions of
*	memory were tested.
*
	move.w	#ERROR_TEST9,d0 * the ninth test
	movea.l a2,a0				* recover our starting address
	clr.l		d1					* our data pattern

	move.l	d4,d3				* recover our starting length
	lsl.l		#2,d3				* X4, to get the byte count!
	move.l	d3,d5				* length into 2 words for long counts!
	swap		d5					* puts the high word within reach

*
* Write "zero" BYTE pass
Loop9A:
	move.b	d1,(a0)+			* write out the 0 to the byte address
	dbF		d3,Loop9A		* loop until done
	dbF		d5,Loop9A		* loop until all of it's done

*
* Read for the "zero" bytes by word increment
	movea.l a2,a0				* recover our starting address

	move.l	d4,d3				* recover our starting length
	subq.l	#1,d3				* can't have odd bytes
	lsl.l		#1,d3				* X2, to get the word length
	move.l	d3,d5				* length into 2 words for long counts!
	swap		d5					* puts the high word within reach

Loop9B:
	move.w	(a0)+,d2			* the actual value
	cmp.w		d1,d2				* are they = ?
	bNE.s		Error				* the only way out!
	dbF		d3,Loop9B		* loop until done
	dbF		d5,Loop9B		* loop until all of it's done

	clr.l		d0					* no errors, clear the guy and return

Error:
	suba		#4,a0				* if realign to point to address in error	

Exit:
	jmp		(a5)				* ".. good nite Gracie.."!

*
**********************************************************************



******* WarmStartTest ************************************************
*
*  NAME
*	WarmStartTest -- Begin testing from a previously running state
*
*
*  SYNOPSIS
*	results = WarmStartTest( start_addr , end_addr , return )
*    D0 			A2	    A3	      A5
*
*
*  FUNCTION
*	This test is a quick, non-destructive test.  Returning, in
*	D0, a zero if test conclusion was successful.
*
*
*  INPUTS
*	start_addr	- the address at which to begin testing
*	end_addr		- the last address to be tested
*	return		- the address through which to return
*
*
*  OUTPUTS
*  results     - D0 is set according to the results of the test.
*
*  A zero result is returned in D0 if the test reached a successful
*  conclusion.	A non-zero result will contain the test number.
*
*  Occurance of any error will terminate this test.  The address in
*  error will be contained in A0.
*
*  Additionally the caller may examine D1 for the expected pattern,
*  and (A0) for the actual pattern found.
*
*
*	REGISTERS
*	D0	-	returns the test results.
*	D1 -	the expected pattern.
*	D2	-	scratch, is used to hold the test cells previous contents, ret unchgd.
*	D3 -	scratch, the test length, this register is returned unchanged
*	A0	-	contains the current test ( failing ) address.
*	A2	-	the starting address, this register is returned unchanged.
*	A3	-	the ending address, this register is returned unchanged.
*	A5	-	the return address, this register is returned unchanged.
*
*
*	EXCEPTIONS
*
*  SEE ALSO
*
*	MISCELLANY
*
**********************************************************************
*

WarmStartTest:

*
* First preserve our environment
	movem.l d2/d3,-(sp)		* push them on the stack
*
* This enhancement was added to give use some more speed
	move.l	a3,d3				* capture the high address
	sub.l		a2,d3				* subtract the low address

	lsr.l		#2,d3				* now divide by 4 to get LONGWORDS to test
	movea.l a2,a0				* make a copy of our starting address

*
*	SOFTTEST -- FIRST TEST
* 
*	This test will "pickup" the original contents of the cell, and test
*	under it, with the cells address, and its complement, restoring it
*	when done.
*
	clr.l		d0					* clear out the register
	move.w	#ERROR_TEST10,d0	* the softtest error code

LoopWS:
	move.l	a0,d1				* the current address
	move.l	(a0),d2 		* save the  contents of our test address

	move.l	d1,(a0) 		* write out your own address
	cmp.l		(a0),d1 		* are the contents same as the address??
	bNE		ErrorWS 		* then just return

	not.l		d1					* invert it
	move.l	d1,(a0) 		* write out the inverted address
	cmp.l		(a0),d1 		* are the contents same as /address??
	bNE		ErrorWS 		* then just return

	move.l	d2,(a0)+			* restore the contents
	dbF		d3,LoopWS		* do until done

	clr.l		d0					* no errors so return a 0

ErrorWS:
	movem.l (sp)+,d2/d3		* recover the saved register

ExitWS:
	jmp		(a5)				* bye all..

*
	end							; -- test end
