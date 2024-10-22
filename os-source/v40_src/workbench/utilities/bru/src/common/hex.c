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
 *	hex.c    routines to convert to/from hex
 *
 *  SCCS
 *
 *	@(#)hex.c	12.8	11 Feb 1991
 *
 *  DESCRIPTION
 *
 *	For greater portability across machine boundries, all
 *	numeric information in the archive header blocks is
 *	stored in ascii hex form.  These routines convert between
 *	the external and internal forms.
 *
 *  NOTE
 *
 *	Up until version 12.1, bru generated and expected archives
 *	with lowercase hex (I.E. "abcdef" instead of "ABCDEF").  No
 *	known released version of bru ever generated uppercase hex,
 *	and in fact, due to a coding oversight, would not have been
 *	able to read it's own archives if it did.  The current code
 *	now accepts archives with either upper or lowercase hex.
 *
 */

#include "globals.h"

#define GETBITS(x) (isdigit(x) ? ((x)-'0') : ((s_tolower(x))-'a'+10))


/*
 *  FUNCTION
 *
 *	fromhex    convert from hex to internal format
 *
 *  SYNOPSIS
 *
 *	S32BIT fromhex (cpin, insize)
 *	char *cpin;
 *	int insize;
 *
 *  DESCRIPTION
 *
 *	Given pointer to an ascii hex string and the size of the
 *	string, converts the string to a signed 32 bit number and
 *	returns the result.  Note that the string may not be null
 *	terminated, therefore its size is determined by "insize".
 *
 *	Note that previous versions of this routine have used
 *	sscanf or strtol.  Profiling indicated that bru
 *	was spending a significant percentage of it's time in
 *	these routines so the conversion is now handled "brute force".
 *
 *	Note that this version is now less portable since it assumes
 *	that the native character set is ascii and that integers
 *	have "normal" bit patterns so they can be built by shifts and
 *	masks in the order coded in.  These potential problems would
 *	have been hidden in strtol() or sscanf().
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin fromhex
 *	    Initialize result accumulator to zero
 *	    If string pointer is bad or size is bad then
 *		Inform user about bug
 *	    Else
 *		While there is leading whitespace
 *		    Skip the whitespace character
 *		    One less character to convert
 *		End while
 *		While there are more characters that are hex
 *		    Shift result left to make room for four bits
 *		    Mask in the lower bits
 *		    Skip to next input character
 *		End while
 *	    End if
 *	    Return result
 *	End fromhex
 *
 */

S32BIT fromhex (cpin, insize)
char *cpin;
int insize;
{
    S32BIT result;

    result = 0L;
    if (cpin == NULL || insize > 8 || insize < 1) {
	bru_message (MSG_BUG, "fromhex");
    } else {
	while (isspace (*cpin)) {
	    ++cpin;
	    --insize;
	}
	while (insize-- && isxdigit (*cpin)) {
	    result <<= 4;
	    result |= GETBITS (*cpin);
	    ++cpin;
	}
    }
    return (result);
}


/*
 *  FUNCTION
 *
 *	tohex    convert from internal format to ascii hex
 *
 *  SYNOPSIS
 *
 *	VOID tohex (out, val, outsize)
 *	char *out;
 *	S32BIT val;
 *	int outsize;
 *
 *  DESCRIPTION
 *
 *	Given pointer to place to store ascii string (out), value
 *	to convert to ascii hex (val), and the size of the output
 *	buffer, converts the value to hex and saves in the the
 *	specified buffer.  Note that the result is not null terminated.
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin tohex
 *	    If no output buffer or bad buffer size
 *		Inform user about bug
 *	    Else
 *		Convert value to hex string
 *		Copy string to output
 *	    End if
 *	End tohex
 *
 */

VOID tohex (out, val, outsize)
char *out;
S32BIT val;
int outsize;
{
    char buf[9];
    char *bufp;

    if (out == NULL || outsize > 8 || outsize < 1) {
	bru_message (MSG_BUG, "tohex");
    } else {
	(VOID) s_sprintf (buf, "%*lx", outsize, val);
	for (bufp = buf; *bufp; bufp++) {
	    *bufp = s_tolower (*bufp);
	}
	(VOID) s_memcpy (out, buf, outsize);
    }
}

