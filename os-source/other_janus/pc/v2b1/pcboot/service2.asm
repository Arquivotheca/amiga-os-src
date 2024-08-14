TITLE	SERVICE2  -  COPYRIGHT (C) 1986 - 1988 Commodore Amiga Inc. 
PAGE	60,132	
;*****************************************************************************
;*
;* Janus Services 2.Generation:
;*
;
; New code  :	30-Mar-88 Torsten Burgdorff
; 45.Update :  	 2-Apr-88 TB	add service 10,11,12
; 46.Update :  	10-Apr-88 TB	change structures according to RJ
; 47.Update :	14-Apr-88 TB    change memory locking, add service 13,14
; 49.Update :	26-Apr-88 TB    forget channeling
; 51.Update :	14-Jun-88 TB    fix "Wait-for-service" in service 11
; 53.Update :	23-Jun-88 TB    cleanup and fix service 14
; 54.Update :   24-Jun-88 TB	use new set of include files
; 55.Update :   28-jun-88 TB    add service 15,16
; 56.Update :   18-jul-88 TB    add flags to service 10,11
; 57.Update :   20-jul-88 TB	use memory.inc
; 58.Update :   21-jul-88 TB	RJ fixed UserCount
; 59.Update :   25-jul-88 TB	add Amiga/PC UserCount, fix service 12
; 60.Update :   27-jul-88 TB	fixed Seg/Off order of PCMemPtr	and
;				 update AmigaMemPtr in service 10
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
	
include        	janus\janusvar.inc
include		janus\services.inc
include		janus\memory.inc
include		privserv.inc
include        	macros.inc
include        	vars_ext.inc
include		words.inc
include 	equ.inc
include		debug.inc

;*----------------------------------------------------------------------------
;*
;* InitServicePtr		Looks for a pointer to the shared Service
;* ==============		structures and initialize PC pointers
;*				identically.
;*				
;
; Expects:
;	nothing
;
; Returns:
;	AL 	:	Status (JSERV_OK, JSERV_NOJANUSBASE)
;
;----------------------------------------------------------------------------
;
public	        InitServicePtr

InitServicePtr	proc	near

	PUSHALL
	mov	cx,WAIT_FREE_LOOP
LoopBase:
	mov	al,JSERV_AMIGASERVICE		;* read base of Amiga services
	call	GetBase
	or	al,al
	jne	NoBase
	cmp	di,-1				;* valid base found ?		
	jne	FoundBase
	loop	LoopBase
	mov 	al,JSERV_NOJANUSBASE		;* no => set error and exit
	jmp	NoBase

FoundBase:
	mov	bx,di				;* yes => set PC services base
	mov	al,JSERV_PCSERVICE		;*	 = Amiga services base
	call	SetParam
		
NoBase:
	POPALL
	ret

InitServicePtr	endp


;*----------------------------------------------------------------------------
;*
;* Service 10: AddService	Allocates memory block for a new ServiceData	
;* ======================	structure and introduces this service to the
;*				system.
;*
; Expects:
;	AH	:	JFUNC_ADDSERVICE
;	DS:SI 	:	ApplicationID	 	
;	CX 	: 	LocalID	
;	BX 	:	Size of customers memory
;      	DX 	:   	Type of customers memory (MEMF_PARAMETER, MEMF_BUFFER)	
;	AL	:	Flags (ADDS_LOCKDATA) 
;	ES:DI 	: 	Pointer to service handler
;				    		
; Returns:
;	AL 	:	Status (JSERV_OK, JSERV_NOJANUSMEM, JSERV_NOSERVICE,
;				JSERV_NOJANUSBASE)
;	ES:DI 	:	Pointer to initialized ServiceData
;
;----------------------------------------------------------------------------
;
public	        AddService

AddService	proc	near

	pushall
	mov 	bp,sp
						;* get ServiceParam
	mov	cx,WAIT_FREE_LOOP
AS_LoopBase:
	mov	al,JSERV_AMIGASERVICE
	call	GetBase
	cmp	di,-1				;** valid base found ?		
	jne	AS_LockStruc
	loop	AS_LoopBase
	mov 	al,JSERV_NOJANUSBASE		;** no => set error and exit
	jmp	AS_NoBase

AS_LockStruc: 					; DI = ServiceParam
	JLOCK  	es:ServiceParam.ServiceLock[di]	;* lock ServiceParam structure
		      				;* look for matching service
	mov	si,-1
	cmp	es:ServiceParam.FirstServiceData[di],-1
	je	NoService
	mov	si,es:ServiceParam.FirstServiceData[di]
						; SI = current ServiceData
