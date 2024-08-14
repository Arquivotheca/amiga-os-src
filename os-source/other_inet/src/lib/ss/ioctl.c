/* -----------------------------------------------------------------------
 * ioctl.c
 *
 * $Locker:  $
 *
 * $Id: ioctl.c,v 1.2 91/08/07 14:16:47 bj Exp $
 *
 * $Revision: 1.2 $
 *
 * $Log:	ioctl.c,v $
 * Revision 1.2  91/08/07  14:16:47  bj
 * rcs header added.
 * 
 *
 * $Header: AS225:src/lib/ss/RCS/ioctl.c,v 1.2 91/08/07 14:16:47 bj Exp $
 *
 *------------------------------------------------------------------------
 */
/****** socket.library/s_ioctl ******************************************
*
*   NAME
*       s_ioctl -- Control socket options.
*
*   SYNOPSIS
*       return = s_ioctl( s, cmd, data )
*       D0              D0 D1   A0
*
*       int s_ioctl ( int, int, char * );
*
*   FUNCTION
*	Manipulates device options for a socket.
*
*   INPUTS
*       s		- socket descriptor.
*       cmd		- command.
*       data		- data.
*
*       The following commands are supported:
*
*       command         description                     data points to
*       -------         -----------                     --------------
*       FIONBIO         set/clear nonblocking I/O       int
*       FIOASYNC        set/clear async I/O             int
*       FIONREAD        get number of bytes to read     int
*       SIOCATMARK      at out-of-band mark?            int
*       SIOCSPGRP       set process group               int
*       SIOCGPGRP       get process group               int
*       SIOCADDRT       add route                       struct rtentry
*       SIOCDELRT       delete route                    struct rtentry
*       SIOCGIFCONF     get ifnet list                  struct ifconf
*       SIOCGIFFLAGS    get ifnet flags                 struct ifreq
*       SIOCSIFFLAGS    set ifnet flags                 struct ifreq
*       SIOCGIFMETRIC   get IF metric                   struct ifreq
*       SIOCSIFMETRIC   set IF metric                   struct ifreq
*       SIOCGARP        get ARP entry                   struct arpreq
*       SIOCSARP        get ARP entry                   struct arpreq
*       SIOCDARP        delete ARP entry                struct arpreq
*
*       For more information, see a Unix reference manual.
*
*   RESULT
*       return		- 0 on success, else -1 (errno will be set to the
*			  specific error code).
*
*   EXAMPLE
*       int one = 1;
*       ioctl ( s, FIOASYNC, (char *)&one);
*
*   NOTES
*       The standard Unix ioctl() function operates on both files and
*       sockets.  Some compiler vendors may supply an ioctl() function.
*       Because of this, and because this function works only with
*       sockets, it has been renamed to s_ioctl().
*
*   BUGS
*
*   SEE ALSO
*	setsockopts()
*
******************************************************************************
*
*/

#include "sslib.h"

int __saveds __asm s_ioctl(register __d0 int s,
    register __d1 int cmd,
    register __a0 char *data)
{
    struct ioctlargs ioa;

    GETSOCK(s, ioa.fp);
    ioa.cmd = (short)cmd;
    ioa.data = data;
    ioa.errno = (short)0;
    IoCtlAsm(&ioa);
    if (ioa.errno) {
        *ss_errno = ioa.errno;
        return (-1);
    }
    *ss_errno = 0;
    return 0;
}

