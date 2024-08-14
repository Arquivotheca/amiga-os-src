// This file contains stack definition and macro routines
// March 1991 P.J. © CBM-Amiga
// Submitted to RCS on 05/APR/91

#ifndef STACK_H
#define STACK_H

/* Implementation Sizes */
#define E_SIZE	250
#define O_SIZE	500			// similar to NEC Silentwriter
#define D_SIZE	20
#define G_SIZE	31

/* Stack header. Holds current stack pointer and number of elements on stack */
typedef struct {
	int num;		/* Num of elements on this stack */
	ps_obj *sp;		/* Stack Pointer */
} StackHeadRec;

/* Stack structure, referenced through support functions */

typedef struct {
	StackHeadRec o;	/* Operand Stack */
	StackHeadRec e;	/* Execution Stack */
	StackHeadRec d;	/* Dictionary Stack */
	StackHeadRec g;	/* Gsave Stack */
} StackRec;	

/* Get Stack Pointer Macros */
#define OPERSP(CTXT)	((CTXT)->space.stacks.o.sp)
#define EXECSP(CTXT)	((CTXT)->space.stacks.e.sp)
#define DICTSP(CTXT)	((CTXT)->space.stacks.d.sp)
#define GSAVSP(CTXT)	((CTXT)->space.stacks.g.sp)

/* Stack Entry count Macros */
#define NUMOPER(CTXT)	((CTXT)->space.stacks.o.num)
#define NUMEXEC(CTXT)	((CTXT)->space.stacks.e.num)
#define NUMDICT(CTXT)	((CTXT)->space.stacks.d.num)
#define NUMGSAV(CTXT)	((CTXT)->space.stacks.g.num)

/* Operand Stack Init macro */
#define InitOStack(CTXT) {(CTXT)->space.stacks.o.sp += (CTXT)->space.stacks.o.num;\
			 (CTXT)->space.stacks.o.num = 0;}

/* Stack Push Macros */
#define PUSHOPER(CTXT,obj) {(CTXT)->space.stacks.o.num++;\
					  *(--(CTXT)->space.stacks.o.sp) = *obj;}

#define PUSHINT(I)  {*(--ctxt->space.stacks.o.sp) = int_obj;\
					ctxt->space.stacks.o.sp->obj.intval=I;\
					ctxt->space.stacks.o.num++;}

#define PUSHREAL(R)  {*(--ctxt->space.stacks.o.sp) = real_obj;\
					ctxt->space.stacks.o.sp->obj.realval=R;\
					ctxt->space.stacks.o.num++;}

#define PUSHNAME(N) {*(--ctxt->space.stacks.o.sp) = name_obj;\
		ctxt->space.stacks.o.sp->obj.nameval=AddName(ctxt,N);\
					ctxt->space.stacks.o.num++;}

#define PUSHEXEC(CTXT,obj) {(CTXT)->space.stacks.e.num++;\
					  *(--(CTXT)->space.stacks.e.sp) = *obj;}
#define PUSHDICT(CTXT,obj) {(CTXT)->space.stacks.d.num++;\
					  *(--(CTXT)->space.stacks.d.sp) = *obj;}
#define PUSHGSAV(CTXT,obj) {(CTXT)->space.stacks.g.num++;\
					  *(--(CTXT)->space.stacks.g.sp) = *obj;}

/* Stack Pop Macros */
#define POPOPER(CTXT) {(CTXT)->space.stacks.o.num--;\
					   (CTXT)->space.stacks.o.sp++;}
#define POPEXEC(CTXT) {(CTXT)->space.stacks.e.num--;\
					   (CTXT)->space.stacks.e.sp++;}
#define POPDICT(CTXT) {(CTXT)->space.stacks.d.num--;\
					   (CTXT)->space.stacks.d.sp++;}
#define POPGSAV(CTXT) {(CTXT)->space.stacks.g.num--;\
					   (CTXT)->space.stacks.g.sp++;}

#define POPNUMOPER(CTXT,N) {(CTXT)->space.stacks.o.num -= (N);\
			     (CTXT)->space.stacks.o.sp += (N);}

/* 4 Different stacks ! */
#define OPER	1L
#define EXEC	2L
#define DICT	3L
#define GSAVE	4L
	
#endif
