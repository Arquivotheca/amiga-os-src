TITLE	HDINT  -  COPYRIGHT (C) 1986 - 1988 Commodore Amiga Inc. 
PAGE	60,132	
;**************************************************************************
;* Module: hdint     Entry: janus_dsk_abs	  Type: Near		  *
;*									  *
;* Function: Perform Janus Disk IO - physical disk access		  *
;*									  *
;* Parameters: ES:DI - Pointer to a dsk_abs_req 			  *
;*									  *
;* Returns: status in dsk_abs_status					  *
;*									  *
;* All Registers preserved.						  * 
;**************************************************************************
			 	
public    janus_dsk_abs		; absolute disko io	 
public    janus_disk_io		; logical disk io for amiga

cseg segment   para public 'code'
assume    cs:cseg,ds:cseg,ss:nothing,es:nothing

extrn     pstrng:near
extrn     outhxw:near
extrn     newline:near

include   equ.inc
include   janus\harddisk.inc		
include   debug.inc
include   vars_ext.inc
include   macros.inc
include   words.inc

w    equ  word ptr


janus_dsk_abs 	proc near

	pushall
	push 	es
	pop  	ds
	mov  	si,di		   

	mov  	ah,byte ptr [si].har_fktCode
	mov  	al,byte ptr [si].har_count
	mov  	ch,byte ptr [si].har_track
	mov  	cl,byte ptr [si].har_track+1
	ror  	cl,1
	ror  	cl,1
	or   	cl,byte ptr [si].har_sector
	mov  	dl,byte ptr [si].har_drive
	mov  	dh,byte ptr [si].har_head

	if   	DBG_regmsg	 
	 cmp	ch,3
	 jae	no_output
	 push	ds   
	 push	cs
	 pop	ds
	 push	ax
	 push	si
	 mov	si,offset regmsg
	 call	pstrng
	 call	outhxw
	 mov	ax,bx
	 call	outhxw
	 mov	ax,cx
	 call	outhxw
	 mov	ax,dx
	 call	outhxw
	 call	newline
	 pop	si
	 pop	ax
	 pop	ds
no_output:
	endif
 
	mov  	bx,[si].har_offset
	mov  	es,cs:janus_buffer_seg

	pushf 					; stack like int now
	call 	dword ptr cs:[bios_int13]	; call long instead of int
	     	
	sub  	al,al
	mov  	[si].har_status,ax
	popall 
	ret

janus_dsk_abs endp


;**************************************************************************
;* Module: hdint     Entry: janus_disk_io	  Type: Near		  *
;*									  *
;* Function: Perform Janus Disk IO - logical disk access		  *
;*									  *
;* Parameters: ES:DI - Pointer to a amiga_dsk_req structure		  *
;*									  *
;* Returns: status in hdr_err						  *
;*									  *
;* All Registers preserved.						  * 
;**************************************************************************

janus_disk_io proc   near

	pushall
	push 	cs
	pop  	ds		   	; make sure we are home

	mov  	cx,es:[di].hdr_part 	; get partition number
	mov  	ax,hdr_err_part	   	; if not exist
	cmp  	es:[di].hdr_fnctn,hdr_fnctn_init
	je   	noprt
;
; find requested partition descriptor
;
	inc  	cx
	mov  	bx,janus_part_base	; get partition pointer
	jmp  	short prtfnd

fndprt: mov  	bx,[bx].hdp_next	; get next partion pointer

prtfnd: or   	bx,bx			; valid ?
	jz   	goto_status		; no if z
	loop 	fndprt			; until done
					; now ds:bx points to our part descr.
noprt:
	mov  	si,es:[di].hdr_fnctn	; what shall we do ?
	mov  	ax,hdr_err_fnct		; dont't know the command
	cmp  	si,maxfnctn
	ja   	goto_status

goahead:
	if   	DBG_diskmsg1	 
	 push	ds
	 push	cs
	 pop	ds
	 push	si
	 mov	si,offset diskmsg1
	 call	pstrng
	 pop	ax			; get SI from stack
	 push	ax
	 call	outhxw
	 call	newline
	 pop	si
	 pop	ds
	endif

	sub  	ax,ax
	shl  	si,1
	jmp  	word ptr cs:hdr_do [si]

init_dsk: 
	mov  	bx,cs:janus_part_base
	jmp  	short init_1st

init_nxt:
	mov  	bx,[bx].hdp_next		; next list link
	inc  	ax				; inc partiton count
init_1st:
	or   	bx,bx				; is it valid ?
	jne  	init_nxt 			; yes if ne

	mov  	es:[di].hdr_part,ax		; mark # of partitions found
	sub  	ax,ax				; no error

goto_status:
	jmp  	janus_dsk_status

janu_info:
	mov  	si,bx			     	; current partion info pointer
	cmp  	w es:[di].hdr_count+high_word,0
	jnz  	janu_info_2
	mov  	cx,w es:[di].hdr_count+low_word ; that much space we have
	cmp  	cx,size HDskPartition	   	; more than we need ?
	jb   	janu_info_1		     	; if b, less	   
