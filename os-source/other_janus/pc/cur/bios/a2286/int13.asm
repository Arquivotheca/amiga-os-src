include	title.inc
subttl	Winchester Disk Driver  (INT 13H)

.xlist
include external.inc
include equates.inc

public	wd_drv
public	wd_intr
public	wd_setup

;extrn	wd_config_tbl:near

extrn	fixed_wait:near			;ROUTINES.ASM
extrn	display:near
extrn	hf_num:byte
extrn	ctrl_byte:byte
extrn	port_off:byte
extrn	hf_st_reg:byte
extrn	hf_err_reg:byte
extrn	hf_int_flg:byte
extrn	disk_stat_1:byte
extrn	hf_cntrl:byte
extrn	wd_cmd_file:byte
extrn	wd0_parm_vec:dword
extrn	wd1_parm_vec:dword
extrn	wd_config_tbl:near
extrn	beep:near
extrn	special:byte
EXTRN	RESET_KEY:WORD
.list

;***************************************************************************
;*		INT 13H		Hard Disk Driver	CBM                *
;***************************************************************************
;
;
;		START OF MAIN CODE .......

zero	segment at 0			;start of absolute segment
zero	ends

dataa	segment	at 40h			;start of data segment
dataa	ends

eproma	segment	byte public
	assume	cs:eproma, ds:dataa

public aa_int13
aa_int13 label byte

;	Perform initial Power-on diagnostics on the hard disk. This routine
;	is called just once at boot-up time.
;	input AH = fixed disk type byte from CMOS RAM

wd_setup	proc	near		
	push	ax			;save the fixed disk configuration
	call	Init_Int2		;Initialize second 8259
	call	Init_Int1		;let Interrupts pass through the first
					;8259
	pop	ax			;restore the fixed disk configuration

; check how many disk drives do we have

	mov	bx,0
	mov	port_off,bh		;	
	push	ds			;save DS reg
	mov	ds,bx			;setup DS register to 0
	assume	ds:zero

	mov	bl,ah			;save it
	mov	ch,0			;# of fixed disk
	shr	ah,4			;drive C type to low nibble
	mov	al,19h+80h		;extended drive C type
	mov	si,offset wd0_parm_vec	;interrupt vector addr. of INT 41
	call	get_tbl_addr		;get fixed disk parameter addr.
	jc	wd_setup1		;invalid drive type

	mov	ah,bl			;restore drive D type
	and	ah,0fh			;mask off drive C type
	mov	al,1ah+80h		;extended drive D type addr. in CMOS
	mov	si,offset wd1_parm_vec	;interrupt vector addr. of INT 46
	call	get_tbl_addr		;get fixed disk parameter addr.
wd_setup1:
	pop	ds			;restore DS reg
	assume	ds:dataa

	mov	hf_num,ch		;save # of available hard disks
	or	ch,ch			;no fixed disk?
	jz	wd_setup_rtn		;yes, return to POD

;	CMP	RESET_KEY,1234H
;	MOV	WORD PTR USER_REBOOT,0
;	JNE	DO_DIAG

	mov	cx,2
diag:	push	cx
	CALL	DRV_RESET
	jc	more
	mov	dx,80h			;check the controller
	mov	ah,14h			;perform controller diagnostics
	int	13h			;DISK IO
more:	pop	cx
	jnc	reset_drives		;no error ,go reset drives
	loop	diag	  		;error, do it again (v2.02 W. Chessen)

	mov	al,cmos_status_1+80h	;go write cmos status byte
	call	read_cmos
	or	ah,hd_fail		;set hard disk fail bit
	dec	al			;cmos_status_1
	call	write_cmos
	mov	si,offset wd_err_msg2	;error go tell the user
					;'1782-Disk controller failure'
	call	display			;go print a messages
;	mov	ah,0
	mov	ah,1			;set ZF to NZ (v2.02 W. Chessen)
	or	ah,ah			;ZF=1
	jmp	short wd_setup_rtn	;yes,return

reset_drives:
	mov	dh,hf_num		;# of drives to reset
reset_1:
	push	dx			;save # of disk & drive code
	call	wd_reset		;set drive parameter & recalibrate
	pop	dx			;restore # of disk & drive code
	jnz	wd_setup_rtn		;error
	inc	dl			;do next drive
	dec	dh			;all drive done?
	jnz	reset_1			;no do more
wd_setup_rtn:
	ret				;and return

;Soft reset to all drives (v2.02 W. Chessen)

DRV_RESET	PROC	NEAR
	MOV	DX,3F6H	   		;Digital output register
	MOV	AL,4			;enable intrrupt, reset drives
	OUT	DX,AL
	MOV	CX,750			;wait about 500 us (12MHz system)
	LOOP	$
	SUB	AL,4			;clear reset 
	OUT	DX,AL
	call	chk_4_ready		;wait for controller to get ready
	jnz	reset_error
	mov	bl,2
	xor	cx,cx
	mov	dx,wd_stat_port
w1:	in	al,dx
	and	al,50h
	cmp	al,50h
	je	chk_rst_stat
	loop	w1
	dec	bl
	jnz	w1
	jmp	short reset_error
chk_rst_stat:
	mov	dx,01F1h		;get reset status
	in	al,dx
	cmp	al,1			;
	jz	reset_done
reset_error:
	stc
reset_done:
	RET
DRV_RESET	ENDP

;get fixed disk parameter table
;input	AH - disk type in low nibble (0-15)
;	AL - extended drive type addr. in CMOS
;	DS:SI - interrupt vector addr. of parameter table
;output	if CY = 0, fixed disk type is valid
;	if CY = 1, invalid fixed disk type

