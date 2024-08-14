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

w	       	equ     word ptr
		
include        	janus\janusvar.inc
include		janus\services.inc
include		janus\memory.inc
include		privserv.inc
include        	macros.inc
include        	vars_ext.inc
include		debug.inc
include		equ.inc

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


;--------------------------------------------------------------------
;
; Service 1: GET_BASE 		Gets Segmemts & offset of Janus Memory
; ===================
;
; Expects:
;	AH	:	JFUNC_GETBASE
; 	AL 	:	Janus Service Number
;
; Returns :
; 	ES 	: 	Janus Parameter Segment
; 	DI 	: 	Janus Parameter Offset (if defined),
; 			 else -1
; 	DX 	:	Janus Buffer Segment
; 	AL 	:	Status (JSERV_OK)
;--------------------------------------------------------------------
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


;--------------------------------------------------------------------
;
; Service 2: ALLOC_MEM		 Allocates Janus Memory
; ====================
;
; Expects:
;	AH	:	JFUNC_ALLOCMEM
;	AL 	: 	Type of Mem to allocate (MEMF_PARAMETER, MEMF_BUFFER)
;	BX 	:    	Number of Bytes to allocate
;
; Returns:
;	BX 	:	Offset of requested memory if success, else 0
;	AL 	:    	Status (JSERV_OK, JSERV_NOJANUSMEM)
;-------------------------------------------------------------------- 
;
public	       AllocMem
AllocMem      proc	near

	push	cx
	push	dx
	push	di
	push	si
	push	es
	push	ds

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
	JLOCK	es:JanusAmiga.jbm_Lock
	lea	si,es:JanusAmiga.jbm_First
	push	si				; save back pointer
	push	es
	mov	si,es:JanusAmiga.jbm_First	; points to 1. buffer mem chunk
	mov	ax,es:JanusAmiga.jbm_8088Segment
 	mov	ds,ax				; DS=buffer mem seg
	jmp	Find_Mem

Lock_Para:		
	JLOCK	es:JanusAmiga.jpm_Lock
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
	UNLOCK	es:JanusAmiga.jbm_Lock	
	jmp	End_Mem

Update_Para:
	sub	es:JanusAmiga.jpm_Free,bx	; update Free count
	UNLOCK	es:JanusAmiga.jpm_Lock	

End_Mem:	
	add	si,4
	mov	bx,si				; mem pointer now BX
	mov	al,JSERV_OK			; set OK flag
	jmp	exit_Mem

Mem_Failed:					; unlock memory,
	pop	ds 				; restore back pointer,
	pop	si
	UNLOCK	es:JanusAmiga.jbm_Lock	
	UNLOCK	es:JanusAmiga.jpm_Lock	

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


;-------------------------------------------------------------------- 
;
; Service 3: FREE_MEM		 Releases Janus Memory
; ===================		 Caution: Crash if offset/type is wrong 
;
;
; Expects:
;	AH	:	JFUNC_FREEMEM
;	AL 	: 	Type of Memory to free (MEMF_PARAMETER, MEMF_BUFFER)
;	BX 	:    	Offset of Memory to free
;
; Returns:
; 	AL 	: 	Status (JSERV_OK, JSERV_NOJANUSMEM)
;--------------------------------------------------------------------
;
public	       FreeMem
FreeMem       proc	near

	push	cx
	push	dx
	push	di
	push	si
	push	es
	push	ds

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
	cmp	bx,es:JanusAmiga.jbm_Max		; BX in range ?
	jae	Bad_Range
	JLOCK	es:JanusAmiga.jbm_Lock
	lea	si,es:JanusAmiga.jbm_First
	push	si				; save back pointer
	push	es
	mov	si,es:JanusAmiga.jbm_First	; points to 1. buffer mem chunk
	mov	ax,es:JanusAmiga.jbm_8088Segment
 	mov	ds,ax				; DS=buffer mem seg
	mov	ax,ds:[bx]
	add	es:JanusAmiga.jbm_Free,ax	; update Free count
	jmp	Find_Head

FLock_Para:		
	cmp	bx,es:JanusAmiga.jpm_Max		; BX in range ?
	jae	Bad_Range
	JLOCK	es:JanusAmiga.jpm_Lock
	lea	si,es:JanusAmiga.jpm_First		
	push	si				; save back pointer
	push	es
	mov	si,es:JanusAmiga.jpm_First	; points to 1. para mem chunk	
	mov	ax,es:JanusAmiga.jpm_8088Segment
 	mov	ds,ax				; DS=para mem seg
	mov	ax,ds:[bx]
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
	mov	ax,es:JanusMemChunk.jmc_Size[di]	;  and AX, may be we 
	mov	bx,di					;  can combine with
							;  next mem chunk
Check:	 						; check next	 
	add	ax,bx
	cmp	ax,si	
	jne	No_Combine

; -------   combine next and curent chunks

	mov	ax,ds:JanusMemChunk.jmc_Size[si]       
	add	ds:JanusMemChunk.jmc_Size[bx],ax 
	mov	ax,ds:JanusMemChunk.jmc_Next[si]	
	mov	ds:JanusMemChunk.jmc_Next[bx],ax	

No_Combine:
	mov	al,JSERV_OK				; set OK flag
	UNLOCK	es:JanusAmiga.jbm_Lock	
	UNLOCK	es:JanusAmiga.jpm_Lock	
	jmp	exit_Mem

FreeMem       endp


;--------------------------------------------------------------------
;
; Service 4: SET_PARAM		 Set the default parameter memory pointer
; ====================
;
; Expects:
;	AH	:	JFUNC_SETPARAM
;	AL 	: 	Janus Service Number to support
;	BX 	:	Default Offset of Param Memory to install
;
; Returns:
;	AL 	: 	Status (JSERV_OK)
;--------------------------------------------------------------------
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


;--------------------------------------------------------------------
;
; Service 7: CALL_AMIGA		Calls the requested function on AMIGA side.
; =====================		Does not wait for the call to complete.
;				Returns with ERROR, if Service# out of range.
;
; Expects:
;	AH	:	JFUNC_CALLAMIGA
;	AL 	: 	AMIGA Service to call
;				    		
; Returns:
;	AL 	: 	Status (JSERV_PENDING,JSERV_FINISHED,JSERV_NOSERVICE)
;--------------------------------------------------------------------
;
; Service 8: WAIT_AMIGA		Waits for a previos issued CALL_AMIGA 
; ===================== 	to complete. 
;
; Expects:
;	AH	:	JFUNC_WAITAMIGA
;	AL 	:	Service Number to wait for
;
; Returns:
;	AL 	:    	Status (JSERV_FINISHED, JSERV_NOSERVICE)
;--------------------------------------------------------------------
;
; Service 9: CHECK_AMIGA	Checks completion status 
; ======================	of a pending CALL_AMIGA
;
; Expects:
;	AH	:	JFUNC_CHECKAMIGA
;	AL 	: 	Service Number to check
;
; Returns:
;	AL 	:	Status (JSERV_PENDING,JSERV_FINISHED,JSERV_NOSERVICE)
;
;--------------------------------------------------------------------
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