/* -----------------------------------------------------------------------
 * rshd.c        New version for socket.library 4.0 and later.
 *
 * $Locker:  $
 *
 * $Id: rshd.c,v 1.12 93/03/04 11:37:55 bj Exp $
 *
 * $Revision: 1.12 $
 *
 * $Log:	rshd.c,v $
 * Revision 1.12  93/03/04  11:37:55  bj
 * Fixed problem where SAS's global optimizer would create
 * "enforcer hit code" (see comment in source.)  sheesh.
 * 
 * Revision 1.11  93/02/24  16:13:37  bj
 * Fixed the problem where no matter what errors occurred, the
 * response was "error code not set". This was because the initial
 * "null send()" on the socket back to the caller was located
 * past the point in the code where most such errors occurred.
 * 
 * Also changed the remaining Execute() call (used for rcp) to
 * SystemTagList(). NO real value one way or the other, actually.
 * 
 * Revision 1.10  93/02/05  15:53:17  bj
 * Add NP_WindowPtr tag to CreateNewProc() of the child process.
 * This eliminates the problem of remote commands bringing up
 * requesters (on the remote machine) if the user mistypes or
 * otherwise asks for a non-mounted volume.
 * 
 * Revision 1.9  92/10/22  13:52:30  bj
 * Added Syslog reporting of logins. syslog priority "LOG_INFO" or >
 * 
 * Revision 1.8  92/10/16  14:58:56  bj
 * 
 * Binary 1.8
 * o added extra syslog messages for better error reporting.
 * 
 * Revision 1.7  92/09/28  17:35:13  bj
 * Binary 1.7
 * 
 * 1. Fixed Readargs bug. Now checks for an error return.
 * 2. Changed Readargs template so that the "/K" options are now "/S"
 *    This was causing readargs to fail. Since Martin wasn't checking
 *    for a bad return, the arg checking continued but was usiing a 
 *    bad readargs handle. Mungwall!
 * 3. added better error checking and error messages.
 * 
 * Revision 1.6  92/09/10  16:41:40  bj
 * Removed an errant kprintf().
 * 
 * Revision 1.5  92/09/10  16:20:50  bj
 * Added code to ensure that the child process's stack is at least
 * what the user has specified with the 'stack' command.
 * R2 AS225.
 * 
 * Revision 1.4  92/09/09  17:47:51  bj
 * Fixed the problem where rshd would hang if the amount of data
 * it was asked to send was larger than the pipe: buffer. Now uses
 * a child process to write to the pipe while the parent reads from
 * it.
 * 
 * Details of no consequence (better comments, etc.)
 * 
 * Revision 1.3  92/08/07  16:23:33  bj
 * Rev 1.3 of rshd (AS225 R2)
 * Adds code to handle the search for "/.rhosts" file. Needed
 *   to handle the ":" and "/" characters when "/.rhosts" was
 * appended to the user's homedirectory as defined in the 
 * inet:db/passwd file.  Old version could get things like:
 * 
 *   Server:/.rhosts    or    Server:bin//.rhosts
 * 
 * 
 * Revision 1.2  92/07/10  17:27:56  bj
 * 1. Added pr_WindowPtr code to stop remote requesters.
 * 2. Requires socket.library v4.0 or later.
 * 3. Added RRCS header.
 * 
 *
 * $Header: AS225:src/serv/rshd/RCS/rshd.c,v 1.12 93/03/04 11:37:55 bj Exp $
 *
 *------------------------------------------------------------------------
 */


// #define DEBUG 1
#ifdef DEBUG
kprintf(char *, ... ) ;
#define D(x) kprintf(x)
#else
#define D(x) ;
#endif

