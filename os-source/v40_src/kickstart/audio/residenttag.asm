	TTL    '$Id: residenttag.asm,v 36.2 90/08/29 17:54:18 mks Exp $'
**********************************************************************
*                                                                    *
*   Copyright 1986, Commodore-Amiga Inc.   All rights reserved.      *
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
* $Id: residenttag.asm,v 36.2 90/08/29 17:54:18 mks Exp $
*
* $Locker:  $
*
* $Log:	residenttag.asm,v $
* Revision 36.2  90/08/29  17:54:18  mks
* Changed revision to 36...  Change ownership to MKS...
* Changed $Header to $Id
* Changed init priority to -120 as per spec...
* 
* Revision 32.2  89/02/27  12:16:53  kodiak
* remove COLDSTART flag, drop priority to -128
*
* Revision 32.1  86/01/14  21:23:21  sam
* revision set to 32
*
* Revision 1.1  86/01/14  20:48:42  sam
* Initial revision
*
*
**********************************************************************

	SECTION		audio

*------ Included Files -----------------------------------------------

	INCLUDE		"exec/types.i"
	INCLUDE		"exec/nodes.i"
	INCLUDE		"exec/resident.i"
	INCLUDE		"exec/strings.i"
        INCLUDE         "exec/lists.i"
        INCLUDE         "exec/memory.i"
        INCLUDE         "exec/ports.i"
        INCLUDE         "exec/libraries.i"
        INCLUDE         "exec/devices.i"
        INCLUDE         "exec/tasks.i"
        INCLUDE         "exec/io.i"
        INCLUDE         "exec/interrupts.i"
        INCLUDE         "exec/errors.i"
        INCLUDE         "exec/initializers.i"

	INCLUDE		"audio.i"
	INCLUDE		"device.i"
	INCLUDE		"audio_rev.i"


*------ Imported Names -----------------------------------------------

	XREF		ADEndModule


*------ Imported Functions -------------------------------------------

        XREF            _CADInit
        XREF            ADOpen
        XREF            ADClose
        XREF            ADExpunge
        XREF            ADExtFunc
        XREF            ADBeginIO
        XREF            ADAbortIO


*------ Exported Names -----------------------------------------------

	XDEF		_ADName


**********************************************************************

residentAD:
		DC.W	RTC_MATCHWORD
		DC.L	residentAD
		DC.L	ADEndModule
		DC.B	RTF_AUTOINIT
		DC.B	VERSION
		DC.B	NT_DEVICE
		DC.B	-120
		DC.L	_ADName
		DC.L	identAD
		DC.L	ADNode

_ADName:
		STRING	<'audio.device'>
identAD:
		VSTRING

**********************************************************************
*
*   NAME
*       Node - Autoinit node to initialize the Audio device
*
*   FUNCTION
*       This node is used to initialize the device.  It should only be
*       used if the device is not available to be opened (i.e. it has
*       been successfully Expunged).
*
**********************************************************************
ADNode:
		DC.L    ad_SIZEOF
		DC.L    adFuncInit
		DC.L    adStructInit
		DC.L    adInit

adStructInit:
*           ;------ Initialize the library
		INITBYTE    ln_Type,NT_DEVICE
		INITLONG    ln_Name,_ADName
		INITBYTE    lib_Flags,LIBF_SUMUSED!LIBF_CHANGED
		INITWORD    lib_Version,VERSION
		INITWORD    lib_Revision,REVISION
		INITLONG    lib_IdString,identAD
		DC.W        0

adInit:
*           ;------ This is called from within MakeLibrary, after
*           ;------ all the memory has been allocated
                MOVEM.L D0/A0,-(SP)
                BSR     _CADInit
                ADDQ.L	#8,SP
                RTS

adFuncInit:
                DC.W    -1

                DC.W    ADOpen+(*-adFuncInit)
                DC.W    ADClose+(*-adFuncInit)
                DC.W    ADExpunge+(*-adFuncInit)
                DC.W    ADExtFunc+(*-adFuncInit)
                DC.W    ADBeginIO+(*-adFuncInit)
                DC.W    ADAbortIO+(*-adFuncInit)

                DC.W    -1

		END
