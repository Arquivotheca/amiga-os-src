
page	60,132
include	title.inc
subttl	Intr 17h  (Printer I/O)

.xlist
include externals.inc
include equates.inc
public	print
extrn	print_time:near
extrn	parallel_table:near
.list

dataa	segment	at 40h
dataa	ends

zero	segment	at 0h
zero	ends

eproma	segment	byte public

	assume cs:eproma,ds:dataa,es:dataa

.xlist
start_of_int17	equ	$+0AAh		;increased by 40h  03/04/85
.list
page
;******************************************************************************
;
;     			PRINTER  17H
;
;     INPUT  (AH)=0  PRINT CHARACTER IN (AL)
;            (AH)=1  INITILIZE PORT
;            (AH)=2  READ STATUS
;     
;            (DX)=PRINTER# TO USE
;     OUTPUT  (AH)=STATUS
;     REGISTERS CHANGED  (AH)
;
;     0     TIMEOUT   
;     1-2   UNUSED
;     3     I/O ERROR
;     4     SELECTED
;     5     OUT OF PAPER
;     6     ACKNOWLEDGE
;     7     BUSY
;
;******************************************************************************

page

;------------------------------------------------------------------------|
;	Please note: the entry point for this module is origined at 100  |
;------------------------------------------------------------------------|

printer:

;GET PRINTER PARAMETERS

        MOV     BX,DX
        MOV     CL,byte ptr PRINT_TIME[BX]
        SHL     BX,1                   ;POINT TO TABLE
        MOV     DX,word ptr PARALLEL_TABLE[BX]  ;GET PRINTER ADDRESS

;JUMP TO APPROPRIATE ROUTINE

        OR      DX,DX			;do we have a printer here
        JZ      E9                     ;RETURN IF NO PRINTER
        CMP     AH,0 
        JZ      E1                     ;JUMP IF (AH)=0
        CMP     AH,1                   
        JZ      E4                     ;JUMP IF (AH)=1
        CMP     AH,2
        JZ      E6                     ;JUMP IF (AH)=2
        JMP     SHORT E9               ;EXIT 

;	Function code 0: AH=0   OUTPUT CHARACTER

E1:     
; ToBu (25) 29.11.88 Ver 3.4 - Wait for NOT busy before putting char into
;		         	data output reg
	
	mov	bx,ax		; save character
;	OUT     DX,AL                  ;CHARACTER TO PRINTER
        
;
;	test for printer busy. If so then make call to INT 15 for
;	device busy
;

	INC     DX                     ;MOVE TO STATUS PORT
	IN      AL,DX                  ;STATUS TO (AL)
        TEST    AL,080H                ;TEST BUSY
        JNZ     E3                     ;JUMP NOT BUSY

	mov	ah,90h			;function code for INT 15
	mov	al,0feh			;device
	int	15h			;

E3:
	mov	ch,0			;
	clc				;
	shl	cx,1			;multiply by 4
	shl	cx,1			;
	
E13:    
	push	cx			;save it
E10:
	mov	cx,0			;
E11:
	in	al,dx			;get status
	test	al,80h			;
	loopz	E11			;
	jnz	E12
	pop	cx			;check outer loop
	loop	E13			;
	and	al,0f8h			;
	or	al,1			;
	xor	al,48h			;
	mov	byte ptr [bp+bp_ax+1],al	;save it in AH for return
	jmp	E9			;and return	
page




;	|---------------------------------------------------------------|
;	|	PLEASE NOTE: DO NOT CHANGE THE LOCATION OF THIS 	|
;	|	ROUTINE. THE ORIGIN IS FIXED.  RLM   03/04/85		|
;	|---------------------------------------------------------------|

		ORG	100

PRINT 	PROC 	NEAR

        STI                            ;INTERRUPTS ON
        PUSH    AX
	call	save_all
	jmp	printer			;


;	RETURN

E9:     
	call	restore_all
	POP     AX
        IRET                           ;RETURN


E12:
        pop     cx			;fix stack

; ToBu (25) 29.11.88 Ver 3.4 
	mov	ax,bx		; recover character
	dec	dx   		; dx point to data reg
	out	dx,al
	inc	dx

        INC     DX			; dx point to control reg
        MOV     AL,0DH                
        OUT     DX,AL                    ; STROBE LOW

;give more time for STROBE - V2.56
	jmp	$+2
; ToBu (26) 29.11.88 Ver 3.4 - longer strobe signal
	jmp	$+2
	jmp	$+2
 
        DEC     AL        
        OUT     DX,AL                  ;STROBE HIGH
        SUB     DX,2                   ;MOVE TO STATUA PORT
        JMP     SHORT E6               ;JUMP TO GET STATUS

;	(AH)=1 INITILIZE

E4:     
        add     dx,2
        MOV     AL,08H
        OUT     DX,AL                  ;CLEAR PORT
        MOV     CX,2000
E5:     LOOP    E5
        MOV     AL,0CH
        OUT     DX,AL                  ;INTERRUPT OFF, NO AUTO LF   
        SUB     DX,2                   ;MOVE TO STATUS PORT
			        ;fall through in prt_status routine
;	AH=2 STATUS    

E6:
	call	prt_stat		;go get printer status
	mov	byte ptr [bp+bp_ax+1],al	;save it in ah
	jmp	E9

;
;	prt_stat: routine to read printer status. on input dx should
;		contain printer address. On output al will contain the printer
;		status.

prt_stat	proc	near
	inc	dx
	in	al,dx
	and	al,0f8h
	xor	al,48h
	ret
prt_stat	endp

	
;
PRINT ENDP

page


;*************************** Please note this ********************************
;
;	Size of this module must be 0AAh bytes
;

empty_space	db	11 dup (0)

end_of_int17	label	byte

	if	(end_of_int17 - start_of_int17)

;	error

	endif



eproma	ends
	end

