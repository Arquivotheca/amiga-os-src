page 63,132
;*****************************************************************************
; Janus Services 2.Generation:
;
;
; New code  :	30-Mar-88 Torsten Burgdorff
; 45.Update :  	 2-Apr-88 TB	add service 10,11,12
; 46.Update :  	10-Apr-88 TB	change structures according to RJ
; 47.Update :	14-Apr-88 TB    change memory locking, add service 13,14
; 49.Update :	26-Apr-88 TB    forget channeling
; 
;******************************************************************************

cseg segment   	para public 'code'

     assume    	cs:cseg,ss:cseg,ds:cseg,es:nothing

;
; external utilities			
;	       	
extrn	       	outhxw:near		     ; prints hex word in ax
extrn	       	outhxb:near		     ; prints hex byte in al
extrn	       	outint:near		     ; prints integer in ax
extrn	       	newline:near		     ; prints cr,lf
extrn	       	pstrng:near		     ; prints out string
;
extrn		SendJanusInt:near
extrn		GetBase:near	
extrn		SetParam:near
extrn		AllocMem:near
extrn		FreeMem:near
extrn		Dummy:near
	
w	       	equ     word ptr
	
include        	janusvar.inc
include        	macros.inc
include        	vars_ext.inc
include        	mes.inc
include        	service.inc

;--------------------------------------------------------------------
;
; InitServicePtr		Looks for a pointer to the shared Service
; ==============		structures and initialize PC pointers
;				identically.
;				
;
; Expects:
;	nothing
;
; Returns:
;	AL 	:	Status (J_OK, J_NO_BASE, J_NO_SERVICE)
;
;--------------------------------------------------------------------
;
public	        InitServicePtr

InitServicePtr	proc	near

	PUSHALL
	mov	cx,WAIT_FREE_LOOP
LoopBase:
	mov	al,JSERV_AMIGASERV
	call	GetBase
	or	al,al
	jne	NoBase
	cmp	di,-1				; valid base found ?		
	jne	FoundBase
	loop	LoopBase
	mov 	al,J_NO_BASE
	jmp	NoBase

FoundBase:
	mov	bx,di
	mov	al,JSERV_PCSERV
	call	SetParam
		
NoBase:
	POPALL
	ret

InitServicePtr	endp


;--------------------------------------------------------------------
;
; Service 10: AddService	Allocates memory block and a channel	
; ======================	number for a new service and introduceses 
;				this service to the system.
;
; Expects:
;	SI:DX 	:	ApplicationID	 	
;	CX 	: 	ServiceID	
;	BX 	:	ParamSize
;      	BP 	:   	ParamType	
;	ES:DI 	: 	Pointer to service handler
;				    		
; Returns:
;	AL 	:	Status (J_OK, J_NO_MEMORY, J_NO_SERVICE_ID,
;				J_NO_BASE, J_NO_CHANNEL, J_NO_SERVICE)
;	ES:DI 	:	Pointer to initialized ServiceData
;
;--------------------------------------------------------------------
;
public	        AddService

AddService	proc	near

	pushall
	mov 	bp,sp
						; get ServiceParam
	mov	cx,WAIT_FREE_LOOP
AS_LoopBase:
	mov	al,JSERV_AMIGASERV
	call	GetBase
	cmp	di,-1				; valid base found ?		
	jne	AS_LockStruc
	loop	AS_LoopBase
	mov 	al,J_NO_BASE
	jmp	AS_NoBase

AS_LockStruc: 					; DI = ServiceParam
	JLOCK  	es:ServiceParam.ServiceLock[di]	; lock ServiceParam structure
		      				; look for this service
	mov	si,-1
	cmp	es:ServiceParam.FirstServiceData[di],-1
	je	NoService
	mov	si,es:ServiceParam.FirstServiceData[di]
						; SI = current ServiceData
