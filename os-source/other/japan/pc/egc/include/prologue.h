;	prologue.h	
;	standard prologue for c86 assembly(small model)

;	DEFINE ARGUMENT BASE RELATIVE FROM BP

@AB	EQU	4

_TEXT   SEGMENT BYTE PUBLIC 'CODE'
_TEXT	ENDS
_DATA   SEGMENT WORD PUBLIC 'DATA'
_DATA	ENDS
CONST   SEGMENT WORD PUBLIC 'CONST'
CONST   ENDS
_BSS    SEGMENT WORD PUBLIC 'BSS'
_BSS    ENDS
STACK	SEGMENT PARA STACK 'STACK'
STACK	ENDS
DGROUP	GROUP	_DATA,CONST,_BSS,STACK

_TEXT	SEGMENT 
	ASSUME	CS:_TEXT,DS:DGROUP

;	END OF PROLOGUE.h
