#include "netcomm.h"
#include "sdninet.h"

#define DriverBase (tg->DriverBase)

/*
** Read "n" bytes from a socket descriptor.
** Use in place of read() when using TCP
**
*/

int readn (tg, fd, ptr, nbytes)
INETGLOBAL tg;
int fd;
char *ptr;
int nbytes;
{
   int nleft;
   struct recvargs ra;
   struct selectargs sa;
   long netevent, timerevent, event, mask;

   nleft = nbytes;

   /* abort previous timer commands and clear bit */
   if(!CheckIO(&tg->tr)) {
      if(!AbortIO(&tg->tr))
         WaitIO(&tg->tr);
      SetSignal(0,1L<<tg->alarm_bit);
   }


   /* now send a new request */
   tg->tr.tr_node.io_Command = TR_ADDREQUEST;
   tg->tr.tr_time.tv_secs = 10;
   tg->tr.tr_time.tv_micro = 0;
   SendIO(&tg->tr);

   netevent = 1L << tg->socket_sigio;
   timerevent = 1L << tg->alarm_bit;

   ra.fp = tg->_socks[fd];
   ra.flags = 0;

   sa.socks  = tg->_socks;
   sa.task   = FindTask(NULL);
   sa.sigbit = tg->socket_sigio;
   sa.numfds = fd+1;
   sa.rd_set = &mask;
   sa.wr_set = 0;
   sa.ex_set = 0;
   sa.errno  = 0;
   sa.rval   = 0;

   while(nleft > 0) {
      mask = INETMASK(fd);
      SelectAsm(&sa);
      if (sa.errno) {
         BUG(("select error\n"));
         return(-1);
      }
      if (mask & INETMASK(fd)) {
         ra.buf = ptr;
         ra.len = nleft;
         ra.errno = 0L;
         RecvAsm(&ra);
         if(ra.errno != 0)
         {
            BUG(("receive error\n"))
            return(-1);
         }
         nleft -= ra.rval;
         ptr += ra.rval;
      } else {
         event = Wait(timerevent | netevent);
         if (event & timerevent) {
            BUG(("timer expired\n"));
            return (-1);
         }
      }
   }
   return (nbytes-nleft);
}



int writen (tg, fd, ptr, nbytes)
INETGLOBAL tg;
int fd;
char *ptr;
int nbytes;
{
   int nleft;
   struct sendargs sn;
   struct selectargs sa;
   long netevent, timerevent, event, mask;

   nleft = nbytes;

   /* abort previous timer commands and clear bit */
   if(!CheckIO(&tg->tr)) {
      if(!AbortIO(&tg->tr))
         WaitIO(&tg->tr);
      SetSignal(0,1L<<tg->alarm_bit);
   }

   /* now send a new request */
   tg->tr.tr_node.io_Command = TR_ADDREQUEST;
   tg->tr.tr_time.tv_secs = 10;
   tg->tr.tr_time.tv_micro = 0;
   SendIO(&tg->tr);

   netevent = 1L << tg->socket_sigio;
   timerevent = 1L << tg->alarm_bit;

   sn.fp = tg->_socks[fd];
   sn.flags = 0;

   sa.socks  = tg->_socks;
   sa.task   = FindTask(NULL);
   sa.sigbit = tg->socket_sigio;
   sa.numfds = fd+1;
   sa.rd_set = 0;
   sa.wr_set = &mask;
   sa.ex_set = 0;
   sa.errno  = 0;
   sa.rval   = 0;

   while(nleft > 0) {
      mask = INETMASK(fd);
      SelectAsm(&sa);
      if (sa.errno) {
         BUG(("select error\n"));
         return(-1);
      }

      if (mask & INETMASK(fd)) {
         sn.buf = ptr;
         sn.len = nleft;
         sn.errno = 0L;
         SendAsm(&sn);
         if(sn.errno != 0L)
         {
            BUG(("send error2\n"))
            return(-1);
         }
         nleft -= sn.rval;
         ptr += sn.rval;
      } else {
         event = Wait(netevent | timerevent);
         if (event & timerevent) {
            BUG(("timer expired\n"));
            return (-1);
         }
      }
   }
   return (nbytes-nleft);
}


int set_sockopts(tg, fd)
INETGLOBAL tg;
int fd;
{
	int on = 1;
	struct setsockoptargs ssa;
	struct ioctlargs ioa;

  	ssa.fp      = tg->_socks[fd];
	ssa.errno   = 0L;
	ssa.level   = IPPROTO_TCP;
	ssa.name    = TCP_NODELAY;
	ssa.valsize = sizeof(on);
	ssa.val     = (caddr_t)&on;
	SetSockOptAsm(&ssa);
        on = -1;
	ioa.fp      = tg->_socks[fd];
	ioa.cmd     = FIONBIO;
	ioa.data    = &on;
	ioa.errno   = 0L;
	IoCtlAsm(&ioa);
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

int iconnect(tg, fd, name, len)
INETGLOBAL tg;
int fd;
void *name;
int  len;
{
   struct connectargs ca;

   ca.errno = 0L;
   ca.fp = tg->_socks[fd];
   ca.name = name;
   ca.namelen = len;
   ConnectAsm(&ca);
   return(ca.errno ? -1:0);
}

int iaccept(tg, fd, name, lenp)
INETGLOBAL tg;
int fd;
void *name;
int     *lenp;
{
   struct acceptargs aa;
   int ns;

   for (ns = 0; ns < FD_SETSIZE; ns++)
   {
      if (tg->_socks[ns] == 0) break;
   }
   if (ns >= FD_SETSIZE)
      return(-1);

   aa.fp = tg->_socks[fd];
   aa.errno = 0;
   aa.rval = 0;
   aa.namelen = lenp? *lenp:0;
   aa.name = name;
   AcceptAsm(&aa);
   if(aa.errno)
      return(-1);

   if (ns > tg->fhscount) tg->fhscount = ns;

   tg->_socks[ns] = aa.rval;
   if(lenp)
   {
      *lenp = aa.namelen;
   }

   return ns;
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

   if (ns > tg->fhscount) tg->fhscount = ns;

   sa.errno    = 0 ;
   sa.domain   = AF_INET ;
   sa.type     = SOCK_STREAM;
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
   if (tg->fhscount == fd) tg->fhscount--;
}

int INETprivate()
{
   return(0);
}

