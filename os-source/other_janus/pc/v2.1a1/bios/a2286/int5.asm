
page	60,132
include	title.inc
subttl	INT 5 (Print Screen)

.xlist
include equates.inc
include externals.inc

extrn	disp_cols:word
extrn	current_page:byte
extrn	print_stat:byte
public	screen_print
.list

zero	segment at 0
zero	ends

dataa	segment	at 40h
dataa	ends

eproma	segment	byte public

	assume	cs:eproma,ds:dataa,es:dataa

.xlist
start_of_int5	equ	$+180h
.list
page
;******************************************************************************
;     			PRINT SCREEN  05H
;                
;     REGISTGERS ALTERED  NONE 
;
;     PRINT_STAT = 0   NOT BUSY
;                  1   BUSY
;                  FF  ERROR
;******************************************************************************

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

;GET CURSOR POSITION AND SAVE IT

        MOV     BH,CURRENT_PAGE
	MOV	AH,3
	INT	10H
        PUSH    DX                     ;SAVE CURSOR POSITION

;GET SCREEN SIZE                   

        MOV     CH,25                  ;CH = # ROWS
        MOV     CL,BYTE PTR DISP_COLS  ;CL = # COLUMNS      

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


;
;	SUBROUTINE TO SEND CHARACTER TO PRINTER
;
OUT_TO_PRINTER PROC NEAR
        PUSH    DX                     ;SAVE (DX)
        XOR     DX,DX
        XOR     AH,AH
        INT     017H                   ;PRINT CHARACTER IN (AL)
        POP     DX                     ;RECOVER (DX)
        TEST    AH,029H
        JZ      H5                     ;JUMP IF NO ERROR
        MOV     PRINT_STAT,0FFH        ;SET ERROR FLAG
H5:     RET                                            
OUT_TO_PRINTER ENDP

page

;*********************Please note this**********************************
;
;	The size of this module must be 180h
;

empty_space	db	248 dup (0)

end_of_int5	label	byte

	if	(end_of_int5 - start_of_int5)
;	error
	endif

eproma	ends
	end

