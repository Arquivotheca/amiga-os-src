**
**	$Id: config.asm,v 36.12 92/05/19 16:47:51 darren Exp $
**
**	configure boards asking for CONFIG w/ a valid diag area.
**
**	(C) Copyright 1987,1988 Commodore-Amiga, Inc.
**	    All Rights Reserved
**
	SECTION	strap

**	Includes

	INCLUDE	"strap.i"

	INCLUDE	"exec/nodes.i"
	INCLUDE	"exec/lists.i"
	INCLUDE	"exec/ports.i"
	INCLUDE	"exec/tasks.i"
	INCLUDE	"exec/libraries.i"
	INCLUDE	"exec/memory.i"
	INCLUDE	"exec/execbase.i"
	INCLUDE	"exec/resident.i"
	INCLUDE	"exec/alerts.i"

	INCLUDE	"libraries/expansionbase.i"

	IFND	eb_BoardList
eb_BoardList	EQU	eb_Private05
	ENDC

	INCLUDE	"libraries/configvars.i"

	INCLUDE	"devices/trackdisk.i"
	INCLUDE	"resources/disk.i"
	INCLUDE "internal/librarytags.i"


**	Exports

	XDEF	RMInit


**	Imports

	XLVO	AddHead			; Exec
	XLVO	AllocMem		;
	XLVO	CloseLibrary		;
	XLVO	Enqueue			;
	XLVO	InitResident		;
	XLVO	OpenLibrary		;
	XLVO	OpenResource		;
	XLVO	TaggedOpenLibrary	;

	XLVO	ObtainConfigBinding	; Expansion
	XLVO	ReleaseConfigBinding	;
	XLVO	SetCurrentBinding	;

	XREF	SMAlert


**	Locals

 STRUCTURE	StrapFloppyNode,0
	STRUCT	sfn_DevNode,DeviceNode_SIZEOF
	STRUCT	sfn_FSSM,FileSysStartupMsg_SIZEOF
	STRUCT	sfn_Envec,(DE_BOOTBLOCKS+1)*4
	STRUCT	sfn_DriveName,5		; 3,'DF0',0
	ALIGNLONG
	STRUCT	sfn_DevName,18		; 17,'trackdisk.device',0
	ALIGNWORD
	STRUCT	sfn_BootNode,BootNode_SIZEOF
	LABEL	StrapFloppyNode_SIZEOF


**	Assumptions

	IFNE	CurrentBinding_SIZEOF-16
	FAIL	"consider using newer CurrentBinding structure"
	ENDC
	IFNE	DAC_CONFIGTIME-$10
	FAIL	"DAC_CONFIGTIME not bit 4, recode"
	ENDC
	IFNE	da_Config
	FAIL	"da_Config not zero, recode"
	ENDC
	IFNE	cb_ConfigDev
	FAIL	"cb_ConfigDev not zero, recode"
	ENDC
	IFNE	DRT_AMIGA
	FAIL	"DRT_AMIGA not zero, recode"
	ENDC
	IFNE	sfn_DevNode
	FAIL	"sfn_DevNode not zero, recode"
	ENDC


**********************************************************************
RMInit:
		movem.l	d2/a2-a6,-(a7)

		;-- initialize CurrentBinding local variable invariants
		moveq	#0,d0
		move.l	d0,-(a7)	; cb_ToolTypes
		move.l	d0,-(a7)	; cb_ProductString
		move.l	d0,-(a7)	; cb_FileName
		subq.l	#4,a7		; cb_ConfigDev

		;-- get expansion library

		moveq	#OLTAG_EXPANSION,d0
		CALLLVO	TaggedOpenLibrary
		tst.l	d0
		bne.s	rExpLibOK
		move.l	#AN_BootStrap!AG_OpenLib!AO_ExpansionLib,d0
		bsr	SMAlert
		bra	rError1

