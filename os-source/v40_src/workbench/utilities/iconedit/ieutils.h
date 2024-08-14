

#include <exec/types.h>


#define ASM __asm
#define REG(x)	register __ ## x

/* Macro to get longword-aligned stack space for a structure	*/
/* Uses ANSI token catenation to form a name for the char array */
/* based on the variable name, then creates an appropriately	*/
/* typed pointer to point to the first longword boundary in the */
/* char array allocated.					*/
#define D_S(type,name) char a_##name[sizeof(type)+3]; \
		       type *name = (type *)((LONG)(a_##name+3) & ~3);

kprintf(STRPTR,...);
