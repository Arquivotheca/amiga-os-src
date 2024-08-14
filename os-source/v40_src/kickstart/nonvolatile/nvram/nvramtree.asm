******************************************************************************
*
*	$Id: nvramtree.asm,v
*
******************************************************************************
*
*	$Log:	nvramtree.asm,v $
* Revision 40.22  93/09/14  10:36:40  darren
* Revision 40.21 introduced a bug; the semaphore is now obtained
* multiple times, but not released an equal number of times.  Also
* a bug is present if NVRT_Command_Init did not obtain the
* memory for nvram copy ... NVRT_Command_Cleanup does not
* call ReleaseSemaphore in this case, even though ObtainSemaphore
* was called regardless of the memory allocation success.
* 
* Revision 40.21  93/05/06  15:49:51  brummer
* Move test of multicommand bit to after Semaphore is obtained so noone
* can sneak in while GetNVList() is going on.
* 
* Revision 40.20  93/05/06  14:09:14  brummer
* Add SetNVRAMMultiCommand and ClrNVRAMMultiCommand functions
*
* Revision 40.19  93/05/03  16:49:03  brummer
* Cosmetic fix for RCS screw up in date/time.
*
* Revision 40.18  93/05/03  15:50:46  brummer
* Fix to speed up setprotection function.  THis fix writes only the single
* lock byte for the item instead of writing all NVRAM.
*
* Revision 40.17  93/05/03  15:40:44  brummer
* Fix storing items that have App/Item names greater than 31 characters.
*
* Revision 40.16  93/04/21  12:15:12  brummer
* Same as previous rev except i put in bgt instead of bge in shrink
* routine.
*
* Revision 40.15  93/04/21  10:31:56  brummer
* Fix NVRT_Shrink_Data_Area routine to not scan for End tag but rather
* find it by traversing tree first and use pointer to end data move
* loop.  This fixes the CATS bug of deleteing first entry in App and
* the rest of the Items disappear.
*
* Revision 40.14  93/03/31  16:29:40  brummer
* Change to GetNVItemSize() so that if item exists on disk, the item
* in NVRAM is ignored (info not updated).
*
* Revision 40.13  93/03/23  13:13:39  brummer
* Add conditional assembly instructions for DISK based version.
* /
*
* Revision 40.12  93/03/18  10:36:07  brummer
* Fix SizeNVList to add in a NULL for each Item name found.  This is
* in addition to string length and NVEntry size.
*
* Revision 40.11  93/03/10  17:23:21  brummer
* Fix for returning only 16 bytes reserved data.  This prevents
* read/writes to NVRAM tag in first 2 long words of NVRAM.
*
* Revision 40.10  93/03/09  14:07:27  brummer
* Add NVRAMtree recovery routine.
* Make misc changes suggested by Martin.
*
* Revision 40.9  93/03/05  11:23:00  brummer
* Add check for Arizona gate array before accessing the NVRAM device.
* Add fix to Freemem call that caused mungwall hit.
*
* Revision 40.8  93/03/04  15:16:26  brummer
* Modify code to use two NULL strings for reserved data access.
* Fix bug in access reserved data.
*
* Revision 40.7  93/03/03  10:47:00  brummer
* Remove comments on obtain/release semaphore.  RAMKICK problem was
* that InitNVRAM was being called at library init time and semaphore
* initialization was happening at library open time.  See associated
* fix in start.asm rev 40.7
*
* Revision 40.6  93/03/02  23:14:11  brummer
* Modified code to allocate RAM and read NVRAM device for each command.
* Modified code to do full tree structure verification on Init.
* Modified code to use Exec semaphore for RAM buffer access.  However,
* the Obtain and Release is temp commented out to allow for RAMKICKing
* the library.
*
* Revision 40.5  93/03/02  10:05:34  brummer
* Fix out of range branch in previous rev.!!???!
*
* Revision 40.4  93/03/02  09:34:54  brummer
* Fixed and tested version for automatic LRU deletion.
*
* Revision 40.3  93/03/01  15:04:49  brummer
* Fix string compares to use utility.library string case insensitive
* string compare function.  This allows NVRAM to store cased App and
* Item names, however searches for read/write/delete are case
* insensitive.
*
* Revision 40.2  93/02/26  17:16:51  brummer
* change the value returned in GetNVRAMInfo from total to total-rsvd.
* change far branches to short where possible
*
* Revision 40.1  93/02/25  19:22:23  brummer
* fix sizeNVRAMlist to not add NULL to total count for each node.
* fix SizeNVRAMList to use a3 as Item and a4 as App pointers
* fix GetNVRAMInfo to use d0 as ptr to Memblock instead of a0
*
* Revision 40.0  93/02/25  12:04:00  brummer
* fix revision to 40.0
*
* Revision 39.2  93/02/25  11:31:53  brummer
* functional version of NVRAM tree/data structure handler
* Tested for : init,string functions, read/write using free space, deletes, protection
* Still required : testing for LRU deletion, use of EXEC semiphoe on writes/deletes,
*  allocation and read of device on every command, verification of entire tree on init, recovery of data on invalid tree.
*
* Revision 39.1  93/02/04  11:17:43  brummer
* code shell
*
*
*
*	(C) Copyright 1993 Commodore-Amiga, Inc.
*	    All Rights Reserved
*
******************************************************************************

	NOLIST
	INCLUDE	"dos/dos.i"
        INCLUDE "exec/ables.i"
        INCLUDE	"exec/interrupts.i"
	INCLUDE	"exec/io.i"
        INCLUDE	"exec/lists.i"
	INCLUDE	"exec/macros.i"
	INCLUDE	"exec/memory.i"
        INCLUDE	"exec/nodes.i"
	INCLUDE "exec/strings.i"
        INCLUDE "exec/types.i"
	INCLUDE "nvram.i"
	LIST
	INCLUDE	"nvramtree.i"
	INCLUDE	"/nonvolatile.i"
	INCLUDE	"/nonvolatilebase.i"

	XREF	nvram_read_one
	XREF	nvram_write_one
	XREF	nvram_read_mult
	XREF	nvram_write_mult

	SECTION	Code


*****i* nonvolatile.library/NVRAM/#? ****************************************
*
*  This file contains routines responsible for mid level access to the
*  serial access Nonvolatile RAM chip.  The low level access to Nonvolatile
*  RAM is through the routines in the file nvram.asm.
*
*  This module is conditionally assembled to generate a ROM or a DISK based
*  version of the nonvolatile.library.
*
*  Data is stored and retreived in the NVRAM by associating data to an App
*  (application) and an Item string.  There are no pointers used in storing
*  data in NVRAM.  The App/Item/Data elements are in contiguious memory locn.
*
*  The NVRAM data is stored as follows :
*	- There is a tag associated with each element (App, Item, Data) that
*	  contains the type,length, and LRU of the element (see nvramtree.i).
*	- Every App always has at least one Item element.
*	- Every Item has one and only one Data element.
*	- Writes will cause automatic deletion of the least recently used Item
*	  in order to acquired enough space.
*
*  The following mid level routines are available :
*
*  Assembly entry points :
*
	XDEF	InitNVRAM
	XDEF	ReleaseNVRAM
	XDEF	GetNVRAMInfo
	XDEF	SizeNVRAMList
	XDEF	GetNamesFromNVRAM
	XDEF	SizeItemNVRAM
	XDEF	SetNVRAMProtection
	XDEF	ReadNVRAM
	XDEF	WriteNVRAM
	XDEF	DeleteNVRAM
	XDEF	SetNVRAMMultiCommand
	XDEF	ClrNVRAMMultiCommand
*
*  C entry points :
*
	XDEF	_InitNVRAM
	XDEF	_ReleaseNVRAM
	XDEF	_GetNVRAMInfo
	XDEF	_SizeNVRAMList
	XDEF	_GetNamesFromNVRAM
	XDEF	_SizeItemNVRAM
	XDEF	_SetNVRAMProtection
	XDEF	_ReadNVRAM
	XDEF	_WriteNVRAM
	XDEF	_DeleteNVRAM
	XDEF	_SetNVRAMMultiCommand
	XDEF	_ClrNVRAMMultiCommand
*
*  Note :	Access to the NVRAM port address may be harmfull
*		on some Amiga platforms.  This includes CDTV, and A4000.
*		Code should be added for the RAM library version to detect
*		these platforms and return not available to NVRAM access.
*
******************************************************************************





*****i* nonvolatile.library/InitNVRAM ****************************************
*
*   NAME
*	InitNVRAM - Initialization for NVRAM access functions.
*
*   SYNOPSIS
*	InitNVRAM (NVBase)
*		   a5
*
*	InitNVRAM (APTR)
*
*   FUNCTION
*	Allocates any memory required for NVRAM access and inits any required
*	structures for internal processing.  The pointer to allocated NVRAMData
*	will be moved to Nonvolatile library base.
*
*	The following actions are taken by this rouitne :
*
*	- If the DISK based assembler option is TRUE :
*		- Return
*	- If the DISK based assembler option is FALSE :
*		- Call common command init routine to move the NVRAM
*		  contents to RAM and verify the tag.
*			- If not valid, init RAM copy of NVRAM and write to device.
*		- Verify the structure of the NVRAM contents :
*			- If not valid, perform tree recovery (write device).
*		- Return
*
*   INPUTS
*	NVBase		Pointer to Nonvolatile library base
*
*   RESULT
*	None
*
******************************************************************************
_InitNVRAM:
InitNVRAM:

 IFGT	DISKVERSION

	rts					;

 ELSE

	movem.l	a0-a6/d0-d7,-(sp)		; save registers
;
; Perform generic command initialization :
;
	bsr	NVRT_Command_Init		; get semaphore/allocate buffer
	bne.s	20$				; if no, j to return
	bsr	NVRT_Verify_NVRAM_Tag		; is data area OK ?
	bne.s	4$				; if no, j to init
;
; Perform complete tree verification :
;
	bsr	NVRT_Verify_NVRAM_Tree		; is tree structure OK ?
	beq.s	20$				; if yes, j to return
	bsr	NVRT_Tree_Recovery		; recover data, write device
	bra.s	20$				; j to return
;
; Initialize NVRAM copy :
;
4$:	move.l	nv_NVRAMCopy(a5),a0		; a0 gets ptr to base
	move.l	a0,a1				; save base in a1
	moveq.l	#0,d0				; clear d0
	move.l	#(nvram_device_size/4)-1,d1	; d1 gets proper loop count
6$:	move.l	d0,(a0)+			; write zero to RAM copy area
	dbra.s	d1,6$				; loop for all long words
	move.l	#NVRT_Init_Tag,(a1)			; write init key to copy area
	move.b	#NVRT_End_Tag,NVRT_FIRST_APP_LOCN(a1)	; write end marker to copy area
;
; Write copy out to NVRAM device :
;
	bsr	NVRT_Device_Write		; update device w/inited buffer
;
; Perform generic command cleanup and return :
;
20$:	bsr	NVRT_Command_Cleanup		; free buffer/release semaphore
	movem.l	(sp)+,a0-a6/d0-d7		; restore registers
	rts					; return to caller

 ENDC



*****i* nonvolatile.library/GetNVRAMInfo *************************************
*
*   NAME
*	GetNVRAMInfo - Reports information on the NVRAM
*
*   SYNOPSIS
*	GetNVRAMInfo (MemBlock, NVBase)
*		      d0	a5
*
*	VOID GetNVRAMInfo(APTR, APTR)
*
*   FUNCTION
*	Returns information about the current NVRAM chip.
*
*	Currently this information consists of two long words.  The first
*	is the byte size of the NVRAM chip - the amount reserved for system.
*	The second is the number of bytes available in the NVRAM chip.
*
*	Note:	Since the NVRAM allows for automatic deletion on an LRU
*		basis, the number of bytes available is as follows :
*		AVAIL = (SIZE OF NVRAM - RESERVED BYTES - LOCKED BYTES)
*
*   INPUTS
*	MemBlock	Pointer to first of two long word for NVRAM info.
*	NVBase		Nonvolatile library base
*
*   RESULT
*	MemBlock	Two long words at this location are updated.
*
******************************************************************************
_GetNVRAMInfo:
GetNVRAMInfo:

 IFGT	DISKVERSION

	move.l	a0,-(sp)			; save register
	move.l	d0,a0				; a0 gets ptr to memblock
	clr.l	(a0)				; nvram size gets 0
	clr.l	4(a0)				; nvram avail gets 0
	move.l	(sp)+,a0			; restore register
	rts					;

 ELSE

	movem.l	a0-a6/d0-d7,-(sp)		; save registers
