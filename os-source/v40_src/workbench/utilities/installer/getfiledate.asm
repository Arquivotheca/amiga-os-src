************************************************************************
* GetFileDate.asm - gets the date of a file                            *
* Written August 1988 by Talin                                         *
* Assembly version Oct 1988 by JPearce								   *
************************************************************************

		section	getfiledate.asm,code

		include 'exec/types.i'
		include 'lat_macros.i'
		include 'exec/memory.i'
		include 'libraries/dosextens.i'


LNEW			macro
						; NEW	rn,size <,t>
						; where rn is a register
						; and t is CHIP,CLEAR or PUBLIC

			subq.w		#4,sp
			movem.l		a0/a1/a6/d0/d1,-(sp)
;			move.l		AbsExecBase,a6

			moveq		#0,d1

			move.l		#\2,d0
			CallEx		AllocMem
			move.l		d0,20(sp)
			movem.l		(sp)+,a0/a1/a6/d0/d1
			move.l		(sp)+,d0
			move.l		d0,\1
			endm

LDELETE		macro		; DELETE rn,size  (rn cannot be d0/a1/a6)

			xref		_LVOFreeMem
			movem.l		a0/a1/a6/d0/d1,-(sp)
			move.l		\1,d0
			beq.s		\@DEL1
			move.l		4,a6
			move.l		d0,a1
			move.l		#\2,d0
			jsr			_LVOFreeMem(a6)
\@DEL1		movem.l		(sp)+,a0/a1/a6/d0/d1

			endm


		xref	_DOSBase

		DECLAREL 	GetFileDate			; GetDateStamp(filename,ds_buffer)

		SaveM		a2/a3/a6/d2/d4

		moveq		#0,d4				; result = FALSE

		LNEW		a3,fib_SIZEOF		; get storage for FileInfoBlock
		tst.l		d0
		beq.s		scat1				; out of memory error

		move.l		4+20(sp),d1			; get Lock on file
		moveq		#ACCESS_READ,d2
		CallDos		Lock
		move.l		d0,a2
		tst.l		d0
		beq.s		scat2				; no such object

		move.l		a2,d1				; examine the object
		move.l		a3,d2
		Call		Examine
		tst.l		d0
		beq.s		scat3				; examine failed

		move.l		8+20(sp),a0			; copy datestamp into buffer
		lea			fib_DateStamp(a3),a1
		move.l		(a1)+,(a0)+
		move.l		(a1)+,(a0)+
		move.l		(a1)+,(a0)+
		moveq		#1,d4				; result = TRUE

scat3
		move.l		a2,d1				; free lock
		Call		UnLock
scat2
		LDELETE		a3,fib_SIZEOF		; free memory
scat1
		move.l		d4,d0				; return result
		RestoreM	a2/a3/a6/d2/d4
		rts

		end

ULONG GetFileDate(filename,ds) char *filename; struct DateStamp *ds;
{	register struct FileLock 		*fl=NULL, *Lock();
	register struct FileInfoBlock	*fib=NULL;
	register ULONG					result = FALSE,
									success;

	if (!MakeStruct(fib)) SCAT;			/* create the file info block */
	if (!(fl = Lock(filename,ACCESS_READ))) SCAT;

	if (success = Examine(fl,fib)) *ds = fib->fib_Date;	/* fill in date */
	result = TRUE;										/* return success */
ex:
	if (fl) UnLock(fl);									/* get rid of lock */	
	UnMakeStruct(fib);									/* de-alloc fib */
	return result;
}
