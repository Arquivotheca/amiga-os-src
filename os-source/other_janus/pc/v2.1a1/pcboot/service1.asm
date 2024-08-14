 TITLE	SERVICE1  -  COPYRIGHT (C) 1986 - 1988 Commodore Amiga Inc. 
PAGE	60,132	
;*****************************************************************************
;
; Janus Service Handler for PC:	 (1.Generation)
;
;
; New code  :	30-Mar-88 Torsten Burgdorff
; 44.Update :  	 1-Apr-88 TB	add service 0,4,5
; 53.Update :	23-Jun-88 TB    cleanup
; 54.Update :   24-Jun-88 TB	use new set of include files
; 57.Update :   20-jul-88 TB	use memory.inc, range check services 7,8,9
; 64.Update :	28-jul-89 TB	check for zero in AllocMem and FreeMem
; 71.Update :	28-oct-89 TB	added JFunction 21-25
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
extrn		SendJanusInt:near	     ; send interrupt to Amiga side
extrn		JanusUnlock:near
extrn		JanusLock:near

w	       	equ     word ptr
		
include		equ.inc
include        	janus\janusvar.inc
include		janus\services.inc
include		janus\memory.inc
include		privserv.inc
include        	macros.inc
include        	vars_ext.inc
include		debug.inc

;--------------------------------------------------------------------
;
; Service 0: GET_SERVICE	 Gets a new Service Number
; ========================
;
; Expects:
;	AH	:	JFUNC_GETSERVICE1
;
; Returns:
;	AL 	:      Status (JSERV_NOFUNCTION)
;--------------------------------------------------------------------
	
public	       	GetService1
GetService1    	proc	near

	mov	al,JSERV_NOFUNCTION
	ret

GetService1    	endp

;****** JanusHandler/GetBase ******************************************
;
;   NAME
;	GetBase -- Get the segment addresses and offsets for an old
;		   style service.
;
;   SYNOPSIS
;	mov	AH,Function
;	mov	AL,Service_number
;	int	0bh
;
;   FUNCTION
;	Returns the segment address for buffer and parameter memory.
;	Also returns the parameter memory offset for the old style
;	service if defined.
;
;   INPUTS
;	AH = Function
;
;	AL = Janus Service Number
;
;   RESULT
;	ES = Parameter memory segment
;
;	DI = Parameter offset if defined, else -1
;
;	DX = Buffer memory segment
;
;	AL = Status return (JSERV_OK)
;
;   EXAMPLE
;	mov	AH,JFUNC_GETBASE
;	mov	AL,JSERV_AMIGASERVICE
;	int	JFUNC_JINT
;
;   NOTES
;
;   BUGS
;
;   SEE ALSO
;
;****************************************************************************
;
;
public	       	GetBase
GetBase       	proc	near

	push	si				; save it
	push	bx
	push	ax
	xor	bh,bh	
	mov 	bl,al  				; service # in BX
	mov	dx,cs:janus_buffer_seg		; 1.Buffer seg
	mov	ax,cs:janus_param_seg
	mov    	es,ax				; 2.Para seg
	mov	si,es:JanusAmiga.ja_Parameters	; points to parameter table
	shl	bx,1
	mov	di,es:[si][bx]			; 3.Para offset
	pop	ax
	pop 	bx	
	pop	si
	mov     al,JSERV_OK
	ret

GetBase       endp

;****** JanusHandler/AllocMem ******************************************
;
;   NAME
;	AllocMem -- Allocate Janus memory
;
;   SYNOPSIS
;	mov	AH,Function
;	mov	AL,Type
;	mov	BX,Size
;	int	0bh
;
;   FUNCTION
;	Allocates memory from the Janus dual-port RAM.
;
;   INPUTS
;	AH = Function
;
;	AL = Type of memory to allocate (MEMF_BUFFER, MEMF_PARAMETER)
;
;	BX = Number of bytes to allocate
;
;   RESULT
;	BX = Offset of allocated memory else 0
;
;	AL = Status (JSERV_OK,JSERV_NOJANUSMEM)
;
;   EXAMPLE
;	mov	AH,JFUNC_ALLOCMEM
;	mov	AL,MEMF_BUFFER
;	mov	BX,200
;	int	JFUNC_JINT
;
;   NOTES
;
;   BUGS
;
;   SEE ALSO
;	FreeMem()
;
;****************************************************************************
;
;
public	       AllocMem
AllocMem      proc	near

	push	cx
	push	dx
	push	di
	push	si
	push	es
	push	ds
	
	or 	bx,bx		       		; check for zero first
	jnz	Alloc_Zero
	mov	al,JSERV_OK			; set OK flag
	jmp	exit_Mem

Alloc_Zero:
	push	ax
	mov	ax,bx
	INFO_AX	AM_Alloc
	pop	ax

	mov	cl,al				; store type in CL
	add	bx,7				; round up size
	and	bl,0fch
	mov	ax,cs:janus_param_seg		; ES point to para mem
	mov    	es,ax   
	cmp	cl,MEMF_PARAMETER
	je 	Lock_Para
	cmp	cl,MEMF_BUFFER
	je	Lock_Buff
	jmp 	Mem_Type_Failed

