
#include <netcomm.h>
extern int socket_sigio;

/*
** Read "n" bytes from a socket descriptor.
** Use in place of read() when using TCP
**
*/

int readn (fd, ptr, nbytes)
int fd;
char *ptr;
int nbytes;
{
    int nleft, nread;

    nleft = nbytes;
    while(nleft > 0) {
        nread = recv(fd,ptr,nleft,0);
        if(nread < 0) 
            return(nread);
        else if (nread==0)
            break;

        nleft -= nread;
        ptr += nread;
    }
    
    return (nbytes-nleft);
}


int writen (fd, ptr, nbytes)
int fd;
char *ptr;
int nbytes;
{
    int nleft, nwritten;

    nleft = nbytes;
    while(nleft > 0) {
        nwritten = send(fd,ptr,nleft,0);
        if(nwritten <= 0)
            return(nwritten);
        nleft -= nwritten;
        ptr += nwritten;
    }

    return (nbytes-nleft);
}
