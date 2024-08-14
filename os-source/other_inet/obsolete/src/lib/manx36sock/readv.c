/*
** Dummy readv
*/

#include <sys/types.h>
#include <sys/uio.h>

int
readv(fd, iovp, iovcnt)
	int	fd;
	register struct iovec *iovp;
	int	iovcnt;
{
	register int total, cc, cnt;
	register caddr_t p;

	for(total = 0; iovcnt--; iovp++){
		if(!iovp->iov_len){
			continue;
		}

		p = iovp->iov_base;
		for(cnt = 0; cnt < iovp->iov_len; cnt += cc){
			cc = read(fd, p, iovp->iov_len - cnt); 
			if(cc <= 0){
				total += cnt;
				/*
				** I'm not sure what the "right" thing to
				** do here is.  If we've already ready read
				** data, then should we return -1 on error or
				** the partial buffer cnt?  We'll be optimistic
				** and return all data up until the error and
				** let the next readv() bomb...
				*/
				return cnt==0? -1:total; 
			}

			cnt += cc;
			p += cc;
		}

		total += iovp->iov_len;
	}

bottom:
	return total;
}