Lock_Buff:
;	JLOCK	es:JanusAmiga.jbm_Lock
	push	di
	lea	di,es:JanusAmiga.jbm_Lock
	call	JanusLock
	pop	di

	lea	si,es:JanusAmiga.jbm_First
	push	si				; save back pointer
	push	es
	mov	si,es:JanusAmiga.jbm_First	; points to 1. buffer mem chunk
	mov	ax,es:JanusAmiga.jbm_8088Segment
 	mov	ds,ax				; DS=buffer mem seg
	jmp	Find_Mem

Lock_Para:		
;	JLOCK	es:JanusAmiga.jpm_Lock
	push	di
	lea	di,es:JanusAmiga.jpm_Lock
	call	JanusLock
	pop	di

	lea	si,es:JanusAmiga.jpm_First		
	push	si				; save back pointer
	push	es
	mov	si,es:JanusAmiga.jpm_First	; points to 1. para mem chunk	
	mov	ax,es:JanusAmiga.jpm_8088Segment
 	mov	ds,ax				; DS=para mem seg
	jmp	Find_Mem

Loop_Mem:					; look in next chunk	
	pop	di				; restore dummy
	pop	di
	push	si				; save back pointer
	push	ds
	mov	si,ds:JanusMemChunk.jmc_Next[si]	

Find_mem:	
	INFO	MemChunk

	if	DBG_MemChunk
	 mov	ax,si
	 call	outhxw				; print mem offset
	 call	newline
	endif

	cmp	si,-1				; was last chunk ?
	je	Mem_Failed			; Y -> exit
	cmp	ds:JanusMemChunk.jmc_Size[si],bx
	jb	Loop_Mem			; mem chunk to small
	je	Got_Mem				; it fits exactly

Mem_Split:					; mem chunk to big,
	mov	di,si				;  let's split it
	add	di,bx				; DI points behind our mem

; -------   Create a new mem chunk behind the current one

	mov	dx,ds:JanusMemChunk.jmc_Next[si] 	
	mov	ds:JanusMemChunk.jmc_Next[di],dx				     	
	mov	dx,ds:JanusMemChunk.jmc_Size[si]	
	sub	dx,bx					; with a new size
	mov	ds:JanusMemChunk.jmc_Size[di],dx				     	
	mov	ds:JanusMemChunk.jmc_Next[si],di	; adjust pointer'next'

Got_mem:
	mov	dx,ds:JanusMemChunk.jmc_Next[si] 	
	mov	ds:JanusMemChunk.jmc_Size[si],bx      	; prepare mem chunk
	mov	ds:JanusMemChunk.jmc_Next[si],0		  
	pop	ds
	pop	di					; restore back ptr
	mov	ds:[di],dx				; and update ptr
	cmp	cl,MEMF_PARAMETER
	je	Update_Para

	sub	es:JanusAmiga.jbm_Free,bx		; update Free count
;	UNLOCK	es:JanusAmiga.jbm_Lock	
	push	di
	lea	di,es:JanusAmiga.jbm_Lock
	call	JanusUnlock
	pop	di
	jmp	End_Mem

Update_Para:
	sub	es:JanusAmiga.jpm_Free,bx	; update Free count
;	UNLOCK	es:JanusAmiga.jpm_Lock	
	push	di
	lea	di,es:JanusAmiga.jpm_Lock
	call	JanusUnlock
	pop	di

End_Mem:	
	add	si,4
	mov	bx,si				; mem pointer now BX
	mov	al,JSERV_OK			; set OK flag
	jmp	exit_Mem
						
Mem_Failed:					; unlock memory,
	pop	ds 				; restore back pointer,
	pop	si
;	UNLOCK	es:JanusAmiga.jbm_Lock	
	push	di
	lea	di,es:JanusAmiga.jbm_Lock
	call	JanusUnlock
	pop	di
;	UNLOCK	es:JanusAmiga.jpm_Lock	
	push	di
	lea	di,es:JanusAmiga.jpm_Lock
	call	JanusUnlock
	pop	di

Mem_Type_Failed:
	mov	al,JSERV_NOJANUSMEM		; set error flag,
	mov	bx,0  				; and zero the offset

Exit_Mem:
	pop	ds
	pop	es
	pop	si
	pop	di
	pop	dx
	pop	cx
        ret

AllocMem      endp
  

;****** JanusHandler/AllocJRemember ****************************************
;
;   NAME   
;     	AllocJRemember -- Allocate Janus memory and link into a Remember list
;
;   SYNOPSIS
;	mov	AH,Function
;	mov	AL,Type
;	mov	BX,Size
;	mov	ES,JRememberKey_Segment
;	mov	DI,JRememberKey_Offset
;	int	0bh
;
;   FUNCTION
;	Allocates memory from the Janus dual-port RAM, but also adds the 
;	details of the allocation into a master list so that you make 
;      	just one call the FreeJRemember() routine at a later time to  
;      	free all the memory you allocated.
;
;   INPUTS
;   	AH = Function
;
;	AL = Type of memory to allocate (MEMF_BUFFER, MEMF_PARAMETER)
;
;	BX = Number of bytes to allocate
;
;	ES = Segment of JRememberKey
;
;	DI = Offset of JRememberKey  
;	     ( Before your very first call to AllocJRemember(),	)
;	     ( the pointer should be set to NULL. )
;
;   RESULT
;	BX = Offset of allocated memory else 0
;
;	AL = Status (JSERV_OK,JSERV_NOJANUSMEM)
;
;   EXAMPLE
;	mov	AH,JFUNC_ALLOCJREMEMBER
;	mov	AL,MEMF_BUFFER
;	mov	BX,200
;	mov	ES,JRememberKey_Segment
;	mov	DI,JRememberKey_Offset
;	int	JFUNC_JINT
;
;   NOTES
;
;   BUGS
;
;   SEE ALSO
;     	AllocRemember(), AttachJRemember(), FreeJRemember()
;
;*****************************************************************************
;
public	       AllocJRemember
AllocJRemember	proc	near

	pushall
	mov 	bp,sp

	mov	cx,WAIT_FREE_LOOP
