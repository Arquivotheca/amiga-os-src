
*************************************************************************
*									*
*	Copyright (C) 1985, Commodore Amiga Inc.  All rights reserved.	*
*									*
*************************************************************************

*************************************************************************
*
* test.asm
*
* Source Control
* ------ -------
* 
* $Header: test.asm,v 27.2 85/06/26 12:32:43 neil Exp $
*
* $Locker: neil $
*
* $Log:	test.asm,v $
* Revision 27.2  85/06/26  12:32:43  neil
* Fixed format problems.
* 
* Revision 27.1  85/06/24  13:37:58  neil
* Upgrade to V27
* 
* Revision 26.1  85/06/17  15:13:36  neil
* *** empty log message ***
* 
* 
*************************************************************************

	SECTION section

******* Included Files ***********************************************

	NOLIST
	INCLUDE 'exec/types.i'
	INCLUDE 'exec/nodes.i'
	INCLUDE 'exec/lists.i'
	INCLUDE 'exec/ports.i'
	INCLUDE 'exec/libraries.i'
	INCLUDE 'exec/io.i'
	INCLUDE 'exec/devices.i'
	INCLUDE 'exec/tasks.i'
	INCLUDE 'exec/interrupts.i'
	INCLUDE	'exec/execbase.i'
	INCLUDE	'exec/memory.i'
	INCLUDE	'exec/strings.i'

	INCLUDE 'resources/disk.i'

	INCLUDE 'devices/timer.i'
	INCLUDE 'devices/bootblock.i'

	INCLUDE 'libraries/dos.i'

	INCLUDE 'trackdisk.i'
	INCLUDE 'asmsupp.i'
	INCLUDE 'internal.i'
	INCLUDE 'mess1.i'
	LIST


******* Imported Names ***********************************************

*------ Tables -------------------------------------------------------

*------ Functions ----------------------------------------------------

*------ System Library Functions -------------------------------------

	EXTERN_LIB AllocMem
	EXTERN_LIB AllocSignal
	EXTERN_LIB CloseDevice
	EXTERN_LIB Debug
	EXTERN_LIB DoIO
	EXTERN_LIB FreeMem
	EXTERN_LIB FreeSignal
	EXTERN_LIB OpenDevice
	EXTERN_LIB RemDevice
	EXTERN_LIB SendIO
	EXTERN_LIB SetSignal
	EXTERN_LIB Wait

	XREF	_AbsExecBase

******* Exported Names ***********************************************

*------ Functions ----------------------------------------------------

	XDEF	_DiskTest
	XDEF	cleartest
	XDEF	discchange
	XDEF	discwait
	XDEF	formatdisc
	XDEF	motor_test
	XDEF	readdisc
	XDEF	stats
	XDEF	test_end
	XDEF	test_top
	XDEF	updatetest
	XDEF	writedisc
	XDEF	writepattern
	XDEF	kickstart

*------ Data ---------------------------------------------------------


******* Local Definitions ********************************************

	STRUCTURE DATASTRUCT,0
	STRUCT	D_IO0,IOTD_SIZE
	STRUCT	D_SEC,TD_SECTOR*NUMSECS
	STRUCT	D_PORT,MP_SIZE
	STRUCT	D_LABEL,TD_LABELSIZE*NUMSECS
	STRUCT	D_INTSERV0,IS_SIZE
	STRUCT	D_INTSERV1,IS_SIZE
	ULONG	D_MAXSECTOR
	ULONG	D_MAXTRACK
	APTR	D_IOB
	LABEL	D_SIZE

COM_FORMAT	EQU 0
COM_READSEC	EQU 1
COM_READWRITE	EQU 2
COM_REPETATIVE	EQU 3
COM_READTRACK	EQU 4
COM_WRITEPATRN	EQU 5
COM_SEEKTEST	EQU 6
COM_SHORTDISC	EQU 7
COM_MOTORTEST	EQU 8
COM_DISCCHANGE	EQU 9
COM_DISCWAIT	EQU 10
COM_UPDATE	EQU 11
COM_CLEAR	EQU 12
COM_KICKSTART	EQU 13
COM_LABEL	EQU 14

BSRLIB	MACRO	* &target,&scratch,&newA6
		MOVE.L	A6,-(SP)
		MOVE.L	\3,A6
		BSR	\1
		MOVE.L	(SP)+,A6
		ENDM

*------ Registers -----------------------------------------------------
*
*	A2 -- Track Disk data pointer
*
*
*----------------------------------------------------------------------



_DiskTest:
*		;------ save eight registers
		MOVEM.L	D2/D3/D6/D7/A2/A3/A4/A6,-(SP)
		LINK	A5,#0
		MOVE.L	_AbsExecBase,A6
		MOVE.L	4+9*4(SP),D6
		MOVE.L	8+9*4(SP),D7


		MOVE.L	#D_SIZE,D0
		MOVE.L	#MEMF_CLEAR,D1
		CALLSYS AllocMem

		TST.L	D0
		BNE.S	test_havemem

		PUTFMT	nomemmsg,0
		RTS

