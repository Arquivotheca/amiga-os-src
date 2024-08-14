	IFND	RESOURCES_FILESYSRES_I
RESOURCES_FILESYSRES_I	SET	1
**
**	$Filename: resources/filesysres.i $
**	$Release: 1.4 $
**	$Revision: 1.1 $
**	$Date: 91/03/22 09:47:06 $
**
**	FileSystem.resource description
**
**	(C) Copyright 1988 Kodiak Software
**

	IFND	EXEC_NODES_I
	INCLUDE	"exec/nodes.i"
	ENDC
	IFND	EXEC_LISTS_I
	INCLUDE	"exec/lists.i"
	ENDC

 STRUCTURE  FileSysResource,LN_SIZE	; on resource list
    CPTR    fsr_Creator			; name of creator of this resource
    STRUCT  fsr_FileSysEntries,LH_SIZE	; list of FileSysEntry structs
    LABEL   FileSysResource_SIZEOF

 STRUCTURE  FileSysEntry,LN_SIZE	; on fsr_FileSysEntries list
					; LN_NAME is of creator of this entry
    ULONG   fse_DosType		; DosType of this FileSys
    ULONG   fse_Version		; Version of this FileSys
    ULONG   fse_PatchFlags	; bits set for those of the following that need
				;   to be substituted into a standard device
				;   node for this file system: e.g. $180
				;   for substitute SegList & GlobalVec
    ULONG   fse_Type		; device node type: zero
    CPTR    fse_Task		; standard dos "task" field
    ULONG    fse_Lock		; not used for devices: zero
    ULONG    fse_Handler		; filename to loadseg (if SegList is null)
    ULONG   fse_StackSize	; stacksize to use when starting task
    LONG    fse_Priority	; task priority when starting task
    ULONG    fse_Startup		; startup msg: FileSysStartupMsg for disks
    ULONG    fse_SegList		; code to run to start new task
    ULONG    fse_GlobalVec	; BCPL global vector when starting task
    ; no more entries need exist than those implied by fse_PatchFlags
    LABEL   fse_SIZEOF

	ENDC	; RESOURCES_FILESYSRES_I