get_tbl_addr	proc	near
	cmp	ah,0			;type 0?
	je	gta_err			;drive not present
	cmp	ah,0fh			;type 15?
	jne	gta_1			;no, use this type byte
	call	read_cmos		;get extended type
	cmp	ah,0			;type 0?
	je	gta_err			;invalid type
	cmp	ah,48			;type 48 or above?
	jae	gta_err			;invalid type
gta_1:
	dec	ah			;table start from type 1
	mov	al,0
	xchg	al,ah
	shl	ax,4			;16 bytes each
	add	ax,offset wd_config_tbl	;add base table addr.
	mov	word ptr [si],ax	;offset of int vector
	mov	word ptr [si+2],cs	;segment of int vector
	inc	ch			;increment # of fixed disk
	clc				;no error
	ret
gta_err:
	stc				;invalid drive type
	ret
get_tbl_addr	endp

; reset drives, set parameters and recaliberate

wd_reset	proc	near
	push	dx			;save drive code in DL
	mov	ah,8			;get drive parameter
	int	13h			;DISK IO
	cmp	dh,0			;# of heads = 0?
	pop	dx			;restore drive code in DL
	je	wd_err3			;error if # of heads = 0

	push	cx			;save # of cyls
	and	cl,3fh			;remove cyls in bits 7,6
	cmp	cl,0			;# of sectors = 0?
	pop	cx			;restore # of cyls
	je	wd_err3			;error if # of sectors = 0

	and	cl,0c0h			;remove # of sectors in bits 5-0
	cmp	cx,0			;# of cyls = 0?
	jne	wd_reset4		;no
wd_err3:
	mov	si,offset wd_err_msg3	;print error messages
					;'1790-Disk 0 error'
	cmp	dl,80h			;is it first drive?
	je	wd_err4			;yes,go print error messages

	mov	si,offset wd_err_msg4	;report an error on drive 1
					;'1791-Disk 1 error'
wd_err4:
	call	display			;display messages
	mov	ah,0ffh
	or	ah,ah			;ZF=0
	jmp	short wd_reset_ret

wd_reset4:
	mov	bl,10			;try 10 times
wd_reset1:
	mov	ah,9			;set drive parameters
	int	13h			;DISK IO
	jnc	wd_reset2		;no errors

	call	wait_5_secs		;wait 5 secs
	dec	bl			;decrement loop counter
	jnz	wd_reset1		;try again
	jmp	short wd_err1		;error

;	recalibrate drive

wd_reset2:
	mov	bl,5			;retry 5 times
wd_reset3:
	mov	ah,11h			;recalibrate
	int 	13h			;DISK IO
	jnc	wd_reset5		;if no error

	call	wait_5_secs		;wait for 5 secs
	dec	bl			;decrement retry count
	jnz	wd_reset3		;try again

; go print error messages and return

wd_err1:
	mov	si,offset wd_err_msg1	;assume disk 1 failure
					;'1781-Disk 1 failure'
	cmp	dl,81h			;is it second drive?
	je	wd_err2			;yes

	mov	si,offset wd_err_msg0	;'1780-Disk 0 failure'
	mov	al,cmos_status_1+80h	;go set disk fail error
	call	read_cmos
	or	ah,hd_fail		;set disk fail bit
	dec	al			;cmos_status_1
	call	write_cmos		;go write it
wd_err2:
	call	display			;display messages
wd_reset5:
	mov	ah,0
	or	ah,ah			;ZF=1
wd_reset_ret:
	ret				;rerurn to caller
wd_reset	endp			;end wd_reset


; delay routines for 5 seconds

wait_5_secs	proc	near
	push	cx			;save regs
	push	bx
	mov	bl,80h			;give a beep   2/26/85 rlm
	call	beep
	mov	bl,5			;4943 ms = 988.65886 * 5
	mov	cx,0			;988.65886 ms = 15.085737 * 65536
wait_5_1:
	call	fixed_wait		;15.085737 us
	dec	bl			;decrement loop count
	jnz	wait_5_1		;wait more...
	pop	bx			;restore regs
	pop	cx
	ret
wait_5_secs	endp

wd_setup	endp			;end of wd_setup

wd_err_msg0	db 	'1780-Disk 0 failure',cr,lf,-1
wd_err_msg1 	db 	"1781-Disk 1 failure", CR, LF, -1
wd_err_msg2 	db 	"1782-Disk controller failure", CR, LF, -1
wd_err_msg3 	db 	"1790-Disk 0 error", CR, LF, -1
wd_err_msg4 	db 	"1791-Disk 1 error", CR, LF, -1

;	define command branch table for hard disk

wd_func_tbl	label	word
	dw	wd_reset_cmd		;ah = 0	 reset the disk system
	dw	wd_get_status		;ah = 1  get current status
	dw	wd_read_sec		;ah = 2  read a sector into memory
	dw	wd_write_sec		;ah = 3	 write a sector to the disk
	dw	wd_verify_cmd		;ah = 4  verify disk data
	dw	wd_fmt_track		;ah = 5  format hard disk track
	dw	wd_unused_cmd		;ah = 6  unused command code
	dw	wd_unused_cmd		;ah = 7  unused command code
	dw	wd_get_parms		;ah = 8  get parameters
	dw	wd_init_drv		;ah = 9  initialize drives
	dw	wd_read_long		;ah = A  read long
	dw	wd_write_long		;ah = B  write long
	dw	wd_seek_cmd		;ah = C  seek to desired track
	dw	wd_reset_cmd		;ah = D	 same as ah = 0
	dw	wd_unused_cmd		;ah = E  unused command code
	dw	wd_unused_cmd		;ah = F  unused command code
	dw	wd_rdy_test		;ah = 10 test for drive ready
	dw	wd_recal_drv		;ah = 11 recalibrate drive
	dw	wd_unused_cmd		;ah = 12 unused command code
	dw	wd_unused_cmd		;ah = 13 unused command code
	dw	wd_cntrl_diag		;ah = 14 controller internal diag
	dw	wd_dasd_type		;ah = 15 read DASD type