nomemmsg:	STRINGR <'DiscTest: can''t get memory'>

test_havemem:
		MOVE.L	D0,A2

		LEA	D_PORT(A2),A0
		MOVE.L	A0,D_IO0+MN_REPLYPORT(A2)
		LEA	MP_MSGLIST(A0),A0
		NEWLIST	A0

		MOVEQ	#-1,D0
		CALLSYS	AllocSignal

		MOVE.L	ThisTask(A6),D_PORT+MP_SIGTASK(A2)
		MOVE.B	D0,D_PORT+MP_SIGBIT(A2)
		LEA	tdName,A0
		MOVE.L	A0,D_PORT+LN_NAME(A2)

		IFGE	INFO_LEVEL-50
		MOVE.L	D6,-(SP)
		INFOMSG	50,<'%s/test: d6 is 0x%lx'>
		ADDQ.L	#4,SP
		ENDC

*		;------ Open the device
		LEA	D_IO0(A2),A1
		LEA	tdName,A0
		MOVE.L	A0,LN_NAME(A1)

		CLEAR	D0
		CLEAR	D1
		SWAP	D6
		MOVE.W	D6,D0
		SWAP	D6

		CALLSYS OpenDevice

		TST.L	D0
		BEQ.S	test_OpenOk

		PEA	0
		SWAP	D6
		MOVE.W	D6,2(SP)
		SWAP	D6

		PEA	0
		MOVE.B	D_IO0+IO_ERROR(A2),3(SP)

		PUTFMT	MesOpenFailed,0
		ADDQ.L	#8,SP
		BRA	test_done

test_OpenOk:

		MOVE.L	D_IO0+IO_DEVICE(A2),A4

		MOVE.L	A2,-(SP)
		MOVE.L	A4,-(SP)
		PUTFMT	Mes0,0
		ADDQ.L	 #8,SP

		LEA	D_IO0(A2),A1
		MOVE.W	#TD_GETNUMTRACKS,IO_COMMAND(A1)
		CALLSYS	DoIO
		MOVE.L	D_IO0+IO_ACTUAL(A2),D0
		MOVE.L	D0,D_MAXTRACK(A2)
		MULU	#NUMSECS,D0
		MOVE.L	D0,D_MAXSECTOR(A2)


test_top:

*		;------ store away the io block
		LEA	D_IO0(A2),A0
		MOVE.L	A0,D_IOB(A2)


*		;-- figure out what we should do
*		;-- command 0: format the disc, then do a read check
		CMP.W	#COM_FORMAT,D6
		BNE.S	command10
		BSR	formatdisc
		BSR	writedisc
		BSR	readdisc
		BRA	test_end

command10:
*		;-- command 1: read the disc sector by sector
		CMP.W	#COM_READSEC,D6
		BNE.S	command20
		BSR	readdisc
		BRA	test_end

command20:
*		;-- command 2: write the disc, then read
		CMP.W	#COM_READWRITE,D6
		BNE.S	command30
		BSR	writedisc
		BSR	readdisc
		BRA	test_end

command30:
*		;-- command 3: read the disc D7 times, then print out
*		;--	statistics
		CMP.W	#COM_REPETATIVE,D6
		BNE.S	command40
command3_loop:

		MOVE.W	D7,-(SP)
		PUTFMT	Mes9,0
		ADDQ.L	#2,SP

		MOVE.L	D7,D0
		AND.L	#$03,D0
		BEQ.S	command3_write

*		;-- 3 out of four times do a read
		BSR	readdisc
		BRA.S	command3_char
command3_write:
		BSR	writedisc
command3_char:
*		;-- read in a char (if present) and do the correct thing
		BSR	mayget
		CMP.W	#-1,D0
		BEQ.S	command3_end

		CMP.B	#$071,D0			; 'q'
		BEQ	test_end
		CMP.B	#$07F,D0			; DEL
		BEQ	test_end

		CMP.B	#$073,D0			; 's'
		BNE.S	command3_end
		BSR	stats

command3_end:
		DBF	D7,command3_loop

		BRA	test_end

command40:
*		;-- command 1: read the disc sector by sector
		CMP.W	#COM_READTRACK,D6
		BNE.S	command50
		BSR	readdisc
		BRA	test_end

command50:
		CMP.W	#COM_WRITEPATRN,D6
		BNE.S	command60

	BRA	command_err

		MOVE.L	D_IOB(A2),A1
		MOVE.L	IO_UNIT(A1),A3
		MOVE.W	#TD_MOTOR,IO_COMMAND(A1)
		MOVE.L	#1,IO_LENGTH(A1)
		CALLSYS DoIO

		BSR	writepattern

		BRA	test_end

