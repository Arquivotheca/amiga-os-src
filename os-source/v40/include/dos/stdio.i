	IFND DOS_STDIO_I
DOS_STDIO_I	EQU	1
**
**	$Id: stdio.i,v 37.2 91/11/01 22:24:39 jesup Exp $
**
**	ANSI-like stdio defines for dos buffered I/O
**
**	(C) Copyright 1989,1990,1991 Commodore-Amiga, Inc.
**	    All Rights Reserved
**


* types for SetVBuf()
BUF_LINE	EQU	0	; flush on \n, etc
BUF_FULL	EQU	1	; never flush except when needed
BUF_NONE	EQU	2	; no buffering

* EOF return value
ENDSTREAMCH	EQU	-1

	ENDC	; DOS_STDIO_I

