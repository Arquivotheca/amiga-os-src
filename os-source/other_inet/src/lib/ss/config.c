/* -----------------------------------------------------------------------
 * config.c
 *
 * $Locker:  $
 *
 * $Id: config.c,v 1.6 93/11/08 14:59:31 kcd Exp $
 *
 * $Revision: 1.6 $
 *
 * $Log:	config.c,v $
 * Revision 1.6  93/11/08  14:59:31  kcd
 * Fixed length calculation on string copies.
 * 
 * Revision 1.5  93/03/24  15:13:49  kcd
 * Fixed two cases where strings wouldn't get properly terminated.
 *
 * Revision 1.4  92/08/21  19:15:55  kcd
 * Added extra configuration code for DNS.
 *
 * Revision 1.2  91/08/06  17:57:24  bj
 * Added reconfig() call.
 * Rewrote init_config() to support reconfig()
 *  (uses a boolean arg now.)
 * RCS header added.
 *
 *
 * $Header: AS225:src/lib/ss/RCS/config.c,v 1.6 93/11/08 14:59:31 kcd Exp $
 *
 *------------------------------------------------------------------------
 */
/* sslib - config.c     */

/* configuration stuff */
#include "sslib.h"
#include <exec/ports.h>
#include <exec/memory.h>
#include <sys/param.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <stdlib.h>
#include <string.h>


/*
 *************************************************************************
 *   format of config record.
 *
 *  struct config {
 *  	struct MsgPort mp;		/ * so we can hang on ports list	* /
 *  	short	num_gid;		/ * numberof gid's in array	* /
 *  	gid_t	gids[NGROUP];		/ * groups machine belongs to	* /
 *  	uid_t	uid;			/ * Unix "uid" of machine	* /
 *  	mode_t	umask;			/ * "umask" of machine		* /
 *  	short	tz_offset;		/ * tz offset, min west of GMT	* /
 *  	char	username[32];		/ * ASCII username of user	* /
 *  	char	host[MAXHOSTNAMELEN];	/ * hostname of machine		* /
 *  	char	yphost[MAXHOSTNAMELEN];	/ * YP hostname			* /
 *  	char	ypdomain[MAXDOMAIN];	/ * YP domain we belong to	* /
 *  	char	gateway[MAXHOSTNAMELEN];/ * Default gateway, if any	* /
 *  	char	configname[20];		/ * "port" name			* /
 *  	char	configfile[80];		/ * location of config file	* /
 *  	char	broadcast[20];		/ * IP broadcast address		* /
 *  	char	subnetmask[20];		/ * subnet mask value		* /
 *  	char	tzname[8];		/ * Name of timezone we're in	* /
 *  	gid_t	gid;			/ * GID to use in file/dir create* /
 *  	char	domain[MAXDOMAIN];	/ * Internet domain		* /
 *	BOOL	usedns;			/ * Should we use DNS? * /
 *  	short   syslog_window_pri;  / * <= this means message goes to window * /
 *  	short   syslog_file_pri;    / * <= this send message to file * /
 *  	char	syslog_file[MAXSYSLOGFILEMAME] ; / * path/name to syslog file * /
 *  };
 *
 *  #define CONFIG_NAME "inet.configuration"
 *
 *  #define GETCONFIG(p) {	Forbid(); \
 *  			((p) = (struct config *)FindPort(CONFIG_NAME));\
 *  			Permit();\
 *  		     }
 **************************************************************************
 */

extern struct SocketLibrary *SockBase;

int __saveds reconfig(void) ;
struct config *init_config(int);

/****** socket.library/getuid ******************************************
*
*   NAME
*	getuid  -- Get user id.
*
*   SYNOPSIS
*	#include <sys/types.h>
*
*	uid = getuid()
*	D0
*
*	uid_t getuid (void);
*
*   FUNCTION
*	Returns the user id.
*
*   INPUTS
*	None.
*
*   RESULT
*	uid		- user ID or -1 (on error).  An error requester will
*			  be displayed if there is a problem reading the
*			  current configuration.
*
*   EXAMPLE
*
*   NOTES
*	This is an emulation of the Unix getuid() function.
*	geteuid() is equivalent to getuid() on the Amiga.
*
*	Currently, the configuration information is stored in memory
*	in a configuration structure.  If this structure cannot be
*	found, it will be initialized by reading in inet:s/inet.config.
*	This may change in the future.  There is no support for multiple
*	user IDs on a single Amiga because the Amiga OS has no concept
*	of multiple users.
*
*   BUGS
*
*   SEE ALSO
*	getgid(), getgroups()
*
******************************************************************************
*
*/