#define FD_SETSIZE	32
#include <exec/types.h>
#include <ss/socket.h>
#include <exec/memory.h>
#include <exec/tasks.h>
#include <dos/dos.h>
#include <dos/dostags.h>
#include <dos/dosextens.h>
#include <devices/timer.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/utility_protos.h>
#include <clib/alib_protos.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/utility_pragmas.h>
/* #include <pragmas/alib_exec_pragmas.h> */
#include <sys/ioctl.h>
#include <sys/syslog.h>
#include <sys/time.h>
#include <sys/param.h>
#include <netdb.h>
#include <pwd.h>
#include <stdarg.h>
#include <string.h>
#include <utility/date.h>

#include "rshd_rev.h"

/* This is used with the templates to cause the version string to be     */
/* stored in the command.  It depends upon the command having a #include */
/* of the appropriate version file to provide the value for this string  */
#define CMDREV  "\0$VER: " VSTRING

#define bzero(a,n)	memset(a,'\0',n)

#define TEMPLATE    "NAME/A,SOCKPTR/A/N,ID/A/N,-L/S,-N/S,-D/S" CMDREV
#define OPT_NAME    0
#define	OPT_SOCKPTR	1
#define OPT_ID		2
#define OPT_L		3
#define OPT_N		4
#define OPT_D		5
#define OPT_COUNT   6

#define TEMPLATE_CHILD "CPN/A,PN/A,PA/A/F"  /* childport_name,pipename,pipeargs */
#define PORT_NAME        0
#define PIPE_NAME        1
#define PIPE_ARGS        2
#define CHILD_ARGS_COUNT 3

#define CHILD_PORT_NAME_BASE "rshdport"

long childproc(void) ;

	/* NP_Arguments -must- be tags[2] !!! */
struct TagItem tags[] = {
	{ NP_Entry,(LONG) childproc },
	{ NP_Name,(LONG) "rshdchild" },
	{ NP_Arguments, NULL },
	{ NP_StackSize,20000L }, 
	{ NP_ConsoleTask, 0L },
	{ NP_Cli,TRUE },
	{ NP_WindowPtr, -1L},
	TAG_DONE,
} ;

char *months[] = { "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec" } ;

void sprintf(char *, ...) ;
void Dprintf(char *fmt, ...) ;
void syslog(int pri, ...) ;
void main(void) ;
int local_domain(char *h) ;
void getstr(char *buf, int cnt, char *err) ;
void error(char *) ;
int rresvport(int *alport) ;
int ruserok( char *rhost, int superuser, char *ruser, char *luser) ;
static int _validuser( BPTR hostf, char *rhost, char *luser, char *ruser,int baselen) ;
static int _checkhost( char *rhost, char *lhost, int len) ;

int	errno ;
int	keepalive = 1 ;
BPTR debugfh=0 ;
struct Library *SockBase ;
struct Library *DOSBase ;
struct Library *UtilityBase ;
int s ;
int _check_rhosts_file=1 ;
int Debug_Flag=0 ;
struct sockaddr_in from ;


