/* ---------------------------------------------------------------------------------
 * SHOWMOUNT.C    (Manx 36)
 *
 * $Locker$
 *
 * $Id$
 *
 * $Revision$
 *
 * $Header$
 *
 *-----------------------------------------------------------------------------------
 * 
 *  showmount: print mount export list of specified host
 */

#include <libraries/dos.h> 

#include <sys/types.h>
#include <rpc/rpc.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <rpcsvc/mount.h>
#include <netdb.h>

#include "showmount_rev.h"

static char *compiler = "MANX36 MANX36" ;
static char *smvers = VERSTAG ;

void list_exports();
void dump_all();
void usage();
void doit();

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

		printf ("%s \t%s\n", hostname, dirpath);

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
			printf ("\n");
			return (TRUE);
		}
		if (!xdr_string(xdrs, &gp, sizeof(group))){
			return (FALSE);
		}

		if(group[0]){
			printf ("%s, ", group);
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

		printf ("%s\t", dirpath);

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
	printf ("Filesystem\tGroups\n");
	clnt_call(cl, MOUNTPROC_EXPORT, xdr_void, 	 (caddr_t)NULL,
					xdr_export_list, (caddr_t)NULL, tout);
}

void
dump_all( cl , tout)
	CLIENT	*cl;
	struct	timeval	tout;
{
	printf ("Host\tFilesystem\n");
	clnt_call(cl, MOUNTPROC_DUMP, 	xdr_void, 	(caddr_t)NULL,
					xdr_mountlist, 	(caddr_t)NULL, tout);
}

void
usage(code)
int code;
{
	printf ("usage: showmount [-e] hostname\n");
	exit(code);
}

struct	timeval	totaltout = {20, 0}, period = {2, 0};

void
doit(host)
	char *host;
{
	CLIENT 	*cl;

	cl = clnt_create(host, MOUNTPROG, MOUNTVERS, "udp");
	if (cl == (CLIENT *)0){
		clnt_spcreateerror("showmount");
		exit(1);
	}

	clnt_control(cl, CLSET_TIMEOUT, &totaltout);
	clnt_control(cl, CLSET_RETRY_TIMEOUT, &period);

	if (eopt){
		dump_all(cl, totaltout);
	} else {
		list_exports(cl, totaltout);
	}
		
	clnt_destroy(cl);
}

void
main(argc, argv)
	int	argc;
	char	**argv;
{
	if (argc < 2 || argc > 3)
		usage(RETURN_WARN);

	argv++;
	if( argv[0][0] == '-') {
		if (argv[0][1]=='e') {
			eopt++;
			argv++;
		} else
			usage(RETURN_WARN);
	} 
	doit(*argv);
}
