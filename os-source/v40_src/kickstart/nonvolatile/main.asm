******************************************************************************
*
*	$Id: main.asm,v 40.30 93/09/14 10:57:03 darren Exp $
*
******************************************************************************
*
*	$Log:	main.asm,v $
* Revision 40.30  93/09/14  10:57:03  darren
* Update autodocs to indicate a DOS process is required for
* use (except as we know in the case of the nvram editor)
* 
* Revision 40.29  93/09/08  14:36:13  vertex
* Fixed background autodoc sample script
* 
* Revision 40.28  93/08/03  11:04:29  vertex
* Autodoc cleanup
*
* Revision 40.27  93/07/30  17:09:49  vertex
* Autodoc and include cleanup
*
* Revision 40.26  93/05/21  13:29:35  vertex
* Added check for name length in StoreNV
*
* Revision 40.25  93/05/06  14:08:08  brummer
* Add call to NVRAM function SetNVRAMMultiCommand in GetNVList().
*
* Revision 40.24  93/05/04  10:04:10  brummer
* Reformat GetNVInfo() AUTODOC.
*
* Revision 40.23  93/05/03  16:49:55  brummer
* Fix again the description of nvi_availstorage in GetNVInfo().
*
* Revision 40.22  93/04/21  12:30:16  brummer
* Fix documentation for CI176, nvi_freeStorage definition
*
* Revision 40.22  13/.1/.0  .2:.0:.2  brummer
* Add documentation for CI176, nvi_FreeStorage definition.
*
* Revision 40.21  93/04/07  16:47:34  brummer
* Fix to remove forced crash if user attempts to set nonused bits in DeleteNV() function.
*
* Revision 40.20  93/04/07  13:26:18  brummer
* Add comment to AUTODOC about rescan on GetNVInfo() or close/open.
*
* Revision 40.19  93/04/07  10:16:16  brummer
* Fix the getnvlist routine - the code was origonally checking for more Apps
* to do on a single App call.  The single App call does not have the App
* name in the allocated buffer, so the code was doing an invalid test for end
* Added a check for single App before checking the termination condition.
* Also, fixed some comments.
*
* Revision 40.18  93/03/31  11:51:22  brummer
* Fix to prevent writing NVRAM entry when disk is write protected.
* (related to disk/fileio.asm 40.7)
* Added AUTODOC background
*
* Revision 40.17  93/03/26  10:20:18  brummer
* Fix AUTODOCs per CATS request.
* Fix GetNVInfo() error that had MAX > AVAIL.
*
* Revision 40.16  93/03/18  10:55:49  brummer
* Replace 68020 instructions with generic 68000 code.
*
* Revision 40.15  93/03/16  11:25:03  brummer
* Fix register push/pop mismatch in setprotection.
*
* Revision 40.14  93/03/11  14:25:46  brummer
* Remove LLName definition.  This is fix related to nonvolatilebase.i
* 40.5
*
* Revision 40.13  93/03/09  14:02:46  brummer
* Fix autodocs to add invalid chars description.
* Fix GetNVList autodoc to not suggest it returns an array
* Make misc fixes suggested by Martit.
*
* Revision 40.12  93/03/05  15:02:24  brummer
* Update AUTODOCs.
*
* Revision 40.11  93/03/05  12:58:36  brummer
* Remove origonal method of disabling requesters and use the parameterized method instead.
* If the parameter is TRUE, a open of lowlevel.library will be done
* and a call will be made to KillReq() and RestoreReq()
*
* Revision 40.10  93/03/04  15:17:39  brummer
* Fix code to allow two NULL strings for App/Item access to
* reserved data.  Prevent access if either but not both are NULL.
*
* Revision 40.9  93/03/02  11:16:34  brummer
* Modified code to use already opened utility.library from nv.lib base.
* All changes are marked with an ## in the comment fields.
*
* Revision 40.8  93/02/26  17:15:45  brummer
* fix incorrect branch after writing NV disk
*
* Revision 40.7  93/02/26  12:20:17  Jim2
* Corrected error in parsing total NVRAM size in GetNVInfo.
* If there are no ItemNames for an AppName in GetNVList, do not
* list the AppName.
*
* Revision 40.6  93/02/25  19:29:25  brummer
* fix out of range branch because of previous fix
*
* Revision 40.5  93/02/25  18:53:07  brummer
* test d0 on return from WriteNVRAM calls
*
* Revision 40.4  93/02/25  08:55:54  Jim2
* Added StoreNV.
*
* Revision 40.3  93/02/23  13:17:28  Jim2
* GetNVList also talks in quantities of ten bits.  GetNVList
* builds an ExecList rather than returning an array of NVEntry(s).
*
* Revision 40.2  93/02/18  11:03:58  Jim2
* Added SetNVProtection.  The values returned by GetNVInfoare in
* tens of bytes, not single bytes.  The structure returned
* by GetNVList includes the protection which needs to be set
* to no protection for AppNames.
*
* Revision 40.1  93/02/16  13:47:59  Jim2
* Altered GetNVList to return the new data structure.
*
* Revision 40.0  93/02/09  10:23:35  Jim2
* GetNVList now sorts and combines the lists rather than depending
* on the NVRAM routines.
*
* Revision 39.0  93/02/03  11:07:48  Jim2
* Initial Release - Tested Disk functions.
*
*
*	(C) Copyright 1992,1993 Commodore-Amiga, Inc.
*	    All Rights Reserved
*
******************************************************************************

		INCLUDE	'exec/macros.i'
		INCLUDE	'exec/memory.i'

		INCLUDE	'dos/dosextens.i'

		INCLUDE	'nonvolatilebase.i'
		INCLUDE	'nonvolatile.i'
		INCLUDE 'NVRAM/nvramtree.i'

				;NVRAM/
		XREF	GetNVRAMInfo
		XREF	SizeNVRAMList
		XREF	GetNamesFromNVRAM
		XREF	SizeItemNVRAM
		XREF	SetNVRAMProtection
		XREF	WriteNVRAM
		XREF	SetNVRAMMultiCommand
		XREF	ClrNVRAMMultiCommand
				;Disk/GetDiskInfo.asm
		XREF	GetDiskInfo
				;Disk/SizeDiskList.asm
		XREF	SizeDiskList
				;Disk/MakeList.asm
		XREF	GetNamesFromDisk
		XREF	SizeItemDisk
				;Disk/SetNVRAMProtection.asm
		XREF	SetDiskProtection
				;Disk/FileIO.asm
		XREF	WriteDisk


		XDEF	GetNVInfo
		XDEF	GetNVList
		XDEF	SetNVProtection
		XDEF	StoreNV