void main()
{
	struct linger linger ;
	int on = 1 ;
	int fromlen ;
	long opts[OPT_COUNT] ;
	struct RDargs *rdargs ;
	long sockptr ;
	char cmdbuf[256] ;
	char remotehost[2 * MAXHOSTNAMELEN + 1] ;
	char locuser[16] ;
	char remuser[16] ;
	char pipename[32] ;
	char childportname[32] ;
	BYTE pbuffer[512] ;
	BYTE buffer[512] ;
	struct passwd *pwd ;
	struct hostent *hp ;
	BPTR fh, lock, oldlock ;
	int cc ; 
	int s2 ;
	char *hostname ;
	short secport ;
	u_long rd ;
	struct timeval time ;
	struct Process *myself ;
	APTR oldwindowptr ;
	struct Library *SysBase= (*((struct Library **) 4)) ;
    struct MsgPort *parentport, *childport ;
    struct Message msg ;
	long cnt, count ;
    struct Process *proc ;
	struct CommandLineInterface *cli ;
    struct TagItem *ti ;
    struct DateStamp datestamp ;
    struct ClockData clockdata ;
    ULONG hours, mins ;
    long stacksize ;
    long system_error ;

	myself = (struct Process *)FindTask(NULL) ;
	oldwindowptr = myself->pr_WindowPtr ;
	cli = (struct CommandLineInterface *)Cli() ;
	stacksize = cli ? (cli->cli_DefaultStack << 2 ) : 20000L ;

	if(UtilityBase = OpenLibrary("utility.library",36L))
	{
		if( ti = FindTagItem(NP_StackSize, tags ))
		{
			ti->ti_Data = stacksize ;
		}
		else
		{
			Debug_Flag=1 ;
			Dprintf("Cannot find tagitem 'NP_StackSize.'\n") ;
			Close(debugfh) ;
			myself->pr_WindowPtr = oldwindowptr ;
			CloseLibrary(UtilityBase) ;
			Exit(10) ;
		}
		DateStamp(&datestamp) ;
		Amiga2Date( (ULONG)(datestamp.ds_Days * ((24 * 60) * 60 )), &clockdata ) ;
		CloseLibrary(UtilityBase) ;
	}
	else
	{
		Debug_Flag=1 ;
		Dprintf("Can't open utility.library\n") ;
		Close(debugfh) ;
		myself->pr_WindowPtr = oldwindowptr ;
		Exit(10) ;
	}
	
	memset((char *)opts, 0, sizeof(opts)) ;

	if(rdargs = ReadArgs(TEMPLATE, opts, NULL))
	{
		if(opts[OPT_SOCKPTR]) 
		{
			sockptr=*(long *)opts[OPT_SOCKPTR] ;
		} 
		else 
		{
			Debug_Flag=1 ;
			Dprintf("received NULL socket pointer\n") ;
			FreeArgs(rdargs) ;
			Close(debugfh) ;
			myself->pr_WindowPtr = oldwindowptr ;
			Exit(10) ;
		}

		if(opts[OPT_ID]) 
		{
			if(*(long *)opts[OPT_ID] != 0) 
			{
				Debug_Flag=1 ;
				Dprintf("ID is not 0\n") ;
				FreeArgs(rdargs) ;
				Close(debugfh) ;
				myself->pr_WindowPtr = oldwindowptr ;
				Exit(10) ;
			}
		}
		if(opts[OPT_D])
		{
			Debug_Flag=1 ;
		}

		if(opts[OPT_L]) 
		{
			Dprintf("rhosts checking turned off\n"); 
			_check_rhosts_file = 0 ;
		}

		if(opts[OPT_N]) 
		{
			Dprintf("Keepalive turned off\n") ;
			keepalive = 0 ;
		}

		FreeArgs(rdargs) ;
	}
	else
	{
		Fault( (long)IoErr(), "rshd - readargs :", buffer, 400L) ;
		*(buffer + 200) = '\0' ;
		Debug_Flag=1 ;
		Dprintf("rdargs failed: buffer\n") ;
		myself->pr_WindowPtr = oldwindowptr ;
		Close(debugfh) ;
		Exit(10) ;
	}


	if ((SockBase = OpenLibrary( "inet:libs/socket.library", 4L ))==NULL) 
	{
		Debug_Flag=1 ;
		myself->pr_WindowPtr = oldwindowptr ;
		Dprintf("Error opening socket library. Need version >= 4.0.\n") ;
		Close(debugfh) ;
		Exit(10) ;
	}
	setup_sockets(5,&errno) ;


	s = s_inherit((void *)sockptr) ;

	fromlen = sizeof (from) ;
	if (getpeername(s, (struct sockaddr *)&from, &fromlen) < 0) 
	{
		syslog(LOG_ERR,"RSHD: getpeername: %s\n", strerror(errno)) ;
		if(debugfh)
		{
			Close(debugfh) ;
		}
		cleanup_sockets() ;
		CloseLibrary(SockBase) ;
		Exit(10) ;
	}
	if (keepalive && setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, (char *)&on,sizeof(on)) < 0)
    {
		syslog(LOG_WARNING, "RSHD: setsockopt (SO_KEEPALIVE): %s\n",strerror(errno)) ;
	}


	linger.l_onoff = 1 ;
	linger.l_linger = 60 ;
	if (setsockopt(s, SOL_SOCKET, SO_LINGER, (char *)&linger,sizeof (linger)) < 0)
	{
		syslog(LOG_WARNING, "RSHD: setsockopt (SO_LINGER): %s\n",strerror(errno)) ;
	}

	from.sin_port = ntohs((u_short)from.sin_port) ;
	if (from.sin_family != AF_INET) 
	{
		syslog(LOG_ERR,"RSHD: malformed from address\n") ;
		goto exit ;
	}

	if (from.sin_port >= IPPORT_RESERVED || from.sin_port < IPPORT_RESERVED/2) 
	{
		syslog(LOG_NOTICE, "RSHD: Connection from %s on illegal port\n",
			inet_ntoa(from.sin_addr.s_addr)) ;
		goto exit ;
	}


	secport = 0 ;
	time.tv_sec = 10 ;
	time.tv_usec = 0 ;

	while(1) 
	{
		char c ;
		int n ;
		rd = 1 << s ;
		n = select(s+1,(fd_set *)&rd,0,0,&time) ;
		if(n && rd&(1<<s) ) 
		{
			if ((cc = recv(s, &c, 1, 0)) != 1) 
			{
				if (cc < 0)
				{
					syslog(LOG_NOTICE, "RSHD: read: %s\n",strerror(errno)) ;
				}
				shutdown(s, 1+1) ;
				goto exit ;
			}
			if (c == 0) {
				break ;
			}
			secport = secport * 10 + c - '0' ;
		}
		if(n==0) /* timed out */
			break ;
	}

	Dprintf("received secondary port number %ld\n",secport) ;


	if (secport != 0) 
	{
		int lport = IPPORT_RESERVED - 1 ;
		s2 = rresvport(&lport) ;
		if (s2 < 0) 
		{
			syslog(LOG_ERR, "can't get stderr port: %s\n",strerror(errno)) ;
			goto exit ;
		}
		if (secport >= IPPORT_RESERVED) 
		{
			syslog(LOG_ERR, "2nd port not reserved\n") ;
			goto exit ;
		}
		from.sin_port = htons((u_short)secport) ;
		if (connect(s2, (struct sockaddr *)&from, sizeof (from)) < 0) 
		{
			syslog(LOG_INFO, "connect second port: %s\n",strerror(errno)) ;
			goto exit ;
		}
	}

	hp = gethostbyaddr((char *)&(from.sin_addr), sizeof(struct in_addr),from.sin_family) ;
	if (hp) 
	{
		/*
		 * If name returned by gethostbyaddr is in our domain,
		 * attempt to verify that we haven't been fooled by someone
		 * in a remote net; look up the name and check that this
		 * address corresponds to the name.
		 */
		if (local_domain(hp->h_name)) 
		{
			strncpy(remotehost, hp->h_name, sizeof(remotehost) - 1) ;
			remotehost[sizeof(remotehost) - 1] = 0 ;
			hp = gethostbyname(remotehost) ;
			if (hp == NULL) 
			{
				syslog(LOG_INFO,"Couldn't look up address for %s\n",remotehost) ;
				error("Couldn't look up address for your host") ;
				goto exit ;
			} 
			else 
			{
				for (; ; hp->h_addr_list++) 
				{
					if (!bcmp(hp->h_addr_list[0],(caddr_t)&from.sin_addr,sizeof(from.sin_addr)))
					{
						break ;
					}
					if (hp->h_addr_list[0] == NULL) 
					{
						syslog(LOG_NOTICE,
						  "Host addr %s not listed for host %s\n",
						    inet_ntoa(from.sin_addr.s_addr),
						    hp->h_name) ;
						error("Host address mismatch") ;
						goto exit ;
					}
				}
			}
		}
		hostname = hp->h_name ;
	} 
	else 
	{
		hostname = inet_ntoa(from.sin_addr.s_addr) ;
	}

	
	Dprintf("connecting to %s\n",hostname) ;
	hours = datestamp.ds_Minute / 60 ;
	mins  = datestamp.ds_Minute - (60 * hours) ;
	

	getstr(remuser, sizeof(remuser), "remuser") ;
	getstr(locuser, sizeof(locuser), "locuser") ;
	getstr(cmdbuf, sizeof(cmdbuf), "command") ;

	Dprintf("remoteuser=%s localuser=%s\n",remuser,locuser) ;
	sprintf(pbuffer, "%s (%s) at %02ld:%02ld %ld-%s",(char *)inet_ntoa(from.sin_addr.s_addr), 
					hostname,hours,mins, (long)clockdata.mday, months[clockdata.month-1]) ;
	syslog(LOG_INFO,"RSHD: connect %s %s.\n\t\t\"%s\".\n",remuser, pbuffer,cmdbuf) ;
	syslog(LOG_INFO,"       \"%s\".\n", cmdbuf) ;

	(void) send(s, "", 1, 0) ;
	setpwent(1) ;
	pwd = getpwnam(locuser) ;
	if (pwd == NULL) 
	{
		Dprintf("login incorrect for locuser = %s\n", locuser) ;
		error("Login incorrect.\n") ;
		goto exit ;
	}
	endpwent() ;

	lock = Lock(pwd->pw_dir,ACCESS_READ) ;
	if( lock==0 ) {
		lock = Lock(":",ACCESS_READ) ;
	}
	oldlock = CurrentDir(lock) ;

	if (pwd->pw_passwd != 0 && *pwd->pw_passwd != '\0' &&
	            ruserok(hostname, pwd->pw_uid == 0, remuser, locuser) < 0) 
    {
		error("Permission denied.\n") ;
		goto exit ;
	}

	/* at this point, we have a socket (s) and possibly a  */
	/* secondary socket (s2).  We need to execute "cmdbuf" */
	/* and use s for input and output to that process      */
	/* s2 can be used for stderr, whatever that means on an Amiga */

	/* Matt Dillon's fifo.library looks like it would fit in here   */
	/* very nicely.  Remote shells on Amigas are cool, but I really */
	/* don't have the time to get it working right, so I'll punt    */
	/* for now.                                                     */

	/* special hack to get rcp working */
	/* this wouldn't be necessary if we had real stdin/stdout functionality */

	if(!strncmp(cmdbuf,"rcp ",4)) 
	{
		strcpy(buffer,"run >nil: ") ;
		strcat(buffer,cmdbuf);
		sprintf(cmdbuf," %ld",s_release(s)) ;
		strcat(buffer,cmdbuf) ;
		Dprintf("rcp: buffer = '%s'\n", buffer) ;
		system_error = SystemTagList(buffer,NULL) ;
		Dprintf("rshd: rcp: syste7m_error = %ld\n", system_error) ;
		goto exit ;
	}

	if( parentport = CreateMsgPort() )
	{
		sprintf(childportname,"%s%ld",CHILD_PORT_NAME_BASE,(LONG)myself) ;
		sprintf(pipename, "pipe:%ld", (LONG)myself) ;
		sprintf(buffer,"%s %s %s\n",childportname,pipename,cmdbuf) ;
		tags[2].ti_Data = (LONG)buffer ;
		
		if((proc = CreateNewProc(tags)) == NULL)
		{
			DeleteMsgPort(parentport) ;
			goto exit ;
		}
		else
		{
			count = 50 ;  /* max wait approx 2 seconds (50 * delay(2)) */
			do 
			{
				if(childport = FindPort(childportname))
				{
					break ;
				}
				Delay(2L) ;
			}
			while( count-- ) ;
		}

		if( childport )
		{
			msg.mn_ReplyPort = parentport ;
			msg.mn_Length    = (UWORD)sizeof(struct Message) ;
			PutMsg( childport, &msg ) ;
			WaitPort(parentport) ;
			while( GetMsg(parentport )) {} ;
		}
		else
		{
			error( "error on remote machine.\n") ;
		}
		DeleteMsgPort(parentport ) ;

		/** reading from pipe: **/

		if(fh = Open(pipename , MODE_OLDFILE))
		{
			do 
			{
				cnt = Read(fh,pbuffer,sizeof(pbuffer)) ;
				if( cnt > 0L )
				{
					send(s,pbuffer,cnt,0) ;
				}
			} 
			while(cnt > 0L) ;

			Close(fh) ;
		}
		else
		{
			error( "error on remote machine.\n") ;
		}
	}

