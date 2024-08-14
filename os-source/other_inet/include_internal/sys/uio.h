#ifndef SYS_UIO_H
#define SYS_UIO_H

enum uio_op {UIO_READ=1, UIO_WRITE=2};

struct iovec {
	char	*iov_base;
	int	iov_len;
};

struct uio {
	int	uio_resid;		/* Total data length in UIO	*/
	int	uio_iovcnt;		/* number of IOVs		*/
	struct	iovec *uio_iov;		/* struct iovec[uio_iovcnt];	*/
};

#endif
