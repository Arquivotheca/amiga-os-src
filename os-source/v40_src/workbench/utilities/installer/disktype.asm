* Disktype.asm -- returns the id_DiskType of a DOS device

			section		code

			include		'exec/types.i'
			include		'libraries/dosextens.i'
			include		'libraries/filehandler.i'
			include		'macros.i'

			xref		InitSimplePacket
			xref		SendPacket

			xdef		DiskType,_DiskType

; register usage:
;	d2		result
;	a2		address of device node task
;	a3		saved stack pointer

; stackframe: A StandardPacket followed by an InfoData structure.

SP_OFFSET	equ			(sp_SIZEOF+3)&~3
TEMP_SIZE	equ			SP_OFFSET+id_SIZEOF+8

_DiskType:	move.l		4(sp),a0
DiskType					; (a0: dev_node)

			movem.l		d2/a2-a3/a6,-(sp)		; save registers
			moveq		#0,d2					; default result (NULL) in d2

			move.l		dn_Task(a0),d0			; get task pointer into d0
			beq.s		99$						; return NULL if not a filesystem
			move.l		d0,a2					; device task address

			move.l		sp,a3					; save stack pointer in a3

			moveq.l		#TEMP_SIZE/4,d0			; size of needed memory in longwords
1$			clr.l		-(sp)					; clear a longword down
			dbra		d0,1$					; and loop till memory buf created

			move.l		sp,d0					; now get stack pointer in d0
			addq.l		#3,d0					; round to next highest longword
			and.w		#~3,d0
			move.l		d0,sp					; and move to stack pointer

			move.l		sp,a0					; packet address in a0
			moveq.l		#ACTION_DISK_INFO,d0	; ACTION_DISK_INFO
			jsr			InitSimplePacket		; init the packet (xlib func)

			move.l		sp,d0					; get address of temp data
			add.l		#SP_OFFSET,d0			; add offset of infodata struct
			lsr.l		#2,d0					; turn into bptr (ptr >> 2)
			move.l		d0,sp_Pkt+dp_Arg1(sp)	; and store in packet

			move.l		a2,a0					; port to send packet to
			move.l		sp,a1					; address of packet to send
			jsr			SendPacket				; send the packet (xlib func)
			tst.l		sp_Pkt+dp_Res1(sp)		; if there was a non-zero result
			beq.s		98$						; then
			move.l		SP_OFFSET+id_DiskType(sp),d2 ; get the result in d2

98$			move.l		a3,sp					; restore stack pointer
99$			move.l		d2,d0					; put result in d0
			movem.l		(sp)+,d2/a2-a3/a6		; restore registers
			rts

			end