;
; Init registers for this command :
;
	move.l	d0,a2				; a2 gets memblock ptr
;
; Perform generic command initialization :
;
	bsr	NVRT_Command_Init		; get semaphore/allocate buffer
	bne.s	2$				; if no, j to return NFG info
	bsr	NVRT_Verify_NVRAM_Tag		; is data area OK ?
	beq.s	4$				; if yes, j to return sizes info
;
; RAM data area is not valid :
;	- write zeros to memblock
;	- j to return
;
2$:	clr.l	(a2)				; nvram size gets 0
	clr.l	4(a2)				; nvram avail gets 0
	bra.s	20$				; j to return to caller
;
; RAM data area is valid :
;	- write device size-rsvd to memblock
;	- call routine to calc avail space and write to memblock
;	- fall thru to return
;
4$:	move.l	#nvram_device_size-NVRT_RSVD_DATA_SIZE,(a2)	; write size to mem block
	bsr	NVRT_Calc_Avail_Space		; d0 gets avail space
	subq.l	#NVRT_Full_Tag_Size,d0		; reduce by overhead
	move.l	d0,4(a2)			; write value to mem block
;
; Perform generic command cleanup and return :
;
20$:	bsr	NVRT_Command_Cleanup		; free buffer/release semaphore
	movem.l	(sp)+,a0-a6/d0-d7		; restore registers
	rts					; return to caller

 ENDC



*****i* nonvolatile.library/ReleaseNVRAM *************************************
*
*   NAME
*	ReleaseNVRAM - Shutdown function for NVRAM access functions.
*
*   SYNOPSIS
*	ReleaseNVRAM (NVBase)
*		      a5
*
*	VOID ReleaseNVRAM (APTR)
*
*   FUNCTION
*	Frees whatever memory allocated by InitNVRAM function.
*
*   INPUTS
*	NVBase		Pointer to Nonvolatile library base
*
*   RESULT
*	None
*
******************************************************************************
_ReleaseNVRAM:
ReleaseNVRAM:

	rts					; return to caller



*****i* nonvolatile.library/SetNVRAMMultiCommand *************************************
*
*   NAME
*	SetNVRAMMultiCommand - Set multi-command flag in NV library base to
*	allow several NVRAM function calls to be made without reallocating
*	memory for the NVRAM copy.  This is used only in the GetNVList()
*	function to speed access to all the App and Items.
*
*   SYNOPSIS
*	SetNVRAMMultiCommand (NVBase)
*		      		a5
*
*	VOID SetNVRAMMultiCommand (APTR)
*
*   FUNCTION
*	This function will call the command init routine that will acquire the
*	NVRAM semaphore, allocate a RAM buffer and copy in the NVRAM data.  It
*	will then set the multi-command flag so that individual function will
*	not repeatedly allocate and release NVRAM copies.
*
*   INPUTS
*	NVBase		Pointer to Nonvolatile library base
*
*   RESULT
*	None
*
******************************************************************************
_SetNVRAMMultiCommand:
SetNVRAMMultiCommand:

 IFGT	DISKVERSION

	rts						; ## return

 ELSE

	bclr.b	#nv_FlagsB_MultiCommand,nv_Flags(a5)	; ## clear flag to allow init to work
	bsr	NVRT_Command_Init			; ## get semaphore/allocate buffer
	bne.s	20$					; ## if error, j to skip flag set
	bset.b	#nv_FlagsB_MultiCommand,nv_Flags(a5)	; ## set multi-command flag
20$:	rts						; ## return to caller

 ENDC



*****i* nonvolatile.library/ClrNVRAMMultiCommand *************************************
*
*   NAME
*	ClrNVRAMMultiCommand - Clear multi-command flag in NV library base.
*	Refer to SetNVRAMMultiCommand for details.
*
*   SYNOPSIS
*	ClrNVRAMMultiCommand (NVBase)
*		      		a5
*
*	VOID SetNVRAMMultiCommand (APTR)
*
*   FUNCTION
*	This function will call the command cleanup routine to free the allocated
*	NVRAM copy area and release the NVRAM semaphore.  It will then clear the
*	multi command flag.
*
*   INPUTS
*	NVBase		Pointer to Nonvolatile library base
*
*   RESULT
*	None
*
******************************************************************************
_ClrNVRAMMultiCommand:
ClrNVRAMMultiCommand:

 IFGT	DISKVERSION

	rts						; ## return

 ELSE

	bclr.b	#nv_FlagsB_MultiCommand,nv_Flags(a5)	; ## clear flag to allow cleanup to work
	bsr	NVRT_Command_Cleanup			; ## release semaphore/free buffer
	rts						; ## return to caller

 ENDC



*****i* nonvolatile.library/SizeNVRAMList ************************************
*
*   NAME
*	SizeNVRAMList - Bytes required for a list of Disk items/applications.
*
*   SYNOPSIS
*	Size = SizeNVRAMList (AppName, NVBase)
*	d0		     a0	      a5
*
*	LONG SizeNVRAMList (char *, struct NVBase *)
*
*   FUNCTION
*	Determines how large a block of memory is required to store an
*	EXEC list (as defined in Nonvolatile.i) containing the items currently
*	stored in NVRAM.  If the AppName parameter is NULL, the size returned
*	is that reqired to build a list for all App and Item names in the
*	NVRAM.  If the AppName is not NULL, then the size returned is that
*	required to store all the Items associated to the request App.
*
*   INPUTS
*	AppName		Pointer to the application name string or NULL if
*			size of strings for all of NVRAM is to be requested.
*
*   RESULT
*	Size		Number of bytes required to save the list.
*
*   SEE ALSO
*	SizeDiskList()
*
******************************************************************************
_SizeNVRAMList:
SizeNVRAMList:

 IFGT	DISKVERSION

	moveq.l	#0,d0				; clear size
	rts					; return

 ELSE

	movem.l	a0-a6/d1-d7,-(sp)		; save registers
;
; Init registers for this command :
;
	moveq.l	#0,d2				; d2 gets temp accumulated length
	move.l	a0,a3				; a3 gets ptr to App name
;
; Perform generic command initialization :
;
	bsr	NVRT_Command_Init		; get semaphore/allocate buffer
	bne.s	20$				; if no, j to return
	bsr	NVRT_Verify_NVRAM_Tag		; is data area OK ?
	bne.s	20$				; if no, j to return 0
;
; Determine if this is single App or all Apps :
;
	move.l	a3,d3				; is the App name NULL ?
	beq.s	4$				; if yes, j to hadle all APPs
;
; Single App name requested, find App name match :
;
	bsr	NVRT_Find_App_Match		; a0 gets offset of App tag
	bne.s	20$				; if not found, j to return
2$:	bsr	NVRT_Find_Next_Item_Tag		; find next item tag
	bne.s	20$				; if not found, j to return
	add.w	#NVENTRY_SIZE+1,d2		; add node size + string NULL to total
	add.w	d0,d2				; add Item string length to total
	bra.s	2$
;
; All App lengths requested :
;
4$:	bsr	NVRT_Read_Tag			; read the first tag
	bra.s	8$				; j to decode
;
; Loop thru all tags and update accumulated string size :
;
6$:	bsr	NVRT_Incr_To_Next_Tag		; d0 get len, d1 gets type
8$:	cmpi.b	#NVRT_App_Tag,d1		; is it App tag ?
	beq.s	10$				; if yes, j to continue
	cmpi.b	#NVRT_Item_Tag,d1		; is it Item tag ?
	beq.s	10$				; if yes, j to continue
	cmpi.b	#NVRT_Data_Tag,d1		; is it Item tag ?
	beq.s	6$				; if yes, j to continue
	cmpi.b	#NVRT_End_Tag,d1		; is it End tag ?
	beq.s	20$				; if yes, j to return
	bra.s	86$				; j to continue
;
; App or Item tag found :
;
10$:	add.w	#NVENTRY_SIZE+1,d2		; add node size + string NULL to total
	add.w	d0,d2				; add string length to total
	bra.s	6$				; j to continue
;
; End tag found, return to caller with length in d0 :
;
20$:	move.l	d2,d0				; d0 gets return parameter
;
; Perform generic command cleanup and return :
;
22$:	bsr	NVRT_Command_Cleanup		; free buffer/release semaphore
	movem.l	(sp)+,a0-a6/d1-d7		; restore registers
	rts					; return to caller
;
; Error found in tree structure :
;
86$:	bsr	NVRT_Tree_Recovery		; attempt to recover data
	bra.s	22$				; j to return

 ENDC



*****i* nonvolatile.library/GetNamesFromNVRAM ********************************
*
*   NAME
*	GetNamesFromNVRAM - Creates a list of Names from disk.
*
*   SYNOPSIS
*	FreeByte = GetNamesFromNVRAM (MemBlock, NVBase, AppName)
*	a2			     a2	       a5      a4
*
*	char * GetNamesFromNVRAM (char *, struct NVBase *, char *)
*
*   FUNCTION
*	Reads the users nonvolatile device and creates a NULL terminated
*	list as described below.  If AppName is not NULL, the list
*	should contain all Items associated to that App.  If AppName is NULL,
*	the list contains all Apps in the NVRAM.
*
*	List structure to be returned :
*
*	If AppName is a NULL :
*   		App1<NULL>App2<NULL>App3<NULL><NULL>
*
*	If AppName is not NULL :
*		Item1<NULL>Item2<NULL><NULL>
*
*	Note:  The resulting pointer is return in a2.
*
*   INPUTS
*	MemBlock	Pointer to memory block used for storing Names
*	NVBase		Pointer the base of nonvolatile.library.
*	AppName		Pointer to application name string or NULL if
*			string for all Apps is requested.
*
*   RESULT
*	FreeByte	Pointer to beyond the terminating NULL of AppNames.
*
*   SEE ALSO
*	GetNamesFromDisk()
*
******************************************************************************
_GetNamesFromNVRAM:
GetNamesFromNVRAM:

 IFGT	DISKVERSION

	clr.b	(a2)+				; write NULL for entire list
	rts					;

 ELSE

	movem.l	a3-a6/d2-d7,-(sp)		; save registers
;
; Init registers for this command :
;
	move.l	a4,a3				; a3 gets ptr to App name
	move.l	a2,d6				; d6 gets pointer to Memblock
;
; Perform generic command initialization :
;
	bsr	NVRT_Command_Init		; get semaphore/allocate buffer
	bne.s	20$				; if no, j to return
	bsr	NVRT_Verify_NVRAM_Tag		; is data area OK ?
	bne.s	20$				; if no, j to return 0
;
; Determine if this is single App or all Apps :
;
	move.l	a3,d3				; is the App name NULL ?
	beq.s	4$				; if yes, j to hadle all APPs
;
; Single App name requested, find App name match :
;
	bsr	NVRT_Find_App_Match		; a0 gets offset of App tag
	beq.s	2$				; if not found, j to return
	move.l	d6,a2				; restore ptr to Memblock
	bra.s	22$				; j to exit
;
; App match found, move Item strings to Memblock
;
2$:	bsr	NVRT_Find_Next_Item_Tag		; find next item tag
	bne.s	20$				; if not found, j to return
	move.w	#NVRT_Item_Tag_Size,d1		; d1 gets tag size
	bsr.s	30$				; j to move string to buffer
	bra.s	2$				; loop for all items
;
; All App names requested :
;
4$:	bsr	NVRT_Read_Tag			; read the first tag
	bra.s	8$				; j to decode
;
; Loop thru all items adding up locked space :
;
6$:	bsr	NVRT_Incr_To_Next_Tag		; read next tag
8$:	cmpi.b	#NVRT_App_Tag,d1		; is it App tag ?
	beq.s	10$				; if yes, j to handle
	cmpi.b	#NVRT_Item_Tag,d1		; is it Item tag ?
	beq.s	6$				; if yes, j to continue
	cmpi.b	#NVRT_Data_Tag,d1		; is it Data tag ?
	beq.s	6$				; if yes, j to continue
	cmpi.b	#NVRT_End_Tag,d1		; is it End tag ?
	beq.s	20$				; if yes, j to handle
	bra.s	86$				; error in tree structure !!
;
; App tag found :
;
10$:	move.w	#NVRT_App_Tag_Size,d1		; d1 gets tag size
	bsr.s	30$				; j to move string to buffer
	bra.s	6$				; j to continue