max_cmd	equ	($ - wd_func_tbl)/2	;maximum # of valid command



;	define command table

wd_cmd_tbl	label	byte

	db	0			;ah = 0 Reset 
	db	0			;ah = 1 Read status
	db	wd_read			;ah = 2 read
	db	wd_write		;ah = 3 write
	db	wd_verify		;ah = 4 verify
	db	wd_fmt_trk		;ah = 5 format track
	db	0			;ah = 6
	db	0			;ah = 7
	db	0			;ah = 8
	db	wd_set_parms		;ah = 9 set drive parameters
	db	wd_read+ecc_mode	;ah = 0A read long
	db	wd_write+ecc_mode	;ah = 0B write long
	db	wd_seek			;ah = 0C seek
	db	0			;ah = 0D alternate disk reset
	db	0			;ah = 0E
	db	0			;ah = 0F
	db	0			;ah = 10 test drive ready
	db	wd_recal		;ah = 11 drive recalibrate
	db	0			;ah = 12
	db	0			;ah = 13
	db	wd_diag			;ah = 14 controller diagnostics
	db	0			;ah = 15


err_tbl	db	0ah			;bad sector error
	db	10h			;ECC error
	db	0BBh			;don't know error
	db	04			;bad record error
	db	0bbh			;don't know error
	db	1			;bad command error
	db	40h			;seek error
	db	02			;bad address mark error
	db	0e0h			;no error

;	************************************************************
;	*	Start of Main Hard Disk Driver code.               *
;	*	This is the entry point  for INT 13H               *
;	************************************************************

wd_drv	proc	far
	assume ds:nothing, es:nothing
	cmp	dl,1			;check for floppy or hard disk
	ja	wd_drv1			;hard disk
	int	40h			;no diskette driver
wd_quick_rtn:
	ret	2			;quick return
wd_drv1:
	assume	ds:dataa
	sti				;enable interrupts again
	cmp	ah,0			;do we need to reset the disk
	jne	wd_drv2			;no
	push	dx			;save drive code
	and	dl,1			;make sure drive code is correct
	int	40h			;
	pop	dx			;get drive code back
	mov	ah,0			;ignore status from int40
	cmp	dl,81h			;
	ja	wd_quick_rtn		;illegal drive code for reset


;	check for drive code

wd_drv2:
	push	ax			
	mov	al,cmos_sp_flag+80h	;get apeed flag from cmos
	call	read_cmos
	test	ah,80h			;see if low speed hdisk IO is needed
	jnz	hd_continue		;skip if not
	test	ah,03h			;see if we are at low speed
	jz	hd_continue		;skip if we are
	push	dx			;change to low speed
	mov	dx,180h
	xor	al,al
	out	dx,al
	pop	dx
hd_continue:
	pop	ax
	push	ax			;save registers
	call	save_all		;
	call	branch			;go execute the command
	call	set_data		;set data segment

	push	ax
	mov	al,cmos_sp_flag+80h	;get speed flag
	call	read_cmos
	and	ah,03h			;check system speed (before hd IO)
	jz	hd_done			;skip if it was low speed
	mov	dx,180h			;otherwise switch back to the
	mov	al,ah			;  original speed
	out	dx,al
	;
hd_done:
	pop	ax
	cmp	disk_stat_1,0ffh	;special error code from DASD_type Fn
	jne	wd_drv3			;no
	mov	disk_stat_1,0		;yes,then change it to 0
	stc				;set carry
	jmp	short wd_drv4		;
wd_drv3:
	cmp	ax,0333h		;special sucess code from function 15h.
	jne	not_15h			;  fix for OS/2 v1.2 FDISK problem (v2.03 W. Chessen)
	xor	al,al
	mov	[bp+bp_ax],ax
	jmp	short wd_drv4
not_15h:
	mov	ah,disk_stat_1		;get status of the operation
	xor	al,al			;###################
	mov	[bp+bp_ax],ax		;save it to be returned to user
	cmp	ah,1			;set carry or clear carry
	cmc				;
wd_drv4:
	;
	;
	call	restore_all		;restore register
	pop	ax			;and ax
	ret	2			;return without flags
wd_drv	endp


;
;	this routine will execute the command and return to user
;

branch	proc	near
	;
	;
;	push	ax			;5/23/88 - jmf
;	push	dx			;disable mouse if enabled
;	push	ds			;save current DS (??????)			
;	mov	ax,data
;	mov	ds,ax			;set DS to SEGMENT 'DATA'
;	mov	dx,230h
;	mov	al,special
;	and	al,0efh
;	out	dx,al			;disable mouse
;	jmp	$+2
;	in	al,0a1h
;	jmp	$+2
;	or	al,2
;	out	0a1h,al			;mask IRQ 9
;	pop	ds
;	pop	dx
;	pop	ax
	;
	;
	cmp	ah,1			;check for command code of 0 or 1
	jbe	wd_drv5			;if equal then do not check DL
	and	dl,1			;get the drive code  0 or 1
	mov	al,hf_num		;how many drives are connected
	cmp	dl,al			;
	jae	wd_unused_cmd		;bad command,return


;	check for valid command

wd_drv5:
	mov	ax,[bp+bp_ax]		;get ax back
	cmp	ah,max_cmd		;valid command
	jae	wd_unused_cmd		;
	call	set_cmd_parms		;go setup parameters 
	push	ax			;# must be from 0 - 15
	xchg	al,ah			;
	mov	ah,0			;
	shl	ax,1			;setup ax for jump
	mov	si,ax			;
	pop	ax			;restore ax
	jmp	word ptr cs:[si+offset wd_func_tbl]


