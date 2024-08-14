#ifndef __setjmp_h
#define __setjmp_h 1
/**
*
* This structure is used by the setjmp/longjmp functions to save the
* current environment on the 68000.
*
*/
struct JMP_BUF
	{
	long jmpret,		/* return address */
	     jmp_d1, jmp_d2, jmp_d3, jmp_d4, jmp_d5, jmp_d6, jmp_d7,
            jmp_a1, jmp_a2, jmp_a3, jmp_a4, jmp_a5, jmp_a6, jmp_a7;
	};

typedef struct JMP_BUF jmp_buf[1];

#ifndef NARGS
extern int setjmp(jmp_buf);
extern void longjmp(jmp_buf, int);
#else
extern int setjmp();
extern void longjmp();
#endif
#endif