/* -----------------------------------------------------------------------
 * sockets.c   rlogin v39   rewrite
 *
 *    socket init and delete stuff.
 *
 * $Locker:  $
 *
 * $Id: sockets.c,v 38.0 93/04/08 15:46:27 bj Exp $
 *
 * $Revision: 38.0 $
 *
 * $Log:	sockets.c,v $
 * Revision 38.0  93/04/08  15:46:27  bj
 * This is the complete rewrite of rlogin (v38.xx)
 * These are the socket init and delete functions.
 * 
 * Revision 38.0  93/04/08  15:29:14  bj
 *  This is the complete rewrite of rlogin (v 38.xx.)
 * socket init and delete functions
 * 
 *
 * $Header: AS225:src/c/rlogin/RCS/sockets.c,v 38.0 93/04/08 15:46:27 bj Exp $
 *
 *------------------------------------------------------------------------
 */


#include "rl.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <ss/ssfunctions.h>
#include <ss/socket_pragmas.h>


/* =====================================================================
 * Initialize the socket stuff
 * =====================================================================
 */

BOOL
InitSocketStuff(struct glob *g)
{
	SYSBASE ;
	DOSBASE(g) ;
	struct Library *SockBase ;
	char *hostptr = (char *) &(g->g_host) ;
	int cc ;
	ULONG ne, ue ;
	
	g->g_error = ERR_INITSOCKET ;
	if (SockBase = OpenLibrary("inet:lib/socket.library", 0L ))
	{
		g->g_SockBase = SockBase ;
		setup_sockets(MAXSOCKS, &g->g_errno) ;
		g->g_sockets_are_setup = 1L ;

		ne = (ULONG)(1L << s_getsignal( SIGIO )) ;
		ue = (ULONG)(1L << s_getsignal( SIGURG )) ;
		if( ue && ne )
		{
			g->g_netevent = ne ;
			g->g_urgevent = ue ;

			if(  (*(g->g_user)) == '\0' ) /* user didnt use -l=user option */
			{
				CopyMem(getlogin(), g->g_user, MAX_USER);
			}
			CopyMem(g->g_user, g->g_remote_user, MAX_USER) ;
			
			if((g->g_socket = rcmd( &hostptr, 513, g->g_user,g->g_remote_user,g->g_termtype,(int *)NULL)) >=0L)
			{
				cc = -1 ;
				if(s_ioctl(g->g_socket,FIONBIO,(BYTE *)&cc)<0 || s_ioctl(g->g_socket, FIOASYNC, (BYTE *)&cc)<0)
				{
					PutStr("ioctl\n") ;
				}
				return(TRUE) ;
			}
		}
	}
	return(FALSE) ;
}

/* =====================================================================
 * Kill the socket stuff
 * =====================================================================
 */

VOID
KillSocketStuff(struct glob *g)
{
	SYSBASE ;
	SOCKBASE(g) ;

	if( SockBase )
	{
		if( g->g_sockets_are_setup )
		{
			cleanup_sockets() ;
		}
		if(g->g_SockBase)
		{
			CloseLibrary(g->g_SockBase) ;
		}
	}
}

	
/* end */
