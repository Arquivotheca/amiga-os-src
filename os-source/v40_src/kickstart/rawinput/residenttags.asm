	TTL    '$Id: residenttags.asm,v 36.7 90/04/13 12:45:50 kodiak Exp $
**********************************************************************
*                                                                    *
*   Copyright 1985, Commodore-Amiga Inc.   All rights reserved.      *
*   No part of this program may be reproduced, transmitted,          *
*   transcribed, stored in retrieval system, or translated into      *
*   any language or computer language, in any form or by any         *
*   means, electronic, mechanical, magnetic, optical, chemical,      *
*   manual or otherwise, without the prior written permission of     *
*   Commodore-Amiga Incorporated, 983 University Ave. Building #D,   *
*   Los Gatos, California, 95030                                     *
*                                                                    *
**********************************************************************
*
*	raw input devices initialization tag
*
**********************************************************************

	SECTION		rawinput

*------ Included Files -----------------------------------------------

	INCLUDE		"exec/types.i"
	INCLUDE		"exec/nodes.i"
	INCLUDE		"exec/resident.i"
	INCLUDE		"exec/strings.i"

	INCLUDE		"rawinput_rev.i"
	INCLUDE		"kbdata.i"
	INCLUDE		"gpdata.i"
	INCLUDE		"inputevent.i"
	INCLUDE		"iddata.i"


*------ Imported Names -----------------------------------------------

	XREF		GDFuncInit
	XREF		GDStructInit
	XREF		GDInit
	XREF		KDFuncInit
	XREF		KDStructInit
	XREF		KDInit
	XREF		IDFuncInit
	XREF		IDStructInit
	XREF		IDInit
	XREF		EndModule


*------ Exported Names -----------------------------------------------

	XDEF		GDName
	XDEF		KDName
	XDEF		IDName
	XDEF		VERSION
	XDEF		REVISION


**********************************************************************

residentGD:
		DC.W	RTC_MATCHWORD
		DC.L	residentGD
		DC.L	residentKD
		DC.B	RTF_AUTOINIT!RTW_COLDSTART
		DC.B	VERSION
		DC.B	NT_DEVICE
		DC.B	60
		DC.L	GDName
		DC.L	identGD
		DC.L	autoGD

autoGD:
		dc.l	gd_SIZEOF
		dc.l	GDFuncInit
		dc.l	GDStructInit
		dc.l	GDInit


residentKD:
		DC.W	RTC_MATCHWORD
		DC.L	residentKD
		DC.L	residentID
		DC.B	RTF_AUTOINIT!RTW_COLDSTART
		DC.B	VERSION
		DC.B	NT_DEVICE
		DC.B	45
		DC.L	KDName
		DC.L	identKD
		DC.L	autoKD

autoKD:
		dc.l	kd_SIZEOF
		dc.l	KDFuncInit
		dc.l	KDStructInit
		dc.l	KDInit


residentID:
		DC.W	RTC_MATCHWORD
		DC.L	residentID
		DC.L	EndModule
		DC.B	RTF_AUTOINIT!RTW_COLDSTART
		DC.B	VERSION
		DC.B	NT_DEVICE
		DC.B	40
		DC.L	IDName
		DC.L	identID
		DC.L	autoID

autoID:
		dc.l	id_SIZEOF
		dc.l	IDFuncInit
		dc.l	IDStructInit
		dc.l	IDInit

GDName:
		STRING	<'gameport.device'>
identGD:
		DC.B	'gameport '
		VSTRING

KDName:
		STRING	<'keyboard.device'>
identKD:
		DC.B	'keyboard '
		VSTRING

IDName:
		STRING	<'input.device'>
identID:
		DC.B	'input '
		VSTRING

		END
