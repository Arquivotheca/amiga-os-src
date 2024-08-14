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
 *	exeinfo.h    runtime information collection structure
 *
 *  SCCS
 *
 *	@(#)exeinfo.h	12.8	11 Feb 1991
 *
 *  SYNOPSIS
 *
 *	#include "exeinfo.h"
 *
 *  DESCRIPTION
 *
 *	Define a structure for collecting runtime information about
 *	bru's execution.  Things of interest include number of errors,
 *	number of warnings, total blocks read, total blocks written,
 *	total checksum errors, total soft read/write errors, etc.
 *
 */

struct exe_info {
    int warnings;		/* Number of warnings */
    int errors;			/* Number of errors */
    long breads;		/* Archive blocks read */
    long bwrites;		/* Archive blocks written */
    int rsoft;			/* Soft read errors */
    int wsoft;			/* Soft write errors */
    int rhard;			/* Hard read errors */
    int whard;			/* Hard write errors */
    int chkerr;			/* Checksum errors */
    int zmin;			/* Minimum compression ratio */
    int zmax;			/* Maximum compression ratio */
    long filebytes;		/* Accumulated file sizes */
    long zfilebytes;		/* Accumulated compressed sizes */
};
