*
* C initial startup procedure under AmigaDOS
* 
* Use the following command line to make c.o
* asm -u -iINCLUDE: c.a
*
* Use the following command line to make cres.o
* asm -u -dRESIDENT -iINCLUDE: -ocres.o c.a
*
	INCLUDE	"exec/types.i"
	INCLUDE	"exec/alerts.i"
	INCLUDE	"exec/nodes.i"
	INCLUDE	"exec/lists.i"
	INCLUDE	"exec/ports.i"
	INCLUDE	"exec/libraries.i"
	INCLUDE	"exec/tasks.i"
	INCLUDE	"exec/memory.i"
	INCLUDE	"exec/execbase.i"
	INCLUDE	"libraries/dos.i"
	INCLUDE	"libraries/dosextens.i"
	INCLUDE	"workbench/startup.i"
	INCLUDE	"exec/funcdef.i"
	INCLUDE	"exec/exec_lib.i"
	INCLUDE	"libraries/dos_lib.i"

MEMFLAGS	EQU	MEMF_CLEAR+MEMF_PUBLIC
AbsExecBase	EQU	4

;;;
;;;   Stack map.
;;;
      OFFSET  0
           ds.b    4
savereg    ds.b    13*4
stackbtm   ds.b    4



; some usefull macros:

callsys macro
	CALLLIB _LVO\1
	endm
	
	xdef	XCEXIT		* exit(code) is standard way to leave C.
	xdef	@XCEXIT
	
	xref	LinkerDB	* linker defined base value
	xref	_BSSBAS		* linker defined base of BSS
	xref	_BSSLEN		* linker defined length of BSS
	IFD	RESIDENT
	xref	RESLEN
	xref	RESBASE
	xref	NEWDATAL
	xref	_stack
	ENDC
	
*       library references

	section text,code

	IFND	TINY
	xref    _main			* Name of C program to start with.
	ELSE
	xref    _tinymain		* Name of C program to start with.
	ENDC

	xref    MemCleanup		* Free all allocated memory
	xref    __fpinit		* initialize floating point
	xref    __fpterm		* terminate floating point

start:
	movem.l d1-d6/a0-a6,-(a7)       * save registers

	move.l  a0,a2			* save command pointer
	move.l  d0,d2			* and command length
	lea     LinkerDB,a4		* load base register
	move.l  AbsExecBase.W,a6

	IFND	RESIDENT
	lea     _BSSBAS,a3		* get base of BSS
	moveq   #0,d1
	move.l  #_BSSLEN,d0		* get length of BSS in longwords
	bra.s   clr_lp			* and clear for length given
clr_bss move.l  d1,(a3)+
clr_lp  dbf     d0,clr_bss
	move.l  a7,_StackPtr(A4)       * Save stack ptr
	move.l  a6,SysBase(A4)
	ENDC
	

	IFD	RESIDENT
	move.l	d2,-(a7)
	movem.l	a0-a2,-(a7)

*------ get the size of the stack, if CLI use cli_DefaultStack
*------			    	   if WB use a7 - TC_SPLOWER
	move.l	ThisTask(a6),A3
	move.l  pr_CLI(A3),d1
	beq.s   fromwb
	lsl.l	#2,d1
	move.l	d1,a0
	move.l	cli_DefaultStack(a0),d1
	lsl.l	#2,d1			* # longwords -> # bytes
	bra.s	dostack

fromwb:
	move.l	a7,d1		 	
	sub.l	TC_SPLOWER(a3),d1
dostack:
	moveq	#0,d2			* use d2 as flag for newstack or not
	move.l	#RESLEN,d0
	cmp.l	_stack(a4),d1		* This a4 is in the original 
					* set of data
	bcc.s	nochange	
	move.l	_stack(a4),d1
	add.l	d1,d0			* increase size of mem for new stack
	moveq	#1,d2			* set flag
		
nochange:
	move.l	d1,a3			* save stacksize to set up stack checking
	move.l	#MEMFLAGS,d1
	callsys AllocMem
	tst.l	d0
	bne.s	ok1
	movem.l	(a7)+,d2/a0-a2
	bra.w   return

ok1:	move.l	d0,a0
	move.l	d0,a2

;a2 now has difference
	move.l	d0,a1
	move.l	#NEWDATAL,d0
	sub.l	#RESBASE,a4
;copy data over
cpy:    move.l	(a4)+,(a0)+
	subq.l	#1,d0
	bne.s	cpy
;a4 now points at number of relocs
	move.l	(a4)+,d0
reloc:  beq.s	nreloc
	move.l	a1,a0
	add.l	(a4)+,a0		* a0 now has add of reloc
	add.l	(a0),a2
	move.l	a2,(a0) 
	move.l	a1,a2			* restore offset
	subq.l	#1,d0
	bra.s	reloc
	
