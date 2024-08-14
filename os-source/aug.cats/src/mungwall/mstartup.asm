**      $Id: mstartup.asm,v 37.71 1993/10/02 03:50:15 Vic rel $
**
**      Mungwall startup code.
**
**      (C) Copyright 1990 Commodore-Amiga, Inc.
**          All Rights Reserved
**
;;  $Log: mstartup.asm,v $
*  Revision 37.71  1993/10/02  03:50:15  Vic
*  Changed - the way the length of the task/process name is calculated.
*            There was a slight bug if the name had no 'path' in it and
*            was not terminated with a null which could cause problems
*            if some arguments had originally been given to the program
*
*  Revision 37.70  1993/10/01  21:25:36  Vic
*  Added   - RCS revision log
*           also forcing the revision up to current mungwall
*

**  37.50 - add reg preserves in GetCallerName - NAMETAG OK on 68000 (cas)
**  37.51 - add AvailMem wedge to lowe MEMF_LARGEST report
**  37.52 - Added check for not a BPTR (not longword aligned), and GetCC
**  37.53 - Fix 68000 compat - get rid of .l checks in taskname stuff
**	    Fix checks for name not bptr, etc.
**  37.54 - Moved Write()'s out of initial Forbid()
**  37.56 - copy tailpathed task or command name to passed buffer, null term there
**  37.59 - add ShowPC option
**  37.60 - fix ShowPC to use TypeOfMem
**  37.61 - add once-allocated Keeper port and secondary KeeperKey cookie
**		INFOSIZE increased by 8 to 48
**  37.62 - add ROLLFILL default (may be overridden with FILLCHAR option)
**  37.63 - add SHOWHUNK and NOSHOWHUNK
**  37.64 - add SegTracker support
**  37.66 - TimeStamp support
**  37.68 - some rearrangement
**

	INCLUDE "exec/types.i"
	INCLUDE "exec/nodes.i"
	INCLUDE "exec/execbase.i"
	INCLUDE "exec/resident.i"
	INCLUDE "exec/interrupts.i"
	INCLUDE "exec/ables.i"
	INCLUDE "hardware/cia.i"
	INCLUDE "dos/dosextens.i"
	INCLUDE "mungwall_rev.i"
	section code

	XREF	_new_AllocMem
	XREF	_new_FreeMem
	XREF	_new_AvailMem
	XREF	_intena
	XREF	_startup
	XREF    _FindLayers
	XREF    _HandleSignal
	XREF    _MakeTask
	XREF    _CreateTask
	XREF	_LVOAllocMem
	XREF	_LVOFreeMem
	XREF	_LVOAvailMem
	XREF	_LVOSetFunction
	XREF	_LVOEnable
	XREF	_LVODisable
	XREF    _LVOForbid
	XREF    _LVOPermit
	XREF	_LVOOpenDevice
	XREF	_LVOCloseDevice

; ---
	XDEF	_SysBase
	XDEF	_grab_em
	XDEF	_free_em
	XDEF	_old_AllocMem
	XDEF	_old_FreeMem
	XDEF	_old_AvailMem
	XDEF	_PreSize
	XDEF	_PostSize
	XDEF	_FillChar
	XDEF	_Snoop
	XDEF	_ShowStack
	XDEF	_NameTag
	XDEF	_ShowFail
	XDEF	_ShowPC
	XDEF	_ShowHunk
	XDEF	_TimeStamp
	XDEF	_RollFill
	XDEF	_KeeperKey
	XDEF	_romtag
	XDEF    _LayersStart
	XDEF    _LayersEnd
	XDEF    _ErrorWait
	XDEF    _Taskname
;	XDEF    _StackPtr
	XDEF    _MungPort
	XDEF    _Except
	XDEF    _ConfigInfo
	XDEF    _VerTitle
	XDEF	_PreMunging
	XDEF	_PostMunging
	XDEF	_InitMunging
	XDEF	_GetCallerName
	XDEF	_TimerBase

