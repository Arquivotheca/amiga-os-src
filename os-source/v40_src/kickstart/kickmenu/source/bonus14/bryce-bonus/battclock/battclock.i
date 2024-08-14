******************************************************************
*                                                                *
* Copyright (C) 1989, Commodore Amiga Inc.  All rights reserved. *
*                                                               *
******************************************************************


*****************************************************************
*
* SOURCE CONTROL
* ------ -------
* $Header: battclock.i,v 36.1 89/10/06 13:58:24 rsbx Exp $
*
* $Locker:  $
*
*****************************************************************

	IFND	RESOURCES_BATTCLOCK_I
RESOURCES_BATTCLOCK_I	SET	1

BATTCLOCKNAME	MACRO
		dc.b	'battclock.resource',0
		ds.w	0
		ENDM

	ENDC