FindService:					; check SERVICE_DELETED
	INFO	ServiceSearch
	test	es:ServiceData.Flags[si],SERVICE_DELETED
	jne	NextService	
						; check ApplicationID
	mov	ax,ss:StackFrame.RegSI[bp]
 	cmp	es:w ServiceData.ApplicationID[si],ax
	jne	NextService
	mov	ax,ss:StackFrame.RegDX[bp]
 	cmp	es:w ServiceData.ApplicationID+2[si],ax
	jne	NextService
						; check ServiceID
	mov	ax,ss:StackFrame.RegCX[bp]
 	cmp	es:ServiceData.ServiceID[si],ax
	je	ServiceFound

NextService:
	cmp	es:ServiceData.NextServiceData[si],-1
	je	NoService
	mov 	si,es:ServiceData.NextServiceData[si] 
	jmp	FindService

ServiceFound:					; check UserCount
	cmp	es:ServiceData.UserCount[si],0
	jne     UpdateUserCount
	mov	al,J_NO_SERVICE_ID
	jmp	ErrExist

NoService:					; init new structure
	INFO	MakeService
	mov	bx,ss:StackFrame.RegSI[bp]
	mov	dx,ss:StackFrame.RegDX[bp]
	mov	cx,ss:StackFrame.RegCX[bp]
	call	MakeServiceData			; allocate ServiceData struc
	or	al,al				; error?
	jz	UpdateUserCount
	jmp	ErrAllocS	

UpdateUserCount:				; update UserCount
	mov	es:ServiceData.UserCount[si],1
						; allocate and clear ParamMem
	INFO	AllocParam
	mov	ax,ss:StackFrame.RegBP[bp]
	xor	ah,ah
	mov	bx,ss:StackFrame.RegBX[bp]
	call	AllocMem
	or	al,al				; error?
	jz	UpdateParam
	jmp	ErrAllocP	

UpdateParam: 					; BX = offset Param
						; clear this memory
	mov	al,0
	cld
	mov	cx,ss:StackFrame.RegBX[bp]
	push	di
	mov	di,bx
	rep	stosb
	pop	di
						; set ParamSize and Type
	INFO	InitPointers
	mov	ax,ss:StackFrame.RegBX[bp]
	mov	es:ServiceData.ParamSize[si],ax
	mov	ax,ss:StackFrame.RegBP[bp]
	mov	es:ServiceData.ParamType[si],ax
						; set MemOffset, ParamPtr 
						;  and PC flag
	mov	es:ServiceData.MemOffset[si],bx
	mov	es:w ServiceData.PC8088_Ptr[si],es
	mov	es:w ServiceData.PC8088_Ptr+2[si],bx
	or	es:ServiceData.Flags[si],PC_PTR

						; set PC_Customer
	mov	ax,ss:StackFrame.RegDI[bp]
	mov	es:w ServiceData.PC_Customer[si],ax
	mov	ax,ss:StackFrame.RegES[bp]
	mov	es:w ServiceData.PC_Customer+2[si],ax

						; init customers lock
	mov	es:ServiceData.CustomerLock[si],7Fh 	; to unlocked

	INFO	ServiceAdded
						; send ADDED_SERVICE to Amiga
   	if	CHANNELING
	 mov	al,es:ServiceData.Channel[si]
	 mov	es:ServiceParam.PCAddService[di],al
	 mov	al,ADD_SERVICE
    	 call	CallServiceInternal
	 or	al,al	
	 jz	PrepareReturn
	 jmp	ErrFatal
	else
PAS_not_found:
	 mov	bx,0
	 cmp	es:ServiceParam.PCAddService[di][bx],-1
 	 je 	PAS_found
	 add	bx,2
	 cmp	es:ServiceParam.PCAddService[di][bx],-1
 	 jne	PAS_not_found
PAS_found:
	 mov	es:ServiceParam.PCAddService[di][bx],si
         mov	bl,JSERV_AMIGASERV
	 call	SendJanusInt
	endif 

PrepareReturn:					; return with pointer to structure
	mov	ss:StackFrame.RegES[bp],es
	mov	ss:StackFrame.RegDI[bp],si
	mov	al,J_OK
	jmp	AS_Exit

ErrFatal:					; free para mem, service mem
	push	ax  				;  and channel
	mov	al,PARA_TYPE
	mov	bx,es:ServiceData.MemOffset[si]
	call	FreeMem				; ignore errors
	pop	ax	