nreloc: move.l	a1,a4			* set up new base register
	add.l	#RESBASE,a4

	move.l	#RESLEN,realdatasize(a4)
	movem.l (a7)+,a0-a2

	move.l  a6,SysBase(A4)
	tst.b	d2
	movem.l	(a7)+,d2		* restore d2 
	movem.l a7,_StackPtr(A4)	* Save stack ptr (movem doesn't
					* change flags
	beq.s	nochg2

*------ set up new stack
	move.l	a4,d0
	sub.l	#RESBASE,d0
	add.l	#RESLEN,d0
	add.l	_stack(a4),d0		* here a4 will be pointing at the
					* new data, but _stack will be the
					* same if all goes well
	sub.l	#128,d0			* 128 down for good measure
	move.l	d0,a7
	move.l	_stack(a4),d0
	move.l	d0,4(a7)		* fill in size of new stack	
	add.l	d0,realdatasize(a4) 	* need to know how much to free later

nochg2:
*------ Set _base for stack checking
	move.l	a7,d1
	sub.l	a3,d1			* get top of stack
	add.l   #128,D1			* allow for parms overflow
	move.l  D1,_base(A4)    	* save for stack checking

	ENDC

clrwb:
	clr.l   WBenchMsg(A4)

*-----  clear any pending signals
	moveq	#0,d0
	move.l	#$00003000,d1
	callsys	SetSignal
	

*------ attempt to open DOS library:
	lea     DOSName(PC),A1
	moveq.l #0,D0
	callsys OpenLibrary
	move.l  D0,DOSBase(A4)
	bne.s	ok2
	moveq.l #100,d0
	bra.w   exit2

ok2:
*------ are we running as a son of Workbench?
	move.l	ThisTask(a6),A3
	move.l  pr_CurrentDir(A3),curdir(A4)
	tst.l   pr_CLI(A3)
	beq.w  fromWorkbench
	
*=======================================================================
*====== CLI Startup Code ===============================================
*=======================================================================
*
* Entry: D2 = command length
*	A2 = Command pointer
fromCLI:
	ifnd	RESIDENT	* we need to set _base if not resident
        move.l  a7,D0           * get top of stack
        sub.l   stackbtm(a7),D0 * compute bottom
        add.l   #128,D0         * allow for parms overflow
        move.l  D0,_base(A4)    * save for stack checking
	endc
	
*------ find command name:
	move.l  pr_CLI(a3),a0
	add.l   a0,a0	   * bcpl pointer conversion
	add.l   a0,a0
	move.l  cli_CommandName(a0),a1
	add.l   a1,a1	   * bcpl pointer conversion
	add.l   a1,a1

*------ collect parameters:
	move.l  d2,d0		   * get command line length
	moveq.l #0,d1
	move.b  (a1)+,d1
	move.l  a1,_ProgramName(A4)
	add.l   d1,d0		   * add length of command name
	addq.l  #7,d0		   * allow for space after command, quotes
			           * and null terminator, as well as 
	andi.w  #$fffc,D0	   * force to long word boundary
        move.l   d0,Commandlen(a4)
   
        movem.l  d1/a1,-(a7)
	move.l	#MEMFLAGS,d1
	callsys AllocMem
	tst.l   d0
	bne.s   ok_copy
   
	move.l  #1000,d0        * what should th return code be for out of mem?
	move.l  d0,-(a7)        * put a return code on the stack
	beq.w   nodofree        * Was exitToDOS
   
ok_copy:
	move.l   d0,a0
	move.l   d0,Commandbuf(a4)
	movem.l  (a7)+,d1/a1
         
*------ copy command line into memory
	move.l  d2,d0		   * get command line length
	subq.l  #1,d0
	add.l   d1,d2
   
copy_line:
	move.b  0(A2,D0.W),2(A0,D2.W)   * copy command line to stack
	subq.l  #1,d2
	dbf     d0,copy_line
	move.b  #' ',2(a0,d2.w)	 * add space between command and parms
	subq.l  #1,d2
	move.b  #'"',2(a0,d2.w)	 * add end quote
   
copy_cmd:
	move.b  0(a1,d2.w),1(a0,d2.w)   * copy command name to stack
	dbf     d2,copy_cmd
        move.b  #'"',(a0)
	move.l  A0,-(A7)		* push command line address
	bra.s   main	  		 * call C entrypoint
*=======================================================================
*====== Workbench Startup Code =========================================
*=======================================================================

fromWorkbench:

	ifnd	RESIDENT	* we need to set _base if not resident
        move.l  TC_SPLOWER(a3),_base(A4) * set base of stack
	moveq   #127,d0
	addq.l	#1,d0                    * Efficient way of getting in 128
        add.l   d0,_base(A4)             * allow for parms overflow
	endc

*------ we are now set up.  wait for a message from our starter
	lea     pr_MsgPort(A3),a0       * our process base
	callsys WaitPort
	lea     pr_MsgPort(A3),a0       * our process base
	callsys GetMsg
	move.l  d0,WBenchMsg(a4)
	move.l  d0,-(SP)
*
	move.l  d0,a2		   * get first argument
	move.l  sm_ArgList(a2),d0
	beq.s   do_cons
	move.l  DOSBase(a4),a6
	move.l  d0,a0
	move.l  wa_Lock(a0),d1
	move.l  d1,curdir(A4)
	callsys CurrentDir
do_cons:
	IFND	TINY
	move.l  sm_ToolWindow(a2),d1    * get the window argument
	beq.s   do_main
	move.l  #MODE_OLDFILE,d2
	callsys Open
	move.l  d0,stdin(a4)
	beq.s   do_main
	lsl.l   #2,d0
	move.l  d0,a0
	move.l  fh_Type(a0),pr_ConsoleTask(A3)
	ENDC
do_main:
	move.l  WBenchMsg(A4),a0	* get address of workbench message
	move.l  a0,-(a7)		* push argv
	pea     NULL(a4)		* push argc
	move.l  sm_ArgList(a0),a0       * get address of arguments
	move.l  wa_Name(a0),_ProgramName(A4)       * get name of program

*=============================================
*------ common code --------
*=============================================

main    jsr     __fpinit(PC)	    * Initialize floating point

	IFND	TINY
	jsr     _main(PC)	       * call C entrypoint
	ELSE
	jsr     _tinymain(PC)	       * call C entrypoint
	ENDC

	moveq.l #0,d0		   * set successful status
	bra.s   exit2
*

XCEXIT:
	move.l  4(SP),d0		* extract return code
@XCEXIT:
exit2:
        movea.l _StackPtr(a4),a7         * restore stack ptr
	move.l  d0,-(a7)
	move.l  _ONEXIT(A4),d0	  * exit trap function?
	beq.s   exit3
	move.l  d0,a0
	jsr     (a0)
exit3:
	jsr     MemCleanup(PC)	  * cleanup leftover memory alloc.
	jsr     __fpterm(PC)	    * clean up any floating point

*------ if we ran from CLI, skip workbench cleanup:
	tst.l   WBenchMsg(A4)
	beq.s   exitToDOS
	move.l  stdin(a4),d1
	beq.s   done_4
        move.l  DOSBase(A4),a6
	callsys Close
done_4:

*------ return the startup message to our parent
*       we forbid so workbench can't UnLoadSeg() us
*       before we are done:
	move.l  AbsExecBase.W,A6
	callsys Forbid
	move.l  WBenchMsg(a4),a1
	callsys ReplyMsg
	bra.s   nodofree

exitToDOS:
*------ free the command line buffer
	move.l   Commandlen(a4),d0
	beq.s    nodofree
	move.l   Commandbuf(a4),a1
	move.l  AbsExecBase.W,a6
	callsys FreeMem
   
*------ this rts sends us back to DOS:
nodofree:
	move.l  DOSBase(A4),a1
	callsys CloseLibrary	    * close Dos library

	IFD	RESIDENT
	move.l	realdatasize(a4),d0
	move.l  a4,a1
	sub.l   #RESBASE,a1
	callsys FreeMem
	ENDC
	move.l  (a7)+,d0

return:	
	movem.l (a7)+,d1-d6/a0-a6
	rts


DOSName	 dc.b    'dos.library',0

	section __MERGED,BSS
*
	xref    DOSBase

	xdef    NULL,SysBase,WBenchMsg
	xdef    curdir,_mbase,_mnext,_msize,_tsize
	xdef    _oserr,_OSERR,_FPERR,_SIGFPE,_ONERR,_ONEXIT,_ONBREAK
	xdef    _SIGINT
	xdef    _ProgramName,_StackPtr,_base

*
	ifd	RESIDENT
realdatasize	ds.b	4	* size of memory allocated for data +
				* possible stack
	endc
	
NULL		ds.b	4	*
_base		ds.b	4	* base of stack
_mbase		ds.b    4	* base of memory pool
_mnext		ds.b    4	* next available memory location
_msize		ds.b    4	* size of memory pool
_tsize		ds.b    4	* total size?
_oserr		equ     *
_OSERR		ds.b    4
_FPERR		ds.b    4
_SIGFPE		ds.b    4
_SIGINT		ds.b    4
_ONERR		ds.b    4
_ONEXIT		ds.b    4
_ONBREAK	ds.b    4
curdir	 	ds.b    4
SysBase		ds.b    4
WBenchMsg	ds.b    4
_StackPtr	ds.b    4
stdin		ds.b    4
_ProgramName	ds.b    4
Commandbuf	ds.b    4
Commandlen	ds.b    4
		END