///////////////////////////////////////////////////////////////////////////
D("This spot can cause enforcer hits if the SAS 5.10 optimizer is used.\n") ;
D("Test and be sure!\n") ;
///////////////////////////////////////////////////////////////////////////

exit:
	myself->pr_WindowPtr = oldwindowptr ;
	(void)CurrentDir(oldlock) ;
	UnLock(lock) ;
	if(debugfh)
	{
		Close(debugfh) ;
	}		
	cleanup_sockets() ;
	CloseLibrary(SockBase) ;
}


/******************************************************
 * Output debug into to a window. Call only from parent!
 *******************************************************
 */

void Dprintf(char *fmt, ...)
{

    va_list args ;

	if(Debug_Flag) 
	{
	    if (debugfh==NULL) 
	    {
			debugfh = Open("con:0/0/640/100/RSHD/AUTO/WAIT", MODE_NEWFILE) ;
	    }
	    va_start(args,fmt) ;
		VFPrintf(debugfh,fmt,(LONG *)args) ;
	    va_end(args) ;
	}
}

/*******************************************************
 * send error message to caller
 *******************************************************
 */

void error(char *str)
{
	(void)send(s, str, strlen(str), 0) ;
}

/******************************************************
 * extract a string from the caller's stream
 ******************************************************
 */


