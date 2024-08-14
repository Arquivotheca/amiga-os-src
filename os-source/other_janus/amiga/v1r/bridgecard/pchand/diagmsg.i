
;***********************************************************************
;
; asmsupp.i
;
; Copyright (c) 1986, Commodore Amiga Inc., All rights reserved
;
;***********************************************************************



		IFND	ASMSUPP_I
ASMSUPP_I	SET	1


		IFND	EXEC_LIBRARIES_I
		INCLUDE "exec/libraries.i"
		ENDC


CLEAR		MACRO
		MOVEQ	#0,\1
		ENDM

BHS		MACRO
		IFC	'\0',''
			BCC	\1
		ENDC
		IFNC	'\0',''
			BCC.\0	\1
		ENDC
		ENDM

BLO		MACRO
		IFC	'\0',''
			BCS	\1
		ENDC
		IFNC	'\0',''
			BCS.\0	\1
		ENDC
		ENDM

PUTMSG		MACRO
		IFGE	INFOLEVEL-\1
		XREF	KPutFmt
		XREF	JDiskName
		PEA	JDiskName
		MOVEM.L D0/D1/A0/A1,-(SP)
		LEA	16(sp),a1
		LEA	MSG\@(PC),a0
		JSR	KPutFmt
		MOVEM.L (SP)+,D0/D1/A0/A1
		ADDQ.L	#4,SP
		BRA.S	SKIP\@
MSG\@		DC.B	\2,10,0
		DS.W	0
SKIP\@:
		ENDC
		ENDM

INFOMSG 	MACRO
		IFGE	INFOLEVEL-\1
		XREF	KPutFmt
		MOVEM.L D0/D1/A0/A1,-(SP)
		LEA	16(sp),a1
		LEA	MSG\@(PC),a0
		JSR	KPutFmt
		MOVEM.L (SP)+,D0/D1/A0/A1
		BRA.S	SKIP2\@
MSG\@		DC.B	\2,10,0
		DS.W	0
SKIP2\@:
		ENDC
		ENDM



		ENDC	!ASMSUPP_I

