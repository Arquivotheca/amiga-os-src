
* *** linkage.asm *************************************************************
* 
* linkage.asm -- link library for janus stuff
* 
* Copyright (C) 1986, 1988, Commodore Amiga Inc.
* All rights reserved
* 
* Date       Name               Description
* ---------  -----------        -----------------------------------------------
* 10-Apr-88  -RJ Mical          Added 9 new function calls 
* early 86   Katin/Burns clone  Created this file!
* 
* *****************************************************************************



	INCLUDE "exec/types.i"



FUNCDEF MACRO
	XREF	_LVO\1
	XDEF	_\1
	ENDM


	INCLUDE "jfuncs.i"
	INCLUDE "jfuncs2.i"


SAVE	MACRO
	move.l	a6,-(sp)
	move.l	_JanusBase,a6
	ENDM


RESTORE MACRO
	move.l	(sp)+,a6
	rts
	ENDM


CALL	MACRO
	JSR	_LVO\1(a6)
	ENDM


	XREF	_JanusBase



_SetJanusHandler:	; ( jintnum, intserver )(d0/a1)
	SAVE
	movem.l 8(sp),d0/a1
	CALL	SetJanusHandler
	RESTORE


_SetJanusEnable:	; ( jintnum, newvalue )(d0/d1)
	SAVE
	movem.l 8(sp),d0/d1
	CALL	SetJanusEnable
	RESTORE


_SetJanusRequest:	; ( jintnum, newvalue )(d0/d1)
	SAVE
	movem.l 8(sp),d0/d1
	CALL	SetJanusRequest
	RESTORE


_SendJanusInt:		; ( jintnum )(d0)
	SAVE
	move.l 8(sp),d0
	CALL	SendJanusInt
	RESTORE


_CheckJanusInt: 	; ( jintnum )(d0)
	SAVE
	move.l	8(sp),d0
	CALL	CheckJanusInt
	RESTORE


_AllocJanusMem: 	; ( size, type )(d0/d1)
	SAVE
	movem.l 8(sp),d0/d1
	CALL	AllocJanusMem
	RESTORE


_FreeJanusMem:		; ( ptr, size )(a1,d0)
	SAVE
	move.l	8(sp),a1
	move.l	12(sp),d0
	CALL	FreeJanusMem
	RESTORE


_JanusMemBase:		; ( type )(d0)
	SAVE
	move.l	8(sp),d0
	CALL	JanusMemBase
	RESTORE


_JanusMemType:		; ( ptr )(d0)
	SAVE
	move.l	8(sp),d0
	CALL	JanusMemType
	RESTORE


_JanusMemToOffset:	; ( ptr )(d0)
	SAVE
	move.l	8(sp),d0
	CALL	JanusMemToOffset
	RESTORE


_GetParamOffset:	 ; ( jintnum )(d0)
	SAVE
	move.l	8(sp),d0
	CALL	GetParamOffset
	RESTORE


_SetParamOffset:	; ( jintnum, offset )(d0/d1)
	SAVE
	movem.l 8(sp),d0/d1
	CALL	SetParamOffset
	RESTORE


_GetJanusStart: 	; ()()
	SAVE
	CALL	GetJanusStart
	RESTORE


_SetupJanusSig: 	; ( jintnum, signum, memsize, memtype )(d0/d1/d2/d3)
	SAVE
	movem.l d2/d3,-(sp)
	movem.l 8+8(sp),d0/d1/d2/d3
	CALL	SetupJanusSig
	movem.l (sp)+,d2/d3
	RESTORE


_CleanupJanusSig:	; ( setupsig )(a0)
	SAVE
	move.l	8(sp),a0
	CALL	CleanupJanusSig
	RESTORE


_JanusLock:		; ( ptr )(a0)
	SAVE
	move.l	8(sp),a0
	CALL	JanusLock
	RESTORE


_JanusUnlock:		; ( ptr )(a0)
	SAVE
	move.l	8(sp),a0
	CALL	JanusUnlock
	RESTORE

_JBCopy:		; ( source, destination, length )(a0/a1,d0)
	SAVE
	movem.l 8(sp),a0/a1
	move.l	16(sp),d0
	CALL	JBCopy	      
	RESTORE



