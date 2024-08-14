/* -----------------------------------------------------------------------
 * showmount.c   SAS 5.10
 *
 * $Locker:  $
 *
 * $Id: showmount.c,v 1.3 94/03/24 18:52:02 jesup Exp $
 *
 * $Revision: 1.3 $
 *
 * $Log:	showmount.c,v $
 * Revision 1.3  94/03/24  18:52:02  jesup
 * Checkin of BJ's changes.  template is different, better error handling.
 * 
 *
 * $Header: Hog:Other/inet/src/c/showmount/RCS/showmount.c,v 1.3 94/03/24 18:52:02 jesup Exp $
 *
 *------------------------------------------------------------------------
 */
/*
** showmount: print mount export list of specified host
*/

/* shared library stuff */
#include <ss/socket.h>

#include <dos/dos.h>
#include <proto/all.h>

#include <sys/types.h>
#include <rpc/rpc.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <rpcsvc/mount.h>
#include <netdb.h>

#include <string.h>

void list_exports( CLIENT *cl, struct timeval tout);
void dump_all( CLIENT *cl, struct timeval tout);
int main(void);
int doit(char *host);
xdr_mountlist( XDR *xdrs, int dummy);
xdr_groups( XDR *xdrs, int glist);
xdr_export_list ( XDR *xdrs, int elist);

#include "showmount_rev.h"

static char *showmvers = VERSTAG ;

struct Library *SockBase ;

int eopt;

xdr_mountlist( xdrs, dummy)
	XDR	*xdrs;
	int	dummy;
{
	char	hostname [MNTNAMLEN];
	char	dirpath [MNTPATHLEN];
	char	*host = hostname, *path = dirpath, *s;
	bool_t	more;

	do {
		if (!xdr_bool( xdrs, &more)){
			return ( FALSE );
		}

		if (!more){
			return( TRUE );
		}

		if (!xdr_string( xdrs, &host, sizeof(hostname))){
			return( FALSE );
		}

		s = host;
		while (*s != '\0'){
			if (*s < ' ' || (int)*s > 127){
				*s = '\0';
			} else {
				s++;
			}
		}

		if (!xdr_string( xdrs, &path, sizeof(dirpath))){
			return( FALSE );
		}

		Printf ("%s \t%s\n", hostname, dirpath);

	} while (TRUE);
}

/*
** Output group list separated by commas
*/
xdr_groups( xdrs, glist )
	XDR	*xdrs;
	int	glist;
{
	char group[MNTNAMLEN], *gp;
	bool_t more;

	for(gp = group; TRUE; ) {
		if (!xdr_bool(xdrs, &more)){
			return (FALSE);
		}

		if (!more) {
			PutStr ("\n");
			return (TRUE);
		}
		if (!xdr_string(xdrs, &gp, sizeof(group))){
			return (FALSE);
		}

		if(group[0]){
			Printf ("%s, ", group);
		}
	}
}

xdr_export_list ( xdrs, elist )
	XDR	*xdrs;
	int	elist;
{
	bool_t	more;
	char	dirpath[MNTPATHLEN], *dp;

	for(dp = dirpath; TRUE; ) {
		if (!xdr_bool(xdrs, &more)){
			return (FALSE);
		}

		if (!more){
			return (TRUE);
		}

		if (!xdr_string(xdrs, &dp, sizeof(dirpath))){
			return (FALSE);
		}

		Printf ("%s\t", dirpath);

		if (!xdr_groups(xdrs, NULL)){
			return (FALSE);
		}
	}
}

void
list_exports( cl, tout )
	CLIENT	*cl;
	struct	timeval	tout;
{
	PutStr ("Filesystem\tGroups\n");
	clnt_call(cl, MOUNTPROC_EXPORT, xdr_void, 	 (caddr_t)NULL,
					xdr_export_list, (caddr_t)NULL, tout);
}

void
dump_all( cl , tout)
	CLIENT	*cl;
	struct	timeval	tout;
{
	PutStr ("Host\tFilesystem\n");
	clnt_call(cl, MOUNTPROC_DUMP, 	xdr_void, 	(caddr_t)NULL,
					xdr_mountlist, 	(caddr_t)NULL, tout);
}

struct	timeval	totaltout = {10, 0}, period = {2, 0};

int doit (char *host)
{
	CLIENT 	*cl;

	cl = clnt_create(host, MOUNTPROG, MOUNTVERS, "udp");
	if (cl == (CLIENT *)0){
		clnt_spcreateerror("showmount");
		Printf("Error connecting to host %s\n",host);
		return(RETURN_ERROR);
	}

	clnt_control(cl, CLSET_TIMEOUT, &totaltout);
	clnt_control(cl, CLSET_RETRY_TIMEOUT, &period);

	if (eopt){
		dump_all(cl, totaltout);
	} else {
		list_exports(cl, totaltout);
	}

	clnt_destroy(cl);
	return(RETURN_OK) ;
}

#define CMDREV		"\0$VER: " VSTRING
#define TEMPLATE	"-E=EXPORTS/S,HOST/A" CMDREV
#define OPT_E		0
#define OPT_HOST	1
#define OPT_COUNT	2

int MAXSOCKS = 5;
struct RDargs *rdargs;

int main(void)
{
	long opts[OPT_COUNT+1];
	int error = RETURN_FAIL ;

    if(SockBase = OpenLibrary( "inet:libs/socket.library", 1L )) 
    {
    	setup_sockets(MAXSOCKS,&errno) ;
	    memset((char *)opts, 0, sizeof(opts));
    	if(rdargs = ReadArgs(TEMPLATE, opts, NULL))
		{
			if(opts[OPT_E])
			{
				eopt++;
			}

			if(opts[OPT_HOST])
			{
				error = doit((char *)(opts[OPT_HOST]));
			}
			else
			{
				PutStr ("usage: showmount [-e] hostname\n");
			}
			FreeArgs(rdargs) ;
		}
		else
		{
			PrintFault(IoErr(), "showmount") ;
		}
    	cleanup_sockets() ;
		CloseLibrary(SockBase);
	}    		
    else
    {
    	PutStr("Opening of socket library failed.\n");
    }
	return(error) ;
}

