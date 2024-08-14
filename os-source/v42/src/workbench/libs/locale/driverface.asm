	OPTIMON

;---------------------------------------------------------------------------

	NOLIST

	INCLUDE "exec/types.i"
	INCLUDE "exec/memory.i"
	INCLUDE "exec/lists.i"
	INCLUDE "dos/dos.i"

	INCLUDE "localebase.i"

	LIST

;---------------------------------------------------------------------------

	XDEF _ConvToLowerLVO
	XDEF _ConvToUpperLVO
	XDEF _GetCodeSet
	XDEF _GetDriverInfo
	XDEF _GetLocaleStrLVO
	XDEF _IsAlNumLVO
	XDEF _IsAlphaLVO
	XDEF _IsCntrlLVO
	XDEF _IsDigitLVO
	XDEF _IsGraphLVO
	XDEF _IsLowerLVO
	XDEF _IsPrintLVO
	XDEF _IsPunctLVO
	XDEF _IsSpaceLVO
	XDEF _IsUpperLVO
	XDEF _IsXDigitLVO
	XDEF _StrConvertLVO
	XDEF _StrnCmpLVO

;---------------------------------------------------------------------------
* WARNING: Also check patches.asm when making any changes to this routine!

_ConvToLowerLVO:
        move.l	el_ConvToLower(a0),a0
	jmp	(a0)

;---------------------------------------------------------------------------
* WARNING: Also check patches.asm when making any changes to this routine!

_ConvToUpperLVO:
        move.l	el_ConvToUpper(a0),a0
	jmp	(a0)

;---------------------------------------------------------------------------

_GetCodeSet:
        move.l	el_GetCodeSet(a0),a0
	jmp	(a0)

;---------------------------------------------------------------------------

_GetDriverInfo:
        move.l	el_GetDriverInfo(a0),a0
	jmp	(a0)

;---------------------------------------------------------------------------

_GetLocaleStrLVO:
        move.l	el_GetLocaleStr(a0),a0
	jmp	(a0)

;---------------------------------------------------------------------------

_IsAlNumLVO:
        move.l	el_IsAlNum(a0),a0
	jmp	(a0)

;---------------------------------------------------------------------------

_IsAlphaLVO:
        move.l	el_IsAlpha(a0),a0
	jmp	(a0)

;---------------------------------------------------------------------------

_IsCntrlLVO:
        move.l	el_IsCntrl(a0),a0
	jmp	(a0)

;---------------------------------------------------------------------------

_IsDigitLVO:
        move.l	el_IsDigit(a0),a0
	jmp	(a0)

;---------------------------------------------------------------------------

_IsGraphLVO:
        move.l	el_IsGraph(a0),a0
	jmp	(a0)

;---------------------------------------------------------------------------

_IsLowerLVO:
        move.l	el_IsLower(a0),a0
	jmp	(a0)

;---------------------------------------------------------------------------

_IsPrintLVO:
        move.l	el_IsPrint(a0),a0
	jmp	(a0)

;---------------------------------------------------------------------------

_IsPunctLVO:
        move.l	el_IsPunct(a0),a0
	jmp	(a0)

;---------------------------------------------------------------------------

_IsSpaceLVO:
        move.l	el_IsSpace(a0),a0
	jmp	(a0)

;---------------------------------------------------------------------------

_IsUpperLVO:
        move.l	el_IsUpper(a0),a0
	jmp	(a0)

;---------------------------------------------------------------------------

_IsXDigitLVO:
        move.l	el_IsXDigit(a0),a0
	jmp	(a0)

;---------------------------------------------------------------------------

_StrConvertLVO:
        move.l	el_StrConvert(a0),a0
	jmp	(a0)

;---------------------------------------------------------------------------

_StrnCmpLVO:
        move.l	el_StrnCmp(a0),a0
	jmp	(a0)

;---------------------------------------------------------------------------

	END