void getstr(char *buf, int cnt, char *err)
{
	char c ;
	struct Library *SysBase= (*((struct Library **) 4)) ; 

	do {
		if (recv(s, &c, 1, 0) != 1)
		{
			goto failed ;
		}
		*buf++ = c ;
		if (--cnt == 0) 
		{
			error(err) ;
			error("too long\n") ;
			goto failed ;
		}
	} while (c != 0) ;

	return ;

failed:
	if(debugfh) 
	{
		Close(debugfh) ;
	}
	cleanup_sockets() ;
	CloseLibrary(SockBase) ;
	Exit(10) ;

}

/****************************************************
 * Check whether host h is in our local domain,
 * as determined by the part of the name following
 * the first '.' in its name and in ours.
 * If either name is unqualified (contains no '.'),
 * assume that the host is local, as it will be
 * interpreted as such.
 ****************************************************
 */
 
int local_domain(char *h)
{
	char localhost[MAXHOSTNAMELEN] ;
	char *p1, *p2 = strchr(h, '.') ;

	(void) gethostname(localhost, sizeof(localhost)) ;
	p1 = strchr(localhost, '.') ;
	if (p1 == NULL || p2 == NULL || !stricmp(p1, p2))
	{
		return(1) ;
	}
	return(0) ;
}

