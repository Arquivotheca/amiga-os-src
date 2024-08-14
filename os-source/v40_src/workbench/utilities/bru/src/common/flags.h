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
 *	flags.h    command line flags structure definition
 *
 *  SCCS
 *
 *	@(#)flags.h	12.8	11 Feb 1991
 *
 *  SYNOPSIS
 *
 *	#include "flags.h"
 *
 *  DESCRIPTION
 *
 *	Rather than make all flags global, they are collected into
 *	a single structure which is then made global.  This simplifies
 *	the bookkeeping.
 */


struct cmd_flags {		/* Flags set on command line */
    BOOLEAN aflag;		/* -a option given */
    BOOLEAN Aflag;		/* -A option given */
    BOOLEAN Acflag;		/* -A option with c arg */
    BOOLEAN Afflag;		/* -A option with f arg */
    BOOLEAN Aiflag;		/* -A option with i arg */
    BOOLEAN Arflag;		/* -A option with r arg */
    BOOLEAN Asflag;		/* -A option with s arg */
    BOOLEAN bflag;		/* -b option given */
    BOOLEAN Bflag;		/* -B option given */
    BOOLEAN cflag;		/* -c option given */
    BOOLEAN Cflag;		/* -C option given */
    int dflag;			/* -d option given (multilevel) */
    BOOLEAN Dflag;		/* -D option given */
    BOOLEAN eflag;		/* -e option given */
    int fflag;			/* -f option given (multiple devices) */
    BOOLEAN Fflag;		/* -F option given */
    BOOLEAN gflag;		/* -g option given */
    BOOLEAN hflag;		/* -h option given */
    BOOLEAN iflag;		/* -i option given */
    BOOLEAN Iflag;		/* -I option given */
    BOOLEAN lflag;		/* -l option given */
    BOOLEAN Lflag;		/* -L option given */
    BOOLEAN mflag;		/* -m option given */
    BOOLEAN nflag;		/* -n option given */
    BOOLEAN Nflag;		/* -N option given */
    BOOLEAN oflag;		/* -o option given */
    BOOLEAN pflag;		/* -p option given */
    BOOLEAN Pflag;		/* -P option given */
    BOOLEAN PEflag;		/* -P option with e/E arg */
    BOOLEAN PFflag;		/* -P option with f/F arg */
    BOOLEAN PPflag;		/* -P option with p/P arg */
    BOOLEAN Rflag;		/* -R option given */
    BOOLEAN sflag;		/* -s option given */
    BOOLEAN Sflag;		/* -S option given */
    BOOLEAN tflag;		/* -t option given */
    BOOLEAN uflag;		/* -u option given */
    BOOLEAN uaflag;		/* -u option with a arg */
    BOOLEAN ubflag;		/* -u option with b arg */
    BOOLEAN ucflag;		/* -u option with c arg */
    BOOLEAN udflag;		/* -u option with d arg */
    BOOLEAN ulflag;		/* -u option with l arg */
    BOOLEAN upflag;		/* -u option with p arg */
    BOOLEAN urflag;		/* -u option with r arg */
    int vflag;			/* -v option given (multilevel verbosity) */
    BOOLEAN wflag;		/* -w option given */
    BOOLEAN xflag;		/* -x option given */
    int Zflag;			/* -Z option; use file compression */
};


/*
 *	The Zflag contains various bits which describe what sort
 *	of things to do with compression.
 */

#define Z_OFF	(00000)		/* No file compression to be used */
#define Z_AUTO	(00001)		/* Sometimes try compression */
#define Z_ON	(00002)		/* Always try compression */