******* nonvolatile.library/--background-- ****************************************
*
*   PURPOSE
*	The nonvolatile library provides a simple means for an application
*	developer to manage nonvolatile storage.
*
*   OVERVIEW
*	The nonvolatile library is meant to be used transparently across all
*	configurations. Currently, nonvolatile storage may consist of NVRAM
*	and/or disk devices. nonvolatile.library will automatically
*	access the best nonvolatile storage available in the system. Disk
*	based storage will be selected first and if not available, NVRAM
*	storage will be accessed.
*
*	* NVRAM
*
*	On low-end diskless Amiga platforms, NVRAM may be available. This
*	RAM will maintain its data contents when the system is powered down.
*	This is regardless of whether batteries or battery-backed clock are
*	present. The data stored in NVRAM is accessible only through the
*	ROM-based nonvolatile library funtion calls. The size of NVRAM
*	storage	is dependant on the system platform and is attainable through
*	the GetNVInfo() function.
*
*	* Disk
*
*	In keeping with the general configurability of the Amiga, the actual
*	disk location used by nonvolatile library when storing to disk may be
*	changed by the user.
*
*	The prefs directory is used on the Amiga for storing many user
*	configurable options. The location for nonvolatile disk storage
*	is contained in the file prefs/env-archive/sys/nv_location. This
*	file should contain a data string that specifies a lockable location.
*	If the string does not specify a lockable location, the file will
*	be ignored.
*
*	When opened, the nonvolatile library will search all drives within
*	the system until it finds this file and successfully accomplishes
*	a Lock on the location specified in the file. To force a rescan of
*	all drives, the library may be closed and reopened or execute the
*	GetNVInfo() function.
*
*	A simple method for creating a floppy disk for saving nonvolatile
*	data is the following:
*
*	Format a disk with the volume name NV
*	Create a file prefs/env-archive/sys/nv_location on this disk with
*	  the following contents:  NV:nonvolatile
*	Create a directory nonvolatile
*
*	The following is a script file that can be used to make a floppy
*	for use with nonvolatile library:
*
*	.KEY DRIVE/A,DISK
*	.BRA {
*	.KET }
*	format Drive {DRIVE} Name {DISK$NV} noicons ffs
*	makedir {DRIVE}prefs
*	makedir {DRIVE}nonvolatile
*	makedir {DRIVE}prefs/env-archive
*	makedir {DRIVE}prefs/env-archive/sys
*	echo {DISK$NV}:nonvolatile >{DRIVE}prefs/env-archive/sys/nv_location
*
*	!!!NOTE!!!
*
*	Because NVRAM performs disk access, you must open and use its
*	functionality from a DOS process, not an EXEC task.  Normally
*	your CDGS application is invoked as a DOS process so this
*	requirement generally should cause you no concern.  You just
*	need to be aware of this requirement should you create an
*	EXEC task and try to invoke nonvolatile.library from that task.
*
*******************************************************************************