command60:
		CMP.W	#COM_SEEKTEST,D6
		BNE.S	command70

		MOVE.L	D_IOB(A2),A1
		MOVE.W	#TD_MOTOR,IO_COMMAND(A1)
		MOVE.L	#1,IO_LENGTH(A1)
		CALLSYS DoIO

		MOVE.L	D_IOB(A2),A1
		MOVE.W	#TD_SEEK,IO_COMMAND(A1)
		CLEAR	D0
		MOVE.W	D7,D0
		MULU	#NUMSECS,D0
		MOVEQ	#TD_SECSHIFT,D1
		LSL.L	D1,D0
		MOVE.L	D0,IO_OFFSET(A1)
		CALLSYS DoIO

		BSR	getchar

		BRA	test_end

command70:
*		;-- just like a command #3, but of limited length
		CMP.W	#COM_SHORTDISC,D6
		BNE.S	command80
		MOVE.L	D2,D_MAXSECTOR(A2)
		BRA	command3_loop

command80:
*		;-- Test the motor
		CMP.W	#COM_MOTORTEST,D6
		BNE.S	command90
		BRA	motor_test
command90:
*		;-- Test the motor
		CMP.W	#COM_DISCCHANGE,D6
		BNE.S	command100
		BRA	discchange

command100:
*		;-- Test the motor
		CMP.W	#COM_DISCWAIT,D6
		BNE.S	command110

		BRA	discwait

command110:
*		;-- Test the motor
		CMP.W	#COM_UPDATE,D6
		BNE.S	command120
		BRA	updatetest

command120:
*		;-- Test the motor
		CMP.W	#COM_CLEAR,D6
		BNE.S	command130
		BRA	cleartest

command130:
*		;-- Test the motor
		CMP.W	#COM_KICKSTART,D6
		BNE.S	command140
		BRA	kickstart

command140:
*		;-- Test the motor
		CMP.W	#COM_LABEL,D6
		BNE.S	command150
		BRA	label

command150:
command_err:
*		;-- default -- just go away
		MOVE.L	D0,-(SP)
		PUTFMT	Mes6,0
		ADDQ.L	#4,SP
		BRA	test_end



formatdisc:
		MOVE.L	D2,-(SP)
		CLEAR	D2

formatdisc_loop:
		BSR	formattrack

		ADDQ.L	#1,D2
		CMP.L	D_NUMTRACKS(A2),D2
		BLT.S	formatdisc_loop

formatdisc_end:
		MOVE.L	(SP)+,D2
		RTS

formattrack:
		BSR	formatfill

		;------ now write it out
		MOVE.L	D_IOB(A2),A1
		LEA	D_SEC(A2),A0
		MOVE.L	A0,IO_DATA(A1)
		LEA	D_LABEL(A2),A0
		MOVE.L	A0,IOTD_SECLABEL(A1)
		MOVE.L	#-1,IOTD_COUNT(A1)
		MOVE.L	#TD_SECTOR*NUMSECS,IO_LENGTH(A1)

		;---!!! should be ETD_FORMAT, but extended format is broke
		MOVE.W	#TD_FORMAT,IO_COMMAND(A1)
		MOVE.L	D2,D0
		MULU	#NUMSECS,D0
		MOVEQ	#TD_SECSHIFT,D1
		LSL.L	D1,D0
		MOVE.L	D0,IO_OFFSET(A1)

		CALLSYS	DoIO

		CLEAR	D0
		MOVE.L	D_IOB(A2),A1
		MOVE.B	IO_ERROR(A1),D0
		BEQ.S	formattrack_end

		MOVE.L	D2,-(SP)
		MOVE.L	D0,-(SP)
		MOVE.L	D0,-(SP)
		EXT.W	D0
		EXT.L	D0
		MOVE.L	D0,4(SP)
		INFOMSG	1,<'%s/format: error %ld (%ld) on track %ld'>
		LEA	12(SP),SP

formattrack_end:
		RTS

formatfill:
		MOVEM.L	D3/D4,-(SP)
		MOVE.L	D2,D3
		MULU	#NUMSECS,D3
		MOVEQ	#NUMSECS,D4

		LEA	D_SEC(A2),A0
		LEA	D_LABEL(A2),A1

formatfill_loop:
		MOVE.W	D3,D1
		SWAP	D1
		MOVE.W	D3,D1
		NOT.W	D1

		MOVE.W	#(TD_SECTOR>>2)-1,D0
10$:
		MOVE.L	D1,(A0)+
		DBRA	D0,10$

		SWAP	D1
		MOVE.W	#(TD_LABELSIZE>>2)-1,D0
20$:
		MOVE.L	D1,(A1)+
		DBRA	D0,20$

		;------ bump loop counters
		ADDQ.L	#1,D3
		SUBQ.L	#1,D4
		BNE.S	formatfill_loop

