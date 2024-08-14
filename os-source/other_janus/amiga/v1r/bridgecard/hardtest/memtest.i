	IFND	MEMTEST_I
MEMTEST_I	SET	1
*
******* Public EQUates ********************************************
*
* The Error Codes
OKAY			equ	0

BYTEACCESS		equ	1	* added to error code if byte access
LONGACCESS		equ	2	* added to error code if long access

MT_AAAA 		equ	$10	* test alternate bits with A's
MT_5555 		equ	$20	* test alternate bits with 5's
MT_FFFF 		equ	$30	* test with All 1's as pattern
MT_7777 		equ	$40	* test sliding 0's
MT_0000 		equ	$50	* test with 0's as pattern
MT_8888 		equ	$60	* test sliding 1's
MT_ADDR 		equ	$70	* test with ADDRESS as pattern
MT_NADDR		equ	$80	* test with /ADDRESS as pattern
MT_B0000		equ	$90	* test with byte writes of zeros
MT_SPOTBYTE		equ	$A0	* test writes to wrong address
	ENDC	!MEMTEST_I
