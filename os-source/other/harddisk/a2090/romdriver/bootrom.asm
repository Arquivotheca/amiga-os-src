*******************************************************************************
*
*	Source Control
*	--------------
*	$Header: bootrom.asm,v 34.31 88/04/03 18:38:31 bart Exp $
*
*	$Locker: bart $
*
*	$Log:	bootrom.asm,v $
*   Revision 34.31  88/04/03  18:38:31  bart
*   return null in d0 if executed directly from disk
*   
*   Revision 34.30  88/04/02  13:46:03  bart
*   overscan support
*   
*   Revision 34.29  88/02/23  11:45:03  bart
*   *   ds.b    $8000   ; not necessary for disk based code
*   *                   ; or if we're burning 2764's
*   
*   Revision 34.28  88/01/21  18:04:35  bart
*   compatible with disk based / binddriver useage
*   
*   Revision 34.27  87/12/04  19:13:27  bart
*   checkpoint
*   
*   Revision 34.26  87/12/04  12:08:00  bart
*   checkpoint before adding check for existing dosname on eb_mountlist
*   
*   Revision 34.25  87/10/26  16:31:06  bart
*   checkpoint
*   
*   Revision 34.24  87/10/25  12:59:59  bart
*   yes, virginia... we need to dc.b 8000 bytes if we are burning 27256 roms
*   
*   Revision 34.23  87/10/15  09:42:55  bart
*   *   ds.b    $8000   ; not necessary -- bart
*   
*   Revision 34.22  87/10/15  09:34:04  bart
*   10-13 rev 1
*   
*   Revision 34.21  87/10/14  14:15:31  bart
*   beginning update to cbm-source.10.13.87
*   
*   Revision 34.20  87/07/08  14:00:31  bart
*   y
*   
*   Revision 34.19  87/06/12  10:51:22  bart
*   removed references to romboot_base... not necessary anymore!
*   
*   Revision 34.18  87/06/11  15:47:20  bart
*   working autoboot 06.11.87 bart
*   
*   Revision 34.17  87/06/11  15:36:06  bart
*   ok
*   
*   Revision 34.16  87/06/10  19:26:43  bart
*   JSR     (A0)            ;   rock 'n roll !!!
*   
*   Revision 34.15  87/06/10  17:54:23  bart
*   *** empty log message ***
*   
*   Revision 34.14  87/06/10  10:05:30  bart
*   pass address of ConfigDev to Init
*   
*   Revision 34.13  87/06/03  13:05:41  bart
*   AutoBoot
*   
*   Revision 34.12  87/06/03  10:58:41  bart
*   checkpoint
*   
*   Revision 34.11  87/05/31  18:31:33  bart
*   ;------ we'll need to find our copy of DiagArea at AutoBoot time
*   LEA     cd_Rom+er_Reserved0c(A3),A1
*   MOVE.L  A2,(A1)
*   
*   Revision 34.10  87/05/31  18:20:33  bart
*   stuff romtag pointer in cd_Driver
*   
*   Revision 34.9  87/05/31  16:35:23  bart
*   chickpoint
*   
*   Revision 34.8  87/05/29  19:38:59  bart
*   checkpoint
*   
*   Revision 34.7  87/05/29  17:52:02  bart
*   *** empty log message ***
*   
*   Revision 34.6  87/05/28  20:02:12  bart
*   *** empty log message ***
*   
*   Revision 34.5  87/05/28  19:56:39  bart
*   *** empty log message ***
*   
*   Revision 34.4  87/05/28  18:37:15  bart
*   checkpoint
*   
*   Revision 34.3  87/05/28  16:20:13  bart
*   *** empty log message ***
*   
*   Revision 34.2  87/05/28  16:16:44  bart
*   *** empty log message ***
*   
*   Revision 34.1  87/05/28  15:34:18  bart
*   *** empty log message ***
*   
*   Revision 34.0  87/05/28  15:31:30  bart
*   added to rcs for updating
*   
*
*******************************************************************************

	SECTION	section

	INCLUDE	"bootrom.i"
	INCLUDE	"bootrom_rev.i"
	INCLUDE	"hddisk.i"
	INCLUDE	"libraries/expansion.i"



	XDEF	_HDBootBlock
	XDEF	hdName
	XDEF	Rom_DiagArea
	XDEF	ExLibName
	XDEF	IntuitLibName
	XDEF	AutoBoot_Dosname
	XDEF	VERNUM
	XDEF	REVNUM

	XREF	Init
	XREF	_LVOFindResident

Boot_Rom:

	MOVEQ.L	#0,D0
	RTS

 	ds.b	$7FFC	; not necessary for disk based code

Rom_DiagArea:	

	dc.b	(DAC_WORDWIDE+DAC_CONFIGTIME)
	dc.b	(NULL)
	dc.w	(EndCode-Rom_DiagArea)
	dc.w	(Rom_Diagnostics-Rom_DiagArea)
	dc.w	(AutoBoot_Vector-Rom_DiagArea)
	dc.w	(Boot_Name-Rom_DiagArea)
	dc.w	(NULL)
	dc.w	(NULL)

