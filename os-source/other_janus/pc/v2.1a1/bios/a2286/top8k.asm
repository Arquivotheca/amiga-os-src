include	title.inc
subttl	 TOP-8K Bios Module  

.xlist
include 	equates.inc
include	externals.inc

extrn	start:near
extrn	boot:near
extrn	serial_io:near
extrn	key:near
extrn	keyboard:near
extrn	wd_drv:near
extrn	floppy_intr:near
extrn	print:near
extrn	memory_size:word
extrn	equip_flag:word
extrn	intr_15:near
extrn	clock:near
extrn	time:near
extrn	unexpected_interrupt:near
;*extrn	disk_parm:dword
extrn	alarm:near
extrn	int71:near
extrn	math:near
extrn	screen_print:near
extrn	video:near
extrn	rd_dot:near
extrn	ram_size_1:near
extrn	config_1:near
extrn	parity_error:near

public	wd_config_tbl
public	config_table					      	
public	ver_msg
public	sign_on
public	video_start
public	key_table_1
public	key_table_2
public	key_table_3
public	key_table_4
public	key_table_5
public	key_table_6
public	key_table_7
public	begin
public	boot1
public	serial_io1
public	key1
public	keyboard1
public	wd_drv1
public	floppy_intr1
public	print1
public	video_parm_tbl
public	ram_size
public	config
public	intr_15_1
public	char_gen_table
public	time1
public	clock1
public	pwr_on_reset
public	copyright
public	nmi_int
public	floppy_parms

.list


zero	segment	at 0		 
zero	ends

dataa	segment at 40h
dataa	ends

work_ram	segment at 2000h

extrn	equip:byte
extrn	in_buff:byte
extrn	in_buff_ptr:byte
extrn	current_field:byte	
extrn	floppy:byte

work_ram	ends
		        	
disp_bfr	segment	at  0B800h
disp_bfr	ends

page

eproma	segment	public

	assume	cs:eproma,ds:dataa,es:dataa
	org	0CC0h				;start of top 8k module
;	org	9B90h				;start of top 8k module


copyright	label	byte

;	db	'FE-AT BIOS, COPYRIGHT (C) FARADAY ELECTRONICS  1/1/1985',0,0,0
	db 'A2286 BIOS, COPYRIGHT (C) 1988  Commodore Business Machines  '

ver_msg		label	byte
		
	if	SYSTEM_ID EQ A_TEASE
;	db	'Version 2.51',cr,lf,-1		;rlm  4/19/85
;	db	'Version 2.52',cr,lf,-1		;rlm  5/07/85
;	db	'Version 2.53',cr,lf,-1		;jwy  5/28/85
;	db	'Version 2.54',cr,lf,-1		;jwy  6/13/85
;	db	'Version 2.55',cr,lf,-1		;jwy  6/27/85
;	db	'Version 2.56A'			;jwy  8/02/85
;	db	'Version 2.56',cr,lf,-1		;jwy  8/16/85
;	db	'Version 2.57'			;jwy  11/5/85
;	db	'Version 2.58'			;jwy  01/15/86
;	db	'Version 2.59'			;rjj  07/07/86
;	db	'Version 2.60'			;rjj  12/08/86
;	db	'Version 3.01'			;tb   02/16/87
;	db	'Version 3.1'			;tb   02/27/87
;	db	'Version 3.2'			;tb   05/20/87
;	db	'Version 3.3'			;tb   11/23/87
;	db	'Version 3.4'			;tb   09/09/88
;	db	'Version 3.5'			;tb   12/09/88
	db	'Version 3.6'			;BK   10/10/89
	endif

ver_msg_end	label	word

.xlist
	db	0,'  ToBu was here '
.list

	org	copyright + 5Bh

reset	label	far			;jump here at reset time

begin:	jmp	start			;go do power on diags first

 
;******************************************************************************
;
;     			SIGN_ON MESSAGE 
;
;******************************************************************************

SIGN_ON LABEL BYTE     

 DB 'ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿',cr,lf
 DB '³ Commodore A2286 BIOS     Rev. 3.6           380 682 - 03    ³',cr,lf
 DB '³                                             380 683 - 03    ³',cr,lf
 DB 'ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´',cr,lf
 DB '³ Copyright (C) 1987, 1988  by Commodore Electronics Ltd.     ³',cr,lf
 DB '³                    All Rights Reserved.                     ³',cr,lf
 DB 'ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ',cr,lf
 DB lf,lf,-1


	org	copyright+02c3h

NMI_INT:
	jmp	parity_error

page+

;
;	Define hard disk parameter table for different types of disk drives.
;	Each entry in the table is 16 bytes long and defined as follows:
;
;		byte 0,1    =  maximum # of cylinders
;		byte 2      =  maximum # of heads
;		byte 3,4    = reserved
;		byte 5,6    = value of write precomp cylinder
;		byte 7	  = reserved
;		byte 8      = control byte (bit 7 disable retries
;				        bit 3 more than 8 heads)
;		byte 9 - 11 = reserved
;		byte 12,13  = head landing cylinder value
;		byte 14	  = # of sectors per track
;		byte 15     = reserved
;
;



	org	copyright+401h			;start of wd parameter tabel

wd_config_tbl:

;	define parameters for drive type 1

	dw	306			;maximum cylinders
	db	4			;max heads
	dw	0			;reserved
	dw	128			;precomp value
	db	0			;reserved
	db	0			;control byte
	db	0,0,0
	dw	305			;landing zone
	db	17			;sectors per track
	db	0			;

;	drive type 2: 	21.4 Mb, 615 cyl, 4 heads

	dw	615
	db	4
	dw	0
	dw	300
	db	0,0,0,0,0
	dw	615
	db	17
	db	0

;	drive type 3:	32.1 Mb, 615 cyl, 6 heads

	dw	615
	db	6
	dw	0
	dw	300
	db	0,0,0,0,0
	dw	615
	db	17
	db	0

;	drive type 4:	65.4 Mb, 940 cyl, 8 heads

	dw	940
	db	8
	dw	0
	dw	512
	db	0,0,0,0,0
	dw	940
	db	17
	db	0

;	drive type 5:	49.0 Mb, 940 cyl, 6 heads

	dw	940
	db	6
	dw	0
	dw	512
	db	0,0,0,0,0
	dw	940
	db	17
	db	0

;	drive type 6:	21.4 Mb, 615 cyl, 4 heads

	dw	615
	db	4
	dw	0
	dw	-1			;no pre-compensation
	db	0,0,0,0,0
	dw	615			;
	db	17
	db	0

