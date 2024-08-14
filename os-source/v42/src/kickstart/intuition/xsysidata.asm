*
* This file contains the data for the vector image class.
* By downcoding it, we can use a WORD-sized pointer to vector structures,
* instead of a LONG.  This saves many bytes.  As well, some of the vector
* datas picked up unnecessary pad-bytes in C.
*
*  $Id: xsysidata.asm,v 40.0 94/02/15 17:48:34 davidj Exp $
*

		SECTION text,code

		INCLUDE	'intuition/screens.i'
		INCLUDE 'vectorclass.i'

_Vbase:

* Some shorthand definitions

VF_BOTH		EQU	VF_MONO!VF_COLOR

VS_NSI		EQU	VS_NORMAL!VS_SELECTED!VS_INANORMAL
VS_NS		EQU	VS_NORMAL!VS_SELECTED
VS_NI		EQU	VS_NORMAL!VS_INANORMAL
VS_SI		EQU	VS_SELECTED!VS_INANORMAL
VS_S		EQU	VS_SELECTED
VS_N		EQU	VS_NORMAL
VS_I		EQU	VS_INANORMAL


; window-depth gadget
Vwdepth3:	dc.b	1,9,4,19,8
Vwdepth2:	dc.b	7,5,2,15,2,15,4,9,4,9,6,5,6,5,2
Vwdepth1:	dc.b	1,5,2,15,6

; screen-depth gadget
Vsdepth4:	dc.b	1,0,0,22,10
Vsdepth3:	dc.b	1,8,4,18,8
Vsdepth2:	dc.b	7,4,2,14,2,14,4,8,4,8,6,4,6,4,2
Vsdepth1:	dc.b	1,4,2,14,6

; window-zoom gadget
Vwzoom2:	dc.b	3,6,2,18,8,6,2,12,5,7,2,11,5
Vwzoom1:	dc.b	1,7,2,11,5

; window-close gadget
Vwclose1:	dc.b	1,7,3,11,7

; window-sizing gadget
Vwsize1:	dc.b	6,4,7,14,7,14,2,13,2,4,6,4,7

; up-arrow
Vup1:		dc.b	9,4,7,8,3,9,3,13,7,12,7,9,5,8,5,5,7,4,7

; down-arrow
Vdown1:		dc.b	9,4,3,8,7,9,7,13,3,12,3,9,5,8,5,5,3,4,3

; left-arrow
Vleft1:		dc.b	5,10,2,5,4,10,6,8,4,10,2

; right-arrow
Vright1:	dc.b	5,5,2,10,4,5,6,7,4,5,2

; checkmark
Vcheck1:	dc.b	9,19,2,17,2,12,7,11,7,9,5,7,5,10,8,12,8,18,2

; mx-button
Vmx1:		dc.b	9,2,0,0,2,0,6,2,8,3,8,1,6,1,2,3,0,2,0
Vmx2:		dc.b	9,14,8,16,6,16,2,14,0,13,0,15,2,15,6,13,8,14,8
Vmx3:		dc.b	2,3,0,13,0
Vmx4:		dc.b	2,3,8,13,8
Vmx5:		dc.b	9,5,2,11,2,12,3,12,5,11,6,5,6,4,5,4,3,5,2

; menu-checkmark
Vmcheck1:	dc.b	10,14,0,12,0,6,6,5,6,3,4,0,4,1,4,4,7,6,7,13,0

; menu-Amiga-key
Vmamiga1:	dc.b	9,6,0,38,0,44,2,44,12,38,14,6,14,0,12,0,2,6,0
Vmamiga2:	dc.b	17,16,12,16,11,12,11,28,3,30,3,30,11,28,11,28,12
		dc.b	38,12,38,11,34,11,34,2,27,2,9,11,6,11,6,12,16,12
Vmamiga3:	dc.b	1,14,9,30,10


; These offsets will fit in a WORD-sized storage

Owdepth3	EQU	Vwdepth3-_Vbase
Owdepth2	EQU	Vwdepth2-_Vbase
Owdepth1	EQU	Vwdepth1-_Vbase

Osdepth4	EQU	Vsdepth4-_Vbase
Osdepth3	EQU	Vsdepth3-_Vbase
Osdepth2	EQU	Vsdepth2-_Vbase
Osdepth1	EQU	Vsdepth1-_Vbase

Owzoom2		EQU	Vwzoom2-_Vbase
Owzoom1		EQU	Vwzoom1-_Vbase

Owclose1	EQU	Vwclose1-_Vbase

Owsize1		EQU	Vwsize1-_Vbase

