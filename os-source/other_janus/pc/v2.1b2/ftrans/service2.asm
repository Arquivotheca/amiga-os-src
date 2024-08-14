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
; 63.Update :    6-oct-88 TB	implement PCWait flag
; 64.Update :	28-jul-89 TB	implement 'zero mem alloc' in AddService
; 65.Update :   02-Aug-89 BK	Fixed remove service to use the MemType
;				instead of MEMF_PARMTER when freeing
;				service mem.
; 66.Update :	10-Aug-89 BK	Changed MakeServiceData to init the lock
;				to 7f and JRememberKey to -1
; 67.Update :	10-Aug-89 BK	Changed AddService to set mem ptrs to NULL
;				if no mem allocated.
; 68.Update :	24-Aug-89 BK	Fixed AddService() to set the SERVICE_ADDED
;				flag in sd_Flags so the Amiga can do 
;				GETS_WAIT
; 69.Update :	28-Aug-89 BK	Fixed Release Service to return the 
;				sd_FirstPCCustomer field to -1 rather than 0
; 70.Update :	28-Aug-89 BK	Fixed Delete Service to return the 
;				sd_FirstPCCustomer field to -1 rather than 
;				leaving it at some random value.
; 71.Update :	30-Oct-89 TB	Added AutoLoad function to GetService 
;		19-feb-91 rsd	fixed GetService retval in case of nonzer
;				customer counts (B10301)
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
	
include 	equ.inc
include        	janus\janusvar.inc
include		janus\services.inc
include		janus\memory.inc
include		autoload.inc
include		privserv.inc
include        	macros.inc
include        	vars_ext.inc
include		words.inc
include		debug.inc

.list

HARDWARE_LOCK	=	0	;1 for hardware 0 for software
LOCK_ATTEMPT	=	0	;1 for lockateempt 0 for inline

;****i* JanusHandler/InitServicePtr *****************************************
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
;	AL 	:	Status (JSERV_OK, JSERV_NOJANUSBASE)
;
;***************************************************************************
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

;****** JanusHandler/AddService ******************************************
;
;   NAME
;	AddService -- Add a service to the system.
;
;   SYNOPSIS
;	mov	AH,Function
;	mov	DS,ApplicationID_high_word
;	mov	SI,ApplicationID_low_word
;	mov	CX,LocalID
;	mov	BX,Size
;	mov	DX,Type
;	mov	AL,Flags
;	mov	ES,Handler_Segment
;	mov	DI,Handler_Offset
;	int	0bh
;
;   FUNCTION
;	Adds a service to the system.
;
;   INPUTS
;	AH = Function
;
;	DS = High word of 32 bit application ID
;
;	SI = Low word of application ID
;
;	CX = Local ID
;
;	BX = Size in bytes of memory to allocate
;
;	DX = Type of memory to allocate (MEMF_BUFFER, MEMF_PARAMETER)
;
;	AL = AddService flags from services.[hi]
;
;	ES = Segment of assembly language routine to call when this service
;	     is called or NULL.
;
;	DI = Offset of assembly language routine to call when this service
;	     is called or NULL.
;
;   RESULT
;	AL = Status (JSERV_OK, JSERV_NOJANUSMEM, JSERV_NOSERVICE,
;			JSERV_NOJANUSBASE)
;
;	ES = Segment of ServiceData structure
;
;	DI = Offset of ServiceData structure
;
;   EXAMPLE
;	mov	AH,JFUNC_ADDSERVICE
;	mov	DS,ApplicationID_high_word
;	mov	SI,ApplicationID_low_word
;	mov	CX,LocalID
;	mov	BX,Size
;	mov	DX,MEMF_BUFFER
;	mov	AL,ADDS_EXCLUSIVE | ADDS_TOAMIGA_ONLY
;	mov	ES,Handler_Segment
;	mov	DI,Handler_Offset
 ;	int	JFUNC_JINT
;
;   NOTES
;
;   BUGS
;
;   SEE ALSO
;	CallService(), DeleteService(), GetService(), ReleaseService()
;
;****************************************************************************
;
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
;	JLOCK  	es:ServiceParam.ServiceLock[di]	;* lock ServiceParam structure
	push	di
	lea	di,es:ServiceParam.ServiceLock[di]
	call	JanusLock
	pop	di
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
	mov	al,JSERV_DUPSERVICE		
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
	or 	cx,cx				; check for zero
	jz	Skip_Clear
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
Skip_Clear:
	mov	ax,ss:StackFrame.RegDI[bp]
	mov	es:w ServiceData.sd_FirstPCCustomer[si],ax
	mov	ax,ss:StackFrame.RegES[bp]
	mov	es:w ServiceData.sd_FirstPCCustomer+2[si],ax
						;* init customers lock
	mov	ax,ss:StackFrame.RegAX[bp]
