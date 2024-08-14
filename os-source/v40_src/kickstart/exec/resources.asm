*	   ************************************************
*	   **						 **
*	   **	   --------------------------------	 **
*	   **	    ROM OPERATING SYSTEM EXECUTIVE	 **
*	   **	   --------------------------------	 **
*	   **						 **
*	   **		   Resource Support		 **
*	   **						 **
*	   ************************************************

*************************************************************************
*									*
*   Copyright 1984,85,88,89 Commodore-Amiga, Inc.			*
*   All rights reserved.  No part of this program may be reproduced,	*
*   transmitted, transcribed, stored in retrieval system, or		*
*   translated into any language or computer language, in any form	*
*   or by any means, electronic, mechanical, magnetic, optical, 	*
*   chemical, manual or otherwise, without the prior written		*
*   permission of Commodore-Amiga, Incorporated.			*
*									*
*************************************************************************

**********************************************************************
*
*   Source Control:
*
*	$Id: resources.asm,v 39.1 92/03/16 13:31:42 mks Exp $
*
*	$Log:	resources.asm,v $
* Revision 39.1  92/03/16  13:31:42  mks
* Optimized the RemResource and FindResource code
* 
* Revision 39.0  91/10/15  08:28:42  mks
* V39 Exec initial checkin
*
**********************************************************************


;****** Included Files ***********************************************

    NOLIST
    INCLUDE	"assembly.i"

    INCLUDE	"types.i"
    INCLUDE	"nodes.i"
    INCLUDE	"lists.i"
    INCLUDE	"libraries.i"
    INCLUDE	"execbase.i"
    INCLUDE	"ables.i"

    INCLUDE	"calls.i"
    LIST


;****** Imported Globals *********************************************

    TASK_ABLES

    EXTERN_BASE ResourceList


;****** Imported Functions *******************************************

    EXTERN_CODE AddNode
    EXTERN_CODE FindNode


;****** Exported Functions *******************************************

    XDEF	AddResource
;    XDEF	RemResource
    XDEF	OpenResource



*****o* exec.library/AddResource ********************************************
*
*   NAME
*	AddResource -- add a resource to the system
*
*   SYNOPSIS
*	AddResource(resource)
*		    A1
*
*   FUNCTION
*	This function adds a new resource to the system and makes it
*	available to other users.  The resource should be ready to
*	be called at this time.
*
*   INPUTS
*	resource - pointer to a properly initialized resource node
*
*   SEE ALSO
*	RemResource, OpenResource
*
**********************************************************************

AddResource:
	    LEA     ResourceList(A6),A0
	    BRA     AddNode


*****o* exec.library/RemResource ********************************************
*
*   NAME
*	RemResource -- remove a resource from the system
*
*   SYNOPSIS
*	RemResource(resource)
*		    A1
*
*   FUNCTION
*	This function removes an existing resource from the system.
*
*   INPUTS
*	resource - pointer to a resource node
*
*   SEE ALSO
*	AddResource
*
**********************************************************************

;RemResource:	equ	RemNode		; Just do RemNode


*****o* exec.library/OpenResource *******************************************
*
*   NAME
*	OpenResource -- gain access to a resource
*
*   SYNOPSIS
*	resource = OpenResource(resName)
*	D0			A1
*
*   FUNCTION
*	This function returns a pointer to a resource that was prev-
*	iously installed into the system.
*
*   INPUTS
*	resName - the name of the resource requested.
*
*   RESULTS
*	resource - if successful, a resource pointer, else null
*
*   SEE ALSO
*	CloseResource
*
**********************************************************************

OpenResource:	LEA.L	ResourceList(A6),A0
		bra	FindNode	* (a0,a1)

    END
