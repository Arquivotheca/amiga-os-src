
page 	60,132
include	title.inc
subttl	INTR 15H

.xlist
include equates.inc
include externals.inc

extrn	shut_down:near
extrn	write_cmos:near
extrn	shut:near
public	shutdown_9
public	Intr_15

extrn	config_table:byte		;TOP8K.ASM
extrn	event_wait_flag:byte
extrn	count_low:word
extrn	count_high:word
extrn	event_flag_addr:word
extrn	event_flag_seg:word
extrn	rom_address:word
extrn	rom_segment:word
extrn	interrupt_flag:word
extrn	shut_down:near
public	end_of_int15
public	gate_A20
.list

;	define some local equates

set_A20	equ	0dfh		;set address A20 bit
reset_A20	equ	0ddh		;reset address A20 bit
dis_parity	equ	0ch		;
en_parity	equ	0f3h		;

zero	segment at 0
zero	ends


dataa	segment at 40h
dataa	ends

.286P

eproma	segment byte public

.xlist
start_of_int15	equ	$+41dh
.list	

	Assume cs:eproma, ds:dataa, es:zero
page
Intr_15		proc	far

	sti				;enable interrupts again
	push	ax
	call	save_all			;save all registers
	cmp	ah,80h			;check for valid function code
	jb	intr15_err		;error return if between 00-7f

; ToBu (20) 19.9.88 Ver 3.4   new func 0c0h
	cmp	ah,0c0h			;see if func c0h
	je	func_c0			;yes, return config parameter table addr

	and	ah,07fh			;subtract 80 from function code
	mov	al,ah			;
	xor	ah,ah
	cmp	al,max_cmd		;check for valid code
	jae	intr15_err		;error return
	shl	al,1			;make it word boundary
	mov	si,ax
	jmp	word ptr cs:[si + offset jmp_table]	;go execute the code

Intr15_err:
	mov	ah,86h
	mov	[bp+bp_ax+1],ah		;return error code
err_rtn2:
	stc				;set carry for error return
Normal_rtn:
	sti				;enable interrupts just in case
	call	restore_all
	pop	ax			;
	ret	2			;
intr_15	endp

page

; ToBu (20) 19.9.88 Ver 3.4
; 
;	Function code 0C0H:	Get Configuration Parameter Table Address
;	Input parameters:	None
;	Output parameters:
;		AH = 0
;		ES:BX - configuration parameter table address in TOP8K

func_c0:
	mov	word ptr [bp+bp_es],cs		;segment address
	mov	word ptr [bp+bp_bx],offset config_table	;offset address
	mov	byte ptr [bp+bp_ax+1],0		;AH=0
	clc
	jmp	Normal_rtn

;
;	Function code 83:	Wait_4_Event
;	Input parameters:	al = 0 set interval
;			al = 1 cancel interval
;	if al = 0:	es:bx	address of symaphore byte in caller's
;				memory space to indicate event
;			cx,dx	# of microsecs to wait before returning
;
wait_4_event	proc	near
	assume cs:eproma,ds:dataa

; ToBu (21) 19.9.88 Ver 3.4
	mov	byte ptr [bp+bp_ax+1],0	;AH = 0
	cmp	byte ptr [bp+bp_ax],0	;see if set interval
	je	wait_event_1		;set interval
	cmp	byte ptr [bp+bp_ax],1	;see if cancel
	je	wait_event_3		;cancel
	dec	byte ptr [bp+bp_ax]	;decrement AL
	jmp	err_rtn2		;invalid sub-function code

;AL = 0, set interval

wait_event_1:
	mov	byte ptr [bp+bp_ax],0	;AL = 0
	test	event_wait_flag,01	;is wait flag already set
	jz	wait_event_2		;no 	
f_err_rtn2:
	jmp	err_rtn2		;yes,error

wait_event_2:				;set event
	cli				;no interruption please
	in	al,int2+1		;read 8259
	and	al,0feh
	out	int2+1,al		;make sure interrupts are unmasked
	mov	count_high,cx		;
	mov	count_low,dx		;setup real time clock
	mov	event_flag_addr,bx	;and address of user variable
	mov	event_flag_seg,es	;
	mov	event_wait_flag,1	;set function active flag
	call	cmos_io_en		;enable PIE interrupt
	jmp	short wait_event_4	;and return		

