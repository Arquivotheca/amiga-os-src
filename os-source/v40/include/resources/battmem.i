	IFND	RESOURCES_BATTMEM_I
RESOURCES_BATTMEM_I	SET	1
**
**	$Id: battmem.i,v 36.3 90/05/01 14:43:48 rsbx Exp $
**
**	BattMem resource name strings.
**
**	(C) Copyright 1989,1990 Commodore-Amiga Inc.
**		All Rights Reserved
**

BATTMEMNAME	MACRO
		dc.b	'battmem.resource',0
		ds.w	0
		ENDM

	ENDC	; RESOURCES_BATTMEM_I
