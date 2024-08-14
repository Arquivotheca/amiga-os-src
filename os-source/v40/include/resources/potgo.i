	IFND	RESOURCES_POTGO_I
RESOURCES_POTGO_I	EQU	1
**
**	$Id: potgo.i,v 36.0 90/04/13 11:32:48 kodiak Exp $
**
**	potgo resource name
**
**	(C) Copyright 1986 Commodore-Amiga, Inc.
**	    All Rights Reserved
**
POTGONAME MACRO
		dc.b	'potgo.resource',0
		ds.w	0
	ENDM

	ENDC	; RESOURCES_POTGO_I