******* nonvolatile.library/GetNVInfo ****************************************
*
*   NAME
*	GetNVInfo -- report information on the current nonvolatile storage.
*		     (V40)
*
*   SYNOPSIS
*	information = GetNVInfo(killRequesters);
*	D0			D1
*
*	struct NVInfo *GetNVInfo(BOOL);
*
*   FUNCTION
*	Finds the user's preferred nonvolatile device and reports information
*	about it.
*
*   INPUTS
*	killRequesters - suppress system requesters flag. TRUE if all system
*			 requesters are to be suppressed during this function.
*			 FALSE if system requesters are allowed.
*
*   RESULT
*	information - pointer to an NVInfo structure. This structure contains
*		      information on the NV storage media with the largest
*		      storage. The structure contains 2 longword fields:
*		      nvi_MaxStorage and nvi_FreeStorage. Both values are
*		      rounded down to the nearest ten. The nvi_MaxStorage
*		      field is defined as the total amount of nonvolatile
*		      storage available on this device. The nvi_FreeStorage is
*		      defined as the amount of available space for NVDISK or
*		      the amount of non-locked storage for NVRAM. For NVDISK,
*		      the nvi_FreeStorage takes into account the amount of
*		      overhead room required to store a new App/Item. This
*		      amount is 3 blocks to allow room for storing a new Item
*		      file and possibly a new App directory. For NVRAM, the
*		      amount of overhead is 5 bytes. However, the amount of
*		      room required to store a new NVRAM item depends on the
*		      length of the App and Item names. Refer to StoreNV()
*		      function for storage details.
*
*		      This function may return NULL in the case of failure.
*
*   SEE ALSO
*	FreeNVData(), StoreNV(), <libraries/nonvolatile.h>
*
******************************************************************************
GetNVInfo
		movem.l	a5/a6,-(sp)
		move.l	a6,a5
		move.l	nv_ExecBase(a5),a6

		ChkKillRequesters		; ###

		move.l	#NVINFO_SIZE+4,d0
		move.l	#MEMF_PUBLIC|MEMF_CLEAR,d1
		JSRLIB	AllocMem		;Allocate space for the struct NVInfo.
		tst.l	d0			;If we can't get the space exit.
		beq.s	gnvi_Exit
				;Got the memory.
		move.l	a4,-(sp)
		move.l	d0,a4
		move.l	#NVINFO_SIZE+4,(a4)+	;Store the size of the allocated memory in the first long word.
		addq.l	#4,d0
		move.l	a5,a6			;Get base in a6 for GetDiskInfo.
		bsr	GetDiskInfo		;Get information for the disk.
		sub.l	#NVINFO_SIZE,sp		;Get another struct NVInfo off the stack.
		move.l	sp,d0
		bsr	GetNVRAMInfo		;Get information for the NVRAM chip.
		move.l	(sp),d0
		cmp.l	(a4),d0			;Is there more space on disk, or NVRAM.
		bmi.s	MoreOnDisk
				;More space in NVRAM.
		move.l	a4,a6			;Reset the pointers to the start of the data.
		move.w	#(NVINFO_SIZE/4)-1,d0	;This many long words to copy.
