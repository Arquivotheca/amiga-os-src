#ifndef SYS_TYPES_H
#define SYS_TYPES_H

/*	#include <exec/types.h>		*/

typedef	char * 		caddr_t;
typedef unsigned long 	u_long;
typedef unsigned int	u_int;
typedef unsigned short	u_short;
typedef unsigned char	u_char;
typedef unsigned short	ushort;
typedef unsigned long	ulong;
typedef unsigned char	uchar;
typedef short		dev_t;

#define	btod(p, t) ((t)(((long)p)<<2))
#define dtob(p) ((BPTR)((long)(p)>>2))

#define MIN(x, y) ((x) < (y) ? (x):(y))

#define DOS_TRUE -1
#define DOS_FALSE 0

#endif SYS_TYPES_H
