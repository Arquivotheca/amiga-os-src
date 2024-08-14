

*          ************************************************ 
*          **  Math Library C Compiler Interface         ** 
*          **                                            ** 
*          ************************************************ 
***********************************************************************
**                                                                    *
**   Copyright 1984, Amiga Computer Inc.   All rights reserved.       *
**   No part of this program may be reproduced, transmitted,          *
**   transcribed, stored in retrieval system, or translated into      *
**   any language or computer language, in any form or by any         *
**   means, electronic, mechanical, magnetic, optical, chemical,      *
**   manual or otherwise, without the prior written permission of     *
**   Amiga Computer Incorporated, 3350 Scott Blvd, Bld #7,            *
**   Santa Clara, CA 95051                                            *
**                                                                    *
***********************************************************************

* 3-26-86  Dale converted to use new greenhills compiler, cleaned up abit
* 11:50 04-Jun-85 LAH Broke up math library to create ROM, RAM and Link.
* 15:50 14-Apr-85 KBS Converted to V25 standard compatible library.
* 09:35 28-Jan-85 MJS moved float/string conversion to V2xstr.asm
* 14:41  1-Jan-85 MJS added float/string conversion
* 16:00 12-Dec-84 MJS first version
* 10:56 30-Dec-84 MJS version for RAM or ROM math library

                xref     _MathIeeeDoubBasBase

*------ Basic Math Function Vector Offsets --------------------------

                xref    _LVOIEEEDPFix,_LVOIEEEDPFlt
                xref    _LVOIEEEDPCmp,_LVOIEEEDPTst,_LVOIEEEDPAbs
                xref    _LVOIEEEDPNeg,_LVOIEEEDPAdd
                xref    _LVOIEEEDPSub,_LVOIEEEDPMul,_LVOIEEEDPDiv

                xdef    ffixi,fflti
                xdef    fcmpi
                xdef    faddi,fsubi,fmuli,fdivi

* new ones from Dale
* added 3/26/86
* these have parameters already in registers d0,d1
* these are routines just for single precision FLOAT

shit_do:
                movem.l d2/d3/a6,-(sp)
		move.l	_MathIeeeDoubBasBase,a6
		add.l	a6,a0
		movem.l	16(sp),d2/d3
*               move.l  16(sp),d2               * get second parameter
*               move.l  20(sp),d3
                jsr     (a0)
                movem.l (sp)+,d2/d3/a6             * restore
                move.l  (sp),8(sp)              * setup return address
                addq.l  #8,sp                   * non-standard c-interface
                rts

* hack stuff here
fmuli:
                lea     _LVOIEEEDPMul,a0
                bra     shit_do
faddi:
                lea     _LVOIEEEDPAdd,a0
                bra     shit_do
fsubi:
                lea     _LVOIEEEDPSub,a0
                bra     shit_do
fdivi:
                lea     _LVOIEEEDPDiv,a0
                bra     shit_do
fcmpi:
                lea     _LVOIEEEDPCmp,a0
*               bra     shit_do         ; can't do this, need cc's
                movem.l d2/d3/a6,-(sp)
                move.l   _MathIeeeDoubBasBase,a6
		add.l	a6,a0
		movem.l	16(sp),d2/d3
*               move.l  16(sp),d2               * get second parameter
*               move.l  20(sp),d3
                jsr     (a0)
                movem.l (sp)+,d2/d3/a6             * restore
                move.l  (sp),8(sp)              * setup return address
                addq.l  #8,sp                   * non-standard c-interface
                tst.l   d0
                rts

single_do:
	move.l	a6,-(sp)
	move.l	_MathIeeeDoubBasBase,a6
	add.l	a6,a0	
	jsr	(a0)
	move.l	(sp)+,a6
	rts

*	unsupported yet
ffixi:
                lea	_LVOIEEEDPFix,a0	; sign extended I hope
		bra	single_do

fflti:
                lea	_LVOIEEEDPFlt,a0
		bra	single_do

                end

