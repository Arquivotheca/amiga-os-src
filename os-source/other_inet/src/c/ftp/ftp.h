/* AMIGA stuff */
#define UNIX_COMPAT 1
#include <ss/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <exec/exec.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
extern struct Library *DOSBase;
extern struct ExecBase *SysBase;

#define index(a,b)	strchr(a,b)
#define rindex(a,b)	strrchr(a,b)
#define strcasecmp(a,b)	stricmp(a,b)
#define strncasecmp(a,b,n)	strnicmp(a,b,n)

/* unix emulation functions */
void syslog(int pri, ...);
void sleep(int);
int stat(char *, struct stat *buf);
int fstat(int, struct stat *);
void clean_sock(void);
int init_sock(void);

typedef void (*sig_t)();
void Dprintf(char *, ...);

/* ftp.c */
void pswitch(int flag);

/* cmds.c */
void setpeer(int argc, char *argv[]);

/* glob.c */
void blkfree(char **av0);

#ifndef FTP_VAR_H
extern int	trace;			/* trace packets exchanged */
extern int	hash;			/* print # for each buffer transferred */
extern int	sendport;		/* use PORT cmd for each data connection */
extern int	verbose;		/* print messages coming back from server */
extern int	connected;		/* connected to server */
extern int	fromatty;		/* input is from a terminal */
extern int	interactive;		/* interactively prompt on m* cmds */
extern int	debug;			/* debugging level */
extern int	bell;			/* ring bell on cmd completion */
extern int	doglob;			/* glob local file names */
extern int	autologin;		/* establish user account on connection */
extern int	proxy;			/* proxy server connection active */
extern int	proxflag;		/* proxy connection exists */
extern int	sunique;		/* store files on server with unique name */
extern int	runique;		/* store local files with unique name */
extern int	mcase;			/* map upper to lower case for mget names */
extern int	ntflag;			/* use ntin ntout tables for name translation */
extern int	mapflag;		/* use mapin mapout templates on file names */
extern int	code;			/* return/reply code for ftp command */
extern int	crflag;			/* if 1, strip car. rets. on ascii gets */
extern char	pasv[64];		/* passive port for proxy data connection */
extern char	*altarg;		/* argv[1] with no shell-like preprocessing  */
extern char	ntin[17];		/* input translation table */
extern char	ntout[17];		/* output translation table */
#include <sys/param.h>
extern char	mapin[MAXPATHLEN];	/* input map template */
extern char	mapout[MAXPATHLEN];	/* output map template */
extern char	typename[32];		/* name of file transfer type */
extern int	type;			/* requested file transfer type */
extern int	curtype;		/* current file transfer type */
extern char	structname[32];		/* name of file transfer structure */
extern int	stru;			/* file transfer structure */
extern char	formname[32];		/* name of file transfer format */
extern int	form;			/* file transfer format */
extern char	modename[32];		/* name of file transfer mode */
extern int	mode;			/* file transfer mode */
extern char	bytename[32];		/* local byte size in ascii */
extern int	bytesize;		/* local byte size in binary */

extern char	*hostname;		/* name of host connected to */
extern int	unix_server;		/* server is unix, can use binary for ascii */
extern int	unix_proxy;		/* proxy is unix, can use binary for ascii */

extern struct	servent *sp;		/* service spec for tcp/ftp */

#include <setjmp.h>
extern jmp_buf	toplevel;		/* non-local goto stuff for cmd scanner */

extern char	line[200];		/* input line buffer */
extern char	*stringbase;		/* current scan point in line buffer */
extern char	argbuf[200];		/* argument storage buffer */
extern char	*argbase;		/* current storage point in arg buffer */
extern int	margc;			/* count of arguments on input line */
extern char	*margv[20];		/* args parsed from input line */
extern int     cpend;                  /* flag: if != 0, then pending server reply */
extern int	mflag;			/* flag: if != 0, then active multi command */

extern int	options;		/* used during socket creation */

/*
 * Format of command table.
 */
extern struct cmd {
	char	*c_name;	/* name of command */
	char	*c_help;	/* help string */
	char	c_bell;		/* give bell when command completes */
	char	c_conn;		/* must be connected to use command */
	char	c_proxy;	/* proxy server may execute */
	int	(*c_handler)();	/* function to call */
};

extern struct macel {
	char mac_name[9];	/* macro name */
	char *mac_start;	/* start of macro in macbuf */
	char *mac_end;		/* end of macro in macbuf */
};

extern int macnum;			/* number of defined macros */
extern struct macel macros[16];
extern char macbuf[4096];

#endif /* FTP_VAR_H */ 