_HDBootBlock:	

	DC.L    ID_DOS_DISK ; 4 character identifier

						; for the purposes FindRomTag,
						; the checksum field will double as SegSize
						; and the DOS patch field as NextSeg
SegSize:

	DC.L	$00000000	; boot block checksum (balance)

NextSeg:

	DC.L	$00000000   ; reserved for DOS patch

AutoBoot_Vector:

	MOVE.L	D0,-(SP)		; save d0

	LEA	AutoBoot_Dosname(PC),A1

	JSR		_LVOFindResident(A6)	; initialize the device driver

	TST.L	D0
	BEQ.S	AutoBoot_NoDos

	;------ and let the strap call the dos

	MOVE.L	D0,A0
	MOVE.L	RT_INIT(A0),A0	; set bootstrap vector

	JSR		(A0)			;	rock 'n roll !!!

AutoBoot_Return:

	MOVE.L	(SP)+,D0		; restore d0
	RTS

AutoBoot_NoDos:
	MOVE.L	D0,A0			; clear bootstrap vector
	BRA.S	AutoBoot_Return

	CNOP    0,4 ; Make sure string is long word aligned

AutoBoot_Dosname:
	DOSNAME

	CNOP    0,4 ; Make sure string is long word aligned

Boot_Name:

	BOOT_NAME

	DS.W	0	; Make sure tag is word aligned

initDDescrip:					;STRUCTURE RT,0
	DC.W	RTC_MATCHWORD		; UWORD RT_MATCHWORD
rt_MatchTag:
	DC.L	initDDescrip		; APTR	RT_MATCHTAG
rt_EndCode:
	DC.L	EndCode				; APTR	RT_ENDSKIP
	DC.B	RTW_COLDSTART		; UBYTE RT_FLAGS
	DC.B	VERSION				; UBYTE RT_VERSION
	DC.B	NT_DEVICE			; UBYTE RT_TYPE
	DC.B	20					; BYTE	RT_PRI
rt_Name:
	DC.L	hdName				; APTR  RT_NAME
rt_IDString:
	DC.L	hdIDString			; APTR	RT_IDSTRING
rt_Init:
	DC.L	Init				; APTR	RT_INIT == long offset from ROM base
								; LABEL RT_SIZE

	CNOP    0,4 ; Make sure string is long word aligned
ExLibName	EXPANSIONNAME	; Expansion Library Name

	CNOP    0,4 ; Make sure string is long word aligned
IntuitLibName	
		DC.B	'intuition.library',0
		DS.W	0

	CNOP    0,4 ; Make sure string is long word aligned
hdName:
		HD_NAME

*		;------ our name identification string
	CNOP    0,4 ; Make sure string is long word aligned
hdIDString:	VSTRING

VERNUM:		EQU	VERSION

REVNUM		EQU	REVISION

Rom_Diagnostics: 			; 	config time entry point

;	upon entry
;	d0 == 0
;	a0 == pointer to boardbase
;	a2 == pointer to copy of diagarea 
;	a3 == pointer to configdev structure for this board

	MOVEM.L	A1,-(SP)

	; must resolve references in romtag if loaded as raw memory image

	LEA		rt_Init(PC),A1 		; in order to execute out of ROM
	MOVE.L	(A1),D0				; get long offset from ROM origin
	ADD.L	A0,D0				; add the boardbase to the offset
	MOVE.L	D0,(A1)				; and replace offset with absolute

	LEA		hdName(PC),A0
	LEA		rt_Name(PC),A1
	MOVE.L	A0,(A1)

	LEA		initDDescrip(PC),A0
	LEA		rt_MatchTag(PC),A1
	MOVE.L	A0,(A1)

	LEA		hdIDString(PC),A0
	LEA		rt_IDString(PC),A1
	MOVE.L	A0,(A1)

	LEA		EndCode(PC),A0
	LEA		rt_EndCode(PC),A1
	MOVE.L	A0,(A1)

	;------ now imitate a Segment List

	LEA.L	SegSize(PC),A1		; address of segment size variable
	SUBA.L	A1,A0				; calculate byte "segment" size
	MOVE.L	A0,D0
	LSR.L	#2,D0				; adjust to word size
	MOVE.L	D0,(A1)				; and store in "segment" size variable

	LEA.L	NextSeg(PC),A1		; address of "next segment" pointer 
	MOVEQ	#0,D0
	MOVE.L	D0,(A1)				; no "next segment"

	;------ we'll need to find our copy of DiagArea at AutoBoot time

	LEA		cd_Rom+er_Reserved0c(A3),A1
	MOVE.L	A2,(A1)

	;------ now let AutoBoot bring us up by the bootstraps

	MOVEM.L	(SP)+,A1

	MOVEQ.L	#1,D0				; indicate "success"
	RTS

EndCode:

	DC.L	0

	END
