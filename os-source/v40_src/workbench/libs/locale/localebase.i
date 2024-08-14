	IFND	LOCALEBASE_I
LOCALEBASE_I	SET	1

;---------------------------------------------------------------------------

	INCLUDE "exec/types.i"
	INCLUDE "exec/lists.i"
	INCLUDE "exec/libraries.i"
	INCLUDE "exec/semaphores.i"
	INCLUDE "prefs/locale.i"
	INCLUDE	"locale.i"

;---------------------------------------------------------------------------

  STRUCTURE ExtLocale,Locale_SIZEOF
  	UWORD	  el_UsageCnt
	UWORD	  el_MaxDateLen

	APTR      el_Language
	APTR	  el_ConvToLower
	APTR	  el_ConvToUpper
	APTR	  el_GetCodeSet
	APTR	  el_GetDriverInfo
	APTR	  el_GetLocaleStr
	APTR	  el_IsAlNum
	APTR	  el_IsAlpha
	APTR	  el_IsCntrl
	APTR	  el_IsDigit
	APTR	  el_IsGraph
	APTR	  el_IsLower
	APTR	  el_IsPrint
	APTR	  el_IsPunct
	APTR	  el_IsSpace
	APTR	  el_IsUpper
	APTR	  el_IsXDigit
	APTR	  el_StrConvert
	APTR	  el_StrnCmp
	STRUCT	  el_LocaleName,32
	STRUCT    el_Prefs,LocalePrefs_SIZEOF
	STRUCT	  el_NumFormat,16
   LABEL ExtLocale_SIZEOF

;---------------------------------------------------------------------------

   STRUCTURE LocaleLib,LIB_SIZE
        BOOL	lb_PatchedOS
        STRUCT  lb_LibLock,SS_SIZE
        UWORD	lb_UsageCnt
	ULONG   lb_SysLib
	ULONG   lb_DosLib
	ULONG   lb_UtilityLib
	ULONG	lb_IntuitionLib
	ULONG   lb_SegList
	APTR	lb_SMult32
	APTR	lb_UMult32
	APTR	lb_SDivMod32
	APTR	lb_UDivMod32
	APTR	lb_DOSCatalog
	APTR    lb_OldGetDOSString
	STRUCT	lb_Catalogs,MLH_SIZE
	APTR    lb_DefaultLocale
	STRUCT  lb_PrefLocale,ExtLocale_SIZEOF
	APTR    lb_OldReqTitle
	APTR    lb_OldWBTitle
   LABEL LocaleLib_SIZEOF

;---------------------------------------------------------------------------

CALL MACRO <Function_Name>
 	jsr _LVO\1(A6)
     ENDM

;---------------------------------------------------------------------------

GO   MACRO <Function_Name>
 	jmp _LVO\1(A6)
     ENDM

;---------------------------------------------------------------------------

	ENDC	; LOCALEBASE_I