;AL = 1, cancel

wait_event_3:
	mov	byte ptr [bp+bp_ax],0	;AL = 0
	cli				;no interruption please
	mov	event_wait_flag,0	;clear function active flag
	call	cmos_io_dis		;disable PIE interrupt
wait_event_4:
	sti				;enable interrupts again
	clc				;clear carry
	jmp	normal_rtn		;and return		
wait_4_event	endp			;

;
;	let us enable Periodic Interrupt bit (PIE) in the cmos-ram
;	register

cmos_io_en	proc	near
	mov	al,0bh			;read B status register
	call	read_cmos		;
	and	ah,07fh			;get rid of high bit
	or	ah,40h			;set PIE bit (bit 6)
	dec	al			;
	call	write_cmos		;
	ret
cmos_io_en	endp

;
;	let us disable Periodic Interrupt bit (PIE) in the cmos-ram
;	register

cmos_io_dis	proc	near
	mov	al,0bh			;read B status register
	call	read_cmos			;read data
	and	ah,10111111b		;reset PIE bit (bit 6)
	dec	al			;register B
	call	write_cmos		;write data
	ret
cmos_io_dis	endp


page

;
;	Function code 84h:	Joy Stick Support
;		
;	Input	DX = 0	Read current switches and return in Al bits 4-7
;		     1	Read the resistive inputs and return as follows:
;			AX = A(x) value
;			BX = A(y) value
;			CX = B(x) value
;			DX = B(y) value
;
;	Carry set if there is no joy stick support available
;

joy_stick	proc	near
	sti				;enable interrupts
	cmp	dl,1			;check for valid input
	ja	f_err_rtn2		;illegal sub function, error
	test	dl,1			;is it 1
	jnz	joy_stick_1		;go execute it
	mov	dx,201h			;get port address
	in	al,dx			;read switches
	mov	byte ptr [bp+bp_ax],al	;
	clc				;clear carry
	jmp	normal_rtn


;
;	execute sub-function code 1 i.e read resistive inputs
;

joy_stick_1:
	mov	bl,1			;check for A(x) value
	call	test_inputs
	mov	[bp+bp_ax],cx		;save return parameter
	mov	bl,2			;
	call	test_inputs		;check for A(y) value
	mov	[bp+bp_bx],cx		;save it
	mov	bl,4			;
	call	test_inputs		;check for B(x) value
	mov	[bp+bp_cx],cx		;save it
	mov	bl,8			;check for B(y) value
	call	test_inputs		;
	mov	[bp+bp_dx],cx		;save it
	sti				;enable interrupts
	clc				;clear carry
	jmp	normal_rtn

test_inputs	proc	near
	cli				;no interrupts while checking
	mov	dx,201h			;get game port
	call	get_timer		;
	out	dx,al			;start timer
	mov	cx,04ffh		;set count
	push	ax
test_a:
	in	al,dx
	test	al,bl			;end of pulse
	jz	test_b			;if yes,then leave loop
	loop	test_a			;otherwise loop till done
	jmp	short test_c

test_b:
	call	get_timer
	pop	cx			;put ax in cx
	cmp	cx,ax			;check previous value with new value
	jae	test_d			;
	push	ax			;save ax
	not	ax			;complement it
	add	cx,ax			;adjust cx
	pop	ax			;
	jmp	short test_f		;
test_d:
	sub	cx,ax			;
test_f:
	shr	cx,1
	shr	cx,1
	shr	cx,1
	shr	cx,1			;
	clc
	push	ax			;save ax
test_c:
	sti	
	push	cx
	mov	cx,04ffh
test_h:
	in	al,dx
	and	al,0fh			;
	loopnz	test_h
	pop	cx			;restore registers
	pop	ax			;
	ret				;

get_timer	proc	near
	mov	al,0
	out	43h,al			;set latch
	call	delay
	in	al,40h			;get low byte first
	xchg	al,ah			;save it
	call	delay
	in	al,40h			;get high byte
	xchg	al,ah			;adjust ax
	ret				;