FindService:					;** check SERVICE_DELETED
	INFO	ServiceSearch
	test	es:ServiceData.sd_Flags[si],SERVICE_DELETED
	jne	NextService	
						;** check ApplicationID
	mov	ax,ss:StackFrame.RegDS[bp]
 	cmp	es:w ServiceData.sd_ApplicationID[si],ax
	jne	NextService
	mov	ax,ss:StackFrame.RegSI[bp]
 	cmp	es:w ServiceData.sd_ApplicationID+2[si],ax
	jne	NextService
						;** check LocalID
	mov	ax,ss:StackFrame.RegCX[bp]
 	cmp	es:ServiceData.sd_LocalID[si],ax
	je	ServiceFound

NextService:					;* numbers didn't match
						;*  => try next SDstruc
	cmp	es:ServiceData.sd_NextServiceData[si],-1
	je	NoService			;* no more => create SDstruc
	mov 	si,es:ServiceData.sd_NextServiceData[si] 
	jmp	FindService

ServiceFound:					;* matching SDstruc found
						;* check UserCounts
	cmp	es:ServiceData.sd_PCUserCount[si],0
	jne	ServiceExists			;** UserCounts = 0 ?
	cmp	es:ServiceData.sd_AmigaUserCount[si],0
	je     	AllocMemory			;**  yes => Update

ServiceExists:
	mov	al,JSERV_NOSERVICE		
	jmp	ErrExist			;**  no  => set error exit

NoService:					;* create new SDstructure
	INFO	MakeService
	mov	bx,ss:StackFrame.RegDS[bp]
	mov	dx,ss:StackFrame.RegSI[bp]
	mov	cx,ss:StackFrame.RegCX[bp]
	call	MakeServiceData			;* allocate ServiceData struc
	or	al,al				;** error?
	jz	AllocMemory			
	jmp	ErrAllocS			;** yes => set error and exit

AllocMemory:					;* allocate Parameter Mem
	INFO	AllocParam
	mov	ax,ss:StackFrame.RegDX[bp]
	xor	ah,ah
	mov	bx,ss:StackFrame.RegBX[bp]
	call	AllocMem
	or	al,al				;** error?
	jz	UpdateParam			
	jmp	ErrAllocP			;** yes => set error and exit

UpdateParam: 					; BX = offset Param
						;* clear this Parameter Mem
	mov	al,0
	cld
	mov	cx,ss:StackFrame.RegBX[bp]
	push	di
	mov	di,bx
	rep	stosb
	pop	di
						;* set ParamSize and Type
	INFO	InitPointers
	mov	ax,ss:StackFrame.RegBX[bp]
	mov	es:ServiceData.sd_MemSize[si],ax
	mov	ax,ss:StackFrame.RegDX[bp]
	mov	es:ServiceData.sd_MemType[si],ax
						;* set MemOffset 
	mov	es:ServiceData.sd_MemOffset[si],bx
						;* set ParamPtr
	mov	es:w ServiceData.sd_PCMemPtr[si],bx
	mov	es:w ServiceData.sd_PCMemPtr+2[si],es
	and	ax,MEM_TYPEMASK			;** get mem type
	cmp	ax,MEMF_PARAMETER		;** we know param seg, but we
	je	Found_PC_Para			;**  have to load buffer seg
	mov	ax,es:JanusAmiga.jbm_8088Segment 
	mov	es:w ServiceData.sd_PCMemPtr+2[si],ax
					       	
Found_PC_Para:					;* calculate AmigaMemPtr
	mov	ax,ss:StackFrame.RegDX[bp]	;** get mem type
						;** get Amiga janus base
	mov	cx,es:w JanusAmiga.jpm_68000Base
	mov	dx,es:w JanusAmiga.jpm_68000Base+low_word
	and	ax,MEM_TYPEMASK	 
	cmp	ax,MEMF_PARAMETER
	je	Found_Para		
	mov	cx,es:w JanusAmiga.jbm_68000Base
	mov	dx,es:w JanusAmiga.jbm_68000Base+low_word
Found_Para:					
	add     dx,bx				;** add offset ServiceParam
	mov	bx,cx
	mov	ax,ss:StackFrame.RegDX[bp]	;** get access type
	and	ax,MEM_ACCESSMASK			
	mov	cx,TYPEACCESSTOADDR		;** convert it into 	
	xchg	ah,al
	shl	ax,cl				;**  Amiga mem offset
	add	bx,ax				;**  and add it to janus base
						;* set AmigaMemPtr
	mov	es:w ServiceData.sd_AmigaMemPtr[si],bx
	mov	es:w ServiceData.sd_AmigaMemPtr+low_word[si],dx
						;* set FirstPCCustomer
	mov	ax,ss:StackFrame.RegDI[bp]
	mov	es:w ServiceData.sd_FirstPCCustomer[si],ax
	mov	ax,ss:StackFrame.RegES[bp]
	mov	es:w ServiceData.sd_FirstPCCustomer+2[si],ax
						;* init customers lock
	mov	ax,ss:StackFrame.RegAX[bp]
	mov	es:ServiceData.sd_ServiceDataLock[si],MemUnLock ; to unlocked
	test	al,ADDS_LOCKDATA
	jz	AS_no_customer_lock
	mov	es:ServiceData.sd_ServiceDataLock[si],MemLock 	; or to locked
		
