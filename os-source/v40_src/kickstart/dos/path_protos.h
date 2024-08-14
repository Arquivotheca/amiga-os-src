/* path_protos.h */
/* use ANSI token concatenation operator - ## */

#define ASM __asm
#define REG(x)	register __ ## x

LONG ASM NameFromLock (REG(d1) BPTR lock, REG(d2) char *buffer,	       /* NEW */
		       REG(d3) LONG buflen);

LONG ASM NameFromFH (REG(d1) BPTR fh, REG(d2) char *buffer,
		     REG(d3) LONG buflen);

LONG ASM GetCurrentDirName (REG(d1) UBYTE *buffer, REG(d2) LONG len);
LONG ASM GetPrompt (REG(d1) UBYTE *buffer, REG(d2) LONG len);
LONG ASM GetProgramName (REG(d1) UBYTE *buffer, REG(d2) LONG len);
LONG ASM SetCurrentDirName (REG(d1) UBYTE *dirname);
LONG ASM SetPrompt (REG(d1) UBYTE *prompt);
LONG ASM SetProgramName (REG(d1) UBYTE *name);
