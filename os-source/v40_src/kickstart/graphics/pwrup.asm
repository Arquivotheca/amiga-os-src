*******************************************************************************
*
*	$Id: pwrup.asm,v 39.2 93/02/09 15:57:34 chrisg Exp $
*
*******************************************************************************

    include 'exec/types.i'
    include 'exec/nodes.i'
    include 'exec/lists.i'
    include 'exec/resident.i'
    include 'gfxbase.i'
    include 'graphics_rev.i'

IC_COLDRESET    equ 1

    section graphics

    xdef	_SysBaseOffset
_SysBaseOffset equ	gb_ExecBase

    xdef	_UtilityBaseOffset
_UtilityBaseOffset equ	gb_UtilBase

    xref	gfx_end

gfx_tag:
    dc.w    RTC_MATCHWORD
    dc.l    gfx_tag
    dc.l    gfx_end
    dc.b    RTW_COLDSTART
    dc.b    VERSION  * version number
    dc.b    NT_LIBRARY
    dc.b    65  * initialization priority
    dc.l    _gfx_lib_name    * node name
    dc.l    _rev_name
    dc.l    apwrup

	xdef	_gfx_lib_name
_gfx_lib_name    dc.b    'graphics.library',0

*	rev_name    dc.b    'graphics 33.99 July 23 1986',10,0
	xdef	_rev_name
_rev_name	VSTRING

	xdef	_VERSION
	xdef	_REVISION
_VERSION	EQU	VERSION
_REVISION	EQU	REVISION

    ds.w    0

	xref	_LVOMakeLibrary
	xref	_vectbl
	xref    _cpwrup

apwrup: movem.l a2,-(sp)
	moveq.l #0,d1
	move.l	#gb_SIZE,d0
	move.l  d1,a2
	move.l  d1,a1
	movea.l	#_vectbl,a0
	jsr	_LVOMakeLibrary(a6)
	tst.l	d0
	beq.s	exit
	move.l	d0,a2
	move.l	a6,gb_ExecBase(a2)
	move.l	a2,a6 				* stuff graphics base
	jsr	_cpwrup
	move.l	gb_ExecBase(a2),a6		* restore exec base
exit:	movem.l (sp)+,a2
	rts

	end
