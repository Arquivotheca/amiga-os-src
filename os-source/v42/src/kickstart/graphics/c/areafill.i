*******************************************************************************
*
*	Source Control
*	--------------
*	$Id: areafill.i,v 42.0 93/06/16 11:17:21 chrisg Exp $
*
*******************************************************************************

MDFLAG1	equ	1
MDFLAG2	equ	2
DRWBNDRY	equ	4
EXTENDED	equ	128

MDFLAG1n	equ	0
MDFLAG2n	equ	1
DRWBNDRYn	equ	2
EXTENDEDn	equ	7

*	extended instructions, valid range 1..127  do NOT use 0 !

CIRCLE	equ	1
ELLIPSE	equ	2
