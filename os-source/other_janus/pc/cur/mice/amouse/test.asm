CSEG	SEGMENT PARA PUBLIC 'CODE'

	ASSUME	CS:CSEG,DS:CSEG
	ASSUME	SS:CSEG,ES:CSEG

	ORG	100H

ENTRY:
	push	ds
	push	es

	mov	ax,0
	int	33h

	mov	ax,12
	mov	cx,1fh
	mov	dx,offset subr
	push	cs
	pop	es
	int	33h

	mov	cs:flag,0

looop:	cmp	cs:flag,0
	jz	looop

	push	ax
	mov	al,'!'
	call	printcal
	call	printcr
	pop	ax

	mov	ax,12
	mov	cx,0
	mov	dx,0
	push	dx
	pop	es
	int	33h

	pop	es
	pop	ds
	ret

flag	db	0

dump:
	push	ax

	call	printax
	mov	ax,bx
	call	printax
	mov	ax,cx
	call	printax
	mov	ax,dx
	call	printax
	call	printcr

	pop	ax
	ret

printax:
	push	ax
	mov	al,ah
	call	printal
	pop	ax
	push	ax
	call	printal
	call	printsp
	pop	ax
	ret

printal:
	push	ax
	shr	al,1
	shr	al,1
	shr	al,1
	shr	al,1
	call	printall
	pop	ax
printall:
	and	al,0fh
	cmp	al,10
	jnc	ishex
	add	al,'0'
	jmp	printcal
ishex:	add	al,'A'-10
	jmp	printcal

printsp:
	mov	al,' '
	jmp	printcal

printcr:
	mov	al,13
	call	printcal
	mov	al,10
printcal:
	push	bx		;video
	xor	bx,bx		;video
	mov	ah,0eh		;video
	int	10h		;video
	pop	bx		;video
	ret	

subr	proc	far

	push	ds
	push	cs
	pop	ds

	call	dump
	test	ax,02h
	jz	retsub

	mov	cs:flag,1
	push	ax
	mov	al,'*'
	call	printcal
	call	printcr
	pop	ax
retsub:
	pop	ds
	ret

subr	endp

CSEG	ENDS

	END ENTRY
