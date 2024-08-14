/*
 * main dispatch loop for NFSd
 */

#include <rpc/rpc.h>
#include <nfs/nfs.h>
#include <stdio.h>
#include <errno.h>
#include <dos/dos.h>
#include <string.h>

#include "nfsd.h"
#include "stat.h"

struct serverstats stat;	/* server statistics variable		*/
static struct MsgPort *mp;	/* Port where rexx messages arrive 	*/

static void rexx_parse(struct RexxMsg *rm);

/*
 * main loop of interpreter.  Flow of control:
 *
 * do forever {
 *	selectwait - wait for Rexx msg or NFS request
 *	if rexx message has arrived then {
 *		parse message and execute it
 *	}
 *	if NFS request has arrived then {
 *		parse request and execute it
 *	}
 * }
 */
void
svc_run()
{
	register struct RexxMsg *rm;
	void rexx_parse();
	extern int errno;
	fd_set readfds;
	long mask, rtn;
	int tsize;

	tsize = _rpc_dtablesize();
	while(1) {
		readfds = svc_fdset;
		mask = 1L << mp->mp_SigBit;

		rtn = selectwait(tsize, &readfds, (int *)0, (int *)0,
					(struct timeval *)0, &mask);

		while(rm = (struct RexxMsg *)GetMsg(mp)){
			stat.rexx_call++;
			if(CheckRexxMsg(rm)){
				rexx_parse(rm);
			}
			ReplyMsg(rm);
		}

		switch(rtn){
		case -1:
			if (errno == EINTR) {
				continue;
			}
			perror("svc_run: - select failed");
			return;
		case 0:
			continue;
		default:
			svc_getreqset(&readfds);
		}
	}
}

/*
 * Initialize NFSd service
 */
void init_svc()
{
	char *portname = "NFSd";
	int created;

	if(mp == 0){
		created = 0;
		Forbid();
		if(!FindPort(portname)){
			mp = CreatePort(portname, 0);
			created++;
		}
		Permit();

		if(!created){
			fprintf(stderr, "NFSd: could not create port\n");
			cleanup(RETURN_WARN);
		}
	}
}

/*
 * cleanup service related stuff in preparation to quit
 */

void clean_svc()
{
	register struct RexxMsg *rm;

	if(mp){
		Forbid();
		while(rm = (struct RexxMsg *)GetMsg(mp)){
			seterror(rm, NFSERR_STALE);
			ReplyMsg(rm);
		}
		DeletePort(mp);
		mp = 0;
		Permit();
	}
}

/*
 * function dispatch table
 */

#define ARGCNT_0  1	/* no arguments		*/
#define ARGCNT_1  2	/* one arguments	*/
#define ARGCNT_2  3	/* two arguments	*/
#define ARGCNT_3  4	/* three arguments	*/

static struct ftable {
	char	*name;
	nfsstat	(*proc)();
	short	args;
} t[] = {
	{"buildmap",	build_map_file,	ARGCNT_3},
	{"buildvol",	build_volume, 	ARGCNT_1},
	{"deletemap",	delete_map,	ARGCNT_1},
	{"flushbitmap",	flush_bitmap,	ARGCNT_0},
	{"flushcache",	flush_open,	ARGCNT_0},
	{"getpath",	rexx_get_path,	ARGCNT_1},
	{"initcache",	init_cache,	ARGCNT_1},
	{"initmap",	init_map,	ARGCNT_1},
	{"nfsderrtxt",	nfserrtxt,	ARGCNT_1},
	{"query",	query,		ARGCNT_1},
	{"quit",	clean_up,	ARGCNT_0}
};
#define NUMCMD (sizeof(t)/sizeof(t[0]))

/*
 * Parse rexx msg and execute.  Returns error 10.1 if command not in table,
 * or error 10.17 if wrong number of args.
 *
 * Errors: a function returns NFSERR_OK on successful execution, otherwise
 * a specific nfsstat error code is returned.
 */
static void rexx_parse(struct RexxMsg *rm)
{
	register struct ftable *p;
	register int i, cnt;
	nfsstat err;

	rm->rm_Result1 = rm->rm_Result2 = 0;
	for(p = t; p < (t + NUMCMD); p++){
		if(stricmp(rm->rm_Args[0], p->name) == 0){
			/* check for right # of args */
/*
			for(cnt = i = 0; i < ARGCNT_3; i++){
				if(rm->rm_Args[i]){
					cnt++;
				}
			}
*/
cnt = p->args;
			if(cnt != p->args){
				rm->rm_Result1 = RC_ERROR;
				rm->rm_Result2 = ERR10_017; /* wrong # args */
			} else {
				err = (*p->proc)(rm);
				if(err == NFS_OK){
					if(!rm->rm_Result2){
						rm->rm_Result2 = CVi2arg(0, 8);
					}
				} else {
					seterror(rm, err);
				}
			}
			break;
		}
	}

	if(p == (t + NUMCMD)){
		rm->rm_Result1 = RC_ERROR;
		rm->rm_Result2 = ERR10_001;
	}
}