ErrAllocP:					; free service mem and channel
	push	ax
	if	CHANNELING
	 mov	ah,es:ServiceData.Channel[si]	; free channel
	 xor	al,al
	 mov	cl,3
	 shr    ax,cl
	 mov    bl,ah 				; AL = ChannelMasks bit#
	 xor	bh,bh				; BX = ChannelMasks byte#
	 mov	cl,4
	 shr	al,cl				; adjust bit count
	 mov 	cl,1
	 xchg	al,cl
	 or	cl,cl				; check for bit0
	 jz	Bit0Found
	 shl	al,cl
Bit0Found:
 	 not	al
	 and	es:ServiceParam.ChannelMasks[di][bx],al
	endif
						; free service memory
	mov	al,PARA_TYPE
	mov	bx,si
	call	FreeMem				; ignore errors
	pop	ax	

ErrAllocS:					; free nothing
	mov	ss:StackFrame.RegES[bp],0
	mov	ss:StackFrame.RegDI[bp],0
	
ErrExist:      					; Err status already set
AS_NoBase:
AS_Exit:					; unlock structure
	UNLOCK  es:ServiceParam.ServiceLock[di]	
	mov	ah,J_ADD_SERVICE
	mov	ss:StackFrame.RegAX[bp],ax	
	POPALL
	ret

AddService	endp

;--------------------------------------------------------------------
;			
; Service 11: GetService	Looks for an according service and	
; ======================	connects the calling application
;				with this service.
;
; Expects:
;	SI:DX 	:	ApplicationID	  	
;	CX 	: 	ServiceID	
;	AL 	:   	WaitService	
;	ES:DI 	: 	Pointer to application
;			0 : Application doesn't support Amiga calls
;	    		
; Returns:
;	AL 	: 	Status (J_OK, J_NO_SERVICE_ID, J_NO_SERVICE)
;	ES:DI 	: 	Pointer to initialized ServiceData
;
;--------------------------------------------------------------------
;
public	        GetService

GetService	proc	near

	pushall
	mov 	bp,sp
						; get ServiceParam
	mov	cx,WAIT_FREE_LOOP
GS_LoopBase:
	mov	al,JSERV_AMIGASERV
	call	GetBase
	cmp	di,-1				; valid base found ?		
	jne	GS_LockStruc
	loop	GS_LoopBase
	mov 	al,J_NO_BASE
	jmp	GS_NoBase

GS_LockStruc: 					; DI = ServiceParam
	JLOCK  	es:ServiceParam.ServiceLock[di]	; lock ServiceParam structure
		      				; look for this service
	mov	si,-1
	cmp	es:ServiceParam.FirstServiceData[di],-1
	je	GS_CheckWait
	mov	si,es:ServiceParam.FirstServiceData[di]
						; SI = current ServiceData
GS_FindService:					; check SERVICE_DELETED
	INFO	ServiceSearch
	test	es:ServiceData.Flags[si],SERVICE_DELETED
	jne	GS_NextService	
						; check ApplicationID
	mov	ax,ss:StackFrame.RegSI[bp]
 	cmp	es:w ServiceData.ApplicationID[si],ax
	jne	GS_NextService
	mov	ax,ss:StackFrame.RegDX[bp]
 	cmp	es:w ServiceData.ApplicationID+2[si],ax
	jne	GS_NextService
						; check ServiceID
	mov	ax,ss:StackFrame.RegCX[bp]
 	cmp	es:ServiceData.ServiceID[si],ax
	je	GS_ServiceFound

GS_NextService:
	cmp	es:ServiceData.NextServiceData[si],-1
	je	GS_CheckWait
	mov 	si,es:ServiceData.NextServiceData[si] 
	jmp	GS_FindService

GS_ServiceFound:					; check UserCount
	cmp	es:ServiceData.UserCount[si],0
	jne     GS_UpdateUserCount
	test	ss:StackFrame.RegAX[bp],WAIT_FOR_SERVICE
	jne	GS_NewCustomer
	jmp	ErrID


