/* -----------------------------------------------------------------------
 * xprlink.c		handshake_src
 *
 * $Locker:  $
 *
 * $Id: xprlink.asm,v 1.1 91/05/09 15:47:23 bj Exp $
 *
 * $Revision: 1.1 $
 *
 * $Header: HOG:Other/inet/src/c/hs/RCS/xprlink.asm,v 1.1 91/05/09 15:47:23 bj Exp $
 *
 * $Log:	xprlink.asm,v $
* Revision 1.1  91/05/09  15:47:23  bj
* Initial revision
* 
* Revision 1.1  91/05/09  15:45:02  bj
* Initial revision
* 
* Revision 1.1  91/05/09  15:37:01  bj
* Initial revision
* 
 *
 *------------------------------------------------------------------------
 */

;  liblink.asm -- jimm mackraz, oct 28, 1986
;  application side interface for C-routines calling example library

	include "exec/types.i"
	include "exec/libraries.i"

	section text,code

	LIBINIT
	LIBDEF	_LVOXProtocolCleanup
	LIBDEF	_LVOXProtocolSetup
	LIBDEF	_LVOXProtocolSend
	LIBDEF	_LVOXProtocolReceive
	LIBDEF	_LVOXProtocolHostMon
	LIBDEF	_LVOXProtocolUserMon


	; --- xref from application
	xref	_XProtocolBase

	; --- xdef for application
        xdef	_XProtocolCleanup
        xdef	_XProtocolSetup
        xdef	_XProtocolSend
	    xdef	_XProtocolReceive
        xdef    _XProtocolHostMon
        xdef    _XProtocolUserMon

_XProtocolCleanup:
	movea.l	4(sp),a0		; First argument on stack
	move.l	_XProtocolBase,a6
	jmp	_LVOXProtocolCleanup(a6)

_XProtocolSetup:
	movea.l	4(sp),a0		; First argument on stack
	move.l	_XProtocolBase,a6
	jmp	_LVOXProtocolSetup(a6)

_XProtocolSend:
	movea.l	4(sp),a0		; First argument on stack
	move.l	_XProtocolBase,a6
	jmp	_LVOXProtocolSend(a6)

_XProtocolReceive:
	movea.l	4(sp),a0		; First argument on stack
	move.l	_XProtocolBase,a6
	jmp	_LVOXProtocolReceive(a6)


_XProtocolHostMon:
	move.l	4(sp),a0		; First  argument on stack
	move.l	8(sp),a1		; Second argument on stack
	move.l	12(sp),d0		; Third  argument on stack
	move.l	16(sp),d1		; Fourth argument on stack
	move.l	_XProtocolBase,a6
	jmp	_LVOXProtocolHostMon(a6)

_XProtocolUserMon:
	move.l	4(sp),a0		; First  argument on stack
	move.l	8(sp),a1		; Second argument on stack
	move.l	12(sp),d0		; Third  argument on stack
	move.l	16(sp),d1		; Fourth argument on stack
	move.l	_XProtocolBase,a6
	jmp	_LVOXProtocolUserMon(a6)


    end