formatfill_end:
		MOVEM.L	(SP)+,D3/D4
		RTS
		


writedisc:
*		;-- initialize the sector number
		MOVEQ	#0,D5

writesector:

*		;-- first fill the buffer with its sector address
		LEA	D_SEC(A2),A0
		MOVE.W	D5,D1
		SWAP	D1
		MOVE.W	D5,D1
		NOT.W	D1

		MOVE.W	#(TD_SECTOR>>2)-1,D0
writesec_10:
		MOVE.L	D1,(A0)+
		DBF	D0,writesec_10

		LEA	D_LABEL(A2),A0
		SWAP	D1
		MOVE.W	#(TD_LABELSIZE>>2)-1,D0
writesec_11:
		MOVE.L	D1,(A0)+
		DBF	D0,writesec_11


*		;-- now write it out
		MOVE.L	D_IOB(A2),A1
		MOVE.W	#ETD_WRITE,IO_COMMAND(A1)
		LEA	D_SEC(A2),A0
		MOVE.L	A0,IO_DATA(A1)
		LEA	D_LABEL(A2),A0
		MOVE.L	A0,IOTD_SECLABEL(A1)
		MOVE.L	#-1,IOTD_COUNT(A1)
		MOVE.L	D5,D0
		MOVEQ	#TD_SECSHIFT,D1
		LSL.L	D1,D0
		MOVE.L	D0,IO_OFFSET(A1)
		MOVE.L	#TD_SECTOR,IO_LENGTH(A1)

		CALLSYS DoIO

		MOVE.L	D_IOB(A2),A1
		TST.B	IO_ERROR(A1)
		BEQ.S	writesec_next

		MOVE.W	D5,-(SP)
		CLEAR	D0
		MOVE.B	IO_ERROR(A1),D0
		EXT.W	D0
		MOVE.W	D0,-(SP)
		PUTFMT	Mes4,0
		ADDQ.L	#4,SP


writesec_next:

		ADDQ.L	#1,D5
		CMP.L	D_MAXSECTOR(A2),D5
		BLT	writesector

		RTS

readdisc:
*		;-- D5 holds the current sector number
		MOVEQ	#0,D5

readsector:
		MOVE.L	D_IOB(A2),A1
		MOVE.W	#ETD_READ,IO_COMMAND(A1)
		LEA	D_SEC(A2),A0
		MOVE.L	A0,IO_DATA(A1)
		LEA	D_LABEL(A2),A0
		MOVE.L	A0,IOTD_SECLABEL(A1)
		MOVE.L	#-1,IOTD_COUNT(A1)
		MOVE.L	D5,D0
		MOVEQ	#TD_SECSHIFT,D1
		LSL.L	D1,D0
		MOVE.L	D0,IO_OFFSET(A1)
		MOVE.L	#TD_SECTOR,IO_LENGTH(A1)

		CALLSYS DoIO

		MOVE.L	D_IOB(A2),A1
		TST.B	IO_ERROR(A1)
		BEQ.S	readsec_checksec

		MOVE.W	D5,-(SP)
		CLEAR	D0
		MOVE.B	IO_ERROR(A1),D0
		EXT.W	D0
		MOVE.W	D0,-(SP)
		PUTFMT	Mes2,0
		ADDQ.L	#4,SP

		BRA	readsec_next

readsec_checksec:

*		;-- check to see if the right stuff is in the sector
		LEA	D_SEC(A2),A0
		MOVE.W	D5,D1
		SWAP	D1
		MOVE.W	D5,D1
		NOT.W	D1

		MOVE.W	#(TD_SECTOR>>2)-1,D0
readsec_ch10:
		CMP.L	(A0)+,D1
		DBNE	D0,readsec_ch10

*		;-- see if we did the whole sector
		CMP.L	-(A0),D1
		BEQ.S	readsec_checklabel

*		;-- we failed
		MOVE.L	D1,-(SP)		; what we expected
		MOVE.L	(A0),-(SP)		; what we got
		SUB.W	#(TD_SECTOR>>2)-1,D0
		NEG.W	D0
		LSL.W	#2,D0
		MOVE.W	D0,-(SP)		; byte offset
		MOVE.L	D5,-(SP)		; sector number
		PUTFMT	Mes3,0
		LEA	10(SP),SP
		MOVE.L	(SP)+,D1

readsec_checklabel:
*		;-- check to see if the right stuff is in the label
		LEA	D_LABEL(A2),A0
		SWAP	D1

		MOVE.W	#(TD_LABELSIZE>>2)-1,D0
readsec_ch20:
		CMP.L	(A0)+,D1
		DBNE	D0,readsec_ch20

*		;-- see if we did the whole sector
		CMP.L	-(A0),D1
		BEQ.S	readsec_next