;	mov	es:ServiceData.sd_ServiceDataLock[si],MemUnLock ; to unlocked
	test	al,ADDS_LOCKDATA
	jz	AS_no_customer_lock
	mov	es:ServiceData.sd_ServiceDataLock[si],MemLock 	; or to locked
		
AS_no_customer_lock:
	INFO	ServiceAdded

	mov	es:ServiceData.sd_Flags[si],SERVICE_ADDED
						;* send ADDED_SERVICE to Amiga
PAS_not_found:					;** find free slot
;	UNLOCK  es:ServiceParam.ServiceLock[di]	;*** unlock structure and
	push	di
	lea	di,es:ServiceParam.ServiceLock[di]
	call	JanusUnlock
	pop	di
;	nop					;***  wait some cycles 
;	nop					;***  for the Amiga
;	nop
;	nop
;	nop
;	nop
;	JLOCK   es:ServiceParam.ServiceLock[di]	;*** lock ServiceParam
	push	di
	lea	di,es:ServiceParam.ServiceLock[di]
	call	JanusLock
	pop	di
						;***   structure
	mov	bx,0				
	cmp	es:ServiceParam.PCAddService[di][bx],-1
 	je 	PAS_found
	add	bx,2
	cmp	es:ServiceParam.PCAddService[di][bx],-1
 	jne	PAS_not_found			;** try again, until free found

PAS_found:					;** free slot found
	mov	es:ServiceParam.PCAddService[di][bx],si
;	UNLOCK  es:ServiceParam.ServiceLock[di]	;** unlock structure
	push	di
	lea	di,es:ServiceParam.ServiceLock[di]
	call	JanusUnlock
	pop	di
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
	INFO	ADDS_EXIT
;	UNLOCK  es:ServiceParam.ServiceLock[di]	
	push	di
	lea	di,es:ServiceParam.ServiceLock[di]
	call	JanusUnlock
	pop	di
	mov	ah,JFUNC_ADDSERVICE
	mov	ss:StackFrame.RegAX[bp],ax	
	POPALL
	ret

AddService	endp
;****** JanusHandler/GetService ******************************************
;
;   NAME
;	GetService -- Get a service and its ServiceData pointer
;
;   SYNOPSIS
;	mov	AH,Function
;	mov	DS,ApplicationID_high_word
;	mov	SI,ApplicationID_low_word
;	mov	CX,LocalID
;	mov	AL,Flags
;	mov	ES,Handler_Segment
;	mov	DI,Handler_Offset
;	int	0bh
;
;   FUNCTION
;	Gets a pointer to a Services ServiceData structure if the Service
;	exists.
;
;   INPUTS
;	AH = Function
;
;	DS = High word of 32bit application ID
;
;	SI = Low word of 32bit application ID
;
;	CX = Local ID
;
;	AL = Flags
;	
;	ES = Segment of assembly language handler to be called whenever this
;	     service is called or NULL.
;
;	DI = Offset of assembly language handler to be called whenever this
;	     service is called or NULL.
;
;   RESULT
;	AL = Status (JSERV_OK, JSERV_NOSERVICE)
;
;	ES = Segment of ServiceData structure
;
;	DI = Offset of ServiceData structure
;
;   EXAMPLE
;	mov	ax,ApplicationID_high_word
;	mov	DS,ax
;	mov	SI,ApplicationID_low_word
;	mov	CX,LocalID
;	mov	ax,Handler_Segment
;	mov	ES,ax
;	mov	DI,Handler_Offset
;	mov	AH,JFUNC_GETSERVICE
;	mov	AL,GETS_WAIT
;	int	JFUNC_JINT
;
;   NOTES
;
;   BUGS
;
;   SEE ALSO
;	AddService(), CallService(), DeleteService(), ReleaseService()
;
;****************************************************************************
;
;
public	        GetService

GetService	proc	near

	pushall
	mov 	bp,sp
	   
	;**********************************************
	;* Get Pointer to ServiceParam struc in es:di *
	;**********************************************
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
	;*********************
	;* Lock ServiceParam *
	;*********************
	push	di
	lea	di,es:ServiceParam.ServiceLock[di]
	call	JanusLock
	pop	di

	;*******************************************
	;* No services so check for GETS_WAIT flag *
	;*******************************************
	mov	si,-1
	cmp	es:ServiceParam.FirstServiceData[di],-1
	je	GS_CheckWait

	;************************************************************
	;* Loop through existing services ignoring deleted services *
	;************************************************************
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

	;**************************
	;* Found existing service *
	;**************************
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

GS_CheckWait:					;* check wait flag
	test	ss:StackFrame.RegAX[bp],GETS_WAIT
	jne	GS_Wait				;** set => create SDstructure
	jmp	ErrId				;** no  => set error and exit

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
	       					
