TITLE	VAR  -  COPYRIGHT (C) 1986 - 1988 Commodore Amiga Inc. 
PAGE	60,132	
;******************************************************************************
; VAR:		Here we define our variables at an offset greater 2000h
; ====  	for load module,	     
;	     	contains variable datas and messageses for PC janus handler;
;	   
;*****************************************************************************

cseg segment   para public 'code'

     assume    cs:cseg,ss:cseg,ds:cseg,es:nothing

include        	HDPart.inc
include 	janus\harddisk.inc


public		dummy_null, datatag, dataend
;
dummy_null     	db   0900h dup ('a')	; the next offset must be greater
					; then 2000h
datatag		db	'*****DATA START*****'
;
;
;--- global data definition --------------------------------------------------
;
public	  	chain_vec
public	       	bios_int10
public	       	bios_int13
public	       	bios_int16
public	       	bios_int1c
public	       	DOS_int28
;
		even
chain_vec	dd	 0		; save redirected pointers here
bios_int10	dd	 0		; pointer to bios int10 service
bios_int13	dd	 0		; pointer to bios int13 service
bios_int16	dd	 0		; pointer to bios int16 service
bios_int1c	dd	 0		; pointer to timer tick
DOS_int28	dd	 0		; pointer to DOS idle int28
;		
;
public	        janus_base_seg
public	       	janus_param_seg
public	       	janus_buffer_seg
public	       	janus_part_base
;	       	
			even
janus_base_seg	dw	 0		; segment of janus base
janus_param_seg dw	 0		; segment of janus parameter area
janus_buffer_seg dw	 0		; segment of janus buffer ram
janus_part_base	dw	 0		; points to janus HD partition
;
;
public	       	int_req
public	       	int_enable
;
		even
int_req 	dd	 0		; save AMIGA int requests here
int_enable	dd	 0ffffff00h	; mask AMIGA interrupt requests
				     	; FOR TESTING: disable all HW ints
				     	; enable all AMIGA SW-ints 8-31
;
public	       	ActiveFlag
public	       	FakeDosFlag	
public	       	ScrollFlag
public	       	WaitFlag
public	       	DummyFlag	
;
ActiveFlag	db	0		; save int level here
FakeDosFlag	db   	0		; set, if called before
ScrollFlag	db	0		; set, if we are scrolling
WaitFlag	db   	0		; set, if service came available
DummyFlag	db   	0		; free
;
;
public	       	ustack
public	       	sstack
;
		even
sstack		dd	 0		; pointer to user stack
ustack		dd	 0		; save old stack pointer here
;		      
;
public	       	ticks
public		IntCount
ticks		dw	 0		; timer ticks counter	
IntCount       	dw   	 0		; interrupt counter for HW diagnostic
;
;
public	      	ParmPtr
public	      	HandlerPtr
;
ParmPtr 	dw	 0,0		; temp pointer to int86 handler
HandlerPtr 	dw	 0,0		; temp pointer to service handler
;
;
public	       	ServStatTab
ServStatTab	db   32 dup(0ffh)	; Status for Janus interrupts	
;

public	       	PCint_done
		even
PCint_done	dd	 0		; save done PC interrupts here
;
public	       	space
space		db	 100 dup(0)	; free space for later upgrades
;					
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
PartMarker	db	2 DUP (?)	; 55AAh, if bootblock is defined
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
AM_HD:		db	7*(size HDskPartition) dup (0)
;
;--- messageses -----------------------------------------------------------
;		       		    Priority
;
public	       inmsg		      ;	 0
public	       NoDriveMsg	      ;	 0
public	       NoPartMsg	      ;	 0
public	       LoadErrMsg	      ;	 0
public	       loadmsg		      ;	10
public	       UpdateMsg	      ;	20
public	       BaseMsg		      ; 20
public	       NoBaseMsg 	      ; 20
public	       NoServiceBaseMsg	      ; 20
public	       IntsMsg		      ; 20
public	       IdleMsg		      ; 20
public	       ParasMsg		      ; 20
public	       BufferMsg	      ; 20
public	       MemChunk		      ; 20
public	       HWMsgP		      ;	40
public	       HWMsgPC		      ;	40
public	       HWMsgR		      ;	40
public	       NoHanMes		      ;	40
public	       TestIntMes	      ;	40
public	       FoundIntMes	      ;	46
public	       ExeIntMes	      ;	47
public	       ESDI_Mes		      ;	47
public	       NoIntMes		      ;	47
public	       Diskmsg1		      ;	48
public	       IntCtrlMes	      ;	48
public	       RegMsg		      ;	49
public	       DosMsgA		      ;	50
public	       DosMsgN		      ;	50
public	       IntTblMes	      ;	55
public	       WaitStatus
public	       ServiceSearch
public	       MakeService
public	       AllocParam
public	       InitPointers
public	       InitWaitPointers
public	       ServiceAvailable
public	       ServiceAdded
public	       GotServiceInt 
public	       CallAmigaService
public	       ChannelMsg
public	       HandlerMsg    
public	       NextSlotMsg
public	       ExitSDMsg	
public	       UCountMsg
public	       RemoveMsg	
public	       ServiceDeleted	
public	       AmigaCallMsg
public	       AmigaAddMsg
public	       AmigaDeleteMsg
;
;
inmsg	       db   7,"Inthandler already loaded!",13,10,0
loadmsg        db   " Inthandler loaded at CS = ",0
BaseMsg        db   " JanusBase found at  = ",0
NoBaseMsg      db   " No Janus Memory Structure found!",13,10,0
NoServiceBaseMsg db " No Service Structure found!",13,10,0
IntsMsg        db   " Int Table offset    = ",0
ParasMsg       db   " Int Paras offset    = ",0
BufferMsg      db   " Buffer Mem found at = ",0
IdleMsg        db   " DOS Idle ",0
DosMsgA        db   " SW Int13 active ",13,10,0
DosMsgN        db   " SW Int13 ready ",13,10,0
HWMsgP	       db   " HW interrupt pending ",0
HWMsgPC	       db   " SW IntB active ",0
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
UpdateMsg      db   " PIC updated ! ",13,10,0	
WaitStatus     db   " Status : ",0		
MemChunk       db   " Memory Chunk found ! ",13,10,0	
ServiceSearch  db   " Search for ServiceData structure. ",13,10,0
MakeService    db   " Allocate ServiceData structure. ",13,10,0
AllocParam     db   " Allocate memory. ",13,10,0
InitPointers   db   " Init ServiceData structure. ",13,10,0
InitWaitPointers db " Init Wait pointer. ",13,10,0
ServiceAvailable db " Service now available. ",13,10,0	 
ServiceAdded   db   " Service added! ",13,10,0
CallAmigaService db " Call ServiceData at : ",0
GotServiceInt  db   " Got Service Interrupt ",13,10,0	 
ChannelMsg     db   " Channel # : ",0	
HandlerMsg     db   " Call Handler at : ",0	    
NextSlotMsg    db   " Checking next slot for channel number. ",13,10,0	
ExitSDMsg      db   " Leaving Service Dispatcher. ",13,10,0	
UCountMsg      db   " UserCount was : ",0
RemoveMsg      db   " Remove ServiceData structure at :	",0
ServiceDeleted db   " Service deleted!",13,10,0
AmigaCallMsg   db   " Amiga calls structure at : ",0	
AmigaAddMsg    db   " Amiga adds structure at : ",0	
AmigaDeleteMsg db   " Amiga deletes structure at : ",0	
;
;************ bill variables **********
;
		even
include	abvars.inc

;
	       db   '*****DATA END****'
dataend	       db   '*'
;
cseg	ends
 
end 