GS_CheckWait:	
	test	ss:StackFrame.RegAX[bp],WAIT_FOR_SERVICE
	jne	GS_Wait
	jmp	ErrId

GS_Wait:
	INFO	MakeService
	mov	bx,ss:StackFrame.RegSI[bp]
	mov	dx,ss:StackFrame.RegDX[bp]
	mov	cx,ss:StackFrame.RegCX[bp]
	call	MakeServiceData			; allocate ServiceData struc
	or	al,al				; error?
	jz	GS_NewCustomer
	jmp	ErrExit	

GS_UpdateUserCount:				; update UserCount
	inc	es:ServiceData.UserCount[si]

GS_NewCustomer:					; set PC_Customer
	INFO	InitPointers
	mov	ax,ss:StackFrame.RegES[bp]
	mov	bx,ss:StackFrame.RegDI[bp]
	mov	es:w ServiceData.PC_Customer[si],bx
	mov	es:w ServiceData.PC_Customer+2[si],ax

						; init customers lock
	mov	es:ServiceData.CustomerLock[si],7Fh 	; to unlocked

     						; return with pointer to structure
	mov	ss:StackFrame.RegES[bp],es
	mov	ss:StackFrame.RegDI[bp],si
	mov	al,J_OK
	jmp	GS_Exit

ErrID:
	mov	al,J_NO_SERVICE_ID
	
GS_NoBase:
ErrExit:
	mov	ss:StackFrame.RegES[bp],0
	mov	ss:StackFrame.RegDI[bp],0

GS_Exit:					; unlock structure
	UNLOCK  es:ServiceParam.ServiceLock[di]	
	mov	ah,J_ADD_SERVICE
	mov	ss:StackFrame.RegAX[bp],ax	
	POPALL
	ret

GetService	endp


;--------------------------------------------------------------------
;
; MakeServiceData		Allocates parameter memory for a ServiceData
; ===============		structure, allocates a communication channel
;				and clears and initialize structure with
;				ID's and channel numbers.
;				
;
; Expects:
;    	BX:DX 	:	ApplicationID	 	
;	CX 	: 	ServiceID	
;	ES:DI 	: 	Pointer to ServiceParam
;	ES:SI 	: 	Pointer to last ServiceData structure
;			SI = -1, if no ServiceData available
; Returns:
;	ES:SI 	: 	Pointer to ServiceData
;
;
;--------------------------------------------------------------------
;
public	        MakeServiceData


MakeServiceData	proc	near

	push	bx
	push	cx
	push	dx
	push	bp
	mov	bp,bx
						; allocate JanusMem
	mov	al,PARA_TYPE			;  as parameter memory
	mov 	bx,size ServiceData
	call	AllocMem			; BX = offset
	or	al,al
	jz	AllocChannel	
	jmp	ErrAllocJ

AllocChannel:					; allocate channel number
	if	CHANNELING
	 push	bx	   			; save offset
	 mov	bx,0
NextMask:
	 mov	al,es:ServiceParam.ChannelMasks[di][bx]
	 xor    al,-1 				; '1'bit = free channel 
	 jnz	ByteFound			; BX = Byte#
	 inc	bx
	 cmp	bx,31
	 jna	NextMask	  		
	 mov    al,J_NO_CHANNEL			; no channel found
	 jmp	ErrChannel

ByteFound:
	 mov	bh,0				; BL = Byte#
NextBit: 
	 clc
	 shr	al,1
	 jc	BitFound 			; BH = Bit#
	 inc	bh
	 jmp     NextBit

BitFound:	  			; looks not exciting, but works
	 shl	bl,1
	 shl	bl,1
	 shl	bl,1 					
	 add	bl,bh	
	 mov	ah,bl			       	; AH = channel#
	 xor	al,al
	 push	ax				; save channel#
	 mov	cl,3
	 shr    ax,cl
	 mov    bl,ah 				; AL = ChannelMasks bit#
	 xor	bh,bh				; BX = ChannelMasks byte#
	 mov	cl,4
	 shr	al,cl				; adjust bit count
	 mov 	cl,1
	 xchg	al,cl
	 or	cl,cl				; check for bit0
	 jz	Bit1Found
	 shl	al,cl
