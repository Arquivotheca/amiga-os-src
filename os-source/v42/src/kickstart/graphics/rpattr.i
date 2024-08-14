	IFND GRAPHICS_RPATTR_H
GRAPHICS_RPATTR_H set 1

**
**	$Id: rpattr.i,v 42.0 93/06/16 11:11:12 chrisg Exp $
**
**	tag definitions for GetRPAttr, SetRPAttr
**
**

RPTAG_Font 		equ	$80000000	; get/set font 
RPTAG_APen 		equ	$80000002	; get/set apen 
RPTAG_BPen 		equ	$80000003	; get/set bpen 
RPTAG_DrMd 		equ	$80000004	; get/set draw mode 
RPTAG_OutLinePen 	equ	$80000005	; get/set outline pen 
RPTAG_OutlinePen 	equ	$80000005	; get/set outline pen. corrected case.
RPTAG_WriteMask 	equ	$80000006	; get/set WriteMask 
RPTAG_MaxPen 		equ	$80000007	; get/set maxpen 

RPTAG_DrawBounds	equ	$80000008	; *get-only* rastport draw bounds. pass &rect 

	endc	; GRAPHICS_RPATTR_H
