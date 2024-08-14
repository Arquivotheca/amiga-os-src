**
**	$Id: txtdata.i,v 39.1 92/07/08 11:02:47 darren Exp $
**
**	graphics library private text equates 
**
**	(C) Copyright 1988 Commodore-Amiga, Inc.
**	    All Rights Reserved
**


TE_MATCHWORD	EQU	19995

    BITDEF  TE0,BYTECELL,1	; the char data is all byte aligned
    BITDEF  TE0,KERNLESS,2	; char bits fit completely within character cell
    BITDEF  TE0,DUPLICATE,7	; Duplicate font - differs by DPI or DotSize