AS_no_customer_lock:
	INFO	ServiceAdded
						;* send ADDED_SERVICE to Amiga
PAS_not_found:					;** find free slot
	UNLOCK  es:ServiceParam.ServiceLock[di]	;*** unlock structure and
	nop					;***  wait some cycles 
	nop					;***  for the Amiga
	nop
	JLOCK   es:ServiceParam.ServiceLock[di]	;*** lock ServiceParam
						;***   structure
	mov	bx,0				
	cmp	es:ServiceParam.PCAddService[di][bx],-1
 	je 	PAS_found
	add	bx,2
	cmp	es:ServiceParam.PCAddService[di][bx],-1
 	jne	PAS_not_found			;** try again, until free found

PAS_found:					;** free slot found
	mov	es:ServiceParam.PCAddService[di][bx],si
	UNLOCK  es:ServiceParam.ServiceLock[di]	;** unlock structure
	mov	bl,JSERV_AMIGASERVICE
	call	SendJanusInt			;** send AddedService
						;* update PCUserCount
	inc	es:ServiceData.sd_PCUserCount[si]

PrepareReturn:					;* return with pointer
	mov	ss:StackFrame.RegES[bp],es	;*  to ServiceData structure
	mov	ss:StackFrame.RegDI[bp],si
	mov	al,JSERV_OK
	jmp	AS_Exit
	     					
ErrFatal:					; free parameter and
	push	ax  				;  service memory
	mov	al,MEMF_PARAMETER
	mov	bx,es:ServiceData.sd_MemOffset[si]
	call	FreeMem				; ignore errors
	pop	ax	

ErrAllocP:					; free service memory
	push	ax  			     
	mov	al,MEMF_PARAMETER
	mov	bx,si
	call	FreeMem				; ignore errors
	pop	ax	

ErrAllocS:					; free nothing
	mov	ss:StackFrame.RegES[bp],0
	mov	ss:StackFrame.RegDI[bp],0
	
ErrExist:      					; Err status already set
AS_NoBase:
AS_Exit:					;* unlock structure
	UNLOCK  es:ServiceParam.ServiceLock[di]	
	mov	ah,JFUNC_ADDSERVICE
	mov	ss:StackFrame.RegAX[bp],ax	
	POPALL
	ret

AddService	endp

;*----------------------------------------------------------------------------
;*			
;* Service 11: GetService	Looks for an according service and	
;* ======================	connects customer application to
;*				this service.
;*
; Expects:
;	AH	:	JFUNC_GETSERVICE
;	DS:SI 	:	ApplicationID	  	
;	CX 	: 	LocalID	
;	AL	:	Flags (GETS_WAIT, GETS_ALOAD_A) 
;	ES:DI 	: 	Pointer to application
;			0 : Application doesn't support Amiga calls
;	    		
; Returns:
;	AL 	: 	Status (JSERV_OK, JSERV_NOSERVICE)
;	ES:DI 	: 	Pointer to initialized ServiceData
;
;----------------------------------------------------------------------------
;
public	        GetService

GetService	proc	near

	pushall
	mov 	bp,sp
						;* get ServiceParam
	mov	cx,WAIT_FREE_LOOP
GS_LoopBase:
	mov	al,JSERV_AMIGASERVICE
	call	GetBase
	cmp	di,-1				;** valid base found ?		
	jne	GS_LockStruc
	loop	GS_LoopBase
	mov 	al,JSERV_NOJANUSBASE		;** no => set error and exit
	jmp	GS_NoBase

GS_LockStruc: 					; DI = ServiceParam
	JLOCK  	es:ServiceParam.ServiceLock[di]	;* lock ServiceParam structure
		      				;* look for this service
	mov	si,-1
	cmp	es:ServiceParam.FirstServiceData[di],-1
	je	GS_CheckWait
	mov	si,es:ServiceParam.FirstServiceData[di]
						; SI = current ServiceData
GS_FindService:					;** check SERVICE_DELETED
	INFO	ServiceSearch
	test	es:ServiceData.sd_Flags[si],SERVICE_DELETED
	jne	GS_NextService	
						;** check ApplicationID
	mov	ax,ss:StackFrame.RegDS[bp]
 	cmp	es:w ServiceData.sd_ApplicationID[si],ax
	jne	GS_NextService
	mov	ax,ss:StackFrame.RegSI[bp]
 	cmp	es:w ServiceData.sd_ApplicationID+2[si],ax
	jne	GS_NextService
						;** check LocalID
	mov	ax,ss:StackFrame.RegCX[bp]
 	cmp	es:ServiceData.sd_LocalID[si],ax
	je	GS_ServiceFound

