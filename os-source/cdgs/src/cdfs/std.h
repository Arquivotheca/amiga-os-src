#ifndef	STD_DEFS
#define	STD_DEFS

#include <exec/types.h>
#include <stdio.h>
#include <ctype.h>
#include <setjmp.h>

/***********************************************************************
**	Scope Declarations
***********************************************************************/

#define	IMPORT	extern
#define	EXPORT
#define FORWARD
#define LOCAL	static
#define	REG	register

/***********************************************************************
**	Type Definitions
***********************************************************************/

#define	U	unsigned
#ifndef TRUE
typedef	U char	BOOL;
typedef	char	BYTE;
typedef	U char	UBYTE;
typedef	short	WORD;
typedef	U short	UWORD;
typedef	long	LONG;
typedef	U long	ULONG;
typedef	double	FLOAT;
typedef char *  BSTR;
#endif
typedef	char	CHAR;
typedef	U char	UCHAR;
typedef	char *	CSTR;
typedef	int	INT;
typedef	U int	UINT;
typedef	LONG	(*FUNC)();
#define	JUMP	jmp_buf

/***********************************************************************
**	Global Constants
***********************************************************************/

#ifndef TRUE
#define	TRUE	1
#define	FALSE	0
#endif
#define NIL	0
#define	KILO	1024
#define	K	*1024 
#define PI	3.14159265358979323846
#define	PI_RAD	(PI/180.0)

/***********************************************************************
**	Special Macros
***********************************************************************/

#define	EVEN(n)		(((n)+1) & 0xfffffffe)
#define	ALIGN2(n)	(((n)+1) & 0xfffffffe)
#define	ALIGN4(n)	(((n)+3) & 0xfffffffc)
#define	MAX(a,b)	(((a)<(b))?(b):(a))
#define	MIN(a,b)	(((a)>(b))?(b):(a))
#define	CTRL(c)		(c - 'A' + 1)
#define	LOOP		while(1) 
#define	SPAN(v,n)	for(v=0;v<n;v++) 
#define	ISNIL(n)	((n) == NIL)
#define	SHOWI(z)	Print("z=%ld\n",z)
#define	SHOWH(z)	Print("z=0x%lx\n",z)
#define	SHOWS(z)	Print("z=%ls\n",z)
#define	DEBUG(s)	if (DebugFlag) PutStr(s);
#define	DEBUG1(s,v)	if (DebugFlag) Print(s,v)
#define	DEBUG2(s,v,x)	if (DebugFlag) Print(s,v,x)

#endif
