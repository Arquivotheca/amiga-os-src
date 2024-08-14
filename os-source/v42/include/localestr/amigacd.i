	IFND TEXTTABLE_I
TEXTTABLE_I	SET	1


;-----------------------------------------------------------------------------


* This file was created automatically by CatComp.
* Do NOT edit by hand!
*


	IFND EXEC_TYPES_I
	INCLUDE 'exec/types.i'
	ENDC

	IFD CATCOMP_ARRAY
CATCOMP_NUMBERS SET 1
CATCOMP_STRINGS SET 1
	ENDC

	IFD CATCOMP_CODE
CATCOMP_BLOCK SET 1
	ENDC


;-----------------------------------------------------------------------------


	IFD CATCOMP_NUMBERS

ERR_NO_FREE_STORE EQU 0
ERR_REQUIRES_3_0 EQU 1
ERR_COULDNT_OPEN_NONVOLATILE EQU 2
ERR_COULDNT_OPEN_LOWLEVEL EQU 3
ERR_COULDNT_OPEN_ASL EQU 4
ERR_COULDNT_OPEN_TABS EQU 5
ERR_COULDNT_OPEN_BUTTON EQU 6
ERR_COULDNT_OPEN_LED EQU 7
ERR_COULDNT_LOCK_DIRECTORY EQU 8
ERR_REQUIRES_A_DIRECTORY_NAME EQU 9
ERR_COULDNT_CREATE_NONVOLATILE EQU 10
ERR_COULDNT_CREATE_POINTER EQU 11
TAB_STARTUP EQU 12
TAB_STORAGE EQU 13
TAB_LANGUAGE EQU 14
ID_AMIGA EQU 15
ID_AMIGACD EQU 16
ID_LOCK EQU 17
ID_DELETE EQU 18
ID_LOCKED EQU 19
ID_UNLOCKED EQU 20
ID_PREPARE EQU 21
WIN_TITLE EQU 22
WIN_PREPARE EQU 23

	ENDC ; CATCOMP_NUMBERS


;-----------------------------------------------------------------------------


	IFD CATCOMP_STRINGS

ERR_NO_FREE_STORE_STR: DC.B 'Not enough memory',$00
ERR_REQUIRES_3_0_STR: DC.B 'Requires AmigaDOS Version 3.0',$00
ERR_COULDNT_OPEN_NONVOLATILE_STR: DC.B 'Couldn',39,'t open nonvolatile.library',$00
ERR_COULDNT_OPEN_LOWLEVEL_STR: DC.B 'Couldn',39,'t open lowlevel.library',$00
ERR_COULDNT_OPEN_ASL_STR: DC.B 'Couldn',39,'t open asl.library',$00
ERR_COULDNT_OPEN_TABS_STR: DC.B 'Couldn',39,'t open tabs.gadget',$00
ERR_COULDNT_OPEN_BUTTON_STR: DC.B 'Couldn',39,'t open button.gadget',$00
ERR_COULDNT_OPEN_LED_STR: DC.B 'Couldn',39,'t open led.image',$00
ERR_COULDNT_LOCK_DIRECTORY_STR: DC.B 'Couldn',39,'t lock directory %s',$00
ERR_REQUIRES_A_DIRECTORY_NAME_STR: DC.B 'Requires a directory name',$00
ERR_COULDNT_CREATE_NONVOLATILE_STR: DC.B 'Couldn',39,'t create nonvolatile directory',$00
ERR_COULDNT_CREATE_POINTER_STR: DC.B 'Couldn',39,'t create nonvolatile pointer file',$00
TAB_STARTUP_STR: DC.B 'Startup',$00
TAB_STORAGE_STR: DC.B 'Storage',$00
TAB_LANGUAGE_STR: DC.B 'Language',$00
ID_AMIGA_STR: DC.B 'Amiga',$00
ID_AMIGACD_STR: DC.B 'Amiga CD',$00
ID_LOCK_STR: DC.B 'Lock',$00
ID_DELETE_STR: DC.B 'Delete',$00
ID_LOCKED_STR: DC.B 'Locked',$00
ID_UNLOCKED_STR: DC.B 'Unlocked',$00
ID_PREPARE_STR: DC.B 'Prepare...',$00
WIN_TITLE_STR: DC.B 'CD Preferences',$00
WIN_PREPARE_STR: DC.B 'Select Volume to Prepare NV On',$00

	ENDC ; CATCOMP_STRINGS


;-----------------------------------------------------------------------------


	IFD CATCOMP_ARRAY

   STRUCTURE CatCompArrayType,0
	LONG cca_ID
	APTR cca_Str
   LABEL CatCompArrayType_SIZEOF

	CNOP 0,4