GS_NextService:					;* numbers didn't match
						;*  => try next service struc
	cmp	es:ServiceData.sd_NextServiceData[si],-1
	je	GS_CheckWait			 ;* no more => wait flag set?
	mov 	si,es:ServiceData.sd_NextServiceData[si] 
	jmp	GS_FindService

GS_ServiceFound:				;* matching SDstrucure found
						;* check UserCounts
	cmp	es:ServiceData.sd_PCUserCount[si],0
	jne	GS_UpdateUserCount		;** UserCounts > 0 ?
	cmp	es:ServiceData.sd_AmigaUserCount[si],0
	jne     GS_UpdateUserCount		;**  yes => Update
						;* check wait flag 
	test	ss:StackFrame.RegAX[bp],GETS_WAIT
	jne	GS_NewCustomer			;** set => update SDstructure
	jmp	ErrID				;** no  => set error and exit

GS_UpdateUserCount:				;* update UserCount
	inc	es:ServiceData.sd_PCUserCount[si]

GS_NewCustomer:					;* Update SDstructure
	INFO	InitPointers
	mov	ax,ss:StackFrame.RegES[bp]	;** set FirstPCCustomer
	mov	bx,ss:StackFrame.RegDI[bp]
	mov	es:w ServiceData.sd_FirstPCCustomer[si],bx
	mov	es:w ServiceData.sd_FirstPCCustomer+2[si],ax

						;** init customers lock
	mov	es:ServiceData.sd_ServiceDataLock[si],MemUnLock	; to unlocked
	       					
						
	mov	ax,ss:StackFrame.RegAX[bp]	;** set flags
	and	ax,GETS_ALOAD_A			; only one flag defined !
	mov	es:ServiceData.sd_Flags[si],ax							
     						;* return with pointer
	mov	ss:StackFrame.RegES[bp],es	;*  to ServiceData structure
	mov	ss:StackFrame.RegDI[bp],si
	mov	al,JSERV_OK
	jmp	GS_Exit

GS_CheckWait:					;* check wait flag
	test	ss:StackFrame.RegAX[bp],GETS_WAIT
	jne	GS_Wait				;** set => create SDstructure
	jmp	ErrId				;** no  => set error and exit

GS_Wait:					;* create new SDstructure
	INFO	MakeService
	mov	bx,ss:StackFrame.RegDS[bp]
	mov	dx,ss:StackFrame.RegSI[bp]
	mov	cx,ss:StackFrame.RegCX[bp]
	call	MakeServiceData			;** allocate ServiceData struc
	or	al,al				;** error?
;	jz	GS_NewCustomer
	jz	GS_WaitForService
	jmp	ErrExit				;** yes => set error and exit

GS_WaitForService:				;* wait for service come avail.
	INFO	InitWaitPointers
	mov	ax,cs				;** set FirstPCCustomer
	mov	bx,offset WaitS 		;** to WaitHandler
	mov	es:w ServiceData.sd_FirstPCCustomer[si],bx
	mov	es:w ServiceData.sd_FirstPCCustomer+2[si],ax

						;** init customers lock
	mov	es:ServiceData.sd_ServiceDataLock[si],MemUnLock	; to unlocked

	mov	ax,ss:StackFrame.RegAX[bp]	;** set flags
	and	ax,GETS_ALOAD_A			; only one flag defined !
	mov	es:ServiceData.sd_Flags[si],ax							
						
	mov	cs:WaitFlag,0			;** reset wait flag
	UNLOCK  es:ServiceParam.ServiceLock[di]	;** unlock memory and
	sti					;**  reenable interrupts
GS_WaitLoop:					
	nop					;** wait for service come
	cmp	cs:WaitFlag,0			;**  available loop
	je	GS_WaitLoop
	cli					;** we GOT IT
	JLOCK	es:ServiceParam.ServiceLock[di]	;** disable interrupts, lock 
	mov	cs:WaitFlag,0			;** memory and clear wait flag
	INFO	ServiceAvailable
	jmp	GS_UpdateUserCount		;* Update SDstructure

ErrID:
	mov	al,JSERV_NOSERVICE
	
GS_NoBase:
ErrExit:
	mov	ss:StackFrame.RegES[bp],0
	mov	ss:StackFrame.RegDI[bp],0

GS_Exit:					;* unlock structure
	UNLOCK  es:ServiceParam.ServiceLock[di]	
	mov	ah,JFUNC_GETSERVICE
	mov	ss:StackFrame.RegAX[bp],ax	
	POPALL
	ret

GetService	endp

;----------------------------------------------------------------------------

WaitS	proc	far				;* Routine will be called, if  
						;*  Amiga adds this service
	mov  	cs:WaitFlag,-1		      	;** set flag
	ret					;**  and return

WaitS	endp


