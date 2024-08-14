/* -----------------------------------------------------------------------
 * CLNT_GENERIC.C (NFSC)
 *
 * $Locker:  $
 *
 * $Id: clnt_generic.c,v 1.3 91/08/06 15:59:17 martin Exp $
 *
 * $Revision: 1.3 $
 *
 * $Header: inet2:src/c/nfsc/RCS/clnt_generic.c,v 1.3 91/08/06 15:59:17 martin Exp $
 *
 *------------------------------------------------------------------------
 */



/*
 * Copyright (C) 1987, Sun Microsystems, Inc.
 */
#include "fs.h"
#include <sys/socket.h>
#include <sys/time.h>

/*
 * Generic client creation: takes (hostname, program-number, protocol) and
 * returns client handle. Default options are set, which the user can
 * change using the rpc equivalent of ioctl()'s.
 */
CLIENT *
clnt_create(hostname, prog, vers, proto)
	char *hostname;
	unsigned prog;
	unsigned vers;
	char *proto;
{
	struct sockaddr_in sin;
	struct timeval tv;
	CLIENT *client;
	int sock;

	sin.sin_addr.s_addr = inet_addr(hostname);
	if (sin.sin_addr.s_addr == -1) {
		rpc_createerr.cf_stat = RPC_UNKNOWNHOST;
		return (NULL);
	}
	sin.sin_family = AF_INET;
	sin.sin_port = 0;
	if (strcmp(proto, "udp") != 0) {
		rpc_createerr.cf_stat = RPC_UNKNOWNPROTO;
		rpc_createerr.cf_error.re_errno = EPFNOSUPPORT;
		return (NULL);
	}
	bzero(sin.sin_zero,sizeof(sin.sin_zero));
	sock = RPC_ANYSOCK;
	tv.tv_sec = 5;
	tv.tv_usec = 0;
	client = clntudp_create(&sin, prog, vers, tv, &sock);
	if (client == NULL) {
		return (NULL);
	}
	tv.tv_sec = 25;
	clnt_control(client, CLSET_TIMEOUT, &tv);
	return (client);
}
