/* -----------------------------------------------------------------------
 * util.c  nfsmgr   SAS
 *
 * $Locker:  $
 *
 * $Id: util.c,v 1.4 94/03/24 22:13:42 jesup Exp $
 *
 * $Revision: 1.4 $
 *
 * $Header: Hog:Other/inet/src/c/nfsmgr/RCS/util.c,v 1.4 94/03/24 22:13:42 jesup Exp $
 *
 *------------------------------------------------------------------------
 */

// #define DEBUG 1
#ifdef DEBUG
	#define DB(x) kprintf(x)
#else
	#define DB(x) ;
#endif

/*
 * Util modules for nfsmgr code.
 */

#include <exec/types.h>
#include <exec/ports.h>
#include <dos/dos.h>
#include <proto/exec.h>
#include <proto/dos.h>

#include <sys/types.h>
#include <sys/time.h>

#include <rpc/types.h>
#include <rpc/rpc.h>
#include <nfs/nfs.h>
#include <nfs/clmsg.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

/* shared library stuff */
#include <ss/socket.h>
struct Library *SockBase ;
int errno;

#include "nfsmgr_rev.h"
static char *nfsmgrvers = VERSTAG ;

static int line = 1;

static pos = 0;
static char cmdline[256];
static FILE *infile;
static struct MsgPort *myreplyport=NULL;

void yyerror( char * );
struct MsgPort *FindClient( int );
struct MsgPort *FindServer( int );
void client( struct Message * );
int nfs_error( struct nfs_req * );
int myungetc( char );
char mygetchar( void );
void refreshclient( void );
void serverstats( char *volume );
void clientstats( char *volume );
void printstats( struct nfs_stats_req * );
void cleanup( int );

/* nfsmgr.y */
int yyparse( void );

void yyerror(char *s)
{
	printf("nfsmgr: line %d: %s\n", line, s);
}

struct MsgPort *FindClient(int start_if_not_found)
{
	struct MsgPort *port;

	DB("find client \n") ;
	Forbid();
	port = FindPort(CLIENT_COMMAND_PORT);
	Permit();

	if(!port && start_if_not_found){
		int retry;

		Execute("run >nil: inet:c/NFSc", 0L, 0L);
		for(retry = 10; retry-- > 0;){
			Forbid();
			port = FindPort(CLIENT_COMMAND_PORT);
			Permit();

			if(port){
				break;
			}
			Delay(TICKS_PER_SECOND);
		}
	}

	return port;
}

struct MsgPort *FindServer(int start_if_not_found)
{
	return NULL;
}

void client(struct Message *msg)
{
	struct MsgPort *port;

	DB("client \n") ;
	port = FindClient(1);
	if(!port){
		fprintf(stderr, "nfsmgr: could not start client\n");
		cleanup(1);		
	}
	msg->mn_ReplyPort = myreplyport;

	/*
	 * We FindPort() here again to avoid race condition where client
	 * elects to die at the same time we're sending a message.
	 */
	Forbid();
	port = FindPort(CLIENT_COMMAND_PORT);
	if(!port){
		fprintf(stderr, "nfsmgr: client NFS died\n");
		Permit();
		cleanup(1);
	}
	PutMsg(port, msg);
	Permit();

	if(WaitPort(myreplyport)){
		GetMsg(myreplyport);
	}
}