AJR_LoopBase:
	mov	al,JSERV_AMIGASERVICE
	call	GetBase				; we need parameter segment 
						; in ES only
 	cmp	di,-1				;** valid base found ?		
	jne	AJR_AllocStruc
	loop	AJR_LoopBase
	mov 	al,JSERV_NOJANUSBASE		;** no => set error and exit
	mov	bx,0				;Failed so return offset 0
	jmp	AJR_Exit

AJR_AllocStruc:
	INFO	AJR_StrucMem
	mov	al,MEMF_PARAMETER		; allocate para_memory for
	mov	bx,size JanusRemember 		; JRemember structure	
	call	AllocMem
	or	al,al
	jz	AJR_ClearMem	
	jmp	AJR_Exit

AJR_ClearMem:					;* clear structure
	cld
	push	cx
	push	di
	mov	cx,size JanusRemember
	mov	di,bx
	rep	stosb
	pop	di
	pop	cx

	mov	si,bx				; keep copy of memory pointer

	INFO	AJR_CustomMem

	mov	ax,ss:StackFrame.RegAX[bp]	; allocate caller's memory
	xor	ah,ah
	mov 	dx,ax				; keep copy of memory type
	mov	bx,ss:StackFrame.RegBX[bp]
	mov	cx,bx				; keep copy of memory size
	call	AllocMem
	or	al,al
	jz	AJR_Init
	jmp	AJR_ErrAlloc

AJR_Init:					; initialize new JRemember
	INFO	AJR_InitStruc
	mov	es:JanusRemember.jrm_Offset[si],bx	; with memory offset,
	mov	es:JanusRemember.jrm_Size[si],cx    	; size and
	mov	es:JanusRemember.jrm_Type[si],dx    	; type

	mov	ax,ss:StackFrame.RegES[bp]	; link JRemember structure
	mov	ds,ax				; to given key
	mov	di,ss:StackFrame.RegDI[bp]  	
	mov	es:JanusRemember.jrm_NextRemember[si],-1
	cmp	word ptr [di],0		    	; key unused?
	je	AJR_FirstUser
						; no: link old JRemember struc	
	mov	ax,[di]				;     to our one			
	mov	es:JanusRemember.jrm_NextRemember[si],ax

AJR_FirstUser:					; link our JRemember struc
	mov	[di],si				; to JRememberKey
	mov	al,JSERV_OK			; set OK flag
	jmp	AJR_Exit			

AJR_ErrAlloc:					; deallocate memory
	push	ax  			     
	mov	al,MEMF_PARAMETER
	mov	bx,si
	call	FreeMem				; ignore errors
	pop	ax	

AJR_Exit:				 
	INFO	AJR_End
	mov	ah,JFUNC_ALLOCJREMEMBER
	mov	ss:StackFrame.RegAX[bp],ax	; return code	
	mov	ss:StackFrame.RegBX[bp],bx	; return mem offset
	POPALL
	ret

AllocJRemember	endp	

