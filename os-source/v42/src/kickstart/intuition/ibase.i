	IFND  INTUITION_INTUITIONBASE_I
INTUITION_INTUITIONBASE_I SET  1
**
** $Id: ibase.i,v 40.0 94/02/15 18:06:52 davidj Exp $
**
** The INTERNAL PRIVATE part of  IntuitionBase structure
** and supporting structures
**
**  (C) Copyright 1985,1986,1987,1988,1989,1990 Commodore-Amiga, Inc.
**	    All Rights Reserved
**

	IFND EXEC_TYPES_I
	INCLUDE "exec/types.i"
	ENDC

	IFND EXEC_LIBRARIES_I
	INCLUDE "exec/libraries.i"
	ENDC

	IFND	GRAPHICS_VIEW_I
	INCLUDE	"graphics/view.i"
	ENDC

* jimm: 1/10/86: Intuition Locking
* Let me say it again: don't even think about using this information
* in a program.

ISTATELOCK	equ	0	; Intuition() not re-entrant
LAYERINFOLOCK	equ	1	; dummy lock used to check protocol
GADGETSLOCK	equ	2	; gadget lists, refresh, flags
LAYERROMLOCK	equ	3	; (dummy) for lock layerrom
IBASELOCK	equ	4	; protexts IBase pointers and lists
VIEWLOCK	equ	5	; access to ViewLord
VIEWCPRLOCK	equ	6	; Dummy for GfxBase ActiViewCprSemaphore
RPLOCK		equ	7	; use of IBase->RP
NUMILOCKS	equ	8

* Be sure to protect yourself against someone modifying these data as
* you look at them.  This is done by calling:
*
* lock = LockIBase(0), which returns a ULONG.  When done call
*  D0             D0
* UnlockIBase(lock) where lock is what LockIBase() returned.
*              A0
* NOTE: these library functions are simply stubs now, but should be called
* to be compatible with future releases.

* ======================================================================== *
* === IntuitionBase ====================================================== *
* ======================================================================== *
 STRUCTURE IntuitionBase,0

	STRUCT	ib_LibNode,LIB_SIZE
	STRUCT	ib_ViewLord,v_SIZEOF
	APTR	ib_ActiveWindow
	APTR	ib_ActiveScreen

* the FirstScreen variable points to the frontmost Screen.  Screens are
* then maintained in a front to back order using Screen.NextScreen

	APTR	ib_FirstScreen
	ULONG	ib_Flags	; private meaning
	WORD	ib_MouseY	; these are supposed to be backwards, 
	WORD	ib_MouseX	;  but weren't, recently

	ULONG	ib_Seconds
	ULONG	ib_Micros

* there is not 'sizeof' here because...
*
*

***** ***** ***** ***** ***** ***** ***** ***** *****/
***** ***** ***** PRIVATE DATA HERE ***** ***** *****/
***** ***** ***** ***** ***** ***** ***** ***** *****/
	STRUCT	ib_Private,1300

* --------------- base vectors -----------------------------------
* DO MOVE THESE OFFSETS WITHOUT ADJUSTING EQUATES IN IWORK.ASM 
* this is automatically handled by standalone program offsets.c
*
    APTR		ib_SysBase
    APTR		ib_GfxBase
    APTR		ib_LayersBase
    APTR		ib_UtilityBase
    APTR		ib_KeymapBase
    APTR		ib_InputDeviceTask

	ENDC


