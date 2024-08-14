;*****************************************************************************
;
; vars.inc contains variable datas and messageses for PC janus handler;
;	   will be stored on the top of this handler.
;
;--- global data definition --------------------------------------------------
;
public	       chain_vec
public	       bios_int13
public	       DOS_int28
public	       PCint_done
public	       int_req
public	       int_enable
public	       ActiveFlag
public	       FakeDosFlag	
public	       ustack
public	       sstack
public	       parmptr
public	       janus_part_base
public	       janus_base_seg
public	       janus_param_seg
public	       janus_buffer_seg
public	       ServStatTab
public	       space
;					     ; => (fixed at offset 6)
;
chain_vec	    dd	 0		     ; save redirected pointers here
;
bios_int13	    dd	 0		     ; pointer to bios int13 service
;
DOS_int28	    dd	 0		     ; pointer to DOS idle int28
;
PCint_done	    dd	 0		     ; save done PC interrupts here
;
int_req 	    dd	 0		     ; save AMIGA int requests here
;
int_enable	    dd	 0ffffff00h	     ; mask AMIGA interrupt requests
					     ; FOR TESTING: disable all HW ints
					     ; enable all AMIGA SW-ints 8-31
;
janus_base_seg	    dw	 0		     ; segment of janus base
janus_param_seg     dw	 0		     ; segment of janus parameter area
janus_buffer_seg    dw	 0		     ; segment of janus buffer ram
;
janus_part_base     dw	 0		     ; points to janus HD partition
;
ActiveFlag	    db	 0		     ; save int level here
FakeDosFlag	    db   0		     ; set, if called before
;
sstack		    dd	 0		     ; pointer to user stack
ustack		    dw	 0,0		     ; save old stack pointer here
;		      
parmptr 	    dw	 0,0		     ; temp save for int86 handler
;
ServStatTab	    db   32 dup(0ffh)	     ; Status for Janus interrupts	
;
space		    db	 100 dup(0)	     ; free space for later upgrades
;
;------ PARTITION TABLE :  READ VIA INT 13;2 ------------------------------
;
public	       BootBlock, Partmarker
public	       HDPart1, HDPart2, HDPart3, HDPart4
;
BootBlock	db	sectorsize-(4*size HDPart0)-2 DUP (?)
HDPart1:	db	size HDPart0 DUP (0)
HDPart2:	db	size HDPart0 DUP (0)
HDPart3:	db	size HDPart0 DUP (0)
HDPart4:	db	size HDPart0 DUP (0)
PartMarker	dw	?		; 55AAh, if bootblock is defined
;
;------ VARIABLES :  USED BY ADISK ----------------------------------------
;
public		Max_drive, Max_head, Max_cyl, Max_sec
public		CurDrive, CurTable, PartCount, AM_HD
;
Max_drive	db	?		; current drive parameters
Max_head	db	?
Max_cyl 	dw	?
Max_sec 	db	?
CurDrive	DB	0		; CURRENT DRIVE NUMBER
CurTable	dw	?		; pointer to current dsk_partition
PartCount	DB	?		; NUMBER OF amiga partitions	  
AM_HD:		db	7*(size dsk_partition) dup (0)
;
;--- messageses -----------------------------------------------------------
;		       		    Priority
;
public	       inmsg		      ;	 0
public	       loadmsg		      ;	10
public	       BaseMsg		      ; 20
public	       IntsMsg		      ; 20
public	       ParasMsg		      ; 20
public	       BufferMsg	      ; 20
public	       DosMsgA		      ;	50
public	       DosMsgN		      ;	50
public	       HWMsgP		      ;	40
public	       HWMsgPC		      ;	40
public	       HWMsgR		      ;	40
public	       RegMsg		      ;	49
public	       IntTblMes	      ;	55
public	       FoundIntMes	      ;	46
public	       ExeIntMes	      ;	47
public	       ESDI_Mes		      ;	47
public	       TestIntMes	      ;	40
public	       IntCtrlMes	      ;	48
public	       NoHanMes		      ;	40
public	       NoIntMes		      ;	47
public	       NoDriveMsg	      ;	 0
public	       NoPartMsg	      ;	 0
public	       LoadErrMsg	      ;	 0
public	       Diskmsg1		      ;	48
;
inmsg	       db   7,"Inthandler already loaded!",13,10,0
loadmsg        db   " Inthandler loaded at CS = ",0
BaseMsg        db   " JanusBase found at  = ",0
IntsMsg        db   " Int Table offset    = ",0
ParasMsg       db   " Int Paras offset    = ",0
BufferMsg      db   " Buffer Mem found at = ",0
DosMsgA        db   " SW Int13 active ",13,10,0
DosMsgN        db   " SW Int13 ready ",13,10,0
HWMsgP	       db   " HW interrupt pending ",13,10,0
HWMsgPC	       db   " SW IntB active ",13,10,0
HWMsgR	       db   " HW interrupt ready ",13,10,0
RegMsg	       db   " -AX--BX--CX--DX- Regs before INT13",13,10,0
IntTblMes      db   " Janus Interrupt Table at offset ",0
FoundIntMes    db   " Found pending interrupt #: ",0
ExeIntMes      db   " Executing interrupt #: ",0
ESDI_Mes       db   " ES:DI points at  ",0
TestIntMes     db   " Amiga Interrupt Diagnostic :   ",0
IntCtrlMes     db   " => Interrupt Enable Register :  ",0
NoHanMes       db   7," ERROR! No handler installed on PC side. ",13,10,0
NoIntMes       db   7," ERROR! No enabled interrupt found. ",13,10,0
NoDriveMsg     db   " No harddisk found. ",13,10,0
NoPartMsg      db   7," ERROR! Incorrect partition on harddisk. ",13,10,0
LoadErrMsg     db   7," ERROR! Unable to read partition table. ",13,10,0
Diskmsg1       db   " Disk I/O: Function ",0
;
;************ bill variables **********
public bill_int13, jparmseg, jparmoff, jbuffseg, jbuffoff, dataseg
public dataoff, b_file_open, bk_NumHeads, bk_TrackSecs
public	billmsg0, billmsg1, billmsg2, billmsg3, billmsg4, billmsg5
public	AB_file, okflag

bill_int13	dd	0

jparmseg	dw	0
jparmoff	dw	0
jbuffseg	dw	0
jbuffoff	dw	0

dataseg		dw	0
dataoff		dw	0

b_file_open	dw	0
bk_NumHeads	dw	4
bk_TrackSecs	dw	17

billmsg0	db	"AB INT 13 Func = ",0
billmsg1	db	" Drive = ",0
billmsg2	db	" Cyl = ",0
billmsg3	db	" Head = ",0
billmsg4	db	" Sec = ",0
billmsg5	db	" #Secs = ",0
AB_file		db	"SYS:sidecar/fake_c",0,0,"end"
okflag		dw	1