;****** JanusHandler/AttachJRemember ***************************************
;
;   NAME   
;     	AttachJRemember -- Attach the list of one Janus memory key to another
;
;   SYNOPSIS
;	mov	AH,Function
;	mov	ES,To_JRememberKey_Segment
;	mov	DI,To_JRememberKey_Offset
;	mov	DS,From_JRememberKey_Segment
;	mov	SI,From_JRememberKey_Offset
;	int	0bh
;
;   FUNCTION
;	This routine accepts two Janus RememberKeys created by calls to 
;     	AllocJRemember() and attaches the contents of the FromKey to 
;     	the ToKey.  In the process, FromKey is set equal to NULL.  
;
;   INPUTS
;	ES = Segment of To_JRememberKey
;
;	DI = Offset of To_JRememberKey  
;	     ( address of a pointer to JanusRemember structures, which ) 
;	     ( pointer is going to receive the list pointed to by FromKey )
;
;	DS = Segment of From_JRememberKey
;
;	SI = Offset of From_JRememberKey  
;	     ( address of a pointer to JanusRemember structures, which ) 
;	     ( pointer has the list that's going to be attached ToKey, )
;            ( after which the FromKey variable will be set to NULL. )
;	
;   RESULT
;     	None
;
;   EXAMPLE
;
;   NOTES
;
;   BUGS
;
;   SEE ALSO
;     	AllocRemember(), AllocJRemember(), FreeJRemember()
;
;*****************************************************************************
public		AttachJRemember
AttachJRemember	proc	near

	push	bx
	push	cx
	push	dx
	push	di
	push	si
	push	ds
	push	es

	push	ax
	mov	ax,di
	INFO_AX	AtJR_To
	mov	ax,si
	INFO_AX	AtJR_From
	pop	ax
	
	mov	bx,es:[di]			; check To_JRemember for zero
	or	bx,bx
	jne	AtJR_Find
	movsw					; attach From_JRemember directly
	mov	word ptr ds:[si-2],0  		; set From_JRemember to zero
	mov	al,JSERV_OK			; set OK flag
	jmp	AtJR_Exit			

AtJR_Find:					; find end of To_JRemember list
	INFO	AtJR_FindEnd
	mov	cx,WAIT_FREE_LOOP
AtJR_LoopBase:
	mov	al,JSERV_AMIGASERVICE
	call	GetBase				; we need parameter segment 
						; in ES only
 	cmp	di,-1				;** valid base found ?		
	jne	AtJR_Next
	loop	AtJR_LoopBase
	mov 	al,JSERV_NOJANUSBASE		;** no => set error and exit
	jmp	AtJR_Exit

AtJR_Next:
	mov	di,bx				
	mov 	bx,es:JanusRemember.jrm_NextRemember[di]  		
	cmp	bx,-1
	jne	AtJR_Next			

	push	ax
	mov	ax,di
	INFO_AX	AtJR_To
	mov	ax,si
	INFO_AX	AtJR_From
	pop	ax

	mov     ax,ds:[si] 			  	 ; attach From_JRemember to the
	mov 	es:JanusRemember.jrm_NextRemember[di],ax ; end of To_JRemember list  		
	mov	word ptr ds:[si],0  			 ; set From_JRemember to zero
	mov	al,JSERV_OK				 ; set OK flag
						
AtJR_Exit:
	INFO	AtJR_End
	mov	ah,JFUNC_ATTACHJREMEMBER

	pop	bp
	pop	ds
	pop	si
	pop	di
	pop	dx
	pop	cx
	pop	bx
	ret

AttachJRemember	endp	

;****** JanusHandler/FreeJRemember *****************************************
;
;   NAME   
;     	FreeJRemember -- Free memory allocated by calls to AllocJRemember()
;
;   SYNOPSIS
;	mov	AH,Function
;	mov	AL,ReallyForget
;	mov	ES,JRememberKey_Segment
;	mov	DI,JRememberKey_Offset
;	int	0bh
;
;   FUNCTION
;	This function frees up Janus memory allocated by the AllocJRemember() 
;     	function.  It will either free up just the JanusRemember structures
;	(ReallyForget = FALSE),  which supply the link nodes that tie your
;	allocations together, or it will deallocate both the link nodes AND
;     	your memory buffers too (ReallyForget = TRUE).
;
;   INPUTS
;   	AH = Function
;
;	AL = ReallyForget (BOOL)
;	     ( a BOOL FALSE or TRUE describing, respectively, whether you )
;	     ( want to free up only the Remember nodes or if you want this ) 
;	     ( procedure to really forget about all of the memory, including ) 
;	     ( both the nodes and the memory buffers pointed to by the nodes. )
;
;	ES = Segment of JRememberKey
;
;	DI = Offset of JRememberKey  
;	     ( the address of a pointer to struct JanusRemember. This pointer )
;	     ( should either be NULL or set to some value (possibly NULL) by )
;	     ( a call to AllocRemember()
;
;   RESULT
;     	None
;
;   EXAMPLE
;
;   NOTES
;
;   BUGS
;
;   SEE ALSO
;     	AllocJRemember(), FreeRemember(), AttachJRemember()
;
;*****************************************************************************
public		FreeJRemember
FreeJRemember	proc	near
						
	push	bx
	push	cx
	push	dx
	push	di
	push	si
	push	es

	mov	dx,ax				; save ReallyForget flag
	mov	ax,0				; set JRememberKey to zero
	mov	si,es:[di]			; get offset of first JRemember struc
	stosw 
	or	si,si				; check JRemember for zero
	jne	FJR_GetFirst			; all done
	mov	al,JSERV_OK			; set OK flag
	jmp	FJR_Exit

FJR_GetFirst:
	mov	cx,WAIT_FREE_LOOP
FJR_LoopBase:
	mov	al,JSERV_AMIGASERVICE
	call	GetBase				; we need parameter segment 
						; in ES only
 	cmp	di,-1				;** valid base found ?		
	jne	FJR_GetNext
	loop	FJR_LoopBase
	mov 	al,JSERV_NOJANUSBASE		;** no => set error and exit
	jmp	FJR_Exit

FJR_GetNext:
	INFO	FJR_Next
	cmp	dl,TRUE				; check ReallyForget flag
	jne	FJR_StrucOnly						
	mov	ax,es:JanusRemember.jrm_Type[si]  
	mov	bx,es:JanusRemember.jrm_Offset[si]						
	call	FreeMem		       		; free attached memory

FJR_StrucOnly:					; keep copy of offset of 
	mov	bx,si 				; current JRem structure
						; get next JRem struct					
	mov 	si,es:JanusRemember.jrm_NextRemember[si]  		
	mov	al,MEMF_PARAMETER
	call	FreeMem			     	; free cuurent JRem struc

	cmp	si,-1				; check for end of JRem list
	jne	FJR_GetNext
	mov	al,JSERV_OK			; set OK flag

FJR_Exit:
	INFO	FJR_End
	mov	ah,JFUNC_FREEJREMEMBER
	pop	es
	pop	si
	pop	di
	pop	dx
	pop	cx
	pop	bx
	ret

FreeJRemember	endp	

;****** JanusHandler/AllocServiceMem ***************************************
;
;   NAME   
;	AllocServiceMem -- Allocate Janus memory linked to a ServiceData struct
;
;   SYNOPSIS
;	mov	AH,Function
;	mov	AL,Type
;	mov	BX,Size
;	mov	DI,ServiceData_Offset
;	int	0bh
;
;  FUNCTION
;     	This routine allocates memory for you and records the details of 
;     	the allocation in the specified ServiceData structure. This memory,
;     	unless you free it explicitly with a call to FreeServiceMem(), 
;     	will be automatically freed when the service is deleted and 
;     	removed from the system.  
;
;   INPUTS
;   	AH = Function
;
;	AL = Type of memory to allocate (MEMF_BUFFER, MEMF_PARAMETER)
;
;	BX = Number of bytes to allocate
;
;	DI = Offset of ServiceData structure  
;
;   RESULT
;	BX = Offset of allocated memory else 0
;
;	AL = Status (JSERV_OK,JSERV_NOJANUSMEM)
;
;   EXAMPLE
;
;   NOTES
;
;   BUGS
;
;   SEE ALSO
;	FreeServiceMem()
;
;*****************************************************************************
public		AllocServiceMem
AllocServiceMem	proc	near

	push	cx
	push	dx
	push	di
	push	si
	push	es
	push	bp

	mov	bp,ax				; keep copy of mem type
	mov	si,di				; keep copy of ServiceData off

	mov	cx,WAIT_FREE_LOOP
ASM_LoopBase:
	mov	al,JSERV_AMIGASERVICE
	call	GetBase				; we need parameter segment 
						; in ES only
 	cmp	di,-1				;** valid base found ?		
	jne	ASM_LockStruc
	loop	ASM_LoopBase
	mov 	al,JSERV_NOJANUSBASE		;** no => set error and exit
	mov	bx,0	       			; BX = no valid offset
	jmp	ASM_Exit

ASM_LockStruc: 		     			; DI = ServiceParam
	push	di			 	;* lock ServiceParam structure
	lea	di,es:ServiceParam.ServiceLock[di]
	call	JanusLock
	pop	di
	
	INFO	ASM_Alloc
 	mov	ax,bp				; allocate memory with
	xor 	ah,ah				; AllocJRemember
	push	di				; JRememberKey is part of
	lea	di,es:ServiceData.sd_JRememberKey[si]	; ServiceData struc
	call 	AllocJRemember	
	pop	di

ASM_Exit:					;* unlock ServiceParam struc
	push	di
	lea	di,es:ServiceParam.ServiceLock[di]
	call	JanusUnlock
	pop	di

	INFO	ASM_End
	mov	ah,JFUNC_ALLOCSERVICEMEM
	pop	bp
	pop	es
	pop	si
	pop	di
	pop	dx
	pop	cx
	ret
						   
AllocServiceMem	endp	
;****** JanusHandler/FreeServiceMem ****************************************
;
;   NAME   
;	FreeServiceMem -- Free mem added to a ServiceData by AllocServiceMem()
;
;   SYNOPSIS
;	mov	AH,Function
;	mov	AL,Type
;	mov	BX,ServiceMem_Offset
;	mov	DI,ServiceData_Offset
;	int	0bh
;
;   FUNCTION
;   	This routine frees memory that had been allocated with a call 
;     	to AllocServiceMem().  You can choose to free a single block of 
;     	memory or all the memory of the ServiceData structure.  
;
;   INPUTS
;   	AH = Function
;
;	AL = Type of memory to allocate (MEMF_BUFFER, MEMF_PARAMETER)
;
;	BX = Offset of prior allocated memory or NULL if you want to delete
;	     all of the ServiceData's memory
;
;	DI = Offset of ServiceData structure  
;
;   RESULT
;     	None
;
;   EXAMPLE
;
;   NOTES
;
;   BUGS
;
;   SEE ALSO
;	AllocServiceMem()
;
;*****************************************************************************
;*
public		FreeServiceMem
FreeServiceMem	proc	near

	pushall
	mov 	bp,sp

	mov	cx,WAIT_FREE_LOOP
FSM_LoopBase:
	mov	al,JSERV_AMIGASERVICE
	call	GetBase				; we need parameter segment 
						; in ES only
 	cmp	di,-1				;** valid base found ?		
	jne	FSM_LockStruc
	loop	FSM_LoopBase
	mov 	al,JSERV_NOJANUSBASE		;** no => set error and exit
	jmp	FSM_Exit

FSM_LockStruc: 	     				;* lock ServiceParam structure
	mov	dx,di				; keep copy of ServiceParam offset
	lea	di,es:ServiceParam.ServiceLock[di]
	call	JanusLock

	mov	si,ss:StackFrame.RegDI[bp]	
	mov	bx,ss:StackFrame.RegBX[bp]	
	or 	bx,bx				; check memory pointer
	jz	FSM_FreeAll			; free all

	mov	ax,bx
	INFO_AX	FSM_FreeOne

	lea	di,es:ServiceData.sd_JRememberKey[si]
		; ES:DI pointer	to current JRemember structure

	mov	al,JSERV_OK				
	or	di,di	  			; exit, if JRemKey is NULL  
	jz	FSM_Exit


	add	si,ServiceData.sd_JRememberKey-JanusRemember.jrm_NextRemember
		; ES:SI pointer to JRememberKey	in Service Data structure

	jmp	short FSM_Search

FSM_Next:
	mov	si,di			   
		; ES:SI pointer	to previous JRemember structure

	mov 	di,es:JanusRemember.jrm_NextRemember[di] 
		; ES:DI pointer	to current JRemember structure

	cmp	di,-1				; check for end of JRem list
	jne	FSM_Search
	mov	al,JSERV_NOJANUSMEM
	jmp	short FSM_Exit

FSM_Search:
	cmp	bx,es:JanusRemember.jrm_Offset[di]	; search for specific
	jne	FSM_Next				;  offset and
	mov	ax,ss:StackFrame.RegAX[bp]
	xor	ah,ah					;  type 
	cmp	ax,es:JanusRemember.jrm_Type[di] 	;  in JRemember list
	jne	FSM_Next			  

	call	FreeMem					; free this memory and 
	or	al,al
	jnz	FSM_Exit

	mov 	ax,es:JanusRemember.jrm_NextRemember[di] ;  unlink from list
	mov 	es:JanusRemember.jrm_NextRemember[si],ax

	INFO_AX	FSM_Join

	mov	bx,di 	  			; free JRemember structure
	mov	al,MEMF_PARAMETER
	call	FreeMem			     	
	jmp	FSM_Exit

FSM_FreeAll:
	INFO	FSM_Free
	lea	di,es:ServiceData.sd_JRememberKey[si]
	mov	al,TRUE
	call	FreeJRemember		
	  
FSM_Exit:					;* unlock ServiceParam struc
	mov	di,dx				; get copy of ServiceParam offset
	lea	di,es:ServiceParam.ServiceLock[di]
	call	JanusUnlock

	INFO	FSM_End
	mov	ah,JFUNC_ALLOCSERVICEMEM
	mov	ss:StackFrame.RegAX[bp],ax	; return code	
	POPALL
	ret

FreeServiceMem	endp	

;
;****** JanusHandler/FreeMem ******************************************
;
;   NAME
;	FreeMem -- Free Janus memory previously allocated with AllocMem.
;
;   SYNOPSIS
;	mov	AH,Function
;	mov	AL,Type
;	mov	BX,Offset
;	int	0bh
;
;   FUNCTION
;	Frees Janus dual-port memory previously allocated with AllocMem.
;
;   INPUTS
;	AH = Function
;
;	AL = Type of memory to free (MEMF_BUFFER, MEMF_PARAMETER)
;
;	BX = Offset of memory to free. (same value returned by AllocMem)
;
;   RESULT
;	AL = Status (JSERV_OK, JSERV_NOJANUSMEM)
;
;   EXAMPLE
;	mov	AH,JFUNC_FREEMEM
;	mov	AL,MEMF_BUFFER
;	mov	BX,Offset
;	int	JFUNC_JINT
;
;   NOTES
;	Crashes if Type/Offset not valid!
;
;   BUGS
;
;   SEE ALSO
;	AllocMem()
;
;****************************************************************************
;
;
public	       FreeMem
FreeMem       proc	near

	push	cx
	push	dx
	push	di
	push	si
	push	es
	push	ds

	or 	bx,bx		       		; check for zero first
	jnz	Free_Zero
	mov	al,JSERV_OK			; set OK flag
	jmp	exit_Mem

Free_Zero:
	push	ax
	mov	ax,bx
	INFO_AX	FM_Free
	pop	ax

	mov	cl,al				; store type in CL
	sub	bx,4				; points to hidden size entry,
						;  the beginning of this chunk
	mov	ax,cs:janus_param_seg
	mov    	es,ax   			; ES point to para mem
	cmp	cl,MEMF_PARAMETER
	je 	FLock_Para
	cmp	cl,MEMF_BUFFER
	je	FLock_Buff
	jmp 	Mem_Type_Failed

FLock_Buff:
	INFO	FM_Fre2
	cmp	bx,es:JanusAmiga.jbm_Max		; BX in range ?
	jae	Bad_Range
;	JLOCK	es:JanusAmiga.jbm_Lock
	push	di
	lea	di,es:JanusAmiga.jbm_Lock
	call	JanusLock
 	pop	di

	lea	si,es:JanusAmiga.jbm_First
	push	si				; save back pointer
	push	es
	mov	si,es:JanusAmiga.jbm_First	; points to 1. buffer mem chunk
	mov	ax,es:JanusAmiga.jbm_8088Segment
 	mov	ds,ax				; DS=buffer mem seg
	mov	ax,ds:[bx+2]
	add	es:JanusAmiga.jbm_Free,ax	; update Free count
	jmp	Find_Head

FLock_Para:		
	INFO	FM_Fre1
	cmp	bx,es:JanusAmiga.jpm_Max		; BX in range ?
	jae	Bad_Range
;	JLOCK	es:JanusAmiga.jpm_Lock
	push	di
	lea	di,es:JanusAmiga.jpm_Lock
	call	JanusLock
	pop	di

	lea	si,es:JanusAmiga.jpm_First		
	push	si				; save back pointer
	push	es
	mov	si,es:JanusAmiga.jpm_First	; points to 1. para mem chunk	
	mov	ax,es:JanusAmiga.jpm_8088Segment
 	mov	ds,ax				; DS=para mem seg
	mov	ax,ds:[bx+2]
	add	es:JanusAmiga.jpm_Free,ax	; update Free count
	jmp	Find_Head

Bad_Range:
	mov	al,JSERV_NOJANUSMEM		; and set error flag
	jmp	Exit_Mem

Loop_Head:					; look in next chunk	
	pop	di				; restore dummy
	pop	di
	push	si				; save back pointer
	push	ds
	mov	si,ds:JanusMemChunk.jmc_Next[si]	

Find_Head:	
	INFO	MemChunk

	if	DBG_MemChunk
	 mov	ax,si
	 call	outhxw				; print mem offset
	 call	newline
	endif

	cmp	si,bx 				; we are behind 'free PTR'?
	jb	Loop_Head			; NO -> try next
	pop	es
	pop	di				; restore back ptr

; -------   now we have all needed pointers:	ES:DI = Previous PTR
;						DS:BX = Current PTR
;						DS:SI = Next PTR
; -------   create new Mem Chunk Head

	mov	ds:JanusMemChunk.jmc_Next[bx],si	; create next entry
	mov	es:JanusMemChunk.jmc_Next[di],bx	; create next entry
							; of previous PTR
; -------   check whether we can combine two mem chunks

	mov	dx,es:JanusMemChunk.jmc_Size[di]	; check previous
	add	dx,di
	cmp	dx,bx
	jne	Check

; -------   combine previous and current chunks

	mov	ax,ds:JanusMemChunk.jmc_Size[bx]	
	add	es:JanusMemChunk.jmc_Size[di],ax 
	mov	ax,ds:JanusMemChunk.jmc_Next[bx]	
	mov	es:JanusMemChunk.jmc_Next[di],ax	
	mov	ax,es					; prepare DS:BX
	mov	ds,ax
	mov	bx,di					
							
Check:	 						
	mov	ax,ds:JanusMemChunk.jmc_Size[bx]	
	add	ax,bx
	cmp	ax,si	
	jne	No_Combine

; -------   combine next and curent chunks

	mov	ax,ds:JanusMemChunk.jmc_Size[si]       
	add	ds:JanusMemChunk.jmc_Size[bx],ax 
	mov	ax,ds:JanusMemChunk.jmc_Next[si]	
	mov	ds:JanusMemChunk.jmc_Next[bx],ax	

No_Combine:
;	UNLOCK	es:JanusAmiga.jbm_Lock	
	push	di
	lea	di,es:JanusAmiga.jbm_Lock
	call	JanusUnlock
	pop	di
;	UNLOCK	es:JanusAmiga.jpm_Lock	
	push	di
	lea	di,es:JanusAmiga.jpm_Lock
	call	JanusUnlock
	pop	di
	mov	al,JSERV_OK				; set OK flag
	jmp	exit_Mem

FreeMem       endp

;****** JanusHandler/SetParam ******************************************
;
;   NAME
;	SetParam -- Set the parameter memory offset for an old style
;		    service.
;
;   SYNOPSIS
;	mov	AH,Function
;	mov	AL,ServiceNumber
;	mov	BX,Offset
;	int	0bh
;
;   FUNCTION
;	Sets the parameter memory pointer for the old style service to
;	the offset given.
;
;   INPUTS
;	AH = Function
;
;	AL = Janus Service Number
;
;	BX = Offset of parameter memory for this service
;
;   RESULT
;	AL = Status (JSERV_OK)
;
;   EXAMPLE
;	mov	AH,JFUNC_SETPARAM
;	mov	AL,JSERV_AMIGASERVICE
;	mov	BX,Offset
;	int	JFUNC_JINT
;
;   NOTES
;	This is a low level Janus call retained for compatibility with
;	V1.0. Service programmers should NOT use this function.
;
;   BUGS
;
;   SEE ALSO
;
;****************************************************************************
;
;
public	       SetParam	 
SetParam      proc	near

	PUSHALL
	mov	di,bx
	xor	bh,bh	
	mov 	bl,al  				; service # in BX
	mov	ax,cs:janus_param_seg
	mov    	es,ax				
	mov	si,es:JanusAmiga.ja_Parameters	; points to parameter table
	shl	bx,1
	mov	es:[si][bx],di			; fill in the new offset
	POPALL
	mov     al,JSERV_OK
	ret

SetParam      endp


;--------------------------------------------------------------------
;
; Service 5: SET_SERVICE	Set an address for a far call 
; ======================	for that service
;
; Expects:
;	AH	:	JFUNC_SETSERVICE
;	AL 	: 	Janus Service Number to support
;	ES:DX 	:	Entry address for FAR call
;
; Returns:
;	AL 	: 	Status (JSERV_NOFUNCTION)
;--------------------------------------------------------------------
;
public	       SetService
SetService    proc	near

	mov	al,JSERV_NOFUNCTION
	ret

SetService    endp

;--------------------------------------------------------------------
;
; Service 6: STOP_SERVICE	Prevents AMIGA from using the far call
; =======================	(see above) for this function and releases
;				this Service Number.
; 				No memory is freed up. No calls are accepted
;				from either side anymore.
;
; Expects:
;	AH	:	JFUNC_STOPSERVCIE
;	AL 	:	Number of Service to stop
;
; Returns:
;	AL 	: 	Status (JSERV_NOFUNCTION)
;
;--------------------------------------------------------------------
;
public	       StopService
StopService   proc	near

	mov	al,JSERV_NOFUNCTION
	ret

StopService   endp
;****i* JanusHandler/CallAmiga ******************************************
;
;   NAME
;	CallAmiga -- Call an old style service
;
;   SYNOPSIS
;	mov	AH,Function
;	mov	AL,Service_Number
;	int	0bh
;
;   FUNCTION
;	Signals the Amiga side of the given old style service
;
;   INPUTS
;	AH = Function
;
;	AL = Janus Service Number
;
;   RESULT
;	AL = Status (JSERV_PENDING, JSERV_FINISHED, JSERV_NOSERVICE)
;
;   EXAMPLE
;	mov	AH,JFUNC_CALLAMIGA
;	mov	AL,JSERV_AMIGASERVICE
;	int	JFUNC_JINT
;
;   NOTES
;	This is a low level Janus call retained for compatibility with
;	V1.0. Service programmers should NOT use this function.
;
;   BUGS
;
;   SEE ALSO
;
;****************************************************************************
;
;
;****i* JanusHandler/WaitAmiga ******************************************
;
;   NAME
;	WaitAmiga -- Wait for a previously issued CallAmiga to colplete.
;
;   SYNOPSIS
;	mov	AH,Function
;	mov	AL,Service_Number
;	int	0bh
;
;   FUNCTION
;	Waits for the Amiga to respond to the last CallAmiga.
;
;   INPUTS
;	AH = Function
;
;	AL = Janus Service Number
;
;   RESULT
;	AL = Status (JSERV_FINISHED, JSERV_NOSERVICE)
;
;   EXAMPLE
;	mov	AH,JFUNC_WAITAMIGA
;	mov	AL,JSERV_AMIGASERVICE
;	int	JFUNC_JINT
;
;   NOTES
;	This is a low level Janus call retained for compatibility with
;	V1.0. Service programmers should NOT use this function.
;
;   BUGS
;
;   SEE ALSO
;
;****************************************************************************
;
;
;****i* JanusHandler/CheckAmiga ******************************************
;
;   NAME
;	CheckAmiga -- Check the status of a previously issued CallAmiga.
;
;   SYNOPSIS
;	mov	AH,Function
;	mov	AL,Service_Number
;	int	0bh
;
;   FUNCTION
;	Checks the response status of the last CallAmiga.
;
;   INPUTS
;	AH = Function
;
;	AL = Janus Service Number
;
;   RESULT
;	AL = Status (JSERV_PENDING, JSERV_FINISHED, JSERV_NOSERVICE)
;
;   EXAMPLE
;	mov	AH,JFUNC_CHECKAMIGA
;	mov	AL,JSERV_AMIGASERVICE
;	int	JFUNC_JINT
;
;   NOTES
;	This is a low level Janus call retained for compatibility with
;	V1.0. Service programmers should NOT use this function.
;
;   BUGS
;
;   SEE ALSO
;
;****************************************************************************
;
;
public	       CallAmiga, WaitAmiga, CheckAmiga
 
CallAmiga     proc	near
				
	cmp	al,MaxInt			; check range
	ja	RangeErr

	push	ax
	push	bx
	xor	bh,bh
	mov	bl,al				; service # in BX
	mov	byte ptr cs:ServStatTab[bx],JSERV_PENDING 
	call	SendJanusInt			; set status = pending
 	pop     bx  				; fall into Check routine
	pop	ax				; with AH=0

CheckAmiga:					; Service 9 -----------------
						; AH=0 on this entry

WaitAmiga:					; Service 8 -----------------
   						; AH=1 on this entry 
;	call 	JanIntEn			; enable janus interrupts
	cmp	al,MaxInt			; check range
	ja	RangeErr

	sti					; enable ints now
	push	bx
	xor	bh,bh
	mov	bl,al 				; service # in BX

Wait_Amiga_Ready:
 	mov	al,byte ptr cs:ServStatTab[bx] 	; read service status
	or	ah,ah				; called via Wait_Amiga ?
	jz	Wait_Amiga_Exit			; AH=1 means yes !		
	cmp	al,JSERV_FINISHED
	jne	Wait_Amiga_Ready		; loop if pending

Wait_Amiga_Exit:
	pop	bx
	jmp	short	Common_Exit

RangeErr:
	mov	al,JSERV_NOSERVICE		; yes -> set error and return

Common_Exit:
	INFO_AX WaitStatus
	ret

CallAmiga	endp


cseg		ends

end
