/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
* |_o_o|\\ Copyright (c) 1989 The Software Distillery.                    *
* |. o.| ||          All Rights Reserved                                  *
* | .  | ||          Written by Doug Walker                               *
* | o  | ||          The Software Distillery                              *
* |  . |//           235 Trillingham Lane                                 *
* ======             Cary, NC 27513                                       *
*                    BBS:(919)-471-6436                                   *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#if MWDEBUG

#include <exec/types.h>

#ifdef MANX
#define MWARGS(a) /* No support for prototypes */
#else
#define MWARGS(a) a
#endif

/* Flags for MWInit */
#define MWF_EXT      0x00000001 /* Want to communicate w/ external func */
#define MWF_NOLOG    0x00000002 /* No debug messages                    */
#define MWF_NOCHECK  0x00000000 /* (compatibility - do not use)         */
#define MWF_CHECK    0x00000004 /* Check mem whenever mem rtns called   */
#define MWF_NOFREE   0x00000008 /* Don't free nonfreed memory           */
#define MWF_NOFTRASH 0x00000010 /* Don't trash memory upon free         */
#define MWF_NOATRASH 0x00000020 /* Don't trash memory upon alloc        */
#define MWF_NOFKEEP  0x00000040 /* Don't keep memory after free         */

#define MWF_ACTIVE   0x80000000 /* PRIVATE - MemWatch is active          */
#define MWF_ERROR    0x40000000 /* PRIVATE - Error discovered, terminate */

#define AllocMem(size,flags) MWAllocMem(size, flags, __FILE__, __LINE__)
#define FreeMem(mem,size)    MWFreeMem(mem, size, 0, __FILE__, __LINE__)
#define malloc(size)         MWAllocMem(size, 0, __FILE__, __LINE__)
#define calloc(nelt,esize)   malloc((nelt)*(esize))

#ifdef free
#undef free
#endif
#define free(mem)            MWFreeMem(mem, -1, 1, __FILE__, __LINE__)

#define DosAllocMem(g,s)     MWAllocMem(s, MEMF_CLEAR, __FILE__,__LINE__)
#define DosFreeMem(m)        MWFreeMem(m, -1, 1, __FILE__, __LINE__)

/* realloc is not supported yet, but this will at least cause an undef */
/* global when linking                                                 */
#define realloc(mem,size)    MWrealloc(mem,size,__FILE__,__LINE__)

/* Flags to tell MWReport how much to report */
#define MWR_NONE 0   /* Don't report anything; just return    */
#define MWR_SUM  1   /* Report current and total usage        */
#define MWR_FULL 2   /* Report on all outstanding allocations */

void MWInit      MWARGS((LONG, LONG, char *));
void MWTerm      MWARGS((void));
void *MWAllocMem MWARGS((long, long, char *, long));
void MWFreeMem   MWARGS((void *, long, long, char *, long));
void MWCheck     MWARGS((void));
void MWReport    MWARGS((char *, long));
void MWLimit     MWARGS((LONG, LONG));

#else

/* No memory debugging - make everything go away */

#define MWInit(a,b,c)
#define MWTerm()
#define MWCheck()
#define MWReport()
#define MWLimit(a,b)

#endif