Oup1		EQU	Vup1-_Vbase

Odown1		EQU	Vdown1-_Vbase

Oleft1		EQU	Vleft1-_Vbase

Oright1		EQU	Vright1-_Vbase

Ocheck1		EQU	Vcheck1-_Vbase

Omx1		EQU	Vmx1-_Vbase
Omx2		EQU	Vmx2-_Vbase
Omx3		EQU	Vmx3-_Vbase
Omx4		EQU	Vmx4-_Vbase
Omx5		EQU	Vmx5-_Vbase

Omcheck1	EQU	Vmcheck1-_Vbase

Omamiga1	EQU	Vmamiga1-_Vbase
Omamiga2	EQU	Vmamiga2-_Vbase
Omamiga3	EQU	Vmamiga3-_Vbase

		XDEF	_Vbase

		XDEF	_vdepth
		XDEF	_vsdepth
		XDEF	_vzoom
		XDEF	_vclose
		XDEF	_vsize
		XDEF	_upar
		XDEF	_dnar
		XDEF	_lfar
		XDEF	_rtar
		XDEF	_ckmk
		XDEF	_mutx
		XDEF	_mchk
		XDEF	_amky

		cnop	0,2

* These are all struct Vector's:

* Window Depth
_vdepth:	dc.b	VF_FILLRECT!VF_COLOR!VF_BACKG,VS_NS
		dc.w	Owdepth1

		dc.b	VF_LINEPOLY!VF_COLOR!VF_SHADOW,VS_NSI
		dc.w	Owdepth2

		dc.b	VF_FILLRECT!VF_COLOR!VF_SHINE,VS_NS
		dc.w	Owdepth3

		dc.b	VF_LINERECT!VF_COLOR!VF_SHADOW,VS_NSI
		dc.w	Owdepth3

		dc.b	VF_LINERECT!VF_COLOR!VF_SHADOW,VS_S
		dc.w	Owdepth1

		dc.b	VF_LINEPOLY!VF_MONO!VF_BACKG,VS_N
		dc.w	Owdepth2

		dc.b	VF_FILLRECT!VF_MONO!VF_BACKG,VS_N
		dc.w	Owdepth3

		dc.b	VF_LINEPOLY!VF_MONO!VF_SHADOW,VS_SI
		dc.w	Owdepth2


		dc.b	VF_FILLRECT!VF_MONO!VF_SHADOW,VS_SI
		dc.w	Owdepth3


* Screen Depth
_vsdepth:	dc.b	VF_FILLRECT!VF_MONO!VF_SHADOW,VS_S
		dc.w	Osdepth4

		dc.b	VF_LINEPOLY!VF_COLOR!VF_SHADOW,VS_NSI
		dc.w	Osdepth2

		dc.b	VF_FILLRECT!VF_COLOR!VF_SHINE,VS_NS
		dc.w	Osdepth3


		dc.b	VF_LINERECT!VF_COLOR!VF_SHADOW,VS_NSI
		dc.w	Osdepth3

		dc.b	VF_LINERECT!VF_COLOR!VF_SHADOW,VS_S
		dc.w	Osdepth1

		dc.b	VF_LINEPOLY!VF_MONO!VF_SHADOW,VS_N
		dc.w	Osdepth2

		dc.b	VF_FILLRECT!VF_MONO!VF_SHADOW,VS_N
		dc.w	Osdepth3


		dc.b	VF_LINEPOLY!VF_MONO!VF_BACKG,VS_SI
		dc.w	Osdepth2

		dc.b	VF_FILLRECT!VF_MONO!VF_BACKG,VS_SI
		dc.w	Osdepth3


* Window Zoom
_vzoom:		dc.b	VF_FILLRECT!VF_COLOR!VF_SHINE,VS_N
		dc.w	Owzoom1

		dc.b	VF_FILLRECT!VF_COLOR!VF_SHINE,VS_S
		dc.w	Owzoom2

		dc.b	VF_FILLRECT!VF_BOTH!VF_FILL,VS_S
		dc.w	Owzoom1

		dc.b	VF_LINERECT!VF_MONO!VF_BACKG,VS_N
		dc.w	Owzoom2


		dc.b	VF_LINERECT!VF_MONO!VF_SHADOW,VS_SI
		dc.w	Owzoom2

		dc.b	VF_LINERECT!VF_COLOR!VF_SHADOW,VS_NSI
		dc.w	Owzoom2


