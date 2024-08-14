*********************************************************************
* Puts.asm - like standard puts(), but uses rom RawDoFmt()          *
* Written October 1988 by Joe Pearce								*
*********************************************************************

		section	code

		include 'macros.i'

		xref	_Fprintf,_Printf

*	Fputs(handle,string)

		DECLAREA Fputs

		move.l	4(sp),d0
		move.l	8(sp),a1
Fputs
		lea		form,a0
		movem.l	d0/a0/a1,-(sp)
		jsr		_Fprintf			; Fprintf(handle,"%s\n",string)
		lea		12(sp),sp
		rts

*	Puts(string)

		DECLAREA	Puts

		move.l	4(sp),a1
Puts
		lea		form,a0
		movem.l	a0/a1,-(sp)
		jsr		_Printf				; Printf("%s\n",string)
		addq.w	#8,sp
		rts

form
		dc.b	'%s',$0a,0
		ds.w	0

		end
