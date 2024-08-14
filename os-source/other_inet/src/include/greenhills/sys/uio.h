/*
** Scatter/gather memory specifier.
*/

#ifndef SYS_UIO_H
#define SYS_UIO_H

struct iovec {
	caddr_t	iov_base;
	long	iov_len;
};

#endif SYS_UIO_H