int rresvport(int *alport)
{
	struct sockaddr_in sin ;
	int s ;

    /* first get a TCP socket */
	sin.sin_family = AF_INET ;
	sin.sin_addr.s_addr = INADDR_ANY ;
	s = socket(AF_INET, SOCK_STREAM, 0) ;
	if (s < 0)
	{
		return (-1) ;
	}


	/* now bind it to a reserved port */
	for (;;) {
		/* first try the port we asked for... */
		sin.sin_port = htons((u_short)*alport) ;
		if (bind(s, (struct sockaddr *)&sin, sizeof (sin)) >= 0)
		{
			return (s) ;
		}

		/* check to see if we had a real error */
		if (errno != EADDRINUSE) 
		{
			s_close(s) ;
			return (-1) ;
		}

		/* lets try a smaller port :^) */
		(*alport)-- ;
		if (*alport == IPPORT_RESERVED/2) 
		{
			s_close(s) ;
			errno = EAGAIN ;
			return (-1) ;
		}
	}
}


/****** socket/ruserok  *************************************************
*
*	NAME
*		ruserok - authenticate clients requesting service with rcmd.
*
*	SYNOPSIS
*		valid = ruserok( rhost, superuser, ruser, luser )
*
*		int = ruser( char *, int, char *, char * )
*
*	FUNCTION
*
*		The ruserok subroutine takes a remote host's name, as          
*		returned by a gethostent(3n) routine, two user names and a     
*		flag indicating if the local user's name is the superuser.     
*		It then checks the files /etc/hosts.equiv and .rhosts in the   
*		user's home directory to see if the request for service is     
*		allowed.  A 1 is returned if the machine name is listed in     
*		the hosts.equiv file, or the host and remote user name are     
*		found in the .rhosts file. Otherwise ruserok returns -1. If  
*		the superuser flag is 1, the checking of the hosts.equiv       
*		file is bypassed.                                              
*
*	INPUT
*
*		rhost     - The desired remote hosts name
*		superuser - Flag telling if local user's name is superuser.
*		ruser     - The remote user's name.
*		luser     - The local user's name.
*
*	RESULT
*		valid - (-1) on error or invalid user. Zero (0) upon success.
*
*
*
*	SEE ALSO
*
*   
********************************************************************
*
*
*/