gnvi_CopyLoop:	move.l	(sp)+,(a6)+
		dbf	d0,gnvi_CopyLoop
		bra.s	Leave			;We are ready to leave.
				;More space on disk.
MoreOnDisk:	add.l	#NVINFO_SIZE,sp		;Clear away NVInfo structure from the stack.

Leave:
		move.l	a4,-(sp)
		move.l	nv_UTILBase(a5),a6	; a6 gets utility.library base
		move.l	(a4),d0
		moveq.l	#10,d1
		JSRLIB	UDivMod32
		move.l	d0,(a4)+

		move.l	(a4),d0
		moveq.l	#10,d1
		JSRLIB	UDivMod32
		move.l	d0,(a4)+
		move.l	(sp)+,d0

		move.l	(sp)+,a4
gnvi_Exit:
		ChkRestoreRequesters		; ###

		movem.l	(sp)+,a5/a6
		rts

******* nonvolatile.library/GetNVList ****************************************
*
*   NAME
*	GetNVList -- return a list of the items stored in nonvolatile
*		     storage. (V40)
*
*   SYNOPSIS
*	list = GetNVList(appName, killRequesters);
*	D0		 A0	  D1
*
*	struct MinList *GetNVList(STRPTR, BOOL);
*
*   FUNCTION
*	Returns a pointer to an Exec list of nonvolatile Items associated
*	with the appName requested.
*
*	The string appName may not contain the '/' or ':' characters.
*	It is recommended that these characters be blocked from user input
*	when requesting an appName string.
*
*   INPUTS
*	appName - NULL terminated string indicating the application name
*		  to be matched. Maximum length is 31.
*	killRequesters - Suppress system requesters flag. TRUE if all system
*			 requesters are to be suppressed during this function.
*			 FALSE if system requesters are allowed.
*
*   RESULT
*	list - a pointer to an Exec MinList of NVEntries. A NULL will be
*	       returned if there is insufficient memory. If there are no
*	       entries in the nonvolatile storage for the AppName, an
*	       empty list will be returned.
*
*   NOTE
*	The protection field contains more bits than are required for
*	storing the delete protection status. These bits are reserved
*	for other system usage and may not be zero. When checking for
*	the delete status use either the field mask NVIF_DELETE, or the
*	bit definition NVIB_DELETE.
*
*   SEE ALSO
*	FreeNVData(), SetNVProtection()
*
******************************************************************************
*
*	Memory space is allocated for all of the elements and all of the
*	strings. The node are located at the start of allocated memory.
*	The strings are therefore located at the end of allocated memory.
*
*	However, the strings can be found on either NVRAM, or disk, or both.
*	First the disk then the NVRAM is queried for the strings. Then the
*	two lists are combined, droping any duplicates. This is done without
*	requiring any scratch memory.  None is requried since the memory
*	allocation is enough to hold every string without need to drop any.
*	So, the two lists are placed in memory following the last utilized
*	array entry.  The list is searched from last to first, and placed
*	in front of all pevious lists.  Then the nodes are created
*	for each of these strings.
*
*	It sounds more confusing than it is.
GetNVList
		movem.l	d1-d7/a1-a6,-(sp)	; Place the AppName pointer on the stack.
		move.l	a6,a5			; a5 gets NVBase
		move.l	a0,a4			; a4 gets AppName pointer
		move.l	a0,d6			; d6 gets AppName pointer for single App test
		move.l	nv_ExecBase(a5),a6	; a6 gets ExecBase
;
; Kill requester based on parameter in d1 :
;
		ChkKillRequesters		;
;
; Set NVRAM multi-command option :
;
		bsr	SetNVRAMMultiCommand	; ##
;
; Determine size of requested data and attempt to allocated memory :
;
		bsr	SizeNVRAMList		; d0 gets NVRAM size requirement
		move.l	d0,-(sp)		; save on stack
		bsr	SizeDiskList		; d0 gets NVDISK size requirements
		add.l	(sp)+,d0		; d0 gets sum of size required from NVRAM and disk
		add.l	#4+MLH_SIZE,d0		;Add a long word to remember the size.  Plus the MinList
		move.l	d0,-(sp)		;Remember this size temporarily
		move.l	#MEMF_PUBLIC|MEMF_CLEAR,d1
		JSRLIB	AllocMem
		move.l	(sp)+,d1		;Remove the size from the stack.
		move.l	d0,-(sp)
		beq	gnvl_Close
