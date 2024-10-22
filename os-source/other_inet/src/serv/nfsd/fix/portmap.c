/*
 * Copyright (c) 1984 by Sun Microsystems, Inc.
 */

/*
 * portmap.c, Implements the program,version to port number mapping for
 * rpc.
 */

#include <rpc/rpc.h>
#include <rpc/pmap_prot.h>
#include <nfs/nfs.h>
// #include <rpcsvc/mount.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <dos/dos.h>
#include "nfsd.h"

/* local functions */
static int makesock(int port);
static void startpmap(void);
static struct pmaplist *find_service(u_long prog, u_long vers, int prot);
static bool_t xdr_encap_parms(XDR *xdrs, struct encap_parms *epp);
static bool_t xdr_rmtcall_args(register XDR *xdrs, register struct rmtcallargs *cap);
static bool_t xdr_rmtcall_result(register XDR *xdrs, register struct rmtcallargs *cap);
static bool_t xdr_opaque_parms(XDR *xdrs, struct rmtcallargs *cap);
static bool_t xdr_len_opaque_parms(register XDR *xdrs, struct rmtcallargs *cap);

static struct pmaplist *pmaplist;
static int debugging = 1;
static struct pmaplist *mnt, *nfs;
static SVCXPRT *nfstransp, *mnttransp;

void
abort()
{
	cleanup(RETURN_ERROR);
}

static
makesock(port)
	int	port;
{
	struct sockaddr_in my_sock;
	int s;

	my_sock.sin_addr.s_addr = INADDR_ANY;
	my_sock.sin_family = AF_INET;
	my_sock.sin_port = htons(port);

	if((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		return -1;
	}
	if(bind(s, (struct sockaddr *)&my_sock, sizeof(my_sock)) < 0){
		return -1;
	}

	return s;
}

int
init_pmap()
{
	startpmap();

	mnttransp = svcudp_create(RPC_ANYSOCK);
	if (mnttransp == NULL) {
		return NFSERR_IO;
	}

	mnt = (struct pmaplist *)malloc((u_int)sizeof(struct pmaplist));
	mnt->pml_map.pm_prog = MOUNTPROG;
	mnt->pml_map.pm_vers = MOUNTVERS;
	mnt->pml_map.pm_prot = IPPROTO_UDP;
	mnt->pml_map.pm_port = mnttransp->xp_port;
	mnt->pml_next = pmaplist;
	pmaplist = mnt;
	(void)svc_register(mnttransp, MOUNTPROG, MOUNTVERS,
					mountprog_1, FALSE);

	nfstransp = svcudp_create(makesock(NFS_PORT));
	if (nfstransp == NULL) {
		return NFSERR_IO;
	}

	nfs = (struct pmaplist *)malloc((u_int)sizeof(struct pmaplist));
	nfs->pml_map.pm_prog = NFS_PROGRAM;
	nfs->pml_map.pm_vers = NFS_VERSION;
	nfs->pml_map.pm_prot = IPPROTO_UDP;
	nfs->pml_map.pm_port = htons(NFS_PORT);
	nfs->pml_next = pmaplist;
	pmaplist = nfs;
	(void)svc_register(nfstransp, NFS_PROGRAM, NFS_VERSION,
					nfs_program_2, FALSE);

	return NFS_OK;
}

void
clean_pmap()
{
	if(mnttransp){
		SVC_DESTROY(mnttransp);
		mnttransp = 0;
	}

	if(nfstransp){
		SVC_DESTROY(nfstransp);
		nfstransp = 0;
	}

	if(mnt){
		free(mnt);
		mnt = 0;
	}
	if(nfs){
		free(nfs);
		nfs = 0;
	}
}

static void
startpmap()
{
	SVCXPRT *xprt;
	int sock;
	struct sockaddr_in addr;
	int len = sizeof(struct sockaddr_in);
	register struct pmaplist *pml;

	if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		perror("portmap cannot create socket");
		cleanup(RETURN_ERROR);
	}

	addr.sin_addr.s_addr = 0;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PMAPPORT);
	if (bind(sock, (struct sockaddr *)&addr, len) != 0) {
		perror("portmap cannot bind UDP");
		cleanup(RETURN_ERROR);
	}

	if ((xprt = svcudp_create(sock)) == (SVCXPRT *)NULL) {
		fprintf(stderr, "couldn't do udp_create\n");
		cleanup(RETURN_ERROR);
	}
	/* make an entry for ourself */
	pml = (struct pmaplist *)malloc((u_int)sizeof(struct pmaplist));
	pml->pml_next = 0;
	pml->pml_map.pm_prog = PMAPPROG;
	pml->pml_map.pm_vers = PMAPVERS;
	pml->pml_map.pm_prot = IPPROTO_UDP;
	pml->pml_map.pm_port = PMAPPORT;
	pmaplist = pml;

	if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		perror("portmap cannot create socket");
		cleanup(RETURN_ERROR);
	}
	if (bind(sock, (struct sockaddr *)&addr, len) != 0) {
		perror("portmap cannot bind TCP");
		cleanup(RETURN_ERROR);
	}
	if ((xprt = svctcp_create(sock, RPCSMALLMSGSIZE, RPCSMALLMSGSIZE))
	    == (SVCXPRT *)NULL) {
		fprintf(stderr, "couldn't do tcp_create\n");
		cleanup(RETURN_ERROR);
	}
	/* make an entry for ourself */
	pml = (struct pmaplist *)malloc((u_int)sizeof(struct pmaplist));
	pml->pml_map.pm_prog = PMAPPROG;
	pml->pml_map.pm_vers = PMAPVERS;
	pml->pml_map.pm_prot = IPPROTO_TCP;
	pml->pml_map.pm_port = PMAPPORT;
	pml->pml_next = pmaplist;
	pmaplist = pml;

	(void)svc_register(xprt, PMAPPROG, PMAPVERS, reg_service, FALSE);
}