get_timer	endp
test_inputs	endp
joy_stick	endp

page

;	Function code : 86h	Delay Routine
;			cx,dx contains the # of microseconds to wait
;			before returning to the caller. 
;			cx contains MSW and dx has the LSW
;

delay_rtn	proc	near
					
	test	event_wait_flag,01		;test for function active
	jnz	delay_rtn_2			;error return if set to 1

;	setup wait loop

	cli				;no interupts please
	in	al,0A1h			;make sure interrupts are unmasked
	and	al,0feh			;
	out	0A1h,al			;
	mov	event_wait_flag,01	;set wait in progress
	mov	count_low,dx		;setup #of tics value
	mov	count_high,cx		;
	mov	event_flag_seg,ds	;and the address of event_flag
	mov	event_flag_addr,offset event_wait_flag
	call	cmos_io_en		;
	sti				;let interrupts occur now;
;
;	wait here for bit 7 in the wait flag to go high
;
; ToBu (22) 19.9.88 Ver 3.4	use CX:DX as a loop counter 
;			to eleminate infinite loop

	mov	dx,[bp+bp_cx]		;get MSB
	mov	cx,[bp+bp_dx]		;get LSB
delay_rtn_1:
	test	event_wait_flag,80h	;has the event occurred?
	jnz	delay_rtn_3		;yes, return
	loop	delay_rtn_1		;no, then wait here
	or	dx,dx			;see if MSB comes to zero
	jz	delay_rtn_3		;all done
	dec	dx			;decrement MSB
	jmp	delay_rtn_1		;wait more...
delay_rtn_3:
	mov	event_wait_flag,0	;yes, reset function inactive
	jmp	Normal_rtn		;

delay_rtn_2:
	jmp	err_rtn2
delay_rtn	endp


shut_9A:	
		mov	al,reset_A20	;matt newell  reset line A20
		call	gate_A20	;matt newell
		jz	shut_9b		;matt newell
		mov	al,3		;matt newell  set error code
shut_9b:	mov	byte ptr [bp+bp_ax+1],al	;matt send error return code
		cmp	al,0		;matt   set Z flag
		sti			;matt newell
		jmp	normal_rtn	;matt	
		;jmp	shutdown_9a	;matt


;shut_9A:	jmp	shutdown_9a
page	


;
;	Int 15 Function 87h: Move Block
;	Transfers block of data from to or from memory (physical or 
;	virtual) above 1 meg of address space in a protected mode
;
;	Input parameters:	ah = 87h
;			 	es:si = address of GDT build by caller
;				cx = # of words to move ; max = 32k
;	Output parameters:	ah = 0 successful
;				   = 1 if RAM parity error
;				   = 2 if exception interrupt error	
;				   = 3 address line 20 is gated active	
;	All registers are preserved except AX
;	Note:	Since all interrupts are disabled during transfer it is
;		the responsibility of the caller to adjust RTC
;	
;	GDT table is 48 bytes long and contains 6 descriptors each 8 bytes
;	as follows:
;	1st descriptor = dummy
;	2nd descriptor = area to save calling routine's GDT
;	3rd descriptor = address of source GDT for block move
;	4th descriptor = address of destination GDT for block move
;	5th descriptor = area to save current code segment GDT
;	6th descriptor = area to save current stack segment

;	each 8 byte descriptor is defined as follow:
;	segment limit = 2 bytes
;	base : low word = 2 bytes
;	base : hi byte  = 1 byte
;	Access rights byte = 1 byte
;	reserved for Intel = 2 bytes

;	define some equates for descriptor table

dummy		equ	0
code_gdt		equ	dummy+8
src_gdt		equ	code_gdt+8	
dst_gdt		equ	src_gdt+8
bios_gdt		equ	dst_gdt+8
stk_gdt		equ	bios_gdt+8

seg_lim		equ	0
base_lo		equ	2
base_hi		equ	4
acc_rights		equ	5
reserved		equ	6	

page

	assume cs:eproma, ds:dataa

; ToBu (23) 19.9.88 Ver 3.4
blk_move	proc	near
	cli				;disable interrupts

	call	restore_all		;modification
	pop	ax			;W. Chessen
	pusha				;by
	push	es			;matt
	push	ds			;newell