Entry:
	movem.l	d2-d7/a2-a6,-(sp)
	move.l	a0,-(a7)	; save args

	;-- may be executed or resident
	move.l	$4,a6		; a6 = execbase already if resident, else
				; I'm a process and can trash it.
	move.l	a6,_SysBase

	;--- if not a process (started from reboot?) just wedge in
	move.l	ThisTask(a6),a0

	cmp.b	#NT_PROCESS,LN_TYPE(a0)
	bne.s	reinstall

	;-- else call startup code in C (ptr to args on stack)
	jsr	_startup

	tst.l	d0
	bra	all_done


reinstall:
        move.l  #8000,-(sp)           ; stack for task
        move.l   #_HandleSignal,-(sp) ; initPC
        move.l  #0,-(sp)              ; priority
        pea     mname                 ; task name
        jsr     _CreateTask
        addq.l  #8,sp
        addq.l  #8,sp
        tst.l   d0
        beq     all_done
	bsr.s 	_grab_em
        bra     all_done

_grab_em:
	move.l	a6,-(sp)
	move.l	$4,a6

	jsr	opentimer

	jsr     _FindLayers

	jsr	_LVODisable(a6)

	lea	newAllocMem(pc),a0
	move.l	a0,d0
	move.l	#_LVOAllocMem,a0
	move.l	a6,a1
	jsr	_LVOSetFunction(A6)
	move.l	d0,_old_AllocMem

	lea	newFreeMem(pc),a0
	move.l	a0,d0
	move.l	#_LVOFreeMem,a0
	move.l	a6,a1
	jsr	_LVOSetFunction(A6)
	move.l	d0,_old_FreeMem

	lea	newAvailMem(pc),a0
	move.l	a0,d0
	move.l	#_LVOAvailMem,a0
	move.l	a6,a1
	jsr	_LVOSetFunction(A6)
	move.l	d0,_old_AvailMem

	jsr	_LVOEnable(a6)

	move.l	(sp)+,a6
	rts


_free_em:
	move.l	a6,-(sp)
	move.l	$4,a6

	jsr	_LVODisable(a6)

	move.l	_old_AllocMem(pc),a0
	move.l	a0,d0
	move.l	#_LVOAllocMem,a0
	move.l	a6,a1
	jsr	_LVOSetFunction(A6)
	move.l	d0,_ret_AllocMem

	move.l	_old_FreeMem(pc),a0
	move.l	a0,d0
	move.l	#_LVOFreeMem,a0
	move.l	a6,a1
	jsr	_LVOSetFunction(A6)
	move.l	d0,_ret_FreeMem

	move.l	_old_AvailMem(pc),a0
	move.l	a0,d0
	move.l	#_LVOAvailMem,a0
	move.l	a6,a1
	jsr	_LVOSetFunction(A6)
	move.l	d0,_ret_AvailMem

* Check if we can exit
	move.l	_ret_AvailMem,d0
	cmp.l	#newAvailMem,d0
	bne.s	nofree
	move.l	_ret_FreeMem,d0
	cmp.l	#newFreeMem,d0
	bne.s	nofree
	move.l	_ret_AllocMem,d0
	cmp.l	#newAllocMem,d0
	bne.s	nofree

	bra.s	freeok

nofree:
	* we didn't get back our wedges, so we'll restore what was in there
	move.l	_ret_AllocMem,a0
	move.l	a0,d0
	move.l	#_LVOAllocMem,a0
	move.l	a6,a1
	jsr	_LVOSetFunction(A6)

	move.l	_ret_FreeMem,a0
	move.l	a0,d0
	move.l	#_LVOFreeMem,a0
	move.l	a6,a1
	jsr	_LVOSetFunction(A6)

	move.l	_ret_AvailMem,a0
	move.l	a0,d0
	move.l	#_LVOAvailMem,a0
	move.l	a6,a1
	jsr	_LVOSetFunction(A6)
	bra.s	freenotok	; failure

freeok:
	jsr	_LVOEnable(a6)
	jsr	closetimer
	move.l	#1,d0		; success
	move.l	(sp)+,a6
	rts

freenotok:
	jsr	_LVOEnable(a6)
	move.l	#0,d0		; failure
	move.l	(sp)+,a6
	rts

