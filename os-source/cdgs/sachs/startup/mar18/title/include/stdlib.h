/* Copyright (c) 1992 SAS Institute, Inc., Cary, NC USA */
/* All Rights Reserved */


#ifndef _STDLIB_H
#define _STDLIB_H 1


#ifndef _SIZE_T
#define _SIZE_T 1
typedef unsigned int size_t;
#endif

#ifndef _WCHAR_T
#define _WCHAR_T 1
typedef char wchar_t;
#endif

typedef struct {
            int quot;
            int rem;
        } div_t;

typedef struct {
            long int quot;
            long int rem;
        } ldiv_t;

#ifndef NULL
#define NULL 0L
#endif

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 20

#ifdef _SHORTINT
#define RAND_MAX   32767
#else
#define RAND_MAX   2147483647
#endif

#define MB_CUR_MAX  __mb_cur_max
extern char __mb_cur_max;

extern double atof(const char *);
extern int atoi(const char *);
extern long int atol(const char *);

extern double strtod(const char *, char **);
extern long int strtol(const char *, char **, int);
extern unsigned long int strtoul(const char *, char **, int);


extern int rand(void);
extern void srand(unsigned int);


/***
*
*     ANSI memory management functions
*
***/

extern void *calloc(size_t, size_t);
extern void free(void *);
extern void *malloc(size_t);
extern void *realloc(void *, size_t);

#ifndef _STRICT_ANSI
extern void *halloc(unsigned long);              /*  Extension  */
extern void *__halloc(unsigned long);            /*  Extension  */
#endif /* _STRICT_ANSI */


/***
*
*     ANSI environment functions
*
***/

extern void abort(void);
extern int atexit(void (*)(void));
extern void exit(int);
extern char *__getenv(const char *);
extern char *getenv(const char *);
#define getenv __getenv
extern int system(const char *);


/***
*
*     ANSI searching and sorting functions
*
***/

extern void *bsearch(const void *, const void *, size_t, size_t,
                     int (*)(const void *, const void *));
extern void qsort(void *, size_t, size_t, int (*)(const void *, const void *));


/***
*
*     ANSI integer arithmetic functions
*
***/

extern int abs(int);
extern div_t div(int, int);
extern long int labs(long int);
extern ldiv_t ldiv(long int, long int);


/***
*
*     ANSI multibyte character functions
*
***/

extern int mblen(const char *, size_t);
extern int mbtowc(wchar_t *, const char *, size_t);
extern int wctomb(char *, wchar_t);
extern size_t mbstowcs(wchar_t *, const char *, size_t);
extern size_t wcstombs(char *, const wchar_t *, size_t);


#ifndef _STRICT_ANSI

#ifndef abs
#define abs(i)   ((i) < 0 ? -(i) : (i))
#endif

#ifndef labs
#define labs(i)  ((i) < 0 ? -(i) : (i))
#endif

/***
*
*     SAS Level 2 memory allocation functions
*
***/

extern void *getmem(unsigned int);
extern void *getml(long);
extern void rlsmem(void *, unsigned int);
extern void rlsml(void *, long);
extern int bldmem(int);
extern long sizmem(void);
extern long chkml(void);
extern void rstmem(void);


/***
*
*     SAS Level 1 memory allocation functions
*
***/

extern void *sbrk(unsigned int);
extern void *lsbrk(long);
extern int rbrk(void);
extern void __stdargs _MemCleanup(void);

extern unsigned long _MemType;
extern void *_MemHeap;
extern unsigned long _MSTEP;


/**
*
* SAS Sort functions
*
*/

extern void dqsort(double *, size_t);
extern void fqsort(float *, size_t);
extern void lqsort(long *, size_t);
extern void sqsort(short *, size_t);
extern void tqsort(char **, size_t);


/***
*
*     SAS startup, exit and environment functions.
*
***/

extern void __exit(int);
extern void __stdargs __main(char *);
extern void __stdargs __tinymain(char *);
extern void __stdargs _XCEXIT(long);
extern char *argopt(int, char**, char *, int *, char *);
extern int iabs(int);
extern int onexit(void (*)(void));
extern int putenv(const char *);

#define XCEXIT _XCEXIT                   /* compatibility with v5.0 */
#define _main __main
#define _exit __exit
#define WBenchMsg _WBBenchMsg


#endif /* _STRICT_ANSI */

#endif