;	mov	si,[bp+bp_si]		;matt newell restore si   rlm  2/26/85
 
	mov	al,set_A20		;go enable address bit 20
	call	gate_A20		;
	jnz	shut_9A			;error from 8042, do a shutdown

;
;	setup the GDT defination; first create a 24 bit address
;	out of es:si
;

blk_move_2:
	mov	bx,es
	mov	ah,bh			;save the high byte
;03/25/86, jwy
	shr	ah,4			;shift
	shl	bx,4			;
	mov	dx,si			;
	add	dx,bx			;get offset
	jnc	blk_move_1		;no overflow
	inc	ah			;inc ah due to overflow

;	now store this address into GDT 

blk_move_1:
	mov	word ptr es:[si+code_gdt+base_lo],dx	;save low word
	mov	byte ptr es:[si+code_gdt+base_hi],ah	;save high byte
	mov	word ptr es:[si+code_gdt+seg_lim],-1	;max len 
	mov	word ptr es:[si+code_gdt+reserved],0	;



;	save current code segment descriptor

	mov	word ptr es:[si+bios_gdt+base_lo],0  	;bios code segment is
	mov	byte ptr es:[si+bios_gdt+base_hi],0FH	; --- F0000 ---
	mov	byte ptr es:[si+bios_gdt+acc_rights],09Bh	;
	mov	word ptr es:[si+bios_gdt+seg_lim],-1	;
	mov 	word ptr es:[si+bios_gdt+reserved],0	;


;
;	save user stack area. First create a 24 bit address
;
	mov	bx,ss			;
	mov	ah,bh			;
;03/25/87, jwy
	shr	ah,4			;use only high nibble
	shl	bx,4			;multiply bx by 16
	mov	word ptr es:[si+stk_gdt+base_lo],bx
	mov	byte ptr es:[si+stk_gdt+base_hi],ah
	mov	word ptr es:[si+stk_gdt+seg_lim],-1
	mov	byte ptr es:[si+stk_gdt+acc_rights],93h	;access rights


;	
;	load global descriptor table register
;

;03/25/87, jwy
	lgdt	qword ptr es:[si].code_gdt


;	load the IDT
	
;	push	bp				; comented by matt newell
	mov	bp,offset IDT_vec_addr		;get rom based IDT table

;03/25/87, jwy
	lidt	qword ptr cs:[BP]


;	
;	Save caller's stack pointer and stack segment in low memory,
;	Remember bp is on top of the stack to be retrieved after shutdown
;
	call	set_data		;W. Chessen	
	mov	bx,sp			;get stack pointer
	mov	rom_address,bx		;
	mov	bx,ss			;save stack segment
	mov	rom_segment,bx		;


;
;	setup cmos shutdown byte to return here
;

	mov	al,80H + SHUTDOWN	; address of shutdown byte
	mov	ah,9			; shutdown code
	call	write_cmos		; write into CMOS


;	now switch to virtual mode by issuing a LMSW instruction

	mov	ax,1			;set virtual enable bit
	

; 5/28/85 jwy
;03/25/87, jwy
	lmsw	ax

	db	0eah			;make a FAR jump into this 
	dw	(offset Virt_jump)	;segment at this address
	dw	bios_gdt		;segment entry into GDT

;	setup user stack in case of exception errors

Virt_jump:
	mov	ax,stk_gdt		;
	mov	ss,ax			;setup stack segment entry

	
;	now is the time to do a block move... FINALLY...

	mov	ax,dst_gdt		;get destination segment
	mov	es,ax			;
	mov	ax,src_gdt		;get source segment
	mov	ds,ax			;
	xor	si,si			;set si to 0
	mov	di,si			;
	cld				;set direction
	rep	movsw			;....MOVE... cx contains the word count

;	ok now is the time to do shutdown

blk_move_4:
	jmp	shut_down		;go do a shutdown


;
;	return here from shutdown
;
shutdown_9:
	mov	al,0			;enable NMI interrupts
	out	70h,al			;

;	restore user stacks

	call	set_data		;setup ds register
	mov	ax,rom_address		;get stack pointer first
	mov	sp,ax			;
	mov	ax,rom_segment		;get stack segment
	mov	ss,ax			;