;	drive type 7:	32.1 Mb, 462 cyl, 8 heads

	dw	462
	db	8
	dw	0
	dw	256
	db	0,0,0,0,0
	dw	511
	db	17
	db	0

;	drive type 8:	31.9 Mb, 733 cyl, 5 heads

	dw	733
	db	5
	dw	0
	dw	-1
	db	0,0,0,0,0
	dw	733
	db	17
	db	0

;	drive type 9	117.5 Mb, 900 cyl, 15 heads

	dw	900
	db	15
	dw	0
	dw	-1
	db	0,8,0,0,0
	dw	901
	db	17
	db	0

;	drive type 10:	21.4 Mb, 820 cyl, 3 heads

	dw	820
	db	3
	dw	0
	dw	-1
	db	0,0,0,0,0
	dw	820
	db	17
	db	0

;	drive type 11:	37.2 Mb, 855 cyl, 5 heads

	dw	855
	db	5
	dw	0
	dw	-1
	db	0,0,0,0,0
	dw	855
	db	17
	db	0

;	drive type 12:	52.0 Mb, 855 cyl, 7 heads

	dw	855
	db	7
	dw	0
	dw	-1
	db	0,0,0,0,0
	dw	855
	db	17
	db	0

;	drive type 13:	21.3 Mb, 306 cyl, 8 heads

	dw	306
	db	8
	dw	0
	dw	128
	db	0,0,0,0,0
	dw	319
	db	17
	db	0

;	drive type 14:	44.6 Mb, 733 cyl, 7 heads

	dw	733
	db	7
	dw	0
	dw	-1
	db	0,0,0,0,0
	dw	733
	db	17
	db	0

;	drive type 15

	dw	0,0,0,0,0,0,0,0		;not used

	
page+

;	drive type 16:	MINISCRIBE	; 20.3 MB

	dw	612			;no of cylinders
	db	4			;no of heads
	dw	0
	dw	0			;write precompensation
	db	0,0,0,0,0			;control byte
	dw	663			;landing zone
	db	17			;sectors per track
	db	0

;	drive type 17:	MITSUBITSHI MR535	; 40.3 MB

	dw	977			;no of cylinders
	db	5			;no of heads
	dw	0
	dw	300			;write precompensation
	db	0,0,0,0,0			;control byte
	dw	977			;landing zone
	db	17			;sectors per track
	db	0

;	drive type 18:	56.8 Mb

	dw	977			;no of cylinders
	db	7			;no of heads
	dw	0
	dw	-1			;write precompensation
	db	0,0,0,0,0		;control byte
	dw	977			;landing zone
	db	17			;sectors per track
	db	0

;	drive type 19:	44.6 Mb, 1024 cyls, 7 heads

	dw	1024			;no of cylinders
	db	7			;no of heads
	dw	0
	dw	512			;write precompensation
	db	0,0,0,0,0		;control byte
	dw	1023			;landing zone
	db	17			;sectors per track
	db	0

;	drive type 20:	RODIME RO5090	; 72.8 MB

	dw	1224			;no of cylinders
	db	7			;no of heads
	dw	0
	dw	-1			;write precompensation
	db	0,8,0,0,0			;control byte
	dw	1224			;landing zone
	db	17			;sectors per track
	db	0

;	drive type 21:	44.6 Mb, 733 cyls, 7 heads

	dw	733			;no of cylinders
	db	7			;no of heads
	dw	0
	dw	300			;write precompensation
	db	0,0,0,0,0		;control byte
	dw	732			;landing zone
	db	17			;sectors per track
	db	0

;	drive type 22:			; 40.5 MB

	dw	977			;no of cylinders
	db	5			;no of heads
	dw	0
	dw	300			;write precompensation
	db	0,0,0,0,0			;control byte
	dw	977			;landing zone
	db	17			;sectors per track
	db	0

;	drive type 23:	44.6 Mb, 306 cyls, 4 heads

	dw	306			;no of cylinders
	db	4			;no of heads
	dw	0
	dw	0			;write precompensation
	db	0,0,0,0,0		  	;control byte
	dw	336			;landing zone
	db	17			;sectors per track
	db	0

;	drive type 24:	40.9 Mb, 805 cyls, 4 heads
;	CONNER 40Mb AT DRIVE
	dw	805			;no of cylinders
	db	4			;no of heads
	dw	0
	dw	0			;write precompensation
	db	0,0,0,0,0		;control byte
	dw	820			;landing zone
	db	26			;sectors per track
	db	0

;	drive type 25: 100 Mb, 776 cyls, 8 heads
;	CONNER 100Mb AT DRIVE
	dw	776			;no of cylinders
	db	8			;no of heads
	dw	0
	dw	0			;write precompensation
	db	0,0,0,0,0		;control byte
	dw	800			;landing zone
	db	33			;sectors per track
	db	0

;	drive type 26:	40 Mb, 745 cyls, 4 heads
;	MINISCRIBE 40MB AT DRIVE
	dw	745			;no of cylinders
	db	4			;no of heads
	dw	0
	dw	0			;write precompensation
	db	0,0,0,0,0		;control byte
	dw	800			;landing zone
	db	28			;sectors per track
	db	0

;	drive type 27; 41.2 Mb, 625 cyls, 5 heads
;	RODIME 40MB AT DRIVE 	
	dw	625			;no of cylinders
	db	5			;no of heads
	dw	0
	dw	0			;write precompensation
	db	0,0,0,0,0		;control byte
	dw	871			;landing zone
	db	27			;sectors per track
	db	0

;	drive type 28 - 40 Mb, 965 cyls, 5 heads
;	QUANTUM 40MB AT DRIVE
	dw	965			;no of cylinders
	db	5			;no of heads
	dw	0
	dw	0			;write precompensation
	db	0,0,0,0,0		;control byte
	dw	1000			;landing zone
	db	17			;sectors per track
	db	0

;	drive type 29 - 80 Mb, 965 cyls, 10 heads
;	QUANTUM 80MB AT DRIVE
	dw	965			;no of cylinders
	db	10			;no of heads
	dw	0
	dw	0			;write precompensation
	db	0,8,0,0,0			;control byte
	dw	1000			;landing zone
	db	17			;sectors per track
	db	0

;	drive type 30 - 			; 40 MB
;	WESTERN DIGITAL 40MB AT DRIVE
	dw	782			;no of cylinders
	db	4			;no of heads
	dw	0
	dw	0			;write precompensation
	db	0,0,0,0,0			;control byte
	dw	800			;landing zone
	db	28			;sectors per track
	db	0

;	drive type 31 - TANDON TM755		; 40.7 MB

	dw	981			;no of cylinders
	db	5			;no of heads
	dw	0
	dw	-1			;write precompensation
	db	0,0,0,0,0			;control byte
	dw	981			;landing zone
	db	17			;sectors per track
	db	0


