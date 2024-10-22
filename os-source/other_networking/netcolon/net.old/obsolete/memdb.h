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

/* Flags for MWInit */
#define MWF_EXT      0x00000001 /* Want to communicate w/ external func */
#define MWF_NOLOG    0x00000002 /* No debug messages                    */
#define MWF_NOCHECK  0x00000004 /* check mem whenever mem rtns called   */
#define MWF_NOFREE   0x00000008 /* Don't free nonfreed memory           */
#define MWF_NOFTRASH 0x00000010 /* Don't trash memory upon free         */
#define MWF_NOATRASH 0x00000020 /* Don't trash memory upon alloc        */
#define MWF_NOFKEEP  0x00000040 /* Don't keep memory after free         */
#define MWF_CLOSELOG 0x00000080 /* Close log window upon exit           */
#define MWF_SERIAL   0x00000100 /* Use serial port for debugging        */

#define MWF_ACTIVE   0x80000000 /* PRIVATE - MemWatch is active          */
#define MWF_ERROR    0x40000000 /* PRIVATE - Error discovered, terminate */

#define AllocMem(size,flags) MWAllocMem(global, size, flags, __FILE__, __LINE__)
#define FreeMem(mem,size)    �MWFreeMem(global, mem, size, 0)
#define DosAllocMem(size)    AllocMem(global, size,MEMF_PUBLIC|MEMF_CLEAR)
#define DosFreeMem(mem)      MWFreeMem(global, mem, -1, 1)

/* Flags to tell MWReport how much to report */
#define MWR_NONE 0   /* Don't report anything; just return    */
#define MWR_SUM  1   /* Report current and total usage        */
#define MWR_FULL 2   /* Report on all outstanding allocations */

void MWInit      (GLOBAL, LONG);
void MWTerm      (GLOBAL);
void *MWAllocMem (GLOBAL, long, long, char *, long);
void MWFreeMem   (GLOBAL, void *, long, int);
void MWCheck     (GLOBAL);
void MWReport    (GLOBAL, char *, int);
void MWLimit     (GLOBAL, LONG, LONG);

#else

/* No memory debugging - make everything go away */

#define MWInit(x)
#define MWTerm()
#define MWCheck()
#define MWReport()
#define MWLimit(a,b)

#endif
