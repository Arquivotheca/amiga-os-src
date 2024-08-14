        PLEN 55
        TTL    "*** blib global vector definitions ***"

*
* I play a lot of games in order to use bra.s as much as possible
*
	INCLUDE	"exec/types.i"
	INCLUDE	"exec/memory.i"
	INCLUDE "exec/alerts.i"
	INCLUDE	"dos/dos.i"
	INCLUDE "dos/dosextens.i"

	INCLUDE "libhdr.i"
	INCLUDE "calldos.i"

BLIB	MACRO	;\1 - symbolname
	XREF	_\1
@\1	EQU	_\1
	ENDM

	XREF	CALLDOS
	BLIB	noreqloadseg
	BLIB	makeglob
	BLIB	parent
	BLIB	readbytes
	BLIB	writebytes
	BLIB	delay
	BLIB	sendpacket
	BLIB	rdch
	BLIB	UnGetC
	BLIB	wrch
	BLIB	readwords
	BLIB	writewords
	BLIB	findinput
	BLIB	findoutput
	BLIB	endread
	BLIB	endwrite
	BLIB	readn
	BLIB	newline
	BLIB	bwritet
	BLIB	writed
	BLIB	writen
	BLIB	writehex
	BLIB	writeoct
	BLIB	bwrites
	BLIB	bwritef
	BLIB	blib_rdargs
	BLIB	blib_rditem
	BLIB	blib_findarg
	BLIB	loadseg
	BLIB	unloadseg
	BLIB	adddevice
	BLIB	datstamp
	BLIB	waitforchar
	BLIB	deletefile
	BLIB	renameobj
	BLIB	endstream
	BLIB	execute
	BLIB	deviceproc
	BLIB	fault
	XREF	@bsplitname
	BLIB	locateobj
	BLIB	freeobj
	BLIB	loaddevice
	BLIB	copydir
	BLIB	err_report
	BLIB	finddev
	BLIB	createdir
	BLIB	dotimer
	BLIB	setdosdate
	BLIB	endtask
	BLIB	isinteractive
	BLIB	seek
	BLIB	findstream
	BLIB	write
	BLIB	read
 
	XREF	@openwindow
	XREF	@qpkt
	XREF	@abort
	XREF	@taskwait
	XREF	@setresult2
	XREF	@input
	XREF	@comparedate

	XREF	REQ0
	XREF	REQUEST2

	XDEF	FAKEBLIB
	XDEF	FAKEBLIB_SEG

FAKEBLIB_SEG
	DC.L	0			; no next segment
FAKEBLIB
	DC.L	(FAKEBLIBEND-FAKEBLIB)/4

	; The worst sendpkt call is in RUN/NEWCLI/etc, and uses upto arg6!
	; the C sendpkt can only handle arg4, so we have a version of the
	; bcpl one here.  The interface for the C version was big, anyways
	; (about 24 bytes - the new one is about 68 bytes).
	; Because of the BCPL calling conventions and stack, and the funny
	; way BCPL sendpkt is defined, the arguements on the BCPL stack
	; (pointed to by a1) make up a DosPacket.
sendpkt:
	link	a4,#-(MN_SIZE)		; allocate exec message on stack
	move.l	a7,dp_Link(a1)		; pointer to message in packet
	move.l	a1,a3			; save packet pointer
	move.l	a1,d1
	bsr	@qpkt			; send the packet
	tst.l	d0			; did it fail (real hard to)
	bne.s	ok

	move.l	#AN_QPktFail!AT_DeadEnd,d1	; REAL bad error
dosabort:
	bra	@abort			; guru

ok:	bsr	@taskwait
	cmp.l	d0,a3			; did I get back the right packet?
	beq.s	1$
	move.l	#AN_AsyncPkt!AT_DeadEnd,d1	; the infamous...
	bra.s	dosabort

1$	move.l	dp_Res2(a3),d0
	bsr	@setresult2
	move.l	dp_Res1(a3),d1		; return res1

	unlk	a4			; cleanup
	move.l	a3,a1			; restore a0/a1
	sub.l	a0,a0
	jmp	(a6)


; BCPL-only routines that need a temporary stub to get hooked into the
; global vector.  CHANGE TO EQUs!!!!! FIX!
request2:
	lea	REQUEST2(pc),a3
	bra.s	req_common