;
; End tag found, return pointer to end of list :
;
20$:	move.l	d6,a2				; a2 gets ptr to Memblock
	clr.b	(a2)+				; write NULL for entire list
;
; Perform generic command cleanup and return :
;
22$:	bsr	NVRT_Command_Cleanup		; free buffer/release semaphore
	movem.l	(sp)+,a3-a6/d2-d7		; restore registers
	rts					; return to caller
;
;
;
; Subroutine to move App or Item name to MemBlock :
;	- d0 has string length
;	- d1 has tag size
;
30$:	move.l	a0,-(sp)			; save registers
	move.l	d6,a2				; a2 gets ptr to Memblock
	add.w	d1,a0				; increment past tag
	subq.w	#1,d0				; d0 gets correct loop count
32$:	move.b	0(a1,a0.w),(a2)+		; move char to buffer
	addq.w	#1,a0				; increment offset
	dbra.s	d0,32$				; loop for all chars
	clr.b	(a2)+				; write NULL for App or Item name
	move.l	(sp)+,a0			; restore registers
	move.l	a2,d6				; d6 gets ptr to Memblock
	rts
;
; Error found in tree structure :
;
86$:	bsr	NVRT_Tree_Recovery		; attempt to recover data
	bra.s	22$				; j to return

 ENDC



*****i* nonvolatile.library/SizeItemNVRAM ************************************
*
*   NAME
*	SizeItemNVRAM - Determines the size of an item.
*
*   SYNOPSIS
*	SizeItemNVRAM (SizePtr, NVBase, AppName, ItemName)
*		      a2       a5      a4	a3
*
*	SizeItemNVRAM (ULONG *, struct NVBase *, char *, char *)
*
*   FUNCTION
*	Finds the size of the data associated to the App/Item requested.
*	If the data item size passed in is nonzero, then this item exists on
*	the disk.  In this case, the NVRAM item of the same name is ignored
*	by not updating the SizePtr information. If the size passed in is zero,
*	the value at SizePtr is updated.  Also, the protection state of the
*	App/Item is stored in the long word following the SizePtr location.
*
*   INPUTS
*	SizePtr		Pointer to 2 long words containing size and protection.
*	NVBase		Pointer the base of nonvolatile.library.
*	AppName		Pointer identifying the AppName.
*	ItemName	Pointer identifying the ItemName to be sized.
*
*   RESULT
*	SizePtr		Two long words at this location are updated if
*			requested App/Item is found and the data size is
*			larger than the previous size.
*
*   SEE ALSO
*	SizeItemDisk()
*
******************************************************************************
_SizeItemNVRAM:
SizeItemNVRAM:

 IFGT	DISKVERSION

	rts					;

 ELSE
;
; Check if item is already on NVdisk :
;
	tst.l	(a2)				; is disk item size = 0 ?
	beq.s	2$				; if yes, j to continue
	rts
2$:	movem.l	a0-a6/d0-d7,-(sp)		; save registers
;
; Init registers for this command :
;
	move.l	a3,d0				; a3 gets App name and
	move.l	a4,a3				; a4 gets Item name
	move.l	d0,a4				;
	move.l	a2,d6				; d6 gets ptr to Size/Pro data
;
; Perform generic command initialization :
;
	bsr	NVRT_Command_Init		; get semaphore/allocate buffer
	bne.s	20$				; if no, j to return
	bsr	NVRT_Verify_NVRAM_Tag		; is data area OK ?
	bne.s	20$				; if no, j to return not found
;
; Find App name that match requested :
;
	bsr	NVRT_Find_App_Match		; a0 gets offset of App tag
	bne.s	20$				; if not found, j to return
	move.l	a0,d3				; d3 gets offset of App tag
;
; App name match, find Item name that match requested :
;
	bsr	NVRT_Find_Item_Match		; a0 gets offset of Data tag
						; d0 gets data length
						; d2 gets item number
						; d3 gets offset of App tag
						; d4 gets offset of Item tag
	bne.s	20$				; if not found, j to return
;
; App and Item name match found, determine size difference :
;
	move.l	d6,a2				; a2 gets ptr to Memblock
	move.l	(a2),d1				; d1 gets passed in size
	cmp.l	d0,d1				; is found data larger ?
	bgt.s	18$				; if no, j to check protection
;
; Found App/Item data size is larger :
;
	move.l	d0,(a2)				; update Size
18$:	btst.b	#NVRT_LOCKB_ITEM,1(a1,d4.w)	; is protection set in NVRAM ?
	beq.s	20$				; if no, j to return
	ori.l	#NVEF_DELETE,4(a2)		; update Protection
;
; Perform generic command cleanup and return :
;
20$:	bsr	NVRT_Command_Cleanup		; free buffer/release semaphore
	movem.l	(sp)+,a0-a6/d0-d7		; restore registers
	rts					; return to caller

 ENDC



*****i* nonvolatile.library/SetNVRAMProtection *******************************
*
*   NAME
*	SetNVRAMProtection - Attempts to change the protection of a file.
*
*   SYNOPSIS
*	SetNVRAMProtection (AppName, ItemName, NVBase, Protection, Success)
*			    a3		a4	 a5	 d2	     d3
*
*	SetNVRAMProtection (char *, char *, struct NVBase *, LONG, LONG)
*
*   FUNCTION
*	Changes the delete bit for a file to the state indicated by the
*	corresponding bit it Protection. The caller of this routine wishes
*	to accumulate the success of each called routine (1 is success).
*
*	Note:	This routine returns Success in d3.
*
*   INPUTS
*	AppName		Pointer to a NULL terminated App name string.
*	ItemName	Pointer to a NULL terminated Item name string.
*	NVBase		Pointer the base of nonvolatile.library.
*	Protection	New protection status.  Only the least significant bit
*			bit (delete) is used.
*	Success		Whether the protection was changed on a copy of this
*			data on another device.
*
*   RESULT
*	Success		The success of this routine ORd with the previous
*			value of Success.
*
*   SEE ALSO
*	SetDiskProtection()
*
******************************************************************************
_SetNVRAMProtection:
SetNVRAMProtection:

 IFGT	DISKVERSION

	rts					;

 ELSE

	movem.l	a0-a6/d1-d2/d4-d7,-(sp)		; save registers
;
; Init registers for this command :
;
	move.l	d2,d5				; d5 gets new protection state
	move.l	d3,d6				; d6 gets previous success
;
; Perform generic command initialization :
;
	bsr	NVRT_Command_Init		; get semaphore/allocate buffer
	bne.s	20$				; if no, j to return
	bsr	NVRT_Verify_NVRAM_Tag		; is data area OK ?
	bne.s	20$				; if no, j to return not found
;
; Find App name that match requested :
;
	bsr	NVRT_Find_App_Match		; a0 gets offset of App tag
	bne.s	20$				; if not found, j to return
	move.l	a0,d3				; d3 gets offset of App tag
;
; App name match, find Item name that match requested :
;
	bsr	NVRT_Find_Item_Match		; a0 gets offset of Data tag
						; d0 gets data length
						; d2 gets item number
						; d3 gets offset of App tag
						; d4 gets offset of Item tag
	bne.s	20$				; if not found, j to return
;
; App and Item name match found, update lock bit :
;
	moveq.l	#0,d0				; d0 gets MRU and clear lock
	btst.b	#NVEB_DELETE,d5			; set protection ?
	beq.s	8$				; if no, j to continue
	or.b	#NVRT_LOCKF_ITEM,d0		; set lock
8$:	move.b	d0,1(a1,d4.w)			; update lock bit
	moveq.l	#1,d6				; d6 gets success
;
; Perform LRU update and write device :
;
;	bsr	NVRT_LRU_Update			; update all item's LRU fields
	move.l	d4,a0				; a0 gets offset of Item Tag
	addq.l	#1,a0				; incr to offset of LRU/lock byte
	moveq.l	#1,d0				; d0 gets byte count
	bsr	NVRT_Device_Write_Short		; update NVRAM device
;
; Create success status :
;
20$:	move.l	d6,d0				; d3 gets return success
	move.l	d0,d3				; ***** set d0 for testing
;
; Perform generic command cleanup and return :
;
	bsr	NVRT_Command_Cleanup		; free buffer/release semaphore
	movem.l	(sp)+,a0-a6/d1-d2/d4-d7		; restore registers
	rts

 ENDC



*****i* nonvolatile.library/ReadNVRAM ****************************************
*
*   NAME
*	ReadNVRAM - Reads an Item from the NVRAM.
*
*   SYNOPSIS
*	Data = ReadNVRAM(AppName, ItemName, NVBase)
*	d0		 a0	  a1	    a6
*
*	APTR ReadNVRAM(char *, char *, struct NVBase *)
*
*   FUNCTION
*	Searches the NVRAM for the AppName and ItemName.  If found, it
*	allocates storage for the App/Item's data plus one additional long
*	word for the size of memory allocated.  The size of the allocation
*	is stored in the first long word, and the data is stored beginning
*	at the second long word.
*
*	If either the AppName and ItemName cannot be found, or the memory
*	cannot be allocated a NULL will be returned.
*
*
*   INPUTS
*	AppName		Pointer to NULL terminated string specifying App name
*	ItemName	Pointer to NULL terminated string specifying Item name
*	NVBase 		Nonvolatile library base.
*
*   RESULT
*	Data 		Pointer to the allocated memory holding size and data.
*			The pointer is at the begining of data not begining of
*			the allocated memory block.  If NULL, the App/Item
*			was not found.
*
*   SEE ALSO
*	ReadDisk()
*
******************************************************************************
_ReadNVRAM:
ReadNVRAM:

 IFGT	DISKVERSION

	moveq.l	#0,d0				;
	rts					;

 ELSE

	movem.l	a0-a6/d1-d7,-(sp)		; save registers
;
; Init registers for this command :
;
	move.l	a0,a3				; a3 gets ptr to requested App
	move.l	a1,a4				; a4 gets ptr to requested Item
	move.l	a6,a5				; a5 gets ptr to library base
;
; Perform generic command initialization :
;
	bsr	NVRT_Command_Init		; get semaphore/allocate buffer
	bne.s	20$				; if no, j to return
	bsr	NVRT_Verify_NVRAM_Tag		; is data area OK ?
	bne.s	20$				; if no, j to return not found
;
; Check for reserved App/Item name for system data :
;
	bsr	NVRT_Compare_RSVD_String	; call routine to check for rsvd string
	bne.s	4$				; if not, j to continue
;
; Reserved App/Item detected, force pointers and count to reserved data :
;
	move.l	#NVRT_RSVD_ITEM_LOCN-NVRT_Data_Tag_Size,a0	; a0 gets offset to rsvd data
	moveq.l	#NVRT_RSVD_DATA_MOVE,d0		; d0 gets rsvd data size
	move.l	d0,d6				; d6 gets same
	bra.s	10$				; branch to allocate and return
;
; Find App name that match requested :
;
4$:	bsr	NVRT_Find_App_Match		; a0 gets offset of App tag
	bne.s	20$				; if not found, j to return
	move.l	a0,d3				; d3 gets offset of App tag
;
; App name match, find Item name that match requested :
;
	bsr	NVRT_Find_Item_Match		; a0 gets offset of Data tag
						; d0 gets data length
						; d2 gets item number
						; d3 gets offset of App tag
						; d4 gets offset of Item tag
	bne.s	20$				; if not found, j to return
	move.l	a0,d5				; d5 gets offset of Data tag
	move.l	d0,d6				; d6 gets data length
;
; *** App & Item name match found :
;	- make this item the most recently used
;	- call exec to allocate block of memory
;
	bsr	NVRT_LRU_Touch			; make this Item the MRU
;
; Call Exec lib to allocate block of RAM for read data :
;
10$:	addq.l	#4,d0				; make room for long word data length
	movem.l	d0-d6/a0-a1,-(sp)		; save registers
	move.l	nv_ExecBase(a5),a6		; a6 gets exec base
	move.l	#MEMF_PUBLIC!MEMF_CLEAR,d1	; d1 gets mem type and options
	JSRLIB	AllocMem			; allocate memory for copy
	move.l	d0,a2				; save pointer to allocated RAM
	tst.l	d0				; is allocate OK ?
	movem.l	(sp)+,d0-d6/a0-a1		; restore registers
	beq.s	20$				; if no, j to handle error
