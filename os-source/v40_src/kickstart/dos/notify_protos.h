/* notify_protos.h */
/* use ANSI token concatenation operator - ## */

#define ASM __asm
#define REG(x)	register __ ## x

LONG ASM StartNotify (REG(d1) struct NotifyRequest *nreq);
LONG ASM EndNotify (REG(d1) struct NotifyRequest *nreq);
