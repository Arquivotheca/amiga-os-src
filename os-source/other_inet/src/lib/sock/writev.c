
/****** socket/writev  ***********************************************
*
*	NAME
*		writev - write multiple buffers as an atomic operation.
*
*	SYNOPSIS
*		bytes_written = writev(file_descriptor, iovec, count)
*
*		int writev(int , iovec *, int )
*
*	FUNCTION
*		Encapsulates multiple data buffers and writes the buffer
*		to the named file descriptor.
*
*	INPUTS
*		file  - A file descriptor or socket.
*		iovec - A pointer to the base of a structure array holding 
*			the data and length of data to be written.
*		count - The number of iovec structures in the array.
*
*	RESULT
*		Number of bytes written if sucessful.
*		(-1) upon failure with global int 'errno' set to 'EIO' as
*		defined in <errno.h>
*
*	NOTES
*		Errors in midwrite will return a -1 but the real status of things
*		is unknown. IE, you could pass 10 iovec entries and have the
*		call fail after three have been written.
*
*	SEE ALSO
*		readv()
*   
********************************************************************
*
*
*/

#include <sys/types.h>
#include <sys/uio.h>
#include <errno.h>

#ifndef EIO
#define EIO 5
#endif

int
writev(int fd, register struct iovec *iovp, int iovcnt)
{
	register int total, cc, cnt, wlen ;
	register caddr_t p ;

	errno = 0 ;
	for( total = 0 ; iovcnt-- ; iovp++ )
		{
		if( iovp->iov_len == 0 )
			{
			continue ;
			}
		p = iovp->iov_base ;
		for(cnt = 0 ; cnt < iovp->iov_len ; cnt += cc)
			{
			wlen = iovp->iov_len - cnt ;
			cc = write(fd, p, wlen) ; 
			if(cc <= 0)
				{
				errno = EIO ;
				return -1 ;
				}
			cnt += cc ;
			p += cc ;
			}
		total += iovp->iov_len ;
		}
	return total ;
}

/* eof  */