* Window Close
_vclose:	dc.b	VF_FILLRECT!VF_BOTH!VF_SHINE,VS_N
		dc.w	Owclose1

		dc.b	VF_FILLRECT!VF_BOTH!VF_BACKG,VS_S
		dc.w	Owclose1

		dc.b	VF_LINERECT!VF_MONO!VF_BACKG,VS_N
		dc.w	Owclose1

		dc.b	VF_LINERECT!VF_MONO!VF_SHADOW,VS_SI
		dc.w	Owclose1


		dc.b	VF_LINERECT!VF_COLOR!VF_SHADOW,VS_NSI
		dc.w	Owclose1


* Window Size
_vsize:		dc.b	VF_LINEPOLY!VF_MONO!VF_BACKG,VS_NS
		dc.w	Owsize1

		dc.b	VF_FILLPOLY!VF_COLOR!VF_SHINE,VS_NS
		dc.w	Owsize1

		dc.b	VF_LINEPOLY!VF_COLOR!VF_SHADOW,VS_NS
		dc.w	Owsize1

		dc.b	VF_LINEPOLY!VF_BOTH!VF_SHADOW,VS_I
		dc.w	Owsize1


* Up Arrow
_upar:		dc.b	VF_FILLPOLY!VF_BOTH!VF_SHADOW,VS_NSI
		dc.w	Oup1

* OVERWRITES THE PREVIOUS BLACK ONE, for monochrome non-selected
		dc.b	VF_FILLPOLY!VF_MONO!VF_BACKG,VS_N
		dc.w	Oup1


* Down Arrow
_dnar:		dc.b	VF_FILLPOLY!VF_BOTH!VF_SHADOW,VS_NSI
		dc.w	Odown1

* OVERWRITES THE PREVIOUS BLACK ONE, for monochrome non-selected
		dc.b	VF_FILLPOLY!VF_MONO!VF_BACKG,VS_N
		dc.w	Odown1


* Left Arrow
_lfar:		dc.b	VF_FILLPOLY!VF_BOTH!VF_SHADOW,VS_NSI
		dc.w	Oleft1

* OVERWRITES THE PREVIOUS BLACK ONE, for monochrome non-selected
		dc.b	VF_FILLPOLY!VF_MONO!VF_BACKG,VS_N
		dc.w	Oleft1


* Right Arrow
_rtar:		dc.b	VF_FILLPOLY!VF_BOTH!VF_SHADOW,VS_NSI
		dc.w	Oright1

* OVERWRITES THE PREVIOUS BLACK ONE, for monochrome non-selected
		dc.b	VF_FILLPOLY!VF_MONO!VF_BACKG,VS_N
		dc.w	Oright1


* GadTools Checkmark
_ckmk:		dc.b	VF_FILLPOLY!VF_BOTH!VF_TEXT,VS_S
		dc.w	Ocheck1


* GadTools MX button
_mutx:
		dc.b	VF_FILLPOLY!VF_BOTH!VF_SHINE,VS_N
		dc.w	Omx1

		dc.b	VF_FILLPOLY!VF_BOTH!VF_SHADOW,VS_N
		dc.w	Omx2

		dc.b	VF_LINEPOLY!VF_BOTH!VF_SHINE,VS_N
		dc.w	Omx3

		dc.b	VF_LINEPOLY!VF_BOTH!VF_SHADOW,VS_N
		dc.w	Omx4

		dc.b	VF_FILLPOLY!VF_BOTH!VF_SHADOW,VS_S
		dc.w	Omx1

		dc.b	VF_FILLPOLY!VF_BOTH!VF_SHINE,VS_S
		dc.w	Omx2

		dc.b	VF_LINEPOLY!VF_BOTH!VF_SHADOW,VS_S
		dc.w	Omx3

		dc.b	VF_LINEPOLY!VF_BOTH!VF_SHINE,VS_S
		dc.w	Omx4

		dc.b	VF_FILLPOLY!VF_BOTH!VF_FILL,VS_S
		dc.w	Omx5


* Menu Checkmark
_mchk:		dc.b	VF_FILLPOLY!VF_BOTH!VF_BDETAIL,VS_NSI
		dc.w	Omcheck1


* Menu Amiga-key
_amky:		dc.b	VF_FILLPOLY!VF_BOTH!VF_BDETAIL,VS_NSI
		dc.w	Omamiga1

		dc.b	VF_FILLPOLY!VF_BOTH!VF_BBLOCK,VS_NSI
		dc.w	Omamiga2

		dc.b	VF_FILLRECT!VF_BOTH!VF_BBLOCK,VS_NSI
		dc.w	Omamiga3

		END