;	mov	ax,ss:StackFrame.RegAX[bp]	;** set flags
;	and	ax,GETS_ALOAD_A			; only one flag defined !
;	mov	es:ServiceData.sd_Flags[si],ax							
     						;* return with pointer
	mov	ss:StackFrame.RegES[bp],es	;*  to ServiceData structure
	mov	ss:StackFrame.RegDI[bp],si
	mov	al,JSERV_OK
	jmp	GS_Exit

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
;	mov	es:ServiceData.sd_ServiceDataLock[si],MemUnLock	; to unlocked

;	mov	ax,ss:StackFrame.RegAX[bp]	;** set flags
;	and	ax,GETS_ALOAD_A			; only one flag defined !
;	mov	es:ServiceData.sd_Flags[si],ax							
						
	mov	cs:WaitFlag,0			;** reset wait flag
;	UNLOCK  es:ServiceParam.ServiceLock[di]	;** unlock memory and
	push	di
	lea	di,es:ServiceParam.ServiceLock[di]
	call	JanusUnlock
	pop	di
	sti					;**  reenable interrupts

	mov	ax,WAIT_Timeout*18
	test	ss:StackFrame.RegAX[bp],GETS_ALOAD_A	; check for AutoLoad
;	je 	GS_WaitLoop
	jne	GS_Around
	jmp	GS_WaitLoop
GS_Around:			
	;*******************************
	;* GetService() the autoloader *
	;*******************************
;	INFO	AutoLoadService			; AutoLoad
	push	di
	push	si
	push	es
	mov	ax,AUTOLOAD_APPID_H
	mov	DS,ax
	mov	SI,AUTOLOAD_APPID_L
	mov	CX,AUTOLOAD_LocalID
	mov	AX,0		      		; no flags!  Wait for service
	mov	ES,ax				;	     is NOT reentrant
	mov	DI,0
	call	GetService			; get the AutoLoad service
	or	al,al				; exit directly with error code
	jnz	AL_ErrExit

	push	ax
	mov	al,02h
	call	JanusDebug
	pop	ax

	;********************************************************
	;* Find empty autoload field and fill with App/Local ID *
	;********************************************************
	push 	ds
	push	di
	push	ax
;	mov	ax,0d400h
	mov	ax,word ptr es:ServiceData.sd_PCMemPtr+2[di]
	mov	ds,ax
	pop	ax
	mov	si,es:ServiceData.sd_MemOffset[di]
	mov	cx,ds:AutoLoad.al_entries[si] 	; look for free entry in AutoLoad struc
Free_Loop:
	cmp	word ptr ds:AutoLoad.al1_AppID[si],0	
	jne	Not_Free
	cmp	word ptr ds:AutoLoad.al1_AppID[si+2],0	
	je	Free_Found
Not_Free:
	add	si,(size AutoLoad.al1_AppID)+(size AutoLoad.al1_LocalID)			
	loop	Free_Loop
	pop	di
	pop	ds
	jmp	AL_ErrExit	     		; no free entry found
Free_Found:
	push 	ax
	mov	ax,ss:StackFrame.RegDS[bp]	
	mov	word ptr ds:AutoLoad.al1_AppID[si],ax
	mov	ax,ss:StackFrame.RegSI[bp]	
	mov	word ptr ds:AutoLoad.al1_AppID[si+2],ax
	mov	ax,ss:StackFrame.RegCX[bp]	
	mov	word ptr ds:AutoLoad.al1_LocalID[si],ax
	pop	ax
	pop	di
	pop	ds


;	mov	cx,es:AutoLoad.al_entries[di] 	; look for free entry in AutoLoad struc
;Free_Loop:
;	cmp	word ptr es:AutoLoad.al1_AppID[di],0	
;	jne	Not_Free
;	cmp	word ptr es:AutoLoad.al1_AppID[di+2],0	
;	je	Free_Found
;Not_Free:
;	add	di,(size AutoLoad.al1_AppID)+(size AutoLoad.al1_LocalID)			
;	loop	Free_Loop
;	jmp	AL_ErrExit	     		; no free entry found
;
;Free_Found:	       				; set fields in Autoload structure
;	pop	ds
;	pop	si
;
;	push	di
;	lea	di,ds:ServiceData.sd_ServiceDataLock[si]
;	call	JanusLock
;	pop	di
;
;	mov	word ptr es:AutoLoad.al1_AppID[di],1	
;	mov	word ptr es:AutoLoad.al1_AppID[di+2],0	
;	mov	word ptr es:AutoLoad.al1_LocalID[di],1	
;
;	push	di
;	lea	di,ds:ServiceData.sd_ServiceDataLock[si]
;	call	JanusUnlock
;	pop	di
;
;	push	si
;	push	ds
	call	CallService			; call AutoLoad service
						;  and forget about error codes
	call	ReleaseService			; release from AutoLoad service
	or	al,al				; exit with error code, if we
				   		;  couldn't release successfully
