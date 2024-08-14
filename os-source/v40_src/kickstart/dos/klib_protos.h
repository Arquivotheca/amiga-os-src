/* klib_protos.h */
/* use ANSI token concatenation operator - ## */

#define ASM __asm
#define REG(x)	register __ ## x

LONG __regargs udiv32	(ULONG denom, ULONG numer);
LONG __regargs divrem32 (LONG denom, LONG numer, LONG *remainder);
LONG __regargs div32	(LONG denom, LONG numer);
LONG __regargs rem32	(LONG denom, LONG numer);

void * __regargs AllocVecPub (LONG bytes);
void * __regargs AllocVecPubClear (LONG bytes);
void __asm freeVec (REG(a1) void *mem);

BPTR __regargs seglist (void);
UBYTE * __regargs ArgStr (void);
void __regargs exit (void);

BPTR ASM loadseg (REG(d1) char *name);
BPTR ASM loadsegbstr (REG(d1) BSTR name);
void ASM unloadseg (REG(d1) BPTR seg);

LONG ASM requester  (REG(d1) char *s1,REG(d2) char *s2,REG(d3) char *s3);
LONG ASM requester2 (REG(d1) char *s1,REG(d2) char *s2,REG(d3) char *s3,
		     REG(d4) LONG eventflags);

LONG __regargs getresult2 (void);
void __regargs setresult2 (LONG result);
void ASM SetIoErr (REG(d1) LONG code);				/* NEW */

struct MsgPort * __regargs filesystemtask (void);
void ASM setfilesys (REG(d1) struct MsgPort *);
struct MsgPort * __regargs consoletask (void);
void ASM setconsole (REG(d1) struct MsgPort *);
BPTR ASM currentdir (REG(d1) LONG flag,REG(d2) BPTR dir);
struct MsgPort * __regargs taskid(void);
struct Process * __regargs myproc(void);
struct CommandLineInterface * __regargs cli(void);

BPTR __regargs input (void);
BPTR __regargs output (void);
void ASM selectinput (REG(d1) BPTR fh);
void ASM selectoutput (REG(d1) BPTR fh);

void ASM abort (REG(d1) LONG guru);
LONG ASM qpkt (REG(d1) struct DosPacket *dp);
LONG ASM bcpl_qpkt (REG(d1) BPTR dp);
LONG ASM returnpkt (REG(d1) struct DosPacket *dp,REG(d2) LONG res1,
		    REG(d3) LONG res2);
struct DosPacket * __regargs taskwait (void);

struct Process * __regargs MakeProc(LONG *stacksize, UBYTE *name, LONG len);
LONG __regargs AddSegArray(struct Process *new, BPTR seglist, LONG entry);
void __regargs deactcode(void);
void __regargs activecode(void);
void __regargs CNP_ActiveCode(void);
void ASM deletetask(void);

BPTR ASM CtoB (REG(d1) char *source,REG(d2) CPTR dest);
void __regargs BtoC (UBYTE *source,char *dest);
void ASM BtoCstr (REG(d1) CPTR string);
void __regargs CtoBstr (char *string);

struct RootNode * __regargs rootstruct (void);
struct DosLibrary * __regargs dosbase (void);
struct DosLibrary * __regargs opendosbase (void);

BYTE __regargs mystricmp(char *a,char *b);
BYTE __regargs mystrnicmp(char *a,char *b,long len);
char * __regargs mystrcat(char *str, char *tail);
struct TagItem * __regargs findTagItem( ULONG tagVal, struct TagItem *tagList );
ULONG __regargs getTagData( ULONG tagVal, ULONG defaultVal,
	struct TagItem *tagList );
ULONG __regargs packBoolTags( ULONG initialFlags, struct TagItem *tagList,
	struct TagItem *boolMap );
struct TagItem * __regargs cloneTagItems( struct TagItem *tagList );
void __regargs freeTagItems( struct TagItem *tagList );
LONG __regargs filterTagItems( struct TagItem *tagList, ULONG *filterArray,
			       LONG logic );

void __regargs store_IBase(struct Library *lib);
void __regargs MySetPrefs(struct Preferences *prefs,LONG size);

void __stdargs mysprintf (UBYTE *str,UBYTE *fmt,LONG arg1,...);
void __regargs copyMem (APTR src, APTR dest, ULONG size);

LONG ASM InternalRunCommand (REG(d1) BPTR seg, REG(d2) LONG stack,
			     REG(d3) UBYTE *args, REG(d4) LONG length);

/* stuff from sendpkt.asm */
void ASM SendPkt(REG(d1) struct DosPacket *dp, REG(d2) struct MsgPort *port,
		 REG(d3) struct MsgPort *replyport);

/*********************/
/* pragma interfaces */
LONG ASM sendpkt4(REG(d1) struct MsgPort *port,
		  REG(d2) LONG action, REG(d3) LONG arg1,
		  REG(d4) LONG arg2, REG(d5) LONG arg3,
		  REG(d6) LONG arg4);
LONG ASM sendpkt3(REG(d1) struct MsgPort *port,
		  REG(d2) LONG action, REG(d3) LONG arg1,
		  REG(d4) LONG arg2, REG(d5) LONG arg3);
LONG ASM sendpkt2(REG(d1) struct MsgPort *port,
		  REG(d2) LONG action, REG(d3) LONG arg1,
		  REG(d4) LONG arg2);
LONG ASM sendpkt1(REG(d1) struct MsgPort *port,
		  REG(d2) LONG action, REG(d3) LONG arg1);
LONG ASM sendpkt0(REG(d1) struct MsgPort *port,
		  REG(d2) LONG action);

LONG splitname (UBYTE *name, UBYTE split, UBYTE *buf, LONG pos, LONG size);

extern LONG mysplitname();

#pragma libcall (&mysplitname) splitname 0 5432105
