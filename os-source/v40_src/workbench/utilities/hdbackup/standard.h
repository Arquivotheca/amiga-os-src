/************************************************************************
 *									*
 *	Copyright (c) 1989 Enhanced Software Technologies, Inc.		*
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
 * This header just contains includes for headers needed by all
 * (or most) modules, configuration parameters, and some handy macros.
 */

#include <stdio.h>
#include <errno.h>

#include <workbench/workbench.h>
#include <workbench/icon.h>
#include <graphics/display.h>

#include <exec/types.h>
#include <exec/devices.h>
#include <exec/memory.h>
#include <exec/ports.h>

#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <intuition/preferences.h>

#include <libraries/dos.h>
#include <libraries/dosextens.h>

#include <clib/macros.h>


#if LATTICE
#  include <stdlib.h>
#include <ctype.h>
#include <dos.h>
#  define PROTO(a) a
#  include <clib/intuition_protos.h>
#  include <clib/dos_protos.h>
#  include <clib/exec_protos.h>
#  include <clib/graphics_protos.h>
#  include <clib/icon_protos.h>
#  include <clib/diskfont_protos.h>
#  include <clib/utility_protos.h>
#  include <string.h>
#else
#  define PROTO(a) ()
#  include "manxproto.h"
#endif

/*
 *	Configuration parameters.
 */

#define LIB_VER		36			/* Minimum acceptable OS library
								 * version.
								 */

#define BRU_NAME		"bru"
#define BRU_OPTS		" -Af -PF -Pp "
#define BRU_LOGDIR		"SYS:HDBackupLogs"
#define BRU_CONFIG		"S:HDBackup.config"
#define IPCPORT_NAME	"HDBACKUP_CBM"	/* Public IPC port name */
#define EMBED_LOGNAME	"T:HDBackup_tmp_log"

#define MINIMUM_STACK		16000
#define MINIMUM_STACK_SC	"16000"

#define TITLESTRING	"HDBackup"
#define PROCNAME	"HDBackup"

#define SBUF_SIZE		80		/* String buffer size */
#define SCRATCH_START	100		/* Max # of entries in scratch buffer */
#define ROOTNAME		"DH0:"	/* The default volume to backup/restore */
#define MAX_CUR_PATH	300
#define MAX_NAME		32
#define CONFIG_OPTS		128		/* Max number of options from config file */
#define HELP_FILENAME	"HDBackup.help"


/*
 * This one is intended for structure members, used in place of
 * the usual NULL.  Indicates that something WILL go there at run time
 * ( Run-Time-Bound ).
 */
#define RTB 0L

/* Convert non-zero to an Intuition boolean true value */
#define Bool(x) ((BOOL)((x)?TRUE:FALSE))

/*---------------------   These are from RJ's ProSuite   --------------*/

#define SetFlag(v,f)		((v)|=(f))
#define ClearFlag(v,f)		((v)&=~(f))
#define ToggleFlag(v,f) 	((v)^=(f))
#define FlagIsSet(v,f)		((BOOL)(((v)&(f))!=0))
#define FlagIsClear(v,f)	((BOOL)(((v)&(f))==0))


/* I always thought this was supposed to be in the standard headers.
 * It's not.  Oh, well, define it here.
 */
#ifndef MODE_READONLY
#define MODE_READONLY	MODE_OLDFILE
#endif