rExpLibOK:
		move.l	d0,a5		; save ExpansionBase
		exg	a5,a6		; use ExpansionBase, save SysBase

		;-- obtain expansion config binding semaphore
		CALLLVO	ObtainConfigBinding

		;-- scan expansion board list for boards to config
		move.l	eb_BoardList(a6),a2

rScanEBList:
		move.l	(a2),d2		; check for end of board list
		beq.s	rScanEBDone

		;--	check if this board is to be configured
		btst.b	#CDB_CONFIGME,cd_Flags(a2)
		beq.s	rScanEBNext

		btst.b	#ERTB_DIAGVALID,cd_Rom+er_Type(a2)
		beq.s	rScanEBNext

		move.l	cd_Rom+er_Reserved0c(a2),d0
		beq.s	rScanEBNext

		move.l	d0,a3
		btst.b	#4,(a3)		; DAC_CONFIGTIME,da_Config(a3)
		beq.s	rScanEBNext

		;--	find the resident tag in the diagnostic area
		move.w	da_Size(a3),d0	; words to search
		lsr.w	#1,d0
		subq.w	#1,d0
rResTagScan:
		cmp.w	#RTC_MATCHWORD,(a3)+
rResTagScanDBF:
		dbeq	d0,rResTagScan
		bne.s	rScanEBNext	; no resident tag

		lea	-2(a3),a0
		cmp.l	(a3),a0
		bne.s	rResTagScanDBF

		;--	set the cb_ConfigDev field in the config binding
		move.l	a7,a0

		move.l	a2,(a0)		; move.l a2,cb_ConfigDev(a0)

		moveq	#CurrentBinding_SIZEOF,d0
		CALLLVO	SetCurrentBinding

		;--	configure the board
		moveq	#0,d1		; NULL seglist
		lea	-2(a3),a1
		exg	a5,a6		; use SysBase, save ExpansionBase
		CALLLVO	InitResident
		exg	a5,a6		; use ExpansionBase, save SysBase

rScanEBNext:
		move.l	d2,a2
		bra.s	rScanEBList

rScanEBDone:
		;-- release expansion config binding semaphore
		CALLLVO	ReleaseConfigBinding


		;-- add nodes for floppies
		exg	a5,a6		; use SysBase, save ExpansionBase

		;-- get disk resource
		lea	DRName(pc),a1
		CALLLVO	OpenResource
		tst.l	d0
		bne.s	rDiskResOK
		move.l	#AN_BootStrap!AG_OpenRes!AO_DiskRsrc,d0
		bsr	SMAlert
		bra	rError2

rDiskResOK:
		;-- cycle thru all floppies
		moveq	#NUMUNITS,d2
		move.l	a6,a4		; save SysBase
		move.l	d0,a6		; use DiskBase
		bra	rGetUnitIDDBF
rGetUnitIDLoop:
		;--	check if unit is 3.5" drive
		move.l	d2,d0
		jsr	DR_GETUNITID(a6)

		
		cmp.l	#DRT_150RPM,d0
		beq.s	rDriveOk
		tst.l	d0		; cmp.l #DRT_AMIGA,d0
		bne.s	rGetUnitIDDBF
rDriveOk:

		;--	create eb_MountList entry for 3.5" drive
		exg	a4,a6		; use SysBase, save DiskBase
		move.l	#StrapFloppyNode_SIZEOF,d0
		moveq	#MEMF_PUBLIC,d1
		CALLLVO	AllocMem
		tst.l	d0
		beq.s	rAllocMemError

		move.l	d0,a3

		;------ initialize the mount node
		lea	rSFNTemplate(pc),a0
		move.l	a3,a1				; sfn_DevNode(a3) ea
		moveq	#StrapFloppyNode_SIZEOF/2-1,d0