;	drive type 32 - MICROSCIENCE HH-105	; 42.5 MB

	dw	1024			;no of cylinders
	db	5			;no of heads
	dw	0
	dw	-1			;write precompensation
	db	0,0,0,0,0	 		;control byte
	dw	1024			;landing zone
	db	17			;sectors per track
	db	0


;	drive type 33 - HITACHI DK 521-5	; 41 MB

	dw	823			;no of cylinders
	db	6			;no of heads
	dw	0
	dw	-1			;write precompensation
	db	0,0,0,0,0			;control byte
	dw	823			;landing zone
	db	17			;sectors per track
	db	0


;	drive type 34 - TOSHIBA MK 53FB	; 34.5 MB

	dw	830			;no of cylinders
	db	5			;no of heads
	dw	0
	dw	512			;write precompensation
	db	0,0,0,0,0			;control byte
	dw	830			;landing zone
	db	17			;sectors per track
	db	0


;	drive type 35 - TOSHIBA MK 134FA 	; 41 MB

	dw	733			;no of cylinders
	db	7			;no of heads
	dw	0
	dw	512			;write precompensation
	db	0,0,0,0,0		 	;control byte
	dw	733			;landing zone
	db	17			;sectors per track
	db	0


;	drive type 36 - RODIME RO3055		; 43.4 MB

	dw	872			;no of cylinders
	db	6			;no of heads
	dw	0
	dw	650			;write precompensation
	db	0,0,0,0,0			;control byte
	dw	872			;landing zone
	db	17			;sectors per track
	db	0


;	drive type 37 - BASF		; 68 Mb

	dw	1024			;no of cylinders
	db	8			;no of heads
	dw	0
	dw	-1			;write precompensation
	db	0,0,0,0,0			;control byte
	dw	1024			;landing zone
	db	17			;sectors per track
	db	0


;	drive type 38 - SEAGATE ST4086	; 76.5 MB	

	dw	1024			;no of cylinders
	db	9			;no of heads
	dw	0
	dw	-1			;write precompensation
	db	0,8,0,0,0			;control byte
	dw	1024			;landing zone
	db	17			;sectors per track
	db	0


;	drive type 39 - FUJITSU M2227D2	; 40.8 MB

	dw	615			;no of cylinders
	db	8			;no of heads
	dw	0
	dw	128			;write precompensation
	db	0,0,0,0,0			;control byte
	dw	615			;landing zone
	db	17			;sectors per track
	db	0


;	drive type 40 - TOSHIBA MK 56FB 	; 68.9 MB

	dw	830			;no of cylinders
	db	10			;no of heads
	dw	0
	dw	512			;write precompensation
	db	0,8,0,0,0			;control byte
	dw	850			;landing zone
	db	17			;sectors per track
	db	0


;	drive type 41 - SEAGATE ST4096 	; 76.5 MB

	dw	1024			;no of cylinders
	db	9			;no of heads
	dw	0
	dw	128			;write precompensation
	db	0,8,0,0,0			;control byte
	dw	1024			;landing zone
	db	17			;sectors per track
	db	0


;	drive type 42:	48.2 Mb, 925 cyls, 5 heads (CDC 94155-48)

	dw	925			;no of cylinders
	db	5			;no of heads
	dw	0
	dw	-1			;no write precompensation
	db	0,0,0,0,0		;control byte
	dw	926			;landing zone
	db	17			;sectors per track
	db	0

;	drive type 43:	57 Mb, 925 cyls, 6 heads (CDC 94155-57)

	dw	925			;no of cylinders
	db	6			;no of heads
	dw	0
	dw	-1			;no write precompensation
	db	0,0,0,0,0		;control byte
	dw	926			;landing zone
	db	17			;sectors per track
	db	0

;	drive type 44:	67.4 Mb, 925 cyls, 7 heads (CDC 94155-67)

	dw	925			;no of cylinders
	db	7			;no of heads
	dw	0
	dw	-1			;no write precompensation
	db	0,0,0,0,0		;control byte
	dw	926			;landing zone
	db	17			;sectors per track
	db	0

;	drive type 45:	77 Mb, 925 cyls, 8 heads (CDC 94155-77)

	dw	925			;no of cylinders
	db	8			;no of heads
	dw	0
	dw	-1			;no write precompensation
	db	0,0,0,0,0		;control byte
	dw	926			;landing zone
	db	17			;sectors per track
	db	0

;	drive type 46:	86.7 Mb, 925 cyls, 9 heads (CDC 94155-86)

	dw	925			;no of cylinders
	db	9			;no of heads
	dw	0
	dw	-1			;no write precompensation
	db	0
	db	8			;control byte
	db	0,0,0
	dw	926			;landing zone
	db	17			;sectors per track
	db	0

;	drive type 47 - not used

	dw	0,0,0,0,0,0,0,0		;not used


page+
					


	org	copyright+06F2h
boot1:	jmp	boot			;boot_strap loader


config_table	label	byte		;used by int 15h func 0c0h
	dw	config_len - config_1st	;length
config_1st	equ	$
	db	0fch			;model byte
	db	1			;sub-model byte
	db	0			;bios level
	db	01110000b		;bit 7 - DMA ch 3 used by BIOS
					;bit 6 - cascaded int level 2
					;bit 5 - RTC available
					;bit 4 - Keyboard hoot 1ah
	db	0,0,0,0			;reserved
config_len	equ	$

;
;	Intr 14: serial I/O
;
	org	copyright+739h

serial_io1:	
	jmp	serial_io


	org	copyright+82Eh
;
;	Intr 16: Keyboard interrupt
;
Key1:
	jmp	key



;
;	Intr 9:	keyboard
;
	org	copyright+987h		

keyboard1:
	jmp	keyboard


page

	ORG 	copyright+990H
