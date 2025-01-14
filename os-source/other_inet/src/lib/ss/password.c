/* -----------------------------------------------------------------------
 * password.c
 *
 * $Locker:  $
 *
 * $Id: password.c,v 1.3 92/07/21 16:34:23 bj Exp $
 *
 * $Revision: 1.3 $
 *
 * $Log:	password.c,v $
 * Revision 1.3  92/07/21  16:34:23  bj
 * Socket.library 4.0.
 * Removed _asm key word from functions.
 * 
 * Revision 1.2  91/08/07  14:18:50  bj
 * rcs header added.
 * 
 *
 * $Header: AS225:src/lib/ss/RCS/password.c,v 1.3 92/07/21 16:34:23 bj Exp $
 *
 *------------------------------------------------------------------------
 */

#include "sslib.h"
#include <sys/socket.h>
#include <pwd.h>
#include <stdlib.h>
#include <string.h>

BPTR passf =NULL;
UBYTE pass_stayopen = (UBYTE)0;

/****** socket.library/getpwuid **************************************
*
*   NAME
*	getpwuid -- Search user database for a particular uid.
*
*   SYNOPSIS
*	#include <pwd.h>
*
*	passwd = getpwuid( uid )
*	D0                 D1
*
*	struct passwd *getpwuid( uid_t );
*
*   FUNCTION
*	getpwuid() returns a pointer to a passwd structure.  The passwd
*	structure fields are filled in from the fields contained in
*	one line of the password file.
*
*   INPUTS
*	uid		- uid of passwd entry to look up.
*
*   RESULT
*	passwd 		- a pointer to a passwd struct with its elements
*			  filled in from the passwd file.
*
*		struct passwd {
*			char	*pw_name;
*			char	*pw_dir;
*			char	*pw_passwd;
*			char	*pw_gecos;
*			uid_t	pw_uid;
*			gid_t	pw_gid;
*			char	*pw_shell;	\* currently unused *\
*			char	*pw_comment;
*		};
*
*   NOTES
*	The passwd structure is returned in a buffer that will be
*	overwritten on the next call to getpw*().
*
*   SEE ALSO
*	getpwnam()
*
********************************************************************
*
* get pw entry by uid
*/


struct passwd * __saveds __asm getpwuid(register __d1 uid_t uid)
{
	register struct passwd *pw;

	setpwent(0);
	do {
		pw = getpwent();
	} while(pw && (pw->pw_uid != uid));

	if(!pass_stayopen)
		endpwent();

	return (pw);
}

/****** socket.library/getpwnam *************************************************
*
*   NAME
*	getpwnam -- Search user database for a particular name.
*
*   SYNOPSIS
*	#include <pwd.h>
*
*	passwd = getpwnam( name )
*	D0                  A0
*
*	struct passwd *getpwnam( char * );
*
*   FUNCTION
*	getpwnam() returns a pointer to a passwd structure. The passwd
*	structure fields are filled in from the fields contained in
*	one line of the password file.
*
*   INPUTS
*	name 		- user name of passwd entry to look up.
*
*   RESULT
*	passwd 		- a pointer to a passwd struct with its elements
*			  filled in from the passwd file.
*
*		struct passwd {
*			char	*pw_name;
*			char	*pw_dir;
*			char	*pw_passwd;
*			char	*pw_gecos;
*			uid_t	pw_uid;
*			gid_t	pw_gid;
*			char	*pw_shell;	\* currently unused  *\
*			char	*pw_comment;
*		};
*
*   NOTES
*	The passwd structure is returned in a buffer that will be
*	overwritten on the next call to getpw*().
*
*   SEE ALSO
*	getpwuid()
********************************************************************
*
* get passwd by name
*
*/


struct passwd * __saveds __asm getpwnam(
	register __a0 char *name)
{
	register struct passwd *pw;

	setpwent(0);

	do {
		pw = getpwent();
	} while(pw && stricmp(pw->pw_name, name) != 0);

    if(!pass_stayopen)
		endpwent();

	return (pw);
}


