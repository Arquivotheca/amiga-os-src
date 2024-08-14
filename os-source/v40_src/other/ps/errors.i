*  This file contains all the possible Postscript error returns.
*  May 1991 L.V. © CBM-Amiga
*  Submitted to RCS on 05/APR/91

	IFND	ERRORS_I
ERRORS_I	equ	1

ERR_OK					equ	0
ERR_dictfull			equ	1
ERR_dictstackoverflow	equ	2	
ERR_dictstackunderflow	equ	3
ERR_execstackoverflow	equ	4	
ERR_handleerror			equ	5
ERR_interrupt			equ	6	
ERR_invalidaccess		equ	7	
ERR_invalidexit			equ	8
ERR_invalidfileaccess	equ	9	
ERR_invalidfont			equ	10
ERR_invalidrestore		equ	11
ERR_ioerror				equ	12
ERR_limitcheck			equ	13
ERR_nocurrentpoint		equ	14
ERR_rangecheck			equ	15
ERR_stackoverflow		equ	16	
ERR_stackunderflow		equ	17
ERR_syntaxerror			equ	18
ERR_timeout				equ	19
ERR_typecheck			equ	20
ERR_undefined			equ	21
ERR_undefinedfilename	equ	22
ERR_undefinedresult		equ	23
ERR_unmatchedmark		equ	24
ERR_unregistered		equ	25
ERR_VMerror				equ	26

ERR_invalidcontext		equ	27
ERR_invalidid			equ	28

ERR_QUIT_INTERPRETER	equ	29
ERR_NOT_IMPLEMENTED		equ	30

NUM_ERRORS				equ	31

	ENDC