request:
	lea	REQ0(pc),a3
req_common:
	lea	BPTRCPTR!(BPTRCPTR<<ARG2)!(BPTRCPTR<<ARG3)!(CALLDEND<<ARG4),a4
	bra.s	calldos

splitname:
	lea	BPTRCPTR+(NOCHANGE<<ARG2)+(BSTRCSTR<<ARG3)+(CALLDEND<<ARG4),a4
	lea	@bsplitname(pc),a3
	bra.s	calldos
	
loaddevice:
	lea	BPTRCPTR+(BSTRCSTR<<ARG2)+(CALLDEND<<ARG3),a4
	lea	@loaddevice(pc),a3
	bra.s	calldos

rdargs:
	;-- arg3 is really a longs->bytes conversion
	lea	@blib_rdargs(pc),a3
	lea	RETURN_BPTR!BPTRCPTR!(BPTRCPTR<<ARG2)!(BPTRCPTR<<ARG3)!(CALLDEND<<ARG4),a4
	bra.s	calldos

findarg:
	lea	@blib_findarg(pc),a3
two_bptrcptr:
	lea	BPTRCPTR!(BPTRCPTR<<ARG2)!(CALLDEND<<ARG3),a4
	bra.s	calldos

rditem:
	lea	@blib_rditem(pc),a3
	bra.s	two_bptrcptr

comparedate:
	lea	@comparedate(pc),a3
	bra.s	two_bptrcptr

dotimer:
	;-- d3 and d4 unchanged
	lea	NOCHANGE+(BPTRCPTR<<ARG2)+(CALLDEND<<ARG3),a4
	lea	@dotimer(pc),a3
	bra.s	calldos

* bptr->cptr, return bptr
makeglob:
	lea	@makeglob(pc),a3
bptrcptr_bptr:
	lea	RETURN_BPTR!BPTRCPTR!(CALLDEND<<ARG2),a4
	bra.s	calldos
	
datstamp:
	lea	@datstamp(pc),a3
	bra.s	bptrcptr_bptr

* bstr->cstr, return bptr
finddevice:
	;-- args must be placed on BCPL "stack"!
	move.l	#-1,4(a1)		; this is a define in blib_protos.h
	lea	@finddev(pc),a3
	bra.s	bstrcstr_bptr

adddevice:
	lea	@adddevice(pc),a3
bstrcstr_bptr:
	lea	RETURN_BPTR!BSTRCSTR!(CALLDEND<<ARG2),a4
	bra.s	calldos

; common bstr->cstr routine
bstrcstr:
	lea	BSTRCSTR+(CALLDEND<<ARG2),a4
calldos
	bra	CALLDOS

noreqloadseg:
	lea	@noreqloadseg(pc),a3
	bra.s	bstrcstr

findinput:
	lea	@findinput(pc),a3
	bra.s	bstrcstr

findoutput:
	lea	@findoutput(pc),a3
	bra.s	bstrcstr

loadseg:			; actually can take 3 parms */
	lea	@loadseg(pc),a3
	bra.s	bstrcstr

deleteobj:
	lea	@deletefile(pc),a3
	bra.s	bstrcstr

execute:
	lea	@execute(pc),a3
	bra.s	bstrcstr

devicetask:
	lea	@deviceproc(pc),a3
	bra.s	bstrcstr

locateobj:
	lea	@locateobj(pc),a3
	bra.s	bstrcstr

createdir:
	lea	@createdir(pc),a3
	bra.s	bstrcstr

; common bptr->cptr routine
bptrcptr:
	lea	BPTRCPTR+(CALLDEND<<ARG2),a4
	bra.s	calldos

setdosdate:
	lea	@setdosdate(pc),a3
	bra.s	bptrcptr

writef:
	lea	bwritef,a3
	bra.s	bptrcptr	; do NOT convert to cstr!

bwritef:
	moveq	#1,d3		; bcpl-style arguments 
				; DOSCALL must leave a1 intact!!!! 
	move.l	a1,d2		; get pointer to arguments on BCPL stack 
	addq.l	#4,d2		; so it points to second argument 
	bra	@bwritef

; common bstr->cstr and bptr->cptr routine
bstrbptr:
	lea	BSTRCSTR+(BPTRCPTR<<ARG2)+(CALLDEND<<ARG3),a4
	bra.s	noargscalldos

