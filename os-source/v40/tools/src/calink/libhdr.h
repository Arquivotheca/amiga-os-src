#ifdef SUN
#include <setjmp.h>

typedef long word;
typedef char* string;
#endif

#ifdef AMIGA
#include <lattice/setjmp.h>

typedef long word;
typedef char* string;
#endif

#ifdef IBM
#include "\lc\setjmp.h"

typedef long word;
typedef char* string;
#endif

#define NULL 0
#define true 1
#define false 0
#define TRUE true
#define FALSE false
#define bytesperword 4
#define BYTESPERWORD 4
#define bytesper68000word 4
#define wordsper68000word (bytesper68000word/bytesperword)
#define MAXINT 0x7FFFFFFF
#define MININT 0x80000000
#define ENDSTREAMCH (-1)
#define endstreamch ENDSTREAMCH

#define NULLPTR (word*)0
#define NULLSTR (string)0

typedef int* SCBPTR;

extern int (*wrch_fn)();
extern int (*rdch_fn)();
extern int (*unrdch_fn)();

extern word findinput();
extern word findoutput();
extern word gbytes();
extern word *getvec();
extern word input();
extern word note();
extern word output();
extern word point();
extern word pointword();
extern word rdargs();
extern word readbytes();
extern word readn();
extern word readwords();
extern word wbytes();
extern word writewords();

extern word result2;