* *****************************************************************************
* RJ's New Janus Functions, April 1988


_AddService:
* resultcode = AddService(
*              ServiceData, AppID, LocalID, MemSize, MemType, SignalBit, Flags)
* D0:0-15      A0           D0     D1:0-15  D2:0-15  D3:0-15  D4         D5:0-15
	SAVE
	MOVEM.L	D2-D5,-(SP)
	MOVEM.L	8+16(SP),A0
	MOVEM.L	8+16+4(SP),D0-D5
	CALL	AddService
	MOVEM.L	(SP)+,D2-D5
	RESTORE


_GetService:
* resultcode = GetService(
*                    ServiceData, ApplicationID, LocalID, SignalNumber, Flags)
* D0:0-15            A0           D0             D1:0-15  D2:0-15       D3:0-15
	SAVE
	MOVEM.L	D2-D3,-(SP)
	MOVE.L	8+8(SP),A0
	MOVEM.L	8+8+4(SP),D0-D3
	CALL	GetService
	MOVEM.L	(SP)+,D2-D3
	RESTORE


* _CallService:
* * VOID CallService(ChannelNumber)
* *                  D0:0-7
* 	SAVE
* 	MOVE.L	8(SP),D0
* 	CALL	CallService
* 	RESTORE


_CallService:
* VOID CallService(ServiceData)
*                  A0
	SAVE
	MOVE.L	8(SP),A0
	CALL	CallService
	RESTORE


_ReleaseService:
* VOID ReleaseService(ServiceData)
*                     A0
	SAVE
	MOVE.L	8(SP),A0
	CALL	ReleaseService
	RESTORE


_DeleteService:
* VOID DeleteService(ServiceData)
*                    A0
	SAVE
	MOVE.L	8(SP),A0
	CALL	DeleteService
	RESTORE


_JanusOffsetToMem:
* ptr = JanusOffsetToMem(offset, type)
* D0                     D0:0-15 D1
	SAVE
	MOVEM.L	8(SP),D0/D1
	CALL	JanusOffsetToMem
	RESTORE


_TranslateJanusPtr:
* APTR TranslateJanusPtr(ptr, type);
* D0                     A0   D0
	SAVE
	MOVE.L	8(SP),A0
	MOVE.L	12(SP),D0
	CALL	TranslateJanusPtr
	RESTORE


_MakeBytePtr:
* UBYTE *MakeBytePtr(ptr);
* D0                 A0
	SAVE
	MOVE.L	8(SP),A0
	CALL	MakeBytePtr
	RESTORE


_MakeWordPtr:
* UBYTE *MakeWordPtr(ptr);
* D0                 A0
	SAVE
	MOVE.L	8(SP),A0
	CALL	MakeWordPtr
	RESTORE


_AllocJRemember:
* ptr = AllocJRemember(key, size,   type);
* D0,A0                A0   D0:0-15 D1:0-15
	SAVE
	MOVE.L	8(SP),A0
	MOVEM.L	8+4(SP),D0-D1
	CALL	AllocJRemember
	RESTORE


_AttachJRemember:
* VOID AttachJRemember(tokey, fromkey);
*                      A0     A1
	SAVE
	MOVEM.L	8(SP),A0-A1
	CALL	AttachJRemember
	RESTORE


_FreeJRemember:
* VOID FreeJRemember(key, reallyforget);
*                    A0   D0:0-15
	SAVE
	MOVE.L	8(SP),A0
	MOVE.L	12(SP),D0
	CALL	FreeJRemember
	RESTORE


_AllocServiceMem:
* ptr = AllocServiceMem(servicedata, size,   type);
* D0,A0                 A0           D0:0-15 D1:0-15
	SAVE
	MOVE.L	8(SP),A0
	MOVEM.L	8+4(SP),D0-D1
	CALL	AllocServiceMem
	RESTORE


_FreeServiceMem:
* VOID FreeServiceMem(servicedata, ptr);
*                     A0           A1
	SAVE
	MOVE.L	8(SP),A0
	MOVE.L	12(SP),A1
	CALL	FreeServiceMem
	RESTORE



_JanusLockAttempt:		; ( ptr )(a0)
	SAVE
	MOVE.L	8(sp),A0
	CALL	JanusLockAttempt
	RESTORE



	END