unrdch:
	lea	@UnGetC(pc),a3
	bsr	@input		; get filehandle
	move.l	d0,(a1)		; first arg
	moveq	#-1,d0		; unread the last char read
	move.l	d0,4(a1)	; second arg
	bra.s	tonargs

delay:				; if all args are nochange, don't bother
	lea	@delay(pc),a3
	bra.s	tonargs		; can't reach noargs, bra.s to bra.s

readbytes:
	lea	@readbytes(pc),a3
tonargs
	bra.s	noargs
 
writebytes:
	lea	@writebytes(pc),a3
	bra.s	noargs

readwords:
	lea	@readwords(pc),a3
	bra.s	noargs
 
writewords:
	lea	@writewords(pc),a3
	bra.s	noargs

rdch:
	lea	@rdch(pc),a3
	bra.s	noargs

wrch:
	lea	@wrch(pc),a3
	bra.s	noargs

endread:
	lea	@endread(pc),a3
	bra.s	noargs

endwrite:
	lea	@endwrite(pc),a3
	bra.s	noargs

readn:
	lea	@readn(pc),a3
	bra.s	noargs

newline:
	lea	@newline(pc),a3
	bra.s	noargs

unloadseg:
	lea	@unloadseg(pc),a3
	bra.s	noargs

parent:
	lea	@parent(pc),a3
	bra.s	noargs
	
renameobj:
	lea	BSTRCSTR+(BSTRCSTR<<ARG2)+(CALLDEND<<ARG3),a4
	lea	@renameobj(pc),a3
	bra.s	noargscalldos

waitforchar:
	lea	@waitforchar(pc),a3
	bra.s	noargs
	
endstream:
	lea	@endstream(pc),a3
	;-- fall through...
	
; common no-args routine:
noargs:
	lea	CALLDEND,a4
noargscalldos:
	bra	CALLDOS

fault:
	clr.l	4(a1)			; second param NULL
	lea	@fault(pc),a3
	bra.s	noargs
	
freeobj:
	lea	@freeobj(pc),a3
	bra.s	noargs
	
copydir:
	lea	@copydir(pc),a3
	bra.s	noargs
	
errreport:
	lea	@err_report(pc),a3
	bra.s	noargs

endtask:
	lea	@endtask(pc),a3
	bra.s	noargs

isinteractive:
	lea	@isinteractive(pc),a3
	bra.s	noargs

seek:
	lea	@seek(pc),a3
	bra.s	noargs

write:
	lea	@write(pc),a3
	bra.s	noargs

open:
	lea	@findstream(pc),a3
	bra.s	noargs

read:
	lea	@read(pc),a3
	bra.s	noargs

iexecute:
	lea	@execute(pc),a3
	bra.s	noargs

openwindow:
	lea	myopenwindow,a3
	bra.s	noargs
	
myopenwindow:
	asl.l	#2,d5
*	addq.l	#1,d5
	bra	@openwindow

* EVIL!!! FIX!!!!

writed:
	lea	@writed(pc),a3
	bra.s	noargs

writen:
	lea	@writen(pc),a3
	bra.s	noargs

writehex:
	lea	@writehex(pc),a3
	bra.s	noargs

writes:
	lea	@bwrites(pc),a3
	bra.s	noargs

writet:
	lea	@bwritet(pc),a3
	bra.s	noargs

writeoct:
	lea	@writeoct(pc),a3
	bra.s	noargs

