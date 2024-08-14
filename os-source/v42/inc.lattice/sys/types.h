/*  TYPES.H, /usr/include/sys, 06/21/85 */

#ifndef _TYPES_
#define	_TYPES_
/*
 * Basic system types and major/minor device constructing/busting macros.
 */
#if 0 /* not applicable to the AMIGA */
/* major part of a device */
#define	major(x)	((int)(((unsigned)(x)>>8)&0377))

/* minor part of a device */
#define	minor(x)	((int)((x)&0377))

/* make a device number */
#define	makedev(x,y)	((dev_t)(((x)<<8) | (y)))
#endif

typedef	unsigned char	u_char;
typedef	unsigned short	u_short;
typedef	unsigned int	u_int;
typedef	unsigned long	u_long;
typedef	unsigned short	ushort;		/* sys III compat */

#ifdef vax
typedef	struct	_physadr { int r[1]; } *physadr;
typedef	struct	label_t	{
	int	val[14];
} label_t;
#endif
typedef	struct	_quad { long val[2]; } quad;
typedef	long	   daddr_t;
typedef	char *	caddr_t;
typedef	u_long	ino_t;
typedef	long	   swblk_t;
typedef	unsigned int	   size_t;
typedef	long	   time_t;
typedef	long	   dev_t;
typedef	long	   off_t;
typedef  long     uid_t;
typedef  long     gid_t;
typedef	struct	fd_set { int fds_bits[1]; } fd_set;
#endif
