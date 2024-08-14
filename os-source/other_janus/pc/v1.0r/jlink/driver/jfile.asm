;
;
; DiskClose
;
; Close Virtual Disk File
;
; Expects FCB pointer in SI
;
DiskClose:
	push	es
	push	di

	les	di,dword ptr CS:[si].IL_JParam_Offs

	mov	es:[di].adr_Fnctn,ADR_FNCTN_CLOSE
	mov	ax,[si].IL_Handle		; file handle
	mov	es:[di].adr_File,ax

	call	DoJanus

	pop	di
	pop	es
	ret

;
; Read Sectors from disk
;
; Expects:	Pointer to File Block in CS:[si]
;		Sector Count in	CX
;		Buffer Address in DS:DX
;		Start Sector in BX
;
; Returns:	Actual Count in CX
;		Error Status in AX
;
DiskRead:

	push    es
	push	di
	push	bp
	sub	bp,bp
	
	les	di,dword ptr CS:[si].IL_JParam_Offs

	mov	es:[di].adr_Fnctn,ADR_FNCTN_READ

	mov	ax,CS:[si].IL_Handle		; file handle
	mov	es:[di].adr_File,ax

	call	jrw_setOffs

readloop:

	call	jrw_setCount
;
; Call the famous Janus service
;
	call	DoJanus
	mov	al,E_General
	jne	rddone				; Janus Err if ne

	cmp	es:[di].adr_Err,ADR_ERR_OK	; File err if ne
	mov	al,E_ReadFault
	jne	rddone

	cmp	bx,es:[di].adr_Count_l		; Count wrong if ne
	mov	al,E_CRC
	jne	rddone
;
; move that block
;
	add	bp,cx				; count down

	push	es				; preserve
	push	ds
	push	si
	push	cx
 
	push	ds				; there we want it
	pop	es

	xchg	di,dx
	lds	si,dword ptr CS:[si].IL_JBuffer_Offs	; there it is
	mov	cx,bx  				; count
	shr	cx,1 				; always even
	rep	movsw				; more efficient
	xchg	di,dx				; pointer updated

	pop	cx				; restore
	pop	si
	pop	ds
	pop	es

	or	cx,cx				; sectors left ?
	jnz	readloop
	sub	ax,ax

rddone: mov	cx,bp				; actual count left

	pop	bp				; orig. count
	pop	di
	pop	es
	ret
	
;
;
; Write Sectors to disk
;
; Expects:	Pointer to [FCB] in CS:[si]
;		Sector Count in	CX
;		Buffer Address in DS:DX
;		Start Sector in BX
;
; Returns:	Actual Count in CX
;		Error Status in AX
;
DiskWrite:

	push    es
	push	di
	push	bp

	sub	bp,bp

	les	di,dword ptr CS:[si].IL_JParam_Offs

	mov	es:[di].adr_Fnctn,ADR_FNCTN_WRITE
	mov	ax,cs:[si].IL_Handle		; file handle
	mov	es:[di].adr_File,ax

	call	jrw_setOffs

writeloop:
	call	jrw_setCount
;
; move that block to write buffer
;
	push	cx
	push	es
	push	di

	mov	cx,bx
	les	di,dword ptr CS:[si].IL_JBuffer_Offs	; there it is

	xchg	si,dx				; source buffer
	shr	cx,1 				; always even
	rep	movsw				; more efficient
	xchg	si,dx				; pointer updated

	pop	di
	pop	es
	pop	cx				; restore  count

;
; Call the famous Janus service
;
	call	DoJanus
	mov	al,E_General
	jne	wrdone				; Janus Err if ne

	cmp	es:[di].adr_Err,ADR_ERR_OK	; File err if ne
	mov	al,E_WriteFault
	jne	wrdone

	cmp	bx,es:[di].adr_Count_l		; Count wrong if ne
	mov	al,E_CRC
	jne	wrdone

	add	bp,cx				; add sects xferred
		     	
	or	cx,cx
	jnz	writeloop

	sub	ax,ax

wrdone: 
	mov	cx,bp
	pop	bp
	pop	di
	pop	es
	ret

;
; Calculate byte count
;
jrw_setCount:

	mov	bx,cx				; save a copy
	cmp	bx,16				; max count
	jb	jrw10
	mov	bx,16

jrw10:	sub	cx,bx				; sectors left
	xchg	bl,bh				; shl 8
	shl	bx,1				; times 512 total

	mov	es:[di].adr_Count_l,bx		; set up count
 	mov	es:[di].adr_Count_h,0

	ret
;
; Calculate byte offset of virtual disk file
;
jrw_setOffs:
	mov	es:[di].adr_Offset_l,0		; set up offet
	mov	es:[di].adr_Offset_h,0
	mov	byte ptr es:[di].adr_Offset_l+1,bl
	mov	byte ptr es:[di].adr_Offset_h,bh
	shl	es:[di].adr_Offset_l,1
	rcl	es:[di].adr_Offset_h,1
	ret
;
; Send a FILE IO Request to Janus
;	
DoJanus:
	push	ax
	push	bx

	mov	ah,J_CALL_AMIGA
	mov	al,J_JFT
	mov	bx,0ffffh
	int	JANUS
	cmp	al,J_FINISHED
	je	DJret
	cmp	al,J_PENDING
	jne	DJret
	mov	ah,J_WAIT_AMIGA
	mov	al,J_JFT
	int	JANUS
	cmp	al,J_FINISHED

DJret: 	pop	bx
	pop	ax
	ret