;
; Write size of data and the data to the allocated RAM :
;
	move.l	d0,(a2)+			; write size of allocation to block
	move.l	a2,d0				; save pointer to begining of data
	subq.w	#1,d6				; d6 gets correct loop count
	addq.w	#NVRT_Data_Tag_Size,a0		; a0 gets offset of data
12$:	move.b  0(a1,a0.w),(a2)+		; move data to RAM
	addq.l	#1,a0				; add one to offset
	dbra.s	d6,12$				; loop for all data
;
; Perform LRU update and write device :
;
	bsr	NVRT_LRU_Update			; update all Item's LRU values
	bsr	NVRT_Device_Write		;
	bra.s	30$				; j to return
;
; Return App/Item not found or bad memory allocate status :
;
20$:	moveq.l	#0,d0				; d0 gets NFG status
30$:	bsr	NVRT_Command_Cleanup		; free buffer/release semaphore
	movem.l	(sp)+,a0-a6/d1-d7		; restore registers
	rts					; return to caller

 ENDC



*****i* nonvolatile.library/WriteNVRAM ***************************************
*
*   NAME
*	WriteNVRAM - Writes an Item to the NVRAM.
*
*   SYNOPSIS
*	Error = WriteNVRAM(AppName, ItemName, Data, Length, Mode, NVBase)
*	d0		   a0	    a1	      a2    d0	    d1	  a6
*
*	UWORD WriteNVRAM(char *, char *, APTR, ULONG, struct NVBase *)
*
*   FUNCTION
*	Attempts to store the information pointed to by Data into the NVRAM.
*	The data will be associated with AppName and ItemName.  Data will be
*	stored with automatic deletion of least recently used item if free
*	(unused) space is not found.
*
*	This routine is divided into 3 major cases :
*		- App & Item name match found
*		- App name match found, no Item match found
*		- No App name or Item name match found
*	These cases are marked with ***** in the source file comment header.
*
*	Before any writes are made (or overwrites) to NVRAM it must be
*	determined if there is room for new data.  Free space is defined as
*	NVRAM area unused at the moment and available space is defined as
*	space that can be used following LRU deletion.
*
*
*   INPUTS
*	AppName		Pointer to NULL terminated string specifying App name
*	ItemName	Pointer to NULL terminated string specifying Item name
*	Data		Pointer to data to be stored.
*	Length		Size of data in bytes.
*	NVBase		Nonvolatile library base
*
*   RESULT
*	Error		0 if data stored no error.
*			NVERR_FAIL - Data cannot fit in NVRAM.
*			NVERR_FATAL - No NVRAM found in system.
*
*   SEE ALSO
*	WriteDisk()
*
******************************************************************************
_WriteNVRAM:
WriteNVRAM:

 IFGT	DISKVERSION

	moveq.l	#NVERR_FATAL,d0			; d0 gets status
	rts					;

 ELSE

	movem.l	a0-a6/d1-d7,-(sp)		; save registers
;
; Init registers for this command :
;
	move.l	a0,a3				; a3 gets ptr to requested App
	move.l	a1,a4				; a4 gets ptr to requested Item
	move.l	a6,a5				; a5 gets library base
	move.l	d0,d6				; d6 gets write data length
	move.l	d1,d7				; d7 gets write data mode
;
; Perform generic command initialization :
;
	bsr	NVRT_Command_Init		; get semaphore/allocate buffer
	bne	56$				; if no, j to return
	bsr	NVRT_Verify_NVRAM_Tag		; is data area OK ?
	bne	56$				; if no, j to return
;
; Check for reserved App/Item name for system data :
;
	bsr	NVRT_Compare_RSVD_String	; call routine to check for rsvd string
	bne.s	4$				; if not, j to continue
;
; Reserved App/Item detected :
;	- force pointers and count to reserved data
;	- jump to write data and update device
;
	move.l	#NVRT_RSVD_ITEM_LOCN,a0		; a0 gets offset to rsvd data
	moveq.l	#NVRT_RSVD_DATA_MOVE-1,d6	; d6 gets rsvd data size - 1 for loop cnt
2$:	move.b  (a2)+,0(a1,a0.w)		; move data
	addq.l	#1,a0				; add one to offset
	dbra.s	d6,2$				; loop for all data
	bra	51$				; branch to update device
;
; Find App name that match requested :
;
4$:	bsr	NVRT_Find_App_Match		; a0 gets offset of App tag
	bne	30$				; if not found, j to write all new
	move.l	a0,d3				; d3 gets offset of App tag
;
; App name match found, find item name that match requested :
;
	bsr	NVRT_Find_Item_Match		; a0 gets offset of Data tag
						; d0 gets data length
						; d2 gets item number
						; d3 gets offset of App tag
						; d4 gets offset of Item tag
	bne.s	20$				; if not found, j to return
	move.l	a0,d5				; d5 gets offset of Data tag
;
; ***** App & Item name match found :
;	- make this item the most recently used
;	- compare requested data size to current data size
;	- jump to appropriate routine to handle data move
;
	bsr	NVRT_LRU_Touch			; make this Item the MRU
	cmp.w	d6,d0				; compare request vs current size
	beq.s	14$				; if =, j to write data only
	blt.s	12$				; if >, j to stretch and write
						; if <, fall through
;
; Requested data is less than current data size :
;	- calculate difference in data size
;	- call to move buffer data up to fit new data size
;	- call to write new data to RAM data area
;	- call to update NVRAM device
;	- j to return
;
	move.l	d0,d2				; d2 gets current data size
	sub.l	d6,d2				; d2 gets difference in data size

	bsr	NVRT_Incr_To_Next_Tag		; a0 gets ptr to next tag
	bsr	NVRT_Shrink_Data_Area		; call to make less room for new data

	move.l	#NVRT_DATA_REQ,d1		; d1 gets Data space only parameter
	bsr	NVRT_Write_Data_To_Buffer	; call to move new data to buffer

	bra	50$				; j to return no error
;
; Requested data is larger than current data size :
;
;	- call to calculate required space
;	- call to calculate free space
;	- Determine if there is enough free space for new data :
;		- If no :
;			- determine if there is room after LRU deletion :
;				- If no, j to return error
;			- delete the current entry
;			- j to do LRU detetion and insertion
;		- If yes :
;			- call to stretch data to make room for new Data
;			- write new Data to RAM data area
;			- call to update NVRAM device
;			- j to return
;
12$:	move.l	#NVRT_DATA_REQ,d1		; d1 gets Data space only parameter
	bsr	NVRT_Calc_Required_Space	; d1 gets amount of space required
	sub.l	d0,d1				; reduce required space by current data size
	bsr	NVRT_Calc_Free_Space		; d0 gets free space in NVRAM
;
; Determine if there is enough free space for new data :
;
	cmp.l	d1,d0				; is there enough room without deletion ?
	bge.s	128$				; if yes, j to use free space
;
; Not enough free space, check if possible room :
;
	move.b	1(a1,d4.w),d2			; d2 gets current lock state
	bset.b	#NVRT_LOCKB_ITEM,1(a1,d4.w)	; clear protection
	bsr	NVRT_Calc_Avail_Space		; d0 gets possible avail space in NVRAM
	btst.b	#NVRT_LOCKB_ITEM,d2		; was protection set ?
	bne.s	122$				; if no, j to skip
	bclr.b	#NVRT_LOCKB_ITEM,1(a1,d4.w)	; restore protection
122$:	cmp.l	d1,d0				; is there enough room with deletion ?
	blt	54$				; if no, j to return error
;
; Not enough free space, however there will be room after deletion :
;	- mark this item as LRU and unlocked
;	- j to do LRU delete and write item
;
;	move.b	#NVRT_MAX_LRU,1(a1,d4.w)	; make this item the most LRU and unlock
	bra.s	204$				; j to do LRU deletion
;
; There is enough free space to add new data :
;
128$:	movem.l	a0/d1,-(sp)			; save End tag ptr and size diff
	move.l	d5,a0				; a0 gets current data tag offset
	bsr	NVRT_Incr_To_Next_Tag		; a0 gets source ptr
	move.l	a0,d0				; d0 gets same
	movem.l	(sp)+,a0/d1			; restore End tag ptr and size diff
	bsr	NVRT_Stretch_Data_Area		; call to make room for new data
	move.l	#NVRT_DATA_REQ,d1		; d1 gets Data space only parameter
	bsr	NVRT_Write_Data_To_Buffer	; call to move new data to buffer
	bra.s	50$				; j to return no error
;
; Requested data size is same as current data size :
;	- move write data to RAM data area
;	- call to update NVRAM device
;	- j to return
;
14$:	move.l	#NVRT_DATA_REQ,d1		; d1 gets Data space only parameter
	bsr	NVRT_Write_Data_To_Buffer	; call to move new data to buffer
	bra.s	50$				; j to return no error
;
; ***** App name match found, no Item match found :
;	- calculate avail space
;		-  if no room, j to return error
;	- call to stretch data to make room for new Item and Data
;	- write new Item and Data to RAM data area
;	- call to update NVRAM device
;	- j to return
;
20$:	move.l	a0,d4				; d4 gets offset of next tag
	move.l	#NVRT_ITEM_DATA_REQ,d1		; d1 gets Item/Data parameter
	bsr	NVRT_Calc_Required_Space	; d1 gets amount of space required
	bsr	NVRT_Calc_Free_Space		; d0 gets avail space
;
; Determine if there is enough free space for new data :
;
	cmp.l	d1,d0				; is there enough room ?
	bge.s	208$				; if yes, j to continue
;
; Not enough free space, check if possible room :
;
202$:	bsr	NVRT_Calc_Avail_Space		; d0 gets possible avail space in NVRAM
	cmp.l	d1,d0				; is there enough room with deletion ?
	blt.s	54$				; if no, j to return error
;
; Not enough free space, however there will be room after deletion :
;
204$:	bsr	NVRT_LRU_Deletion		; delete least recently used item
	bra	4$				; j to repeat write function
;
; There is enough free space to add new data :
;
208$:	move.l	d4,d0				; d0 gets next tag offset
	bsr	NVRT_Stretch_Data_Area		; call to make room for new data
	move.l	#NVRT_ITEM_DATA_REQ,d1		; d1 gets Item/Data paramter
	bsr	NVRT_Write_Data_To_Buffer	; call to move new data to buffer
	bra.s	50$				; j to return no error
;
; ***** No App name or Item name match found :
;	- is mode = OLDFILE ?
;		- if yes, j to return error
;	- call to calculate free space
;		- if no room, j to return
;	- call to write new App, Item, and Data to RAM data area
;	- call to update NVRAM device
;	- j to return
;
30$:	cmp.w	#MODE_OLDFILE,d7		; is file supposed to be here ?
	beq.s	52$				; if yes, j to return
	move.l	a0,d4				; d4 gets offset of next tag
	move.l	#NVRT_APP_ITEM_DATA_REQ,d1	; d1 gets App/Item/Data parameter
	bsr	NVRT_Calc_Required_Space	; d1 gets amount of space required
	bsr	NVRT_Calc_Free_Space		; d0 gets avail space
;
; Determine if there is enough free space for new data :
;
	cmp.l	d1,d0				; is there enough room ?
	bge.s	308$				; if yes, j to return
;
; Not enough free space, check if possible room :
;
	bsr	NVRT_Calc_Avail_Space		; d0 gets possible avail space in NVRAM
	cmp.l	d1,d0				; is there enough room with deletion ?
	blt.s	54$				; if no, j to return error
;
; Not enough free space, however there will be room after deletion :
;
304$:	bsr	NVRT_LRU_Deletion		; delete least recently used item
	bra	4$				; j to repeat write function
;
; There is enough free space to add new data :
;
308$:	move.l	#NVRT_APP_ITEM_DATA_REQ,d1	; d1 gets App/Item/Data paramter
	bsr	NVRT_Write_Data_To_Buffer	; call to move new data to buffer
	move.b	#NVRT_End_Tag,0(a1,a0.w)	; write end marker to RAM copy area
;
; UPdate LRU values and NVRAM device and jump to return no error :
;
50$:	bsr	NVRT_LRU_Update			; update all Item's LRU values
51$:	bsr	NVRT_Device_Write		; call to write buffer to device
	bra.s	60$				; j to return
;
; Return OLD_FILE request and no App/Item match found or
; no room for data status :
;
52$:
54$:	moveq.l	#NVERR_FAIL,d0			; d0 gets status
	bra.s	62$				; j to return