;
KEY_TABLE_1 LABEL BYTE                 ;KEYPAD TABLE
;
DB 07,08,09,-1,04,05,06,-1,01,02,03,00,-1
;
KEY_TABLE_2 LABEL BYTE                 ;ALT MODE, UPPER BYTE
;
;DB -1
;DB -1,78H,79H,7AH,7BH,7CH,7DH,7EH,7FH,80H,81H,82H,83H,-1  ;1ST ROW
;DB -1,10H,11H,12H,13H,14H,15H,16H,17H,18H,19H,-1,-1,-1    ;2ND ROW
;DB -1,1EH,1FH,20H,21H,22H,23H,24H,25H,26H,-1,-1,-1        ;3RD ROW
;DB -1,-1,2CH,2DH,2EH,2FH,30H,31H,32H,-1,-1,-1,-1,-1       ;4TH ROW
;DB -1,39H,-1                                             ;5TH ROW
;DB 68H,69H,6AH,6BH,6CH,6DH,6EH,6FH,70H,71H                ;F KEYS
;DB -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1           ;KEYPAD
;
DB -1
DB 1,78H,79H,7AH,7BH,7CH,7DH,7EH,7FH,80H,81H,82H,83H,0fh         ;1ST ROW
DB 0a5h,10H,11H,12H,13H,14H,15H,16H,17H,18H,19H,1ah,1bh,1ch      ;2ND ROW
DB -1,1EH,1FH,20H,21H,22H,23H,24H,25H,26H,27h,28h,29h            ;3RD ROW
DB -1,2bh,2CH,2DH,2EH,2FH,30H,31H,32H,33h,34h,35h,-1,-1           ;4TH ROW
DB -1,39H,-1                                                    ;5TH ROW
DB 68H,69H,6AH,6BH,6CH,6DH,6EH,6FH,70H,71H                      ;F KEYS
DB -1,-1,97h,98h,99h,4ah,9bh,4ch,9dh,4eh,9fh,0a0h,0a1h,0a2h,53h ;KEYPAD
DB -1,-1,-1,8bh,8ch						;F11 F12
;
;
KEY_TABLE_3 LABEL BYTE                 ;CTRL MODE, UPPER BYTE
;
;DB -1
;DB 01,-1,03,-1,-1,-1,07,-1,-1,-1,-1,0CH,-1,0EH              ;1ST ROW
;DB -1,10H,11H,12H,13H,14H,15H,16H,17H,18H,19H,1AH,1BH,1CH   ;2ND ROW
;DB -1,1EH,1FH,20H,21H,22H,23H,24H,25H,26H,-1,-1,-1          ;3RD ROW
;DB -1,2BH,2CH,2DH,2EH,2FH,30H,31H,32H,-1,-1,-1,-1,72h        ;4TH ROW
;DB -1,39H,-1                                                ;5TH ROW
;DB 5EH,5FH,60H,61H,62H,63H,64H,65H,66H,67H                  ;F KEYS
;DB -1,00,77H,-1,84H,-1,73H,-1,74H,-1,75H,-1,76H,-1,-1,      ;KEYPAD
;
DB -1
DB 1,-1,3,-1,-1,-1,7,-1,-1,-1,-1,0ch,-1,0eh                  ;1ST ROW
DB 94h,10H,11H,12H,13H,14H,15H,16H,17H,18H,19H,1AH,1BH,1CH    ;2ND ROW
DB -1,1EH,1FH,20H,21H,22H,23H,24H,25H,26H,-1,-1,-1            ;3RD ROW
DB -1,2BH,2CH,2DH,2EH,2FH,30H,31H,32H,-1,-1,-1,-1,72h         ;4TH ROW
DB -1,39H,-1                                                  ;5TH ROW
DB 5EH,5FH,60H,61H,62H,63H,64H,65H,66H,67H                    ;F KEYS
DB -1,-1,77H,8dh,84H,8eh,73H,8fh,74H,90h,75H,91h,76H,92h,93h  ;KEYPAD
DB -1,-1,-1,89h,8ah
;
;
KEY_TABLE_4 LABEL BYTE                 ;CTRL MODE, LOWER BYTE
;
DB -1
DB 1BH,-1,00,-1,-1,-1,1EH,-1,-1,-1,-1,1FH,-1,7FH               ;1ST ROW
;DB -1,11H,17H,05H,12H,14H,19H,15H,09H,0FH,10H,1BH,1DH,0AH      ;2ND ROW
DB  0,11H,17H,05H,12H,14H,19H,15H,09H,0FH,10H,1BH,1DH,0AH      ;2ND ROW
DB -1,01H,13H,04H,06H,07H,08H,0AH,0BH,0CH,-1,-1,-1             ;3RD ROW
DB -1,1CH,1AH,18H,03H,16H,02H,0EH,0DH,-1,-1,-1,-1,0            ;4TH ROW
DB -1,20H,-1                                                   ;5TH ROW
DB  0, 0, 0, 0, 0, 0, 0, 0, 0, 0                               ;F KEYS
DB -1, 0, 0, -1, 0, -1, 0, -1, 0, -1, 0, -1, 0, -1, -1         ;KEYPAD
;
DB -1,-1,-1,0,0					; F11, F12
;

KEY_TABLE_5 LABEL BYTE                 ;UNSHIFTED MODE, LOWER BYTE
;
DB -1
DB 1BH,"1234567890-=", 08                                   ;1ST ROW
DB 09,"qwertyuiop[]", 0DH                                   ;2ND ROW
DB -1,"asdfghjkl;", 27H, 60H                                ;3ND ROW
DB -1,"\zxcvbnm,./", -1, "*"                                ;4RD ROW
DB -1, 20H, -1                                              ;5TH ROW
DB 0,0,0,0,0,0,0,0,0,0                                      ;F KEYS 
;DB -1, -1, 0, 0, 0, "-", 0, -1, 0, "+", 0, 0, 0, 0, 0, 0    ;KEYPAD
;                                                                  
DB -1, -1, 0, 0, 0, "-", 0, 0f0h, 0, "+", 0, 0, 0, 0, 0     ;KEYPAD
DB -1,-1,5ch,85h,86h
;
;
KEY_TABLE_6 LABEL BYTE                 ;SHIFTED MODE, UPPER BYTE
;
DB -1
DB 1,2,3,4,5,6,7,8,9,10,11,12,13,14                            ;1ST ROW
DB 15,16,17,18,19,20,21,22,23,24,25,26,27,28                   ;2ND ROW
DB -1,30,31,32,33,34,35,36,37,38,39,40,41                      ;3RD ROW
;DB -1,43,44,45,46,47,48,49,50,51,52,53,-1,-1                   ;4TH ROW
;DB -1,57,-1                                                    ;5TH ROW
;DB 54H,55H,56H,57H,58H,59H,5AH,5BH,5CH,5DH                     ;F KEYS
;DB -1,-1,71,72,73,74,75,76,77,78,79,80,81,82,83                ;KEYPAD
;
DB -1,43,44,45,46,47,48,49,50,51,52,53,-1,55                   ;4TH ROW
DB -1,57,-1                                                    ;5TH ROW
DB 54H,55H,56H,57H,58H,59H,5AH,5BH,5CH,5DH                     ;F KEYS
DB -1,-1,71,72,73,74,75,76,77,78,79,80,81,82,83                ;KEYPAD
DB -1,-1,56h,87h,88h
;
;
KEY_TABLE_7 LABEL BYTE                 ;SHIFTED LOWER BYTE
;
DB -1
DB 1BH,"!@#$%^&*()_+", 08                                  ;1ST ROW  
DB  0,"QWERTYUIOP{}", 0DH                                  ;2ND ROW
DB -1,"ASDFGHJKL:", 22H, "~"                               ;3RD ROW
;DB -1,"|ZXCVBNM<>?", -1,                                   ;4TH ROW
;DB -1, 20H, -1                                             ;5TH ROW
;DB  0, 0, 0, 0, 0, 0, 0, 0, 0, 0                           ;F KEYS
;DB -1, -1,"789-456+1230."                                  ;KEYPAD
;                                                                   
DB -1,"|ZXCVBNM<>?",-1,"*"                                 ;4TH ROW
DB -1, 20H, -1                                             ;5TH ROW
DB  0, 0, 0, 0, 0, 0, 0, 0, 0, 0                           ;F KEYS
DB -1, -1,"789-456+1230."                                  ;KEYPAD
DB -1,-1,7ch,0,0
;

