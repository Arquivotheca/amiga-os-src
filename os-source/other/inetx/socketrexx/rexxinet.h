/*
 * header for rexxinet
 */

#include <rexx/rxslib.h>
#include <rexx/storage.h>
#include <rexx/errors.h>

#include <libraries/dos.h>
#include <exec/memory.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netdb.h>

#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>

extern struct RxsLib *RexxSysBase;
extern void *OpenLibrary();
extern STRPTR *CreateArgstring();
extern char *inet_ntoa();
extern void *GetMsg();
extern struct MsgPort *CreatePort();
extern int Enable_Abort;
extern fd_set wait;
extern int controlsock;
extern void *AllocMem();

#define NUMPORTS 32		/* Max number of network ports open	*/
struct port {
	struct RexxMsg *rm;	/* message that was blocked		*/
	char *name;		/* name of command we are waiting on	*/
	char sock;		/* if socket, then this is socket fd	*/
	char cmd;		/* command table entry we wait on	*/
	char inuse;		/* this port is allocated		*/
};
extern struct port ports[];
extern struct port *newport(), *getport();

#ifndef min
#define min(a, b) ((a) < (b) ? (a):(b))
#endif

#ifndef max
#define max(a, b) ((a) > (b) ? (a):(b))
#endif

#define REPLY	-1		/* reply to message			*/
#define HOLD(s)	(s)		/* block this message until later	*/
#define FIRST	1		/* first time command has been run	*/
#define RERUN	2		/* command being rerun after block	*/