AL_ErrExit:
	push	ax
	mov	al,01h
	call	JanusDebug
	pop	ax
	call	JanusDebug

	pop	es		  
	pop	si
	pop	di		
	or 	al,al
	jnz	ErrExit
	mov	ax,ALOAD_Timeout*18

GS_WaitLoop:					
	INFO	AutoLoadWait			; wait for a fixed amount
	push	ax
	mov	ax,BD_seg			;  of time
	mov	ds,ax
	pop	ax
	mov	cx,[clock] 			; use BIOS chiffy clock
AL_Wait:
	sti
	cmp	cs:WaitFlag,0			; exit wait loop, if service
	jne	GS_WaitEnd			;  comes available
	mov	bx,[clock]
	sub	bx,cx
	cmp	bx,ax				; timeout reached ?
	jb	AL_Wait
	jmp	ErrID				; timeout error 

GS_WaitEnd:
;	nop					;** wait for service come
;	cmp	cs:WaitFlag,0			;**  available loop
;	je	GS_WaitLoop
	cli					;** we GOT IT
;	JLOCK	es:ServiceParam.ServiceLock[di]	;** disable interrupts, lock 
	push	di
	lea	di,es:ServiceParam.ServiceLock[di]
	call	JanusLock
	pop	di

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
	INFO	LeaveGetService
;	UNLOCK  es:ServiceParam.ServiceLock[di]	
	push	di
	lea	di,es:ServiceParam.ServiceLock[di]
	call	JanusUnlock
	pop	di
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

	mov	es:ServiceData.sd_ServiceDataLock[bx],07fh
;	mov	es:ServiceData.sd_JRememberKey[bx],-1
																		
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

;****** JanusHandler/CallService ******************************************
;
;   NAME
;	CallService -- Signal all other users of a service.
;
;   SYNOPSIS
;	mov	AH,Function
;	mov	DI,ServiceData_Offset
;	int	0bh
;
;   FUNCTION
;	To signal all users of a service except the one that called
;	CallService.
;
;   INPUTS
;	AH = Function
;
;	DI = Offset of the services ServiceData structure.
;
;   RESULT
;	AL = Status (JSERV_OK, JSERV_NOJANUSMEM, JSERV_NOJANUSBASE)
;
;   EXAMPLE
;	mov	AH,JFUNC_CALLSERVICE
;	mov	DI,ServiceData_Offset
;	int	JFUNC_JINT
;
;   NOTES
;
;   BUGS
;
;   SEE ALSO
;	AddService(), DeleteService(), GetService(), ReleaseService()
;
;****************************************************************************
;
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
;	JLOCK  	es:ServiceParam.ServiceLock[di]	;* lock ServiceParam structure
	push	di
	lea	di,es:ServiceParam.ServiceLock[di]
	call	JanusLock
	pop	di
	
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
;	UNLOCK  es:ServiceParam.ServiceLock[di]	;** unlock structure and wait
	push	di
	lea	di,es:ServiceParam.ServiceLock[di]
	call	JanusUnlock
	pop	di
;	nop					;**  some cycles for the Amiga
;	nop
;	nop
;	JLOCK  	es:ServiceParam.ServiceLock[di]	;** lock ServiceParam struc
	push	di
	lea	di,es:ServiceParam.ServiceLock[di]
	call	JanusLock
	pop	di	

	loop	LoopFree			;**  and try again

	mov	al,JSERV_NOJANUSMEM
	jmp	ErrFree

FreeFound:					;* write pointer into free slot
        mov	ax,ss:StackFrame.RegDI[bp]
	mov	es:ServiceParam.PCToAmiga[di][bx],ax
						;* set PCWAIT flag
	or	es:ServiceData.sd_Flags[si],SERVICE_PCWAIT
	   					
;	UNLOCK  es:ServiceParam.ServiceLock[di]	;* unlock SDstructure
;;	push	di
;;	lea	di,es:ServiceParam.ServiceLock[di]
;;	call	JanusUnlock
;;	pop	di
						
	INFO_AX CallAmigaService
	mov	bl,JSERV_AMIGASERVICE		;* send int to Amiga
	call	SendJanusInt
	mov	al,JSERV_OK

ErrFree:
CS_Exit:
;	UNLOCK  es:ServiceParam.ServiceLock[di]	
	push	di
	lea	di,es:ServiceParam.ServiceLock[di]
	call	JanusUnlock
	pop	di


