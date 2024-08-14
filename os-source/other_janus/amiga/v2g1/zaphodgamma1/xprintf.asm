
* *****************************************************************************
*
* Routines for the Zaphod Project
* 
* Copyright (C) 1986, Commodore Amiga, Inc.
*
* CONFIDENTIAL and PROPRIETARY
*
* HISTORY	Name		Description
* ------------	--------------	----------------------------------------------
*
*
* *****************************************************************************

	INCLUDE "zaphod.i"

	PUBLIC	_printf
	PUBLIC	_GetA4


	
	DSEG
	EVEN
xA0	DS.L	1
xA1	DS.L	1
xA2	DS.L	1
xA3	DS.L	1
xA4	DS.L	1
xA5	DS.L	1
xA6	DS.L	1

xD0	DS.L	1
xD1	DS.L	1
xD2	DS.L	1
xD3	DS.L	1
xD4	DS.L	1
xD5	DS.L	1
xD6	DS.L	1
xD7	DS.L	1

returnaddress	DS.L	1


	CSEG
	PUBLIC	_xprintf

_xprintf:
	MOVE.L	A0,xA0
	MOVE.L	A1,xA1
	MOVE.L	A2,xA2
	MOVE.L	A3,xA3
	MOVE.L	A4,xA4
	MOVE.L	A5,xA5
	MOVE.L	A6,xA6

	MOVE.L	D0,xD0
	MOVE.L	D1,xD1
	MOVE.L	D2,xD2
	MOVE.L	D3,xD3
	MOVE.L	D4,xD4
	MOVE.L	D5,xD5
	MOVE.L	D6,xD6
	MOVE.L	D7,xD7

	MOVE.L	(SP)+,returnaddress

	JSR	_GetA4
	JSR	_printf

	MOVE.L	returnaddress,-(SP)

	MOVE.L	xA0,A0
	MOVE.L	xA1,A1
	MOVE.L	xA2,A2
	MOVE.L	xA3,A3
	MOVE.L	xA4,A4
	MOVE.L	xA5,A5
	MOVE.L	xA6,A6

	MOVE.L	xD0,D0
	MOVE.L	xD1,D1
	MOVE.L	xD2,D2
	MOVE.L	xD3,D3
	MOVE.L	xD4,D4
	MOVE.L	xD5,D5
	MOVE.L	xD6,D6
	MOVE.L	xD7,D7

	RTS

	END