page

;
;	Intr 13:  Disk/Diskette IO
;
	org	copyright+0C59h

wd_drv1:
	jmp	wd_drv


;
;	Intr 0Eh: Floppy diskette interrupt routine
;
	org	copyright+0F57h
floppy_intr1:
	jmp	floppy_intr

;
;	Intr 1Eh: Floppy diskette parameter table
;
	org	copyright+0FC7h

Floppy_parms	label	byte
	db	0DFh ;Cfh		;refer to Floppy driver for details
	db	2
	db	25h	 	; motor wait
	db	2		; 512bytes/sec
	db	0fh ;8		; max sec
	db	1Bh ;2Ah		; gap length
	db	0FFh		; data length
	db	54h ;50h		; format gap
	db	0F6h		; format filler pattern
	db	15  ;25		; ms of head settle time
	db	8   ;4		; * 125ms of motor start time


;
;	Intr 17h:  Parallel printer I/O
;
	org	copyright+0FD2h

print1:	jmp	print


;                                        
;                                      
;	VIDEO SUBROUTINE TABLES
;                                        

	org	copyright+1045h
	assume cs:eproma,ds:dataa,es:disp_bfr

.xlist
extrn	set_video_mode:near
extrn	set_cursor_type:near
extrn	set_cursor_posn:near
extrn	rd_cursor_posn:near
extrn	rd_lpen_posn:near
extrn	sel_disp_page:near
extrn	scroll_scr_up:near
extrn	scroll_scr_dwn:near
extrn	rd_atr_char:near
extrn	wr_atr_char:near
extrn	wr_char:near
extrn	set_color_plt:near
extrn	wr_dot:near
extrn	wr_teletype:near
extrn	rd_video_state:near
.list

        DW      OFFSET SET_VIDEO_MODE         ;0
        DW      OFFSET SET_CURSOR_TYPE        ;1
        DW      OFFSET SET_CURSOR_POSN        ;2
        DW      OFFSET RD_CURSOR_POSN         ;3
        DW      OFFSET RD_LPEN_POSN           ;4
        DW      OFFSET SEL_DISP_PAGE          ;5
        DW      OFFSET SCROLL_SCR_UP          ;6
        DW      OFFSET SCROLL_SCR_DWN         ;7
        DW      OFFSET RD_ATR_CHAR            ;8
        DW      OFFSET WR_ATR_CHAR            ;9
        DW      OFFSET WR_CHAR                ;10
        DW      OFFSET SET_COLOR_PLT          ;11
        DW      OFFSET WR_DOT                 ;12
        DW      OFFSET RD_DOT                 ;13
        DW      OFFSET WR_TELETYPE            ;14
        DW      OFFSET RD_VIDEO_STATE         ;15

;
;	Intr 10h:  Video output
;

VIDEO_START: 
	JMP     video

;
;PARAMETERS WHICH SILL BE LOADED INTO CRTC REGS R0 - 15
;FOR 40X25, 80X25, GRAPHICS, MONOCHROME     
;

;	Intr 1Dh:  Video parameters

	ORG 	copyright+10A4H