int *isupper(char) ;
char tolower(char) ;

int
ruserok( char *rhost, int superuser, char *ruser, char *luser)
{
	BPTR hostf ;
	char fhost[MAXHOSTNAMELEN] ;
	int first = 1 ;
	register char *sp, *p ;
	char p2 ;
	int baselen = -1 ;

	sp = rhost ;
	p = fhost ;
	while (*sp) 
	{
		if (*sp == '.') 
		{
			if (baselen == -1)
			{
				baselen = sp - rhost ;
			}
			*p++ = *sp++ ;
		} 
		else 
		{
			*p++ = isupper(*sp) ? tolower(*sp++) : *sp++ ;
		}
	}
	*p = '\0' ;
	hostf = superuser ? 0 : Open(HOSTSEQUIV, MODE_OLDFILE) ;
again:
	if (hostf) 
	{
		if (!_validuser(hostf, fhost, luser, ruser, baselen)) 
		{
			Close(hostf) ;
			return(0) ;
		}
		Close(hostf) ;
	}
	if (first == 1 && (_check_rhosts_file || superuser)) 
	{
		struct passwd *pwd ;
		char pbuf[MAXPATHLEN] ;

		first = 0 ;
		if ((pwd = getpwnam(luser)) == NULL)
		{
			return(-1) ;
		}
		(void)strcpy(pbuf, pwd->pw_dir) ;
		p2 = *(pbuf + (strlen(pbuf)-1) ) ;
		if( p2 == ':' || p2 == '/' )
		{
			(void)strcat(pbuf, ".rhosts") ;
		}
		else
		{
			(void)strcat(pbuf, "/.rhosts") ;
		}
			
		if ((hostf = Open(pbuf, MODE_OLDFILE)) == NULL)
			return(-1) ;
		/*
		 * if owned by someone other than user or root or if
		 * writeable by anyone but the owner, quit
		 */
		 
		goto again ;
	}
	return (-1) ;
}

/* =========================================================
 * internal function 
 * =========================================================
 */


static int _validuser( BPTR hostf, char *rhost, char *luser, char *ruser,int baselen)
{
	char *user ;
	char ahost[MAXHOSTNAMELEN] ;
	register char *p ;

	while ( FGets(hostf, ahost, sizeof(ahost)) ) {
		p = ahost ;
		while (*p != '\n' && *p != ' ' && *p != '\t' && *p != '\0') 
		{
			*p = isupper(*p) ? tolower(*p) : *p ;
			p++ ;
		}
		if (*p == ' ' || *p == '\t') 
		{
			*p++ = '\0' ;
			while (*p == ' ' || *p == '\t')
				p++ ;
			user = p ;
			while (*p != '\n' && *p != ' ' && *p != '\t' && *p != '\0')
				p++ ;
		} 
		else
		{
			user = p ;
		}
		*p = '\0' ;
		if (_checkhost(rhost, ahost, baselen) &&
		    !strcmp(ruser, *user ? user : luser)) 
		{
			return (0) ;
		}
	}
	return (-1) ;
}

