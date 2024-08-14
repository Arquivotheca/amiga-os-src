*************************************************************************
*									*
*	Copyright (C) 1989, Commodore Amiga Inc.  All rights reserved.	*
*									*
*************************************************************************


*************************************************************************
*
* internal.i
*
* Source Control
* ------ -------
* 
* $Id: internal.i,v 36.3 91/01/09 10:08:28 rsbx Exp $
*
*************************************************************************

	INCLUDE	"exec/types.i"
	INCLUDE	"exec/semaphores.i"
	INCLUDE	"exec/resident.i"
	INCLUDE	"exec/libraries.i"
	INCLUDE	"exec/initializers.i"
	INCLUDE	"asmsupp.i"
	INCLUDE	"battmem.i"
	INCLUDE	"battmem_rev.i"
	INCLUDE	"resources/battclock.i"


 STRUCTURE BattMemResource,LIB_SIZE
	UWORD	BTM_Magic

	APTR	BTM_Exec
	APTR	BTM_BattClock

	STRUCT	BTM_Semaphore,SS_SIZE

	LABEL	BTM_SIZE
