#ifndef CONFIG_H
#define CONFIG_H

#include <exec/types.h>
#include <exec/ports.h>
#include <sys/types.h>
#include <sys/param.h>

#ifdef AZTEC_C
#include <functions.h>
#endif

#ifdef LATTICE
#include <proto/exec.h>
#endif

/*
 * format of config record.
 */
struct config {
	struct MsgPort mp;		/* so we can hang on ports list	*/
	short	num_gid;		/* numberof gid's in array	*/
	uid_t	gids[NGROUP];		/* groups machine belongs to	*/
	uid_t	uid;			/* Unix "uid" of machine	*/
	short	umask;			/* "umask" of machine		*/
	short	tz_offset;		/* tz offset, min west of GMT	*/
	char	username[32];		/* ASCII username of user	*/
	char	host[MAXHOSTNAMELEN];	/* hostname of machine		*/
	char	yphost[MAXHOSTNAMELEN];	/* YP hostname			*/
	char	ypdomain[MAXDOMAIN];	/* YP domain we belong to	*/
	char	gateway[MAXHOSTNAMELEN];/* Default gateway, if any	*/
	char	configname[20];		/* "port" name			*/
	char	configfile[80];		/* location of config file	*/
	char	broadcast[20];		/* IP broadcast address		*/
	char	subnetmask[20];		/* subnet mask value		*/
	char	tzname[8];		/* Name of timezone we're in	*/
	uid_t	gid;			/* GID to use in file/dir create*/
	char	domain[MAXDOMAIN];	/* Internet domain		*/
};

#define CONFIG_NAME "inet.configuration"

#define GETCONFIG(p) {	Forbid(); \
			((p) = (struct config *)FindPort(CONFIG_NAME));\
			Permit();\
		     }
#endif	/* CONFIG_H */