wd_unused_cmd:
	mov	disk_stat_1,1		;
	mov	al,0			;
	ret				;

branch	endp				

;
;	Reset the disk system   AH = 0
;

wd_reset_cmd	proc	near
	cli				;clear interrupts
	call	Init_Int2		;go initialize second 8259
	sti				;and turn on the interrupts
	push	bx
	push	cx
	call	drv_reset
	pop	cx
	pop	bx
	jc	reset_cmd_err
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;	mov	al,4			;issue a reset to disk controller
;	mov	dx,03F6h		;
;	out	dx,al			;
;	push	bx
;	mov	bl,5			;give some time to controller to reset
;reset_cmd1:
;	call	delay			;
;	dec	bl
;	jnz	reset_cmd1		;
;	pop	bx			;
;	mov	al,ctrl_byte		;
;	and	al,0fh			;turn off reset
;	out	dx,al			;
;	push	bx
;	call	chk_4_ready		;wait for controller to be ready
;	pop	bx			;
;	jnz	reset_cmd_err		;if error jump
;	push	cx			;check for ready line (v2.02 W. Chessen)
;	push	bx
;	mov	bl,2
;	xor	cx,cx
;	mov	dx,wd_stat_port
;r1:	in	al,dx
;	and	al,50h
;	cmp	al,50h
;	je	rst_ok
;	loop	r1
;	dec	bl
;	jnz	r1
;	pop	bx
;	pop	cx
;	jmp	short reset_cmd_err
;rst_ok:
;	pop	bx
;	pop	cx
;;;;	jz	reset_cmd_err
;	mov	dx,01F1h		;get reset status
;	in	al,dx
;	cmp	al,1			;
;	jnz	reset_cmd_err		;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;	reset and recalibrate drives

	push	bx
	mov	bl,hf_num		;get # of physical drives connected
	mov	dl,0ffh			;
reset_cmd2:
	inc	dl

;save the # of fixed disk & drive # - V2.56

	push	dx			;save drive #
	push	bx			;save # of fixed disk 
	call	issue_reset		;
	pop	bx			;restore # of fixed disk
	pop	dx			;restore drive #
	dec	bl			;
	jnz	reset_cmd2		;do second drive
	pop	bx			;
	mov	disk_stat_1,0		;
	jmp	short reset_cmd4	;03/11/85
reset_cmd_err:
	mov	disk_stat_1,5		;controller bad
reset_cmd4:
	ret

wd_reset_cmd	endp


issue_reset	proc	near

	cmp	dl,0			;
	jnz	issue_reset1		;
;clear drive code for unit 0 - V2.56
	and	wd_cmd_file+5,0efh	;clear drive code for unit 0
	jmp	short issue_reset2	;

issue_reset1:
	or	wd_cmd_file+5,10h	;or drive code for unit 1
issue_reset2:
	mov	wd_cmd_file+6,wd_set_parms
	call	wd_init_drv		;go initialize drive
	mov	wd_cmd_file+6,wd_recal
	call	wd_recal_drv		;
	ret				;
issue_reset	endp

;
;	wd_get_status:	AH = 1
;			return current disk status and reset it to 0
;	

wd_get_status	proc	near
	mov	al,disk_stat_1		;get current status in al
	mov	disk_stat_1,0		;and reset it to 0
	ret
wd_get_status	endp




;
;	wd_read_sec:	Read a sector from the hard disk
;			AH = 2, DL = drive code, DH = head number
;			CL = sector # ( 1-17) bits; 6,7 high bits of cyl #
;			CH = cylinder # ,low 8 bits 
;			AL = number of sectors to read
;			ES:BX = transfer address


wd_read_sec	proc	near
	push	ax			;
	mov	ax,8000h		;
	call	check_dma		;go check dma boundary for xfer address
	pop	ax			;
	jc	abort_wd_cmd		;error, return
	jmp	In_cmd			;go execute
wd_read_sec	endp




;
;	wd_write_sec:	Write a sector to the hard disk
;			AH = 3
;			BX,CX,DX & ES same as in Read command
;

wd_write_sec	proc	near

	push	ax
	mov	ax,8000h		;check xfer address
	call	check_dma		;
	pop	ax
	jc	abort_wd_cmd		;abort if error
	jmp	Out_cmd			;go execute 
wd_write_sec	endp





;	wd_read_long:	AH = 0AH
;			Rest of the input parameters same as read sec
;

wd_read_long	proc	near
	push	ax			;
	mov	ax,7f04h		;
	call	check_dma		;check for valid xfer address
	pop	ax
	jc	abort_wd_cmd		;abort if error
	jmp	In_cmd			;go execute
abort_wd_cmd:
	ret
wd_read_long	endp




;
;	wd_write_long:	AH = 0Bh
;			Rest of the input parameters same as in Read_sec
;

wd_write_long	proc	near
	push	ax
	mov	ax,7f04h		;
	call	check_dma		;
	pop	ax
	jc	abort_wd_cmd		;abort if error
	jmp	Out_cmd			;go execute
wd_write_long	endp



;
;	wd_verify_cmd:	Verify desired sectors
;			AH = 4
;

wd_verify_cmd	proc	near
	jmp	execute_cmd		;go execute the command
wd_verify_cmd	endp

;	wd_fmt_track:	Format a track
;			AH = 5
;			ES:BX points to 512 byte buffer of which only first
;			34 are valid. (2 bytes/sector for 17 sectors/track)
;


wd_fmt_track	proc	near
	call	fix_xfer_addr		;
	jmp	Out_cmd			;go execute the command
wd_fmt_track	endp





