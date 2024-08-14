page 63,132
;*****************************************************************************
; Janus Service Handler for PC:	 (1.Generation)
;
;
;
; New code  :	30-Mar-88 Torsten Burgdorff
; 44.Update :  	 1-Apr-88 TB	add service 0,4,5
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
		
include        	janusvar.inc
include        	macros.inc
include        	vars_ext.inc
include        	mes.inc
include        	service.inc

;--------------------------------------------------------------------
;
; Service 0: GET_SERVICE	 Gets a new Service Number
; ========================
;
; Expects:
;
;	nothing
;
; Returns:
;
;	AL : New Service Number to use
;	     -1 if no service available (J_NO_SERVICE)
;--------------------------------------------------------------------
	
public	       	GetService1
GetService1    	proc	near

	mov	al,J_NO_SERVICE
	ret

GetService1    	endp


;--------------------------------------------------------------------
;
; Service 1: GET_BASE 		Gets Segmemts & offset of Janus Memory
; ===================
;
; Expects:
;
; 	AL :	Janus Service Number
;
; Returns :
;
; 	ES : 	Janus Parameter Segment
; 	DI : 	Janus Parameter Offset (if defined),
; 		else -1
; 	DX :	Janus Buffer Segment
; 	AL :	Status (J_OK, J_NO_SERVICE)
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
	mov	si,es:JanusBase.jb_Parameters	; points to parameter table
	shl	bx,1
	mov	di,es:[si][bx]			; 3.Para offset
	pop	ax
	pop 	bx	
	pop	si
	mov     al,J_OK
	ret

GetBase       endp


;--------------------------------------------------------------------
;
; Service 2: ALLOC_MEM		 Allocates Janus Memory
; ====================
;
; Expects:
;
;	AL : 	Type of memory to allocate
;	BX :    Number of Bytes to allocate
;
; Returns:
;
;	BX : 	Offset of requested memory if success,
;	AL :    Status (J_OK, J_NO_MEMORY)
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
	cmp	cl,Para_type
	je 	Lock_Para
	cmp	cl,Buff_type
	je	Lock_Buff
	jmp 	Mem_Type_Failed

Lock_Buff:
	JLOCK	es:JanusBase.jbm_Lock
	lea	si,es:JanusBase.jbm_First
	push	si				; save back pointer
	push	es
	mov	si,es:JanusBase.jbm_First	; points to 1. buffer mem chunk
	mov	ax,es:JanusBase.jbm_8088Segment
 	mov	ds,ax				; DS=buffer mem seg
	jmp	Find_Mem

Lock_Para:		
	JLOCK	es:JanusBase.jpm_Lock
	lea	si,es:JanusBase.jpm_First		
	push	si				; save back pointer
	push	es
	mov	si,es:JanusBase.jpm_First	; points to 1. para mem chunk	
	mov	ax,es:JanusBase.jpm_8088Segment
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
	cmp	cl,Para_Type
	je	Update_Para

	sub	es:JanusBase.jbm_Free,bx		; update Free count
	UNLOCK	es:JanusBase.jbm_Lock	
	jmp	End_Mem

Update_Para:
	sub	es:JanusBase.jpm_Free,bx	; update Free count
	UNLOCK	es:JanusBase.jpm_Lock	

End_Mem:	
	add	si,4
	mov	bx,si				; mem pointer now BX
	mov	al,J_OK				; set OK flag
	jmp	exit_Mem

Mem_Failed:					; unlock memory
	pop	ds 				; restore back ptr
	pop	si
	UNLOCK	es:JanusBase.jbm_Lock	
	UNLOCK	es:JanusBase.jpm_Lock	

Mem_Type_Failed:
	mov	al,J_NO_MEMORY			; and set error flag

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
; ===================
;
; Expects:
;
;	AL : 	Type of memory to free
;	BX :    Offset of Memory to free
;
; Returns:
;
; 	Crash if offset/type was wrong (J_GOODBYE, later)
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
	cmp	cl,Para_type
	je 	FLock_Para
	cmp	cl,Buff_type
	je	FLock_Buff
	jmp 	Mem_Type_Failed

FLock_Buff:
	cmp	bx,es:JanusBase.jbm_Max		; BX in range ?
	jae	Bad_Range
	JLOCK	es:JanusBase.jbm_Lock
	lea	si,es:JanusBase.jbm_First
	push	si				; save back pointer
	push	es
	mov	si,es:JanusBase.jbm_First	; points to 1. buffer mem chunk
	mov	ax,es:JanusBase.jbm_8088Segment
 	mov	ds,ax				; DS=buffer mem seg
	mov	ax,ds:[bx]
	add	es:JanusBase.jbm_Free,ax	; update Free count
	jmp	Find_Head