all_done:
	addq.l	#4,a7
	movem.l	(sp)+,d2-d7/a2-a6
	moveq	#0,d0
	rts

newAllocMem:
        jsr     _LVOForbid(a6)
        movem.l	a2,-(sp)
        lea	4(sp),a2
; _StackPtr
        jsr     _new_AllocMem
        movem.l (sp)+,a2
        jmp     _LVOPermit(a6)

newFreeMem:
        jsr     _LVOForbid(a6)
        movem.l	a2,-(sp)
        lea	4(sp),a2
; _StackPtr
        jsr     _new_FreeMem
        movem.l	(sp)+,a2
        jmp     _LVOPermit(a6)

newAvailMem:
        jsr     _LVOForbid(a6)
        movem.l	a2,-(sp)
        lea		4(sp),a2
; _StackPtr
        jsr     _new_AvailMem
        movem.l	(sp)+,a2
        jmp     _LVOPermit(a6)


_InitMunging:
		move.l #$ABADCAFE,a0
		bra.s Mung

_PreMunging:
		move.l #$DEADF00D,a0
		bra.s Mung

_PostMunging:
		move.l #$DEADBEEF,a0
Mung:
		;   A0=MungValue
		;   A1=Base Address
		;   D0=raw size
		;
		move.l	a1,d1
		beq.s	BadCall
		tst.l	d0
		beq.s	BadCall		; zero size
		move.l	a0,d1		; mung value to d1
		addq.l	#7,d0
		lsr.l	#3,d0
		beq.s	BadCall
		cmpi.l	#16,d0		; go mingo for <128 byte mung
		bmi.s	mingo

mongo:
		move.l	d0,a0		; save length before additional shifts
		lsr.l	#1,d0		; we'll do 16 bytes at a time
		bra.s	inloop2
loop2:		move.l	d1,(a1)+
		move.l	d1,(a1)+
		move.l	d1,(a1)+
		move.l	d1,(a1)+
inloop2:	dbra	d0,loop2
		sub.l	#$10000,d0
		bpl.s	loop2

		move.l	a0,d0		; get back size before shift
		andi.l	#$1,d0
		beq.s	MungExit	; drop thru munges leftover bytes

mingo:
		bra.s	inloop
loop:		move.l	d1,(a1)+
		move.l	d1,(a1)+
inloop:		dbra	d0,loop

*		sub.l	#$10000,d0	; no longer needed because
*		bpl.s	loop		; we only come here for <128 bytes

MungExit:
BadCall:
		rts



* passed a buffer in a0, copies tailpathed task or command name there
* and null terminates it, returning same buffer pointer
* Max length 32 including null

		XREF	_LVOGetCC
_GetCallerName
		movem.l	d1/a0/a1/a3/a6,-(sp)
		move.l	a0,a3			;name buffer to a3
		movea.l	4,a6

*    move.b  1,d1   ; for debugging

		jsr	_LVOGetCC(a6)		;In supervisor mode ?
		andi.w  #$2000,d0
 		beq.s	notsuper		;nope
		lea.l	supername(PC),a0	;yes
		move.l	#0,d1			;clear BSTR length/flag
		bra	gotdone			;done

notsuper
		move.l	#0,d1			;clear BSTR length/flag
     		movea.l	ThisTask(a6),a1         ;ExecBase this task
     		cmpi.b	#NT_PROCESS,LN_TYPE(a1) ;process ?
     		bne.s	justtask                ;no
     		move.l	pr_CLI(a1),d0           ;cli process ?
     		beq.s	justtask                ;no
     		lsl.l	#2,d0                   ;CLI bptr to ptr
     		move.l	d0,a1                   ;cli ptr in a1
     		move.l	cli_CommandName(a1),d0  ;is a command running ?
 		beq.s	justtask                ;no

*    move.b  2,d1  ;for debugging

		move.l  d0,d1			;command name to d1
		andi.l	#$C0000000,d1		;is too high to be BPTR ?
		beq.s	okbptr			;yes
