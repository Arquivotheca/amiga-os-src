; ---------------------------------------------------------------------------------
; face.asm		handshake_src
;
; $Locker:  $
;
; $Id: face.asm,v 1.1 91/05/09 16:35:28 bj Exp $
;
; $Revision: 1.1 $
;
; $Header: HOG:Other/inet/src/c/hs/RCS/face.asm,v 1.1 91/05/09 16:35:28 bj Exp $
;
;-----------------------------------------------------------------------------------




;;; vltface.asm
;
;   DESCRIPTION:
;   ===========
;
;	This is an interface to VLT callback functions to be handed to
;	external protocol libraries.
;
;   AUTHOR/DATE:  W.G.J. Langeveld, March 1989.
;   ============
;
;;;

setup	macro
	movem.l	d2-d7/a2-a6,-(sp)
	endm

restore	macro
	movem.l	(sp)+,d2-d7/a2-a6	
	rts
	endm

	xdef	_a_fopen
	xref	_xpr_fopen
	xdef	_a_fclose
	xref	_xpr_fclose
	xdef	_a_fread
	xref	_xpr_fread
	xdef	_a_fwrite
	xref	_xpr_fwrite
	xdef	_a_fseek
	xref	_xpr_fseek
	xdef	_a_sread
	xref	_xpr_sread
	xdef	_a_swrite
	xref	_xpr_swrite
	xdef	_a_update
	xref	_xpr_update
	xdef	_a_chkabort
	xref	_xpr_chkabort
	xdef	_a_chkmisc
	xref	_xpr_chkmisc
	xdef	_a_gets
	xref	_xpr_gets
	xdef	_a_setserial
	xref	_xpr_setserial
	xdef	_a_ffirst
	xref	_xpr_ffirst
	xdef	_a_fnext
	xref	_xpr_fnext
	xdef	_a_finfo
	xref	_xpr_finfo
	xdef	_a_sflush
	xref	_xpr_sflush
;	xdef    _a_options
;	xref	_xpr_options

_a_fopen:
	setup
	jsr	_xpr_fopen
	restore

_a_fclose:
	setup
	jsr	_xpr_fclose
	restore

_a_fread:
	setup
	jsr	_xpr_fread
	restore

_a_fwrite:
	setup
	jsr	_xpr_fwrite
	restore

_a_fseek:
	setup
	jsr	_xpr_fseek
	restore

_a_sread:
	setup
	jsr	_xpr_sread
	restore

_a_swrite:
	setup
	jsr	_xpr_swrite
	restore

_a_update:
	setup
	jsr	_xpr_update
	restore

_a_chkabort:
	setup
	jsr	_xpr_chkabort
	restore

_a_chkmisc:
	setup
	jsr	_xpr_chkmisc
	restore

_a_gets:
	setup
	jsr	_xpr_gets
	restore

_a_ffirst
	setup
	jsr	_xpr_ffirst
	restore

_a_fnext
	setup
	jsr	_xpr_fnext
	restore

_a_sflush
	setup
	jsr	_xpr_sflush
	restore

_a_setserial
	setup
	jsr	_xpr_setserial
	restore

_a_finfo
	setup
	jsr	_xpr_finfo
	restore

;_a_options
;	setup
;	jsr	_xpr_options
;	restore

    end
