/* path_protos.h */
/* use ANSI token concatenation operator - ## */

#define ASM __asm
#define REG(x)	register __ ## x

struct Process * ASM CreateNewProc (REG(d1) struct TagList *tags);

struct CommandLineInterface * __regargs makecli (struct Process *myproc,
						 struct TagItem *tags);
struct CommandLineInterface * __regargs AllocCli (struct TagItem *tags);

void __regargs freecli (struct CommandLineInterface *cli, LONG tnum);

void __regargs freepath (BPTR path);

struct MsgPort * ASM createtaskproc (REG(a0) char *name,
				     REG(a1) BPTR seg,
				     REG(a2) BPTR gvec,
				     REG(d0) LONG stacksize,
				     REG(d1) LONG namelen,
				     REG(d2) LONG pri,
				     REG(d3) LONG task);

struct MsgPort * ASM createproc (REG(d1) char *name,
				 REG(d2) LONG pri,
				 REG(d3) BPTR seg,
				 REG(d0) LONG stacksize);

struct MsgPort * ASM createtask (REG(d1) BPTR segarray,
				 REG(d2) LONG stacksize,
				 REG(d3) BPTR pri,
				 REG(d0) BSTR bname,
				 REG(a0) BPTR gvec);

struct RootNode * __regargs taskrootstruct (void);

LONG __regargs CleanupProc (struct Process *p, LONG returncode);
