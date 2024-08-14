*\
*  :ts=8
* Yet Another random number generator.  By Leo Schwab.
* Based on an idea posted on the USENET (Thanks, Sam Dicker!)
* For the Manx assembler.
*
* Calling convention:
*  short rnd (range);
*  short range;
*
* 8606.30
*/

		public    _rnd

_rnd		lea	rndseed,a0	Get address of seed
		move.w	4(sp),d1	Get range argument
		tst.w	d1
		ble.s	setseed		Go reset seed


		move.l	(a0),d0		Get seed
		ADD.L   D0,D0
		BHI.S   over
		EORI.L  #$1D872B41,D0
over
		move.l	d0,(a0)		Save new seed
		andi.l	#$ffff,d0	Coerce into word
		divu	d1,d0		Divide by range
		swap	d0		 and get remainder (modulus)
		rts

setseed		neg.w	d1		Probably don't need this
		move.l	d1,(a0)
		rts

		dseg
rndseed		dc.l	0
		cseg
