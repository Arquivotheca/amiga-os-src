/************************************************************************
 *									*
 *	Copyright (c) 1988 Enhanced Software Technologies, Inc.		*
 *			     All Rights Reserved			*
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
 *	config.h    contains configuration parameters
 *
 *  SCCS
 *
 *	@(#)config.h	12.8	11 Feb 1991
 *
 *  SYNOPSIS
 *
 *	#include "config.h"
 *
 *  DESCRIPTION
 *
 *	This file contains parameters which can be changed
 *	as desired to alter bru's defaults.
 *
 *	However, be aware that changing some of these parameters
 *	may make it incompatible with the older versions of bru
 *	or even one of it's siblings with other parameters.
 *
 */


/*
 *	Fundamental constants, these should be considered sacred and
 *	only changed with great reluctance.  Changing them will make your
 *	bru archives incompatible with the rest of the bru's in the world.
 *
 */

#define BLKSIZE		2048		/* # of bytes per logical block */
#define NAMESIZE	128		/* File name size in info block */
#define XNAMESIZE	1024		/* File name extension size */
#define IDSIZE		64		/* Bru ID string size in info block */
#define BLABELSIZE	64		/* Archive label size in info block */
#define MODESZ		10		/* Size of mode string */
#define UTSELSZ		9		/* Size of my_utsname elements */
#define NZBITS		12		/* Default LZW compression bits */


/*
 *	The following parameters are not so critical and may be
 *	tuned appropriately.
 *
 *	BUFSIZE		The default archive I/O buffer size.  It is
 *			recommended that you do not change this.  It
 *			can be set on a per device basis in the brutab
 *			file, with the "bufsize" parameter.
 *
 *	TERMINAL	The default name of the file to attempt to open
 *			to communicate with an operator.  Can be overridden
 *			with the -I option.
 *
 *	BRUTAB		The default name of the brutab file.  Can be
 *			overridden with the BRUTAB environment variable.
 *
 *	BRUTMPDIR	The default directory to use for temporary files
 *			when needed.  Can be overridden with the BRUTMPDIR
 *			environment variable.
 *
 *	BRUTABSIZE	The size of the buffer used to hold each brutab
 *			entry.
 *			
 *	B_SHMMAX	The default limit on size of each shared memory
 *			segment.
 *
 *	B_SHMSEG	The default number of shared memory segments for
 *			double buffering.
 *
 *	B_SHMALL	The default limit on total amount of shared
 *			memory used.
 */

#define BUFSIZE		(10 * BLKSIZE)	/* Default archive buffer size */

#if amigados
#define TERMINAL	"con:10/15/620/80/BRU Interaction Window"
#define BRUTAB		"s:brutab"	/* Data file for device table */
#define BRUTMPDIR	"ram:"		/* Prefered dir for temp files */
#else
#define TERMINAL	"/dev/tty"	/* Terminal for interaction */
#define BRUTAB		"/etc/brutab"	/* Data file for device table */
#define BRUTMPDIR	"/usr/tmp"	/* Prefered dir for temp files */
#endif

#define BRUTABSIZE 	1024		/* Brutab entry buffer size */

#define B_SHMMAX	(64 * 1024)	/* Limit on each shm segment size */
#define B_SHMSEG	5		/* Limit on number of shm segments */

#define B_SHMALL	(B_SHMMAX * B_SHMSEG)	/* Max shared mem used */

#define FNAMEPREF	"brutmp/"	/* Prefix partial fnames with this */
#define PREFSIZE	7		/* strlen (FNAMEPREF) */

#define DIR_MAGIC	0777		/* Default directory permissions */
					/* This is modified by umask */


/*
 *	RELEASE	is the numeric release number.  It gets incremented
 *		when there are massive changes and/or bug fixes.
 *		This point may be rather arbitrary.
 *
 *	LEVEL	is the numeric release level number.  It get incremented
 *		each time minor changes (updates) occur.
 *
 *	ID	is the verbose bru identification.  It can be
 *		no longer than IDSIZE - 1 characters!
 *
 *	VARIANT is a unique sequentially incremented number which
 *		gets changed each time a potential incompatibility
 *		is introduced.  This allows limited backward
 *		compatibility.
 *
 *	Note that when sccs gets a file for editing, the RELEASE
 *	and LEVEL keyword substitutions are not made.  When testing
 *	such revisions, the preprocessor symbol TESTONLY must be
 *	defined to nonzero.
 *
 */


#if TESTONLY
#    define BRU_RELEASE		(0)
#    define BRU_LEVEL		(0)
#else
#    define BRU_RELEASE		(12)
#    define BRU_LEVEL		(8)
#endif

#if amigados
#define BRU_ID		"Amiga Release 1.2"
#else
#if AUX
#define BRU_ID		"A/UX Release 1.0"
#else
#define BRU_ID		"Fourth OEM Release"
#endif
#endif

#define BRU_VARIANT		(1)

/*
 *  Simple combinations of any above defines, for readability.
 */

#define BRUPATHMAX XNAMESIZE	/* max pathname size including null byte */