;*----------------------------------------------------------------------------
;*
;* MakeServiceData		Allocates parameter memory for a ServiceData
;* ===============		structure, clears it, initialize service ID's
;*				and link structure into structure list.
;*				
;
; Expects:
;    	BX:DX 	:	ApplicationID	 	
;	CX 	: 	LocalID	
;	ES:DI 	: 	Pointer to ServiceParam
;	ES:SI 	: 	Pointer to last ServiceData structure
;			SI = -1, if no ServiceData available
; Returns:
;	ES:SI 	: 	Pointer to ServiceData
;
;
;----------------------------------------------------------------------------
;
public	        MakeServiceData


MakeServiceData	proc	near

	push	bx
	push	cx
	push	dx
	push	bp
	mov	bp,bx
						;* allocate JanusMem
	mov	al,MEMF_PARAMETER 		;*  as parameter memory
	mov 	bx,size ServiceData
	call	AllocMem			; BX = offset
	or	al,al
	jz	ClearMem	
	jmp	ErrAllocJ

ClearMem:					;* clear structure
	cld
	push	cx
	push	di
	mov	cx,size ServiceData
	mov	di,bx
	rep	stosb
	pop	di
	pop	cx
			    			;* fill in ID's
	mov	es:w ServiceData.sd_ApplicationID[bx],bp
	mov	es:w ServiceData.sd_ApplicationID+2[bx],dx
	mov	es:ServiceData.sd_LocalID[bx],cx
																		
	cmp	si,-1				;* link this structure into
	je	FirstService			;*  list of SDstructures
	mov	es:ServiceData.sd_NextServiceData[si],bx
	mov	es:ServiceData.sd_NextServiceData[bx],-1
	jmp	ReturnPointer

FirstService:
	mov	es:ServiceParam.FirstServiceData[di],bx
	mov	es:ServiceData.sd_NextServiceData[bx],-1
		
ReturnPointer:					;* return with pointer 
	mov	si,bx			 	;* to this structure
	xor	al,al 				;* set status OK
	jmp	MSD_Exit			 

ErrChannel:					; free structure
	pop	bx				; restore ServiceData offset
	push	ax
	mov	al,MEMF_PARAMETER  		; BX = offset
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


;*----------------------------------------------------------------------------
;*
;* Service 12: CallService	Calls an available service via an given	
;* =======================	pointer into service structure list.
;*
;
; Expects:
;	AH	:	JFUNC_CALLSERVICE
;	DI 	: 	Pointer to initialized ServiceData structure
;				    		
; Returns:
;	AL 	: 	Status (JSERV_OK, JSERV_NOJANUSMEM, JSERV_NOJANUSBASE)
;					
;----------------------------------------------------------------------------
;
public	        CallService

CallService	proc	near

	PUSHALL
	mov 	bp,sp
						;* get ServiceParam
	mov	cx,WAIT_FREE_LOOP
CS_LoopBase:
	mov	al,JSERV_AMIGASERVICE
	call	GetBase
	cmp	di,-1				;* valid base found ?		
	jne	CS_LockStruc
	loop	CS_LoopBase
	mov 	al,JSERV_NOJANUSBASE		;** no => set error and exit
	jmp	CS_NoBase

CS_LockStruc: 					; DI = ServiceParam
	JLOCK  	es:ServiceParam.ServiceLock[di]	;* lock ServiceParam structure
	
	mov	si,ss:StackFrame.RegDI[bp]	;* AmigaUserCount = 0 ?
	cmp	es:ServiceData.sd_AmigaUserCount[si],0
	jne	FindSlot  			;* no  => call Amiga
	call	InformPC			;* yes => call PC and exit
	jmp	CS_Exit					

FindSlot:					;* look for free slot
	mov	cx,WAIT_FREE_LOOP		;*   in PCToAmiga
LoopFree:
	mov	bx,0
NextFree:
	INFO	NextSlotMsg
						;** found free slot
	cmp	es:ServiceParam.PCToAmiga[di][bx],-1
	je      FreeFound			;** yes => fill in pointer 
	inc	bx				;** no  => try next
	inc	bx
	cmp	bx,6
	jna	NextFree
						;** all slots occupied, so
	UNLOCK  es:ServiceParam.ServiceLock[di]	;** unlock structure and wait
	nop					;**  some cycles for the Amiga
	nop
	nop
	JLOCK  	es:ServiceParam.ServiceLock[di]	;** lock ServiceParam struc
	loop	LoopFree			;**  and try again

	mov	al,JSERV_NOJANUSMEM
	jmp	ErrFree

FreeFound:					;* write pointer into free slot
        mov	ax,ss:StackFrame.RegDI[bp]
	mov	es:ServiceParam.PCToAmiga[di][bx],ax

	UNLOCK  es:ServiceParam.ServiceLock[di]	;* unlock SDstructure
						;* send int to Amiga
	INFO_AX CallAmigaService
	mov	bl,JSERV_AMIGASERVICE
	call	SendJanusInt
	mov	al,JSERV_OK

