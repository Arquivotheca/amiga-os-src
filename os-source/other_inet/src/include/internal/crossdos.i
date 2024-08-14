	IFND	INTERNAL_CROSSDOS_I
INTERNAL_CROSSDOS_I	SET	1
**
**	$Id: crossdos.i,v 39.1 92/10/01 14:42:12 vertex Exp $
**
**	Private structures for CrossDOS commodity <--> handler interfacing
**
**	(C) Copyright 1991 Commodore-Amiga, Inc.
**	All Rights Reserved
**

;---------------------------------------------------------------------------

    IFND EXEC_TYPES_I
    INCLUDE "exec/types.i"
    ENDC

    IFND EXEC_NODES_I
    INCLUDE "exec/nodes.i"
    ENDC

    IFND EXEC_LISTS_I
    INCLUDE "exec/lists.i"
    ENDC

    IFND EXEC_TASKS_I
    INCLUDE "exec/tasks.i"
    ENDC

    IFND EXEC_SEMAPHORES_I
    INCLUDE "exec/semaphores.i"
    ENDC

;---------------------------------------------------------------------------

; This is the global semaphore structure from which life begins
   STRUCTURE CrossDOSLock,SS_SIZE
	UWORD	cdl_StructSize			; size of this structure
	STRUCT	cdl_Handlers,LH_SIZE		; list of CrossDOSHandler
	STRUCT	cdl_TransTables,LH_SIZE		; list of CrossDOSTrans
	STRUCT	cdl_Reserved,16			; for future expansion
   LABEL CrossDOSLock_SIZEOF


;---------------------------------------------------------------------------


;* There is one of these per handler.
;* The cdh_TransTable field points to a translation table. This field may
;* be NULL in which case the default translation should be applied
;*
   STRUCTURE CrossDOSHandler,LN_SiZE
	UWORD   cdh_StructSize		; size of this structure
	ULONG   cdh_Flags
	APTR	cdh_TransTable

	; When the handler changes the cdh_Flags field, it can notify
	; a client task that has installed itself in the handler structure
	; using the cdh_NotifyTask field.
	APTR	cdh_NotifyTask
	ULONG	cdh_NotifySigBit;

	APTR	cdh_UserData;		; for handler use
   LABEL CrossDOSHandler_SIZEOF

; Flag bits for CrossDOSHandler.cdh_Flags
	BITDEF CD,FILTER,0
	BITDEF CD,TRANSLATE,1

;---------------------------------------------------------------------------

; Character translation table
   STRUCTURE CrossDOSTrans,LN_SIZE
	UWORD	cdt_StructSize		; size of this structure
	STRUCT  cdt_AToM,256		; Amiga  -> MS-DOS
	STRUCT	cdt_MToA,256		; MS-DOS -> Amiga
   LABEL CrossDOSTrans_SIZEOF

;---------------------------------------------------------------------------

; Name of the semaphore
CROSSDOSNAME MACRO
	     DC.B '« CrossDOS »',0
	     ENDM

;---------------------------------------------------------------------------

	ENDC	; INTERNAL_CROSSDOS_I
