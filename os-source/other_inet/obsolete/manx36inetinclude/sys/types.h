#ifndef SYS_TYPES_H
#define SYS_TYPES_H

typedef	char * 		caddr_t;
typedef unsigned long 	u_long;
typedef unsigned int	u_int;
typedef unsigned short	u_short;
typedef unsigned char	u_char;
typedef unsigned short	ushort;
typedef unsigned long	ulong;
typedef unsigned char	uchar;
typedef short		dev_t;
typedef long		off_t;
typedef short		uid_t;
typedef short		gid_t;
typedef unsigned long	ino_t;

#define	btod(p, t) ((t)(((long)p)<<2))
#define dtob(p) ((BPTR)((long)(p)>>2))

#define MIN(x, y) ((x) < (y) ? (x):(y))
#define min(x, y) MIN(x, y)

#define DOS_TRUE -1
#define DOS_FALSE 0

#ifndef NULL
#define NULL 0L
#endif

/*
 * Select mask manipulations from Berkeley Unix.
 */
#define NFDPOW		5
#define NFDBITS		(1 << NFDPOW)		/* 32 bits/word */

#ifndef FD_SETSIZE
#define FD_SETSIZE	64
#endif

typedef struct fd_set {
	u_long	fds_bits[FD_SETSIZE/NFDBITS];
} fd_set;

#define OFF(bit) ((bit)>>NFDPOW)
#define BIT(bit) ((bit)&(NFDBITS-1))

#define FD_SET(bit, setp) ((setp)->fds_bits[OFF(bit)] |= (1 << BIT(bit)))
#define FD_CLR(bit, setp) ((setp)->fds_bits[OFF(bit)] &= ~(1 << BIT(bit)))
#define FD_ISSET(bit, setp) ((setp)->fds_bits[OFF(bit)] & (1 << BIT(bit)))
#define FD_ZERO(setp) bzero(setp, sizeof(fd_set));

#ifndef LATTICE
#define defined(x) ((x) != 0)	/* Manx CPP doesn't do defined(x) */
#endif

#endif SYS_TYPES_H
