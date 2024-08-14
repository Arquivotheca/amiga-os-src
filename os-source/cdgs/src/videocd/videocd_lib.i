	IFND	LIBRARIES_VIDEOCD_I
LIBRARIES_VIDEOCD_I	SET	1

**
**	$Id: videocd_lib.i,v 1.2 93/10/12 13:28:59 davidj Exp $
**
**	(C) Copyright 1993 Commodore-Amiga, Inc.
**	    All Rights Reserved
**

*****************************************************************************

	IFND	EXEC_NODES_I
	INCLUDE "exec/nodes.i"
	ENDC

	IFND	UTILITY_TAGITEMS_I
	INCLUDE "utility/tagitem.i"
	ENDC

*****************************************************************************

; Types returned by GetCDTypeA()
CDT_ERROR	equ	-1
CDT_UNKNOWN	equ	0
CDT_MUSIC	equ	1
CDT_KARAOKE	equ	2
CDT_GENERIC	equ	3
CDT_VIDEOCD	equ	4

*****************************************************************************

; structure for returning strings
    STRUCTURE CDText,0
	ULONG	 cs_Length	; Number of characters in string
	ULONG	 cs_Text	; Pointer to the string
    LABEL CDText_SIZEOF

*****************************************************************************

; Tags used by GetVideoCDInfo()
GVCD_Dummy			equ	(TAG_USER+$1000)

GVCD_ErrorCode			equ	(GVCD_Dummy+1)
    ; (ULONG *) Pointer to error code storage location
GVCD_GetSequenceOffset		equ	(GVCD_Dummy+2)
    ; (BOOL) Used to obtain the sequence sector offset

; Code Set Information
GVCD_CodeSet			equ	(GVCD_Dummy+5)
    ; (ULONG) Code Set
VCCS_ISO_646		equ	$01
VCCS_ISO_8859_1		equ	$02
VCCS_JIS		equ	$03
VCCS_SHIFT_JIS		equ	$04

GVCD_GetCodeSets		equ	(GVCD_Dummy+6)
    ; (BOOL) Used to obtain a list of available code sets
GVCD_NumCodeSets		equ	(GVCD_Dummy+7)
    ; (ULONG) Number of available code sets
GVCD_CodeSets			equ	(GVCD_Dummy+8)
    ; (struct MinList *) List of available code sets

; Disc Information
GVCD_DiscTitle			equ	(GVCD_Dummy+10)
    ; (struct CDText *) Title of the disc
GVCD_DiscCatalogNum		equ	(GVCD_Dummy+11)
    ; (struct CDText *) Disc catalog number
GVCD_DiscNumSequences		equ	(GVCD_Dummy+12)
    ; (ULONG) Number of sequences on the disc
GVCD_DiscDescription		equ	(GVCD_Dummy+13)
    ; (struct CDText *) Disc description
GVCD_DiscUser4			equ	(GVCD_Dummy+14)
GVCD_DiscUser5			equ	(GVCD_Dummy+15)
GVCD_DiscUser6			equ	(GVCD_Dummy+16)
GVCD_DiscUser7			equ	(GVCD_Dummy+17)
    ; (struct CDText *) User definable terms

; Sequence Information
GVCD_SequenceISRCCode		equ	(GVCD_Dummy+18)
    ; (struct CDText *) ISRC code of the sequence
GVCD_SequenceTitle		equ	(GVCD_Dummy+19)
    ; (struct CDText *) Sequence title
GVCD_SequenceTitleSort		equ	(GVCD_Dummy+20)
    ; (struct CDText *) Sequence title for sorting
GVCD_SequencePerformer		equ	(GVCD_Dummy+21)
    ; (struct CDText *) Performers name
GVCD_SequencePerformerSort	equ	(GVCD_Dummy+22)
    ; (struct CDText *) Performers name for sorting
GVCD_SequenceWriter		equ	(GVCD_Dummy+23)
    ; (struct CDText *) Name of the song writer(s)
GVCD_SequenceComposer		equ	(GVCD_Dummy+24)
    ; (struct CDText *) Name of the composer(s)
GVCD_SequenceArranger		equ	(GVCD_Dummy+25)
    ; (struct CDText *) Name of the arranger(s)
GVCD_SequencePlayer		equ	(GVCD_Dummy+26)
    ; (struct CDText *) Name of the player(s)
GVCD_SequenceTextHeader		equ	(GVCD_Dummy+27)
    ; (struct CDText *) Header part of the song text
GVCD_SequenceText		equ	(GVCD_Dummy+28)
    ; (struct CDText *) Song text
GVCD_SequenceKaraokiKey		equ	(GVCD_Dummy+29)
    ; (struct CDText *) Karaoke key
GVCD_SequenceOriginalKey	equ	(GVCD_Dummy+30)
    ; (struct CDText *) Original key
GVCD_SequenceDescription	equ	(GVCD_Dummy+31)
    ; (struct CDText *) Description of the contents of the sequence
GVCD_SequenceUser22		equ	(GVCD_Dummy+32)
GVCD_SequenceUser23		equ	(GVCD_Dummy+32)
GVCD_SequenceUser24		equ	(GVCD_Dummy+32)
GVCD_SequenceUser25		equ	(GVCD_Dummy+32)
GVCD_SequenceUser26		equ	(GVCD_Dummy+32)
GVCD_SequenceUser27		equ	(GVCD_Dummy+32)
GVCD_SequenceUser28		equ	(GVCD_Dummy+32)
GVCD_SequenceUser29		equ	(GVCD_Dummy+32)
GVCD_SequenceUser30		equ	(GVCD_Dummy+32)
GVCD_SequenceUser31		equ	(GVCD_Dummy+32)
    ; (struct CDText *) User definable items

GVCD_SequenceOffset		equ	(GVCD_Dummy+40)
    ; (ULONG) Sector offset of the sequence
GVCD_SequenceDuration		equ	(GVCD_Dummy+41)

; info.vcd information
GVCD_InfoVersion		equ	(GVCD_Dummy+101)
    ; Specification version number
GVCD_InfoAlbumID		equ	(GVCD_Dummy+102)
    ; Album identification
GVCD_InfoNumberVolumes		equ	(GVCD_Dummy+103)
    ; Number of volumes in album
GVCD_InfoVolumeNumber		equ	(GVCD_Dummy+104)
    ; Volume number

; entries.vcd information
GVCD_EntriesVersion		equ	(GVCD_Dummy+111)
    ; Specification version number
GVCD_EntriesUsed		equ	(GVCD_Dummy+112)
    ; Entries used
GVCD_EntriesArray		equ	(GVCD_Dummy+113)
    ; NULL terminated array of sector addresses

*****************************************************************************

    STRUCTURE CodeSetNode,MLN_SIZE
	UBYTE		 csn_CharSet		; Character set code
	STRUCTURE	 csn_Country,2		; Country code
	STRUCTURE	 csn_Reserved1,2	; Reserved bytes
	UBYTE		 csn_Offset		; File offset
	UBYTE		 csn_Length		; File length
	STRUCTURE	 csn_Flags,8		; Disc and sequence information flags
	STRUCTURE	 csn_Reserved2,17	; Reserved bytes
    LABEL		 CodeSetNode_SIZEOF

*****************************************************************************

	ENDC	; LIBRARIES_VIDEOCD_I