rCopyTemplate:
		move.w	(a0)+,(a1)+
		dbf	d0,rCopyTemplate

		;------ patch BPTRs
		move.l	a3,d0		; get base BPTR
		lsr.l	#2,d0		;
		move.l	d0,a0		;

		lea	sfn_FSSM/4(a0),a1
		move.l	a1,sfn_DevNode+dn_Startup(a3)
		lea	sfn_DriveName/4(a0),a1
		move.l	a1,sfn_DevNode+dn_Name(a3)
		lea	sfn_DevName/4(a0),a1
		move.l	a1,sfn_FSSM+fssm_Device(a3)
		lea	sfn_Envec/4(a0),a1
		move.l	a1,sfn_FSSM+fssm_Environ(a3)

		;--	patch APTR
		move.l	a3,sfn_BootNode+bn_DeviceNode(a3) ; sfn_DevNode(a3) ea

		;--	patch unit number
		move.w	d2,sfn_FSSM+fssm_Unit+2(a3)
		move.w	d2,d0
		add.b	#'0',d0
		move.b	d0,sfn_DriveName+3(a3)

		;--	patch unit priority
		move.b	rFloppyPriorities(pc,d2.w),d0
		move.b	d0,sfn_BootNode+LN_PRI(a3)
		ext.w	d0
		ext.l	d0
		move.l	d0,sfn_Envec+de_BootPri(a3)

		;--	put this boot node on the eb_MountList
		lea	eb_MountList(a5),a0
		lea	sfn_BootNode(a3),a1
		CALLLVO	Enqueue

rAllocMemError:
		exg	a4,a6		; use DiskBase, save SysBase

rGetUnitIDDBF:
		dbf	d2,rGetUnitIDLoop


rError2:
		;------ release expansion library
		move.l	a5,a1		; ExpansionBase
		move.l	a4,a6		; SysBase
		CALLLVO	CloseLibrary

rError1:
		;------	restore stack, registers
		add.w	#CurrentBinding_SIZEOF,a7
		movem.l	(a7)+,d2/a2-a6
		rts


rFloppyPriorities:
		dc.b	5,-10,-20,-30	; for df0:-df3:

rSFNTemplate:
		dc.l	0,0,0,0		; dn_Next, dn_Type, dn_Task, dn_Lock
		dc.l	0,600,10	; dn_Handler, dn_StackSize, dn_Priority 
		dc.l	0,0,0		; dn_Startup, dn_SegList, dn_GlobalVec
		dc.l	0		; dn_Name
		dc.l	0,0,0		; fssm_Unit, fssm_Device, fssm_Environ
		dc.l	0		; fssm_Flags
		dc.l	19,128,0	; de_TableSize, de_SizeBlock, de_SecOrg
		dc.l	2,1		; de_Surfaces, de_SectorPerBlock
		dc.l	11,2		; de_BlocksPerTrack, de_Reserved
		dc.l	0,0,0		; de_PreAlloc, de_Interleave, de_LowCyl
		dc.l	79,5		; de_HighCyl, de_NumBuffers
		dc.l	MEMF_PUBLIC	; de_BufMemType
		dc.l	$200000		; de_MaxTransfer
		dc.l	$7ffffffe	; de_AddressMask
		dc.l	0		; de_BootPri
		dc.b	'DOS',0		; de_DosType
		dc.l	0,0		; de_Baud, de_Control
		dc.l	2		; de_BootBlocks
		dc.b	3,'DF0',0,0,0,0	; (dn_Name)
		dc.b	17,'trackdisk.device',0
					; (fssm_Device)
		dc.l	0,0		; ln_Succ, ln_Pred
		dc.b	NT_BOOTNODE,0	; ln_Type, ln_Pri
		dc.l	0		; ln_Name
		dc.w	0		; bn_Flags
		dc.l	0		; bn_DeviceNode
rsfntEnd:

	IFNE	(rsfntEnd-rSFNTemplate)-StrapFloppyNode_SIZEOF
	FAIL	'rSFNTemplate size not StrapFloppyNode_SIZEOF'
	ENDC

DRName		dc.b	'disk.resource',0
		ds.w	0

	END
