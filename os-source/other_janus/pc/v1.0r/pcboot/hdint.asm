     page 63,132
     include   macros.inc
     include   dskstruc.inc
     include   words.inc

w    equ  word ptr

cseg segment   para public 'code'

     assume    cs:cseg,ds:cseg,ss:nothing,es:nothing
;
; We export:
;
     public    janus_dsk_abs		; absolute disko io	 
     public    janus_disk_io		; logical disk io for amiga
;
; Global Data references:
;
     extrn     pstrng:near
     extrn     outhxw:near
     extrn     newline:near

     include   mes.inc
     include   equ.inc
     include   data.inc

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

janus_dsk_abs proc near

	  pushall
	  push es
	  pop  ds
	  mov  si,di		   

	  mov  ah,byte ptr [si].dsk_abs_fkt_code
	  mov  al,byte ptr [si].dsk_abs_count
	  mov  ch,byte ptr [si].dsk_abs_track
	  mov  cl,byte ptr [si].dsk_abs_track+1
	  ror  cl,1
	  ror  cl,1
	  or   cl,byte ptr [si].dsk_abs_sector
	  mov  dl,byte ptr [si].dsk_abs_drive
	  mov  dh,byte ptr [si].dsk_abs_head

	  if   (infolevel-49 ge 0)	 
	   cmp	ch,3
	   jae	no_output
	   push ds   
	   push cs
	   pop	ds
	   push ax
	   push si
	   mov	si,offset regmsg
	   call pstrng
	   call outhxw
	   mov	ax,bx
	   call outhxw
	   mov	ax,cx
	   call outhxw
	   mov	ax,dx
	   call outhxw
	   call newline
	   pop	si
	   pop	ax
	   pop	ds
no_output:
	  endif
 
	  mov  bx,[si].dsk_abs_offset
	  mov  es,janus_buffer_seg

	  pushf 			; stack like int now
	  call dword ptr [bios_int13]	; call long instead of int

	  sub  al,al
	  mov  [si].dsk_abs_status,ax
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
;* Returns: status in dsk_err						  *
;*									  *
;* All Registers preserved.						  * 
;**************************************************************************
janus_disk_io proc   near
	  pushall

	  push cs
	  pop  ds		   ; make sure we are home

	  mov  cx,es:[di].dsk_part ; get partition number
	  mov  ax,dsk_err_part	   ; if not exist
	  cmp  es:[di].dsk_fnctn,dsk_fnctn_init
	  je   noprt
;
; find requested partition descriptor
;
	  inc  cx
	  mov  bx,janus_part_base	; get partition pointer
	  jmp  short prtfnd

fndprt:   mov  bx,[bx].part_next	; get next partion pointer

prtfnd:   or   bx,bx			; valid ?
	  jz   goto_status		; no if z
	  loop fndprt			; until done
					; now ds:bx points to our part descr.
noprt:
	  mov  si,es:[di].dsk_fnctn	; what shall we do ?
	  mov  ax,dsk_err_fnct		; dont't know the command
	  cmp  si,maxfnctn
	  ja   goto_status

goahead:
	  if   (infolevel-48 ge 0)	 
	   push ds
	   push cs
	   pop	ds
	   push si
	   mov	si,offset diskmsg1
	   call pstrng
	   pop	ax			; get SI from stack
	   push ax
	   call outhxw
	   call newline
	   pop	si
	   pop	ds
	  endif

	  sub  ax,ax
	  shl  si,1
	  jmp  word ptr cs:dsk_do [si]


init_dsk: 
	  mov  bx,janus_part_base
	  jmp  short init_1st

init_nxt: mov  bx,[bx].part_next	; next list link
	  inc  ax			; inc partiton count
init_1st: or   bx,bx			; is it valid ?
	  jne  init_nxt 		; yes if ne

	  mov  es:[di].dsk_part,ax	; mark # of partitions found
	  sub  ax,ax			; no error

goto_status:
	  jmp  janus_dsk_status

janu_info:
	  mov  si,bx			     ; current partion info pointer
	  cmp  w es:[di].dsk_count+high_word,0
	  jnz  janu_info_2
	  mov  cx,w es:[di].dsk_count+low_word ; that much space we have
	  cmp  cx,size dsk_partition	   ; more than we need ?
	  jb   janu_info_1		     ; if b, less	   