;
;	wd_get_parm:	Get disk parameters
;			AH = 8
;	Output Registers:	dl = #of physical drives attached to 1st controller
;				dh = maximum # of heads
;				ch = maximum # of cylinders (low 8 bits)
;				cl = sectors/track (bits 0-5),
;				     & high two bits of max cyl # (bits 6,7)
;


wd_get_parms	proc	near
	cmp	dl,2			;check for valid drive code
	jae	parms_err		;error,return
	call	get_parm_vec		;get parameter vector
	mov	cx,es:[bx]		;get max # of cylinders
	dec	cx			;adjust to 0 - N number
	dec	cx			;leave one for diagnostic purpose
	xchg	cl,ch			;
	and	cl,3			;adjust two high bits of cylinder
	ror	cl,1			;
	ror	cl,1			;
	or	cl,es:[bx][14]		;add sectors per track
	mov	[bp+bp_cx],cx		;save it for return parameters
	mov	dl,hf_num		;get # of drive
	mov	dh,es:[bx][2]		;get # of heads
	dec	dh			;adjust to # 0 - N
	mov	[bp+bp_dx],dx		;save dx for return
	clc				;clear carry
	mov	disk_stat_1,0		;set no error on return
	jmp	short get_parms_rtn	;return
parms_err:
	mov	ah,wd_init_err		;return error code
	mov	disk_stat_1,ah		;
	mov	al,0			;
	mov	[bp+bp_ax],ax		;return error
	mov	word ptr [bp+bp_cx],0		;
	mov	word ptr [bp+bp_dx],0		;
	stc				;set carry

get_parms_rtn:
	ret
wd_get_parms	endp
	
;
;	wd_init_drv:	Initialize drive 
;			AH = 9, DL = drive code
;

wd_init_drv	proc	near

	call	get_parm_vec		;get drive parameters
	push	cx			;
	mov	cl,es:[bx][14]		;get maximum sector number
	mov	wd_cmd_file+1,cl	;store it in command file
	mov	ch,wd_cmd_file+5	;get sectors/drive/head register

;clear lower 4 bits for head # - V2.56

	and	ch,0f0h			;
	mov	cl,es:[bx][2]		;get # of heads
	dec	cl			;
	or	ch,cl			;merge # of heads
	mov	wd_cmd_file+5,ch	;and save it
	mov	ax,0
	mov	wd_cmd_file+3,0		;reset all flags
	pop	cx
	call	issue_cmd		;go issue command
	jnz	wd_init_cmd_exit	;return
	push	bx			;
	call	chk_4_ready		;
	pop	bx
	jnz	wd_init_cmd_exit	;
	call	check_status
wd_init_cmd_exit:
	ret
wd_init_drv	endp

;	wd_seek_cmd:	seek to desired cylinder
;			AH = 0Ch

wd_seek_cmd	proc	near
	jmp	execute_cmd			;
wd_seek_cmd	endp


;
;	Test disk for ready status: AH = 010h, DL = drive code
;

wd_rdy_test	proc	near
	push	bx
	call	chk_4_ready
	pop	bx
	jnz	wd_rdy_test1
	mov	dx,1F6h			;
	mov	al,wd_cmd_file+5	;go select the drive
	out	dx,al			;
	call	check_stat		;go check status
	jnz	wd_rdy_test1		;
	mov	disk_stat_1,0
wd_rdy_test1:
	ret
wd_rdy_test	endp


;
;	wd_recal_drv:	recalibrate desired drive.
;			AH = 11h, DL = drive code 0,1
;

wd_recal_drv	proc	near
	call	execute_cmd_sub		;
	call	test_4_results		;
	ret				;
wd_recal_drv	endp


;
;	Execute_cmd:	Come here via a jump instruction
;	Execute_cmd_sub:	Come here via call instruction
;

Execute_cmd	label	near
Execute_cmd_sub	proc	near

	call	issue_cmd		;go send command file to the cntrl
	jnz	exit_exec_cmd		;exit if error
	call	wait_4_intr		;go wait for an interrupt
	jnz	exit_exec_cmd		;exit if error
	call	check_status		;
	cmp	disk_stat_1,wd_seek_err	;if seek error,let it go
	jne	exit_exec_cmd		;if not then leave it alone
	and	disk_stat_1,0		;3/11/8+
exit_exec_cmd:
	ret
Execute_cmd_sub	endp			

;	wd_cntrl_diag:	Perform controller diagnostics
;			AH = 14h

wd_cntrl_diag	proc	near
	call	Init_Int2		;initialize second interrupt controller
	call	Init_Int1		;let interrupts pass through 1st 8259
	
;	push	bx     			;##
;	call	chk_4_ready		;
;	pop	bx			;
;	jnz	wd_diag_err		;##

	mov	disk_stat_1,0		;set status to no error
	mov	al,wd_diag		;get diagnostic command
	mov	dx,01F7h		;get port address
	out	dx,al			;
	push	bx			;
	call	chk_4_ready		;
	pop	bx			;
	jnz	wd_diag_err		;report an error
	mov	dx,01F1h		;
	in	al,dx			;get error register
	cmp	al,1			;any errors
	jne	wd_diag_err		;yes,
	ret
wd_diag_err:
	mov	disk_stat_1,20h 		;set bad controler status
	ret
wd_cntrl_diag	endp


;
;	wd_DASD_type:	return type of disk drive 
;			AH = 15h,  DL =80 or 81
;	On return AH = 3 if valid fixed disk or = 0 if invalid
;		CX,DX contains # of 512 byte sectors ob the disk or = 0
;

