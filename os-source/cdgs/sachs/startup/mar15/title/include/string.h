/* Copyright (c) 1992 SAS Institute, Inc., Cary, NC USA */
/* All Rights Reserved */


#ifndef _STRING_H
#define _STRING_H 1

#ifndef _SIZE_T
#define _SIZE_T 1
typedef unsigned int size_t;
#endif

#ifndef NULL
#define NULL  0L
#endif


/***
*
*     ANSI copying functions
*
***/

extern void *memcpy(void *, const void *, size_t);
extern void *memmove(void *, const void *, size_t);
extern char *strcpy(char *, const char *);
extern char *strncpy(char *, const char *, size_t);


/***
*
*     ANSI concatenation functions
*
***/

extern char *strcat(char *, const char *);
extern char *strncat(char *, const char *, size_t);


/***
*
*     ANSI comparison functions
*
***/

extern int memcmp(const void *, const void *, size_t);
extern int strcmp(const char *, const char *);
extern int strcoll(const char *, const char *);
extern int strncmp(const char *, const char *, size_t);
extern size_t strxfrm(char *, const char *, size_t);


/***
*
*     ANSI search functions
*
***/

extern void *memchr(const void *, int, size_t);
extern char *strchr(const char *, int);
extern size_t strcspn(const char *, const char *);
extern char *strpbrk(const char *, const char *);
extern char *strrchr(const char *, int);
extern size_t strspn(const char *, const char *);
extern char *strstr(const char *, const char *);
extern char *strtok(char *, const char *);


/***
*
*     ANSI miscellaneous functions
*
***/

extern void *memset(void *, int, size_t);
extern char *strerror(int);
extern size_t strlen(const char *);

#ifndef _STRICT_ANSI

/***
*
*     SAS string and memory functions.
*
***/

extern int stcarg(char *, char *);
extern int stccpy(char *, char *, int);
extern int stcgfe(char *, char *);
extern int stcgfn(char *, char *);
extern int stcis(char *, char *);
extern int stcisn(char *, char *);
extern int __stcd_i(const char *, int *);
extern int __stcd_l(const char *, long *);
extern int stch_i(char *, int *);
extern int stch_l(char *, long *);
extern int stci_d(char *, int);
extern int stci_h(char *, int);
extern int stci_o(char *, int);
extern int stcl_d(char *, long);
extern int __stcl_h(char *, long);
extern int __stcl_o(char *, long);
extern int stco_i(char *, int *);
extern int stco_l(char *, long *);
extern int stcpm(char *, char *, char **);
extern int stcpma(char *, char *);
extern int stcsma(char *, char *);
extern int astcsma(char *, char *);
extern int stcu_d(char *, unsigned);
extern int __stcul_d(char *, unsigned long);

#define stclen(a) strlen(a)

extern char *stpblk(char *);
extern char *stpbrk(char *, char *);
extern char *stpchr(char *, int);
extern char *stpcpy(char *, char *);
extern char *__stpcpy(char *, char *);
extern char *stpdate(char *, int, char *);
extern char *stpsym(char *, char *, int);
extern char *stptime(char *, int, char *);
extern char *stptok(char *, char *, int, char *);

extern int strbpl(char **, int, char *);
extern int stricmp(char *, char *);
extern char *strdup(char *);
extern void strins(char *, char *);
extern int strmid(char *, char *, int, int);
extern char *__strlwr(char *);
extern void strmfe(char *, char *, char *);
extern void strmfn(char *, char *, char *, char *, char *);
extern void strmfp(char *, char *, char *);
extern int strnicmp(char *, char *, size_t);
extern char *strnset(char *, int, int);

extern char *stpchrn(char *, int);
extern char *strrev(char *);
extern char *strset(char *, int);
extern void strsfn(char *, char *, char *, char *, char *);
extern char *__strupr(char *);
extern int stspfp(char *, int *);
extern void strsrt(char *[], int);

extern int stcgfp(char *, char *);

#define strcmpi stricmp		/* For Microsoft compatibility */
#define strlwr  __strlwr
#define strupr  __strupr
#define stcd_i __stcd_i
#define stcd_l __stcd_l
#define stcl_h __stcl_h
#define stcl_o __stcl_o
#define stcul_d __stcul_d
#define stpcpy __stpcpy

extern void *memccpy(void *, void *, int, unsigned);
extern void movmem(void *, void *, unsigned);
extern void repmem(void *, void *, size_t, size_t);
extern void setmem(void *, unsigned, int);
extern void __swmem(void *, void *, unsigned);
#define swmem  __swmem

/* for BSD compatibility */
#define index(a,b) strchr(a,b)
#define rindex(a,b) strrchr(a,b)
#define bcopy(a,b,c) memmove(b,a,c)
#define bcmp(a,b,c) memcmp(a,b,c)
#define bzero(a,b) memset(a,0,b)

#endif /* _STRICT_ANSI */

/**
*
* Builtin function definitions
*
**/

extern size_t  __builtin_strlen(const char *);
extern int     __builtin_strcmp(const char *, const char *);
extern char   *__builtin_strcpy(char *, const char *);

extern void   *__builtin_memset(void *, int, size_t);
extern int     __builtin_memcmp(const void *, const void *, size_t);
extern void   *__builtin_memcpy(void *, const void *, size_t);

extern int __builtin_max(int, int);
extern int __builtin_min(int, int);
extern int __builtin_abs(int);

#ifdef USE_BUILTIN_MATH
/* if you want the builtin versions of abs(), max(), and mix() */
/* define USE_BUITLIN_MATH, and #include this file BEFORE math.h */
#ifndef max
#define max(a,b) __builtin_max(a,b)
#endif
#ifndef min
#define min(a,b) __builtin_min(a,b)
#endif
#ifndef abs
#define abs(a) __builtin_abs(a)
#endif
#endif

#define strlen(a)   __builtin_strlen(a)
#define strcmp(a,b) __builtin_strcmp(a,b)
#define strcpy(a,b) __builtin_strcpy(a,b)

#define memset(a,b,c) __builtin_memset(a,b,c)
#define memcmp(a,b,c) __builtin_memcmp(a,b,c)
#define memcpy(a,b,c) __builtin_memcpy(a,b,c)


#endif