static struct pmaplist *
find_service(prog, vers, prot)
	u_long prog;
	u_long vers;
{
	register struct pmaplist *hit = NULL;
	register struct pmaplist *pml;

	for (pml = pmaplist; pml != NULL; pml = pml->pml_next) {
		if ((pml->pml_map.pm_prog != prog) ||
			(pml->pml_map.pm_prot != prot))
			continue;
		hit = pml;
		if (pml->pml_map.pm_vers == vers)
		    break;
	}
	return (hit);
}

/*
 * 1 OK, 0 not
 */
void
reg_service(rqstp, xprt)
	struct svc_req *rqstp;
	SVCXPRT *xprt;
{
	struct pmap reg;
	struct pmaplist *pml, *prevpml, *fnd;
	int ans, port;
	caddr_t t;

	switch (rqstp->rq_proc) {

	case PMAPPROC_NULL:
		/*
		 * Null proc call
		 */
		if ((!svc_sendreply(xprt, xdr_void, NULL)) && debugging) {
			abort();
		}
		break;

	case PMAPPROC_SET:
		/*
		 * Set a program,version to port mapping
		 */
		if (!svc_getargs(xprt, xdr_pmap, &reg)){
			svcerr_decode(xprt);
		} else {
			/*
			 * check to see if already used
			 * find_service returns a hit even if
			 * the versions don't match, so check for it
			 */
			fnd = find_service(reg.pm_prog, reg.pm_vers, reg.pm_prot);
			if (fnd && fnd->pml_map.pm_vers == reg.pm_vers) {
				if (fnd->pml_map.pm_port == reg.pm_port) {
					ans = 1;
					goto done;
				}
				else {
					ans = 0;
					goto done;
				}
			} else {
				/*
				 * add to END of list
				 */
				pml = (struct pmaplist *)
				    malloc((u_int)sizeof(struct pmaplist));
				pml->pml_map = reg;
				pml->pml_next = 0;
				if (pmaplist == 0) {
					pmaplist = pml;
				} else {
					for (fnd= pmaplist; fnd->pml_next != 0;
					    fnd = fnd->pml_next);
					fnd->pml_next = pml;
				}
				ans = 1;
			}
		done:
			if ((!svc_sendreply(xprt, xdr_long, (caddr_t)&ans)) &&
			    debugging) {
				fprintf(stderr, "svc_sendreply\n");
				abort();
			}
		}
		break;

	case PMAPPROC_UNSET:
		/*
		 * Remove a program,version to port mapping.
		 */
		if (!svc_getargs(xprt, xdr_pmap, &reg))
			svcerr_decode(xprt);
		else {
			ans = 0;
			for (prevpml = NULL, pml = pmaplist; pml != NULL; ) {
				if ((pml->pml_map.pm_prog != reg.pm_prog) ||
					(pml->pml_map.pm_vers != reg.pm_vers)) {
					/* both pml & prevpml move forwards */
					prevpml = pml;
					pml = pml->pml_next;
					continue;
				}
				/* found it; pml moves forward, prevpml stays */
				ans = 1;
				t = (caddr_t)pml;
				pml = pml->pml_next;
				if (prevpml == NULL)
					pmaplist = pml;
				else
					prevpml->pml_next = pml;
				free(t);
			}
			if ((!svc_sendreply(xprt, xdr_long, (caddr_t)&ans)) &&
			    debugging) {
				fprintf(stderr, "svc_sendreply\n");
				abort();
			}
		}
		break;

	case PMAPPROC_GETPORT:
		/*
		 * Lookup the mapping for a program,version and return its port
		 */
		if (!svc_getargs(xprt, xdr_pmap, &reg))
			svcerr_decode(xprt);
		else {
			fnd = find_service(reg.pm_prog, reg.pm_vers, reg.pm_prot);
			if (fnd)
				port = fnd->pml_map.pm_port;
			else
				port = 0;
			if ((!svc_sendreply(xprt, xdr_long, (caddr_t)&port)) &&
			    debugging) {
				fprintf(stderr, "svc_sendreply\n");
				abort();
			}
		}
		break;

	case PMAPPROC_DUMP:
		/*
		 * Return the current set of mapped program,version
		 */
		if (!svc_getargs(xprt, xdr_void, NULL))
			svcerr_decode(xprt);
		else {
			if ((!svc_sendreply(xprt, xdr_pmaplist,
			    (caddr_t)&pmaplist)) && debugging) {
				fprintf(stderr, "svc_sendreply\n");
				abort();
			}
		}
		break;

	default:
		svcerr_noproc(xprt);
		break;
	}
}