/****** socket.library/getpwent *************************************************
*
*   NAME
*	getpwent -- Read the next line in the password file.
*
*   SYNOPSIS
*	passwd = getpwent()
*	D0
*
*	struct passwd *getpwent( void ) ;
*
*   FUNCTION
*	There is usually no reason to call this function directly.  It
*	is called internally by getpwuid() and getpwnam().  It is provided
*	only for Un*x compatibility.
*
*	Opens the password file if necessary.  Returns the next entry
*	in the file in a passwd structure.
*
*	struct passwd {
*		char	*pw_name;
*		char	*pw_dir;
*		char	*pw_passwd;
*		char	*pw_gecos;
*		uid_t	pw_uid;
*		gid_t	pw_gid;
*		char	*pw_shell;		\* currently unused *\
*		char	*pw_comment;
*	};
*
*   INPUTS
*	None.
*
*   RESULT
*	passwd 		- a pointer to a filled in passwd structure
*			  if successful, else NULL.
*
*   SEE ALSO
*	getpwuid(), getpwnam(), setpwent(), endpwent()
*
********************************************************************
*
*
*/

char *scantok(char *string);


/*
** entry fmt	user|passwd|uid|gid[,gid]|name|dir|shell
*/


struct passwd * __saveds getpwent(void)
{
	char *uid, *gid, *s;
	static struct passwd pwd;
	static char line[128];

	if (passf == NULL) {
		setpwent(0);
		if (passf == NULL)
			return (NULL);
	}

    if(FGets(passf,line,127) == NULL )
    	return (NULL);

	if(s = strchr(line, '\n'))
		*s = '\0';

	pwd.pw_name = line;
	pwd.pw_passwd  = scantok(pwd.pw_name);

	uid = scantok(pwd.pw_passwd);
	gid = scantok(uid);

	pwd.pw_gecos = scantok(gid);
	pwd.pw_dir = scantok(pwd.pw_gecos);
	pwd.pw_shell = scantok(pwd.pw_dir);

	pwd.pw_uid = (uid_t)atoi(uid);
	pwd.pw_gid = (gid_t)atoi(gid);

	return (&pwd);
}

char *scantok(char *string)
{
	char *p = string;
	while(*p) {
		if (*p == '|') {
			*p++ = '\0';
			return(p);
		}
		p++;
	}
	return(p);
}


/****** socket.library/setpwent *************************************************
*
*   NAME
*	setpwent - Opens or rewinds the password file.
*
*   SYNOPSIS
*	setpwent( flag )
*	          D1
*
*	void setpwent( int );
*
*   FUNCTION
*	If the file is already open the file is rewound. Otherwise the
*	file is opened.
*
*   INPUTS
*	flag		- if 'flag' is 1, calls to getpwuid() and getpwnam()
*			  will not close the file between calls.  You must
*			  close the file with an endpwent().  Once set,
*			  'flag' cannot be reset except by calling endpwent().
*
*   RESULT
*	None.
*
*   SEE ALSO
*	endpwent(), getpwent()
*
********************************************************************
*
*
*/

void __saveds __asm setpwent(register __d1 int flag)
{
	if (passf == NULL)
		passf = Open("inet:db/passwd",MODE_OLDFILE);
	else
		Seek(passf,0L,OFFSET_BEGINNING);

	pass_stayopen |= (UBYTE)flag;

}


/****** socket.library/endpwent *************************************************
*
*   NAME
*	endpwent - Closes the password file.
*
*   SYNOPSIS
*	endpwent()
*
*	void endpwent( void );
*
*   FUNCTION
*	Closes the password file if it was open.
*
*   INPUTS
*	None.
*
*   RESULT
*	None.
*
*   SEE ALSO
*	getpwent(), setpwent()
*
********************************************************************
*
*
*/

void __saveds endpwent(void)
{
	if(passf) {
		Close(passf);
		passf = NULL;
		pass_stayopen = (UBYTE)0;
	}
}