CS_NoBase:
ErrFree:
CS_Exit:
	UNLOCK  es:ServiceParam.ServiceLock[di]	
	mov	ah,JFUNC_CALLSERVICE
	mov	ss:StackFrame.RegAX[bp],ax	
	POPALL	
	ret

CallService	endp


;*----------------------------------------------------------------------------
;*
;* Service 13: ReleaseService		Disconnets from service.	
;* ==========================	
;*
;
; Expects:
;	AH	:	JFUNC_RELEASESERVICE
;	DI 	: 	Pointer to initialized ServiceData structure
;				    		
; Returns:
;	AL 	: 	Status (JSERV_OK, JSERV_NOJANUSBASE)
;
;----------------------------------------------------------------------------
;
public	        ReleaseService

ReleaseService	proc	near
						;* lock ServiceParam structure
	pushall
	mov 	bp,sp
						;* get ServiceParam
	mov	cx,WAIT_FREE_LOOP
RS_LoopBase:
	mov	al,JSERV_AMIGASERVICE
	call  	GetBase
	cmp	di,-1				;** valid base found ?		
	jne	RS_LockStruc
	loop	RS_LoopBase
	mov 	al,JSERV_NOJANUSBASE		;** no => set error and exit
	jmp	RS_NoBase

RS_LockStruc: 					; DI = ServiceParam
	JLOCK  	es:ServiceParam.ServiceLock[di]	;* lock ServiceParam structure
	
	mov	si,ss:StackFrame.RegDI[bp]
	xor	ah,ah				;* check UserCount
	or	al,es:ServiceData.sd_PCUserCount[si]

	INFO_AX	UCountMsg
						;* UserCounts = 0 ?
	or	al,es:ServiceData.sd_AmigaUserCount[si]
	jz	RemoveStruc			;* yes => remove SDstructure		
	mov	ax,0				;* let PCCustomer pointer
						;*  point to zero
	mov	es:w ServiceData.sd_FirstPCCustomer[si],ax	
	mov	es:w ServiceData.sd_FirstPCCustomer+2[si],ax
						;* decrement UserCount
	dec	es:ServiceData.sd_PCUserCount[si]
	jnz	RS_Exit				;* PCUserCount = 0 ?
						;* no  => exit
	cmp	es:ServiceData.sd_AmigaUserCount[si],0
	jnz	RS_Exit				;* AmigaUserCount = 0 ?
						;* no  => exit
	
RemoveStruc:					;* remove SDstructure
	mov	ax,si
	INFO_AX	RemoveMsg

	call	RemoveService			;* free memory and links

RS_NoBase:						
RS_Exit:					;* unlock ServiceParam struc
	UNLOCK  es:ServiceParam.ServiceLock[di]	
	mov	ah,JFUNC_RELEASESERVICE
	mov	ss:StackFrame.RegAX[bp],ax	
	POPALL	
	ret

ReleaseService	endp


;*----------------------------------------------------------------------------
;*
;* Service 14: DeleteService	      	Removes service from the system.
;* =========================	
;*
;
; Expects:
;	AH	:	JFUNC_DELETESERVCIE
;	DI 	: 	Pointer to initialized ServiceData structure
;				    		
; Returns:
;	AL 	: 	Status (JSERV_OK, JSERV_NOJANUSBASE)
;
;----------------------------------------------------------------------------
;
public	        DeleteService

DeleteService	proc	near

	pushall
	mov 	bp,sp
						;* get ServiceParam
	mov	cx,WAIT_FREE_LOOP		
DS_LoopBase:
	mov	al,JSERV_AMIGASERVICE
	call	GetBase
	cmp	di,-1				;** valid base found ?		
	jne	DS_LockStruc
	loop	DS_LoopBase
	mov 	al,JSERV_NOJANUSBASE		;** no => set error
	jmp	DS_NoBase

DS_LockStruc: 					; DI = ServiceParam
	JLOCK  	es:ServiceParam.ServiceLock[di]	;* lock ServiceParam structure
						
	mov	si,ss:StackFrame.RegDI[bp]	;* set SERVICE_DELETED flag
	mov	es:ServiceData.sd_Flags[si],SERVICE_DELETED
							
	xor	ah,ah				
	or	al,es:ServiceData.sd_PCUserCount[si]

	INFO_AX	UCountMsg
						;* decrement UserCount
	dec	es:ServiceData.sd_PCUserCount[si]	
	jnz	Inform_Del			;* PCUserCount = 0 ?	
						;* no => inform Amiga
	cmp	es:ServiceData.sd_AmigaUserCount[si],0	
	jne	Inform_Del			;* AmigaUserCount = 0 ?	
						;* no => inform Amiga
	mov	si,ss:StackFrame.RegDI[bp]	;* remove structure
	mov	ax,si

	INFO_AX	RemoveMsg

	call	RemoveService			;** free memory
	jmp	DS_Exit
						;* inform Amiga