VIDEO_PARM_TBL LABEL BYTE

					;     40X25
      DB     38H                       	;R0  HORIZONTAL TOTAL (CHAR)
      DB     28H                       	;R1  HORIZONTAL DISPLAYED (CHAR)
      DB     2DH                       	;R2  HORIZONTAL SYNC POSN (CHAR)
      DB     0AH                       	;R3  HORIZONTAL SYNC WIDTH (CHAR)
      DB     1FH                       	;R4  VERTICAL TOTAL (CHAR ROW)
      DB     06H                       	;R5  VERTIACL TOTAL ADJUST (SCANLINE)
      DB     19H                       	;R6  VERTICAL DISPLAYED (CHAR ROW)
      DB     1CH                       	;R7  VERTICAL SYNC POSN
      DB     02H                       	;R8  INTERLACE MODE
      DB     07H                       	;R9  MAX SCAN LINE ADR (SCANLINE)
      DB     06H                       	;R10 CURSOR START (SCANLINE)
      DB     07H                       	;R11 CURSOR END (SCANLINE)
      DB      0                        	;R12 START ADDR HIGH BYTE
      DB      0                        	;R13 START ADDR LOW BYTE
      DB      0                        	;R14 CURSOR ADDR HIGH BYTE
      DB      0                        	;R15 CURSOR ADDR LOW BYTE

					;     80X25
      DB     71H                       	;R0  HORIZONTAL TOTAL (CHAR)
      DB     50H                       	;R1  HORIZONTAL DISPLAYED (CHAR)
      DB     5AH                       	;R2  HORIZONTAL SYNC POSN (CHAR)
      DB     0AH                       	;R3  HORIZONTAL SYNC WIDTH (CHAR)
      DB     1FH                       	;R4  VERTIACL TOTAL (CHAR ROW)
      DB     06H                       	;R5  VERTICAL TOTAL ADJUST (SCANLINE)
      DB     19H                       	;R6  VERTICAL DISPLAYED (CHAR ROW)
      DB     1CH                       	;R7  VERTICAL SYNC POSN
      DB     02H                       	;R8  INTERLACE MODE
      DB     07H                       	;R9  MAX SCAN LINE ADR (SCANLINE)
      DB     06H                       	;R10 CURSOR START (SCANLINE)
      DB     07H                       	;R11 CURSOR END (SCANLINE)
      DB      0                        	;R12 START ADDR HIGH BYTE
      DB      0                        	;R13 START ADDR LOW BYTE
      DB      0                        	;R14 CURSOR ADDR HIGH BYTE
      DB      0                        	;R15 CURSOR ADDR LOW BYTE

					;     GRAPHICS
      DB     38H                       	;R0  HORIZONTAL TOTAL (CHAR)
      DB     28H                       	;R1  HORIZONTAL DISPLAYED (CHAR)
      DB     2DH                       	;R2  HORIZONTAL SYNC POSN (CHAR)
      DB     0AH                       	;R3  VERTICAL TOTAL (CHAR ROW)
      DB     7FH                       	;R4  VERTICAL TOTAL ADJUST (SCANLINE)
      DB     06H                       	;R5  VERTICAL DISPLAYED (CHAR ROW)
      DB     64H                       	;R6  VERTICAL DISPLAYED (CHAR ROW)
      DB     70H                       	;R7  VERTICAL SYNC POSN
      DB     02H                       	;R8  INTERLACE MODE
      DB     01H                       	;R9  MAX SCAN LINE ADR (SCANLINE)
      DB     06H                       	;R10 CURSOR START (SCANLINE)
      DB     07H                       	;R11 CURSOR END (SCANLINE)
      DB      0                        	;R12 START ADDR HIGH BYTE
      DB      0                        	;R13 START ADDR LOW BYTE
      DB      0                        	;R14 CURSOR ADDR HIGH BYTE
      DB      0                        	;R15 CURSOR ADDR LOW BYTE

					;     MONOCHROME
      DB     61H                       	;R0  HORIZONTAL TOTAL (CHAR)
      DB     50H                       	;R1  HORIAONTAL DISPLAYED (CHAR)
      DB     52H                       	;R2  HORIAONTAL SYNC POSN (CHAR)
      DB     0FH                       	;R3  HORIZONTAL SYNC WIDTH (CHAR)
      DB     19H                       	;R4  VERTICAL TOTAL (CHAR ROW)
      DB     06H                       	;R5  VERTIACL TOTAL ADJUST (SCANLINE)
      DB     19H                       	;R6  VERTICAL DISPLAYED (CHAR ROW)
      DB     19H                       	;R7  VERTICAL SYNC POSN
      DB     02H                       	;R8  INTERLACE MODE
      DB     0DH                       	;R9  MAX SCAN LINE ADR (SCANLINE)
      DB     0BH                       	;R10 CURSOR START (SCANLINE)
      DB     0CH                       	;R11 CURSOR END (SCANLINE)
      DB      0                        	;R12 START ADDR HIGH BYTE
      DB      0                        	;R13 START ADDR LOW BYTE
      DB      0                        	;R14 CURSOR ADDR HIGH BYTE
      DB      0                        	;R15 CURSOR ADDR LOW BYTE

FE_AT_1	label	word
	dw	2048			;40X25
	dw	4096			;80X25
	dw	16384			;graphics
	dw	16384			;

FE_AT_2	label	byte
	db	40,40,80,80,40,40,80,80

FE_AT_3	label	byte
	db	02ch,028h,02dh,029h,02ah,02eh,01eh,029h


page
	org	copyright+1841h
	
ram_size	label	byte
	jmp	ram_size_1



	org	copyright+184Dh

config	label	byte

	jmp	config_1


;
;	Intr 15h:  Virtual mode
;
	org	copyright+1859h

Intr_15_1:
	jmp	Intr_15

page+

;
;	Define character generator table for 320 X 200  and 640 X 200
;	graphics
;

	org	copyright+1A6Eh

