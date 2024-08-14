	IFND UTILITYBASE_I
UTILITYBASE_I SET	1

;-----------------------------------------------------------------------

	INCLUDE "exec/types.i"
	INCLUDE "exec/libraries.i"

;-----------------------------------------------------------------------

   STRUCTURE UtilityLib,LIB_SIZE
        UBYTE	ub_CDLanguage    ; for use by CD game system, leave alone!
        UBYTE   ub_CDReserved
	ULONG	ub_SysBase
	APTR	ub_MasterSpace
	ULONG	ub_Sequence
   LABEL UtilityLib_SIZEOF

;-----------------------------------------------------------------------

	LIBINIT

	LIBDEF	_LVOFindTagItem
	LIBDEF	_LVOGetTagData
	LIBDEF	_LVOPackBoolTags
	LIBDEF	_LVONextTagItem
	LIBDEF	_LVOFilterTagChanges
	LIBDEF	_LVOMapTags
	LIBDEF	_LVOAllocateTagItems
	LIBDEF	_LVOCloneTagItems
	LIBDEF	_LVOFreeTagItems
	LIBDEF	_LVORefreshTagItemClones
	LIBDEF	_LVOTagInArray
	LIBDEF	_LVOFilterTagItems

	LIBDEF	_LVOCallHookPkt
	LIBDEF	_LVOLibReserved1

	LIBDEF	_LVOLibReserved2
	LIBDEF	_LVOAmiga2Date
	LIBDEF	_LVODate2Amiga
	LIBDEF	_LVOCheckDate

	LIBDEF	_LVOSMult32
	LIBDEF	_LVOUMult32
	LIBDEF	_LVOSDivMod32
	LIBDEF	_LVOUDivMod32

	LIBDEF	_LVOStricmp
	LIBDEF	_LVOStrnicmp
	LIBDEF	_LVOToUpper
	LIBDEF	_LVOToLower

	LIBDEF	_LVOApplyTagChanges
	LIBDEF	_LVOLibReserved3

	LIBDEF	_LVOSMult64
	LIBDEF	_LVOUMult64

	LIBDEF	_LVOPackStructureTags
	LIBDEF	_LVOUnpackStructureTags

	LIBDEF	_LVOAddNamedObject
	LIBDEF	_LVOAllocNamedObjectA
	LIBDEF	_LVOAttemptRemNamedObject
	LIBDEF	_LVOFindNamedObject
	LIBDEF	_LVOFreeNamedObject
	LIBDEF	_LVONamedObjectName
	LIBDEF	_LVOReleaseNamedObject
	LIBDEF	_LVORemNamedObject

	LIBDEF	_LVOGetUniqueID

;---------------------------------------------------------------------------

CALL MACRO <Function_Name>
 	jsr _LVO\1(A6)
     ENDM

;---------------------------------------------------------------------------

	ENDC	; UTILITYBASE_I