CS_NoBase:
	mov	ah,JFUNC_CALLSERVICE
	mov	ss:StackFrame.RegAX[bp],ax	
	POPALL	
	ret

CallService	endp

;****** JanusHandler/ReleaseService ******************************************
;
;   NAME
;	ReleaseService -- Release a service previously obtained with 
;			  GetService
;
;   SYNOPSIS
;	mov	AH,Function
;	mov	DI,ServiceData_Offset
;	int	0bh
;
;   FUNCTION
;	To Release use of a service previously gotten via GetService
;
;   INPUTS
;	AH = Function
;
;	DI = Offset of ServiceData structure
;
;   RESULT
;	AL = Status (JSERV_OK, JSERV_NOJANUSBASE)
;
;   EXAMPLE
;	mov	AH,JFUNC_RELEASESERVICE
;	mov	DI,ServiceData_Offset
;	int	JFUNC_JINT
;
;   NOTES
;
;   BUGS
;
;   SEE ALSO
;	AddService(), CallService, DeleteService(), GetService()
;
;****************************************************************************
;
;
public	        ReleaseService

ReleaseService	proc	near
						;* lock ServiceParam structure
	pushall
	mov 	bp,sp
	INFO	RS_Entry
						;* get ServiceParam
	mov	cx,WAIT_FREE_LOOP
RS_LoopBase:
	mov	al,JSERV_AMIGASERVICE
	call  	GetBase
	cmp	di,-1				;** valid base found ?		
	jne	RS_LockStruc
	loop	RS_LoopBase
	mov 	al,JSERV_NOJANUSBASE		;** no => set error and exit
	jmp	RS_Exit

RS_LockStruc: 					; DI = ServiceParam
;	JLOCK  	es:ServiceParam.ServiceLock[di]	;* lock ServiceParam structure
	push	di
	lea	di,es:ServiceParam.ServiceLock[di]
	call	JanusLock
	pop	di
	
 	mov	si,ss:StackFrame.RegDI[bp]
;	xor	ah,ah				;* check UserCount
;	or	al,es:ServiceData.sd_PCUserCount[si]
	mov	al,es:ServiceData.sd_PCUserCount[si]

	INFO_AX	UCountMsg
						;* UserCounts = 0 ?
	or	al,es:ServiceData.sd_AmigaUserCount[si]
	jz	RemoveStruc			;* yes => remove SDstructure		
	mov	ax,-1				;* let PCCustomer pointer
						;*  point to zero
	mov	es:w ServiceData.sd_FirstPCCustomer[si],ax	
	mov	es:w ServiceData.sd_FirstPCCustomer+2[si],ax
						;* decrement UserCount
	dec	es:ServiceData.sd_PCUserCount[si]
	jnz	RS_OKExit				;* PCUserCount = 0 ?
						;* no  => exit
	cmp	es:ServiceData.sd_AmigaUserCount[si],0
	jnz	RS_OKExit				;* AmigaUserCount = 0 ?
						;* no  => exit
	
RemoveStruc:					;* remove SDstructure
	mov	ax,si
	INFO_AX	RemoveMsg

	call	RemoveService			;* free memory and links
	jmp	RS_Exit

RS_OKExit:	     
	mov	al,JSERV_OK
RS_Exit:						
;	UNLOCK  es:ServiceParam.ServiceLock[di]	
	push	di
	lea	di,es:ServiceParam.ServiceLock[di]
	call	JanusUnlock
	pop	di
	mov	ah,JFUNC_RELEASESERVICE
	mov	ss:StackFrame.RegAX[bp],ax	
	POPALL	
	ret

ReleaseService	endp

;****** JanusHandler/DeleteService ******************************************
;
;   NAME
;	DeleteService -- Delete a service previously added with AddService
;
;   SYNOPSIS
;	mov	AH,Function
;	mov	DI,ServiceData_Offset
;	int	0bh
;
;   FUNCTION
;	To Delete a service previously added with AddService
;
;   INPUTS
;	AH = Function
;
;	DI = Offset of ServiceData structure
;
;   RESULT
;	AL = Status (JSERV_OK, JSERV_NOJANUSBASE)
;
;   EXAMPLE
;	mov	AH,JFUNC_DELETESERVICE
;	mov	DI,ServiceData_Offset
;	int	JFUNC_JINT
;
;   NOTES
;
;   BUGS
;
;   SEE ALSO
;	AddService(), CallService(), GetService(), ReleaseService()
;
;****************************************************************************
;
;
public	        DeleteService

DeleteService	proc	near

	pushall
	mov 	bp,sp
						;* get ServiceParam
	INFO	DeleteServiceEnter
	
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

	INFO	DeleteServiceBase	

