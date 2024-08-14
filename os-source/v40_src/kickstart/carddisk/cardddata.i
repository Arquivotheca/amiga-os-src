**
**	$Id: cardddata.i,v 1.2 91/10/21 18:27:16 darren Exp $
**
**	Credit card device privates
**
**	(C) Copyright 1991 Commodore-Amiga, Inc.
**	    All Rights Reserved
**
**	
**

	INCLUDE	"exec/types.i"
	INCLUDE	"exec/interrupts.i"
	INCLUDE	"exec/semaphores.i"
	INCLUDE	"exec/libraries.i"
	INCLUDE	"exec/nodes.i"
	INCLUDE	"exec/lists.i"

	INCLUDE "libraries/configregs.i"
	INCLUDE "libraries/expansionbase.i"
	INCLUDE "libraries/dos.i"
	INCLUDE "libraries/dosextens.i"
	INCLUDE "libraries/filehandler.i"

	INCLUDE	"devices/trackdisk.i"

	INCLUDE	"resources/tuples.i"
	INCLUDE	"resources/card.i"

CDB_STKSIZE	EQU	512		;can be small (I hope)
	BITDEF	CDD,SIGTASK,31

 STRUCTURE	CardDeviceBase,LIB_SIZE
	UBYTE	cdb_Flags
	UBYTE	cdb_Flags2
	APTR	cdb_ExecLib
	APTR	cdb_CardResource
	APTR	cdb_ExpanLib
	STRUCT	cdb_SSemaphore,SS_SIZE	;single-thread r/w's of data

	; task

	ALIGNLONG

	STRUCT	cdb_Stk,CDB_STKSIZE

	STRUCT	cdb_TC,TC_SIZE

	; unit 0 data

	ALIGNLONG

	LABEL	cdb_CardDiskUnit0
	
	ULONG	cdb_ChangeCount

	; location info

	APTR	cdb_MapCard	;address of common memory
	APTR	cdb_DataStart	;address - start of partition
	APTR	cdb_EDCLOC	;address of error-detection code table
				;or NULL, indicating no table
	APTR	cdb_CRCTable	;might be NULL for no table

	; CardHandle

	STRUCT	cdb_CardHandle,CardHandle_SIZEOF

	STRUCT	cdb_Inserted,IS_SIZE
	STRUCT	cdb_Removed,IS_SIZE
	STRUCT	cdb_Status,IS_SIZE

	; geometry info

	ULONG	cdb_NBLOCKS	;total # of sectors per CISTPL_FORMAT
	ULONG	cdb_BKSZ	;block size [128-2048 -- typically 512]
	ULONG	cdb_CYLINDERS	;total # of cylinders
	ULONG	cdb_SECPERCYL	;sec/track * track/cyl = sec/cyl
	ULONG	cdb_TRKPERCYL	;track/cyl (heads)
	ULONG	cdb_SECPERTRK	;sec/track

	ULONG	cdb_MaxOffset	;max IO_OFFSET
	ULONG	cdb_RemMask	;remainder mask

	; state flags (WP, inserted, removed, etc)

        UBYTE	cdb_DiskUnitFlags

	; error detection flags

	UBYTE	cdb_ErrorDetect	;from CISTPL_FORMAT tuple (method to use)
	UBYTE	cdb_EDCLength	;from CISTPL_FORMAT tuple (# of bytes )

	; some buffer space for CopyTuple(), and a DeviceTData struct
	ALIGNWORD

	STRUCT	cdb_buffer,TP_Format_SIZEOF+8
	STRUCT	cdb_ddata,DeviceTData_SIZEOF

	; notification list

	ALIGNLONG

	APTR	cdb_RemoveInt
	STRUCT	cdb_ChangeInt,MLH_SIZE

	; for MakeDosNode

	LABEL	cdb_MakeDosData

	APTR	cdb_DrivePtr		;ptr to DeviceName (CRD0:)
	APTR	cdb_DevicePtr		;ptr to device name (me)
	ULONG	cdb_UnitNum		;0 by default
 	ULONG	cdb_FirstFlags		;for Open - 0 for now

	STRUCT	cdb_DosEnvec,DosEnvec_SIZEOF

	LABEL	CardDeviceBase_SIZEOF

*------ cdb_DiskUnitFlags --------------------------------------

	BITDEF	CDB,CARDINSERTED,0	;TRUE if a card is inserted
	BITDEF	CDB,WRITEPROTECT,1	;TRUE means write-protect

	BITDEF	CDB,CARDREMOVED,2	;TRUE if interrupt removed
	BITDEF	CDB,NEWCARD,3		;TRUE if a new card is inserted
	BITDEF	CDB,WRCHANGE,4		;TRUE if write-protect status changed
	BITDEF	CDB,WRITEABLE,5		;TRUE if card is SRAM or DRAM
	BITDEF	CDB,MOTORON,6		;TRUE if light/motor is on
	BITDEF	CDB,STARTCC0,7		;TRUE if we've set START_CC0 flag

*------ Other flags ---------------------------------------------

ROMBUILD		EQU	1	;set TRUE for ROM build

*------ Other EQUates -------------------------------------------

FALSE			EQU	0
TRUE			EQU	1

*------ Macros --------------------------------------------------

	; JSR through LVO so we can SetFunction later if needed.
	; Preserves all registers except D0

JSRLVO		MACRO	;resource base,label

		IFND	_LVO\1
		XREF	_LVO\1
		ENDC

		jsr	_LVO\1(a6)
		ENDM


