/*
** Dummy writev
*/

#include <sys/types.h>
#include <sys/uio.h>

int
writev(fd, iovp, iovcnt)
	int	fd;
	register struct iovec *iovp;
	int	iovcnt;
{
	register int total, cc, cnt, wlen;
	register caddr_t p;

	for(total = 0; iovcnt--; iovp++){
		if(!iovp->iov_len){
			continue;
		}

		p = iovp->iov_base;
		for(cnt = 0; cnt < iovp->iov_len; cnt += cc){
			wlen = iovp->iov_len - cnt;
			cc = write(fd, p, wlen); 
			if(cc <= 0){
				/*
				** Not really "right" since we may have
				** written data already.  This case is even
				** less clear than readv() failing - so I'll
				** just punt.
				*/
				return -1;
			}

			cnt += cc;
			p += cc;
		}

		total += iovp->iov_len;
	}

bottom:
	return total;
}