;	JLOCK  	es:ServiceParam.ServiceLock[di]	;* lock ServiceParam structure
	push	di
	lea	di,es:ServiceParam.ServiceLock[di]
	call	JanusLock
	pop	di
	       
	INFO	DeleteServiceLock
					
	mov	si,ss:StackFrame.RegDI[bp]	;* set SERVICE_DELETED flag
	mov	es:ServiceData.sd_Flags[si],SERVICE_DELETED
		  
	;reset to -1 BK	  
	push	ax
	mov	ax,-1
	mov	es:w ServiceData.sd_FirstPCCustomer[si],ax
	mov	es:w ServiceData.sd_FirstPCCustomer+2[si],ax
	pop	ax
				
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
;	UNLOCK  es:ServiceParam.ServiceLock[di]	;*** unlock structure and
	push	di
	lea	di,es:ServiceParam.ServiceLock[di]
	call	JanusUnlock
	pop	di
;	nop	 				;***  wait some cycles 
;	nop	 				;***  for the Amiga
;	nop					
;	JLOCK   es:ServiceParam.ServiceLock[di]	;*** lock ServiceParam
	push	di
	lea	di,es:ServiceParam.ServiceLock[di]
	call	JanusLock
	pop	di
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
;	UNLOCK  es:ServiceParam.ServiceLock[di]	;** unlock structure
	push	di
	lea	di,es:ServiceParam.ServiceLock[di]
	call	JanusUnlock	
	pop	di
	mov	bl,JSERV_AMIGASERVICE
	call	SendJanusInt			;** send DeleteService

DS_NoBase:						
DS_Exit:					;* unlock ServiceParam struc
;	UNLOCK  es:ServiceParam.ServiceLock[di]	
	push	di
	lea	di,es:ServiceParam.ServiceLock[di]
	call	JanusUnlock
	pop	di
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
	mov	ax,es:ServiceData.sd_MemType[si]
	mov	bx,es:ServiceData.sd_MemOffset[si]
	cmp	bx,0
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
	mov    	ax,si 				
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
;* ========			skips calling, if offset is zero or -1.
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
						;* reset PCWait flag
	and	es:ServiceData.sd_Flags[si],not SERVICE_PCWAIT
						;* load pointer to handler
	mov 	dx,es:w ServiceData.sd_FirstPCCustomer[si]
	mov	cs:w [HandlerPtr],dx
	mov 	ax,es:w ServiceData.sd_FirstPCCustomer+2[si]
	mov	cs:w [HandlerPtr+2],ax
 
	mov	cx,ax				; save AX for message
 	or	cx,dx				;* Handler = 0 ?
	jz	InformExit			;* yes => exit
	mov	cx,ax				
 	and	cx,dx				
	xor	cx,-1				;* Handler = -1 ?
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

;****** JanusHandler/LockServiceData ******************************************
;
;   NAME
;	LockServiceData -- Perform the steps necessary to Lock a ServiceData
;			   structure.
;
;   SYNOPSIS
;	mov	AH,Function
;	mov	DI,ServiceData_Offset
;	int	0bh
;
;   FUNCTION
;	Performs the necessary steps to Lock a ServiceData structure for
;	exclusive access.
;
;   INPUTS
;	AH = Function
;	
;	DI = Offset of ServiceData structure
;
;   RESULT
;	AL = Status (JSERV_OK)
;
;   EXAMPLE
;	mov	AH,JFUNC_LOCKSERVICEDATA
;	mov	DI,ServiceData_Offset
;	int	JFUNC_JINT
;
;   NOTES
;
;   BUGS
;
;   SEE ALSO
;	UnlockServiceData()
;
;****************************************************************************
;
;
public	        LockServiceData

LockServiceData	proc	near

	push	ax
	push	es
 						
	mov	ax,cs:janus_param_seg 		;* get segment of parameter
	mov    	es,ax				;*  memory      
;	JLOCK  	es:ServiceData.sd_ServiceDataLock[di]	;* lock ServiceData
	push	di
	lea	di,es:ServiceData.sd_ServiceDataLock[di]
	call	JanusLock
	pop	di

	pop	es
	pop	ax
	mov     al,JSERV_OK
	ret

LockServiceData	endp

	
;****** JanusHandler/UnLockServiceData ******************************************
;
;   NAME
;	UnLockServiceData -- Perform the steps necessary to UnLock a 
;			     ServiceData structure.
;
;   SYNOPSIS
;	mov	AH,Function
;	mov	DI,ServiceData_Offset
;	int	0bh
;
;   FUNCTION
;	Performs the necessary steps to UnLock a ServiceData structure
;
;   INPUTS
;	AH = Function
;	
;	DI = Offset of ServiceData structure
;
;   RESULT
;	AL = Status (JSERV_OK)
;
;   EXAMPLE
;	mov	AH,JFUNC_UNLOCKSERVICEDATA
;	mov	DI,ServiceData_Offset
;	int	JFUNC_JINT
;
;   NOTES
;
;   BUGS
;
;   SEE ALSO
;	LockServiceData()
;
;****************************************************************************
;
;
public	        UnlockServiceData