badbptr:
		moveq.l	#0,d1			;clear d1 BSTR length/flag
		lea.l	notbptr(pc),a0		;no - print bad BPTR msg
		bra	gotdone			;   and leave
okbptr:
		lsl.l	#2,d0                   ;CommandName bptr to ptr
		movea.l	d0,a1                   ;ptr command name bstr in a1
		moveq.l #0,d0			;clear out d0
		move.b	(a1)+,d0                ;len to d0 & bump a1 to 1st char
		beq.s	justtask		;name is empty

*    move.b  3,d1;  for debugging


* BSTR length is in d0, pointer first char in a1

		move.l	d0,d1			;save BSTR length
		bra.s	tailpath2
tailpath:
		cmpi.b	#'/',0(a1,d0.w)
		beq.s	tailpath3
		cmpi.b	#':',0(a1,d0.w)
		beq.s	tailpath3
tailpath2:
		dbra.s	d0,tailpath
*
*		if we fall through, d0 == -1 so we need to correct it or
*		our length will be off by one.  The name copy routine will
*		properly stop if the name is null terminated, but at least
*		MetaScope doesn't always have process names terminated thusly
*		This _ONLY_ shows up if the length of the name is congruent
*		to 3 modulo 4.
*
tailpath3:
		addq.w	#1,d0			;result 0 if no / or :
						;using .w is 'safe' as we know
						;that lengths are limited to 32
						;and we can't allow overflow into
						;the hi-order word or we'll move
						;a1 WAY too far.
		adda.l	d0,a1			;tailpath

tailpath4:
		move.l	a1,a0			;commandname to a0
		sub.l   d0,d1			;subtract path size from d1
		andi.l	#$1F,d1			;limit to 31 chars
		bra.s   gotname

justtask:
		movea.l	ThisTask(a6),a0
		movea.l	LN_NAME(a0),a0          ;show task/process name
		moveq.l	#0,d1			;flag this is a task name


* expects name, as determined so far, in a0
* if was a bstr name, d1 contains length for later use, else 0
* need not be null terminated for this

gotname:

*    move.b  4,d1;  for debugging

		cmpi.l	#0,a0			; null name
		bne.s	notnull
		lea.l	nullname(pc),a0		; if null, supply <null> name
		bra.s	gotdone

notnull:
		move.l  a0,d0			; name held in a0
		btst.b	#0,d0
		bne.s	isodd			; is odd
		move.l	(a0),a1			; name not odd, put 4 bytes in a1
		bra.s	oddeven
isodd:
		move.b	0(a0),d0
		lsl.l	#8,d0
		or.b	1(a0),d0
		lsl.l	#8,d0
		or.b	2(a0),d0
		lsl.l	#8,d0
		or.b	3(a0),d0
		movea.l	d0,a1
oddeven:
		cmpi.l	#$DEADBEEF,a1		; if points to beef, show this
		bne.s	notdead1
		lea.l	deadname(pc),a0
		bra.s	gotdone
notdead1:
		cmpi.l	#$ADBEEFDE,a1		; show BPTR's to deadbeef
		bne.s	gotdone
		lea.l	adbename(pc),a0
gotdone:
		move.l	d1,d0			; bstr length or 0 (not bstr)
		beq.s	gotdone1		: branch if not BSTR
		andi.l	#$1F,d0			; limit max length to 31
		bsr.s	bnamecpy		; copy name
		bra.s	gotdone2
gotdone1:
		moveq.l	#31,d0			; up to 31 chars
		bsr.s	bnamecpy		; copy name
gotdone2:
		move.l	a3,d0			; name buffer
		movem.l	(sp)+,d1/a0/a1/a3/a6
		rts

* bnamecpy
* copy d0 or until null characters from a0 to a3, and null terminate
bnamecpy:
		movem.l	d0/a0/a3,-(sp)
		subq.l	#1,d0
		bmi.s	bnamecpyn	; was length of 0
bnamecpy0
		move.b	(a0)+,(a3)+
		beq.s	bnamecpy1	; name was shorter than 32 and terminated
		dbra.s	d0,bnamecpy0
