/*
** Simple passwd entry for the Amiga.
*/

#ifndef PWD_H
#define PWD_H

#ifndef SYS_TYPES_H
#include <sys/types.h>
#endif

struct passwd {
	char	*pw_name;
	char	*pw_dir;
	char	*pw_passwd;
	char	*pw_gecos;
	uid_t	pw_uid;
	gid_t	pw_gid;
	char	*pw_shell;		/* unused */
	char	*pw_comment;
};

struct passwd *getpwent( void ) ;
struct passwd *getpwnam( char * );
struct passwd *getpwuid( uid_t );

#endif
