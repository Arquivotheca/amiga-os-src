
FUNCDEF 	MACRO
		PUBLIC	_LVO\1
_LVO\1		EQU	\2
		ENDM

AZLIB_BASE	EQU	($FFFFFFFA-30)
	FUNCDEF SetJanusHandler,(AZLIB_BASE-0)
	FUNCDEF SetJanusEnable,(AZLIB_BASE-6)
	FUNCDEF SetJanusRequest,(AZLIB_BASE-12)
	FUNCDEF SendJanusInt,(AZLIB_BASE-18)
	FUNCDEF CheckJanusInt,(AZLIB_BASE-24)
	FUNCDEF AllocJanusMem,(AZLIB_BASE-30)
	FUNCDEF FreeJanusMem,(AZLIB_BASE-36)
	FUNCDEF JanusMemBase,(AZLIB_BASE-42)
	FUNCDEF JanusMemType,(AZLIB_BASE-48)
	FUNCDEF JanusMemToOffset,(AZLIB_BASE-54)
	FUNCDEF GetParamOffset,(AZLIB_BASE-60)
	FUNCDEF SetParamOffset,(AZLIB_BASE-66)

	FUNCDEF GetJanusStart,($FFFFFF9A)
	FUNCDEF SetupJanusSig,($FFFFFF94)
	FUNCDEF CleanupJanusSig,($FFFFFF8E)

*	FUNCDEF XGetJanusStart,(AZLIB_BASE-72)
*	FUNCDEF XSetupJanusSig,(AZLIB_BASE-78)
*	FUNCDEF XCleanupJanusSig,(AZLIB_BASE-84)

	FUNCDEF JanusLock,(AZLIB_BASE-90)
	FUNCDEF JanusUnLock,(AZLIB_BASE-96)
	FUNCDEF JBCopy,(AZLIB_BASE-102)