bnamecpyn
		move.b	#0,(a3)		; name longer than 31 - null terminate
bnamecpy1
		movem.l	(sp)+,d0/a0/a3
		rts

opentimer:
            	movem.l	d0-d1/a0-a1/a6,-(sp)  ;Save the registers
	    	lea.l   timeriob,a1
	    	move.l	#0,MN_REPLYPORT(a1)
	    	move.w	#IOTV_SIZE,MN_LENGTH(a1)
		move.b	#NT_UNKNOWN,LN_TYPE(a1)
		move.b	#0,IO_ERROR(a1)
		move.b	#0,IO_FLAGS(a1)
            	lea.l   devname,a0    		;device name
            	moveq   #UNIT_VBLANK,d0		;unit
            	moveq   #0,d1          		;flags
            	movea.l $4,a6
            	jsr     _LVOOpenDevice(a6)
            	tst.l   d0
            	bne.s   opent_exit    		;note OpenDevice 0 = success
	    	lea.l   timeriob,a1
            	move.l  IO_DEVICE(a1),_TimerBase
opent_exit
            	movem.l (sp)+,d0-d1/a0-a1/a6        ;Restore registers
	    	rts

closetimer:
            	tst.l	_TimerBase
            	beq.s	nodev
            	movem.l	d0-d1/a0-a1/a6,-(sp)  ;Save the registers
            	movea.l	$4,a6
            	lea.l	timeriob,a1
            	jsr	_LVOCloseDevice(a6)
            	movem.l (sp)+,d0-d1/a0-a1/a6        ;Restore registers
nodev
            	rts


		CNOP	0,4
;_StackPtr	dc.l	0
_SysBase	dc.l	0
_old_FreeMem:	dc.l	0
_old_AllocMem:	dc.l	0
_old_AvailMem:	dc.l	0
_ret_FreeMem:	dc.l	0
_ret_AllocMem:	dc.l	0
_ret_AvailMem:	dc.l	0
_PreSize:	dc.l	32
_PostSize:	dc.l	32
_LayersStart:   dc.l    0
_LayersEnd:     dc.l    0
_VerTitle:      dc.l    mid
_Taskname:      dc.l    0
_MungPort:      dc.l    0
_KeeperKey:	dc.l	0
_Except:        dc.w    0
_ErrorWait:     dc.w    0
_Snoop:		dc.w	0
_ShowStack:	dc.w	0
_NameTag:	dc.w	0
_ShowFail:	dc.w	0
_ShowPC:	dc.w	0
_ShowHunk:	dc.w	0
_TimeStamp:	dc.w	0
_RollFill	dc.w	1
_FillChar:	dc.b	$81
_ConfigInfo:    dc.b    0

		CNOP 0,4
_TimerBase	DC.L	0

timeriob	DS.B 	IOTV_SIZE
devname	DC.B	'timer.device',0


		CNOP 0,2
nullname:	dc.b	'<NULL NAME>',0
supername:	dc.b	'<INT/EXCEPT>',0
deadname:	dc.b	'<0xDEADBEEF>',0
adbename:	dc.b	'<0xADBEEFDE>',0
notbptr:	dc.b	'<Not BPTR>',0

	CNOP	0,4
_romtag:
	DC.W	RTC_MATCHWORD		;(50) UWORD RT_MATCHWORD
	DC.L	_romtag				;(52) APTR  RT_MATCHTAG
	DC.L	_end				;(56) APTR  RT_ENDSKIP
	DC.B	RTW_COLDSTART		;(5A) UBYTE RT_FLAGS
	DC.B	0					;(5B) UBYTE RT_VERSION
	DC.B	0					;(5C) UBYTE RT_TYPE
	DC.B	107					;(5D) BYTE  RT_PRI
	DC.L	mname				;(5E) APTR  RT_NAME
	DC.L	mid     			;(62) APTR  RT_IDSTRING
	DC.L	Entry				;(66) APTR  RT_INIT
								;(6A) LABEL RT_SIZE
mname:          DC.B    "Mungwall",0
mid:            VSTRING
mversion:       VERSTAG

_end:	; I don't care

        END