;
; Memory allocated, build list head node :
;
		movem.l	d2-d4/a2-a3,-(sp)	; save registers
		move.l	d0,a3
		move.l	d1,(a3)+		;Store the size of memory allocated
		move.l	a3,4*5(sp)		;Remember our return parameter.
		NEWLIST	a3
		add.l	#MLH_SIZE,a3
		move.l	a3,a2
		add.l	-(4+MLH_SIZE)(a3),a3
		sub.l	#(5+MLH_SIZE),a3	;Get pointer to the last byte of the allocation.
		move.l	a3,d3			;Set up termination in the case of a single application.
		cmp.l	a2,a3
		blt	gnvl_Cleanup		;Was there any space allocated for data?
;
; Determine if list is for all Apps or just one :
;
		cmp.l	#0,a4			; is AppName pointer = NULL ?
		bne.s	GetItems
;
; Request is for all Apps :
;
GetApps:	move.l	a2,d2			;Remember the start of memory used.
		move.l	a3,d3			;Remember the end of allocated memory.
		addq.l	#1,a2			;Place a NULL at the start of the list of applications.
		bsr	GetNamesFromDisk
		bsr	GetNamesFromNVRAM
		bsr	Merge
		move.l	a3,a4			;Point a4 to the first AppName to be processed.
		move.l	d2,a2			;Reset a2 to start of memory.
;
;Reset the end of available scratch memory.  a3
;currently points to the first character of useful
;stringns at the end of the allocated memory block.
;
AddAppNode:	move.l	a4,nve_Name(a2)
		move.l	#0,nve_Size(a2)
		move.l	#NVEF_APPNAME,nve_Protection(a2)
		move.l	a2,a1
		add.l	#NVENTRY_SIZE,a2
		move.l	4*5(sp),a0
		ADDTAIL
		subq.l	#1,a3			;Leave a NULL between the lists of ItemNames.
;
; Get Items associated to AppName :
;
GetItems:	move.l	a2,d2			;Remember where the list of ItemNames is to start.
		move.b	#0,(a2)+		;For Merge to work, there must be a NULL at the start of the list.
		bsr	GetNamesFromDisk
		bsr	GetNamesFromNVRAM
		lea	-1(a2),a0
		cmp.l	a0,d2
		bne.s	ItemsExist
;
; No items exist for the current App, remove the AppName node from the list :
;
		sub.l	#NVENTRY_SIZE+1,a2
		move.l	a2,a1
		move.l	4*5(sp),a0
		REMTAIL
		bra.s	SkipToNext
;
; Items exist for the current App :
;	- call merge to get rid of duplicates
;	- for each item in merged list, create an Item list node
;
ItemsExist:	move.l	a3,d4			;Remember the end of memory for the termination of creating elements.
		bsr	Merge
		move.l	a3,-(sp)		;Remember the top of memory used for the ItemName strings.
AddItem:	cmp.l	d4,a3
		bge.s	DoneItems		;A top testing loop, in case of the pathalogical case of no Items.
				;Create a node for this Items.
		move.l	a2,a1
		move.l	4*6(sp),a0
		ADDTAIL
		move.l	a3,nve_Name(a2)
		move.l	#0,nve_Size(a2)		;Initialize the size to zero.
		move.l	#0,nve_Protection(a2)
		add.l	#nve_Size,a2
		bsr	SizeItemDisk
		bsr	SizeItemNVRAM
		move.l	(a2),d1
		add.l	#9,d1			;Ceiling the size to ten byte units.
		divu.w	#10,d1
		andi.l	#$0FFFF,d1		; d1 gets word size operand
		move.l	d1,(a2)
		add.l	#NVENTRY_SIZE-nve_Size,a2	;Advance the to the next array element.
NextItem:	tst.b	(a3)+			;Skip along the string to the NULL.
		bne.s	NextItem
				;Got to the NULL.  a3 actually points to the next non-NULL.
		bra.s	AddItem
				;Processed all of the items for this application.