UnlockServiceData	proc	near

	push	ax
	push	es

	mov	ax,cs:janus_param_seg 		;* get segment of parameter 
	mov    	es,ax				;*  memory
;	UNLOCK 	es:ServiceData.sd_ServiceDataLock[di]	;* lock ServiceData
	push	di
	lea	di,es:ServiceData.sd_ServiceDataLock[di]
	call	JanusUnlock
	pop	di

	pop	es
	pop	ax
	mov     al,JSERV_OK
	ret

UnlockServiceData	endp

;****** JanusHandler/JanusLockAttempt ******************************************
;
;   NAME
;	JanusLockAttempt -- Try once to obtain a Lock
;
;   SYNOPSIS
;	mov	AH,Function
;	mov	ES,Segment
;	mov	DI,Offset
;	int	0bh
;
;   FUNCTION
;	Attempts once to gain a Lock.
;
;   INPUTS
;	AH = Function
;	
;	ES = Segment of lock byte
;
;	DI = Offset of lock byte
;
;   RESULT
;	AL = TRUE if lock is gotten FALSE otherwise.
;
;   EXAMPLE
;	mov	AH,JFUNC_LOCKATTEMPT
;	mov	ES,Segment
;	mov	DI,Offset
;	int	JFUNC_JINT
;
;   NOTES
;
;   BUGS
;
;   SEE ALSO
;	JanusInitLock(), JanusLock(), JanusUnlock()
;
;****************************************************************************
;
;
public	        JanusLockAttempt

JanusLockAttempt proc	near

	;*****************************
	;* Software locking protocol *
	;*****************************
	IFE	HARDWARE_LOCK
	push	ds
	push	ax
	mov	ax,cs:janus_param_seg
	mov    	ds,ax 
	pop	ax
	mov	ds:JanusAmiga.ja_PCFlag,1
	mov	ds:JanusAmiga.ja_Turn,1
aga:
	cmp	ds:JanusAmiga.ja_AmigaFlag,1
	jne	ot
;;;
	cmp	ds:JanusAmiga.ja_Turn,1
;	cmp	ds:JanusAmiga.ja_Turn,0
;;;
	je	aga
ot:
	cmp	es:byte ptr[di],0ffh
	je	fal
	mov	es:byte ptr[di],0ffh
	mov	al,1
	jmp	tru
fal:
	mov	al,0	
tru:
	mov	ds:JanusAmiga.ja_PCFlag,0
  	pop	ds

	;*****************************
	;* Hardware locking protocol *
	;*****************************
	ELSE
	mov	al,1
jlstart:
	stc	
    	lock  rcl  es:byte ptr [di],1 	; !!! should use lock prefix
	jnc	jlexit
	mov	al,0
jlexit:
	ENDIF

	ret

JanusLockAttempt endp
;****** JanusHandler/JanusLock ******************************************
;
;   NAME
;	JanusLock -- Obtain a Lock
;
;   SYNOPSIS
;	mov	AH,Function
;	mov	ES,Segment
;	mov	DI,Offset
;	int	0bh
;
;   FUNCTION
;	Get a lock.
;
;   INPUTS
;	AH = Function
;	
;	ES = Segment of lock byte
;
;	DI = Offset of lock byte
;
;   RESULT
;	Returns when the lock is gotten.
;
;   EXAMPLE
;	mov	AH,JFUNC_LOCK
;	mov	ES,Segment
;	mov	DI,Offset
;	int	JFUNC_JINT
;
;   NOTES
;
;   BUGS
;
;   SEE ALSO
;	JanusInitLock(), JanusLockAttempt(), JanusUnlock();
;
;****************************************************************************
;
;
public	        JanusLock

JanusLock	proc	near

	;***************	
	;* Inline code *
	;***************	
	IFE	LOCK_ATTEMPT
	;*******************
	;* Inline software *
	;*******************
	IFE	HARDWARE_LOCK
	push	ds
	push	ax
	mov	ax,cs:janus_param_seg
	mov    	ds,ax 
	pop	ax
l1:	mov	ds:JanusAmiga.ja_PCFlag,1
	mov	ds:JanusAmiga.ja_Turn,1
aga1:
	cmp	ds:JanusAmiga.ja_AmigaFlag,1
	jne	ot1
;;;
	cmp	ds:JanusAmiga.ja_Turn,1
;	cmp	ds:JanusAmiga.ja_Turn,0
;;;
	je	aga1