*		;-- we failed
		MOVE.L	D1,-(SP)		; what we expected
		MOVE.L	(A0),-(SP)		; what we got
		SUB.W	#(TD_LABELSIZE>>2)-1,D0
		NEG.W	D0
		LSL.W	#2,D0
		MOVE.W	D0,-(SP)		; byte offset
		MOVE.L	D5,-(SP)		; sector number
		PUTFMT	MesD,0
		LEA	14(SP),SP



readsec_next:
		CMP.L	#COM_READTRACK,D6
		BNE.S	readsec_next10
		ADD.L	#NUMSECS,D5
		BRA.S	readsec_next20
readsec_next10:
		ADDQ.L	#1,D5				; bump sector number
readsec_next20:

		CMP.L	D_MAXSECTOR(A2),D5
		BLT	readsector

		RTS


writepattern:
*		;------ write a worst case pattern on the disc
*		;------
*		;------ D2 -- loop counter
*		;------ A0 -- buffer pointer
*		;------ A3 -- unit pointer


*		;------ For each track, fill the buffer with 1010001's
		MOVE.W	#(MFM_TRKBUF/6)-1,D2
		MOVE.L	TDU_BUFPTR(A3),A0
		LEA	TDB_BUF(A0),A0

wp_fillloop:
		MOVE.L	#$0A28A28A2,(A0)+
		MOVE.W	#$08A28,(A0)+
		DBF	D2,wp_fillloop

*		;------ load the unit pointer
		MOVE.L	D_NUMTRACKS(A2),D2
		SUBQ.L	#1,D2

wp_trackloop:
*		;------ seek to the right place on the disc
		MOVE.L	D_IOB(A2),A1
		MOVE.W	#TD_SEEK,IO_COMMAND(A1)
		MOVE.L	D2,IO_LENGTH(A1)
		CALLSYS DoIO

*		;------ write the track out
		MOVE.L	TDU_BUFPTR(A3),A0
		LEA	TDB_BUF(A0),A0
		MOVE.L	#MFM_MAXTRACK,D0
*****		BSRLIB	TDTrkWrite,Ax,A4

		DBF	D2,wp_trackloop

		RTS

test_end:

		BSR	stats

		CLEAR	D0
		MOVE.B	D_PORT+MP_SIGBIT(A2),D0
		CALLSYS	FreeSignal

		MOVE.L	D_IOB(A2),A1
		CMP.L	#-1,IO_DEVICE(A1)
		BEQ.S	test_done

		CALLSYS CloseDevice

test_done:

		MOVE.L	A2,A1
		MOVE.L	#D_SIZE,D0
		CALLSYS FreeMem

		MOVE.L	D6,D0

		UNLK	A5
		MOVEM.L	(SP)+,D2/D3/D6/D7/A2/A3/A4/A6

		RTS


Mes0:	STRINGR <'OpenDevice returned dev @ 0x%lx, loc buf @ 0x%lx'>
Mes1:	STRINGR <'Track 0x%lx, buf 0x%lx, filled with %d'>
Mes2:	STRINGR <'Error # %d on TDIO read of sec %d'>
Mes3:	STRINGR <'Failed compare sec %4ld off %d got %08lx vs %08lx'>
Mes4:	STRINGR <'Error # %d on TDIO write of sec %d'>
Mes5:	STRINGR <'We are running on a Paula'>
Mes6:	STRINGR <'Command number (%ld) out of range'>
Mes7:	DC.B	10
	STRINGR <'Outputing error statistics for unit %ld'>
Mes8:	STRINGR <'    track %3d:  %3d hard, %3d soft (%3d errs)'>
Mes9:	STRINGR <'%d iterations to go'>
MesA:	STRINGR <'  totals:	  %3ld hard, %3ld soft (%3ld errs)'>
MesB:	STRINGR <'		  --------  -------- ----------'>
MesD:	STRINGR <'Failed label check: sec %4ld off %3d got %08lx vs %08lx'>
MesE:	STRINGR <'Disc Change [%ld: #%-3ld state %3ld. prot %ld]'>
MesF:	STRINGR <'Disc Change [#%-3ld state %3ld]'>

MesOpenFailed: STRINGR <'OpenDevice failed (%ld) for unit %ld'>
tdName: TD_NAME


getchar:
		BSR	mayget
		TST.L	D0
		BMI	getchar
		RTS

*		; mayget() -- returns char if one there, else -1
mayget:
		CLEAR	D0
		MOVE.L	SIGBREAKF_CTRL_C,D1
		CALLSYS	SetSignal

		MOVE.L	D0,D1
		CLEAR	D0
		TST.L	D1
		BNE.S	mayget_end

		MOVEQ	#-1,D0

mayget_end:
		RTS


	IFNE ErrorMonitor
