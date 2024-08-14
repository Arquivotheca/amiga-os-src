/* record_protos.h */
/* use ANSI token concatenation operator - ## */

#define ASM __asm
#define REG(x)	register __ ## x

LONG ASM LockRecord (REG(d1) BPTR fh,
		     REG(d2) LONG position,
		     REG(d3) LONG length,
		     REG(d4) LONG mode,
		     REG(d5) LONG timeout);  /* ran out of regs */


LONG ASM UnLockRecord (REG(d1) BPTR fh,
		       REG(d2) LONG position,
		       REG(d3) LONG length);

LONG ASM LockRecords (REG(d1) struct RecordLock *array,
		      REG(d2) ULONG timeout);

LONG ASM UnLockRecords (REG(d1) struct RecordLock *array);
