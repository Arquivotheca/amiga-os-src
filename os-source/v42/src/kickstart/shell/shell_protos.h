#include <exec/types.h>

int extend (UBYTE **buf, LONG *size, LONG newsize);
LONG addchar (LONG pos, UBYTE **buf, LONG *size, UBYTE ch);
LONG MyGetVar (struct Global *gv, UBYTE *name, UBYTE **buf, LONG *size, ULONG flags);
int pipecheck(struct Global *gv,BOOL status,UBYTE *type);
LONG ticks(struct Global *gv,UBYTE **string,LONG *size);
int readitem(struct Global *gv,UBYTE *string);

int ungetchar(struct Global *gv);
int getchar(struct Global *gv,int lastch);

/*void __stdargs writef( struct DosLibrary *db, UBYTE *fmt, UBYTE *args); */

void __stdargs writef( struct DosLibrary *db, UBYTE *fmt, ... );

ULONG testflags(long flag);
struct Segment *searchsegs(struct DosLibrary *db,UBYTE *name,
	struct Segment *startseg);
struct Segment *useseg(struct DosLibrary *db,UBYTE *name);
LONG Err(struct Global *gv,LONG m,BPTR si, BPTR so);
BPTR openf(struct Global *gv,UBYTE *name,LONG act);
LONG run(struct Global *gv,BPTR module,BOOL pipe_expand);
int testscript (struct Global *gv,UBYTE *file,UBYTE qflag);
BPTR load(struct Global *gv,UBYTE *file, BPTR ldir, LONG *segment, LONG qflag);
BPTR findLoad(struct Global *gv,UBYTE *commandname, LONG *segment, 
	struct DosPacket *parm_pkt,BOOL status);
void __asm startup( register __a0 struct DosPacket *parm_pkt);
int CXBRK(void);
struct RDArgs *getargs(struct Library **db, struct Library **ub,
	UBYTE *template, LONG *opts[],int size);

int expand(struct Global *gv, UBYTE **to, LONG *size);
void setv(struct Global *gv,LONG *rc);

void do_end(struct DosLibrary *DOSBase);
VOID CtoBstr(UBYTE *string);
int testpure(struct DosLibrary *db,UBYTE *file,LONG force);

VOID __stdargs kprintf(UBYTE *fmt, ...);

int cmd_stack(void);
int cmd_ask(void);
int cmd_failat(void);
int cmd_getenv(void);
int cmd_why(void);
int cmd_cd(void);
int cmd_prompt(void);
int cmd_path(void);
int cmd_else(void);
int cmd_echo(void);
int cmd_if(void);
int cmd_endif(void);
int cmd_endcli(void);
int cmd_setenv(void);
int cmd_skip(void);
int cmd_fault(void);
int cmd_quit(void);

int link_setenv(void);
int link_endif(void);
int link_endcli(void);
int link_echo(void);
int link_resident(void);
int link_cd(void);
int link_prompt(void);
int link_getenv(void);
int link_ask(void);
int link_else(void);
int link_failat(void);
int link_fault(void);
int link_if(void);
int link_path(void);
int link_quit(void);
int link_skip(void);
int link_why(void);
int link_stack(void);
int link_which(void);
int link_newshell(void);
int link_run(void);