FLock_Para:		
	cmp	bx,es:JanusBase.jpm_Max		; BX in range ?
	jae	Bad_Range
	JLOCK	es:JanusBase.jpm_Lock
	lea	si,es:JanusBase.jpm_First		
	push	si				; save back pointer
	push	es
	mov	si,es:JanusBase.jpm_First	; points to 1. para mem chunk	
	mov	ax,es:JanusBase.jpm_8088Segment
 	mov	ds,ax				; DS=para mem seg
	mov	ax,ds:[bx]
	add	es:JanusBase.jpm_Free,ax	; update Free count
	jmp	Find_Head

Bad_Range:
	mov	al,J_NO_MEMORY			; and set error flag
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

;	mov	ax,ds:[bx]
;	mov	ds:JanusMemChunk.jmc_Size[bx],ax	; create size entry
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
	mov	al,J_GOODBYE				; set OK flag
	jmp	exit_Mem

FreeMem       endp


;--------------------------------------------------------------------
;
; Service 4: SET_PARAM		 Set the default parameter memory pointer
; ====================
;
; Expects:
;	
;	AL : 	Janus Service Number to support
;	BX :	Default Offset of Param Memory to install
;
; Returns:
;
;	AL : 	Status (J_OK, J_NO_SERVICE)
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
	mov	si,es:JanusBase.jb_Parameters	; points to parameter table
	shl	bx,1
	mov	es:[si][bx],di			; fill in the new offset
	POPALL
	mov     al,J_OK
	ret

SetParam      endp


;--------------------------------------------------------------------
;
; Service 5: SET_SERVICE	Set an address for a far call 
; ======================	for that service
;
; Expects:
;	
;	AL : 	Janus Service Number to support
;	ES:DX :	Entry address for FAR call
;
; Returns:
;
;	AL : 	Status (J_OK, J_NO_SERVICE)
;--------------------------------------------------------------------
;
public	       SetService
SetService    proc	near

	mov	al,J_NO_SERVICE
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
;	AL :	Number of Service to stop
;
; Returns:
;
;	AL : 	Status (J_OK, J_NO_SERVICE)
;
;--------------------------------------------------------------------
;
public	       StopService
StopService   proc	near

	mov	al,J_NO_SERVICE
	ret

StopService   endp


;--------------------------------------------------------------------
;
; Service 7: CALL_AMIGA		Calls the requested function on AMIGA side.
; =====================		Does not wait for the call to complete.
; 				If J_SET_SERVICE defined, calls it on completion.
;
; Expects:
;	AL : 	AMIGA Service to call
;	BX : 	New Parameter Memory offset to use,
;		-1 : Use default offset
;				    		
; Returns:
;	AL : 	Status (J_OK, J_NO_SERVICE)
;--------------------------------------------------------------------
;
; Service 8: J_WAIT_AMIGA	Waits for a previos issued CALL_AMIGA 
; ======================= 	to complete. This function is used if
;				no SET_SERVICE is defined.
;
; Expects:
;	AL :	Service Number to wait for
;
; Returns:
;	AL :    Status (J_FINISHED, J_NO_SERVICE)
;--------------------------------------------------------------------
;
; Service 9: J_CHECK_AMIGA	Checks completion status 
; ========================	of a pending CALL_AMIGA
;
; Expects:
;	AL : 	Service Number to check
;
; Returns:
;	AL :	Status (J_PENDING, J_FINISHED, J_NO_SERVICE)
;
;--------------------------------------------------------------------
;
public	       CallAmiga, WaitAmiga, CheckAmiga

CallAmiga     proc	near

	cmp	bx,-1
	je	Def_Mem
	mov	al,J_No_service		; cannot handle this feature now
	ret

Def_Mem:
	push	ax
	push	bx
	xor	bh,bh
	mov	bl,al					; service # in BX
	mov	byte ptr cs:ServStatTab[bx],J_PENDING 	; set status = pending
	call	SendJanusInt			; Int # in BL
	pop     bx  				; fall into Check routine
	pop	ax				; with AH=0


CheckAmiga:					; Service 9 -----------------
						; AH=0 on this entry

WaitAmiga:					; Service 8 -----------------
   						; AH=1 on this entry 
;	call 	JanIntEn			; enable janus interrupts
	sti					; enable ints now
	push	bx
	xor	bh,bh
	mov	bl,al 				; service # in BX

Wait_Amiga_Ready:
 	mov	al,byte ptr cs:ServStatTab[bx] 	; read service status
	or	ah,ah				; called via Wait_Amiga ?
	jz	Wait_Amiga_Exit			; AH=1 means yes !		
	cmp	al,J_FINISHED
	jne	Wait_Amiga_Ready		; loop if pending

Wait_Amiga_Exit:
	pop	bx
	INFO_AX WaitStatus
	ret

CallAmiga	endp


cseg		ends

end
