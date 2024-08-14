*
* support.asm
* 
* asm -iINCLUDE: -oobj/ support.a
*
        INCLUDE "exec/types.i"
        INCLUDE "exec/nodes.i"
        INCLUDE "exec/lists.i"
        INCLUDE "exec/ports.i"
        INCLUDE "exec/libraries.i"
        INCLUDE "exec/tasks.i"
        INCLUDE "exec/memory.i"
        INCLUDE "exec/execbase.i"
        INCLUDE "dos/dos.i"
        INCLUDE "dos/dosextens.i"
        INCLUDE "dos/dos_lib.i"
        INCLUDE "dos/exall.i"

FUNCDEF	MACRO
	XREF	_LVO\1
	ENDM

        INCLUDE "exec/exec_lib.i"

	INCLUDE "libhdr.i"
	INCLUDE "dos/dos.i"

callsys macro
        CALLLIB _LVO\1
        endm
        
*
* blib, etc routines
*
BLIB	MACRO	;\1 - symbolname
	XREF	_\1
@\1	EQU	_\1
	ENDM

*DEBUG	EQU	1

DBUG	macro
	ifd	DEBUG
	movem.l	d0/d1/a0/a1,n2\@
	pea	n1\@
	jsr	_kprintf
	addq.w	#4,a7
	bra.s	n3\@
n1\@	dc.b	\1,10,0
	cnop	0,2
n2\@	ds.l	4
n3\@
	movem.l	n2\@,d0/d1/a0/a1
	endc
	endm

SYSBASE	EQU	4

	XDEF @getstring
	XDEF _getstring
	XDEF @BtoC
	XDEF _BtoC
	XDEF @CtoBstr
	XDEF _CtoBstr
	XDEF @toUpper
	XDEF _toUpper
	XDEF @cloneTagItems
	XDEF _cloneTagItems
	XDEF @packBoolTags
	XDEF _packBoolTags
	XDEF @findTagItem
	XDEF _findTagItem
	XDEF @filterTagItems
	XDEF _filterTagItems
	XDEF @freeTagItems
	XDEF _freeTagItems
	XDEF @getTagData
	XDEF _getTagData
	XDEF @mystricmp
	XDEF _mystricmp
	XDEF @mystrnicmp
	XDEF _mystrnicmp
	XDEF @cpymax
	XDEF _cpymax
	XDEF @mystrchr
	XDEF @mystrrchr
	XDEF @objact
	XDEF _objact
	XDEF @CompareDates
	XDEF _CompareDates
	XDEF @Atol
	XDEF @ExAll
	XDEF @ExAllEnd
	XDEF _ExamineFH
	XDEF _examine
	XDEF _exnext
	XDEF _info
	XDEF _parent
	XDEF _copydir
	XDEF _setprotection
	XDEF @SetOwner
	XDEF _setdate
	XDEF _deletefile
	XDEF _createdir
	XDEF _lock
	XDEF _locateobj
	XDEF _MakeLink
	XDEF _read
	XDEF _write
	XDEF _noflushseek
	XDEF _seek
	XDEF _SetFileSize
	XDEF _DupLockFromFH
	XDEF _ParentOfFH
	XDEF _UnLockRecord
	XDEF _LockRecord
	XDEF _readwrite
	XDEF _setmode
	XDEF _waitforchar
	XDEF _inhibit
	XDEF _addbuffers
	XDEF _relabel
	XDEF _Format
	XDEF _devact_str

	XREF _kprintf
	XREF @getresult2
	XREF @setresult2
	BLIB err_report
	BLIB getdevproc
	BLIB freedevproc
	BLIB ReadLink
	XREF @sendpacket
	XREF @filesystemtask
	XREF @AllocVecPub
	BLIB ErrorReport
	BLIB do_lock
	BLIB devact
	BLIB Flush
	XREF _fake_exall
	XREF @CtoB

	XDEF @mystrcat
	XREF @strcpy
	XREF @strlen
	XREF get_ubase
	XREF @dosbase
	XREF _freeVec

	XREF _LVOPackBoolTags
	XREF _LVOFindTagItem
	XREF _LVOStricmp
	XREF _LVOStrnicmp
	XREF _LVOGetTagData
	XREF _LVOToUpper
	XREF _LVOCloneTagItems
	XREF _LVOFilterTagItems
	XREF _LVOFreeTagItems
	XREF _LVODosGetString

*
* routine stub to call dos getstring function
*
@getstring:
_getstring:
	movem.l	d1/a6,-(sp)
	bsr	@dosbase
	move.l	d0,a6
	move.l	(sp),d1			; get argument back off stack! TRICKY!
	jsr	_LVODosGetString(a6)
	movem.l	(sp)+,d1/a6
	rts