/*
 * Stuff for the rmtcall service
 */
#define ARGSIZE 9000

typedef struct encap_parms {
	u_long arglen;
	char *args;
};

static bool_t
xdr_encap_parms(xdrs, epp)
	XDR *xdrs;
	struct encap_parms *epp;
{

	return (xdr_bytes(xdrs, &(epp->args), &(epp->arglen), ARGSIZE));
}

typedef struct rmtcallargs {
	u_long	rmt_prog;
	u_long	rmt_vers;
	u_long	rmt_port;
	u_long	rmt_proc;
	struct encap_parms rmt_args;
};

static bool_t
xdr_rmtcall_args(xdrs, cap)
	register XDR *xdrs;
	register struct rmtcallargs *cap;
{

	/* does not get a port number */
	if (xdr_u_long(xdrs, &(cap->rmt_prog)) &&
	    xdr_u_long(xdrs, &(cap->rmt_vers)) &&
	    xdr_u_long(xdrs, &(cap->rmt_proc))) {
		return (xdr_encap_parms(xdrs, &(cap->rmt_args)));
	}
	return (FALSE);
}

static bool_t
xdr_rmtcall_result(xdrs, cap)
	register XDR *xdrs;
	register struct rmtcallargs *cap;
{
	if (xdr_u_long(xdrs, &(cap->rmt_port)))
		return (xdr_encap_parms(xdrs, &(cap->rmt_args)));
	return (FALSE);
}

/*
 * only worries about the struct encap_parms part of struct rmtcallargs.
 * The arglen must already be set!!
 */
static bool_t
xdr_opaque_parms(xdrs, cap)
	XDR *xdrs;
	struct rmtcallargs *cap;
{

	return (xdr_opaque(xdrs, cap->rmt_args.args, cap->rmt_args.arglen));
}

/*
 * This routine finds and sets the length of incoming opaque paraters
 * and then calls xdr_opaque_parms.
 */
static bool_t
xdr_len_opaque_parms(xdrs, cap)
	register XDR *xdrs;
	struct rmtcallargs *cap;
{
	register u_int beginpos, lowpos, highpos, currpos, pos;

	beginpos = lowpos = pos = xdr_getpos(xdrs);
	highpos = lowpos + ARGSIZE;
	while ((int)(highpos - lowpos) >= 0) {
		currpos = (lowpos + highpos) / 2;
		if (xdr_setpos(xdrs, currpos)) {
			pos = currpos;
			lowpos = currpos + 1;
		} else {
			highpos = currpos - 1;
		}
	}
	xdr_setpos(xdrs, beginpos);
	cap->rmt_args.arglen = pos - beginpos;
	return (xdr_opaque_parms(xdrs, cap));
}