;
; Return no NVRAM in system status :
;
56$:	moveq.l	#NVERR_FATAL,d0			; d0 gets status
	bra.s	62$				; j to return
;
; Return OK :
;
60$:	moveq.l	#0,d0				; d0 gets good status
;
; Perform generic command cleanup and return :
;
62$:	bsr	NVRT_Command_Cleanup		; free buffer/release semaphore
	movem.l	(sp)+,a0-a6/d1-d7		; restore registers
	rts

 ENDC



*****i* nonvolatile.library/DeleteNVRAM **************************************
*
*   NAME
*	DeleteNVRAM - Deletes an Item from the NVRAM.
*
*   SYNOPSIS
*	Success = DeleteNVRAM(AppName, ItemName, NVBase)
*	d0		      a0       a1	 a6
*
*	BOOL DeleteNVRAM(char *, char *, struct NVBase *)
*
*   FUNCTION
*	Searches the NVRAM for the AppName and ItemName.  If they exist, any
*	data assocated with them is freed (ie ItemName is unlinked).  If
*	there are no other ItemNames assocated with AppName it is also
*	removed.
*
*   INPUTS
*	AppName		Pointer to NULL terminated string specifying App name
*	ItemName	Pointer to NULL terminated string specifying Item name
*	NVBase		Nonvolatile library base
*
*   RESULT
*	Success 	TRUE if AppName and ItemName are found.
*			FALSE if AppName and ItemName are not found.
*
*   SEE ALSO
*	ReadDisk()
*
******************************************************************************
_DeleteNVRAM:
DeleteNVRAM:

 IFGT	DISKVERSION

	moveq.l	#0,d0				; d0 gets FALSE
	rts					;

 ELSE

	movem.l	a0-a6/d1-d7,-(sp)		; save registers
;
; Init registers for this command :
;
	move.l	a0,a3				; a3 gets ptr to requested App
	move.l	a1,a4				; a4 gets ptr to requested Item
	move.l	a6,a5				; a5 gets library base
;
; Perform generic command initialization :
;
	bsr	NVRT_Command_Init		; get semaphore/allocate buffer
	bne.s	52$				; if no, j to return not found
	bsr	NVRT_Verify_NVRAM_Tag		; is data area OK
	bne.s	52$				; if no, j to return not found
;
; Check for reserved App/Item name for system data :
;
	bsr	NVRT_Compare_RSVD_String	; call routine to check for rsvd string
	beq.s	52$				; if yes, j to return not found
;
; Find App name that match requested :
;
4$:	bsr	NVRT_Find_App_Match		; a0 gets offset of App tag
	bne.s	52$				; if not found, j to write all new
	move.l	a0,d3				; d3 gets offset of App tag
;
; App name match found, find item name that match requested :
;
	bsr	NVRT_Find_Item_Match		; a0 gets offset of Data tag
						; d0 gets data length
						; d2 gets item number
						; d3 gets offset of App tag
						; d4 gets offset of Item tag
	bne.s	52$				; if not found, j to return
	move.l	a0,d5				; d5 gets offset of Data tag
	move.l	d0,d6				; d6 gets data length
;
; App and Item name match found :
;	- determine if lock bit is set :
;		- If yes, j to return
;	- determine if this is only Item for App
;		- If yes, set pointers to delete Item/Data and App
;		- If no, set pointers to delete Item/Data
;
	btst	#NVRT_LOCKB_ITEM,1(a1,d4.w)	; is this item locked ?
	bne.s	52$				; if yes, j to return

	bsr	NVRT_Incr_To_Next_Tag		; a0 gets offset of next NVRAM tag
	cmp	#NVRT_Item_Tag,d1		; is next tag another item ?
	beq.s	22$				; if yes, j to delete only item
	cmp	#1,d2				; is found item number > 1
	bgt.s	22$				; if yes, j to delete only item

	move.l	d3,d1				; d1 gets offset of App tag
	bra.s	24$				; j to calculate amount to shrink

22$:	move.l	d4,d1				; d1 gets offset of Item tag

24$:	move.l	a0,d2				; d2 gets offset of next tag
	sub.l	d1,d2				; d2 gets amount to shrink
;
; Call routine to delete requested Item (maybe App) :
;
	bsr	NVRT_Shrink_Data_Area		; call to make less room for new data
;
; Update the NVRAM device :
;
	bsr	NVRT_Device_Write		; call to write buffer to device
;
; Return success flag :
;
50$:	moveq.l	#1,d0				; d0 gets TRUE
	bra.s	60$				; j to return
;
; Return non success flag :
;
52$:	moveq.l	#0,d0				; d0 gets FALSE
;
; Perform generic command cleanup and return :
;
60$:	bsr	NVRT_Command_Cleanup		; free buffer/release semaphore
	movem.l	(sp)+,a0-a6/d1-d7		; restore registers
	rts

 ENDIF



*****i*************************************************************************
*
*
* The following are support subroutines used by the above routines
*
*
*****i*************************************************************************


 IFLE	DISKVERSION


; *****************************************************************************
;
; The following subroutine will find out how much space is available after
; any necessary automatic deletion.  The amount available is as follows :
;
;	AVAIL = (SIZE OF NVRAM - RESERVED BYTES - LOCKED BYTES)
;
;	On Entry:	a1 = pointer to RAM data area
;
;	On Exit:	d0 = byte count of avail space
;
;	Regs used:	d0
;
NVRT_Calc_Avail_Space:

	movem.l	d1-d7/a0-a6,-(sp)		; save registers
;
; Init registers :
;
	move.l	#NVRT_FIRST_APP_LOCN,a0		; a0 gets offset of first App tag
	moveq.l	#0,d2				; d2 gets total locked size
	move.l	d2,d3				; d3 gets App size holder
	move.l	d2,d4				; d4 gets lock flag
	bsr	NVRT_Read_Tag			; read the first tag
	bra.s	8$				; j to decode
;
; Loop thru all items adding up locked space :
;
6$:	bsr	NVRT_Incr_To_Next_Tag		; read next tag
8$:	cmpi.b	#NVRT_App_Tag,d1		; is it App tag ?
	beq.s	10$				; if yes, j to handle
	cmpi.b	#NVRT_Item_Tag,d1		; is it Item tag ?
	beq.s	12$				; if yes, j to handle
	cmpi.b	#NVRT_Data_Tag,d1		; is it Data tag ?
	beq.s	14$				; if yes, j to handle
	cmpi.b	#NVRT_End_Tag,d1		; is it End tag ?
	beq.s	16$				; if yes, j to handle
	bra.s	30$				;
;
; App tag found :
;
10$:	bclr	#0,d4				; is lock flag set ?
	beq.s	102$				; if no, j to continue
	add.w	d3,d2				; add previous App's size to total
	add.w	#NVRT_App_Tag_Size,d2		; add App tag size
102$:	move.b	d0,d3				; d3 gets this App's size
	bra.s	6$				; j to continue
;
; Item tag found :
;
12$:	btst	#NVRT_LOCKB_ITEM,1(a1,a0.w)	; is this item locked ?
	beq.s	6$				; if no, j to continue
	move.b	#3,d4				; set lock flag for data and app
	add.w	d0,d2				; add this Item size to total
	add.w	#NVRT_Item_Tag_Size,d2		; add Item tag size
	bra.s	6$				; j to continue
;
; Data tag found :
;
14$:	bclr	#1,d4				; is lock flag set ?
	beq.s	6$				; if yes, j to continue
	lsl.w	#8,d0				; make room for other half of data size
	move.b  1(a1,a0.w),d0			; d0 gets other half of size
	add.w	d0,d2				; add data size to total
	add.w	#NVRT_Data_Tag_Size,d2		; add Data tag size
	bra.s	6$				; j to continue
;
; End tag found :
;
16$:	add.w	#NVRT_End_Tag_Size,d2		; add End tag size
	btst	#0,d4				; is lock flag set ?
	beq.s	18$				; if no, j to continue
	add.w	d3,d2				; add previous App's size to total
;
; Calculate nonreserved - amount locked :
;
18$:	move.l	#(nvram_device_size-NVRT_RSVD_DATA_SIZE-NVRT_End_Tag_Size),d0
	sub.l	d2,d0				; return avail size

20$:	movem.l	(sp)+,d1-d7/a0-a6		; restore registers
	rts					; return to caller
;
30$:	bsr	NVRT_Tree_Recovery
	bra.s	20$



; *****************************************************************************
;
; The following subroutine will find out how much free space is currently
; in the NVRAM.  It will search for the End tag from the current location.
;
;
;	On Entry:	a0 = offset of current tag
;			a1 = base of RAM data area
;			d1 = tag type of current tag
;
;	On Exit:	a0 = offset of End tag
;			a1 = unchanged
;			d0 = byte count of free space
;
;	Regs used:	d0, a0
;
NVRT_Calc_Free_Space:

	movem.l	d1-d7/a1-a6,-(sp)		; save registers
;
; Call routine to find End tag :
;
4$:	bsr	NVRT_Find_Next_End_Tag		; a0 gets offset of End tag
	bne.s	10$				; if error, j to return 0
;
; Calculate remaining number of bytes :
;
8$:	moveq.l	#0,d1				; clear ms word of d1
	move.w	a0,d1				; d1 gets offset of End tag
	move.l	#nvram_device_size,d0		; d0 gets max size
	addq.l	#NVRT_End_Tag_Size,d1		; add one for size of End tag
	sub.l	d1,d0				; d0 gets # of bytes remaining
	bra.s	20$				; j to return

10$:	moveq.l	#0,d0				; d0 gets no space remaining
20$:	movem.l	(sp)+,d1-d7/a1-a6		; restore registers
	rts					; return to caller



; *****************************************************************************
;
; The following subroutine will calculate the required space for the write
; request.  The amount of data to add to the buffer depends on the parameter
; passed in d1 as follows :
;
;	NVRT_DATA_REQ 		= Calculate only Data size
;	NVRT_ITEM_DATA_REQ	= Calculate Item and Data size
;	NVRT_APP_ITEM_DATA_REQ	= Calculate App, Item and Data size
;
;
;	On Entry:	a0 = offset of Data tag
;			a1 = RAM data area base
;			a3 = ptr to requested App name
;			a4 = ptr to requested Item name
;			d1 = amount to check parameter
;			d3 = offset of App tag
;			d4 = offset of Item tag
;			d5 = offset of Data tag
;			d6 = data length
;
;	On Exit:	d1 = required space for write
;
;	Regs used:	none
;
NVRT_Calc_Required_Space:

	movem.l	d0/d2-d7/a0-a6,-(sp)		; save registers
	moveq.l	#0,d0				; clear required space variable
;
; Calculate space for App, Item, and Data :
;
	cmpi.l	#NVRT_APP_ITEM_DATA_REQ,d1	; calculate App, Item, Data ?
	bne.s	6$				; in no, j to continue
	addq.w	#NVRT_Full_Tag_Size,d0		; add size of App/Item/Data tags
	moveq.l	#NVRT_Max_String_Length,d2	; d0 gets max string length
2$:	tst.b	(a3)+				; is this NULL ?
	beq.s	8$				; if yes, j to continue
	addq.w	#1,d0				; add 1 to required space var
	dbra.s	d2,2$				; check entire string
	bra.s	8$				; j to continue
;
; Calculate space for Item, and Data :
;
6$:	cmpi.l	#NVRT_ITEM_DATA_REQ,d1		; move item and data ?
	bne.s	14$				; in no, j to continue
	addq.w	#NVRT_Item_Tag_Size+NVRT_Data_Tag_Size,d0	; add size of Item, Data tag
8$:	moveq.l	#NVRT_Max_String_Length,d2	; d0 gets max string length
10$:	tst.b	(a4)+				; is this NULL ?
	beq.s	16$				; if yes, j to continue
	addq.w	#1,d0				; add 1 to required space var
	dbra.s	d2,10$				; check entire string
	bra.s	16$				;
;
; Calculate space for Data :
;
14$:	cmpi.l	#NVRT_DATA_REQ,d1		; move just data ?
	bne.s	20$				; in no, j to exit
16$:	add.w	d6,d0				; add data len to req space var

20$:	move.l	d0,d1				; d1 gets return parameter
	movem.l	(sp)+,d0/d2-d7/a0-a6		; restore registers
	rts



