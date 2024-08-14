
/****** socket/readv  *************************************************
*
*	NAME
*		readv - read data from a file into multiple buffers
*
*	SYNOPSIS
*		bytes = readv( file, iovec, count )
*
*		int  readv( int, struct iovec *, int )
*
*	FUNCTION
*		Readv() attempts to read nbytes of data from the object 
*		referenced by the file descriptor, scattering the input
*		data into the buffers specified by the members of the 
*		iovec following array: iov[0], iov[1], ..., iov[iovcnt-1].
*
*		The iovec structure:
*
*		struct iovec {
*			caddr_t   iov_base;
*			int  iov_len;
*		};
*
*		Each iovec entry specifies the base address and length of an
*		area in memory where data should be placed. The readv system
*		call fills an area completely before proceeding to the next
*		area.
*
*	INPUTS
*		file  - a socket or file descriptor
*		iovec - a pointer to an array of iovec structures
*		count - the number of iovec structures in iovec
*
*	RESULT
*		success - number of bytes read is successful
*		        - (-1) upon failure with the global integer 'errno'
*		          set to EOI (as defined in <errno.h>)
*
*	SEE ALSO
*		writev()
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
readv(int fd, register struct iovec *iovp, int iovcnt)
{
	register int total = 0 ;
	register int cc, cnt ;
	register caddr_t p ;

	errno = 0 ;
	for(total = 0 ; iovcnt-- ; iovp++) 
		{
		if(!iovp->iov_len) 
			{
			continue ;
			}
		p = iovp->iov_base ;
		for(cnt = 0 ; cnt < iovp->iov_len ; cnt += cc)
			{
			cc = read(fd, p, iovp->iov_len - cnt) ; 
			if(cc <= 0)
				{
				errno = EIO ;
				return (-1) ; 
				}
			cnt += cc ;
			p += cc ;
			}
		total += iovp->iov_len ;
		}
	return (total) ;
}