Bit1Found:
         or	es:ServiceParam.ChannelMasks[di][bx],al
	 pop	ax				; restore channel#
	 pop	bx				; restore ServiceData offset
	endif	
						; clear structure
	cld
	push	cx
	push	di
	mov	cx,size ServiceData
	mov	di,bx
	rep	stosb
	pop	di
	pop	cx
			    			; fill in ID's
	mov	es:w ServiceData.ApplicationID[bx],bp
	mov	es:w ServiceData.ApplicationID+2[bx],dx
	mov	es:ServiceData.ServiceID[bx],cx
	
	if	CHANNELING 				; fill in channel number
	 mov	es:ServiceData.Channel[bx],ah
	endif
						; link this structure into list
	cmp	si,-1
	je	FirstService
	mov	es:ServiceData.NextServiceData[si],bx
	mov	es:ServiceData.NextServiceData[bx],-1
	jmp	ReturnPointer

FirstService:
	mov	es:ServiceParam.FirstServiceData[di],bx
	mov	es:ServiceData.NextServiceData[bx],-1
		
ReturnPointer:					; return with pointer 
	mov	si,bx			 	; to this structure
	xor	al,al 				; set status OK
	jmp	MSD_Exit

ErrChannel:					; free structure
	pop	bx				; restore ServiceData offset
	push	ax
	mov	al,PARA_TYPE			; BX = offset
	call	FreeMem				; ignore errors
	pop	ax	

ErrAllocJ:
						; error status already set
MSD_Exit:
	xor	ah,ah
	pop	bp
	pop	dx
	pop	cx
	pop	bx
   	ret

MakeServiceData	endp


;--------------------------------------------------------------------
;
; Service 12: CallService	Calls an available service via	
; =======================	a communication channel.
;
;
; Expects:
;	DI : Offset of ServiceData structure	
;				    		
; Returns:
;	AL : 	Status (J_OK, J_NO_CHANNEL, J_NO_SERVICE)
;					
;--------------------------------------------------------------------
;
public	        CallService

CallService	proc	near

	PUSHALL
	mov 	bp,sp
						; get ServiceParam
	mov	cx,WAIT_FREE_LOOP
CS_LoopBase:
	mov	al,JSERV_AMIGASERV
	call	GetBase
	cmp	di,-1				; valid base found ?		
	jne	CS_LockStruc
	loop	CS_LoopBase
	mov 	al,J_NO_BASE
	jmp	CS_NoBase

CS_LockStruc: 					; DI = ServiceParam
	JLOCK  	es:ServiceParam.ServiceLock[di]	; lock ServiceParam structure
	jmp	FindSlot

CallServiceInternal:				; Entry for internal using
	PUSHALL					;  we must not search nor
	mov 	bp,sp				;  lock the structure
	
FindSlot:					; look for zero byte in PCToAmiga
	mov	cx,WAIT_FREE_LOOP
LoopFree:
	mov	bx,0
NextFree:
	INFO	NextSlotMsg

	if	CHANNELING
	 cmp	es:ServiceParam.PCToAmiga[di][bx],0
	else
	 cmp	es:ServiceParam.PCToAmiga[di][bx],-1
	endif

	je      FreeFound

	if	CHANNELING
	 inc	bx
	 cmp	bx,3
	else
	 inc	bx
	 inc	bx
	 cmp	bx,6
	endif

	jna	NextFree

	UNLOCK  es:ServiceParam.ServiceLock[di]	; unlock structure and wait
	nop					;  some cycles for the Amiga
	nop
	nop
	JLOCK  	es:ServiceParam.ServiceLock[di]	; lock ServiceParam structure
	loop	LoopFree

	mov	al,J_NO_CHANNEL
	jmp	ErrFree

FreeFound:					; write channel number
	if	CHANNELING
	 mov	ax,ss:StackFrame.RegAX[bp]
         mov	es:ServiceParam.PCToAmiga[di][bx],al
	else
         mov	ax,ss:StackFrame.RegDI[bp]
	 mov	es:ServiceParam.PCToAmiga[di][bx],ax
	endif
					; send int to AMiga
	INFO_AX CallAmigaService
	mov	bl,JSERV_AMIGASERV
	call	SendJanusInt
	mov	al,J_OK

