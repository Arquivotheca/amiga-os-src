

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

                xref     _MathIeeeSingBasBase

*------ Basic Math Function Vector Offsets --------------------------

                xref    _LVOIEEESPFix,_LVOIEEESPFlt
                xref    _LVOIEEESPCmp,_LVOIEEESPTst,_LVOIEEESPAbs
                xref    _LVOIEEESPNeg,_LVOIEEESPAdd
                xref    _LVOIEEESPSub,_LVOIEEESPMul,_LVOIEEESPDiv

                xdef    fsfixi,ffltis
                xdef    fscmpi
		xdef	fsmuli,fsdivi,fssubi,fsaddi

* new ones from Dale
* added 3/26/86
* these have parameters already in registers d0,d1
* these are routines just for single precision FLOAT

fsmuli:
		move.l	a6,-(sp)
		move.l	_MathIeeeSingBasBase,a6
		jsr	_LVOIEEESPMul(a6)
		move.l	(sp)+,a6
		rts
fsaddi:
		move.l	a6,-(sp)
		move.l	_MathIeeeSingBasBase,a6
		jsr	_LVOIEEESPAdd(a6)
		move.l	(sp)+,a6
		rts
fssubi:
		move.l	a6,-(sp)
		move.l	_MathIeeeSingBasBase,a6
		jsr	_LVOIEEESPSub(a6)
		move.l	(sp)+,a6
		rts
fsdivi:
		move.l	a6,-(sp)
		move.l	_MathIeeeSingBasBase,a6
		jsr	_LVOIEEESPDiv(a6)
		move.l	(sp)+,a6
		rts
fscmpi:
                movem.l a6,-(sp)
                move.l   _MathIeeeSingBasBase,a6
		jsr	_LVOIEEESPCmp(a6)
                move.l (sp)+,a6             * restore
                rts

*	unsupported yet
fsfixi:
	move.l	a6,-(sp)
	move.l	_MathIeeeSingBasBase,a6
	jsr	_LVOIEEESPFix(a6)
	move.l	(sp)+,a6
	rts

ffltis:
	move.l	a6,-(sp)
	move.l	_MathIeeeSingBasBase,a6
	jsr	_LVOIEEESPFlt(a6)
	move.l	(sp)+,a6
	rts

               end

