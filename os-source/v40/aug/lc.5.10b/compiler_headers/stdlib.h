#ifndef __ARGS
#ifdef NARGS
#define __ARGS(a) ()
#else
#define __ARGS(a) a
#endif
#endif
/**
*
* Level 3 memory allocation services
*
**/
extern void *malloc __ARGS((unsigned));
extern void *calloc __ARGS((unsigned,unsigned));
extern void *realloc __ARGS((void*, unsigned));
extern void free __ARGS((void *));

/**
*
* Level 2 memory allocation services
*
**/
extern void *getmem __ARGS((unsigned));
extern void *getml __ARGS((long));
extern int rlsmem __ARGS((void *, unsigned));
extern int rlsml __ARGS((void *, long));
extern int bldmem __ARGS((int));
extern long sizmem __ARGS((void));
extern long chkml __ARGS((void));
extern void rstmem __ARGS((void));

/**
*
* Level 1 memory allocation services
*
**/
extern void *sbrk __ARGS((unsigned));
extern void *lsbrk __ARGS((long));
extern int rbrk __ARGS((void));
extern void MemCleanup __ARGS((void));

/**
*
* Miscellaneous I/O services
*
*/
extern int access __ARGS((char *, int));
extern int chdir __ARGS((char *));
extern int chmod __ARGS((char *, int));
extern char *getcwd __ARGS((char *, int));
extern int mkdir __ARGS((char *));
extern int perror __ARGS((char *));
extern int rmdir __ARGS((char *));
extern char *tmpnam __ARGS((char *));

/**
*
* Sort functions
*
*/
extern void qsort __ARGS((char *, int, int, int (*)()));
extern void dqsort __ARGS((double *, unsigned int));
extern void fqsort __ARGS((float *, unsigned int));
extern void lqsort __ARGS((long *, unsigned int));
extern void sqsort __ARGS((short *, unsigned int));
extern void tqsort __ARGS((char **, unsigned int));

/**
*
* Miscellaneous functions
*
*/
extern void abort __ARGS((void));
extern char *argopt __ARGS((int, char**, char *, int *, char *));
extern int atoi __ARGS((char *));
extern long atol __ARGS((char *));
extern char *ecvt __ARGS((double, int, int *, int *));
/*
*   extern void main __ARGS((int, char **));
*/
extern void _main __ARGS((char *));
extern void _tinymain __ARGS((char *));
extern void exit __ARGS((int));
extern void _exit __ARGS((int));
extern void XCEXIT __ARGS((long));
extern char *fcvt __ARGS((double, int, int *, int *));
extern char *getenv __ARGS((char *));
extern char *gcvt __ARGS((double, int, char *));
extern int getfnl __ARGS((char *, char *, unsigned, int));
extern int iabs __ARGS((int));
extern long labs __ARGS((long));
extern int onexit __ARGS((int(*)()));
extern int putenv __ARGS((char *));
extern int system __ARGS((char *));
/*
* define NULL if it's not already defined
*
*/
#ifndef NULL
#define NULL 0L
#endif