CS_NoBase:
ErrFree:
	UNLOCK  es:ServiceParam.ServiceLock[di]	
	mov	ah,J_CALL_SERVICE
	mov	ss:StackFrame.RegAX[bp],ax	
	POPALL	
	ret

CallService	endp


;--------------------------------------------------------------------
;
; Service 13: ReleaseService		Disconnets from service.	
; ==========================	
;
;
; Expects:
;	ES:DI : Pointer to initialized ServiceData
;				    		
; Returns:
;	AL : 	Status (J_GOODBYE, J_NO_SERVICE)
;
;--------------------------------------------------------------------
;
public	        ReleaseService

ReleaseService	proc	near
						; lock ServiceParam structure
	pushall
	mov 	bp,sp
						; get ServiceParam
	mov	cx,WAIT_FREE_LOOP
RS_LoopBase:
	mov	al,JSERV_AMIGASERV
	call  	GetBase
	cmp	di,-1				; valid base found ?		
	jne	RS_LockStruc
	loop	RS_LoopBase
	mov 	al,J_NO_BASE
	jmp	RS_NoBase

RS_LockStruc: 					; DI = ServiceParam
	JLOCK  	es:ServiceParam.ServiceLock[di]	; lock ServiceParam structure
	
	mov	si,ss:StackFrame.RegDI[bp]
	xor	ah,ah				; check UserCount
	or	al,es:ServiceData.UserCount[si]

	INFO_AX	UCountMsg

	jz	RemoveStruc
	mov	ax,0				; let this pointer point to zero
	mov	es:w ServiceData.PC_Customer[si],ax
	mov	es:w ServiceData.PC_Customer+2[si],ax
	dec	es:ServiceData.UserCount[si]
	jnz	RS_Exit

RemoveStruc:					; remove structure
	mov	ax,si
	INFO_AX	RemoveMsg

	call	RemoveService			; free memory and channel

RS_NoBase:						
RS_Exit:					; unlock ServiceParam structures
	UNLOCK  es:ServiceParam.ServiceLock[di]	
	mov	ah,J_RELEASE_SERVICE
	mov	ss:StackFrame.RegAX[bp],ax	
	POPALL	
	ret

ReleaseService	endp


;--------------------------------------------------------------------
;
; Service 14: DeleteService	      	Removes service from the system.
; =========================	
;
;
; Expects:
;	ES:DI : Pointer to initialized ServiceData
;				    		
; Returns:
;	AL : 	Status (J_GOODBYE, J_NO_SERVICE)
;
;--------------------------------------------------------------------
;
public	        DeleteService

DeleteService	proc	near

	pushall
	mov 	bp,sp
						; get ServiceParam
	mov	cx,WAIT_FREE_LOOP
DS_LoopBase:
	mov	al,JSERV_AMIGASERV
	call	GetBase
	cmp	di,-1				; valid base found ?		
	jne	DS_LockStruc
	loop	DS_LoopBase
	mov 	al,J_NO_BASE
	jmp	DS_NoBase

DS_LockStruc: 					; DI = ServiceParam
	JLOCK  	es:ServiceParam.ServiceLock[di]	; lock ServiceParam structure

	mov	si,ss:StackFrame.RegDI[bp]	; set SERVICE_DELETED flag
	mov	es:ServiceData.Flags[si],SERVICE_DELETED
							
	xor	ah,ah				; check UserCount
	or	al,es:ServiceData.UserCount[si]

	INFO_AX	UCountMsg

	dec	es:ServiceData.UserCount[si]
	jnz	Inform_Del				
						; remove structure
	mov	si,ss:StackFrame.RegDI[bp]
	mov	ax,si

	INFO_AX	RemoveMsg

	call	RemoveService			; free memory and channel
	jmp	DS_Exit

Inform_Del:					; send SERVICE_DELETED to Amiga
	INFO	ServiceDeleted

	if	CHANNELING
	 mov	al,DELETE_SERVICE
    	 call	CallServiceInternal
	else
