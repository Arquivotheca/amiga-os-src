/* device_protos.h */
/* use ANSI token concatenation operator - ## */

#define ASM __asm
#define REG(x)	register __ ## x

struct DosList * ASM LockDosList         (REG(d1) ULONG flags);
struct DosList * ASM AttemptLockDosList  (REG(d1) ULONG flags);
struct DosList * ASM InternalLockDosList (REG(d1) ULONG flags,
					  REG(d2) LONG synch);
struct DosList * __regargs lockit (struct DosList *dl,
				   struct SignalSemaphore *sem,
				   ULONG flags,
				   ULONG synch);
void ASM UnLockDosList (REG(d1) ULONG flags);

LONG ASM RemDosEntry  (REG(d1) struct DosList *dlist);
LONG ASM AddDosEntry  (REG(d1) struct DosList *dlist);
void ASM FreeDosEntry (REG(d1) struct DosList *dlist);

struct DosList * ASM MakeDosEntry (REG(d1) UBYTE *name,
				   REG(d2) LONG  type);
struct DosList * ASM FindDosEntry (REG(d1) struct DosList *dlist,
				   REG(d2) UBYTE *name,
				   REG(d3) ULONG flags);
struct DosList * ASM NextDosEntry (REG(d1) struct DosList *dlist,
				   REG(d2) ULONG flags);
struct DosList * ASM adddevice    (REG(d1) char *name);

struct DosList * __regargs FindDosEntryByTask (struct MsgPort *port);

LONG ASM SameDevice   (REG(d1) BPTR lock1, REG(d2) BPTR lock2);

LONG ASM assign       (REG(d1) char *name, REG(d2) BPTR lock);
LONG ASM assignlate   (REG(d1) char *name, REG(d2) UBYTE *string);
LONG ASM assignstring (REG(d1) char *name, REG(d2) UBYTE *string);
LONG ASM assigncommon (REG(d1) char *name, REG(d2) BPTR lock,
		       REG(d3) LONG type);
void __regargs freelocklist (struct AssignList *list);
LONG ASM addassign    (REG(d1) char *name, REG(d2) BPTR lock);