; *****************************************************************************
;
; The following subroutine will attempt to obtain the nonvolatile.library
; semaphore and then allocate a block of RAM for the NVRAM device copy area.
;
;	On Entry:	a5 = ptr to nonvolatile.library base
;
;	On Exit:	a6 = ptr to Exec Base
;			nv_NVRAMCopy(a5) = updated with ptr to RAM data area
;			ZF = 0 if no success
;			ZF = 1 if success
;
;	Regs used:	CCR
;
NVRT_Command_Init:

	movem.l	d0-d1/a0-a1,-(sp)		; save registers

	move.l	nv_ExecBase(a5),a6		; a6 gets exec base
	lea	nv_Semaphore(a5),a0		; a0 gets ptr to semaphore
	JSRLIB	ObtainSemaphore			; obtain semaphore

	btst.b	#nv_FlagsB_MultiCommand,nv_Flags(a5)	; ## is multi-command set ?
	beq.s	4$					; ## if no, j to continue
	ori	#4,CCR					; ## set Z flag for good status
	bra.s	30$					; ## j to skip allocate and return

4$:	move.l	#nvram_device_size,d0		; d0 gets size of RAM to allocate
	move.l	#MEMF_PUBLIC!MEMF_CLEAR,d1	; d1 gets mem type and options
	JSRLIB	AllocMem			; allocate memory for copy
	move.l	d0,nv_NVRAMCopy(a5)		; write allocation to lib base
	beq.s	20$				; if no, j to handle error

	move.l	d0,a1				; a1 gets ptr to RAM data area
	bsr.s	NVRT_Device_Read		; read all device
	bra.s	30$				; j to return

20$:	andi	#~4,CCR				; clear Z flag
30$:	movem.l	(sp)+,d0-d1/a0-a1		; restore registers
	rts					; return to caller



; *****************************************************************************
;
; The following subroutine will deallocate the RAM data area and release the
; nonvolatile.library semaphore.
;
;	On Entry:	a5 = ptr to nonvolatile.library base
;
;	On Exit:	nv_NVRAMCopy(a5) = 0
;
;	Regs used:	CCR
;
NVRT_Command_Cleanup:

	movem.l	d0-d1/a0-a1,-(sp)		; save registers

	btst.b	#nv_FlagsB_MultiCommand,nv_Flags(a5)	; ## is multi-command set ?
	bne.s	20$					; ## if yes, j skip freemem

	tst.l	nv_NVRAMCopy(a5)		; was any RAM allocated ?
	beq.s	20$				; if no, j to return

	move.l	nv_ExecBase(a5),a6		; a6 gets exec base
	move.l	#nvram_device_size,d0		; d0 gets size of RAM to allocate
	move.l	nv_NVRAMCopy(a5),a1		; a1 gets ptr to RAM data area
	JSRLIB	FreeMem				; free allocated memory
	clr.l	nv_NVRAMCopy(a5)		; clear pointer to RAM data area
20$:
	lea	nv_Semaphore(a5),a0		; a0 gets ptr to semaphore
	JSRLIB	ReleaseSemaphore		; release semaphore

	movem.l	(sp)+,d0-d1/a0-a1		; restore registers
	rts					; return to caller



; *****************************************************************************
;
; The following subroutine will compare the request App and Item names to
; reserved values for system setting.  The status of the compare
; will be returned in the condition register.
;
;	On Entry:	a0 = offset of first App tag
;			a1 = pointer to RAM data area
;			a3 = pointer to requested App string
;			a4 = pointer to requested Item string
;
;	On Exit:	ZF = 1, if match detected
;			ZF = 0, if no match detected
;
;	Regs used:	CCR
;
NVRT_Compare_RSVD_String:
;
; new method of using two NULL strings for reserved name :
;
	tst.b	(a3)
	bne.s	4$
	tst.b	(a4)
4$:	rts
;
; old method of passing some reserved string :
;	movem.l	d0/a0-a1,-(sp)			; save registers
;
; Compare requested App name to reserved App name :
;
;	lea	NVRT_RSVD_APP_NAME,a0		; a0 gets ptr to rsvd App name
;	moveq.l	#NVRT_RSVD_APP_NAME_SIZE,d0	; d0 gets length
;	move.l	a3,a1				; a1 gets ptr to req App string
;	bsr.s	NVRT_Compare_String		; is App name a match ?
;	bne.s	2$				; if no, j to return
;
; Compare requested Item name to reserved Item name :
;
;	lea	NVRT_RSVD_ITEM_NAME,a0		; a1 gets ptr to rsvd Item name
;	moveq.l	#NVRT_RSVD_ITEM_NAME_SIZE,d0	; d0 gets length
;	move.l	a4,a1				; a1 gets ptr to req Item string
;	bsr.s	NVRT_Compare_String		; is Item name a match ?
;
;2$:	movem.l	(sp)+,d0/a0-a1			; restore registers
;	rts					; return to caller with ZF accordingly



; *****************************************************************************
;
; The following subroutine will compare two strings.  String 1 has a known
; length but is not NULL terminated.  This is the string residing in the NV
; RAM.  String 2 is passed by the user as the requested App of Item.  This
; routine will return the result in the condition register.
;
;	On Entry:	a0 = pointer to string 1
;			a1 = pointer to string 2 (NULL terminated)
;			d0 = length of string 1
;			a5 = nonvolatile library base
;
;	On Exit:	ZF = 1, if match detected
;			ZF = 0, if no match detected
;
;	Regs used:	d0, d1, a0, a1
;
NVRT_Compare_String:

	ext.w	d0				; sign extend string length
	ext.l	d0				; string length
	cmp.l	#31,d0				; is string at max
	bge.s	2$				; if yes, dont check for end of 2nd string
	tst.b	0(a1,d0.w)			; is string 2 terminated w/ NULL ?
	bne.s	4$				; if no, j to return
2$:	move.l	a6,-(sp)			; save registers
	move.l	nv_UTILBase(a5),a6		; get utility.library base
	JSRLIB	Strnicmp			; case insensitive string comp
	move.l	(sp)+,a6			; restore registers
	tst.l	d0				; are strings equal ?
4$:	rts					; return with condition code



; *****************************************************************************
;
; The following subroutine will read the entire NVRAM device into the RAM data
; area.  The status of the write will be returned in the CCR.
;
;
;	On Entry:	a1 = RAM data area base
;
;	On Exit:	ZF = 1, write successful
;			ZF = 0, write not successful
;
;	Regs used:	CCR
;
NVRT_Device_Read:
	movem.l	d0-d7/a0-a6,-(sp)		; save registers
	move.l	a1,a0				; a0 gets ptr to RAM data area
	suba.l	a1,a1				; a1 gets offset into buffer
	move.l	#nvram_device_size,d0		; d0 gets count of all data
	bsr	nvram_read_mult			; call low level rouitne to write device
	tst.w	d0				; read OK ?
	movem.l	(sp)+,d0-d7/a0-a6		; restore registers
	rts



; *****************************************************************************
;
; The following subroutine will move the entire RAM data area to the NVRAM
; device.  The status of the write will be returned in the CCR.
;
;
;	On Entry:	a1 = RAM data area base
;
;	On Exit:	ZF = 1, write successful
;			ZF = 0, write not successful
;
;	Regs used:	CCR
;
NVRT_Device_Write:
	movem.l	d0-d7/a0-a6,-(sp)		; save registers
	move.l	a1,a0				; a0 gets ptr to RAM data area
	suba.l	a1,a1				; a1 gets offset into buffer
	move.l	#nvram_device_size,d0		; d0 gets count of all data
	bsr	nvram_write_mult		; call low level rouitne to write device
	tst.w	d0				; read OK ?
	movem.l	(sp)+,d0-d7/a0-a6		; restore registers
	rts



; *****************************************************************************
;
; The following subroutine will move the entire RAM data area to the NVRAM
; device.  The status of the write will be returned in the CCR.
;
;
;	On Entry:	a1 = RAM data area base
;			a0 = RAM data area base
;			d0 = number of bytes to store
;
;	On Exit:	ZF = 1, write successful
;			ZF = 0, write not successful
;
;	Regs used:	CCR
;
NVRT_Device_Write_Short:
	movem.l	d0-d7/a0-a6,-(sp)		; save registers
	exg.l	a0,a1				; a0 gets base, a1 gets offset
	bsr	nvram_write_mult		; call low level rouitne to write device
	tst.w	d0				; read OK ?
	movem.l	(sp)+,d0-d7/a0-a6		; restore registers
	rts



; *****************************************************************************
;
; The following subroutine start at the first tag position and search all
; subsequent Apps for the App name requested.  If found, it returns pointers
; to App tag.  The status of the search is returned in Z flag of the CCR.
;
;	On Entry:	a0 = offset of current tag
;			a1 = pointer to RAM data area
;			a3 = ptr to requested App name string
;			a4 = ptr to requested Item name string
;
;	On Exit:	a0 = offset of found App tag, if successful
;			a0 = offset of End tag, if unsuccessful
;			a1 = unchanged
;			a3 = unchanged
;			a4 = unchanged
;			ZF = 1, if successful
;			ZF = 0, if unsuccessful
;
;	Regs used:	d0, d1, a0, CCR
;
NVRT_Find_App_Match:
;
; Read the first tag byte :
;
	move.l	#NVRT_FIRST_APP_LOCN,a0		; a0 gets offset of first App tag
	bsr	NVRT_Read_Tag			; d0 gets length & d1 gets type
	cmp.b	#NVRT_App_Tag,d1		; is this App tag (might be end) ?
	bne.s	8$				; if no, j to return
;
; Check for matching App name :
;
6$:	movem.l	a0-a1,-(sp)			; save registers
	addq.l	#NVRT_App_Tag_Size,a0		; a0 gets offset of NVRAM App string
	add.l	a1,a0				; a0 gets ptr to App string
	move.l	a3,a1				; a1 gets ptr to requested string
	bsr.s	NVRT_Compare_String		; call routine to compare strings
	movem.l	(sp)+,a0-a1			; restore registers
	beq.s	8$				; if match, j to check Item
	bsr.s	NVRT_Find_Next_App_Tag		; increment to next App tag
	beq.s	6$				; if found, j to check next App name
8$:	rts					; return to caller with ZF accorgingly



; *****************************************************************************
;
; The following subroutine start at the current tag position (found App tag)
; and search all subsequent Items for the Item name requested.  If found,
; it returns pointers to Item and Data tags, and the length of the Data.
; The status of the search is returned in Z flag of the CCR.
;
;	On Entry:	a0 = offset of current App tag
;			a1 = base of RAM data area
;			a3 = pointer to the requested App name string
;			a4 = pointer to the requested Item name string
;			d3 = offset of current App tag
;
;	On Exit:	a0 = offset of found Data tag, if successful
;			a0 = offset of next NVRAM tag, if unsuccessful
;			a1 = unchanged
;			a3 = unchanged
;			a4 = unchanged
;			d0 = data length, if successful
;			d2 = number of items searched befor finding match
;			d3 = unchanged
;			d4 = offset of found Item tag, if successful
;			ZF = 1, if successful
;			ZF = 0, if unsuccessful
;
;	Regs used:	d0, d1, d2, d4, a0, CCR
;
NVRT_Find_Item_Match:
;
; Init registers for Item App name search :
;
	moveq.l	#0,d2				; clear item counter
10$:	addq.l	#1,d2				; increment item counter
	bsr.s	NVRT_Find_Next_Item_Tag		; d0 gets length, d1 gets tag if found
	bne.s	20$				; if not found, j to return
	movem.l	a0-a1,-(sp)			; save registers
	addq.l	#NVRT_Item_Tag_Size,a0		; a0 gets offset of NVRAM Item string
	add.l	a1,a0				; a0 gets ptr to Item string
	move.l	a4,a1				; a1 gets ptr to requested string
	bsr	NVRT_Compare_String		; call routine to compare strings
	movem.l	(sp)+,a0-a1			; restore registers
	bne.s	10$				; if no match, j to check next Item
;
; Item name found, save pointers to Item and Data tags  :
;
	move.l	a0,d4				; d4 gets offset of found Item tag
	bsr.s	NVRT_Find_Next_Data_Tag		; a0 gets offset of Data tag
	bne.s	20$				; if not found, j to return
;
; Get size of Data :
;
	moveq.l	#0,d0				; clear ms word of d0
	move.w  0(a1,a0.w),d0			; d0 gets full Data tag
	andi.l	#~(NVRT_Tag_Mask<<8),d0		; clear tag field
	ori	#4,CCR				; set Z flag