wd_DASD_type	proc	near
	mov	al,hf_num		;get # of fixed drives connnected
	cmp	al,dl			;are we within the range
	jbe	type_error		;no
	call	get_parm_vec		;
	mov	al,es:[bx][14]		;get # of sectors/track
	mov	cl,es:[bx][2]		;get # of heads
	imul	cl			;AX = number of sectors/cylinder
	mov	cx,es:[bx]		;get maximum # of cylinders
	dec	cx			;leave one for diagnostic purpose
	imul	cx			;(# ofsectors/cyl) X (# of cyl)
	push	dx			;results in DX:AX
	push	ax			;put results in CX:DX
	pop	[bp+bp_dx]		;
	pop	[bp+bp_cx]		;
;	xor	ax,ax			;
;	mov	ah,03			;
	mov	ax,0333h		;set special sucess code.  It is checked
;	mov	[bp+bp_ax],ax		;  before exit int 13h. (v2.03 W. Chessen)
	mov	disk_stat_1,0		;set no error status
	clc				;clear carry
	jmp	short DASD_type_rtn	;return

type_error:
	mov	word ptr [bp+bp_ax],0		;return 0 in AX,CX and DX
	mov	word ptr [bp+bp_cx],0
	mov	word ptr [bp+bp_dx],0
	mov	disk_stat_1,0ffh	;set this unique code to be checked
					;before returning to the user
	stc				;set carry	

DASD_type_rtn:
	ret
wd_DASD_type	endp

;	Wait_4_intr:	wait here for interrupt to occur. If no interrupt
;			then report timeout error

wait_4_intr	proc	near
	sti				;enable interrupts
	mov	disk_stat_1,0		;assume no error
	mov	ah,90h			;
	mov	al,0			;serial device
	int	15h			;process some thing else
	jc	wait_4_intr1		;error if carry set
	mov	cx,0			;start of wait loop
	mov	al,op_wait
wait_1:
	test	hf_int_flg,80h		;has interrupt occured
	jnz	wait_4_intr2		;yes, then exit from loop
	loop	wait_1
	dec	al
	jnz	wait_1			;loop again

wait_4_intr1:
	mov	disk_stat_1,80h		;time out error
	jmp	short wait_4_intr3	;

wait_4_intr2:
	and	hf_int_flg,0		;initialize interrupt flag

wait_4_intr3:
	call	test_4_results		;set Z flag
	ret

wait_4_intr	endp


;	
;	wd_intr:	This is a hard disk interrupt service routine
;

wd_intr	proc	near

	push	ax			;save registers
	push	ds			;
	call	set_data		;
	or	hf_int_flg,80h		;set high bit
	mov	al,EOI			;set end of interrupt
	out	int2,al			;
	call	delay
	out	int1,al			;
	sti				;start interrupts
	mov	ax,9100h		;tell it to user
	int	15h			;
	pop	ds
	pop	ax			;
	iret				;return from interrupt
wd_intr	endp

;	Check controller for not busy (ready) status
;	register used ax,bx,cx and dx

chk_4_ready	proc	near
	sti				;make sure interrupts are enabled
	xor	cx,cx			;
	mov	disk_stat_1,cl		;assume no error
	mov	bl,op_wait		;operation delay
	mov	dx,wd_stat_port		;
chk_1:
	in	al,dx			;read status
	test	al,80h			;is it ready
	jz	chk_2			;yes
	loop	chk_1			;try again
	dec	bl			;
	jnz	chk_1			;
	mov	disk_stat_1,time_out	;post error condition
chk_2:
	call	test_4_results		;set flag for results
	ret				;
chk_4_ready	endp			;


test_4_results	proc	near
	cmp	disk_stat_1,0		;set Z flag
	ret				;and return
test_4_results	endp


;
;	Wait_4_DRQ:	routine to wait for data request ready
;	from controller
;

Wait_4_DRQ	proc	near
	mov	dx,wd_stat_port
	mov	cx,drq_wait		;get wait delay constant
	clc				;clear carry
wait_4_DRQ1:
	in	al,dx			;get status
	test	al,8
	jnz	wait_4_DRQ2		;all done retrun
	loop	wait_4_DRQ1		;wait more
	mov	disk_stat_1,time_out	;set timeout error
	cmc				;set carry
wait_4_DRQ2:
	ret				;
Wait_4_DRQ	endp		



;	Check_stat:	routine to check controller's status byte
;	and return error code in AH

check_stat	proc	near
	mov	ax,0			;
	mov	dx,wd_stat_port		;
	in	al,dx			;get disk status
	mov	hf_st_reg,al		;save it
	shl	al,1			;
	jc	check_stat1		;jump if bit 7 is set (busy bit)
	mov	ah,drv_not_rdy		;set error code for not ready
	shl	al,1			;
	jnc	check_stat1		;jump  if bit 6 is zero (ready bit)
	mov	ah,cntrl_wrt_flt	;set error code for write fault
	shl	al,1			;see if bit 5 is set
	jc	check_stat1		;jump if write fault
	mov	ah,wd_seek_err		;
	shl	al,1			;check if bit 4 is zero
	jnc	check_stat1		;jump if seek complete error
	mov	ah,data_valid_err	;set error for ECC
	shl	al,1			;
	shl	al,1			;see if bit 2 is set
	jc	check_stat1		;
	mov	ah,0			;if non of above,then set it to 0
check_stat1:
	mov	al,hf_st_reg		;get the original status back
	mov	disk_stat_1,ah		;set error code
	cmp	ah,11h			;
	jz	check_stat2
	call	test_4_results
check_stat2:
	ret				;
check_stat	endp			;

;	Check_status:	Routine to check status 

Check_status	proc	near
	push	dx
	call	check_stat		;go read status
	jnz	check_status1		;
	test	al,1			;any other errors
	jz	check_status1		;no
	mov	dx,wd_port+1		;get error register
	in	al,dx			;
	mov	hf_err_reg,al		;save error register
	push	bx
	push	cx
	mov	bx, offset err_tbl	;get address of the map table
	mov	cx,8			;check 8 bits
wd_map_1:
	mov	ah,byte ptr cs:[bx]	;assume bit is set
	shl	al,1			;check it
	jc	wd_map_2		;if we are right then return
	inc	bx			;no, check next bit
	loop	wd_map_1		;
	mov	ah,0E0h			;make sure we get no error
wd_map_2:
	mov	disk_stat_1,ah		;set disk status
	pop	cx
	pop	bx

check_status1:
	call	test_4_results		;set condition code
	pop	dx			;
	ret				;
check_status	endp



;
;	check_dma:	This routine checks if the data transfer does
;			occur across 64k boundary
;	AX = 8000h if no ECC is requested or
;	AX = 7F04h if ECC is requested (long reads & writes)
;

check_dma	proc	near
	push	ax			;
	call	fix_xfer_addr		;
	pop	ax			;
	cmp	ah,wd_cmd_file+1	;check # of sectors
	ja	dma_ok			;if above then ok
	jb	dma_error		;if below then error
	cmp	al,bl			;
	jb	dma_error		;
	clc				;
dma_ok:
	ret				;
dma_error:
	mov	disk_stat_1,9		;send error code back
	stc				;set carry
	ret				;return
check_dma	endp

;	Issue_cmd:	This routine send the wd_cmd_file to the controller
;
;	Output	AL = status register,  AH = error register

Issue_cmd	proc	near
	
	call	wait_4_cntrl		;wait till controller is ready
	jnz	issue_cmd_rtn		;not ready, error
	mov	hf_int_flg,0		;reset interrupt flag
	mov	disk_stat_1,0		;assume no errors
	cli
	call	Init_Int2		;initialize second 8259
	call	Init_Int1		;let the interrupts pass through 1st
	sti
	mov	cx,7			;send 7 bytes to controller
	test	ctrl_byte,0c0h		;check for retries allowed
	jz	Issue_cmd3		;no
;	mov	bl,wd_cmd_file+6	;get opcode
;	cmp	bl,20h			;check for read,write and verify
	cmp	[wd_cmd_file+6],20h	;Don't modify BL (v2.01 W. Chessen)
	jb	Issue_cmd3		;commands
;	cmp	bl,42h			;add ECC mode
	cmp	[wd_cmd_file+6],42h
	ja	Issue_cmd3		;
	or	wd_cmd_file+6,retries	;set retry bit

Issue_cmd3:
	mov	di,offset wd_cmd_file
	mov	dx,1F1h			;get port address

Issue_cmd4:
	mov	al,[di]			;get parameter
	out	dx,al			;send it
	jmp	short $+2
	inc	di
	inc	dx			;
	loop	Issue_cmd4		;send 7 bytes
	call	test_4_results
Issue_cmd_rtn:
	ret				;
 Issue_cmd	endp


;
;	Routine to check for controller ready
;

wait_4_cntrl	proc	near

	push	bx			;save it
	push	di			;
	mov	cx,rdy_wait		;

wait_4_1:
	push	cx			;
	call	wd_rdy_test		;
	pop	cx			;
	jz	wait_rtn		;ready, exit from loop
	cmp	disk_stat_1,time_out	;was it time out
	jnz	Issue_cmd5		;no, then retry
	mov	disk_stat_1,20h		;if timeout, then controller is bad
	jmp	short wait_rtn		;

Issue_cmd5:
	loop	wait_4_1		;

wait_rtn:
	pop	di			;restore registers
	pop	bx			;
	call	test_4_results		;set results flag
	ret				;and return
wait_4_cntrl	endp

;	In_cmd:	Routine to read data from the disk

In_cmd	proc	near
	
	call	issue_cmd 		;go send command file to the controller
	jnz	In_cmd_exit		;exit if error
	call	wait_4_intr		;wait for interrupt completion
	jnz	In_cmd_exit		;return if error
	mov	di,bx			;setup di for block input

In_cmd_1:
	mov	cx,256			;setup cx for word count
	mov	dx,wd_port		;data port address

;disable interrupt during repeated input - V2.56

	cli				;clear interrupt during input
	cld				;set direction
	db	0F3h,06Dh		;hard code rep_insw instruction
	sti				;interrupt back on

;	push	cx
;	mov	cx,800h
;	loop	$
;	pop	cx

	call	get_ecc_bytes		;go get ecc bytes if applicable

	call	check_status		;go read status
	jnz	In_cmd_exit		;error
;	test	hf_st_reg,80h		;do we need to read more data

;check command buffer to see if more xfer needed - V2.56

	dec	byte ptr wd_cmd_file+1
	jz	In_cmd_exit		;no
	call	wait_4_intr		;wait for interrupt
	jz	short In_cmd_1		;go read more
In_cmd_exit:
	ret				;
In_cmd	endp


;
;	get_ecc_bytes:	Routine to read 4 bytes of ECC 
;

get_ecc_bytes	proc	near
	
	test	wd_cmd_file+6,ecc_mode	;is ECC requested
	jz	get_ecc_2		;no then exit
	call	wait_4_drq		;wait for data request
	jc	get_ecc_exit		;error
	mov	dx,wd_port		;
	mov	cx,4			;
get_ecc_3:
	cld				;	
	db	06Ch			;read 4 bytes	rlm 04/2/85
	loop	get_ecc_3		;hard coded INSB instruction
get_ecc_2:
	ret				;
get_ecc_exit:
	pop	cx			;get rid of return address
	ret				;return 1 level high
get_ecc_bytes	endp

;	Out_cmd:	Routine to send data to the controller

Out_cmd		proc	near

	call	issue_cmd		;go send command file to controller
	jnz	out_cmd_exit		;error? exit
	call	wait_4_drq		;
	jc	out_cmd_exit		;
	mov	si,bx			;source address
out_cmd_1:
	mov	cx,256			;send 256 words
	mov	dx,wd_port		;
	push	ds			;save ds
	mov	ax,es			;
	mov	ds,ax			;set ds to new value
	cld				;set direction
	db	0F3h,06Fh		;hard code rep_outsw instruction
	pop	ds			;restore ds
	call	send_ecc_bytes		;send ECC bytes if required
	call	wait_4_intr		;wait till command is done
	jnz	out_cmd_exit		;error
	call	check_status
	jnz	out_cmd_exit		;
	test	hf_st_reg,8		;need to send more data
	jnz	short out_cmd_1
out_cmd_exit:
	ret

out_cmd	endp



;	send_ecc_bytes:

send_ecc_bytes	proc	near
	
	test	wd_cmd_file+6,ecc_mode	;do we require ECC
	jz	send_ecc2		;no then exit
	call	wait_4_drq		;
	jc	send_ecc_exit		;error
	push	ds			;save ds register
	mov	ax,es			;put es register in ds  01/02/85
	mov	ds,ax			;
	mov	dx,wd_port
	mov	cx,4			;
send_ecc_3:
	cld				;
	db	06Eh			;output 4 bytes	rlm  04/02/85
	loop	send_ecc_3		;hard coded OUTSB instruction
	pop	ds			;restore ds
send_ecc2:
	ret				;

send_ecc_exit:
	pop	cx			;get rid of return address
	ret				;and go to 1 level high
send_ecc_bytes	endp

;	fix data transfer address

fix_xfer_addr	proc	near
	mov	bx,[bp+bp_bx]		;get original values
	mov	es,[bp+bp_es]		;
;	push	cx			;
;	mov	cl,4			;
;	shr	bx,cl			;use only high 3 nibbles
	shr	bx,4			;use only high 3 nibbles
;	pop	cx
	push	ax			;
	mov	ax,es			;
	add	ax,bx			;add then together
	mov	es,ax			;back into es
	pop	ax
	mov	bx,[bp+bp_bx]		;get original bx
	and	bx,0fh			;use only lower nibble now
	ret				;
fix_xfer_addr	endp			;


;
;	Init_Int1:  Initialize Interrupt controller # 1
;

Init_Int1	proc	near
	in	al,Int1+1
	jmp	short $+2
	and	al,0fbh			;let interrupts pass through the second
	out	Int1+1,al		;8259 chip
	ret
Init_Int1	endp



;
;	Init_Int2:  Initialize Interrupt Controller # 2
;

Init_Int2	proc	near
	in	al,Int2+1		;turn on the interrupts for hard
	jmp	short $+2
	and	al,0bfh			;disk
	out	Int2+1,al		;
	ret
Init_Int2	endp



;	setup parameters for eventual disk command

set_cmd_parms	proc	near
	call	get_parm_vec		;get address of the disk parms
	mov	al,es:[bx+8]		;get control byte modifier
	mov	ah,ctrl_byte		;
	and	ah,0c0h			;
	or	ah,al			;
	mov	ctrl_byte,ah		;save it back
	push	dx
	mov	dx,03f6h		;
	out	dx,al			;set extra head option if any
	mov	ax,[bp+bp_ax]		;get # of sectors to read
	cmp	ah,5			;check for format command
	jnz	parms_1			;no, then leave al alone
 	mov	al,es:[bx+14]		;yes, then get max sector number
parms_1:
	mov	wd_cmd_file+1,al	;save sector count
	push	cx			;
	and	cl,03fh			;get rid of high 2 bits
	mov	wd_cmd_file+2,cl	;save sector # to read (max = 17)
	mov	wd_cmd_file+3,ch	;save low byte of cylinder #
	pop	ax			;get cx in ax
;	mov	cl,6			;
;	shr	al,cl			;get 2 high bits of cyl from cl
	shr	al,6			;get 2 high bits of cyl from cl
	mov	wd_cmd_file+4,al	;save it
	pop	dx			;
;	mov	cl,4			;shift drive code 4 bits
;	shl	dl,cl			;
	shl	dl,4			;
	and	dh,0fh			;make sure head # is between 0-15
	or	dl,dh			;add them
	or	dl,0a0h			;set ECC and sector size (512 bytes)
	mov	wd_cmd_file+5,dl	;save it
	mov	ax,word ptr es:[bx+5]	;get precomp cylinder #
	shr	ax,1			;divide by 4
	shr	ax,1			;
	mov	wd_cmd_file,al		;save it
	mov	ax,[bp+bp_ax]		;restore few registers		
	push	ax
	xchg	al,ah
	mov	ah,0
	mov	si,ax
	mov	al,cs:[si+offset wd_cmd_tbl]
	mov	wd_cmd_file+6,al
	pop	ax
	mov	cx,[bp+bp_cx]		;
	mov	dx,[bp+bp_dx]		;leave ES and BX alone
	and	dx,0f01h		;drive code must be 0 or 1 and head
	ret				;and return
set_cmd_parms	endp


;
;	Get parameter table address for the drive code in dl
;	ES:BX point to the address
;

get_parm_vec	proc	near
	push	ax			;
	push	ds			;
	mov	ax,0			;
	mov	ds,ax			;
	les	bx,wd0_parm_vec		;assume drive code is 0
	test	dl,1			;make sure
	jz	parm_vec_rtn		;yes we are sure
	les	bx,wd1_parm_vec		;no drive code is 1	12/27/84
parm_vec_rtn:
	pop	ds			;
	pop	ax			;changed  rlm   12/27/84
	ret				;
get_parm_vec	endp			;

eproma	ends
	end
 