uid_t __saveds getuid()
{
	register struct config *cf = SockBase->ml_origbase->ml_config ;

	return( cf ? cf->uid : -1 ) ;
}


/****** socket.library/getgid ******************************************
*
*   NAME
*	getgid -- Get group id.
*
*   SYNOPSIS
*	#include <sys/types.h>
*
*	gid = getgid()
*
*	gid_t getgid (void);
*
*   FUNCTION
*	returns the user's group id
*
*   INPUTS
*	None.

*   RESULT
*	gid		- group ID or -1 (on error). An error requester
*			  will be displayed if there is a problem reading
*			  the current configuration.
*
*   EXAMPLE
*
*   NOTES
*	This is an emulation of the Unix getgid() function.
*	getegid() is equivalent to getgid() on the Amiga.  Note that the
*	user has one primary group ID, but may have several additional
*	secondary group IDs which can only be obtained with getgroups().
*
*	Currently, the configuration information is stored in memory
*	in a configuration structure.  If this structure cannot be
*	found, it will be initialized by reading in inet:s/inet.config.
*	This may change in the future.
*
*   BUGS
*
*   SEE ALSO
*	getgroups(), getuid()
*
******************************************************************************
*
*/


gid_t __saveds getgid()
{
	register struct config *cf = SockBase->ml_origbase->ml_config ;

	return( cf ? cf->gid : (gid_t)-1 ) ;
}

/****** socket.library/getgroups ******************************************
*
*   NAME
*	getgroups -- Get group access list.
*
*   SYNOPSIS
*	#include <sys/types.h>
*	#include <sys/param.h>
*
*	num = getgroups(max_gids, gids)
*	D0		D0	  A0
*
*	int getgroups (int, gid_t *);
*
*   FUNCTION
*	The array "gids" is filled with the group ids that the current user
*	belongs to (including the primary gid).  The list may be up to
*	"max_gids" long.  The actual number of gids is returned in 'num.'
*
*   INPUTS
*	max_gids	- length of gids array.
*	gids		- gid_t array.
*
*   RESULT
*	num		- the number of gids is returned if successful.
*			  No errors are currently defined.
*
*   EXAMPLE
*	gid_t gids[10];
*	int number_of_gids;
*
*	number_of_gids = getgroups(10,gids);
*
*   NOTES
*	Currently, the configuration information is stored in memory
*	in a configuration structure.  If this structure cannot be
*	found, it will be initialized by reading in inet:s/inet.config.
*	This may change in the future.
*
*	The upper limit of groups that can be returned by getgroups() is
*	NGROUP (in <sys/param.h>).
*
*   BUGS
*	This routine has had problems including the primary gid.
*	These problems should be fixed, so please report if you
*	see this bug.
*
*   SEE ALSO
*	getgid(), getuid()
*
******************************************************************************
*
*/

int __saveds __asm getgroups(register __d0 int num,register __a0 gid_t *gp)
{
	register struct config *cf = SockBase->ml_origbase->ml_config ;
	register int i;

	if(cf)
	{
		for(i = 0; i < num && i < cf->num_gid; i++)
		{
			gp[i] = cf->gids[i];
		}
		return (int)i;
	}
	else
	{
		return(-1) ;
	}
}

/****** socket.library/getlogin ******************************************
*
*   NAME
*	getlogin -- Get login name.
*
*   SYNOPSIS
*	name = getlogin()
*	D0
*
*	char *getlogin (void);
*
*   FUNCTION
*	Returns a pointer to the current user name.
*
*   INPUTS
*	None.
*
*   RESULT
*	name		- a pointer to the current user name or NULL to
*			  indicate an error.  You can use this pointer for
*			  as long as necessary, but do not write to it.
*
*   NOTES
*	Currently, the configuration information is stored in memory
*	in a configuration structure.  If this structure cannot be
*	found, it will be initialized by reading in inet:s/inet.config.
*	This may change in the future.  There is no support for multiple
*	user names on a single Amiga because the Amiga OS has no concept
*	of multiple users.
*
*
******************************************************************************
*
*/

