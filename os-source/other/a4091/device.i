		INCLUDE	"exec/devices.i"
		INCLUDE	"exec/io.i"
	IFD NCR53C710
		INCLUDE "ncr710.i"
	ENDC

;=============================================================================
; For each Address (ID), we need some shared data for all the units.
;=============================================================================
	STRUCTURE HDAddr,0
		STRUCT ha_Units,MLH_SIZE	// units for this address
		UBYTE	ha_ID			// ID for this address
		UBYTE	ha_DoneSync		// 1=sync negotiation done
						;  (set to 0 on device reset!)
		UBYTE	ha_Reselect		// set to $40 if >1 unit exists
		UBYTE	ha_SyncValue		// value we negotiated for sync
		UBYTE	ha_SupportsSync		// This unit supports sync (Inquiry)
		UBYTE	ha_SentSync		// not used by NCR driver
		UBYTE	ha_SyncNegotiated	// value negotiated, not in use
		UBYTE	ha_res			// pad to longword!
	LABEL ha_SIZEOF

;==============================================================================
; My device structure.  All globals are referenced off the device base (a5).
; This driver actually separates itself into two distinct drivers called
; xt.device and scsi.device.  Each of these drivers gets its own library and
; global area but both of them call the same task to do the actual IO calls.
; Individual copies of this global data is used by these two drivers.
;==============================================================================
	STRUCTURE HDDevice,DD_SIZE		longword-misaligned
		UWORD	hd_Type			0=xt.device 1=scsi.device
		APTR	hd_SysLib		exec.library
		APTR	hd_IOPort		IOTasks message port
		STRUCT	hd_HDUnits,ha_SIZEOF*(MAX_ADDR+1) array of ID structs
		APTR	hd_DiagArea		only valid if autoboot time
		APTR	hd_ConfigDev		and we'll need this too
		APTR	hd_STGlobals
		APTR	hd_IOGlobals
		UBYTE	hd_NumUnits		total number of units
	LABEL HDDevice_SIZEOF

;==============================================================================
; For each unit that has been opened there will be one of these structures.
; The LUN stuff will be ignored for XT devices but it's here so that units can
; be handled by the same code for both SCSI and XT drives.  Various parts of
; the unit structure are for exclusive use by IOTask or SCSITask.
;==============================================================================
	STRUCTURE HDUnit,MLN_SIZE
; This section of the structure is for use by any task.  The data are static.
		APTR	hu_Addr			points to shared per-ID data

		UBYTE	hu_Unit			the physical unit number
		UBYTE	hu_LUN			logical unit number for SCSI
		UBYTE	hu_Type			0=XT/AT 1=SCSI
		UBYTE	hu_SCSIType		0=disk 1=tape 2=printer etc.

		UBYTE	hu_SCSIQual		eg $80 = removable media
		UBYTE	hu_MountDone		1 = partitions mounted
		UBYTE	hu_IsDisk		1 = can accept CMD_anything
		UBYTE	hu_Protected		1 = disk write protected

		UBYTE	hu_Flags		flags from RDB
		UBYTE	hu_ATDriveBit		hu_Unit << AT_UNIT_SHIFT

; The following data are for exclusive use by the IOTask that does mapping etc.
; Generally this will apply only to XT units though some SCSI's may use it too.
		UWORD	hu_TotalMapped		number of mapped blocks
		APTR	hu_BadBlockList		list of mapped bad blocks

		ULONG	hu_BlockSize		byte size of one block
		ULONG	hu_DiskSize		in blocks, from ReadCapacity
		ULONG	hu_MaxBlocks		>256 for extended read/writes
		UWORD	hu_BlockShift		shift for divide by blocksize
		APTR	hu_RDB			RigidDiskBlock data

; this data is only actively used by devices that support removeable media
		ULONG	hu_ChangeState		0=disk in, -1=disk out
		ULONG	hu_RemoveAddress	who to Cause()
		STRUCT	hu_ChangeList,MLH_SIZE	for AddChangeInt