DoneItems	move.l	(sp)+,a3		;Restore the pointer to the end of 'scratch' memory.
;
; Determine if more there are more Apps to do :
;
SkipToNext:	tst.l	d6			; was this a single App request ?
		bne.s	gnvl_Cleanup		; if yes, j to exit
		tst.b	(a4)+			; find end of this App (actually begining of next App)
		bne.s	SkipToNext		;
		cmp.l	a4,d3			; is this the last App (end of allocated memory) ?
		bgt	AddAppNode		; if no, j to do next App
;
; Cleanup and exit :
;
gnvl_Cleanup:	movem.l	(sp)+,d2-d4/a2-a3	;Restore the scratch registers.
gnvl_Close:	move.l	(sp)+,d0		;Get the return value.
gnvl_Exit:
;
; Clear NVRAM multi-command option :
;
		bsr	ClrNVRAMMultiCommand	; ##

		ChkRestoreRequesters		; restore requesters if necessary
		movem.l	(sp)+,d1-d7/a1-a6	;Clear stack and restore last two scratch registers.
		rts




*****l* main.asm/Merge *******************************************************
*
*   NAME
*	Merge - Merges a lists of string with possible duplicates.
*
*   SYNOPSIS
*	start = Merge (Start, End, EndOfMergeBlock, NVBase)
*	A3	       D2     A2   A3		    a5
*
*   FUNCTION
*	Takes a list of strings and removes duplicates.
*
*	This function assumes the input list and output list occupy
*	overlapping areas of memory.  Also, this routine allows for
*	several copies of the data.
*
*   INPUTS
*	Start - Pointer the NULL that is located at the start of the list.
*	End - Pointer beyond the NULL that terminates the last string.
*	EndOfMergeBlock - Pointer beyond the last byte in the memory block
*			  to contain the merged list.
*
*   RESULT
*	start - Pointer to the first non-NULL in the merged list.
*
*   NOTE
*	The value in a2 is lost.
*
*
******************************************************************************
Merge
		movem.l	d2-d3/a4/a6,-(sp)	; ##
		move.l	nv_UTILBase(a5),a6	; ## get utility.library base
		move.b	#0,d3			;Need a NULL.
		move.b	d3,(a3)			;Make sure the merged list ends with a NULL.
GetNext:	cmp.l	d2,a2			;Have we searched the whole list?
		beq.s	AllDone
				;Still looking.  Skip NULLs
		tst.b	-(a2)
		beq.s	GetNext			;The current byte is NULL, check is it the last?
				;Ok got a new string.
		move.b	(a2),-(a3)
Coping:		move.b	-(a2),-(a3)
		bne.s	Coping			;Copy through the terminating NULL.
				;Copied the string.
		move.l	d2,a4			;Walk looking for duplicates from here.
CheckingForDup:	cmp.l	a2,a4
		beq.s	GetNext			;The complete list has been checked.

		tst.b	(a4)+
		beq.s	CheckingForDup		;Skip over NULLs.
				;At the start of a new string
		subq	#1,a4
		move.l	a4,a0
		lea	1(a3),a1
		JSRLIB	Stricmp
		beq.s	ClearWord
				;Not a duplicate skip to the end.
NULLLoop:	tst.b	(a4)+
		bne.s	NULLLoop

		subq.l	#1,a4
		bra.s	CheckingForDup
				;Found a duplicate word.  Clear it from the original list.
ClearWord:	move.b	d3,(a4)+
		tst.b	(a4)
		bne.s	ClearWord

		bra.s	CheckingForDup

AllDone:	addq.l	#1,a3			;Point a3 at the first non-NULL.
		movem.l	(sp)+,d2-d3/a4/a6	; ##
		rts



