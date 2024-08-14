******************************************************************************
*
*	$Id: nvramtree.i,v
*
******************************************************************************
*
*	$Log:	nvramtree.i,v $
* Revision 40.2  93/03/10  17:24:27  brummer
* Fix for returning only 16 bytes reserved data.  This prevents 
* read/writes to NVRAM tag in first 2 long words of NVRAM.
* 
* Revision 40.1  93/03/04  15:15:37  brummer
* Remove reserved string definition.  See related change in
* nvramtree.asm 40.8
*
* Revision 40.0  93/02/25  12:05:59  brummer
* fix revision to 40.0
*
* Revision 1.1  93/02/25  11:35:59  brummer
* Initial revision
*
*
*	(C) Copyright 1993 Commodore-Amiga, Inc.
*	    All Rights Reserved
*
******************************************************************************
;
; Tree structure tag equates :
;
NVRT_Init_Tag		equ	$0056A900		; long word tag for NVRAM
NVRT_Lang_Mask		equ	$000000FF		; language mask

NVRT_App_Tag		equ	%00100000		; App tag type
NVRT_Item_Tag		equ	%01000000		; Item tag type
NVRT_Data_Tag		equ	%01100000		; Data tag type
NVRT_End_Tag		equ	%10100000		; End of data tag type

NVRT_Tag_Mask		equ	%11100000		; tag field mask
NVRT_Length_Mask	equ	%00011111		; length field mask
NVRT_LRU_Mask		equ	%01111111		; LRU data mask
NVRT_MAX_LRU		equ	$07F			; maximum LRU value

	BITDEF	NVRT_LOCK,ITEM,7			; lock bit on item

NVRT_App_Tag_Size	equ	1			; size of App tag
NVRT_Item_Tag_Size	equ	2			; size of Item tag
NVRT_Data_Tag_Size	equ	2			; size of Data tag
NVRT_End_Tag_Size	equ	1			; size of End tag
NVRT_Full_Tag_Size	equ	NVRT_App_Tag_Size+NVRT_Item_Tag_Size+NVRT_Data_Tag_Size

NVRT_Max_String_Length	equ	31			; 5 bits of length = 32 byte strings
;
; System reserved App and Item name equates :
;
; NVRT_RSVD_APP_NAME	STRING	<' System'>		; Reserved App name
; NVRT_RSVD_ITEM_NAME	STRING	<' Rsvd  '>		; Reserved Item name
NVRT_RSVD_APP_NAME_SIZE		equ	7		;
NVRT_RSVD_ITEM_NAME_SIZE	equ	7		;
;
; System reserved App and Item Location equates :
;
NVRT_ABSOLUTE_BASE	equ	0			; Absolute NV RAM base
NVRT_RSVD_ITEM_LOCN	equ	NVRT_ABSOLUTE_BASE+8	; save two longs for NVRAM usage
NVRT_RSVD_DATA_SIZE	equ	24			; 24 bytes reserved
NVRT_RSVD_DATA_MOVE	equ	16			; 16 bytes avail for rd/wd
NVRT_FIRST_APP_LOCN	equ	24			;
;
; Internal parameter for calculating required space and moving data :
;
NVRT_DATA_REQ		equ	1			; include only Data
NVRT_ITEM_DATA_REQ	equ	2			; include Item and Data
NVRT_APP_ITEM_DATA_REQ	equ	3			; include App Item and Data


