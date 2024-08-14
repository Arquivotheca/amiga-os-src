* This file contains stack definition and macro routines
* March 1991 P.J. © CBM-Amiga
* Submitted to RCS on 05/APR/91

	IFND STACK_I
STACK_I	equ	1

* Implementation Sizes 
E_SIZE	equ	250
O_SIZE	equ	500			* similar to NEC Silentwriter
D_SIZE	equ	20
G_SIZE	equ	31

	STRUCTURE StackHeadRec,0
		LONG 	sh_num
		APTR	sh_sp
	LABEL	sh_SIZEOF

* * Stack header. Holds current stack pointer and number of elements on stack 
* typedef struct {
* 	int num;		* Num of elements on this stack 
* 	ps_obj *sp;		* Stack Pointer 
* } StackHeadRec;

* Stack structure, referenced through support functions 

	STRUCTURE Stacks,0
		STRUCT	st_o,sh_SIZEOF
		STRUCT	st_e,sh_SIZEOF
		STRUCT	st_d,sh_SIZEOF
		STRUCT	st_g,sh_SIZEOF
	LABEL	st_SIZEOF

* typedef struct {
* 	StackHeadRec o;	* Operand Stack 
* 	StackHeadRec e;	* Execution Stack 
* 	StackHeadRec d;	* Dictionary Stack 
* 	StackHeadRec g;	* Gsave Stack 
* } StackRec,*Stacks;	

		ENDC