;	pop	bp			;commented by matt newell
	pop	ds			;modified
	pop	es			;by
	popa				;matt
shutdown_9a:				;newell

	mov	al,reset_A20		;reset address line A20
	call	gate_A20		;
	jz	shutdown_9b		;no error from 8042
	mov	al,3			;set error code

shutdown_9b:
;	mov	byte ptr [bp+bp_ax],al	;send error return code

; 5/28/85 jwy
;	mov	byte ptr [bp+bp_ax+1],al	;commented by matt newell
	mov	ah,al
	cmp	al,0			;set z flag
	sti				;
	iret				;matt newell
;	jmp	normal_rtn		;commented by matt newell

blk_move	endp		


page


;	INT15 Function code 88: Get Virtual memory size
;
;	This routine gets memory size beyond 1mbyte that is avaialble in
;	the system. The size is stored in CMOS at location 30,31 during
;	power up diagnostics
;		
;	output parameter  AX =  # of contiguous 1k blocks of memory
;				starting at address 100000

; ToBu (24) 19.9.88 Ver 3.4
Get_ext_mem	proc	near
	sti				;
	push	bx			;save bx register
	mov	al,30h			;
	call	read_cmos		;read low byte
	mov	bl,ah			;save it in bl
	call	read_cmos		;read high byte now
	mov	bh,ah			;put in bh
	mov	[bp+bp_ax],bx		;save it in ax
	pop	bx			;restore bx
;	clc				;clear carry
;	jmp	normal_rtn		;and return
	call	restore_all		;3/13/87 jwy
	pop	ax			;fixed the problem with netware 286
	iret				;restore all the original flags
Get_ext_mem	endp			;including int flag


;
;	This routine controls the address bit A20 via 8042 slave processor
;
;	On input AL = 0DDh	turn off the address bit
;		 AL = 0DFh	turn on the address bit
;	
;	On Output AL = 0	operation successful, Z flag = 0
;		  AL = 2	error from 8042, Z flag is set
;

gate_A20	proc	near
	cli				;
	push	bx			;
	mov	bl,al			; save the command in bl
	call	wait_4_8042		; see if input buffer is empty
	jnz	gate_A20_rtn		; input buffer full
	mov	al,0D1h			;send command to 8042
	out	64h,al	
	call	wait_4_8042		;see if he accepted the command
	jnz	gate_A20_rtn		;no then return
	mov	al,bl			;yes,now send the data
	out	60h,al			;
	call	wait_4_8042

gate_A20_rtn:
	and	al,2			;return 2 or 0 in al and set Z flag
	pop	bx			;restore bx
	ret				;and return

gate_A20	endp


wait_4_8042	proc	near
	push	cx
	mov	cx,0			;
wait_loop:
	in	al,64h			;read status from 8042
	test	al,2			;is the input buffer still full
	loopnz	wait_loop		;yes,try again
	pop	cx			;still busy, error
	ret				;return with error
wait_4_8042	endp

page

;
;	INT 15 Function 89h:	Put user in virtual mode
;
;	On entry es:si should point to the GDT table build by the calling
;	function. This table must be 64 bytes long and must have 8 discriptors
;	eaxh 8 bytes long. The discriptors are defined as follows:
;
;	# 1	offset 0   = 	dummy
;	# 2	offset 8   =	GDT discriptor
;	# 3	offset 16  =	IDT    "
;	# 4	offset 24  =	DS     "
;	# 5	offset 32  =    ES     "
;	# 6	offset 40  =    SS     "
;	# 7	offset 48  =  	CS     " to return to in virtual mode
;	# 8	offset 56  =    used by BIOS to complete this function

;	define some equates

virt_gdt	equ	dummy+8
virt_idt	equ	virt_gdt+8
virt_ds		equ	virt_idt+8
virt_es		equ	virt_ds+8
virt_ss		equ	virt_es+8
virt_cs		equ	virt_ss+8
virt_bios_cs	equ	virt_cs+8