stats:

		MOVEM.L D2/D3/D4/D5/D6/D7/A3,-(SP)

		CLEAR	D7

stats_outloop
		BSR	stats_outter

		ADDQ.L	#1,D7
		CMP.L	#3,D7
		BLE.S	stats_outloop
		
		MOVEM.L (SP)+,D2/D3/D4/D5/D6/A3
		RTS
stats_outter:
*		;-- print out the error statistics
		CLEAR	D6
		CLEAR	D5
		CLEAR	D4
		CLEAR	D3

		MOVE.L	D7,D0
		LSL.L	#2,D0
		LEA	TDU0(A4),A0
		MOVE.L	0(A0,D0),D0
		BEQ.S	10$

		MOVE.L	D0,A3
		MOVE.L	D7,-(SP)
		PUTFMT	Mes7,0
		ADDQ.L	#4,SP
*		;-- A3 -- unit ptr
*		;-- D2 -- loop counter
		MOVEQ	#0,D2
		BSR	stats_inner

10$:
		RTS

stats_inner:

stat_loop:
		TST.W	TDU_SOFTERRS(A3)
		BNE.S	stat_signif

		TST.W	TDU_HARDERRS(A3)
		BEQ	stat_next

stat_signif:
		MOVE.W	TDU_NUMERRS(A3),D6
		ADD.L	D6,D5
		MOVE.W	D6,-(SP)

		MOVE.W	TDU_SOFTERRS(A3),D6
		ADD.L	D6,D4
		MOVE.W	D6,-(SP)

		MOVE.W	TDU_HARDERRS(A3),D6
		ADD.L	D6,D3
		MOVE.W	D6,-(SP)

		MOVE.W	D2,-(SP)
		PUTFMT	Mes8,0
		ADDQ.L	#8,SP

stat_next:
		ADDQ.W	#1,D2
		ADDQ.L	#2,A3
		CMP.W	D_MAXTRACKS(A2),D2
		BLT	stat_loop

*		;------ see if we should print out summary statistics
		TST.L	D5
		BEQ.S	stat_innerend

stat_summary:
*		;------ we has some errors, so do the summary info
		PUTFMT	MesB,0

		MOVEM.L D3/D4/D5,-(SP)
		PUTFMT	MesA,0
		ADD.L	#12,SP

stat_innerend:

		RTS
	ENDC ErrorMonitor
	IFEQ ErrorMonitor
stats:
	    RTS
	ENDC ErrorMonitor

motor_test:
		MOVE.L	#10,D2

motor_loop:
		MOVE.L	D_IOB(A2),A1
		MOVE.L	D2,D1
		AND.L	#1,D1
		MOVE.L	D1,IO_LENGTH(A1)
		MOVE.W	#TD_MOTOR,IO_COMMAND(A1)
		CALLSYS DoIO

		DBF	D2,motor_loop

		BRA	test_end

discchange:
		MOVE.L	D_IOB(A2),A1
		CLR.L	IOTD_COUNT(A1)

discchange_loop:
		MOVE.L	D_IOB(A2),A1
		MOVE.W	#ETD_READ,IO_COMMAND(A1)
		LEA	D_SEC(A2),A0
		MOVE.L	A0,IO_DATA(A1)
		MOVE.L	#TD_SECTOR*30,IO_OFFSET(A1)
		MOVE.L	#TD_SECTOR,IO_LENGTH(A1)
		CLR.L	IOTD_SECLABEL(A1)

		CALLSYS DoIO

		MOVE.L	D_IOB(A2),A1
		CMP.B	#TDERR_DiskChanged,IO_ERROR(A1)
		BNE.S	discchange_mayget

*		;------ we had a change error
		MOVE.W	#TD_CHANGESTATE,IO_COMMAND(A1)
		CALLSYS DoIO

		MOVE.L	D_IOB(A2),A1
		MOVE.L	IO_ACTUAL(A1),-(SP)
		MOVE.W	#TD_CHANGENUM,IO_COMMAND(A1)
		CALLSYS DoIO

		MOVE.L	D_IOB(A2),A1
		MOVE.L	IO_ACTUAL(A1),IOTD_COUNT(A1)
		MOVE.L	IO_ACTUAL(A1),-(SP)

		PUTFMT	MesF,0
		ADDQ.L	#8,SP

discchange_mayget:
		BSR	mayget
		TST.L	D0
		BMI	discchange_loop

		BRA	test_end




updatetest:
		MOVE.L	D_IOB(A2),A1
		MOVE.W	#CMD_READ,IO_COMMAND(A1)
		LEA	D_SEC(A2),A0
		MOVE.L	A0,IO_DATA(A1)
		MOVE.L	#TD_SECTOR*30,IO_OFFSET(A1)
		MOVE.L	#TD_SECTOR,IO_LENGTH(A1)

		CALLSYS DoIO