*/* modifies destination ptr and len */
*void ASM
*cpymax (REG(a0) UBYTE **dest
	
@mystrcat:
	move.l	a0,-(a7)
1$:	tst.b	(a0)+
	bne.s	1$		; find end of string
	subq.l	#1,a0
	bsr	@strcpy		; add tail onto end
	move.l	(a7)+,d0
	rts

*
* char * __regargs BtoC(UBYTE *source, char *dest)
*
@BtoC:
_BtoC:
	move.l	a0,d0		; return value
	moveq	#0,d1
	move.b	(a0)+,d1	; length
	bra.s	2$

1$:	move.b  (a0)+,(a1)+
2$:	dbra	d1,1$

	clr.b	(a1)		; null terminate 
	rts

*
* BPTR __regargs CtoBstr(char *str)
*
@CtoBstr:
_CtoBstr:
	move.l	a0,a1

1$:	move.b	(a0),d1		; overlapping copy
	move.b	d0,(a0)+	; be very careful
	move.b	d1,d0
	bne.s	1$

	sub.l	a1,a0		; figure out size
	subq	#1,a0
	move.l	a0,d1
	move.b	d1,(a1)		; size byte

	move.l	a1,d0
	lsr.l	#2,d0		; return value (BPTR)
	rts


	IFD	USE_MY_STRICMP
*
* int __regargs stricmp (char * s1, char * s2)
* int __regargs strnicmp (char * s1, char * s2, long len)
* strcmp ignoring case, <0 = s1<s2, 0 = equal, >0 = s1>s2
*
@mystricmp:
_mystricmp:
	move.l	#$7fffffff,d0		; infinite length
@mystrnicmp:
_mystrnicmp:
	movem.l	d2/d3/d4,-(a7)	; d4 for check_sub use
	move.l	d0,d3		; save length
	moveq	#0,d0		; clear high 3 bytes, set equal if len=0
* FIX!!!! doesn't handle length == 0!!!!

caseloop:
	subq.l	#1,d3		; done yet?
	bmi.s	make_res	; if done, make result
	move.b	(a0)+,d1	; Deal with either string ending first
	move.b	(a1)+,d2	; must fetch both for comparison, even if d0==0
	beq.s	make_res	; exit if *s2 == 0
	move.b	d1,d0
	beq.s	make_res	; exit if *s1 == 0

	bsr.s	check_sub
	beq.s	1$
	sub.b	#'a'-'A',d1
1$:
	move.b	d2,d0
	bsr.s	check_sub
	beq.s	2$
	sub.b	#'a'-'A',d2

2$:
	cmp.b	d2,d1		; do the comparison (must
	beq.s	caseloop	; check next character (after checking size)

make_res:
	move.b	d1,d0
	sub.b	d2,d0		; generate return value
exit:
	movem.l	(a7)+,d2/d3/d4
	rts

*
* returns d0 = 0 if no sub needed, = 1 if needed (cc's set!!!!)
* can't touch d1/d2/d3, d4 is scratch, d0 has character/return
*
check_sub:
	cmp.b	#$f7,d0		; these are special
	beq.s	no
	move.w	#'za',d4	; Z/z and A/a
	bclr.b	#7,d0		; clear high bit
	beq.s	normal_char
	move.w	#'~`',d4	; Þ/þ and À/à are the highest in the upper area
				; (~ = þ, ' = à minus bit 7)
normal_char:
	cmp.b	d4,d0		; a or à(`)
	blt.s	no
	lsr.l	#8,d4		; get upper bound
	cmp.b	d4,d0		; z or þ(~)
	bgt.s	no
	moveq	#1,d0
	rts
no:	moveq	#0,d0
	rts

	ENDC
	IFND	USE_MY_STRICMP

_toUpper:
@toUpper:
	move.w	#_LVOToUpper,d1		; uses d0
	bra.s	utility_common_d1
_cloneTagItems:
@cloneTagItems:
	move.w	#_LVOCloneTagItems,d1	; uses a0
	bra.s	utility_common_d1
_packBoolTags:
@packBoolTags:
	move.w	#_LVOPackBoolTags,d1	; uses d0/a0/a1
	bra.s	utility_common_d1
_findTagItem:
@findTagItem:
	move.w	#_LVOFindTagItem,d1	; uses d0/a0
	bra.s	utility_common_d1
_filterTagItems:
@filterTagItems:
	move.w	#_LVOFilterTagItems,d1	; uses a0/a1/d0
	bra.s	utility_common_d1
_freeTagItems:
@freeTagItems:
	move.w	#_LVOFreeTagItems,d1	; uses a0
	bra.s	utility_common_d1
_mystricmp:
@mystricmp:
	move.w	#_LVOStricmp,d1		; uses a0/a1
	bra.s	utility_common_d1
_mystrnicmp:
@mystrnicmp:
	move.w	#_LVOStrnicmp,d1	; uses a0/a1/d0

utility_common_d1:
	move.l	a6,-(sp)
	bsr	get_ubase	; returns UtilityBase in a6, no other regs
				; touched
	jsr	0(a6,d1.w)	; call utility library
stricmp_cleanup:
	move.l	(sp)+,a6
	rts

_getTagData:
@getTagData:				; uses d0/d1/a0
	move.l	a6,-(sp)
	bsr	get_ubase	; returns UtilityBase in a6, no other regs
				; touched
	jsr	_LVOGetTagData(a6)	; call utility library
	bra.s	stricmp_cleanup

	ENDC

*/* modifies destination ptr and len */
*void ASM
*cpymax (REG(a0) UBYTE **dest, REG(a1) UBYTE *src,REG(a2) ULONG *len)
*{
*	while (*len && *src)
*	{
*		*((*dest)++) = *src++;
*		(*len)--;
*	}
*	**dest = '\0';
*}

@cpymax:
_cpymax:
	move.l	a3,-(a7)
	move.l	(a0),a3		; get *dest
	move.l	(a2),d1		; get *len
	beq.s 	nospace
  
cpyloop:
	subq	#1,d1		; fix off by one error -Bryce
	beq.s	terminate
	move.b	(a1)+,d0
	beq.s	terminate

	move.b	d0,(a3)+	; store character
	bra.s	cpyloop

terminate:
	; store off len and *dest, and null-terminate string
	clr.b	(a3)
	move.l	a3,(a0)
	move.l	d1,(a2)

nospace:
	move.l	(a7)+,a3
	rts

*
*  char * __regargs mystrchr(char *ptr, int sep)
*
*  find first occurance of sep, otherwise return null
*
@mystrchr:
	move.b	(a0)+,d1
	beq.s	1$
	cmp.b	d1,d0
	bne.s	@mystrchr

	;-- success!
	subq.l	#1,a0
	move.l	a0,d0
	rts

1$:	;-- failure
	moveq	#0,d0
	rts

*
*  char * __regargs mystrrchr (char *ptr, int sep)
*
*  find last occurance of sep, otherwise return null
*
@mystrrchr:
	sub.l	a1,a1		; return NULL if not found
1$:
	move.b	(a0)+,d1
	beq.s	2$
	cmp.b	d1,d0
	bne.s	1$

	;-- success!
	lea	-1(a0),a1	; remember ptr
	bra.s	1$

2$:	;-- end
	move.l	a1,d0		; will be NULL is we didn't find any
	rts

*
* Downcoded version of objact from blib.  Downcoded because it is CRITICAL
* to dos efficency, since many dos calls go through this routine.
* C code version MUST be maintained!
*
*
*
* LONG
* objact (REG(d1) char *name,
* 	  REG(d2) LONG action,
*	  REG(d3) LONG *arg)
*{
*	char *bname,*newname;
*	LONG res,links = MAX_LINKS;
*	struct DevProc *dp = NULL;
*	LONG len;
*
*	/* asm code uses stack for name, and truncates at 256!!!! FIX!!!! */
*	/* extra +1 is for null-terminated bstr */
*	len = strlen(name)+1+1;
*	if (len > 256)
*	{
*		setresult2(ERROR_LINE_TOO_LONG);
*		return 0;
*	}
*	
*	bname = getvec((len+3) >> 2); /* longs */
*	if (!bname)
*	{
*		setresult2(ERROR_NO_FREE_STORE);
*		return 0;
*	}
*
*	/* since we have to pass BCPL strings */
*	CtoB(name,(CPTR) bname);
*	bname[*bname+1] = '\0';		/* null-terminate */
*
*	while (1) {
*		dp = getdevproc(bname+1,dp);
*		if (!dp)
*		{
*			/* getdevproc freed dp for us */
*			freevec(bname);
*			return 0;
*		}
*
*		res = ((action == ACTION_SET_COMMENT ||
*		        action == ACTION_SET_PROTECT ||
*		        action == ACTION_SET_OWNER ||
*			action == ACTION_SET_DATE) ?
*		       sendpkt4(dp->dvp_Port,action, 0 ,dp->dvp_Lock,
*			        TOBPTR(bname),arg[0]) :
*		       sendpkt4(dp->dvp_Port,action, dp->dvp_Lock,
*			        TOBPTR(bname),arg[0],arg[1]));
*
*		if (!res)
*		{
*			switch (getresult2()) {
*			case ERROR_OBJECT_NOT_FOUND:
*				if (dvp->dvp_Flags & DVPF_ASSIGN)
*					continue; /* makes it get next assign */
*				goto exit;
*			case ERROR_IS_SOFT_LINK:
*				if (--links >= 0)
*				{
*				  /* for the time being, max 255 chars */
*				  /* Bstr is allocated in 256 bytes */
*				  /* we need 257 - 1 len, 255 str, 1 null */
*				  newname = getvec(260/4);
*				  if (!newname)
*				    goto exit;
*
*				  BtoCstr((CPTR) bname);
*				  if (ReadLink(dp->dvp_Lock,bname,
*				 	        newname,256) >= 0)
*				  {
*				    freevec(bname);
*				    bname = newname;
*				    CtoBstr(bname);
*				    bname[*bname+1] = '\0';
*				    goto loop;
*				  } else {
*				    /* we can't deal with >255 chars */
*				    /* should still be null-terminated ok */
*				    CtoBstr(bname);
*				    freevec(newname);
*				    setresult2(ERROR_INVALID_COMPONENT_NAME);
*				  }
*				} else {
*				  setresult2(ERROR_TOO_MANY_LEVELS);
*				}
*				goto exit;
*			}
*		}
*
*		if (res || err_report(REPORT_LOCK,dp->dvp_Lock,dp->dvp_Port))
*		{
*exit:			freedevproc(dp);
*			freevec(bname);
*			return res;
*		}
* loop:
*		freedevproc(dp);
*		dp = NULL;
*	} /* while 1 */
*	/*NOTREACHED*/
*}
*

@objact:
_objact:
	movem.l	d2-d7/a2-a4,-(a7)
	link	a5,#-2		; placeholder for stack + 2 bytes for links
	moveq	#MAX_LINKS,d0
	move.w	d0,-2(a5)	; links = MAX_LINKS

	move.l	d1,a2		; save for getdevproc() call
	bsr	name_to_stack	; put name on stack, return it in a2
				; hits a bunch of regs (preserves d2/d3)
	;-- now loop, trying to do action until success or CANCEL
	; register assignments in loop:
	;    a2 - bname		- PERM
	;    a3 - arg		- PERM
	;    a4 - dp
	;    a5 - holds link value - PERM
	;    a6 - free (careful! if we start assuming dosbase in a6!)
	;    d0 - scratch
	;    d1 - dp->dvp_Port
	;    d2 - action	(sometimes saved elsewhere)
	;    d3 - dp->dvp_Lock
	;    d4 - TOBPTR(bname)
	;    d5 - arg[0] (usually)
	;    d6 - arg[1] (usually) otherwise newname
	;    d7 - TOBPTR(bname)	- PERM
	;
	;    -2(a5) - links (word)

	move.l	d3,a3		; save
	move.l	a2,d7		; save (bname)
	lsr.l	#2,d7		; TOBPTR(bname)
	sub.l	a4,a4		; dp = NULL
forever:
	move.l	a2,d1		; bname /* null-terminated bstr */
	addq.l	#1,d1		;      +1
	move.l	d2,d3		; save action
	move.l	a4,d2		; dp
	bsr	@getdevproc	; this handles multi-dirs
	move.l	d0,a4		; new dp or NULL
	tst.l	d0
	bne.s	got_task

	;-- can't find task - exit (dp was freed for us)
	; moveq	#0,d0		; must be
	bra	commonexit

got_task:
	move.l	dvp_Port(a4),d1	; dp->dvp_Port - no need to shift
	move.l	d3,d2		; d3 is holding action
	move.l	dvp_Lock(a4),d3	; dp->dvp_Lock - may have to go to d4
	move.l	d7,d4		; TOBPTR(bname) - may have to go to d5
	move.l	(a3),d5		; arg[0] - may have to go to d6
	move.l	4(a3),d6	; arg[1] - usually unused

	cmp.l	#ACTION_SET_COMMENT,d2
	beq.s	shift_args
	cmp.l	#ACTION_SET_PROTECT,d2
	beq.s	shift_args
	cmp.l	#ACTION_SET_OWNER,d2
	beq.s	shift_args
	cmp.l	#ACTION_SET_DATE,d2
	bne.s	sendit

shift_args:
	;-- have to shift d3-d5 to d4-d6 to put 0 in d3
	move.l	d5,d6
	move.l	d4,d5
	move.l	d3,d4
	moveq	#0,d3

sendit:
	;-- send the packet - all regs set up
	bsr	@sendpacket
	move.l	d2,d5		; save action
	move.l	dvp_Lock(a4),d2	; dp->dvp_Lock into d2...
	moveq	#1,d3		; so we won't loop again after freedevproc
	move.l	d0,d4		; save result code
	bne.s	freedev		; ends up getting result from d4

	; is this a multiple assign, and should we follow it?
	bsr	@getresult2
	cmp.l	#ERROR_OBJECT_NOT_FOUND,d0
	bne.s	check_links
	btst.b	#DVPB_ASSIGN,dvp_Flags+3(a4)
	;- note that was +3 because btst on mem is byte-wide!!!
	bne.s	continue	; branch if multiple assign
	bra.s	real_error	; no more dirs to check - report it

	;-- Here is the soft link handling code!
check_links:
	cmp.l	#ERROR_IS_SOFT_LINK,d0
	bne.s	real_error
	subq.w	#1,-2(a5)	; decrement link count
	bmi.s	too_many_links

	;-- ok, now read the link.  We allocate 256 bytes, read it, then copy
	;-- it back to our stack.  Ugly, but avoids another 256byte stack hit,
	;-- or the allocmem always used by the C version above.
	moveq	#260/4,d0
	asl.l	#2,d0		; = 260
	bsr	@AllocVecPub
	move.l	d0,d6		; save newname
	beq.s	real_error	; couldn't get memory - punt

	;-- now do the readlink
	move.l	dvp_Port(a4),d1	; dp->dvp_Port - no need to shift
				; dp->dvp_Lock is in d2 already
	move.l	d7,d3		; TOBPTR(bname)
	asl.l	#2,d3		; bname
	addq.l	#1,d3		; bname + 1 (MUST be valid cstr!)

				; newname is already in d0 (note: d0!)
	lea	256,a0		; size of newname (note: a0!)
	bsr	@ReadLink
	move.l	d6,d1		; newname (for both link_err and name_...)
	tst.l	d0
	bmi.s	link_err	; either read err or not enough space

	;-- now copy onto stack over old bname
	;-- orig stack ptr was -2(a5)
	lea	-2(a5),a7	; drop old bname (stack is now as after link)
	bsr.s	name_to_stack	; hits a bunch of regs - ret in a2
				; we don;t care that d4 gets trashed because
				; we loop (it preserves d6)
	move.l	a2,d7
	lsr.l	#2,d7		; tobptr(new bname);

	; free newname now that string has been copied onto stack
	move.l	d6,a1		; free before overwriting!
	bsr	_freeVec	; a1
	moveq	#0,d3		; so freedev doesn't dump us out
	bra.s	freedev		; loop back and start again!

	;-- link errors come to here
link_err:
	;-- d1 already has newname in it.
	move.l	d1,a1
	bsr	_freeVec	; a1
	move.l	#ERROR_INVALID_COMPONENT_NAME,d1
	bra.s	link_err_comm

too_many_links:
	move.l	#ERROR_TOO_MANY_LEVELS,d1
link_err_comm:
	bsr	@setresult2
	; fall thru

real_error:
	; error - put up requester, repeat on retry, give up on cancel
	; d2 already set to dvp_Lock
	moveq	#REPORT_LOCK,d1
	move.l	dvp_Port(a4),d3	; dp->dvp_Port
	bsr	@err_report
	move.l	d0,d3		; save result - both paths freedevproc

	; free dp before looping
freedev:			; comes here with d3=1 if we succeed
	move.l	a4,d1
	bsr	@freedevproc
	sub.l	a4,a4		; make sure dp = NULL before looping after error

	; now test err_report result
	tst.l	d3
	bne.s	err_exit

continue:
	move.l	d5,d2		; restore action to d2
	bra	forever

err_exit:
	move.l  d4,d0		; return res
commonexit:			; return value in d0
	unlk	a5		; free stack
	movem.l	(a7)+,d2-d7/a2-a4
	rts

	;-- this takes a ptr to a name in d1, and puts it on the stack as
	;-- a bstr (null-terminated).  Uses a0/a1/a2/d1/d4/d7/a7
	;-- returns ptr to string in a2. (same as a7)
	;-- plays funny games with return addr.
	;-- only use from routines with LINK instructions!
	;-- truncates names to 255 characters!!! FIX!!!!

name_to_stack:
	move.l	d1,a0		; first get length of name
	move.l	a0,a1		; (source for copy later)
1$	tst.b	(a0)+
	bne.s	1$
	sub.l	d1,a0		; includes null!
	move.l	a0,d1		; len

	;-- truncate to 256
	move.l	#255,d0
	cmp.l	d0,d1
	ble.s	2$		; size ok
	move.l	d0,d1		; set size == 255!

2$	move.l	(a7)+,d7	; save ret addr - stack must be clean!
	move.l	d1,d4		; save for copy
	subq.w	#1,d4		;  length without null (max 16 bit len)

	;-- allocate string space off stack
	;-- may need to do a real getvec for links?
	;-- NOTE! trick! not.l dn == neg.l dn; subq.l #1,dn !!!!!!!
	not.l	d1		; with null included
				; 1 byte for null-termination as well as BSTR
	add.l	a7,d1		; take space off - still need to align
	and.w	#$fffc,d1	; make LONGWORD aligned! (moves down to next lw)
	move.l	d1,a7		; our modified stack ptr
	move.l	a7,a2		;  return value
	move.l	a7,a0		;  dest for copy

	;-- copy cstr to bstr on stack (null-terminated BSTR!)
	move.b	d4,(a0)+	; length without null
				; fall thru - we WANT it to be null-terminated!
3$	move.b	(a1)+,(a0)+
	dbra	d4,3$

	move.l	d7,a1		;\
	jmp	(a1)		;- equivalent to move.l a1,-(a7); rts.


******* dos.library/CompareDates ************
*
*   NAME
*	CompareDates -- Compares two datestamps
*
*   SYNOPSIS
*	result = CompareDates(date1,date2)
*	D0                     D1     D2
*
*	LONG CompareDates(struct DateStamp *,struct DateStamp *)
*
*   FUNCTION
*	Compares two times for relative magnitide.  <0 is returned if date1 is
*	later than date2, 0 if they are equal, or >0 if date2 is later than
*	date1.
*
*   INPUTS
*	date1, date2 - DateStamps to compare
*
*   RESULT
*	result       -  <0, 0, or >0 based on comparison of two date stamps
*
*   BUGS
*	returns the result backwards!!!!!!!!  Stuck with it.
*

@CompareDates:
_CompareDates:
	move.l	d1,a0
	move.l	d2,a1

	; first days
	move.l	#2,d1		; count, number of fields (3 total)
check_loop:
	move.l	(a1)+,d0
	sub.l	(a0)+,d0
	bne.s	check_done	; dates not equal
	dbra	d1,check_loop
check_done:
	rts



******* dos.library/Atol ************
*
*   NAME
*	Atol -- string to long value (decimal)
*
*   SYNOPSIS
*	characters = Atol(string,value)
*	D0                 D1     D2
*
*	LONG Atol(UBYTE *,LONG *)
*
*   FUNCTION
*	Converts decimal string into LONG value.  Returns number of characters
*	converted.  Skips over leading spaces & tabs (included in count).  If no
*	decimal digits are found (after skipping leading spaces & tabs), Atol
*	returns -1 for characters converted, and puts 0 into value.
*
*   INPUTS
*	string - Input string.
*	value  - Pointer to long value.  Set to 0 if no digits are converted.
*
*   RESULT
*	result - Number of characters converted or -1.
*

; FIX!!!!!!!!!  overflows should go to the instruction AFTER done:!

@Atol:
	movem.l	d2-d6,-(a7)
	move.l	d1,a0		; initial setup
	move.l	d2,a1		; pointer to value
	moveq	#0,d6		; count of characters read
	moveq	#0,d1		; value
	moveq	#10,d2		; constant for multiply
	moveq	#0,d3		; negative flag
	moveq	#0,d4		; clear upper 24 bits of d4
	moveq	#0,d5		; got digit flag

	; skip white space
loop:	move.b	(a0)+,d4	; get a char
	addq.l	#1,d6
	cmp.b	#' ',d4
	beq.s	loop
	cmp.b	#9,d4		; tab
	beq.s	loop

	; non-white space - check for minus
	cmp.b	#'-',d4
	bne.s	read_digits
	moveq	#1,d3		; negate answer

read_loop:
	move.b	(a0)+,d4	; read another character
	addq.l	#1,d6

	; read digits in - character in d4
read_digits:
	sub.b	#'0',d4
	bmi.s	done
	cmp.b	d2,d4		; d2 has 10 in it
	bge.s	done		; (d4-d2) >= 0
	move.l	d1,d0		; used to call @multiply here
	asl.l	#3,d1		; d1 *= 8
	bvs.s	done		; if overflow, exit
	add.l	d0,d1		; d1 *= 9
	bvs.s	done
	add.l	d0,d1		; d1 *= 10;
	bvs.s	done		; d1 = d1 * d2 (num *= 10) - kills d0
	add.l	d4,d1
	moveq	#1,d5		; flag that we got a digit
	bra.s	read_loop

	; we hit a non-digit
done:	subq.l	#1,d6		; don't count this non-digit
	tst.l	d5
	bne.s	ok		; we got digits
	moveq	#-1,d6		; we didn't get digits

ok:	tst.l	d3		; should we negate?
	beq.s	plus
	neg.l	d1
plus:
	move.l	d1,(a1)		; save result
	move.l	d6,d0
	movem.l	(a7)+,d2-d6
	rts

*
* LONG ASM
* exall (REG(d1) BPTR lock, REG(d2) struct ExAllData *buffer, REG(d3) LONG size,
*        REG(d4) LONG data, REG(d5) struct ExAllControl *control)
*
*
@ExAll:
	move.l	#ACTION_EXAMINE_ALL,d0
	; fall through

* d0 is action (ACTION_EXAMINE_ALL or ACTION_EXAMINE_ALL_END)
* d1-d5: see exall
do_exall:
	movem.l	d1-d7/a2-a4,-(a7)		; TRICKY! see below
	move.l	d0,d2			; put action in right register
	move.l	d5,a3			; ExAllControl
	lea	nomatch_string(pc),a4	; keep this around
try_again:
	;-- get pointer to lock structure
	move.l	(a7),d0			; TRICKY! (was in D1)
	lsl.l	#2,d0
	move.l	d0,a2			; cptr to FileLock

	bsr	@filesystemtask

	;-- handle null locks! (d0 = filesystask)
	move.l	a2,d1
	beq.s	got_fs_task
	move.l	fl_Task(a2),d0		; we have a lock

got_fs_task:
	;-- handle null tasks (NIL:!)
	move.l	d0,d1
	beq.s	exall_exit

	; d2 is action - can change if it's ACTION_EXAMINE_ALL_END!
	movem.l	(a7),d3-d7		; TRICKY! counts on stack frame!
	bsr	@sendpacket
	tst.l	d0
	bne.s	exall_exit

	;-- was this an unsupported packet error?
	bsr	@getresult2
	cmp.l	#ERROR_ACTION_NOT_KNOWN,d0
	bne.s	exall_error

	cmp.l	#ACTION_EXAMINE_ALL,d2
	bne.s	exall_end_unknown	; ExAllEnd: try doing an ExAll
	cmp.l	eac_MatchString(a3),a4	; are we doing an ExAll for ExAllEnd?
	beq.s	exall_exit		; yes, no need to do anything more

	;-- not known - simulate - a7 has parameters on it in right order
	;-- d3-d7 have our params
	movem.l	d3-d7,-(a7)		; since rtn may modify them
	bsr	_fake_exall		; __stdargs rtn
	lea	20(a7),a7		; drop parameters
	tst.l	d0
	bne.s	exall_exit

	;-- failure - display message, then loop or exit with error
exall_error:
	move.l	(a7),d3			; lock - TRICKY!
	move.l	d2,-(a7)		; save action!
	moveq	#REPORT_LOCK,d2
	move.l	a2,d4			; task
	bsr	@getresult2
	move.l	d0,d1			; error code
	bsr	@ErrorReport
	move.l	(a7)+,d2		; restore action
	tst.l	d0
	beq.s	try_again		; user wants to retry

	;-- return failure
	moveq	#0,d0
exall_exit:
	movem.l	(a7)+,d1-d7/a2-a4
	rts

exall_end_unknown:
	;-- set string to ~(#?) and call ExAll
	move.l	a4,eac_MatchString(a3)	; old string saved
	move.l	#ACTION_EXAMINE_ALL,d2
	bra	try_again		; now do a regular ExAll..

nomatch_string:	dc.b	'~(#?)',0
		ds.w	0
*
* LONG ASM
* exallend (REG(d1) BPTR lock, REG(d2) struct ExAllData *buffer, REG(d3) LONG size,
*           REG(d4) LONG data, REG(d5) struct ExAllControl *control)
*
* If ExAllEnd packet fails (ACTION_UNKNOWN), change string to ~(#?) and call
* ExAll.  That matches nothing.
@ExAllEnd:
	move.l	d5,a0
	move.l	eac_MatchString(a0),-(sp)	; save matchstring
	move.l	#ACTION_EXAMINE_ALL_END,d0
	bsr	do_exall			; may do a real exall
	move.l	d5,a0
	move.l	(sp)+,eac_MatchString(a0)	; may have changed
	rts

*
*LONG ASM
*examine (REG(d1) BPTR lock,
*	 REG(d2) struct FileInfoBlock *fib)
*{
*	LONG args = (LONG) fib;
*
*	fib->fib_OwnerUID = fib->fib_OwnerGID = 0;
*	return do_lock(lock,&args,ACTION_EXAMINE_OBJECT);
*}
*
*LONG ASM
*exnext (REG(d1) BPTR lock,
*	REG(d2) struct FileInfoBlock *fib)
*{
*	LONG args = (LONG) fib;
*
*	fib->fib_OwnerUID = fib->fib_OwnerGID = 0;	/* not needed! */
*	return do_lock(lock,&args,ACTION_EXAMINE_NEXT);
*}
*
*LONG ASM
*ExamineFH (REG(d1) BPTR lock,
*	    REG(d2) struct FileInfoBlock *fib)
*{
*	LONG args = (LONG) fib;
*
*	/* lock is really a filehandle */
*	fib->fib_OwnerUID = fib->fib_OwnerGID = 0;
*	return do_lock(lock,&args,ACTION_EXAMINE_FH);
*}
*
*/* next one really takes struct Info's */
*
*LONG ASM
*info (REG(d1) BPTR lock,
*      REG(d2) struct FileInfoBlock *fib)
*{
*	LONG args = (LONG) fib;
*
*	return do_lock(lock,&args,ACTION_INFO);
*}
*
*/* it's convenient to use do_lock */
*
*LONG ASM
*parent (REG(d1) BPTR lock)
*{
*	LONG args = 0;			/* just to keep compiler happy */
*
*	return do_lock(lock,&args,ACTION_PARENT);
*}
*/* copydir is otherwise known as DupLock */
*
*LONG ASM
*copydir (REG(d1) BPTR dir)
*{
*	LONG mydir = dir;
*
*	if (dir == 0)
*		return 0;
*
*	return do_lock(dir,&mydir,ACTION_COPY_DIR);
*}

_ExamineFH:
	move.l	#ACTION_EXAMINE_FH,d0	; really a FileHandle!!! do_lock handles
	bra.s	lock_common_fib		; it by checking action==examine_fh

_examine:
	moveq	#ACTION_EXAMINE_OBJECT,d0
	bra.s	lock_common_fib

_exnext:
	moveq	#ACTION_EXAMINE_NEXT,d0
	tst.l	d1			; lock
	bne.s	lock_common_fib
	move.l	#ERROR_INVALID_LOCK,d1
	bsr	@setresult2
	moveq	#0,d0			; failure
	rts				; FS blows up on ExNext(NULL,...)

_info:
	moveq	#ACTION_INFO,d0
	bra.s	lock_common_fib_not	; don't clear ownerID!!!
lock_common_fib:
	move.l	d2,a0			; set OwnerUID/GID to 0
	clr.l	fib_OwnerUID(a0)	; two words!
lock_common_fib_not:
	move.l	d2,-(a7)
	lsr.l	#2,d2			; need to pass bptr to fib
	bsr.s	lock_common
	move.l	(a7)+,d2
	rts

_parent:
	moveq	#ACTION_PARENT,d0

lock_common:
	move.l	d2,-(a7)		; args array
	move.l	a7,a0
	bsr	@do_lock		; d1 = lock, a0 = args ptr, d0 action
	addq.w	#4,a7
	rts

_copydir:
	move.l	d2,-(a7)
	move.l	d1,d2			; lock is arg1
	move.l	d1,d0			; for "if (lock==0) return(0)"
	beq.s	lock_exit
	moveq	#ACTION_COPY_DIR,d0
	bsr.s	lock_common
lock_exit:
	move.l	(a7)+,d2
	rts

*
*LONG ASM
*setprotection (REG(d1) char *name,
*	       REG(d2) LONG mask)
*{
*	LONG temp = mask;
*
*	return objact(name,ACTION_SET_PROTECT,&temp);
*}
*
*LONG ASM
*SetOwner (REG(d1) char *name,
*	   REG(d2) LONG mask)
*{
*	LONG temp = mask;
*
*	return objact(name,ACTION_SET_OWNER,&temp);
*}
*
*LONG ASM
*setdate (REG(d1) char *name,
*	 REG(d2) struct DateStamp *date)
*{
*	LONG temp = (LONG) date;
*
*	return objact(name,ACTION_SET_DATE,&temp);
*}
*
*LONG ASM
*deletefile (REG(d1) char *name)
*{
*	return objact_noarg(name,ACTION_DELETE_OBJECT);
*}
*
*LONG ASM
*createdir (REG(d1) char *name)
*{
*	return objact_noarg(name,ACTION_CREATE_DIR);
*}
*
*LONG ASM
*lock (REG(d1) char *name,
*      REG(d2) LONG mode)
*{
*	LONG temp = mode;
*
*	return objact(name,ACTION_LOCATE_OBJECT,&temp);
*}
*
*/* locatedir is a define */
*
*LONG ASM
*locateobj (REG(d1) char *name)
*{
*	LONG temp = SHARED_LOCK;
*
*	return objact(name,ACTION_LOCATE_OBJECT,&temp);
*}
*
*/* dest can be (BPTR) lock (for hard links) or a string (for soft links) */
*
*LONG ASM
*MakeLink (REG(d1) char *name,REG(d2) LONG dest,REG(d3) LONG soft)
*{
*	LONG args[2];
*	LONG rc;
*
*	args[0] = dest;		/* args[0] is string or lock */
*	args[1] = soft;
*
*	rc = objact(name,ACTION_MAKE_LINK,&args[0]);
*
*	return rc;
*}

@SetOwner:
	move.l	#ACTION_SET_OWNER,d0
	bra.s	obj_common

_setprotection:
	moveq	#ACTION_SET_PROTECT,d0
	bra.s	obj_common

_setdate:
	moveq	#ACTION_SET_DATE,d0
	bra.s	obj_common

_deletefile:
	moveq	#ACTION_DELETE_OBJECT,d0
	bra.s	obj_common

_createdir:
	moveq	#ACTION_CREATE_DIR,d0
	bra.s	obj_common

_MakeLink:
	move.l	#ACTION_MAKE_LINK,d0	; works by luck - common saves d2/d3!!
	bra.s	obj_common

_lock:
	moveq	#ACTION_LOCATE_OBJECT,d0

obj_common:
	movem.l	d2/d3,-(a7)
	move.l	a7,d3		; points to the value of d2
	move.l	d0,d2
	bsr	@objact		; d1 = name, d2 = action, d3 = args ptr (d2/d3)
	movem.l	(a7)+,d2/d3
	move.l	d0,d1		; REMOVE in 2.1!!!!!!!! for arp, etc! FIX!!!!
	rts

_locateobj:
	move.l	d2,-(a7)
	moveq	#SHARED_LOCK,d2
	bsr.s	_lock
	move.l	(a7)+,d2
	rts

*LONG ASM
*read (REG(d1) BPTR scb,
*      REG(d2) char *v,
*      REG(d3) LONG n)
*{
*	LONG args[2];
*
*	args[0] = pos;
*	args[1] = mode;
*
*	return readwrite(scb,args,'R',TRUE);
*}
*
*LONG ASM
*write (REG(d1) BPTR scb,
*       REG(d2) char *v,
*       REG(d3) LONG n)
*{
*	LONG args[2];
*
*	args[0] = pos;
*	args[1] = mode;
*
*	return readwrite(scb,args,'W',TRUE);
*}
*
*LONG ASM
*seek (REG(d1) BPTR scb,
*      REG(d2) LONG pos,
*      REG(d3) LONG mode)
*{
*	LONG args[2];
*
*	args[0] = pos;
*	args[1] = mode;
*
*	/* allow Seek() on buffered filehandles */
*	Flush(scb);
*
*	return readwrite(scb,args,ACTION_SEEK,TRUE);
*}
*
*LONG ASM
*SetFileSize (REG(d1) BPTR scb,
*	      REG(d2) LONG pos,
*	      REG(d3) LONG mode)
*{
*	LONG args[2];
*
*	args[0] = pos;
*	args[1] = mode;
*	return readwrite(scb,args,ACTION_SET_FILE_SIZE,TRUE);
*}
*
* /* 0's are don't cares */
*
*BPTR ASM
*DupLockFromFH (REG(d1) BPTR fh)
*{
*	LONG args;
*
*	return readwrite(fh,&args,ACTION_COPY_DIR_FH,FALSE);
*}
*
*BPTR ASM
*ParentOfFH (REG(d1) BPTR fh)
*{
*	LONG args;
*
*	return readwrite(fh,&args,ACTION_PARENT_FH,FALSE);
*}
*
*LONG ASM
*UnLockRecord (REG(d1) BPTR fh, REG(d2) LONG position, REG(d3) LONG length)
*{
*	LONG args[2];
*
*	args[0] = position;
*	args[1] = length;
*	return readwrite(fh,args,ACTION_FREE_RECORD,FALSE);
*}
*
* LONG ASM
* LockRecord (REG(d1) BPTR fh, REG(d2) LONG position, REG(d3) LONG length,
* 	      REG(d4) LONG mode, REG(d5) LONG timeout)
* {
*	LONG args[4];
*
*	args[0] = position; args[1] = length;
*	args[2] = mode;     args[3] = timeout;
*
* 	return readwrite(fh,args,ACTION_LOCK_RECORD,FALSE);
* }

_read:
	moveq	#'R',d0
	bra.s	rws_common

_write:
	moveq	#'W',d0
	bra.s	rws_common

_SetFileSize:
	move.l	#ACTION_SET_FILE_SIZE,d0
	bra.s	seek_common

_noflushseek:
	move.l	#ACTION_SEEK,d0		; no flush!!! (called from flush!)
	bra.s	rws_common

_seek:
	move.l	#ACTION_SEEK,d0
seek_common
	movem.l	d0/d1,-(a7)
	bsr	@Flush
	movem.l	(a7)+,d0/d1
rws_common
	lea	1,a1		; flag - this returns <0 for failure
	bra.s	rw_common
*
* The following commands are BOOLEAN - NULL == failure
*
_UnLockRecord:
	move.l	#ACTION_FREE_RECORD,d0
	bra.s	rw_null_common

_DupLockFromFH:
	move.l	#ACTION_COPY_DIR_FH,d0
	bra.s	rw_null_common

_ParentOfFH:
	move.l	#ACTION_PARENT_FH,d0
rw_null_common:
	sub.l	a1,a1			; flag - this returns NULL for error

rw_common:				; readwrite takes d1/a0/d0/a1
	movem.l	d2-d3,-(a7)		; pos/mode, normally
	move.l	a7,a0			; args
	bsr.s	_readwrite
	addq.w	#8,a7			; drop d2/d3 off stack
	rts

_LockRecord:
	move.l	#ACTION_LOCK_RECORD,d0
	movem.l	d2-d5,-(a7)		; seperate copy for speed, not size
	move.l	a7,a0
	sub.l	a1,a1			; flag: null = failure
	bsr.s	_readwrite
	lea	16(a7),a7		; drop 4 regs off stack
	rts

*LONG ASM
*readwrite (REG(d1) BPTR scb,
*	    REG(a0) LONG *args,
*	    REG(d0) LONG code.
*	    REG(a1) LONG returntype)
*{
*	struct MsgPort *task;
*	struct FileHandle *fh;
*	LONG res;
*
*	fh = (struct FileHandle *) BADDR(scb);
*
*	if (!fh)
*	{
*		setresult2(ERROR_INVALID_LOCK);
*		return (returntype ? -1 : 0);
*	}
*	while (1) {
*		task = fh->fh_Type;
*		if (!task)			/* this is where nil: works */
*			return (code == 'W' ? n : 0);
*
*		res = sendpkt5(task,code,fh->fh_Arg1,
*			       args[0],args[1],args[2],args[3]);
*
*		/* careful! different error types! */
*		if (((returntype && res >= 0) ||
*		     (!returntype && res != 0)) ||
*		    err_report(REPORT_STREAM,scb,NULL))
*		{
*			return res;
*		}
*	}
*}

_readwrite:
	movem.l	d2-d7/a2-a5,-(a7)
	move.l	a0,a2			; save args
	move.l	d1,a3			; save scb for error call
	move.l	d0,a4			; save code
	move.l	a1,a5			; save flag
rw_loop:
	lsl.l	#2,d1			; bptr->cptr
	beq.s	rw_badfh
	move.l	d1,a1			; fh
	move.l	fh_Type(a1),d1		; is this nil:?
	bne.s	good_task
	moveq	#'W',d1			; is this a write?
	cmp.l	d0,d1			; (action == 'W')
	beq.s	is_write
	moveq	#0,d0			; no, say it failed
	bra.s	rw_exit
is_write:
	move.l	4(a0),d0		; yes, say it succeeded
	bra.s	rw_exit

good_task:
	;-- send packet
	;-- d1 == msgport, d0 = action, a0 = args, a1 = fh

	move.l	d0,d2			; action (code)
	move.l	fh_Arg1(a1),d3		; filehandle arg1
	movem.l	(a0)+,d4-d7		; arg2 thru arg5
	bsr	@sendpacket

	;-- now figure out what to do
	move.l	a5,d1			; returntype
	bne.s	is_rws			; is read write or seek or setfilesize
	tst.l	d0
	bne.s	rw_exit			; success is d0 != 0
	bra.s	rw_error

is_rws:	tst.l	d0
	bpl.s	rw_exit			; success is d0 >= 0

	;-- error - put up requester, try again or exit
rw_error:
	bsr	@getresult2		; error to d0
	move.l	d0,d1
	moveq	#REPORT_STREAM,d2
	move.l	a3,d3			; scb (BPTR)
	sub.l	a0,a0			; NULL
	bsr	@ErrorReport
	tst.l	d0
	bne.s	rw_err_exit
	move.l	a3,d1			; put back BPTR
	move.l	a2,a0			; restore args
	move.l	a4,d0			; restore code
	bra.s	rw_loop

rw_badfh:
	;-- exit with 0 or -1, and set ioerr
	moveq	#211^255,d0		; 211^255 = complement of lowest 8 bits
	not.b	d0			; ~(~211) = 211
	bsr	@setresult2
	moveq	#-1,d0			; default assume rws call
	; fall through

rw_err_exit:
	;-- exit with 0 for non-rws calls (ErrorReport returns -1)
	move.l	a5,d1
	bne.s	rw_exit
	moveq	#0,d0			; error for LockRecord, etc
rw_exit:
	movem.l	(a7)+,d2-d7/a2-a5
	move.l	d0,d1			; for GODDAMN ovs.asm - assumes d1!
	rts

*
*/* DOESN'T pass fh_Arg1! - no error reporting! */
*
*LONG ASM
*setmode (REG(d1) BPTR scb, REG(d2) LONG mode)
*{
*	struct MsgPort *port;
*
*	port = ((struct FileHandle *) BADDR(scb))->fh_Type;
*
*	if (!port)		/* NIL: works here */
*		return FALSE;
*
*	return sendpkt1(port,ACTION_SCREEN_MODE,mode);
*}
*
*LONG ASM
*waitforchar (REG(d1) BPTR scb, REG(d2) LONG timeout)
*{
*	struct MsgPort *port;
*
*	port = ((struct FileHandle *) BADDR(scb))->fh_Type;
*
*	if (!port)		/* NIL: works here */
*		return FALSE;
*
*	return sendpkt1(port,ACTION_WAIT_CHAR,timeout);
*}
*
_setmode:
	move.l	#ACTION_SCREEN_MODE,d0
	bra.s	fh_one_arg
_waitforchar:
	move.l	#ACTION_WAIT_CHAR,d0

fh_one_arg:
	movem.l	d2-d3,-(sp)
	move.l	d2,d3		;arg
	move.l	d0,d2		;action

	moveq	#0,d0		; default return value
	lsl.l	#2,d1		; BADDR(scb)
	move.l	d1,a0
	move.l	fh_Type(a0),d1	; port
	beq.s	1$		; handle nil:

	bsr	@sendpacket	; port,action,arg

1$:	movem.l	(sp)+,d2-d3
	rts

*
*LONG ASM
*inhibit (REG(d1) char *name, REG(d2) LONG onoff)
*{
*	return devact(name,onoff,ACTION_INHIBIT);
*}
*
*/* pr_result2 has number off buffers if this succeeds */
*
*LONG ASM
*addbuffers (REG(d1) char *name, REG(d2) LONG number)
*{
*	return devact(name,number,ACTION_MORE_CACHE);	/* may be negative */
*}
*

_inhibit:
	moveq	#ACTION_INHIBIT,d0
	bra.s	devact_common

_addbuffers:
	moveq	#ACTION_MORE_CACHE,d0
devact_common:		; takes d1 = name d2 = arg1 d3 = arg2 d0 = action
	bra	@devact	; bsr/rts

*
*LONG ASM
*relabel (REG(d1) char *drive, REG(d2) char *newname)
*{
*	return devact_str(drive,newname,0,ACTION_RENAME_DISK);
*}
*
*LONG ASM
*Format (REG(d1) char *drive, REG(d2) char *volumename, REG(d3) ULONG private)
*{
*	return devact_str(drive,volumename,private,ACTION_FORMAT);
*}
*
*LONG ASM
*devact_str (REG(d1) char *drive, REG(d2) char *string, REG(d3) ULONG arg,
*	    REG(d0) LONG action)
*{
*	UBYTE *str;
*	LONG rc;
*
*	/* FIX! use string on stack? */
*	str = AllocVecPub(strlen(string)+1);
*	if (!str)
*		return FALSE;
*
*	CtoB(string,(CPTR) str);
*	rc = devact(drive,TOBPTR(str),arg,action);
*	freeVec(str);
*
*	return rc;
*}
*

_relabel:
	move.l	#ACTION_RENAME_DISK,d0
	bra.s	_devact_str

_Format:
	move.l	#ACTION_FORMAT,d0
	;-- fall through

_devact_str:
	movem.l	d0/d1/d2,-(a7)		; save action, drive, string
	move.l	d2,a0
	bsr	@strlen
	addq.l	#1,d0
	bsr	@AllocVecPub		; d0 = strlen(string)+1
	move.l	d2,d1			; string (source)
	move.l	d0,d2			; new str space (dest)
	beq.s	2$

	bsr	@CtoB			; (d1 = source, d2 = dest)
	move.l	d0,d2			; returns TOBPTR(d2)
	move.l	(a7)+,d0		; TRICKY d0 = action
	move.l	(a7),d1			; TRICKY d1 = drive
					; d3 is already == arg
	bsr.s	devact_common		; goes to devact()
	move.l	d0,-(a7)		; save result (replaces action)

	lsl.l	#2,d2			; turn back into CPTR
	move.l	d2,a1
	bsr	_freeVec		; a1

1$	movem.l	(a7)+,d0/d1/d2		; TRICKY!! d0 = rc, restore d2!
	rts

2$	clr.l	(a7)			; TRICKY make rc = 0 (was action)
	bra.s	1$

	END

