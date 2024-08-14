
; Structure defs for buffer mem

	Structure AMouseBufMem,0

	UWORD	AmigaPCX
	UWORD	AmigaPCY
	UWORD	AmigaPCLeftP
	UWORD	AmigaPCRightP
	UWORD	AmigaPCLeftR
	UWORD	AmigaPCRightR
	UWORD	AmigaPCStatus

	LABEL	AMouseBufMem_SIZEOF

; structure defs for param mem


	Structure AMouseParamMem,0

	UWORD	BufMemOffset
	UWORD	PCParam
	UBYTE	Pad
	UBYTE	WriteLock			; byte 5 for AMiga, 4 for PC

	LABEL	AMouseParamMem_SIZEOF

; Access masks

WordParam   EQU  MEM_WORDACCESS+MEMF_PARAMETER
WordBuff    EQU  MEM_WORDACCESS+MEMF_BUFFER

; area sizes ( 8-byte aligned )

DataSize    EQU  (((AMouseBufMem_SIZEOF+7)>>3)<<3)
ParamSize   EQU  (((AMouseParamMem_SIZEOF+7)>>3)<<3)