CHAR_GEN_TABLE	LABEL	BYTE

	DB	000H,000H,000H,000H,000H,000H,000H,000H ; D_00
	DB	07EH,081H,0A5H,081H,0BDH,099H,081H,07EH ; D_01
	DB	07EH,0FFH,0DBH,0FFH,0C3H,0E7H,0FFH,07EH ; D_02
	DB	06CH,0FEH,0FEH,0FEH,07CH,038H,010H,000H ; D_03
	DB	010H,038H,07CH,0FEH,07CH,038H,010H,000H ; D_04
	DB	038H,07CH,038H,0FEH,0FEH,07CH,038H,07CH ; D_05
	DB	010H,010H,038H,07CH,0FEH,07CH,038H,07CH ; D_06
	DB	000H,000H,018H,03CH,03CH,018H,000H,000H ; D_07
	DB	0FFH,0FFH,0E7H,0C3H,0C3H,0E7H,0FFH,0FFH ; D_08
	DB	000H,03CH,066H,042H,042H,066H,03CH,000H ; D_09
	DB	0FFH,0C3H,099H,0BDH,0BDH,099H,0C3H,0FFH ; D_0A
	DB	00FH,007H,00FH,07DH,0CCH,0CCH,0CCH,078H ; D_0B
	DB	03CH,066H,066H,066H,03CH,018H,07EH,018H ; D_0C
	DB	03FH,033H,03FH,030H,030H,070H,0F0H,0E0H ; D_0D
	DB	07FH,063H,07FH,063H,063H,067H,0E6H,0C0H ; D_0E
	DB	099H,05AH,03CH,0E7H,0E7H,03CH,05AH,099H ; D_0F

	DB	080H,0E0H,0F8H,0FEH,0F8H,0E0H,080H,000H ; D_10
	DB	002H,00EH,03EH,0FEH,03EH,00EH,002H,000H ; D-11
	DB	018H,03CH,07EH,018H,018H,07EH,03CH,018H ; D_12
	DB	066H,066H,066H,066H,066H,000H,066H,000H ; D_13
	DB	07FH,0DBH,0DBH,07BH,01BH,01BH,01BH,000H ; D_14
	DB	03EH,063H,038H,06CH,06CH,038H,0CCH,078H ; D_15
	DB	000H,000H,000H,000H,07EH,07EH,07EH,000H ; D_16
	DB	018H,03CH,07EH,018H,07EH,03CH,018H,0FFH ; D_17
	DB	018H,03CH,07EH,018H,018H,018H,018H,000H ; D_18
	DB	018H,018H,018H,018H,07EH,03CH,018H,000H ; D_19
	DB	000H,018H,00CH,0FEH,00CH,018H,000H,000H ; D_1A
	DB	000H,030H,060H,0FEH,060H,030H,000H,000H ; D_1B
	DB	000H,000H,0C0H,0C0H,0C0H,0FEH,000H,000H ; D_1C
	DB	000H,024H,066H,0FFH,066H,024H,000H,000H ; D_1D
	DB	000H,018H,03CH,07EH,0FFH,0FFH,000H,000H ; D_1E
	DB	000H,0FFH,0FFH,07EH,03CH,018H,000H,000H ; D_1F

	DB	000H,000H,000H,000H,000H,000H,000H,000H ; SP D_20
	DB	030H,078H,078H,030H,030H,000H,030H,000H ; ! D_21
	DB	06CH,06CH,06CH,000H,000H,000H,000H,000H ; " D_22
	DB	06CH,06CH,0FEH,06CH,0FEH,06CH,06CH,000H ; * D_23
	DB	030H,07CH,0C0H,078H,00CH,0F8H,030H,000H ; $ D_24
	DB	000H,0C6H,0CCH,018H,030H,066H,0C6H,000H ; PER CENT D_25
	DB	038H,06CH,038H,076H,0DCH,0CCH,076H,000H ; & D_26
	DB	060H,060H,0C0H,000H,000H,000H,000H,000H ; ' D_27
	DB	018H,030H,060H,060H,060H,030H,018H,000H ; ( D_28
	DB	060H,030H,018H,018H,018H,030H,060H,000H ; ) D_29
	DB	000H,066H,03CH,0FFH,03CH,066H,000H,000H ; * D_2A
	DB	000H,030H,030H,0FCH,030H,030H,000H,000H ; + D_2B
	DB	000H,000H,000H,000H,000H,030H,030H,060H ; , D_2C
	DB	000H,000H,000H,0FCH,000H,000H,000H,000H ; - D_2D
	DB	000H,000H,000H,000H,000H,030H,030H,000H ; . D_2E
	DB	006H,00CH,018H,030H,060H,0C0H,080H,000H ; / D_2F

	DB	07CH,0C6H,0CEH,0DEH,0F6H,0E6H,07CH,000H ; 0 D_30
	DB	030H,070H,030H,030H,030H,030H,0FCH,000H ; 1 D_31
	DB	078H,0CCH,00CH,038H,060H,0CCH,0FCH,000H ; 2 D_32
	DB	078H,0CCH,00CH,038H,00CH,0CCH,078H,000H ; 3 D_33
	DB	01CH,03CH,06CH,0CCH,0FEH,00CH,01EH,000H ; 4 D_34
	DB	0FCH,0C0H,0F8H,00CH,00CH,0CCH,078H,000H ; 5 D_35
	DB	038H,060H,0C0H,0F8H,0CCH,0CCH,078H,000H ; 6 D_36
	DB	0FCH,0CCH,00CH,018H,030H,030H,030H,000H ; 7 D_37
	DB	078H,0CCH,0CCH,078H,0CCH,0CCH,078H,000H ; 8 D_38
	DB	078H,0CCH,0CCH,07CH,00CH,018H,070H,000H ; 9 D_39
	DB	000H,030H,030H,000H,000H,030H,030H,000H ; : D_3A
	DB	000H,030H,030H,000H,000H,030H,030H,060H ; ; D_3B
	DB	018H,030H,060H,0C0H,060H,030H,018H,000H ; < D_3C
	DB	000H,000H,0FCH,000H,000H,0FCH,000H,000H ; = D_3D
	DB	060H,030H,018H,00CH,018H,030H,060H,000H ; > D_3E
	DB	078H,0CCH,00CH,018H,030H,000H,030H,000H ; ? D_3F

	DB	07CH,0C6H,0DEH,0DEH,0DEH,0C0H,078H,000H ; @ D_40
	DB	030H,078H,0CCH,0CCH,0FCH,0CCH,0CCH,000H ; A D_41
	DB	0FCH,066H,066H,07CH,066H,066H,0FCH,000H ; B D_42
	DB	03CH,066H,0C0H,0C0H,0C0H,066H,03CH,000H ; C D_43
	DB	0F8H,06CH,066H,066H,066H,06CH,0F8H,000H ; D D_44
	DB	0FEH,062H,068H,078H,068H,062H,0FEH,000H ; E D_45
	DB	0FEH,062H,068H,078H,068H,060H,0F0H,000H ; F D_46
	DB	03CH,066H,0C0H,0C0H,0CEH,066H,03EH,000H ; G D_47
	DB	0CCH,0CCH,0CCH,0FCH,0CCH,0CCH,0CCH,000H ; H D_48
	DB	078H,030H,030H,030H,030H,030H,078H,000H ; I D_49
	DB	01EH,00CH,00CH,00CH,0CCH,0CCH,078H,000H ; J D_4A
	DB	0E6H,066H,06CH,078H,06CH,066H,0E6H,000H ; K D_4B
	DB	0F0H,060H,060H,060H,062H,066H,0FEH,000H ; L D_4C
	DB	0C6H,0EEH,0FEH,0FEH,0D6H,0C6H,0C6H,000H ; M D_4D
	DB	0C6H,0E6H,0F6H,0DEH,0CEH,0C6H,0C6H,000H ; N D_4E
	DB	038H,06CH,0C6H,0C6H,0C6H,06CH,038H,000H ; O D_4F

	DB	0FCH,066H,066H,07CH,060H,060H,0F0H,000H ; P D_50
	DB	078H,0CCH,0CCH,0CCH,0DCH,078H,01CH,000H ; Q D_51
	DB	0FCH,066H,066H,07CH,06CH,066H,0E6H,000H ; R D_52
	DB	078H,0CCH,0E0H,070H,01CH,0CCH,078H,000H ; S D_53
	DB	0FCH,0B4H,030H,030H,030H,030H,078H,000H ; T D_54
	DB	0CCH,0CCH,0CCH,0CCH,0CCH,0CCH,0FCH,000H ; U D_55
	DB	0CCH,0CCH,0CCH,0CCH,0CCH,078H,030H,000H ; V D_56
	DB	0C6H,0C6H,0C6H,0D6H,0FEH,0EEH,0C6H,000H ; W D_57
	DB	0C6H,0C6H,06CH,038H,038H,06CH,0C6H,000H ; X D_58
	DB	0CCH,0CCH,0CCH,078H,030H,030H,078H,000H ; Y D_59
	DB	0FEH,0C6H,08CH,018H,032H,066H,0FEH,000H ; Z D_5A
	DB	078H,060H,060H,060H,060H,060H,078H,000H ; ( D_5B
	DB	0C0H,060H,030H,018H,00CH,006H,002H,000H ; BACKLASH D_5C
	DB	078H,018H,018H,018H,018H,018H,078H,000H ; ) D_5D
	DB	010H,038H,06CH,0C6H,000H,000H,000H,000H ; CIRCUMFLEX D_E
	DB	000H,000H,000H,000H,000H,000H,000H,0FFH ; _ D_5F

	DB	030H,030H,018H,000H,000H,000H,000H,000H ;   D_60
	DB	000H,000H,078H,00CH,07CH,0CCH,076H,000H ; LOWER CASE A D_61
	DB	0E0H,060H,060H,07CH,066H,066H,0DCH,000H ; L.C. B D_62
	DB	000H,000H,078H,0CCH,0C0H,0CCH,078H,000H ; L.C. C D_63
	DB	01CH,00CH,00CH,07CH,0CCH,0CCH,076H,000H ; L.C. D D_64
	DB	000H,000H,078H,0CCH,0FCH,0C0H,078H,000H ; L.C. E D_65
	DB	038H,06CH,060H,0F0H,060H,060H,0F0H,000H ; L.C. F D_66
	DB	000H,000H,076H,0CCH,0CCH,07CH,00CH,0F8H ; L.C. G D_67
	DB	0E0H,060H,06CH,076H,066H,066H,0E6H,000H ; L.C. H D_68
	DB	030H,000H,070H,030H,030H,030H,078H,000H ; L.C. I D_69
	DB	00CH,000H,00CH,00CH,00CH,0CCH,0CCH,078H ; L.C. J D_6A
	DB	0E0H,060H,066H,06CH,078H,06CH,0E6H,000H ; L.C. K D_6B
	DB	070H,030H,030H,030H,030H,030H,078H,000H ; L.C. L D_6C
	DB	000H,000H,0CCH,0FEH,0FEH,0D6H,0C6H,000H ; L.C. M D_6D
	DB	000H,000H,0F8H,0CCH,0CCH,0CCH,0CCH,000H ; L.C. N D_6E
	DB	000H,000H,078H,0CCH,0CCH,0CCH,078H,000H ; L.C. O D_6F

	DB	000H,000H,0DCH,066H,066H,07CH,060H,0F0H ; L.C. P D_70
	DB	000H,000H,076H,0CCH,0CCH,07CH,00CH,01EH ; L.C. Q D_71
	DB	000H,000H,0DCH,076H,066H,060H,0F0H,000H ; L.C. R D_72
	DB	000H,000H,07CH,0C0H,078H,00CH,0F8H,000H ; L.C. S D_73
	DB	010H,030H,07CH,030H,030H,034H,018H,000H ; L.C. T D_74
	DB	000H,000H,0CCH,0CCH,0CCH,0CCH,076H,000H ; L.C. U D_75
	DB	000H,000H,0CCH,0CCH,0CCH,078H,030H,000H ; L.C. V D_76
	DB	000H,000H,0C6H,0D6H,0FEH,0FEH,06CH,000H ; L.C. W D_77
	DB	000H,000H,0C6H,06CH,038H,06CH,0C6H,000H ; L.C. X D_78
	DB	000H,000H,0CCH,0CCH,0CCH,07CH,00CH,0F8H ; L.C. Y D_79
	DB	000H,000H,0FCH,098H,030H,064H,0FCH,000H ; L.C. Z D_7A
	DB	01CH,030H,030H,0E0H,030H,030H,01CH,000H ;   D_7B
	DB	018H,018H,018H,000H,018H,018H,018H,000H ;   D_7C
	DB	0E0H,030H,030H,01CH,030H,030H,0E0H,000H ;   D_7D
	DB	076H,0DCH,000H,000H,000H,000H,000H,000H ;   D_7E
	DB	000H,010H,038H,06CH,0C6H,0C6H,0FEH,000H ; DELTA D_7F