int nfs_error(struct nfs_req *msg)
{
	static char *ops[] = {"Unmount", "Mount", "Statistics", "Refresh"};
	static struct nfs_errors {
		char	num;
		char	*msg;
	} nfs_errs[] = {
		{0,	"NFS operation completed properly"},
		{1,	"Permission denied"},
		{2,	"No such file or directory"},
		{5,	"I/O error on server"},
		{6,	"No such device"},
		{13,	"Access denied"},
		{17,	"File/directory exists"},
		{19,	"No device"},
		{20,	"Operation requires a directory"},
		{21,	"Non directory operation on directory"},
		{27,	"Operation too large"},
		{28,	"No space on server"},
		{30,	"Attempted write on read only device"},
		{63,	"Name is too long"},
		{66,	"Directory not empty"},
		{69,	"Exceeded quota"},
		{70,	"Stale filehandle"},
		{99,	"Error when flushing buffers"},
		{-1,	""}
	};

	DB("nfs_error \n") ;

	if(!(msg->error || msg->nfs_error || msg->rpc_error.re_status)){
		return 0;
	}
	fprintf(stderr, "nfsmgr: %s fail: ", ops[msg->opcode]);
	switch(msg->opcode){
	case NFSC_MOUNT:
		fprintf(stderr, "(volume %s)", 
				((struct nfs_mount_req *)msg)->nm_volume);
		break;

	case NFSC_UMOUNT:
		fprintf(stderr, "(volume %s)", 
				((struct nfs_umount_req *)msg)->nu_volume);
		break;

	case NFSC_GETSTAT:
		fprintf(stderr, "(volume %s)", 
				((struct nfs_stats_req *)msg)->ns_volume);
	}
	if(msg->nfs_error != NFS_OK){
		int i;

		for(i = 0; nfs_errs[i].num >= 0; i++){
			if(nfs_errs[i].num == msg->nfs_error){
				fprintf(stderr, " NFS: %s; ", nfs_errs[i].msg);
				break;
			}
		}
		if(nfs_errs[i].num < 0){
			fprintf(stderr, " Unknown NFS error code %d", msg->nfs_error);
		}
	}

	if(msg->rpc_error.re_status != RPC_SUCCESS){
		fprintf(stderr, " %s;", clnt_sperrno(msg->rpc_error.re_status));
	}

	if(msg->error != 0){
		if(msg->error < 0 || msg->error > sys_nerr){
			fprintf(stderr, " SYSTEM: Error %d;", msg->error);
		} else {
			fprintf(stderr, " SYSTEM: %s;", sys_errlist[msg->error]);
		}
	}
	fprintf(stderr, "\n");

	if(msg->error == ECONFIGPROBLEM){
		cleanup(1);
	}

	return -1;
}

int myungetc(char c)
{
	if(c == '\n'){
		line--;
	}

	if(cmdline[0]){
		if(--pos < 0) {
			pos = 0;
			return -1;
		}
	} else {
		ungetc(c, infile);
	}

	return 0;
}

char mygetchar()
{
	char c;

	if(cmdline[0]){
		c = cmdline[pos]==0 ? EOF:cmdline[pos++];
	} else {
		c = getc(infile);
	}

	if(c == '\n'){
		line++;
	}

	return c;
}

/*
 * usage:
 *		nfsmgr -i input_file
 *		nfsmgr [nfs mgt grammar]
 */
main(int argc, char **argv)
{
	DB("  main \n") ;
	if(argc == 1){
		fprintf(stderr, "Usage: nfsmgr command | -i filename\n");
		exit(1);
	}

	DB("main 2\n") ;
	if ((SockBase = OpenLibrary( "inet:libs/socket.library", 1L ))==NULL) {
		PutStr("Opening of socket library failed.\n");
		exit(1);
	}
	
	DB("main 3\n") ;
	setup_sockets( 5, &errno );

	DB("main 4\n") ;
	myreplyport = CreateMsgPort();
	if(!myreplyport){
		fprintf(stderr, "nfsmgr: could not create reply port\n");
		cleanup(1);
	}

	DB("main 5\n") ;
	cmdline[0] = 0;

	DB("main 6\n") ;
	argc--; argv++;
	if(argc == 2 && stricmp(argv[0], "-i")==0){
		DB("main 6a\n") ;
		infile = fopen(argv[1], "r");
		if(!infile){
			perror(argv[1]);
			cleanup(1);
		}
	} else {
		DB("main 6b\n") ;
		while(argc-- > 0){
			strcat(cmdline, *argv++);
			strcat(cmdline, argc > 0 ? " ":"\n");
		}
	}
	DB("main 7\n") ;
	yyparse();

	DB("main 8 (last)\n") ;
	cleanup(0);
}

void cleanup( retval )
int retval;
{
	DB("cloeanup\n") ;
	if( myreplyport)
		DeleteMsgPort(myreplyport);

	cleanup_sockets();
	CloseLibrary(SockBase);
	exit(retval);
}	

