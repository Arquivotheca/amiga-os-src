 /*
  * clib/pools_protos.h
  * 
  * Created 19 May 93 by Christopher A. Wichura (caw@miroc.chi.il.us)
  * 
  * including this file will set up an application for using the link library
  * pools routines transparantly.  We provide the appropriate prototypes for
  * the pool link library and then #define all the exec pool functions to
  * point at the link library.  this makes it very easy to make code written
  * for v39 use pools under older versions of the os.  since the pools
  * routines are in amiga.lib as of v40, one may not even have to edit their
  * makefile.
  */

void *__asm AsmAllocPooled(register __a0 void *,
			   register __d0 ULONG,
			   register __a6 struct ExecBase *);
void *__asm AsmCreatePool(register __d0 ULONG,
			  register __d1 ULONG,
			  register __d2 ULONG,
			  register __a6 struct ExecBase *);
void __asm AsmDeletePool(register __a0 void *,
			 register __a6 struct ExecBase *);
void __asm AsmFreePooled(register __a0 void *,
			 register __a1 void *,
			 register __d0 ULONG,
			 register __a6 struct ExecBase *);

#define AllocPooled(a, b) AsmAllocPooled(a, b, SysBase)
#define CreatePool(a, b, c) AsmCreatePool(a, b, c, SysBase)
#define DeletePool(a) AsmDeletePool(a, SysBase)
#define FreePooled(a, b, c) AsmFreePooled(a, b, c, SysBase)
