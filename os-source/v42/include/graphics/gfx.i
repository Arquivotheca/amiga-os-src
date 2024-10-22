	IFND	GRAPHICS_GFX_I
GRAPHICS_GFX_I	SET	1
**
**	$Id: gfx.i,v 42.1 93/07/20 13:25:06 chrisg Exp $
**
**	
**
**	(C) Copyright 1985,1986,1987,1988,1989 Commodore-Amiga, Inc.
**	    All Rights Reserved
**

    IFND    EXEC_TYPES_I
    include 'exec/types.i'
    ENDC

BITSET      equ $8000
BITCLR      equ 0
AGNUS       equ 1
DENISE      equ 1

    STRUCTURE   BitMap,0
    WORD    bm_BytesPerRow
    WORD    bm_Rows
    BYTE    bm_Flags
    BYTE    bm_Depth
    WORD    bm_Pad
    STRUCT  bm_Planes,8*4
    LABEL   bm_SIZEOF

   STRUCTURE   Rectangle,0
      WORD  ra_MinX
      WORD  ra_MinY
      WORD  ra_MaxX
      WORD  ra_MaxY
   LABEL    ra_SIZEOF

   STRUCTURE   Rect32,0
      LONG  r32_MinX
      LONG  r32_MinY
      LONG  r32_MaxX
      LONG  r32_MaxY
   LABEL    r32_SIZEOF

   STRUCTURE   tPoint,0
      WORD  tpt_x
      WORD  tpt_y
   LABEL    tpt_SIZEOF

    BITDEF  BM,CLEAR,0
    BITDEF  BM,DISPLAYABLE,1
	BITDEF	BM,INTERLEAVED,2
	BITDEF	BM,STANDARD,3
	BITDEF	BM,MINPLANES,4
	BITDEF	BM,FASTMEM,5



BMA_HEIGHT	equ	0
BMA_DEPTH	equ	4
BMA_WIDTH	equ	8
BMA_FLAGS	equ	12

    ENDC	; GRAPHICS_GFX_I