CatCompArray:
AS0:	DC.L ERR_NO_FREE_STORE,ERR_NO_FREE_STORE_STR
AS1:	DC.L ERR_REQUIRES_3_0,ERR_REQUIRES_3_0_STR
AS2:	DC.L ERR_COULDNT_OPEN_NONVOLATILE,ERR_COULDNT_OPEN_NONVOLATILE_STR
AS3:	DC.L ERR_COULDNT_OPEN_LOWLEVEL,ERR_COULDNT_OPEN_LOWLEVEL_STR
AS4:	DC.L ERR_COULDNT_OPEN_ASL,ERR_COULDNT_OPEN_ASL_STR
AS5:	DC.L ERR_COULDNT_OPEN_TABS,ERR_COULDNT_OPEN_TABS_STR
AS6:	DC.L ERR_COULDNT_OPEN_BUTTON,ERR_COULDNT_OPEN_BUTTON_STR
AS7:	DC.L ERR_COULDNT_OPEN_LED,ERR_COULDNT_OPEN_LED_STR
AS8:	DC.L ERR_COULDNT_LOCK_DIRECTORY,ERR_COULDNT_LOCK_DIRECTORY_STR
AS9:	DC.L ERR_REQUIRES_A_DIRECTORY_NAME,ERR_REQUIRES_A_DIRECTORY_NAME_STR
AS10:	DC.L ERR_COULDNT_CREATE_NONVOLATILE,ERR_COULDNT_CREATE_NONVOLATILE_STR
AS11:	DC.L ERR_COULDNT_CREATE_POINTER,ERR_COULDNT_CREATE_POINTER_STR
AS12:	DC.L TAB_STARTUP,TAB_STARTUP_STR
AS13:	DC.L TAB_STORAGE,TAB_STORAGE_STR
AS14:	DC.L TAB_LANGUAGE,TAB_LANGUAGE_STR
AS15:	DC.L ID_AMIGA,ID_AMIGA_STR
AS16:	DC.L ID_AMIGACD,ID_AMIGACD_STR
AS17:	DC.L ID_LOCK,ID_LOCK_STR
AS18:	DC.L ID_DELETE,ID_DELETE_STR
AS19:	DC.L ID_LOCKED,ID_LOCKED_STR
AS20:	DC.L ID_UNLOCKED,ID_UNLOCKED_STR
AS21:	DC.L ID_PREPARE,ID_PREPARE_STR
AS22:	DC.L WIN_TITLE,WIN_TITLE_STR
AS23:	DC.L WIN_PREPARE,WIN_PREPARE_STR

	ENDC ; CATCOMP_ARRAY


;-----------------------------------------------------------------------------


	IFD CATCOMP_BLOCK

CatCompBlock:
	DC.L $0
	DC.W $12
	DC.B 'Not enough memory',$00
	DC.L $1
	DC.W $1E
	DC.B 'Requires AmigaDOS Version 3.0',$00
	DC.L $2
	DC.W $22
	DC.B 'Couldn',39,'t open nonvolatile.library',$00
	DC.L $3
	DC.W $20
	DC.B 'Couldn',39,'t open lowlevel.library',$00,$00
	DC.L $4
	DC.W $1A
	DC.B 'Couldn',39,'t open asl.library',$00
	DC.L $5
	DC.W $1A
	DC.B 'Couldn',39,'t open tabs.gadget',$00
	DC.L $6
	DC.W $1C
	DC.B 'Couldn',39,'t open button.gadget',$00
	DC.L $7
	DC.W $18
	DC.B 'Couldn',39,'t open led.image',$00
	DC.L $8
	DC.W $1C
	DC.B 'Couldn',39,'t lock directory %s',$00,$00
	DC.L $9
	DC.W $1A
	DC.B 'Requires a directory name',$00
	DC.L $A
	DC.W $26
	DC.B 'Couldn',39,'t create nonvolatile directory',$00
	DC.L $B
	DC.W $2A
	DC.B 'Couldn',39,'t create nonvolatile pointer file',$00,$00
	DC.L $C
	DC.W $8
	DC.B 'Startup',$00
	DC.L $D
	DC.W $8
	DC.B 'Storage',$00
	DC.L $E
	DC.W $A
	DC.B 'Language',$00,$00
	DC.L $F
	DC.W $6
	DC.B 'Amiga',$00
	DC.L $10
	DC.W $A
	DC.B 'Amiga CD',$00,$00
	DC.L $11
	DC.W $6
	DC.B 'Lock',$00,$00
	DC.L $12
	DC.W $8
	DC.B 'Delete',$00,$00
	DC.L $13
	DC.W $8
	DC.B 'Locked',$00,$00
	DC.L $14
	DC.W $A
	DC.B 'Unlocked',$00,$00
	DC.L $15
	DC.W $C
	DC.B 'Prepare...',$00,$00
	DC.L $16
	DC.W $10
	DC.B 'CD Preferences',$00,$00
	DC.L $17
	DC.W $20
	DC.B 'Select Volume to Prepare NV On',$00,$00

	ENDC ; CATCOMP_BLOCK


;-----------------------------------------------------------------------------


   STRUCTURE LocaleInfo,0
	APTR li_LocaleBase
	APTR li_Catalog
   LABEL LocaleInfo_SIZEOF

	IFD CATCOMP_CODE

	XREF      _LVOGetCatalogStr
	XDEF      _GetString
	XDEF      GetString
GetString:
_GetString:
	lea       CatCompBlock(pc),a1
	bra.s     2$
1$: move.w  (a1)+,d1
	add.w     d1,a1
2$: cmp.l   (a1)+,d0
	bne.s     1$
	addq.l    #2,a1
	move.l    (a0)+,d1
	bne.s     3$
	move.l    a1,d0
	rts
3$: move.l  a6,-(sp)
	move.l    d1,a6
	move.l    (a0),a0
	jsr       _LVOGetCatalogStr(a6)
	move.l    (sp)+,a6
	rts
	END

	ENDC ; CATCOMP_CODE


;-----------------------------------------------------------------------------


	ENDC ; TEXTTABLE_I