;
;	bl = offset into IDT for 2nd 8259
;	bh = offset into IDT for 1st 8259
;
;	Exit parameters:	ah = 0	successful
;				ah = ff error, carry will be set
;

	assume	cs:eproma,ds:dataa

; ToBu (25) 19.9.88 Ver 3.4
Set_Vir_Mod	proc	far	 	;define a far proc

	cli				;no interrupts
	mov	si,[bp+bp_si]		;restore si  rlm 2/26/85
	mov	al,set_A20		;gate bit 20 first
	call	gate_a20
	jz	Set_2			;no error  fixed rlm   2/26/85
	mov	ah,0ffh			;
	mov	[bp+bp_ax],ax		;save ax
	jmp	err_rtn2		;

;	initialize 8259 interrupt controllers # 1 & #2

ICW1	equ	11h

Set_2:
	mov	al,ICW1			;start init seq
	out	int1,al			;initialize 8259 # 1
	call	delay			;
	out	int2,al			;
	call	delay			;
	mov	al,bh			;
	out	int1+1,al		; # 1, send ICW2
	call	delay			;
	mov	al,bl			;
	out	int2+1,al		;
	mov	al,4			;ICW3 master level 2 for 1st 8259
	call	delay			;
	out	int1+1,al		
	jmp	short $+2		;
	shr	al,1			;
	out	int2+1,al		;
	call	delay			;
	shr	al,1			;
	out	int1+1,al
	call	delay
	out	int2+1,al
	mov	al,0ffh			;mask off all interrupts
	jmp	short $+2			;
	out	int1+1,al		;1st 8259
	call	delay			;
	out	int2+1,al	


;	setup GDT for current bios code

	mov	word ptr es:[si+virt_bios_cs+base_lo],0		;bios code segment is
	mov	byte ptr es:[si+virt_bios_cs+base_hi],0fh	; --- F000 ---
	mov	byte ptr es:[si+virt_bios_cs+acc_rights],09Bh	;
	mov	word ptr es:[si+virt_bios_cs+seg_lim],-1	;
	mov 	word ptr es:[si+virt_bios_cs+reserved],0	;


;	set GDTR and INTR in the cpu

Set_1:

;03/25/87, jwy
;	db	26h,0fh,01,5ch,10h	;'LIDT  [si].virt_idt'
	LIDT	qword ptr es:[si].virt_idt

;03/25/87, jwy
;	db	26h,0fh,01,54h,8	; 'LGDT [si].virt_gdt
	LGDT	qword ptr es:[si].virt_gdt

;
;	restore users registers before going into virtual mode
;
	call	restore_all
	pop	ax


;	Now enable virtual mode 

	mov	ax,1			;set PE bit
	
;03/25/87, jwy
;	db	0fh,1,0f0h		;
	lmsw	ax
	db	0eah			;make a far jump into this segment
	dw	offset Set_VMode_2	;
	dw	virt_bios_cs			;

;	now setup different segment registers

Set_Vmode_2:
	mov	bx,virt_es		;get entry for es reg. in GDT
	mov	es,bx			;
	mov	bx,virt_ds		;get entry for ds register in GDT
	mov	ds,bx			;
	mov	bx,virt_ss		;stack segment
	mov	ss,bx			;

;	setup a return address on the stack and return to the user in
;	virtual mode

	pop	bx			;get return address
	add	sp,4			;get rid of cs and flags
;03/25/87, jwy
;	db	68h
;	dw	virt_cs			;put cs register on stack
	push	virt_cs
	push	bx			;and return IP
	ret				;return to user in virtual mode

Set_Vir_Mod	endp


page

;	device busy...  AH = 90h

dev_busy	proc	near
	clc				;clear carry, no error
	jmp	normal_rtn
dev_busy	endp




;	Interrupt complete... AH = 91

Int_Comp	proc	near
	call	restore_all
	pop	ax
	clc	
	iret				;return
Int_Comp	endp			

page

