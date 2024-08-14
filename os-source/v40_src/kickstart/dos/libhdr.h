/* max soft links gone through in resolving a path */
#define MAX_LINKS		15

/* eof character */
#define ENDSTREAMCH		-1

/* buffered IO buffer size */
#define SCB_BUFFERSIZE		(49*4)

/* BCPL global offset local globals start at */
#define UG			150
#define FG			UG

#define G_DOSBASE		129
#define G_UTILITYBASE		130

/* # of globals before the global pointer */
#define NEGATIVE_GLOBAL_SIZE	50

/* cli command path node */
/* should be getvec()ed */

struct Path {
	BPTR path_Next;
	BPTR path_Lock;
};

/* useful definition */
#define MYPROC (myproc())

/* damn bryce changed exec_old_pragmas.h */
/*#define SysBase (*((struct ExecBase **) 4))*/

#undef strlen
#ifdef SASC_6
extern size_t __regargs strlen (const char *string);
#else
extern int __regargs strlen (char *string);
#endif

/* #define mystrcat strcat */

#undef NewList
extern void __regargs NewList (struct List *list);