******* nonvolatile.library/SetNVProtection **********************************
*
*   NAME
*	SetNVProtection -- set the protection flags. (V40)
*
*   SYNOPSIS
*	success = SetNVProtection(appName, itemName, mask, killRequesters);
*	D0			  A0	   A1	     D2    D1
*
*	BOOL SetNVProtection(STRPTR, STRPTR, LONG, BOOL);
*
*   FUNCTION
*	Sets the protection attributes for an item currently in the
*	nonvolatile storage.
*
*	Although 'mask' is LONG only the delete bit, NVEF_DELETE/NVEB_DELETE,
*	may be set. If any other bits are set this function will return
*	FALSE.
*
*	The strings appName and itemName may not contain the '/' or ':'
*	characters. It is recommended that these characters be blocked
*	from user input when requesting AppName and ItemName strings.
*
*   INPUTS
*	appName - NULL terminated string indicating the application name
*		  to be matched. Maximum length is 31.
*	itemName - NULL terminated string indicated the item within the
*		   application to be found. Maximum length is 31.
*	mask - the new protection mask. Only set the delete bit otherwise
*	       this function WILL CRASH.
*	killRequesters - suppress system requesters flag. TRUE if all system
*			 requesters are to be suppressed during this function.
*			 FALSE if system requesters are allowed.
*
*   RESULT
*	success - FALSE if the protection could not be change (ie the data
*		  does not exist).
*
*   SEE ALSO
*	GetNVList(), <libraries/nonvolatile.h>
*
******************************************************************************
SetNVProtection
		move.l	d2,d0			; d0 gets mask
		and.l	#~NVEF_DELETE,d0	; any other bits set ?
		beq.s	Ok			; if no, j to continue
		moveq.l	#0,d0			; d0 gets FALSE
		rts				; return
;
; User is changing only delete bit, save and init registers :
;
Ok		movem.l	d3/a3-a6,-(sp)		;Place the AppName pointer on the stack.
		move.l	a6,a5			; a5 gets nonvolatile.library base
		move.l	a0,a3			; a3 gets ptr to App
		move.l	a1,a4			; a4 gets ptr to Item
;
; Kill requesters based on parameter :
;
		ChkKillRequesters		; ###

		CLEAR	d3
		bsr	SetDiskProtection
		bsr	SetNVRAMProtection
		move.l	d3,d0

		ChkRestoreRequesters		; ###

		movem.l	(sp)+,d3/a3-a6		;Clear stack and restore last two scratch registers.
		rts




******* nonvolatile.library/StoreNV ******************************************
*
*   NAME
*	StoreNV -- store data in nonvolatile storage. (V40)
*
*   SYNOPSIS
*	error = StoreNV(appName, itemName, data, length, killRequesters);
*	D0		A0	 A1	   A2    D0	 D1
*
*	UWORD StoreNV(STRPTR, STRPTR, APTR, ULONG, BOOL);
*
*   FUNCTION
*	Saves some data in nonvolatile storage. The data is tagged with
*	AppName and ItemName so it can be retrieved later. No single
*	item should be larger than one fourth of the maximum storage as
*	returned by GetNVInfo().
*
*	There is no data compression associated with this function.
*
*	The strings, AppName and ItemName, should be short, but descriptive.
*	They need to be short since the string is stored with the data and
*	the nonvolatile storage for a stand alone game system is limited.
*	The game system allows the user to selectively remove entries from
*	storage, so the string should be desriptive.
*
*	The strings AppName and ItemName may not contain the '/' or ':'
*	characters. It is recommended that these characters be blocked
*	from user input when requesting AppName and ItemName strings.
*
*   INPUTS
*	appName - NULL terminated string identifying the application
*		  creating the data. Maximum length is 31.
*	itemName - NULL terminated string uniquely identifying the data
*		   within the application. Maximum length is 31.
*	data - pointer to the memory block to be stored.
*	length - number of bytes to be stored in the units of tens of
*		 bytes. For example, if you have 23 bytes to store length = 3;
*		 147 byte then length = 15.
*	killRequesters - suppress system requesters flag. TRUE if all system
*			 requesters are to be suppressed during this function.
*			 FALSE if system requesters are allowed.
*
*   RESULT
*	error - 0                means no error,
*	        NVERR_BADNAME    error in AppName, or ItemName.
*	        NVERR_WRITEPROT  Nonvolatile storage is read only.
*	        NVERR_FAIL       Failure in writing data (nonvolatile storage
*				 full, or write protected).
*	        NVERR_FATAL      Fatal error when accessing nonvolatile
*				 storage, possible loss of previously saved
*				 nonvolatile data.
*
*   SEE ALSO
*	GetCopyNV(), GetNVInfo()
*
******************************************************************************
StoreNV
		movem.l	d2/a3-a6,-(sp)
		move.l	a6,a5			;Save NVBase.
		move.l	a0,a3			;Save AppName.
		move.l	a1,a4			;Save ItemName

		andi.l	#$0FFFF,d0		; d0 gets word operand length
		move.l	d0,d2			;
		mulu.w	#10,d2			; Length is in units of tens of bytes.