jmp_table	label	word		; NI means not implemented
	dw	Normal_rtn	;80	device open  (ni)
	dw	Normal_rtn	;81	device close (ni)
	dw	Normal_rtn	;82	program termination (ni)
	dw	wait_4_event	;83	event wait   
	dw	Joy_stick		;84	joystick support    (ni)
	dw	Normal_rtn	;85	systen req key pressed (ni)
	dw	delay_rtn		;86	delay routine
	dw	blk_move		;87	move block
	dw	Get_ext_mem	;88	determine memory size
	dw	Set_Vir_mod	;89	processor to virtual mode
	dw	Intr15_err	;8A	error return
	dw	Intr15_err	;8B	   "
	dw	Intr15_err	;8C	   "
	dw	Intr15_err	;8D	   "
	dw	Intr15_err	;8E	   "
	dw	Intr15_err	;8F	   "
	dw	dev_busy		;90	device busy
	dw	int_comp		;91 	interrupt flag complete.
max_cmd	equ	($ - jmp_table)/2	;maximum value of AH register


;
;	ROM resident Interrupt Descriptor table
;

IDT_length	equ	100h		;32 entries, each entry 8 bytes
Trap_gate	equ	87h		;access rights


IDT_vec_addr:				;define a descriptor
		dw	IDT_length	;length
		dw	Exp_vec_tbl	;execption vector table
		db	0fh		;code segment, high byte
		db	0		;reserved

;
;	Exception Error Interrupt handler
;

Exception_intr:
	
	jmp	shut_down		;go do a shutdown


