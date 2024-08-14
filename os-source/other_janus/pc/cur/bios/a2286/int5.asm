include	title.inc
subttl	INT 5 (Print Screen)

.xlist
include equates.inc
include external.inc

extrn	ega_rows:byte
extrn	disp_cols:word
extrn	current_page:byte
extrn	print_stat:byte
.list

zero	segment at 0
zero	ends

dataa	segment	at 40h
dataa	ends

eproma	segment	byte public

	assume	cs:eproma,ds:dataa,es:dataa


public aa_int05
aa_int05 label byte

;******************************************************************************
;     			PRINT SCREEN  05H
;                
;     REGISTGERS ALTERED  NONE 
;
;     PRINT_STAT = 0   NOT BUSY
;                  1   BUSY
;                  FF  ERROR
;******************************************************************************
public		SCREEN_PRINT

SCREEN_PRINT PROC NEAR
        PUSH    AX                     ;SAVE REGISTERS
	call	save_all		;
;       test	PRINT_STAT,1          
;       Jnz      H4                     ;JUMP IF PRINT IN PROGRESS

;1/17/86 jwy -  To fix the problem with MS-DOS 2.11
	
	mov	al,1
	cmp	print_stat,al
	je	H4			;already in progress
	mov	print_stat,al

;	MOV     PRINT_STAT,1           ;SET BUSY
        STI

;check printer status

	mov	dx,0			;LPT1
	mov	ah,2			;get printer status
	int	17h
	test	ah,80h			;check busy bit
	jz	H5			;printer is busy
	test	ah,20h			;check out of paper bit
	jz	H6			;no error

;out of paper or busy
H5:	mov	print_stat,0ffh		;error flag
	jmp	short H4		;return

;GET CURSOR POSITION AND SAVE IT

H6:	MOV     BH,CURRENT_PAGE
	MOV	AH,3
	INT	10H
        PUSH    DX                     ;SAVE CURSOR POSITION

;GET SCREEN SIZE                   

;	MOV     CH,25			;CH = # ROWS
	MOV     CH,ega_rows		;CH = # ROWS
	inc	ch
        MOV     CL,BYTE PTR DISP_COLS	;CL = # COLUMNS      

;SEND CR, LF TO PRINTER

        MOV     AL,0AH
        CALL    OUT_TO_PRINTER         ;SEND LF TO PRINTER
        JNZ     H3                     ;EXIT IF PRINT ERROR
        MOV     AL,0DH 
        CALL    OUT_TO_PRINTER         ;SEND CR TO PRINTER
        JNZ     H3                     ;EXIT IF PRINT ERROR

;SET CURSOR POSITION

        SUB     DX,DX                  ;NEW CURSOR=0            
H1:     MOV     AH,2
        INT     010H                   ;SET CURSOR POSITION

;READ THE CHARACTER FROM THE SCREEN

        MOV     AH,8
        INT     010H                   ;READ CHARACTER FROM SCREEN
        CMP     AL,0
        JNE     H2                     ;JUMP IF VALID
        MOV     AL,020H                ;(AL)="BLANK"

;PRINT THE CHARACTER

H2:     CALL    OUT_TO_PRINTER         ;PRINT THE CHARACTER 
        JNZ     H3                     ;EXIT IF PRINT ERROR

;MOVE TO THE NEXT CHARACTER         

        INC     DL                     ;GO TO NEXT COLUMN
        CMP     CL,DL                  
        JNE     H1                     ;JUMP IF NOT END OF LINE
        XOR     DL,DL                                            
        MOV     AL,0AH                                         
        CALL    OUT_TO_PRINTER         ;SEND CR TO PRINTER 
        JNZ     H3                     ;JUMP IF PRINT ERROR
        MOV     AL,0DH
        CALL    OUT_TO_PRINTER         ;SEND LF TO PRINTER
        JNZ     H3                     ;EXIT IF PRINT ERROR
        INC     DH                     ;GOTO NEXT LINE
        CMP     CH,DH
        JNE     H1                     ;JUMP IF NOT DONE

;RESTORE THE CURSOR

H3:     POP     DX                     ;RECALL CURSOR POSITION
        MOV     AH,2
        INT     010H                   ;RESTORE CURSOR

;MAKE SURE PROPER STATUS IS IN PRINT STATUS

        TEST    PRINT_STAT,0FFH        ;PRINT ERROR ?
        JE      H4                     ;YES, JUMP
        MOV     PRINT_STAT,0           ;CLEAR BUSY

;RECOVER REGISTERS AND RETURN

H4:     STI
	call	restore_all
        POP     AX
        IRET                           ;RETURN
SCREEN_PRINT ENDP


;	SUBROUTINE TO SEND CHARACTER TO PRINTER
;	input	AL - char to be printed
;	output	ZF = 0 if error
;		ZF = 1 if successful operation

OUT_TO_PRINTER PROC NEAR
        PUSH    DX                     ;SAVE (DX)
        XOR     DX,DX			;LPT1
        XOR     AH,AH
        INT     017H                   ;PRINT CHARACTER IN (AL)
        POP     DX                     ;RECOVER (DX)
        TEST    AH,029H			;out of paper, i/o error or time out
        JZ      OTP1                   ;JUMP IF NO ERROR
        MOV     PRINT_STAT,0FFH        ;SET ERROR FLAG
OTP1:   RET                                            
OUT_TO_PRINTER ENDP

eproma	ends
	end