PDS_not_found:
	 mov	bx,0
	 cmp	es:ServiceParam.PCDeleteService[di][bx],-1
 	 je 	PDS_found
	 add	bx,2
	 cmp	es:ServiceParam.PCDeleteService[di][bx],-1
 	 jne	PDS_not_found
PDS_found:
	 push	bx
	 mov	bx,0
	 cmp	es:ServiceParam.PCAddService[di][bx],si
 	 je 	Old_ADD_found
	 add	bx,2
	 cmp	es:ServiceParam.PCAddService[di][bx],si
 	 jne	No_Old_ADD
Old_ADD_found:						; we are going to delete a	 	
	 mov	es:ServiceParam.PCAddService[di][bx],-1	; service, which never
	 jmp	DS_NoBase				; has been send to the
							; Amiga
No_Old_ADD:
	 mov	es:ServiceParam.PCDeleteService[di][bx],si
         mov	bl,JSERV_AMIGASERV
	 call	SendJanusInt
	
	endif

DS_NoBase:						
DS_Exit:					; unlock ServiceParam structures
	UNLOCK  es:ServiceParam.ServiceLock[di]	
	mov	ah,J_RELEASE_SERVICE
	mov	ss:StackFrame.RegAX[bp],ax	
	POPALL	
	ret

DeleteService	endp


;--------------------------------------------------------------------
;
; RemoveService	      	Removes service memory and channel from the system.
; =============	
;
;
; Expects:
;	ES:SI : Pointer to initialized ServiceData
;	ES:DI : Pointer to ServiceParam
;			    		
; Returns:
;	AL : 	Status (J_GOODBYE, J_NO_SERVICE)
;
;--------------------------------------------------------------------
;
RemoveService	proc	near
						; free parameter memory
	mov	al,PARA_TYPE
	mov	bx,es:ServiceData.MemOffset[si]
	cmp	bx,-1
	je	RS_FreeChannel
	call	FreeMem				; ignore errors

RS_FreeChannel:

	if 	CHANNELING
	 mov	ah,es:ServiceData.Channel[si]	; free channel
	 xor	al,al
	 mov	cl,3
	 shr    ax,cl
	 mov    bl,ah 				; AL = ChannelMasks bit#
	 xor	bh,bh				; BX = ChannelMasks byte#
	 mov	cl,4
	 shr	al,cl				; adjust bit count
	 mov 	cl,1
	 xchg	al,cl
	 or	cl,cl				; check for bit0
	 jz	RS_Bit0Found
	 shl	al,cl
RS_Bit0Found:
	 not	al
	 and	es:ServiceParam.ChannelMasks[di][bx],al
	endif
						; free service memory
	mov	al,PARA_TYPE
	mov	bx,si
	call	FreeMem				; ignore errors

	ret

RemoveService	endp




;--------------------------------------------------------------------
;
; ServiceDispatch		Handles incomming interrupts of
; ===============		Amiga service requests.
;
;
; Expects:
;	ES:DI : Pointer to ServiceParam
;				    		
; Returns:
;	Nothing
;
;--------------------------------------------------------------------
;
public	        ServiceDispatch


ServiceDispatch	proc	near
		   					; get channel number
	PUSHALL
	INFO	GotServiceInt
	mov	bx,0
	mov	ah,bh

LoopSlot:						; DI = ServiceParam
	if	CHANNELING
	 mov	al,es:ServiceParam.AmigaToPC[di][bx]	; BX = Slot #
	 cmp	al,NO_CHANNEL				; AX = Channel #
	 jne	SignalADD
	 jmp     NextSlot

SignalADD:						; clear slot
	 INFO_AX	ChannelMsg

	 mov	es:ServiceParam.AmigaToPC[di][bx],0
						; added service request ?
	 cmp	al,ADD_SERVICE
	 jne	SignalDELETE
						; inform waiting serviceses
	 ; ...

	 jmp	NextSlot

SignalDELETE:					; removed service request ?
	 cmp	al,DELETE_SERVICE
	 jne	SignalUSER
						; inform other serviceses
	 ; ...

	 jmp	NextSlot