;
; Kill all task based requesters based on parameter :
;
		ChkKillRequesters		; ###
;
; Check for NULL App/Item name pointers :
;
		move.l	#NVERR_BADNAME,d0
		move.l	a3,d1
;		tst.l	a3
		beq	sn_Exit			;Null AppName is wrong.
		move.l	a4,d1
;		tst.l	a4
		beq	sn_Exit			;Null ItemName is wrong.
;
; Check for NULL App/Item name strings (allow both NULL for RSVD data) :
;
		tst.b	(a3)			; is App string NULL ?
		bne.s	4$			; if no, j to continue
		tst.b	(a4)			; is Item string also NULL ?
		beq.s	NamesOK			; if yes, j to OK
		bra	sn_Exit			; j to error return
4$:		tst.b	(a4)			; is Item string nonNULL ?
		beq	sn_Exit			; if no, j to error return
;
; Parse APP name for a few invalid chars :
;
		move.l	a3,a0
		moveq	#0,d1		; to count the length

				;Neither :'s nor /'s are allowed in the AppName
CheckAppName:	cmp.b	#'/',(a0)
		beq	sn_Exit

		cmp.b	#':',(a0)+
		beq	sn_Exit

		addq.l	#1,d1		; length counter

		tst.b	(a0)
		bne.s	CheckAppName

		cmp.l	#NVRT_Max_String_Length,d1
		bgt.s	sn_Exit

;
; Parse Item name for a few invalid chars :
;
		move.l	a4,a0
		moveq	#0,d1		; length counter
				;Neither :'s nor /'s are allowed in the AppName
CheckItemName:	cmp.b	#'/',(a0)
		beq.s	sn_Exit

		cmp.b	#':',(a0)+
		beq.s	sn_Exit

		addq.l	#1,d1		; length counter

		tst.b	(a0)
		bne.s	CheckItemName

		cmp.l	#NVRT_Max_String_Length,d1
		bgt.s	sn_Exit
;
; Names OK, do store command :
;
NamesOK:	move.l	a5,a6			;The subroutines all expect NVBase in a6.
		move.l	a4,a1			;ItemName
		move.l	a3,a0			;AppName
		move.l	#MODE_OLDFILE,d1
		move.l	d2,d0			;Length
		bsr	WriteDisk
		tst.l	d0
				;
				;Zero means success so we can exit.  If the high
				;order bit is set it means the files was found,
				;but write protected.  There is no fall back
				;to NVRAM on this occurrance, only if the disk
				;or file is write protected.
				;
		ble.s	sn_Exit

		cmp.l	#NVERR_BADNAME,d0
		beq.s	sn_Exit			;If DOS did not like the name, don't give the NVRAM a chance.

		move.l	a4,a1
		move.l	a3,a0
		move.l	#MODE_OLDFILE,d1
		move.l	d2,d0
		bsr	WriteNVRAM
		tst.l	d0
		beq.s	sn_Exit
				;Updating the file was not possible, try creating it.
		move.l	a4,a1
		move.l	a3,a0
		move.l	#MODE_NEWFILE,d1
		move.l	d2,d0
		bsr	WriteDisk
		tst.l	d0
		ble.s	sn_Exit			; if write is OK, j to return

		move.l	a4,a1
		move.l	a3,a0
		move.l	#MODE_NEWFILE,d1
		exg.l	d2,d0			;Remember the disk failure if NVRAM fails it probably is not present.
		bsr	WriteNVRAM
		tst.l	d0
		beq.s	sn_Exit
;
; NVRAM failed restore disk failure.
;
		exg.l	d2,d0
sn_Exit:	bclr.l	#31,d0				; Clear the high bit.
		ChkRestoreRequesters			; restore requesters

		movem.l	(sp)+,d2/a3-a6
		rts



		end