20$:	rts					; return to caller with ZF accorgingly



; *****************************************************************************
;
; The following subroutine will increment the data area offset to the next
; App tag.  It is allowed to skip over Item and Data tags in the process.
; If an End tag is detected the search will end with a not found.
;
;	On Entry:	a0 = offset into RAM data area at curr tag
;			a1 = base of RAM data area
;			d0 = length field of curr tag
;			d1 = tag field of curr tag
;
;	On Exit:	a0 = offset of next App tag
;			a1 = base of RAM data area
;			d0 = length field of next tag
;			d1 = tag field of next tag
;			ZF = 1, if successful
;			ZF = 0, if unsuccessful
;
;	Regs used:	d0, d1, a0, CCR
;
NVRT_Find_Next_App_Tag:
2$:	bsr.s	NVRT_Incr_To_Next_Tag		; increment pointer to next tag filed
	bne.s	4$				; if error, j to return
	cmp.b	#NVRT_App_Tag,d1		; is this end tag ?
	beq.s	4$				; if yes, j to return
	cmp.b	#NVRT_End_Tag,d1		; is it end tag ?
	bne.s	2$				; if no, j to continue search
	or.b	d1,d1				; clear Z flag
4$:	rts					; return to caller



; *****************************************************************************
;
; The following subroutine will increment the data area offset to the next
; Data tag.  It is NOT allowed to skip over any other tags in the process.
; If any other tag is detected the search will end with a not found.
;
;	On Entry:	a0 = offset into RAM data area at curr tag
;			a1 = base of RAM data area
;			d0 = length field of curr tag
;			d1 = tag field of curr tag
;
;	On Exit:	a0 = offset of next Data tag
;			a1 = unchanged
;			d0 = length field of next tag
;			d1 = tag field of next tag
;			ZF = 1, if successful
;			ZF = 0, if unsuccessful
;
;	Regs used:	d0, d1, a0, CCR
;
NVRT_Find_Next_Data_Tag:
	bsr.s	NVRT_Incr_To_Next_Tag		; increment pointer to next tag filed
	bne.s	4$				; if error, j to return
	cmp.b	#NVRT_Data_Tag,d1		; is this data tag ?
4$:	rts					; return to caller with ZF accordingly



; *****************************************************************************
;
; The following subroutine will increment the data area offset to the next
; End tag.  It is allowed to skip over any other tags in the process.
;
;	On Entry:	a0 = offset into RAM data area at curr tag
;			a1 = base of RAM data area
;			d0 = length field of curr tag
;			d1 = tag field of curr tag
;
;	On Exit:	a0 = offset of next End tag
;			a1 = unchanged
;			d0 = length field of next tag
;			d1 = tag field of next tag
;			ZF = 1, if successful
;			ZF = 0, if unsuccessful
;
;	Regs used:	d0, d1, a0, CCR
;
NVRT_Find_Next_End_Tag:
2$:	bsr.s	NVRT_Incr_To_Next_Tag		; increment pointer to next tag filed
	bne.s	4$				; if error, j to return
	cmp.b	#NVRT_End_Tag,d1		; is this end tag ?
	bne.s	2$				; if no, j to continue search
4$:	rts					; return to caller with ZF accordingly



; *****************************************************************************
;
; The following subroutine will increment the data area offset to the next
; Item tag.  It is allowed to skip over Data tags in the process.
; If any oter type of tag is detected the search will end with a not found.
;
;	On Entry:	a0 = offset into RAM data area at curr tag
;			a1 = base of RAM data area
;			d0 = length field of curr tag
;			d1 = tag field of curr tag
;
;	On Exit:	a0 = offset of next Item tag
;			a1 = unchanged
;			d0 = length field of next tag
;			d1 = tag field of next tag
;			ZF = 1, if successful
;			ZF = 0, if unsuccessful
;
;	Regs used:	d0, d1, a0, CCR
;
NVRT_Find_Next_Item_Tag:
2$:	bsr.s	NVRT_Incr_To_Next_Tag		; increment pointer to next tag filed
	bne.s	4$				; if error, j to return
	cmp.b	#NVRT_Item_Tag,d1		; is this item tag ?
	beq.s	4$				; if yes, j to return
	cmp.b	#NVRT_Data_Tag,d1		; is it data tag ?
	beq.s	2$				; if yes, j to continue search
4$:	rts					; return to caller with ZF accordingly




; *****************************************************************************
;
; The following subroutine will increment to the next tag based on the type
; field of the current tag.  The condition code register will indicate the
; status of the increment.
;
;	On Entry:	a0 = offset into RAM data area of current tag
;			a1 = base of RAM data area
;			d0 = length field of curr tag
;			d1 = tag field of curr tag
;
;	On Exit:	a0 = offset of next tag
;			a1 = unchanged
;			d0 = length field of next tag
;			d1 = tag field of next tag
;			ZF = 1, if successful
;			ZF = 0, if unsuccessful
;
;	Regs used:	d0, d1, a0, CCR
;
NVRT_Incr_To_Next_Tag:

8$:	bsr	NVRT_Read_Tag			; d0 gets current length & d1 gets type
	cmpi.b	#NVRT_App_Tag,d1		; is it App tag ?
	bne.s	10$				; if no, j to continue
	addq.w	#NVRT_App_Tag_Size,a0		; increment offset for tag size
	add.w	d0,a0				; add App string length to offset
	bra.s	16$

10$:	cmpi.b	#NVRT_Item_Tag,d1		; is it Item tag ?
	bne.s	12$				; if no, j to continue
	addq.w	#NVRT_Item_Tag_Size,a0		; increment offset for tag size
	add.w	d0,a0				; add Item string length to offset
	bra.s	16$

12$:	cmpi.b	#NVRT_Data_Tag,d1		; is it Data tag ?
	bne.s	14$				; if no, j to continue
	lsl.w	#8,d0				; make room for other half of data size
	move.b  1(a1,a0.w),d0			; d0 gets other half of size
	addq.w	#NVRT_Data_Tag_Size,a0		; increment offset for tag size
	adda.w	d0,a0				; add data length to ofset
	bra.s	16$

14$:	cmpi.b	#NVRT_End_Tag,d1		; is it End tag ?
	rts					; return to caller with ZF accordingly

16$:	bsr	NVRT_Read_Tag			; d0 gets length & d1 gets type
	ori	#4,CCR				; set Z flag
	rts					; return to caller with ZF = 1



; *****************************************************************************
;
; The following subroutine will delete the least recently used item.  This may
; cause the associated App tag to be deleted as well.  Items that are locked
; will not be LRU deleted.
;
; The routine will start at the top of NVRAM data area and search for the Item
; tag with the highest value LRU field.
;
;	On Entry:	a1 = ptr to RAM data area
;
;	On Exit:	nothing
;
;	Regs used:	none
;
NVRT_LRU_Deletion:

	movem.l	d0-d7/a0-a6,-(sp)		; save registers

	move.l	#NVRT_FIRST_APP_LOCN,a0		; a0 gets offset of first App tag
	move.l	#0,d2				; d2 gets highest LRU value
	bsr	NVRT_Read_Tag			; read the first tag
	bra.s	8$				; j to decode
;
; Loop thru all items and save pointer to largest LRU App/Item :
;
6$:	bsr.s	NVRT_Incr_To_Next_Tag		; read next tag
8$:	cmpi.b	#NVRT_App_Tag,d1		; is it App tag ?
	beq.s	10$				; if yes, j to handle
	cmpi.b	#NVRT_Item_Tag,d1		; is it Item tag ?
	beq.s	12$				; if yes, j to handle
	cmpi.b	#NVRT_Data_Tag,d1		; is it Data tag ?
	beq.s	6$				; if yes, j to continue
	cmpi.b	#NVRT_End_Tag,d1		; is it End tag ?
	beq.s	18$				; if yes, j to handle
	bra.s	30$				;
;
; App tag found :
;
10$:	move.l	a0,a3				; a3 holds current App offset
	bra.s	6$				; j to continue
;
; Item tag found :
;
12$:	move.l	a0,a4				; a4 gets current Item tag offset
	btst	#NVRT_LOCKB_ITEM,1(a1,a0.w)	; is this item locked ?
	bne.s	6$				; if yes, j to continue
	move.b	1(a1,a0.w),d1			; d1 gets current item's LRU
	andi.b	#NVRT_LRU_Mask,d1		; mask for LRU
	cmp.b	d1,d2				; is this larger LRU ?
	bgt.s	6$				; if no, j to continue
	move.l	d1,d2				; d2 gets current largest LRU value
	move.l	a3,d3				; d3 gets current largest App offset
	move.l	a4,d4				; d4 gets current largest Item offset
	bra.s	6$				; j to continue
;
; Largest LRU App/Item found, delete Item :
;
18$:	move.l	d4,a0				; a0 gets ptr to Item tag
	bsr	NVRT_Incr_To_Next_Tag		; inc to data tag
	bsr	NVRT_Incr_To_Next_Tag		; inc past data tag
	move.l	a0,d2				; d2 gets next tag offset
	sub.l	d4,d2				; d2 gets amount to shrink
	bsr	NVRT_Shrink_Data_Area		;
;
; Determine if deletion of App is required :
;
	move.l	d3,a0				; a0 gets ptr to App tag
	bsr	NVRT_Find_Next_Item_Tag		;
	beq.s	20$				; another item found, j to return
	move.l	a0,d2				;
	sub.l	d3,d2				;
	bsr	NVRT_Shrink_Data_Area		;
;
20$:	movem.l	(sp)+,d0-d7/a0-a6		; restore registers
	rts					; return to caller
;
30$:	bsr	NVRT_Tree_Recovery
	bra.s	20$



; *****************************************************************************
;
; The following subroutine will touch the current Item's LRU field and make
; its LRU value the lowest ( 0 ).
;
;	On Entry:	a1 = ptr to RAM data area
;			d4 = offset of current Item tag
;
;	On Exit:	nothing
;
;	Regs used:	none
;
NVRT_LRU_Touch:
	andi.b	#~NVRT_LRU_Mask,1(a1,d4.w)	; clear LRU value
	rts



; *****************************************************************************
;
; The following subroutine will update the LRU fields of all tags.  This update
; will mearly be adding one to all existing LRU fields of all tags.
;
;	On Entry:	a1 = ptr to RAM data area
;
;	On Exit:	nothing
;
;	Regs used:	none
;
NVRT_LRU_Update:

	movem.l	d0-d7/a0-a6,-(sp)		; save registers

	move.l	#NVRT_FIRST_APP_LOCN,a0		; a0 gets offset of first App tag
	bsr.s	NVRT_Read_Tag			; read the first tag
	bra.s	8$				; j to decode
;
; Loop thru tags and update LRU field of Item tags :
;
6$:	bsr	NVRT_Incr_To_Next_Tag		; read next tag
8$:	cmpi.b	#NVRT_App_Tag,d1		; is it App tag ?
	beq.s	6$				; if yes, j to continue
	cmpi.b	#NVRT_Item_Tag,d1		; is it Item tag ?
	beq.s	10$				; if yes, j to continue
	cmpi.b	#NVRT_Data_Tag,d1		; is it Data tag ?
	beq.s	6$				; if yes, j to continue
	cmpi.b	#NVRT_End_Tag,d1		; is it End tag ?
	beq.s	20$				; if yes, j to return
	bra.s	30$				; j to continue
;
; Item tag found :
;
10$:	move.b	1(a1,a0.w),d0			; d0 gets current item's LRU
	move.b	d0,d1				;
	andi.b	#NVRT_LRU_Mask,d0		; d0 gets only LRU value
	andi.b	#NVRT_LOCKF_ITEM,d1		; d1 gets only lock state
	addq.b	#1,d0				; increment LRU value
	cmpi.b	#NVRT_MAX_LRU,d0		; is it at max ?
	ble.s	12$				; if no, j to continue
	move.b	#NVRT_MAX_LRU,d0		; keep at max
12$:	or.b	d1,d0				; restore lock state
	move.b	d0,1(a1,a0.w)			; update LRU value
	bra.s	6$				; j to continue
;
20$:	movem.l	(sp)+,d0-d7/a0-a6		; restore registers
	rts					; return to caller
;
30$:	bsr.s	NVRT_Tree_Recovery
	bra.s	20$


