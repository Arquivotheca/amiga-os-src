/* path_protos.h */
/* use ANSI token concatenation operator - ## */

#define ASM __asm
#define REG(x)	register __ ## x

LONG ASM DeleteVar (REG(d1) UBYTE *name, REG(d2) ULONG flags);
LONG ASM SetVar (REG(d1) UBYTE *name, REG(d2) UBYTE *buffer, REG(d3) LONG size,
		 REG(d4) ULONG flags);
static LONG __regargs makedir (char *path);
void __regargs addvar(struct List *list, struct LocalVar *nlv);
struct LocalVar * ASM FindVar (REG(d1) UBYTE *name, REG(d2) ULONG type);
LONG ASM GetVar (REG(d1) UBYTE *name, REG(d2) UBYTE *buffer, REG(d3) LONG size,
		 REG(d4) ULONG flags);
LONG __regargs xfer_text(char *buffer, char *source, LONG len);
LONG __regargs copyvars (struct Process *np, struct Process *oldp);
void __regargs freevars (struct Process *proc);