SignalUSER:   					; user request ?
						; find ServiceData structure
	 mov	si,es:ServiceParam.FirstServiceData[di]
	 cmp	si,-1
	 je	SD_ErrService

SD_FindService:					; SI = ServiceData
	 cmp	es:ServiceData.Channel[si],al
	 je	SD_ServiceFound
	 cmp	es:ServiceData.NextServiceData[si],-1
	 je	SD_ErrService
	 mov 	si,es:ServiceData.NextServiceData[si] 
	 jmp	SD_FindService
	else
	 mov	si,es:ServiceParam.AmigaToPC[di][bx]	; BX = Slot #
	 cmp	si,-1					; SI = ServiceData
	 je	NextSlot
	 mov	es:ServiceParam.AmigaToPC[di][bx],-1	; free this slot
	endif
	
SD_ServiceFound:
	mov	ax,si
	INFO_AX	AmigaCallMsg

	call	InformPC
	jmp	short NextSlot

SD_ErrService:
							; print warning
NextSlot:
	INFO	NextSlotMsg

	if	CHANNELING
	 inc	bx
	 cmp	bx,3
	 ja	SD_Exit
	else
	 inc	bx
	 inc	bx
	 cmp	bx,6
	 ja	SignalADD
	endif
 
	jmp	LoopSlot

	if	CHANNELING
	else
SignalADD:		 				; look for added
	 mov	bx,0					;  serviceses
	 cmp	es:ServiceParam.AmigaAddService[di][bx],-1
 	 jne 	ADD_found
	 add	bx,2
	 cmp	es:ServiceParam.AmigaAddService[di][bx],-1
 	 je	SignalDELETE
ADD_found:				       		; clear slot
	 mov	si,es:ServiceParam.AmigaAddService[di][bx]	

	 mov	ax,si
	 INFO_AX AmigaAddMsg

	 mov	es:ServiceParam.AmigaAddService[di][bx],-1
	 call	InformPC				; send message 		      

SignalDELETE:  						; look for deleted
	 mov	bx,0					;  serviceses
	 cmp	es:ServiceParam.AmigaDeleteService[di][bx],-1
 	 jne 	DELETE_found
	 add	bx,2
	 cmp	es:ServiceParam.AmigaDeleteService[di][bx],-1
 	 je	SD_Exit
DELETE_found:				       		; clear slot
	 mov	si,es:ServiceParam.AmigaDeleteService[di][bx]	

	 mov    ax,si
	 INFO_AX AmigaDeleteMsg

	 mov	es:ServiceParam.AmigaDeleteService[di][bx],-1
 	 call	InformPC	      
	endif

SD_Exit:
	INFO	ExitSDMsg
	POPALL
	ret

ServiceDispatch	endp


;--------------------------------------------------------------------
;
; InformPC			Call handler on PC side,
; ========			skips calling, if offset is zero.
;
;
; Expects:
;	ES:SI : Pointer to ServiceParam
;				    		
; Returns:
;	Nothing
;
;--------------------------------------------------------------------
;
InformPC	proc	near

	PUSHAC
	mov 	dx,es:w ServiceData.PC_Customer[si]
	mov	cs:w [HandlerPtr],dx
	mov 	ax,es:w ServiceData.PC_Customer+2[si]
	mov	cs:w [HandlerPtr+2],ax
 
	mov	cx,ax					; save AX for message
 	or	cx,dx					; someone there ?
	jz	InformExit

	if	DBG_HandlerMsg
	 push	ax
	 push	si
	 push 	ds
	 push 	cs
	 pop  	ds
	 mov	si,offset HandlerMsg 		
	 call 	pstrng
	 call 	outhxw					; print segment
	 mov	ax,dx
	 call 	outhxw					; print offset
	 call 	newline			
	 pop	ds
	 pop	si
	 pop	ax
	endif

	PUSHALL
	call	dword ptr cs:[HandlerPtr]		; call service handler
	POPALL

InformExit:
	POPAC
	ret

InformPC	endp


cseg		ends

end

















								   