updatetest_loop:
		MOVE.L	D_IOB(A2),A1
		MOVE.W	#CMD_WRITE,IO_COMMAND(A1)
		LEA	D_SEC(A2),A0
		MOVE.L	A0,IO_DATA(A1)
		MOVE.L	#TD_SECTOR*30,IO_OFFSET(A1)
		MOVE.L	#TD_SECTOR,IO_LENGTH(A1)

		CALLSYS DoIO

		MOVE.L	D_IOB(A2),A1
		MOVE.W	#CMD_UPDATE,IO_COMMAND(A1)

		CALLSYS DoIO

		BSR	mayget
		TST.L	D0
		BMI.S	updatetest_loop

		BRA	test_end

cleartest:
		MOVE.L	D_IOB(A2),A1
		MOVE.W	#CMD_READ,IO_COMMAND(A1)
		LEA	D_SEC(A2),A0
		MOVE.L	A0,IO_DATA(A1)
		MOVE.L	#TD_SECTOR*30,IO_OFFSET(A1)
		MOVE.L	#TD_SECTOR,IO_LENGTH(A1)

		CALLSYS DoIO

		MOVE.L	D_IOB(A2),A1
		MOVE.W	#CMD_CLEAR,IO_COMMAND(A1)

		CALLSYS DoIO

		BSR	mayget
		TST.L	D0
		BMI.S	cleartest

		BRA	test_end

discwait:
		MOVE.L	D_IOB(A2),A1
		MOVE.W	#TD_REMOVE,IO_COMMAND(A1)
		LEA	D_INTSERV0(A2),A0
		MOVE.L	A0,IO_DATA(A1)
		MOVE.L	A2,IS_DATA(A0)
		MOVE.L	#discwait_softint,IS_CODE(A0)
		CALLSYS DoIO

discwait_loop:
		BSR	mayget
		TST.L	D0
		BMI	discwait_loop


		MOVE.L	D_IOB(A2),A1
		MOVE.W	#TD_REMOVE,IO_COMMAND(A1)
		MOVE.L	#0,IO_DATA(A1)
		CALLSYS DoIO

		BRA	test_end


discwait_softint:
		MOVEM.L	A2/A3,-(SP)
		MOVE.L	A1,A2
		
*		;------ Check out unit one
		MOVE.L	D_IOB(A2),A1
		MOVE.W	#TD_PROTSTATUS,IO_COMMAND(A1)
		BSR.S	discwait_DoIO
		MOVE.L	IO_ACTUAL(A1),-(SP)

		MOVE.W	#TD_CHANGESTATE,IO_COMMAND(A1)
		BSR.S	discwait_DoIO
		MOVE.L	IO_ACTUAL(A1),-(SP)

		MOVE.W	#TD_CHANGENUM,IO_COMMAND(A1)
		BSR.S	discwait_DoIO
		MOVE.L	IO_ACTUAL(A1),-(SP)

		PEA	0
		SWAP	D6
		MOVE.W	D6,2(SP)
		SWAP	D6

		PUTFMT	MesE,0
		LEA	16(SP),SP

		MOVEM.L	(SP)+,A2/A3
		RTS

discwait_DoIO:
		MOVE.L	A1,-(SP)
		CALLSYS	DoIO
		MOVE.L	(SP)+,A1
		RTS

ROMSIZE		EQU	(256*1024)
kickstart:
		;------ registers used:
		;------  A3 -- address base
		;------  D2 -- offset
		;------  D3 -- loop counter

		MOVEM.L	A3/D2/D3,-(SP)

		;------ first set up the memory locations that we need to
		MOVE.L	D7,A0
		MOVE.L	#ROMSIZE,D1
		MOVE.l	A0,A1
		ADD.L	#ROMSIZE-$18,A1
		CLR.L	(A1)+
		MOVE.L	D1,(A1)
		
		;------ then compute the checksum
		BSR	CheckSumLong

		;------ and store it away
		MOVE.L	D7,A3
		MOVE.L	A3,A1
		ADD.L	#ROMSIZE-$18,A1
		MOVE.L	D0,(A1)

		IFGE	INFO_LEVEL-1
		MOVE.L	D0,-(SP)

		;------ checksum it again, just for fun
		MOVE.L	A3,A0
		MOVE.L	#ROMSIZE,D1
		BSR	CheckSumLong
		MOVE.L	D0,-(SP)
		INFOMSG	1,<'%s/kickstart:  checksum 0x%lx yields 0x%lx'>
		ADDQ.L	#8,SP
		ENDC

		;------ write out the boot block
		MOVE.L	#(TD_SECTOR-1)>>2,D0
		LEA	D_SEC(A2),A0
		CLEAR	D1