char * __saveds getlogin()
{
	register struct config *cf = SockBase->ml_origbase->ml_config ;

	return(cf ? cf->username : NULL) ;
}


/****** socket.library/get_tz ******************************************
*
*   NAME
*	get_tz -- Get timezone offset.
*
*   SYNOPSIS
*	offset = get_tz()
*	D0
*
*	short get_tz(void);
*
*   FUNCTION
*	Returns the number offset from UTC (Universal Time Coordinated,
*	formerly Greenwich Mean Time) in minutes west of UTC.
*
*   RESULT
*	offset		- offset in minutes or -1.  An error requester
*			  is displayed on error.
*
*   NOTES
*	THIS IS AN AMIGA-ONLY FUNCTION.
*
*	Currently the configuration information is stored in memory
*	in a configuration structure.  If this structure cannot be
*	found, it will be initialized by reading in inet:s/inet.config.
*	This may change in the future.
*
*   BUGS
*	Does not account for daylight savings time.  If the time is really
*	important to you (or hosts you communicate with) you must currently
*	change your inet:s/inet.config for daylight savings/standard time.
*
******************************************************************************
*
*/

short __saveds get_tz()
{
	register struct config *cf = SockBase->ml_origbase->ml_config ;

	return( cf ? cf->tz_offset : -1 ) ;
}



/****** socket.library/getdomainname ******************************************
*
*   NAME
*	getdomainname -- Get domain name.
*
*   SYNOPSIS
*	return = getdomainname( name, namelen)
*	D0			A1    D1
*
*	int getdomainname (char *, int);
*
*   FUNCTION
*	Returns the name of the domain into the pointer specified.
*	Name will be null-terminated if sufficient space is available.
*	To find out what a domain name is, check your favorite TCP/IP
*	reference.
*
*   INPUTS
*	name		- pointer to character buffer.
*	namelen		- space available in 'name.'
*
*   RESULT
*	0 is returned if successful.  -1 is returned on error.
*
*   EXAMPLE
*
*   NOTES
*	There is currently no corresponding setdomainname() function.
*
*	Currently the configuration information is stored in memory
*	in a configuration structure.  If this structure cannot be
*	found, it will be initialized by reading in inet:s/inet.config.
*	This may change in the future.
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/


int __saveds __asm getdomainname( register __a1 char *buf, register __d1 int size)
{
	register struct config *cf = SockBase->ml_origbase->ml_config ;
	int len = strlen(cf->domain)+1;

	stccpy(buf, cf->domain, (size < len) ? size : len ) ;
	return (0);
}



/****** socket.library/getumask ******************************************
*
*   NAME
*	getumask -- Get user file creation mask.
*
*   SYNOPSIS
*	#include <sys/types.h>
*
*	umask = getumask()
*	D0
*
*	mode_t getumask (void);
*
*   FUNCTION
*	Returns the umask.
*
*   RESULT
*	-1 will be returned and an error message will be displayed
*	if there is a problem reading the current configuration.
*
*   EXAMPLE
*
*   NOTES
*	THIS IS AN AMIGA-SPECIFIC FUNCTION.  It is not a standard Unix
*	function.  See umask().
*
*	Currently, the configuration information is stored in memory
*	in a configuration structure.  If this structure cannot be
*	found, it will be initialized by reading in inet:s/inet.config.
*	This may change in the future.
*
*   BUGS
*
*   SEE ALSO
*	umask()
*
******************************************************************************
*
*/

mode_t __saveds getumask()
{
	register struct config *cf = SockBase->ml_origbase->ml_config ;

	return (cf ? cf->umask : -1 ) ;
}