Inform_Del:					;* send SERVICE_DELETED
	INFO	ServiceDeleted			;*  to Amiga

PDS_not_found:					;** find free slot
	UNLOCK  es:ServiceParam.ServiceLock[di]	;*** unlock structure and
	nop	 				;***  wait some cycles 
	nop	 				;***  for the Amiga
	nop					
	JLOCK   es:ServiceParam.ServiceLock[di]	;*** lock ServiceParam
		 				;***   structure
	mov	bx,0
	cmp	es:ServiceParam.PCDeleteService[di][bx],-1
 	je 	PDS_found
	add	bx,2
	cmp	es:ServiceParam.PCDeleteService[di][bx],-1
 	jne	PDS_not_found		    	;** try again, until free found

PDS_found:					;** free slot found
	push	bx
	mov	bx,0				;* is this service ever added 
	cmp	es:ServiceParam.PCAddService[di][bx],si
 	je 	Old_ADD_found
	add	bx,2
	cmp	es:ServiceParam.PCAddService[di][bx],si
 	jne	No_Old_ADD			;* yes => inform Amiga

Old_ADD_found:					;* we are going to delete a	 	
	mov	es:ServiceParam.PCAddService[di][bx],-1	
	pop	bx				;*  service, which never has
	jmp	DS_NoBase			;*  been send to the Amiga
						;*  => set error and exit
No_Old_ADD:
	pop	bx
	mov	es:ServiceParam.PCDeleteService[di][bx],si
	UNLOCK  es:ServiceParam.ServiceLock[di]	;** unlock structure
	mov	bl,JSERV_AMIGASERVICE
	call	SendJanusInt			;** send DeleteService

DS_NoBase:						
DS_Exit:					;* unlock ServiceParam struc
	UNLOCK  es:ServiceParam.ServiceLock[di]	
	mov	ah,JFUNC_DELETESERVICE
	mov	ss:StackFrame.RegAX[bp],ax	
	POPALL	
	ret

DeleteService	endp


;*----------------------------------------------------------------------------
;*
;* RemoveService	      	Removes service memory and unlink structure
;* =============		from service structure list.
;*
;
; Expects:
;	ES:SI : Pointer to initialized ServiceData
;	ES:DI : Pointer to ServiceParam
;			    		
; Returns:
;	AL : 	Status (JSERV_OK)
;
;----------------------------------------------------------------------------
;						
RemoveService	proc	near
						;* unlink from structure list
	cmp	si,es:ServiceParam.FirstServiceData[di]
	je	RS_NoMerge
	mov	bx,es:ServiceParam.FirstServiceData[di]
						; find previous pointer
RS_Next:					 
	cmp 	si,es:ServiceData.sd_NextServiceData[bx] 
	je	RS_Merge
	mov 	bx,es:ServiceData.sd_NextServiceData[bx] 
	jmp 	RS_Next
							
RS_Merge:					; set previous to current
	mov	ax,es:ServiceData.sd_NextServiceData[si]    
	mov 	es:ServiceData.sd_NextServiceData[bx],ax    
	jmp	RS_FreeMem

RS_NoMerge:
	mov	es:ServiceParam.FirstServiceData[di],-1
	
RS_FreeMem:					;* free parameter memory
	mov	al,MEMF_PARAMETER
	mov	bx,es:ServiceData.sd_MemOffset[si]
	cmp	bx,-1
	je	RS_FreeServiceStructure
	call	FreeMem				; ignore errors

RS_FreeServiceStructure:	 	  	;* free service memory
	mov	al,MEMF_PARAMETER
	mov	bx,si
	call	FreeMem				; ignore errors

	ret

RemoveService	endp


;*----------------------------------------------------------------------------
;*
;* ServiceDispatch		Handles incomming interrupts of
;* ===============		Amiga service requests.
;*
;
; Expects:
;	ES:DI : Pointer to ServiceParam
;				    		
; Returns:
;	Nothing
;
;----------------------------------------------------------------------------
;
public	        ServiceDispatch


ServiceDispatch	proc	near

	PUSHALL
	INFO	GotServiceInt
	mov	bx,0
	mov	ah,bh
							;* look for used slot
LoopSlot:						; DI = ServiceParam
	mov	si,es:ServiceParam.AmigaToPC[di][bx]	; BX = Slot #
	cmp	si,-1					; SI = ServiceData
							;** found any ?	
	je	NextSlot				;** no  => check next
	mov	es:ServiceParam.AmigaToPC[di][bx],-1	;** yes => free slot
	
SD_ServiceFound:
	mov	ax,si
	INFO_AX	AmigaCallMsg

	call	InformPC			 ;** and call handler on PC

