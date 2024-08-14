#include "netcomm.h"
#include "sdninet.h"

#define DriverBase (tg->DriverBase)

/*
** Read "n" bytes from a socket descriptor.
** Use in place of read() when using TCP
**
*/

int readn (tg, src, ptr, nbytes)
INETGLOBAL tg;
long *src;
char *ptr;
int nbytes;
{
   int nleft, nread;
   struct recvfromargs ra;

   nleft = nbytes;
   while(nleft > 0)
   {
      ra.fp    = tg->_socks[tg->sock];
      ra.buf = ptr;
      ra.len = nleft;
      ra.flags = 0;
      ra.errno = 0L;
      RecvAsm(&ra);
      if (ra.errno)
         return(-1);
      nread = ra.rval;
      if (nread==0)
         break;

      nleft -= nread;
      ptr += nread;
   }
    
   return (nbytes-nleft);
}


int writen (tg, targ, ptr, nbytes)
INETGLOBAL tg;
long targ;
char *ptr;
int nbytes;
{
   int nleft, nwritten;
   struct sendtoargs sa;

   nleft = nbytes;
   while(nleft > 0)
   {
      sa.fp    = tg->_socks[tg->sock];
      sa.buf   = ptr;
      sa.len   = nleft;
      sa.flags = 0;
      sa.to    = &targ;
      sa.tolen = sizeof(long);
      sa.errno = 0;
      SendToAsm(&sa);
      if (sa.errno)
         return(-1);
      nwritten = sa.rval;
      nleft -= nwritten;
      ptr += nwritten;
   }
   return (nbytes-nleft);
}

int ibind(tg, fd, name, len)
INETGLOBAL tg;
int fd;
void *name;
int     len;
{
   struct bindargs ba;

   ba.errno = 0L;
   ba.fp = tg->_socks[fd];
   ba.name = name;
   ba.namelen = len;
   BindAsm(&ba);
   return(ba.errno ? -1:0);
}

int isocket(tg)
INETGLOBAL tg;
{
   struct sockargs sa;
   int ns;

   for (ns = 0; ns < FD_SETSIZE; ns++)
   {
      if (tg->_socks[ns] == 0) break;
   }
   if (ns >= FD_SETSIZE)
      return(-1);

   sa.errno    = 0 ;
   sa.domain   = AF_INET ;
   sa.type     = SOCK_DGRAM;
   sa.protocol = 0;
   sa.sigurg   = tg->socket_sigurg ;
   sa.sigio    = tg->socket_sigio ;

   SocketAsm(&sa) ;
   if (sa.errno)
      return(-1);

   tg->_socks[ns] = sa.rval;

   return ns;
}



void iclose(tg, fd)
INETGLOBAL tg;
int fd;
{
   struct closeargs ca ;
	
   ca.fp = tg->_socks[fd];
   ca.errno = 0;
   NetcloseAsm(&ca);
	tg->_socks[fd] = 0;
}

int INETprivate()
{
   return(0);
}

