	IFND IFFPARSEBASE_I
IFFPARSEBASE_I SET	1

;-----------------------------------------------------------------------

	INCLUDE "exec/types.i"
	INCLUDE "exec/libraries.i"

;-----------------------------------------------------------------------

   STRUCTURE IFFParseLib,LIB_SIZE
        UWORD	ipb_Pad
	ULONG	ipb_SysBase
	ULONG	ipb_DOSBase
	ULONG	ipb_SegList
   LABEL IFFParseLib_SIZEOF

;-----------------------------------------------------------------------

	LIBINIT

	LIBDEF	_LVOAllocIFF
	LIBDEF	_LVOOpenIFF
	LIBDEF	_LVOParseIFF
	LIBDEF	_LVOCloseIFF
	LIBDEF	_LVOFreeIFF
	LIBDEF	_LVOReadChunkBytes
	LIBDEF	_LVOWriteChunkBytes
	LIBDEF	_LVOReadChunkRecords
	LIBDEF	_LVOWriteChunkRecords
	LIBDEF	_LVOPushChunk
	LIBDEF	_LVOPopChunk
	LIBDEF	_LVOReserved1
	LIBDEF	_LVOEntryHandler
	LIBDEF	_LVOExitHandler
	LIBDEF	_LVOPropChunk
	LIBDEF	_LVOPropChunks
	LIBDEF	_LVOStopChunk
	LIBDEF	_LVOStopChunks
	LIBDEF	_LVOCollectionChunk
	LIBDEF	_LVOCollectionChunks
	LIBDEF	_LVOStopOnExit
	LIBDEF	_LVOFindProp
	LIBDEF	_LVOFindCollection
	LIBDEF	_LVOFindPropContext
	LIBDEF	_LVOCurrentChunk
	LIBDEF	_LVOParentChunk
	LIBDEF	_LVOAllocLocalItem
	LIBDEF	_LVOLocalItemData
	LIBDEF	_LVOSetLocalItemPurge
	LIBDEF	_LVOFreeLocalItem
	LIBDEF	_LVOFindLocalItem
	LIBDEF	_LVOStoreLocalItem
	LIBDEF	_LVOStoreItemInContext
	LIBDEF	_LVOInitIFF
	LIBDEF	_LVOInitIFFasDOS
	LIBDEF	_LVOInitIFFasClip
	LIBDEF	_LVOOpenClipboard
	LIBDEF	_LVOCloseClipboard
	LIBDEF	_LVOGoodID
	LIBDEF	_LVOGoodType
	LIBDEF	_LVOIDtoStr

;---------------------------------------------------------------------------

CALL MACRO <Function_Name>
	xref _LVO\1
 	jsr _LVO\1(A6)
     ENDM

;---------------------------------------------------------------------------

GO   MACRO <Function_Name>
	xref _LVO\1
 	jmp _LVO\1(A6)
     ENDM

;---------------------------------------------------------------------------

	ENDC	; IFFPARSEBASE_I
