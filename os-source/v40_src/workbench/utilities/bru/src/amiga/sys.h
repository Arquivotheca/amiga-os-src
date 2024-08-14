/************************************************************************
 *									*
 *	Copyright (c) 1988 Enhanced Software Technologies, Inc.		*
 *			   All Rights Reserved				*
 *									*
 *	This software and/or documentation is protected by U.S.		*
 *	Copyright Law (Title 17 United States Code).  Unauthorized	*
 *	reproduction and/or sales may result in imprisonment of up	*
 *	to 1 year and fines of up to $10,000 (17 USC 506).		*
 *	Copyright infringers may also be subject to civil liability.	*
 *									*
 ************************************************************************
 */


/*
 *  FILE
 *
 *	sys.h    header file to fake all stuff normally found in unix headers
 *
 *  SCCS
 *
 *	@(#)sys.h	12.8	11 Feb 1991
 *
 *  DESCRIPTION
 *
 *	This header file fakes the stuff normally found in the unix header
 *	files, like the definition for the stat structure, typedefs like
 *	"ushort", etc.
 *
 */

#if !unix && !xenix	/* Just in case we try to include this anyway! */

#define O_CTG	0	/* Found in <fcntl.h> on Masscomps */

/*  Normally found in <sys/types.h> */
#if 0	/* is in new Lattice for AmigaDOS */
typedef unsigned short ushort;
typedef long off_t;
typedef short dev_t;
typedef ushort ino_t;
typedef long time_t;
#endif

/* Normally found in <grp.h> */

struct group {
    char *gr_name;
    char *gr_passwd;
    int gr_gid;
    char **gr_mem;
};

/* Normally found in <pwd.h> */

struct passwd {
    char *pw_name;
    char *pw_passwd;
    int pw_uid;
    int pw_gid;
    char *pw_age;
    char *pw_comment;
    char *pw_gecos;
    char *pw_dir;
    char *pw_shell;
};

/*  Define away some errno values that may not be defined */

#if !EPERM
#  define EPERM 1000	/* Bogus value that should never happen */
#endif

#if !EINTR
#  define EINTR 1000	/* Bogus value that should never happen */
#endif

#endif
