* $Id: nmprintf.asm,v 1.3 91/04/24 20:53:35 peter Exp $
	xref	_printf
	xref	_putchar
	xref	_holdRTS
	xdef	_mnprintf

_mnprintf:   
* mnprintf( format, [arg], ... )
*           * print using exactly eight chars -- to print mnemonics
	    move.l (sp)+,_holdRTS
	    jsr	_printf
	    move.l #9,-(sp)
	    jsr _putchar
	    addq.l #4,sp
	    move.l _holdRTS,-(sp)
	    rts