ot1:
	cmp	es:byte ptr[di],0ffh
	jne	ll
	mov	ds:JanusAmiga.ja_PCFlag,0
	jmp	l1
ll:	mov	es:byte ptr[di],0ffh

	mov	ds:JanusAmiga.ja_PCFlag,0
  	pop	ds
	;*******************
	;* Inline hardware *
	;*******************
	ELSE
	stc	
jlstart1:
    	lock  rcl  es:byte ptr [di],1 
	jc	jlstart1

	ENDIF
	;********************
	;* Use lock attempt *
	;********************
	ELSE

	push	ax
more2:
	call	JanusLockAttempt
	or	al,al
	je	more2	
	pop	ax

	ENDIF

	ret

JanusLock	endp
;****** JanusHandler/JanusUnlock ******************************************
;
;   NAME
;	JanusUnlock -- Release a Lock
;
;   SYNOPSIS
;	mov	AH,Function
;	mov	ES,Segment
;	mov	DI,Offset
;	int	0bh
;
;   FUNCTION
;	Release a lock.
;
;   INPUTS
;	AH = Function
;	
;	ES = Segment of lock byte
;
;	DI = Offset of lock byte
;
;   RESULT
;	Returns when the lock is released.
;
;   EXAMPLE
;	mov	AH,JFUNC_UNLOCK
;	mov	ES,Segment
;	mov	DI,Offset
;	int	JFUNC_JINT
;
;   NOTES
;
;   BUGS
;
;   SEE ALSO
;	JanusInitLock(), JanusLockAttempt(), JanusLock()
;
;****************************************************************************
;
;
public	        JanusUnlock

JanusUnlock	proc	near

;	mov	es:byte ptr [di],07fh	
;	ret

JanusUnlock	endp
;****** JanusHandler/JanusInitLock ******************************************
;
;   NAME
;	JanusInitLock -- Initialize a byte for use as a lock byte.
;
;   SYNOPSIS
;	mov	AH,Function
;	mov	ES,Segment
;	mov	DI,Offset
;	int	0bh
;
;   FUNCTION
;	Performs the necessary steps to Initialize a Janus Lock byte
;
;   INPUTS
;	AH = Function
;	
;	ES = Segment of lock byte
;
;	DI = Offset of lock byte
;
;   RESULT
;	The Lock byte is initialized.
;
;   EXAMPLE
;	mov	AH,JFUNC_INITLOCK
;	mov	ES,Segment
;	mov	DI,Offset
;	int	JFUNC_JINT
;
;   NOTES
;
;   BUGS
;
;   SEE ALSO
;	JanusLockAttempt(), JanusLock(), JanusUnlock()
;
;****************************************************************************
;
;
public	        JanusInitLock

JanusInitLock	proc	near

	mov	es:byte ptr [di],07fh	
	ret

JanusInitLock	endp

JanusDebug	proc	near

	push	es
	push	di
	push	ax

	mov	ax,0d000h
	mov	es,ax
	;get ptr
	mov	di,es:3f00h

;	mov	di,03f00h
	pop	ax
	mov	es:3f02h[di],al
	;inc ptr
	inc	di
	;save ptr
	mov	es:3f00h,di

	pop	di
	pop	es
	ret

JanusDebug	endp	
cseg		ends

end



ifdef grapefruit
DEB	MACRO	addr,data
	push	di
	push	cx
	mov	di,addr
	mov	cl,data
	call	debout
	pop	cx
	pop	di
ENDM

DEBSET	MACRO	addr,len
	push	di
	push	cx
	mov	di,addr
	mov	deb_addr,di
	mov	cx,len
	call	debwipe
	pop	cx
	pop	di
ENDM

DEBI	MACRO	data
	push	di
	push	cx
	mov	di,deb_addr
	inc	deb_addr
	mov	cl,data
	call	debout
	pop	cx
	pop	di
ENDM
;----------------------------------------------------------------------------
IF PRINT
assumes cs,Code
assumes ds,Data

debout	proc	far
	push	es
	push	ax

	cmp	mode,86
	jz	debreal
	mov	ax,offset __D000h
	jmp	debit
debreal:
	mov	ax,0D000h
debit:
	mov	es,ax
	pop	ax
	add	di,3900h
	mov	byte ptr es:[di],cl
	pop	es

	ret
debout	endp

debwipe	proc	far
	push	es
	push	ax

	cmp	mode,86
	jz	debwreal
	mov	ax,offset __D000h
	jmp	debwit
debwreal:
	mov	ax,0D000h
debwit:
	mov	es,ax
	pop	ax
	add	di,3900h
debwl:
	mov	byte ptr es:[di],0
	inc	di
	loop	debwl
	pop	es

	ret
debwipe	endp
ENDIF
endif ; grapefruit
;----------------------------------------------------------------------------













								   