/* =========================================================
 * internal function 
 * =========================================================
 */


static int
_checkhost( char *rhost, char *lhost, int len)
{
	static char ldomain[MAXHOSTNAMELEN + 1] ;
	static char *domainp = NULL ;
	static int nodomain = 0 ;
	register char *cp ;

	if (len == -1)
		return(!strcmp(rhost, lhost)) ;
	if (strncmp(rhost, lhost, len))
		return(0) ;
	if (!strcmp(rhost, lhost))
		return(1) ;
	if (*(lhost + len) != '\0')
		return(0) ;
	if (nodomain)
		return(0) ;
	if (!domainp) 
	{
		if (gethostname(ldomain, sizeof(ldomain)) == -1) 
		{
			nodomain = 1 ;
			return(0) ;
		}
		ldomain[MAXHOSTNAMELEN] = NULL ;
		if ((domainp = strchr(ldomain, '.')) == (char *)NULL) 
		{
			nodomain = 1 ;
			return(0) ;
		}
		for (cp = ++domainp; *cp; ++cp)
		{
			if (isupper(*cp))
			{
				*cp = tolower(*cp) ;
			}
		}
	}
	return(!strcmp(domainp, rhost + len +1)) ;
}


/***********************************************************
 * childproc
 *
 * This code is spawned as a separate process to SystemTagList() 
 * the user's requested command and redirect the output to
 * the pipe: device. This is a separate process so that the
 * read and write operations with the pipe don't block each
 * other.
 *************************************************************
 */

LONG childproc(void)
{
	struct Library *SysBase= (*((struct Library **) 4)) ;
	struct DosLibrary *DOSBase ;
	struct MsgPort *fbport ;
	struct RDargs *ra ;
	struct Message *m ;
	BPTR fh ;
	LONG opts[CHILD_ARGS_COUNT] ;
	BYTE *pipename ;
	BYTE *pipeargs ;
	BYTE *cpname ;
	BYTE cbuffer[256] ;
	long count ;

	struct TagItem systags[] = {
	{ SYS_Output, 0L },
	{ TAG_END, 0L }
	} ;

	memset((char *)opts,0,sizeof(opts)) ;

	if(DOSBase = (struct Doslibrary *)OpenLibrary("dos.library",37))
	{
		if(ra = (struct RDargs *)ReadArgs(TEMPLATE_CHILD, opts, NULL))
		{
			if( opts[PORT_NAME])
			{
				cpname = (BYTE *)opts[PORT_NAME] ;
			}
			if(opts[PIPE_NAME])
			{
				pipename = (BYTE *)opts[PIPE_NAME] ;
			}
			if(opts[PIPE_ARGS])
			{
				pipeargs = (BYTE *)opts[PIPE_ARGS] ;
			}
			
			strcpy(cbuffer, pipeargs) ;

			if(fbport = CreatePort(cpname,1L))
			{
				count = 30 ;
				while( count-- )
				{
					if((m = GetMsg(fbport)) != NULL )
					{
						if(fh = Open( pipename, MODE_NEWFILE))
						{
							ReplyMsg( m ) ;
							systags[0].ti_Data = (LONG)fh ;
							SystemTagList( pipeargs, &systags[0] ) ;
							Close(fh) ;
							break ;
						}
					}
					else
					{
						Delay( 2L) ;
					}
				}
            	DeletePort(fbport) ;
			}
			FreeArgs(ra) ;
		}
		CloseLibrary((struct Library *)DOSBase) ;
	}
	return 123 ;
}