; *****************************************************************************
;
; The following subroutine will read the tag byte at the current offset and
; return it split into length and type fields.
;
;	On Entry:	a0 = offset into RAM data area
;			a1 = base of RAM data area
;
;	On Exit:	a0 = unchanged
;			a1 = unchanged
;			d0 = length field of curr tag
;			d1 = tag field of curr tag
;
;	Regs used:	d0, d1
;
NVRT_Read_Tag:
	move.b  0(a1,a0.w),d0			; d0 gets first tag byte
	move.b	d0,d1				; d1 gets copy of tag
	andi.w	#NVRT_Length_Mask,d0		; d0 gets data length only
	andi.w	#NVRT_Tag_Mask,d1		; d1 gets tag only
	rts



; *****************************************************************************
;
; The following subroutine will move the existing data in the RAM data area
; up to account for the replacement of new data.  The new data is smaller than
; the existing data so the existing information must be crunched up to fit.
;
;
;	On Entry:	a0 = ptr to End tag
;			a1 = RAM data area base
;			d0 = offset of begining of data to stretch
;			d1 = how much to stretch
;
;	On Exit:	nothing
;
;	Regs used:	none
;
NVRT_Stretch_Data_Area:
	movem.l	d0-d7/a0-a6,-(sp)		; save registers
	move.l	a0,a2				; a2 gets destination offset
	add.l	d1,a2				;
	add.l	a1,a2				; a2 gets destination pointer
	addq.l	#1,a2				;
4$:	move.b	0(a1,a0.w),-(a2)		; move data
	subq.l	#1,a0				; adjust offset
	cmp.l	a0,d0				; at end ?
	ble.s	4$				; if no, j to continue
	movem.l	(sp)+,d0-d7/a0-a6		; restore registers
	rts



; *****************************************************************************
;
; The following subroutine will move the existing data in the RAM data area
; up to account for the replacement of new data.  The new data is smaller than
; the existing data so the existing information must be crunched up to fit.
;
;
;	On Entry:	a0 = ptr to begin of data to shrink
;			a1 = RAM data area base
;			d2 = how much to shrink
;
;	On Exit:	nothing
;
;	Regs used:	none
;
NVRT_Shrink_Data_Area:
	movem.l	d0-d7/a0-a6,-(sp)		; save registers
;
; Find the end tag by traversing the tree :
;
	move.l	a0,a4				; save a0
	bsr	NVRT_Find_Next_End_Tag		; a0 gets offset of end tag
	lea	0(a1,a0.w),a3			; a3 gets pointer to end tag
	move.l	a4,a0				; restore a0
;
; Calculate source and destination pointers :
;
	move.l	a0,a2				; a2 gets destination offset
	sub.l	d2,a2				;
	add.l	a1,a0				; a0 gets source pointer
	add.l	a1,a2				; a2 gets destination pointer
;
; Move data to overwrite shrink area :
;
4$:	move.b	(a0),(a2)+			; move data
	clr.b	(a0)+				; clear source
	cmp.l	a0,a3				; is source at the end tag ?
	bge.s	4$				; if no, j to continue
6$:	movem.l	(sp)+,d0-d7/a0-a6		; restore registers
	rts



; *****************************************************************************
;
; The following subroutine will attempt to recover as much of the NVRAM data
; as possible.  The caller has found an illegal tag while walking the NVRAM
; data.  It is only possible to save the data up to the point of failure.  Data
; following an illegal tag is not recoverable.
;
;
;	On Entry:	a1 = RAM data area base
;			a0 = offset of illegal tag
;
;	On Exit:	nothing
;
;	Regs used:	CCR
;
NVRT_Tree_Recovery:
	movem.l	d0-d7/a0-a6,-(sp)		; save registers

	move.l	#NVRT_FIRST_APP_LOCN,a0		; a0 gets offset of first App tag
	moveq.l	#2,d6				; init structure level flag
	move.l	a0,a3				; init App offset
	bsr	NVRT_Read_Tag			; read the first tag
	bra.s	8$				; j to decode
;
; Loop thru all tags :
;
6$:	bsr	NVRT_Incr_To_Next_Tag		; read next tag
8$:	cmpi.b	#NVRT_App_Tag,d1		; is it App tag ?
	beq.s	10$				; if yes, j to handle
	cmpi.b	#NVRT_Item_Tag,d1		; is it Item tag ?
	beq.s	12$				; if yes, j to continue
	cmpi.b	#NVRT_Data_Tag,d1		; is it Data tag ?
	beq.s	14$				; if yes, j to continue
	cmpi.b	#NVRT_End_Tag,d1		; is it End tag ?
	beq.s	16$				; if yes, j to handle
	bra.s	18$				;
;
; App tag found :
;
10$:	cmpi.w	#2,d6				; was last level correct ?
	bne.s	18$				; if no, j to error handler
	moveq.l	#1,d6				; set structure level
	move.l	a0,a3				; a3 gets ptr to last good locn
	bra.s	6$				; j to continue
;
; Item tag found :
;
12$:	cmpi.w	#3,d6				; was last level correct ?
	bge.s	18$				; if no, j to error handler
	moveq.l	#3,d6				; set structure level
	bra.s	6$				; j to continue
;
; Data tag found :
;
14$:	cmpi.w	#3,d6				; was last level correct ?
	bne.s	18$				; if no, j to error handler
	moveq.l	#2,d6				; set structure level
	bra.s	6$				; j to continue
;
; End tag found :
;
16$:	cmpi.w	#2,d6				; was last level correct ?
	beq.s	20$				; if yes, j to return
;
; Structure had error :
;	- delete everything after the App tag
;
18$:	move.b	#NVRT_End_Tag,0(a1,a3.w)	; write end marker to RAM copy area
	bsr	NVRT_Device_Write		; call to write buffer to device

20$:	movem.l	(sp)+,d0-d7/a0-a6		; restore registers
	rts



; *****************************************************************************
;
; The following subroutine will read the Nonvolatile library pointer to local
; RAM copy area and verify it is nonNull.  In addition it will read the first
; long word from the RAM copy area and verify it contains a proper key.  The
; result of the check will be returned in the CCR.
;
;
;	On Entry:	a5 = ptr to Nonvolatile library base
;
;	On Exit:	a1 = pointer to RAM data area
;			a0 = offset of first App tag
;			ZF = 0 if area is not valid
;			ZF = 1 if area is OK
;
;	Regs used:	a0, a1, CCR
;
NVRT_Verify_NVRAM_Tag:
	move.l	#NVRT_FIRST_APP_LOCN,a0		; a0 gets offset of first App tag
	move.l	nv_NVRAMCopy(a5),a1		; a1 gets RAM data area ptr
	cmpi.l	#NVRT_Init_Tag,(a1)		; is the init tag correct ?
	rts					;



; *****************************************************************************
;
; The following subroutine will trace through the NVRAM tree structure and
; verify all the tags are correct.
; The result of the check will be returned in the CCR.
;
;
;	On Entry:	a1 = ptr to RAM data area
;
;	On Exit:	ZF = 0 if area is not valid
;			ZF = 1 if area is OK
;
;	Regs used:	CCR
;
NVRT_Verify_NVRAM_Tree:
	movem.l	d0-d7/a0-a6,-(sp)		; save registers
	move.l	#NVRT_FIRST_APP_LOCN,a0		; a0 gets offset of first App tag
	bsr	NVRT_Read_Tag			; read the first tag
	bra.s	8$				; j to decode

6$:	bsr	NVRT_Incr_To_Next_Tag		; read next tag
8$:	cmpi.b	#NVRT_App_Tag,d1		; is it App tag ?
	beq.s	6$				; if yes, j to continue
	cmpi.b	#NVRT_Item_Tag,d1		; is it Item tag ?
	beq.s	6$				; if yes, j to continue
	cmpi.b	#NVRT_Data_Tag,d1		; is it Data tag ?
	beq.s	6$				; if yes, j to continue
	cmpi.b	#NVRT_End_Tag,d1		; is it End tag ?

20$:	movem.l	(sp)+,d0-d7/a0-a6		; restore registers
	rts



; *****************************************************************************
;
; The following subroutine will move the new data into the RAM data area.  The
; amount and structure for the data written to the buffer depends on the
; parameter passed in d1 as follows :
;
;	NVRT_DATA_REQ 		= Move only Data to RAM data area
;	NVRT_ITEM_DATA_REQ	= Move Item and Data to RAM data area
;	NVRT_APP_ITEM_DATA_REQ	= Move App, Item and Data to RAM data area
;
;
;	On Entry:	a1 = RAM data area base
;			a2 = ptr to requested Data
;			a3 = ptr to requested App name
;			a4 = ptr to requested Item name
;			d1 = amount of data to move parameter
;			d3 = offset of App or End tag
;			d4 = offset of Item tag
;			d5 = offset of Data tag
;			d6 = data length
;
;	On Exit:	a0 = offset of next position after write
;			a1 = unchanged
;
;	Regs used:	none
;
NVRT_Write_Data_To_Buffer:

	movem.l	d0-d7/a2-a6,-(sp)		; save registers
;
; Move data for App, Item, and Data :
;	- create an App tag
;	- move App string to buffer
;	- update App tag
;
	cmpi.l	#NVRT_APP_ITEM_DATA_REQ,d1	; move all ?
	bne.s	6$				; in no, j to continue
	move.l	d4,a0				; a0 gets offset for App

	move.l	a0,-(sp)			; save App tag location
	addq.w	#NVRT_App_Tag_Size,a0		; increment past App tag
	moveq.l	#NVRT_Max_String_Length-1,d0	; d0 gets max string length
2$:	tst.b	(a3)				; is src data NULL ?
	beq.s	4$				; if yes, j to continue
	move.b	(a3)+,0(a1,a0.w)		; move App string to buffer
	addq.w	#1,a0				; increment offset
	dbra.s	d0,2$				; copy entire string
4$:	addq.w	#1,d0				; force string count back to zero

	moveq.l	#NVRT_Max_String_Length,d1	; calculate string length
	sub.w	d0,d1				;
	ori.w	#NVRT_App_Tag,d1		; d1 gets App tag value
	move.l	(sp)+,a3			; a3 gets App tag offset
	move.b	d1,0(a1,a3.w)			; update App tag
	bra.s	8$				; j to continue
;
; Move data for Item, and Data :
;	- create an Item tag
;	- move Item string to buffer
;	- update Item tag
;
6$:	cmpi.l	#NVRT_ITEM_DATA_REQ,d1		; move item and data ?
	bne.s	14$				; in no, j to continue
	move.l	d4,a0				; a0 gets offset for Item

8$:	move.l	a0,-(sp)			; save Item tag location
	addq.w	#NVRT_Item_Tag_Size,a0		; increment past Item tag

	moveq.l	#NVRT_Max_String_Length-1,d0	; d0 gets max string length
10$:	tst.b	(a4)				; is src data NULL ?
	beq.s	12$				; if yes, j to continue
	move.b	(a4)+,0(a1,a0.w)		; move Item string to buffer
	addq.w	#1,a0				; increment offset
	dbra.s	d0,10$				; copy entire string
12$:	addq.w	#1,d0				; force string count back to zero

	moveq.l	#NVRT_Max_String_Length,d1	; calculate string length
	sub.w	d0,d1				;
	ori.w	#NVRT_Item_Tag,d1		; d1 gets Item tag value
	move.l	(sp)+,a3			; a3 gets Item tag offset
	move.b	d1,0(a1,a3.w)			; update Item tag
	clr.b	1(a1,a3.w)			; update LRU tag
	bra.s	16$				; j to continue
;
; Move data for Data :
;	- create a Data tag
;	- move data to buffer
;	- update Data tag
;
14$:	cmpi.l	#NVRT_DATA_REQ,d1		; move just data ?
	bne.s	20$				; in no, j to exit
	move.l	d5,a0				; a0 gets offset for data

16$:	move.l	d6,-(sp)			; save Data length
	ori.w	#NVRT_Data_Tag<<8,d6		; create Data tag
	move.w	d6,0(a1,a0.w)			;
	addq.w	#NVRT_Data_Tag_Size,a0		; increment past Data tag
	move.l	(sp)+,d6			; restore data length
	subq.l	#1,d6				; decrement for correct loop cnt

18$:	move.b	(a2)+,0(a1,a0.w)		; move data to buffer
	addq.w	#1,a0				; increment offset
	dbra.s	d6,18$				; copy entire string

20$:	movem.l	(sp)+,d0-d7/a2-a6		; restore registers
	rts


 ENDIF


	END