/****** socket.library/umask ******************************************
*
*   NAME
*	umask  -- get and set user file creation mask
*
*   SYNOPSIS
*	#include <sys/types.h>
*
*	umask = umask( cmask )
*	D0             D0
*
*	mode_t umask ( mode_t );
*
*   FUNCTION
*	The umask() function sets the file creation mask to 'cmask'
*	and returns the old value of the mask.
*
*   RESULT
*	-1 will be returned and an error message will be displayed
*	if there is a problem reading the current configuration.
*
*   EXAMPLE
*
*   NOTES
*	Amiga filesystems, of course, will not use this file creation
*	mask.  We use it for NFS, and provide it in case you can think
*	of something to use it for.
*
*	The new mask value is not saved to the configuration file. It
*	will be reset when the machine is rebooted.
*
*	Currently, the configuration information is stored in memory
*	in a configuration structure.  If this structure cannot be
*	found, it will be initialized by reading in inet:s/inet.config.
*	This may change in the future.
*
*
*   BUGS
*
*   SEE ALSO
*	getumask()
*
******************************************************************************
*
*/

mode_t __saveds __asm umask(register __d0 int new_mask)
{
	register struct config *cf = SockBase->ml_origbase->ml_config ;
	mode_t old_mask = cf->umask;

	cf->umask = (new_mask & 0777);
    return (old_mask);
}

/***************  init_config()  *******************************/

void kprintf( char *, ... ) ;


struct config *init_config( int reconfiguring )
{
	struct config *cf;
	char *p, *name, *value;
	BPTR fh;
	int umask;
	char line[256]; /* must be >128 !! */

	ObtainSemaphore( SockBase->ml_origbase->ml_ConfigSemaphore) ;

	if( ! reconfiguring )
	{
		cf = (struct config *)AllocMem(sizeof(struct config), MEMF_PUBLIC|MEMF_CLEAR);
		if (!cf)
		{
			SockBase->ml_origbase->configured = 0;
			ReleaseSemaphore( SockBase->ml_origbase->ml_ConfigSemaphore) ;
	        return( NULL ) ;
		}
	}
	else
	{
		cf = (struct config *)FindPort(CONFIG_NAME) ;
		if( !cf )
		{
			SockBase->ml_origbase->configured = 0;
			ReleaseSemaphore( SockBase->ml_origbase->ml_ConfigSemaphore) ;
	        return( NULL ) ;
		}
	}

	strcpy(cf->configname, CONFIG_NAME);
	strcpy(cf->syslog_file, "ram:syslog.dat") ;
	cf->syslog_file_pri = 4 ; /* (short)SockBase->ml_origbase->ml_SyslogFilePri ; */
	cf->syslog_window_pri = 3 ; /* (short)SockBase->ml_origbase->ml_SyslogWindowPri ; */
	cf->mp.mp_Node.ln_Name = cf->configname ;

	fh = Open("inet:s/inet.config", MODE_OLDFILE);
	if(!fh)
	{
		if(! reconfiguring )
		{
			FreeMem(cf,sizeof(*cf));
		}
		SockBase->ml_origbase->configured = 0;
		ReleaseSemaphore( SockBase->ml_origbase->ml_ConfigSemaphore) ;
		return (NULL);
	}
        _res.options &= ~RES_INIT;
	cf->num_gid = 0;
	cf->domain[0]=0;

	while((name=FGets(fh,line,sizeof(line))) != NULL)
	{
		value = NULL ;
		if(p = strchr(name, '='))
		{
			*p++ = 0;
			value = p;
			if(p = strchr(value, '\n'))
				*p = 0;
		}
		else
		{
			continue;
		}

		if(stricmp("umask", name) == 0){
			(void)stco_i(value,&umask);
			cf->umask = (mode_t)umask;
		} else if(stricmp("tzname", name) == 0){
			strcpy(cf->tzname, value);
		} else if(stricmp("tz", name) == 0){
			cf->tz_offset = (short)atoi(value);
		} else if(stricmp("user", name) == 0){
			strcpy(cf->username, value);
		} else if(stricmp("uid", name) == 0){
			cf->uid = (uid_t)atoi(value);
		} else if(stricmp("host", name) == 0){
			strcpy(cf->host, value);
		} else if(stricmp("domain", name) == 0){
			strcpy(cf->domain, value);
		} else if(stricmp("usedns", name) ==0){
			if(!stricmp("true",value))
				cf->usedns = TRUE;
		} else if(stricmp("gateway", name) == 0){
			strcpy(cf->gateway, value);
		} else if(stricmp("configfile", name) == 0){
			strcpy(cf->configfile, value);
		} else if(stricmp("broadcast", name) == 0){
			strcpy(cf->broadcast, value);
		} else if(stricmp("subnetmask", name) == 0){
			strcpy(cf->subnetmask, value);
		} else if(stricmp("gid", name) == 0){
			cf->gid = (gid_t)atoi(value);
		} else if(stricmp("gids", name) == 0){
			if(cf->num_gid < NGROUP){
				cf->gids[cf->num_gid++] = (uid_t)atoi(value);
			}
		} else if(stricmp("windowpri", name) == 0){
			cf->syslog_window_pri = (short)atoi(value);
		} else if(stricmp("filepri", name) == 0){
			cf->syslog_file_pri = (short)atoi(value);
		} else if(stricmp("syslogfilename", name) == 0){
			strncpy(cf->syslog_file, (char *)value,MAXSYSLOGFILEMAME-1) ;
		}
	}
	if (_res.defdname[0] == 0) {
		if (p = strchr(cf->host, '.'))
			(void)strcpy(cf->domain, p + 1);
	}