page+


;
;	Intr 1Ah:  Time of day
;
	org	copyright+1E6Eh
clock1:
	jmp	clock


;
;	Timer Interrupt
;
	org	copyright+1EA5h
time1:
	jmp	time


;
;	Interrupt vector table
;
	org	copyright+1EF3h

DW     OFFSET TIME1                    	;08 TIMER
DW     OFFSET KEYBOARD1                	;09 KEYBOARD
DW     OFFSET UNEXPECTED_INTERRUPT     	;0A 
DW     OFFSET UNEXPECTED_INTERRUPT     	;0B SERIAL 2
DW     OFFSET UNEXPECTED_INTERRUPT     	;0C SERIAL 1
DW     OFFSET UNEXPECTED_INTERRUPT     	;0D 
DW     OFFSET floppy_intr		;OE diskette
DW     OFFSET UNEXPECTED_INTERRUPT     	;0F PRINTER
DW     OFFSET VIDEO_START              	;10 VIDEO
DW     OFFSET CONFIG                   	;11 CONFIGURATION
DW     OFFSET RAM_SIZE                 	;12 MEMORY SIZE
DW     OFFSET wd_drv1		;13 DISK
DW     OFFSET SERIAL_IO1               	;14 SERIAL
DW     OFFSET Intr_15		;15 CASSETTE
DW     OFFSET KEY1                     	;16 KEYBOARD
DW     OFFSET PRINT1                   	;17 PRINTR
DW     OFFSET BREAK                    	;18 BASIC           
DW     OFFSET BOOT1                    	;19 BOOT
DW     OFFSET CLOCK1                   	;1A TIME OF DAY
DW     OFFSET BREAK                    	;1B KEYBOARD BREAK
DW     OFFSET BREAK                    	;1C TIMER USER INTERRUPT
DW     OFFSET VIDEO_PARM_TBL           	;1D VIDEO PARAMETERS
DW     OFFSET floppy_parms              	;1E DISK PARAMETERS
DW     0                               	;1F VIDEO GRAPHICS CHAR
DW     OFFSET ALARM                    	;70 CMOS CLOCK CLAENDAR
DW     OFFSET INT71			;71                    
DW     OFFSET UNEXPECTED_INTERRUPT     	;72 
DW     OFFSET UNEXPECTED_INTERRUPT     	;73 
DW     OFFSET UNEXPECTED_INTERRUPT     	;74 COPROCESSOR INTERRUPT
DW     OFFSET MATH                     	;75
DW     OFFSET UNEXPECTED_INTERRUPT     	;76
DW     OFFSET UNEXPECTED_INTERRUPT     	;77

page+

;
;	Dummy interrupts: just do a return
;

	org	copyright+1F53h

break:	iret

;
;	Intr 5: Print Screen
;
Screen1:
	jmp	screen_print

;
;	Come here when CPU is reset. There must be a jump instruction
;	at location FFFF0h

	org	copyright+1ff0h

public	Pwr_on_reset

Pwr_on_reset	proc	far
	db	0eah			;hard coded jump opcode
	dw	0E05Bh	; [offset	reset]	;location of reset code
	dw	0F000h			;in this segment

	db	'09/xx/88 '  		;release date
	db	0fch			;FE-AT identification byte
	db	0			;checksum
Pwr_on_reset	endp

eproma	ends
	end