void refreshclient()
{
	struct nfs_req req;

	DB("refresh \n") ;
	bzero(&req, sizeof(req));
	req.opcode = NFSC_REFRESH;
	client((struct Message *)&req);
	nfs_error(&req);
}

void serverstats(char *volume)
{
	fprintf(stderr, "nfsmgr: NFS server statistics not implemented\n");
}

void clientstats(char *volume)
{
	struct nfs_stats_req req;

	bzero(&req, sizeof(req));
	req.ns_msg.opcode = NFSC_GETSTAT;
	strcpy(req.ns_volume, volume);
	client((struct Message *)&req);
	if(nfs_error((struct nfs_req *)&req) < 0){
		return;
	}

	printstats(&req);
}

#define	NPL	6		/* Stats per line	*/

void printstats(struct nfs_stats_req *req)
{
	static char *nfs_ops[] = {
		"Nop",		"Getattr",	"Setattr",	"Root",
		"Lookup",	"Rdlink",	"Read",		"Writecache",
		"Write",	"Create",	"Remove",	"Rename",
		"Link",		"Symlink",	"Mkdir",	"Rmdir",
		"Readdir",	"Statfs",	0
	};
	char *afmt = "\t%-10s %-10s %-10s %-10s %-10s %-10s\n\t%-10d %-10d %-10d %-10d %-10d %-10d\n";
	int i, j;

	printf("\nNFS usage statistics for volume \"%s\":\n\n", 
				req->ns_volume);
	for(i = NFSPROC_NULL; i <= NFSPROC_STATFS; i += NPL){
		printf("\t");
		for(j = i; j <= NFSPROC_STATFS && j < i+NPL; j++){
			printf("%-10s ", nfs_ops[j]);
		}
		printf("\n\t");
		for(j = i; j <= NFSPROC_STATFS && j < i+NPL; j++){
			printf("%-10d ", req->ns_nstat.ns_nstat[j]);
		}
		printf("\n");
	}

	printf("\nAmiga usage statistics for volume \"%s\":\n\n", 
				req->ns_volume);
	printf(afmt, 
		"Open R/W", "Open New", "Open Old", "Close", "Seek", "Read",
			req->ns_astat.action_rw, 
			req->ns_astat.mode_newfile, 
			req->ns_astat.mode_oldfile, 
			req->ns_astat.action_end,
			req->ns_astat.action_seek, 
			req->ns_astat.action_read);
		
	printf(afmt, 
		"Write", "Info", "DiskInfo", "Locate", "Rename", "FreeLock",
			req->ns_astat.action_write, 
			req->ns_astat.action_info,
			req->ns_astat.action_disk_info,
			req->ns_astat.action_locate_object, 
			req->ns_astat.action_rename_disk, 
			req->ns_astat.action_free_lock);

	printf(afmt, 
		"CopyDir", "SetProtect", "Chmod", "CreateDir", "ExNext", "ExamineNext", 
			req->ns_astat.action_copy_dir,
			req->ns_astat.action_set_protect, 
			req->ns_astat.action_chmod, 
			req->ns_astat.action_create_dir, 
			req->ns_astat.action_ex_next, 
			req->ns_astat.action_examine_next);

	printf(afmt, 
		"Examine", "Ex", "ParentDir", "Rename", "MoreCache", "Flush",
			req->ns_astat.action_examine_object, 
			req->ns_astat.action_ex_object, 
			req->ns_astat.action_parent,
			req->ns_astat.action_rename_object, 
			req->ns_astat.action_morecache, 
			req->ns_astat.action_flush);

	printf("\t%-10s %-10s %-10s %-10s %-10s\n\t%-10d %-10d %-10d %-10d %-10d\n", 
		"SetDate", "CurrentVol", "Delete", "RenameDisk", "Unknown",
			req->ns_astat.action_setdate,
			req->ns_astat.action_current_volume,
			req->ns_astat.action_delete_object,
			req->ns_astat.action_rename_disk,
			req->ns_astat.action_unknown);
}
