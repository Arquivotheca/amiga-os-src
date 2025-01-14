	IFND UTILITY_NAME_I
UTILITY_NAME_I	EQU	1
**
**	$Id: name.i,v 39.3 93/02/10 15:06:07 vertex Exp $
**
**	Namespace definitions
**
**	(C) Copyright 1992 Commodore-Amiga, Inc.
**	All Rights Reserved
**

;---------------------------------------------------------------------------

	IFND EXEC_TYPES_I
	INCLUDE "exec/types.i"
	ENDC

;---------------------------------------------------------------------------

* The named object structure */
*
   STRUCTURE NamedObject,0
	APTR	no_Object	; Your pointer, for whatever you want
   LABEL NamedObject_End

ANO_NameSpace	equ	4000	; Tag to define namespace
ANO_UserSpace	equ	4001	; tag to define userspace
ANO_Priority	equ	4002	; tag to define priority
ANO_Flags	equ	4003	; tag to define flags

* flags for tag ANO_Flags

	BITDEF	NS,NODUPS,0	; defaults to allowing duplicates
	BITDEF	NS,CASE,1	; so it defaults to caseless

;---------------------------------------------------------------------------

	ENDC	; UTILITY_NAME_I