NextSlot:
	INFO	NextSlotMsg

	inc	bx
	inc	bx
	cmp	bx,6
	ja	SignalADD
	jmp	LoopSlot

SignalADD:		 			;* look for added services
	mov	bx,0			      
	cmp	es:ServiceParam.AmigaAddService[di][bx],-1
 	jne 	ADD_found
	add	bx,2				;** added found	?
	cmp	es:ServiceParam.AmigaAddService[di][bx],-1
 	je	SignalDELETE			;** no  => check deleted
ADD_found:				       	;** yes => clear slot
	mov	si,es:ServiceParam.AmigaAddService[di][bx]	

	mov	ax,si
	INFO_AX AmigaAddMsg

	mov	es:ServiceParam.AmigaAddService[di][bx],-1
	call	InformPC			;** and call PC handler 		      

SignalDELETE:  					;* look for deleted services
	mov	bx,0			       
	cmp	es:ServiceParam.AmigaDeleteService[di][bx],-1
 	jne 	DELETE_found
	add	bx,2				;** deleted found ?
	cmp	es:ServiceParam.AmigaDeleteService[di][bx],-1
 	je	SD_Exit				;** no  => exit

DELETE_found:				       	;** yes => clear slot
	mov	si,es:ServiceParam.AmigaDeleteService[di][bx]	

	mov    ax,si
	INFO_AX AmigaDeleteMsg
	
	mov	es:ServiceParam.AmigaDeleteService[di][bx],-1
 	call	InformPC	      		 ;** and call PC handler

SD_Exit:
	INFO	ExitSDMsg
	POPALL
	ret

ServiceDispatch	endp


;*----------------------------------------------------------------------------
;*
;* InformPC			Call handler on PC side,
;* ========			skips calling, if offset is zero.
;*
;
; Expects:
;	ES:SI : Pointer to ServiceParam
;				    		
; Returns:
;	Nothing
;
;----------------------------------------------------------------------------
;
InformPC	proc	near

	PUSHAC
	mov 	dx,es:w ServiceData.sd_FirstPCCustomer[si]
	mov	cs:w [HandlerPtr],dx
	mov 	ax,es:w ServiceData.sd_FirstPCCustomer+2[si]
	mov	cs:w [HandlerPtr+2],ax
 
	mov	cx,ax				; save AX for message
 	or	cx,dx				;* Handler = 0 ?
	jz	InformExit			;* yes => exit

	if	DBG_HandlerMsg
	 push	ax
	 push	si
	 push 	ds
	 push 	cs
	 pop  	ds
	 mov	si,offset HandlerMsg 		
	 call 	pstrng
	 call 	outhxw				; print segment
	 mov	ax,dx
	 call 	outhxw				; print offset
	 call 	newline			
	 pop	ds
	 pop	si
	 pop	ax
	endif

	PUSHALL
	call	dword ptr cs:[HandlerPtr]	;* no  => call PC handler
	POPALL

InformExit:
	POPAC
	ret

InformPC	endp


;*----------------------------------------------------------------------------
;*
;* Service 15: LockServiceData	Sets LOCK byte in according ServiceStructure	
;* ===========================	to prevent the other side trashing the shared  
;*				memory buffer.
;*
; Expects:
;	AH	:	JFUNC_LOCKSERVICEDATA
;	DI 	: 	Pointer to initialized ServiceData structure	
;				    		
; Returns:
;	AL 	:	Status (JSERV_OK)
;
;----------------------------------------------------------------------------
;
public	        LockServiceData

LockServiceData	proc	near

	push	ax
	push	es
 						
	mov	ax,cs:janus_param_seg 		;* get segment of parameter
	mov    	es,ax				;*  memory      
	JLOCK  	es:ServiceData.sd_ServiceDataLock[di]	;* lock ServiceData

	pop	es
	pop	ax
	mov     al,JSERV_OK
	ret

LockServiceData	endp

	
;*----------------------------------------------------------------------------
;*
;* Service 16: UnlockServiceData		Resets LOCK byte in according	
;* =============================		ServiceStructure.
;*
; Expects:
;	AH	:	JFUNC_UNLOCKSERVICEDATA
;	DI 	: 	Pointer to initialized ServiceData structure	
;				    		
; Returns:
;	AL 	:	Status (JSERV_OK)
;
;----------------------------------------------------------------------------
;
public	        UnlockServiceData

UnlockServiceData	proc	near

	push	ax
	push	es

	mov	ax,cs:janus_param_seg 		;* get segment of parameter 
	mov    	es,ax				;*  memory
	UNLOCK 	es:ServiceData.sd_ServiceDataLock[di]	;* lock ServiceData

	pop	es
	pop	ax
	mov     al,JSERV_OK
	ret

UnlockServiceData	endp

	
cseg		ends

end

















								   