; the following data are for exclusive use by the SCSITask that does the work
; NOTE: removing units from the SCSIQueue list will return a pointer to the
; hu_SCSIQueue entry, not the head of the unit struct.  Remember to adjust.
		STRUCT	hu_SCSIQueue,MLN_SIZE	queue linker for SCSITask
		STRUCT	hu_CmdList,MLH_SIZE	command blocks queued on unit
		APTR	hu_CurrentCmd		command block executing now
		UBYTE	hu_QueueType		SCSI queue this unit is on
		UBYTE	hu_WhatNext		where to go when disconnected

		ULONG	hu_TotalCyls		total cylinders on XT drive
		UWORD	hu_TotalBlocks		sectors per track

; this is used by XT task for setting up returns on the next request sense
	IFD XT_OR_AT_IDE
	    IFD IS_IDE
		ULONG	hu_ErrorLBA		where the error happened
		UBYTE	hu_MaxRWSize		max r/w multiple size (blocks)
;						 (0 means not supported)
		UBYTE	hu_Found		this drive exists
		UWORD	hu_Pad2
		STRUCT	hu_ATBuffer,512		512 byte GP buffer
	    ENDC
		ULONG	hu_NextError		for faking XT request sense
		UWORD	hu_CylSize		sectors * heads
		UBYTE	hu_TotalHeads		tracks per cylinder

	ENDC XT_OR_AT_IDE
;
		UBYTE	hu_TagsAllowed	   ; tagged queuing is supported
		UBYTE	hu_NoDisconnect	   ; allow target to disconnect (?)
	IFD NCR53C710
		; MUST BE ON LONGWORD!
		; '710 code that matches LUN nums
		STRUCT	hu_Find_lun,Find_lun_entry_SIZEOF
	ENDC
		UBYTE	hu_UpdateFails	   ; if set don't try Synchronize Cache
		UBYTE	hu_Pad3

;		UBYTE	hu_SyncDone		1=sync negotiation done
;						; (set to 0 on device reset!)
;		UBYTE	hu_SentSync		we just sent a Sync message
;		UBYTE	hu_Reselect		set to $40 if >1 unit exists
;		UBYTE	hu_SyncNegotiated	value we negotiated for sync

	LABEL HDUnit_SIZEOF

;==============================================================================
; This is the definition of an internal command block as used by IOTask and
; passed to SCSITask for queueing on unit structures and passing to hardware.
; The ListNode in the MN struct is used for queuing these command blocks on
; both the free queue and on the pending command queues of individual units.
;
; The following fields are initialised at Command Block allocation time and
; must be restored if they are changed during the handling of an IORequest.
;
;	cb_SCSICmd		the main SCSICmd struct for current IORequest
;	cb_SpareSCSICmd		auxiliary used for error fetching etc.
;	cb_SenseData		a 256 byte buffer
;
; All are guaranteed to be word-aligned.
;
; For the A3090, this structure must start longword-aligned, and be a long-
; word size (so it can be made an array).
;
;==============================================================================
	STRUCTURE CommandBlock,MN_SIZE		used for msg and cmd queues
	  IFD NCR53C710
	; this comes early since it MUST be longword aligned
	; the Head/TailData is only used on '040's, since they like DMA to
	; happen on 16-byte boundaries (because of Copyback cache issues).
		STRUCT  cb_DSA,DSA_entry_SIZEOF	// pointer to NCR IO block
		ULONG	cb_CurrentLength // length of this DMA (gather/scatter)
		ULONG	cb_ExtraData	// ActualSize - expected size
		UWORD	cb_Index	// which DSA block is this (0-254)
		UBYTE	cb_HeadSize	// number of bytes xfered to HeadData
		UBYTE	cb_TailSize	// number of bytes xfered to TailData
		APTR	cb_HeadData	// points to 16-byte aligned in DSA
		APTR	cb_TailData	// points to 16-byte aligned in DSA
	  ENDC
		APTR	cb_IORequest		IORequest we are servicing
		APTR	cb_SCSICmd		the SCSIDirect command block
		APTR	cb_SpareSCSICmd		for Request Sense and mapping
		APTR	cb_UserSCSICmd		saving SCSICmd and user version
		APTR	cb_SenseData		data area for request sense
		APTR	cb_ReturnTo		code to process returning cmds