janu_info_2:
	mov  	cx,size HDskPartition	   	; else truncate
	mov  	w es:[di].hdr_count+low_word,cx ; and mark it
	mov  	w es:[di].hdr_count+high_word,0
janu_info_1:
	push 	es			     	; save req pointer
	push 	di
	mov  	di,es:[di].hdr_bufferAddr     	; pick up target addr
	mov  	es,cs:janus_buffer_seg	     	; set up segment

	if   DBG_BlockMove	 
	 push 	ds
	 push	cs
	 pop	ds
	 push 	si
 	 push	ds
	 push	di
	 push	es
 	 pop 	ax
	 call	outhxw			; print ES
 	 pop 	ax
	 call	outhxw			; print DI
 	 pop 	ax
	 call	outhxw			; print DS
	 pop	ax			; get SI from stack
	 push	ax
	 call	outhxw			; print SI
	 call	newline
	 pop	si
	 pop	ds
	endif

	rep  	movsb			; move block
	pop  	di			; recover
	pop  	es			; ax still zero

	jmp  	janus_dsk_status 

janu_do_io:
	sub  	sp,size HDskAbsReq	; get some space
	mov  	bp,sp			; there we are

	mov  	ax,hdr_cmd [si]
	mov  	[bp].har_fktCode,ax 	; hard work begins now ....
	mov  	ax,hdr_err_offset
	call 	log_to_phys		; fill in track,sector,head
	jnz  	janus_dsk_err		; something wrong if nz

	mov  	cx,ax			; check partition boundary
	mov  	ax,hdr_err_eof		; disk full ?
	cmp  	cx,[bx].hdp_endCyl
	ja   	janus_dsk_err		; yes if above
	cmp  	cx,[bx].hdp_baseCyl
	jb   	janus_dsk_err

	mov  	ax,[bx].hdp_drvnum	; move drive numebr  
	mov  	[bp].har_drive,ax
	mov  	[bp].har_status,0	; clear

	cmp  	[bp].har_fktCode,0ch 	; seek ?
	je   	do_io_1

	mov  	ax,hdr_err_count 		; sector size ok ?
	mov  	dx,word ptr es:[di].hdr_count+high_word
	mov  	cx,word ptr es:[di].hdr_count+low_word
	call 	byte_to_block			; get block count

	jnz  	janus_dsk_err			; nz means alignment error
	mov  	[bp].har_count,ax		; save count
	mov  	ax,es:[di].hdr_bufferAddr	; move buffer pointer
	mov  	[bp].har_offset,ax

do_io_1:
	push 	es
	push 	di
	push 	ss
	pop  	es
	mov  	di,bp
	call 	janus_dsk_abs		; do it
	pop  	di
	pop  	es
	mov  	ax,[bp].har_status	; pick up status

janus_dsk_err:
	add  	sp,size HDskAbsReq	; cancel frame

janus_dsk_status:
	mov  	es:[di].hdr_err,ax
	popall
	ret				; done	puuuh ! 

janus_disk_io 	endp

;
; dispatcher info tables
;
hdr_cmd   dw   0,2,3,0ch,0		    ; pc bios commands

hdr_do	  dw   offset init_dsk		    ; init    
	  dw   offset janu_do_io	    ; read
	  dw   offset janu_do_io	    ; write
	  dw   offset janu_do_io	    ; seek
	  dw   offset janu_info 	    ; info

maxfnctn   =   4

;
; local aux procedures
;
byte_to_block  proc near

	test 	cx,1ffh			; sector alignment ok ?
	jnz  	btbe			; not if nz
	shr  	dx,1			; convert to logical sector #
	rcr  	cx,1
	mov  	al,ch
	mov  	ah,dl
	mov  	dl,dh
	sub  	dh,dh			; lsw in ax, msw in dx ,zf ret
btbe:	ret  	

byte_to_block 	endp

 
log_to_phys     proc near

	mov  	dx,word ptr es:[di].hdr_offset+high_word
	mov  	cx,word ptr es:[di].hdr_offset+low_word
	call 	byte_to_block		; get block count
	jnz  	ltpys			; nz means alignment error

	cmp	dx,1
	jbe	div_ok
	xor 	dx,dx			; illegal offset found
	xor	ax,ax			; reset offset
div_ok:
	div  	[bx].hdp_numsecs	; divide by sectors/track
	inc  	dx			; first sector in #1 !
	mov  	[bp].har_sector,dx	; now we have the sector #
	sub  	dx,dx			; zero high word
	div  	[bx].hdp_numheads	; div by # of heads
	mov  	[bp].har_head,dx	; remainder in dl is head #
	add  	ax,[bx].hdp_baseCyl	; add partition base cyl
	mov  	[bp].har_track,ax	; this is our track
	cmp  	ax,ax			; return zf=on=ok
ltpys:	ret

log_to_phys     endp


cseg	  ends

	  end