Exp_vec_tbl:
					;Exception 0
	dw	Exception_intr		;Interrupt service routine
	dw	bios_gdt		;ISR segment
	db	0			;
	db	Trap_gate		;access rights
	dw	0			;reserved by INTEL

					;Exception 1
	dw	Exception_intr		;Interrupt service routine
	dw	bios_gdt		;ISR segment
	db	0			;
	db	Trap_gate		;access rights
	dw	0			;reserved by INTEL


					;Exception 2
	dw	Exception_intr		;Interrupt service routine
	dw	bios_gdt		;ISR segment
	db	0			;
	db	Trap_gate		;access rights
	dw	0			;reserved by INTEL


					;Exception 3
	dw	Exception_intr		;Interrupt service routine
	dw	bios_gdt		;ISR segment
	db	0			;
	db	Trap_gate		;access rights
	dw	0			;reserved by INTEL


					;Exception 4
	dw	Exception_intr		;Interrupt service routine
	dw	bios_gdt		;ISR segment
	db	0			;
	db	Trap_gate		;access rights
	dw	0			;reserved by INTEL


					;Exception 5
	dw	Exception_intr		;Interrupt service routine
	dw	bios_gdt		;ISR segment
	db	0			;
	db	Trap_gate		;access rights
	dw	0			;reserved by INTEL


					;Exception 6
	dw	Exception_intr		;Interrupt service routine
	dw	bios_gdt		;ISR segment
	db	0			;
	db	Trap_gate		;access rights
	dw	0			;reserved by INTEL


					;Exception 7
	dw	Exception_intr		;Interrupt service routine
	dw	bios_gdt		;ISR segment
	db	0			;
	db	Trap_gate		;access rights
	dw	0			;reserved by INTEL


					;Exception 8
	dw	Exception_intr		;Interrupt service routine
	dw	bios_gdt		;ISR segment
	db	0			;
	db	Trap_gate		;access rights
	dw	0			;reserved by INTEL


					;Exception 9
	dw	Exception_intr		;Interrupt service routine
	dw	bios_gdt		;ISR segment
	db	0			;
	db	Trap_gate		;access rights
	dw	0			;reserved by INTEL


					;Exception 10
	dw	Exception_intr		;Interrupt service routine
	dw	bios_gdt		;ISR segment
	db	0			;
	db	Trap_gate		;access rights
	dw	0			;reserved by INTEL


					;Exception 11
	dw	Exception_intr		;Interrupt service routine
	dw	bios_gdt		;ISR segment
	db	0			;
	db	Trap_gate		;access rights
	dw	0			;reserved by INTEL


					;Exception 12
	dw	Exception_intr		;Interrupt service routine
	dw	bios_gdt		;ISR segment
	db	0			;
	db	Trap_gate		;access rights
	dw	0			;reserved by INTEL


					;Exception 13
	dw	Exception_intr		;Interrupt service routine
	dw	bios_gdt		;ISR segment
	db	0			;
	db	Trap_gate		;access rights
	dw	0			;reserved by INTEL


					;Exception 14
	dw	Exception_intr		;Interrupt service routine
	dw	bios_gdt		;ISR segment
	db	0			;
	db	Trap_gate		;access rights
	dw	0			;reserved by INTEL


					;Exception 15
	dw	Exception_intr		;Interrupt service routine
	dw	bios_gdt		;ISR segment
	db	0			;
	db	Trap_gate		;access rights
	dw	0			;reserved by INTEL


					;Exception 16
	dw	Exception_intr		;Interrupt service routine
	dw	bios_gdt		;ISR segment
	db	0			;
	db	Trap_gate		;access rights
	dw	0			;reserved by INTEL

					;Exception 17
	dw	Exception_intr		;Interrupt service routine
	dw	bios_gdt		;ISR segment
	db	0			;
	db	Trap_gate		;access rights
	dw	0			;reserved by INTEL


					;Exception 18
	dw	Exception_intr		;Interrupt service routine
	dw	bios_gdt		;ISR segment
	db	0			;
	db	Trap_gate		;access rights
	dw	0			;reserved by INTEL


					;Exception 19
	dw	Exception_intr		;Interrupt service routine
	dw	bios_gdt		;ISR segment
	db	0			;
	db	Trap_gate		;access rights
	dw	0			;reserved by INTEL


					;Exception 20
	dw	Exception_intr		;Interrupt service routine
	dw	bios_gdt		;ISR segment
	db	0			;
	db	Trap_gate		;access rights
	dw	0			;reserved by INTEL


					;Exception 21
	dw	Exception_intr		;Interrupt service routine
	dw	bios_gdt		;ISR segment
	db	0			;
	db	Trap_gate		;access rights
	dw	0			;reserved by INTEL


					;Exception 22
	dw	Exception_intr		;Interrupt service routine
	dw	bios_gdt		;ISR segment
	db	0			;
	db	Trap_gate		;access rights
	dw	0			;reserved by INTEL


					;Exception 23
	dw	Exception_intr		;Interrupt service routine
	dw	bios_gdt		;ISR segment
	db	0			;
	db	Trap_gate		;access rights
	dw	0			;reserved by INTEL


					;Exception 24
	dw	Exception_intr		;Interrupt service routine
	dw	bios_gdt		;ISR segment
	db	0			;
	db	Trap_gate		;access rights
	dw	0			;reserved by INTEL


					;Exception 25
	dw	Exception_intr		;Interrupt service routine
	dw	bios_gdt		;ISR segment
	db	0			;
	db	Trap_gate		;access rights
	dw	0			;reserved by INTEL


					;Exception 26
	dw	Exception_intr		;Interrupt service routine
	dw	bios_gdt		;ISR segment
	db	0			;
	db	Trap_gate		;access rights
	dw	0			;reserved by INTEL


					;Exception 27
	dw	Exception_intr		;Interrupt service routine
	dw	bios_gdt		;ISR segment
	db	0			;
	db	Trap_gate		;access rights
	dw	0			;reserved by INTEL


					;Exception 28
	dw	Exception_intr		;Interrupt service routine
	dw	bios_gdt		;ISR segment
	db	0			;
	db	Trap_gate		;access rights
	dw	0			;reserved by INTEL


					;Exception 29
	dw	Exception_intr		;Interrupt service routine
	dw	bios_gdt		;ISR segment
	db	0			;
	db	Trap_gate		;access rights
	dw	0			;reserved by INTEL


					;Exception 30
	dw	Exception_intr		;Interrupt service routine
	dw	bios_gdt		;ISR segment
	db	0			;
	db	Trap_gate		;access rights
	dw	0			;reserved by INTEL


					;Exception 31
	dw	Exception_intr		;Interrupt service routine
	dw	bios_gdt		;ISR segment
	db	0			;
	db	Trap_gate		;access rights
	dw	0			;reserved by INTEL



page
;
;*********************Please Note: ************************************
;	The size of this module must be 41Dh
;
empty_space	db	2 Dup (0)

end_of_int15	label	byte

	if	(end_of_int15 - start_of_int15)

;	Error
	endif

eproma	ends

	end