kick_clear:
		MOVE.L	D1,(A0)+
		DBRA	D0,kick_clear

		MOVE.L	D_IOB(A2),A1
		MOVE.W	#CMD_WRITE,IO_COMMAND(A1)
		LEA	D_SEC(A2),A0
		MOVE.L	kick_data,(A0)
		MOVE.L	A0,IO_DATA(A1)
		MOVE.L	#TD_SECTOR,IO_LENGTH(A1)
		MOVE.L	D1,IO_OFFSET(A1)
		CALLSYS	DoIO
		TST.L	D0
		BNE	kick_error


		;------ now write out the image
		MOVE.L	#TD_SECTOR,D2
		CLEAR	D3
		MOVE.L	D_IOB(A2),A1
		MOVE.W	#CMD_WRITE,IO_COMMAND(A1)
		LEA	D_SEC(A2),A0
		MOVE.L	A0,IO_DATA(A1)
		MOVE.L	#TD_SECTOR,IO_LENGTH(A1)

kick_loop:
		MOVE.L	A3,A0		; source
		LEA	D_SEC(A2),A1	; dest
		MOVE.L	#TD_SECTOR,D0	; count
		BSR	bcopy

	IFGE	INFO_LEVEL-40
	MOVE.L	D2,D0
	MOVEQ	#TD_SECSHIFT,D1
	LSR.L	D1,D0
	MOVE.L	D0,-(SP)
	MOVE.L	A3,-(SP)
	INFOMSG	1,<'%s/kickstart: writting 0x%lx at offset 0x%lx'>
	ADDQ.L	#8,SP
	ENDC

		MOVE.L	D_IOB(A2),A1	; io request block
		MOVE.L	D2,IO_OFFSET(A1)
		MOVE.W	#CMD_WRITE,IO_COMMAND(A1)
		LEA	D_SEC(A2),A0
		MOVE.L	A0,IO_DATA(A1)
		MOVE.L	#TD_SECTOR,IO_LENGTH(A1)
		CALLSYS	DoIO
		TST.L	D0
		BNE	kick_error

		ADD.L	#TD_SECTOR,A3
		ADD.L	#TD_SECTOR,D2
		ADDQ.L	#1,D3

		CMP.L	#512,D3
		BLT	kick_loop

		MOVE.L	D_IOB(A2),A1
		MOVE.W	#CMD_UPDATE,IO_COMMAND(A1)
		CALLSYS	DoIO

		MOVE.L	D_IOB(A2),A1
		MOVE.W	#TD_MOTOR,IO_COMMAND(A1)
		MOVE.L	#0,IO_LENGTH(A1)
		CALLSYS	DoIO

kick_end:
		MOVEM.L	(SP)+,A3/D2/D3
		BRA	test_end

kick_data:	BBID_KICK

kick_error:
		MOVE.L	D3,-(SP)
		PEA	0
		MOVE.L	D_IOB(A2),A1
		MOVE.B	IO_ERROR(A1),3(SP)
		PUTMSG	1,<'%s/kickstart: error %ld at offset %ld'>
		ADDQ.L	#8,SP

		BRA	kick_end

bcopy:
		SUBQ.W	#1,D0
bcopy_loop:
		MOVE.B	(A0)+,(A1)+
		DBRA	D0,bcopy_loop

		RTS
		

CheckSumLong:
*
* a0 -- starting address
* d1 -- number of bytes (must be divisibly by four)
*
* d0 -- sum.  If zero, then it is OK.
*
* -- try this new and improved additive-wraparound-carry checksum
		clr.l   d0		* flush it for the results

CheckSumLong_loop:
		add.l       (a0)+,d0    * the actual data add

		bcc.s       CheckSumLong_cont   * if carry clear then skip
		add.l       #1,d0           * wrap around the carry bit!
 
CheckSumLong_cont:
		subq.l	#4,d1
		bne.s	CheckSumLong_loop
 
		not.l	d0
		rts

labeldata:	BBID_DOS

label:
		INFOMSG	1,<'%s/label: writting from $40000'>

		LEA	$40000,A0
		MOVE.L	#TD_SECTOR*2,D1
		BSR	CheckSumLong

		MOVE.L	D0,$40004

		MOVE.L	D_IOB(A2),A1
		MOVE.W	#CMD_WRITE,IO_COMMAND(A1)
		MOVE.L	#$40000,IO_DATA(A1)
		MOVE.L	#TD_SECTOR*2,IO_LENGTH(A1)
		MOVE.L	#0,IO_OFFSET(A1)

		MOVE.L	labeldata,$40000

		CALLSYS	DoIO
		TST.L	D0
		BEQ	label_update

		MOVE.L	D0,-(SP)
		INFOMSG	1,<'%s/label: error %ld'>
		ADDQ.L	#4,SP

label_update:
		MOVE.L	D_IOB(A2),A1
		MOVE.W	#CMD_UPDATE,IO_COMMAND(A1)
		CALLSYS	DoIO

label_end:
		JMP	test_end

	END