; these are used by the error handling routine to save the callers states
		APTR	cb_ErrorCaller		who called error handling
		APTR	cb_OldReturnTo		old value when handling errors

; these are working copies of the IORequest field and are used only by IOTask
		APTR	cb_Data			source/destination data
		ULONG	cb_Offset		byte offset to read/write
		ULONG	cb_Length		length left to read/write
		ULONG	cb_Done			amount read/written this time
		UWORD	cb_Retries		number of retries performed

; various scsi messaging stuff
		IFND IS_IDE
		UWORD	cb_MsgOutLen		original length of message
		APTR	cb_MsgOutPtr		pointer to next message out
		STRUCT	cb_MessageOut,8		messages are max of 8 bytes

		APTR	cb_MsgInPtr		pointer to next message in
		UWORD	cb_MsgInLeft		# msg bytes left to receive
		UWORD	cb_MsgInLen		total length of message
		STRUCT	cb_MessageIn,8		messages are max of 8 bytes

		UBYTE	cb_MsgParity		FALSE = no parity errors
		UBYTE	cb_DidReadDMA		flag, did a DMA read op.
		UBYTE	cb_DidCacheStuff	flag, did a CachePreDMA call
		UBYTE	cb_Programmed		flag, only do programmed I/O
		ENDC
		IFD IS_IDE
		UBYTE	cb_ATLength		# of blocks for AT transfer
		UBYTE	cb_Pad
		ENDC
		; LW aligned again

; this is used by the diskchange poller to determine how many units are left
; to be polled.
		ULONG	cb_LastPolled		so we can re-search the list
		ULONG	cb_FakeCommand		-1 means IORequest invalid

; these are used by the software bad block mapping, an offset save and a flag
		ULONG	cb_PrevOffset		offset before mapping was done
		UWORD	cb_MappingDone		1 = mapped last read/write

		UBYTE	cb_StateSave		general purpose flag 
		UBYTE	cb_ErrorCode		returned by SCSITask

; These are the active and saved SCSI pointers used exclusively by the SCSITask
; They're re-initialised each time they're used so there's no need to save them
		APTR	cb_WData		working data pointer
		APTR	cb_WCommand		working command pointer
		APTR	cb_WStatus		working status pointer

		IFND IS_IDE
		APTR	cb_CacheData		address we started with
		ULONG	cb_CacheLength		amount we did

		APTR	cb_OrigCacheData	address we started with
		ULONG	cb_OrigCacheLength	amount we did

		APTR	cb_SData		saved data pointer
		APTR	cb_SCommand		saved command pointer
		APTR	cb_SStatus		saved status pointer
		ENDC

		APTR	cb_LinkedCmd		current SCSICmd (may not be 1st)

		IFD FIXING_READS
		APTR	cb_KlugeData		2 block buffer for kluge reads
		APTR	cb_SaveData		original pointer before kluge
		ENDC
	LABEL cb_SIZEOF

;==============================================================================
; A readhandle that's passed to LoadSeg by the LoadFS routine (in mountstuff)
;==============================================================================
	STRUCTURE ReadHandle,0
		APTR	rh_Globals		global data pointer
		APTR	rh_Unit			the unit we want to read from
		APTR	rh_Data			current pointer (0 at start)
		ULONG	rh_DataLeft		how much is left to read
		LABEL	rh_Buff			data buffer right after struct
	LABEL rh_SIZEOF
