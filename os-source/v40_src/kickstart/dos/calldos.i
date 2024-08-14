*
* calldos.i
*
	
	; handle arguement conversion
	; arguements are at (p) and args 1-4 are in regs D1-D4 as well
	; codes in mask are:
	;		00 - no change/unused,
	;		01 - BPTR -> CPTR
	;		10 - BSTR -> CSTR
	;		11 - end of args
	;
	; mask is laid out as follows: bits 0-1 -> 1st arg, 2-3 -> 2nd arg, etc
	; top word of mask == 0 for no shift of result, -1 for >> 2
	;

NOCHANGE	EQU	0
BPTRCPTR	EQU	1
BSTRCSTR	EQU	2
CALLDEND	EQU	3

RETURN_BPTR	EQU	$100		; bit 8 - would be arg5!
RETURN_BPTR_BIT EQU	8

ARG2		EQU	2
ARG3		EQU	4
ARG4		EQU	6
* ARG5		EQU	8		; never used
* ARG6		EQU	10		; never used