; global vector inits for blib routines
	CNOP	0,4

	DC.L	0
	DC.L	G_IEXECUTE/4,(iexecute-FAKEBLIB)
	DC.L	G_IISINTERACTIVE/4,(isinteractive-FAKEBLIB)
	DC.L	G_ISEEK/4,(seek-FAKEBLIB)
	DC.L	G_IWRITE/4,(write-FAKEBLIB)
	DC.L	G_IREAD/4,(read-FAKEBLIB)
	DC.L	G_IOPEN/4,(open-FAKEBLIB)
	DC.L	G_ENDTASK/4,(endtask-FAKEBLIB)
	DC.L	G_REQUESTOR2/4,(request2-FAKEBLIB)
	DC.L	G_REQUESTOR/4,(request-FAKEBLIB)
	DC.L	G_OPENWINDOW/4,(openwindow-FAKEBLIB)
	DC.L	G_NOREQLOADSEG/4,(noreqloadseg-FAKEBLIB)
	DC.L	G_MAKEGLOB/4,(makeglob-FAKEBLIB)
	DC.L	G_PARENT/4,(parent-FAKEBLIB)
	DC.L	G_READBYTES/4,(readbytes-FAKEBLIB)
	DC.L	G_WRITEBYTES/4,(writebytes-FAKEBLIB)
	DC.L	G_DELAY/4,(delay-FAKEBLIB)
	DC.L	G_SENDPKT/4,(sendpkt-FAKEBLIB)
	DC.L	G_WRITET/4,(writet-FAKEBLIB)
	DC.L	G_RDCH/4,(rdch-FAKEBLIB)
	DC.L	G_UNRDCH/4,(unrdch-FAKEBLIB)
	DC.L	G_WRCH/4,(wrch-FAKEBLIB)
	DC.L	G_READWORDS/4,(readwords-FAKEBLIB)
	DC.L	G_WRITEWORDS/4,(writewords-FAKEBLIB)
	DC.L	G_FINDINPUT/4,(findinput-FAKEBLIB)
	DC.L	G_FINDOUTPUT/4,(findoutput-FAKEBLIB)
	DC.L	G_ENDREAD/4,(endread-FAKEBLIB)
	DC.L	G_ENDWRITE/4,(endwrite-FAKEBLIB)
	DC.L	G_READN/4,(readn-FAKEBLIB)
	DC.L	G_NEWLINE/4,(newline-FAKEBLIB)
	DC.L	G_WRITED/4,(writed-FAKEBLIB)
	DC.L	G_WRITEN/4,(writen-FAKEBLIB)
	DC.L	G_WRITEHEX/4,(writehex-FAKEBLIB)
	DC.L	G_WRITEOCT/4,(writeoct-FAKEBLIB)
	DC.L	G_WRITES/4,(writes-FAKEBLIB)
	DC.L	G_WRITEF/4,(writef-FAKEBLIB)
	DC.L	G_RDARGS/4,(rdargs-FAKEBLIB)
	DC.L	G_RDITEM/4,(rditem-FAKEBLIB)
	DC.L	G_FINDARG/4,(findarg-FAKEBLIB)
	DC.L	G_LOADSEG/4,(loadseg-FAKEBLIB)
	DC.L	G_UNLOADSEG/4,(unloadseg-FAKEBLIB)
	DC.L	G_ADDDEVICE/4,(adddevice-FAKEBLIB)
	DC.L	G_DATSTAMP/4,(datstamp-FAKEBLIB)
	DC.L	G_WAITFORCHAR/4,(waitforchar-FAKEBLIB)
	DC.L	G_DELETEOBJ/4,(deleteobj-FAKEBLIB)
	DC.L	G_RENAMEOBJ/4,(renameobj-FAKEBLIB)
	DC.L	G_ENDSTREAM/4,(endstream-FAKEBLIB)
	DC.L	G_EXECUTE/4,(execute-FAKEBLIB)
	DC.L	G_DEVICETASK/4,(devicetask-FAKEBLIB)
	DC.L	G_FAULT/4,(fault-FAKEBLIB)
	DC.L	G_SPLITNAME/4,(splitname-FAKEBLIB)
	DC.L	G_LOCATEOBJ/4,(locateobj-FAKEBLIB)
	DC.L	G_FREEOBJ/4,(freeobj-FAKEBLIB)
	DC.L	G_LOADDEVICE/4,(loaddevice-FAKEBLIB)
	DC.L	G_COPYDIR/4,(copydir-FAKEBLIB)
	DC.L	G_ERRREPORT/4,(errreport-FAKEBLIB)
	DC.L	G_LOCATEDIR/4,(locateobj-FAKEBLIB)
	DC.L	G_FINDDEVICE/4,(finddevice-FAKEBLIB)
	DC.L	G_CREATEDIR/4,(createdir-FAKEBLIB)
	DC.L	G_DOTIMER/4,(dotimer-FAKEBLIB)
	DC.L	G_SETDOSDATE/4,(setdosdate-FAKEBLIB)
        DC.L	G_COMPAREDATE/4,(comparedate-FAKEBLIB)

	DC.L	G_GLOBMAX/4
FAKEBLIBEND
	END