janu_info_2:
	  mov  cx,size dsk_partition	   ; else truncate
	  mov  w es:[di].dsk_count+low_word,cx ; and mark it
	  mov  w es:[di].dsk_count+high_word,0
janu_info_1:
	  push es			     ; save req pointer
	  push di
	  mov  di,es:[di].dsk_buffer_adr     ; pick up target addr
	  mov  es,janus_buffer_seg	     ; set up segment
	  rep  movsb			     ; move block
	  pop  di			     ; recover
	  pop  es			     ; ax still zero

	  jmp  janus_dsk_status 

janu_do_io:
	  sub  sp,size dsk_abs_req	; get some space
	  mov  bp,sp			; there we are

	  mov  ax,dsk_cmd [si]
	  mov  [bp].dsk_abs_fkt_code,ax ; hard work begins now ....
	  mov  ax,dsk_err_offset
	  call log_to_phys		; fill in track,sector,head
	  jnz  janus_dsk_err		; something wrong if nz

	  mov  cx,ax			; check partition boundary
	  mov  ax,dsk_err_dsk_full	; disk full ?
	  cmp  cx,[bx].part_end_cyl
	  ja   janus_dsk_err		; yes if above
	  cmp  cx,[bx].part_base_cyl
	  jb   janus_dsk_err

	  mov  ax,[bx].part_drvnum	; move drive numebr  
	  mov  [bp].dsk_abs_drive,ax
	  mov  [bp].dsk_abs_status,0	; clear

	  cmp  [bp].dsk_abs_fkt_code,0ch ; seek ?
	  je   do_io_1

	  mov  ax,dsk_err_count 	; sector size ok ?
	  mov  dx,word ptr es:[di].dsk_count+high_word
	  mov  cx,word ptr es:[di].dsk_count+low_word
	  call byte_to_block		; get block count

	  jnz  janus_dsk_err		; nz means alignment error
	  mov  [bp].dsk_abs_count,ax	; save count
	  mov  ax,es:[di].dsk_buffer_adr; move buffer pointer
	  mov  [bp].dsk_abs_offset,ax
do_io_1:

	  push es
	  push di

	  push ss
	  pop  es
	  mov  di,bp

	  call janus_dsk_abs		; do it

	  pop  di
	  pop  es

	  mov  ax,[bp].dsk_abs_status	; pick up status

janus_dsk_err:
	  add  sp,size dsk_abs_req	; cancel frame

janus_dsk_status:
	  mov  es:[di].dsk_err,ax
	  popall
	  ret				; done	puuuh ! 
janus_disk_io endp
;
; dispatcher info tables
;
dsk_cmd   dw   0,2,3,0ch,0		    ; pc bios commands

dsk_do	  dw   offset init_dsk		    ; init    
	  dw   offset janu_do_io	    ; read
	  dw   offset janu_do_io	    ; write
	  dw   offset janu_do_io	    ; seek
	  dw   offset janu_info 	    ; info

maxfnctn   =   4
;
; local aux procedures
;
byte_to_block  proc near
	  test cx,1ffh			; sector alignment ok ?
	  jnz  btbe			; not if nz
	  shr  dx,1			; convert to logical sector #
	  rcr  cx,1
	  mov  al,ch
	  mov  ah,dl
	  mov  dl,dh
	  sub  dh,dh			; lsw in ax, msw in dx ,zf ret
btbe:	  ret
byte_to_block endp
 
log_to_phys    proc near
	  mov  dx,word ptr es:[di].dsk_offset+high_word
	  mov  cx,word ptr es:[di].dsk_offset+low_word
	  call byte_to_block		; get block count
	  jnz  ltpys			; nz means alignment error

	  div  [bx].part_numsecs	; divide by sectors/track
	  inc  dx			; first sector in #1 !
	  mov  [bp].dsk_abs_sector,dx	; now we have the sector #
	  sub  dx,dx			; zero high word
	  div  [bx].part_numheads	; div by # of heads
	  mov  [bp].dsk_abs_head,dx	; remainder in dl is head #
	  add  ax,[bx].part_base_cyl	; add partition base cyl
	  mov  [bp].dsk_abs_track,ax	; this is our track
	  cmp  ax,ax			; return zf=on=ok
ltpys:	  ret
log_to_phys    endp


cseg	  ends

	  end
