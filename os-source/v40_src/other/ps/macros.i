* This file contains stack definition and macro routines
* March 1991 P.J. © CBM-Amiga
* Submitted to RCS on 05/APR/91

	IFND MACROS_I
MACROS_I	equ	1

FENCE 	equ $FFFFFFFF+dpsc_space		; should gen overflow 

*=========================================================================
*=========================================================================
* Get Stack Pointer Macros 
* OPERSP	((CTXT)->space.stacks.o.sp)
* EXECSP	((CTXT)->space.stacks.e.sp)
* DICTSP	((CTXT)->space.stacks.d.sp)
* GSAVSP	((CTXT)->space.stacks.g.sp)

OSP	equ	dpss_stacks+st_o+sh_sp			;offset to O stack pointer
ESP	equ	dpss_stacks+st_e+sh_sp			;offset to E stack pointer

OPERSP	MACRO			* OPERSP	CTXT,Xn
		move.l	OSP(\1),\2
		ENDM

EXECSP	MACRO			* EXECSP	CTXT,Xn
		move.l	ESP(\1),\2
		ENDM

*=========================================================================
*=========================================================================
* Stack Entry count Macros 
* NUMOPER	((CTXT)->space.stacks.o.num)
* NUMEXEC	((CTXT)->space.stacks.e.num)
* NUMDICT	((CTXT)->space.stacks.d.num)
* NUMGSAV	((CTXT)->space.stacks.g.num)

ONUM equ	dpss_stacks+st_o+sh_num			;offset to num of O stack elems
ENUM equ	dpss_stacks+st_e+sh_num			;offset to num of E stack elems

NUMOPER	MACRO			* NUMOPER	CTXT,Xn
		move.l	ONUM(\1),\2
		ENDM

NUMEXEC	MACRO			* NUMEXEC	CTXT,Xn
		move.l	ENUM(\1),\2
		ENDM

*=========================================================================
*=========================================================================
* Stack Push Macros 
* PUSHOPER(CTXT,obj) (CTXT)->space.stacks.o.num++;\
* 					  *(--(CTXT)->space.stacks.o.sp) = *obj;
* PUSHEXEC(CTXT,obj) (CTXT)->space.stacks.e.num++;\
* 					  *(--(CTXT)->space.stacks.e.sp) = *obj;
* PUSHDICT(CTXT,obj) (CTXT)->space.stacks.d.num++;\
* 					  *(--(CTXT)->space.stacks.d.sp) = *obj;
* PUSHGSAV(CTXT,obj) (CTXT)->space.stacks.g.num++;\
* 					  *(--(CTXT)->space.stacks.g.sp) = *obj;

PUSHOPER	MACRO		* PUSHOPER	CTXT,An
		OPERSP	\1,A0
		move.l	(\2),(a0)+
		move.l	4(\2),(a0)+
		move.l	a0,OSP(\1)
		addq.l	#1,ONUM(\1)
			ENDM
*=========================================================================
*=========================================================================
* * Stack Pop Macros 
* POPOPER(CTXT) (CTXT)->space.stacks.o.num--;\
* 					   (CTXT)->space.stacks.o.sp++;
* POPEXEC(CTXT) (CTXT)->space.stacks.e.num--;\
* 					   (CTXT)->space.stacks.e.sp++;
* POPDICT(CTXT) (CTXT)->space.stacks.d.num--;\
* 					   (CTXT)->space.stacks.d.sp++;
* POPGSAV(CTXT) (CTXT)->space.stacks.g.num--;\
* 					   (CTXT)->space.stacks.g.sp++;

POPOPER	MACRO		* POPOPER	CTXT
		  addq.l	#8,OSP(\1)
		  subq.l	#1,ONUM(\1)
		ENDM

* 
* POPNUMOPER(CTXT,N) (CTXT)->space.stacks.o.num -= (N);\
* 			     (CTXT)->space.stacks.o.sp += (N);
* 

*=========================================================================
NEED_ARGS	MACRO	* NEED_ARGS	CTXT,N
				moveq	#\2,d0				; guaranteed small number
				cmp.l	ONUM(\1),d0			; check stack counter
				bgt		ostack_uflow		; goto global label if not enough
			ENDM
*=========================================================================

	ENDC