	Close(fh);

	if( ! reconfiguring )
	{
		NewList(&cf->mp.mp_MsgList);
		Forbid();
		AddPort((struct MsgPort *)cf);
		Permit();
		SockBase->ml_origbase->configured = 1;
		SockBase->ml_origbase->ml_config = cf ;
	}

	ReleaseSemaphore( SockBase->ml_origbase->ml_ConfigSemaphore) ;
	return (cf);
}

/****** socket.library/reconfig ****************************************
*
*   NAME
*	reconfig - re-initialize the internal configuration structure
*
*   SYNOPSIS
*	return = reconfig()
*	D0
*
*	BOOL reconfig( VOID ) ;
*
*   FUNCTION
*	Causes the socket library to read the inet:s/inet.config file.
*	This is useful for when you have changed an entry in the file
*	and need the system to recognize the change without a system
*	reboot.
*
*   INPUTS
*	None
*
*   RESULT
*	Boolean return - TRUE upon success, FALSE upon error
*
*   NOTES
*	Make -no- assumptions about how this works internally. The current
*	mechanism is in transition and is guaranteed to change.
*
*   BUGS
*
*   SEE ALSO
*
*************************************************************************
*
*/


int __saveds reconfig(void)
{
	return( init_config( 1 ) ? TRUE : FALSE ) ;

}

/**************************************************************
 * unconfig() - internal
 **************************************************************
 */


/* INTERNAL - this is only called on library expunge */

void __saveds __asm unconfig (register __a6 struct SocketLibrary *libbase)
{
	struct config *cf = libbase->ml_origbase->ml_config ;
	SysBase = (struct ExecBase *)(*((ULONG *)4));

	if(cf)
	{
		Forbid();
		RemPort((struct MsgPort *)cf);
		Permit();
		FreeMem(cf,sizeof(struct config));
	}

	return ;
}



/****** socket.library/gethostname ******************************************
*
*   NAME
*	gethostname -- Get the name of your Amiga.
*
*   SYNOPSIS
*	return = gethostname( name, length )
*	D0                    A0    D0
*
*	int gethostname (char *, int);
*
*   FUNCTION
*	Copies the  null-terminated hostname to 'name'.  The hostname will
*	be truncated if insufficient space is available in 'name'.
*
*   INPUTS
*	name    	- pointer to character array.
*	length  	- size of character array.
*
*   RESULT
*	return		- 0 if successful, else -1.  The only reason for
*			  failure will be if the configuration file is
*			  unavailable, in which case an error requester will
*			  be displayed.
*
*   NOTES
*	Currently, the configuration information is stored in memory
*	in a configuration structure.  If this structure cannot be
*	found, it will be initialized by reading in inet:s/inet.config.
*	This may change in the future.
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/


int __saveds __asm gethostname(register __a0 char *p,register __d0 int plen)
{
	register struct config *cf = SockBase->ml_origbase->ml_config ;
	int len;

	if(!cf || !cf->host[0])
	{
		*p = 0;
		return -1;
	}

	len = strlen(cf->host)+1;
	len = MIN(len, plen);
	stccpy(p, cf->host, len);

	return 0;
}

