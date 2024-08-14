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
 *	chksum.c    routines for computing and testing checksums
 *
 *  SCCS
 *
 *	@(#)chksum.c	12.8	11 Feb 1991
 *
 *  DESCRIPTION
 *
 *	Each archive block has a checksum computed for it.  These
 *	routines compute and test checksums.
 *
 *	The checksum is computed by simply adding all the bytes in the
 *	block, treating each byte as a small signed or unsigned two's
 *	complement integer, as specified by the "uadds" flag, and
 *	accumulating the result as a signed integer of at least 32 bits.
 *	Note that the accumulator will never overflow or underflow given
 *	that we are only computing checksums for small blocks (2K).
 *	Prior to computing the checksum, the bytes used to store the
 *	checksum itself are initialized to a single '0' character
 *	preceeded by blanks.
 *
 *	The original version of bru was developed on a machine for which
 *	the compiler implemented signed characters, and the author was
 *	not (at that time) well versed in the distinction between signed
 *	and unsigned characters, and the implications for portability of
 *	the checksum computation between implementations.  So we are stuck
 *	with the definition that the checksum is to be computed using
 *	signed arithmetic.  However, since there may be instantiations of
 *	bru in real use which were compiled with unsigned chars before this
 *	problem was noted, we need to be able to transparently handle
 *	archives created with either form.  New archives will be created
 *	with the signed arithmetic form.
 *
 *	Unfortunately, many older compilers don't accept the declaration
 *	of character variables as type "signed char", so we have to do
 *	our own explicit sign extension in these cases.  Similarly, for
 *	unsigned char operations we do our own truncation of sign bits.
 *
 */

#include "globals.h"

static BOOLEAN uadds = FALSE;	/* If set, checksums computed unsigned */
static CHKSUM sumblock PROTO((char *base, int nbytes));


/*
 *  FUNCTION
 *
 *	sumblock    compute the actual sum of bytes
 *
 *  SYNOPSIS
 *
 *	static CHKSUM sumblock (base, nbytes)
 *	char *base;
 *	int nbytes;
 *
 *  DESCRIPTION
 *
 *	Given a pointer to the first byte of a block, the size of the
 *	block, and a flag indicating whether or the bytes of the block
 *	are to be treated as signed or unsigned characters, compute a
 *	byte-wise checksum of the block.
 *
 *	Bru typically spends from 20-65 percent of its cpu time computing
 *	checksums.  Be very careful how you modify this routine and be sure
 *	to test carefully for compatibility against the portable C version.
 *
 *	If you wish, you can replace the byte-wise summing code with
 *	custom assembly code for the target machine.  See the notes
 *	in the config script.  This can be a big performance win on
 *	some machines.
 *
 *	This ugly ifdef'd code is an attempt to somewhat optimize portable
 *	signed and unsigned computations, letting the machine do as much
 *	of the work of sign extension or truncation as possible.
 *
 */

#if !FAST_CHKSUM

static CHKSUM sumblock (base, nbytes)
char *base;
int nbytes;
{
    CHKSUM checksum = 0;

    DBUG_ENTER ("sumblock");
    if (uadds) {
	DBUG_PRINT ("sum", ("compute using unsigned arithmetic"));
#if (!CHARS_ARE_SIGNED || UNSIGNED_CHARS_OK)
	{
#if UNSIGNED_CHARS_OK
	    unsigned char *next = (unsigned char *)base;
#else
	    char *next = base;
#endif
	    while (nbytes-- > 0) {
		checksum += *next++;
	    }
	}
#else
	{
	    char *next = base;
	    int val;
	    while (nbytes-- > 0) {
		val = *next++;
		checksum += val & 0xFF;
	    }
	}
#endif
    } else {
	DBUG_PRINT ("sum", ("compute using signed arithmetic"));
#if (CHARS_ARE_SIGNED || SIGNED_CHARS_OK)
	{
#if SIGNED_CHARS_OK
	    signed char *next = (signed char *)base;
#else
	    char *next = base;
#endif
	    while (nbytes-- > 0) {
		checksum += *next++;
	    }
	}
#else
	{
	    char *next = base;
	    int val;
	    while (nbytes-- > 0) {
		val = *next++;
		if (val & 0x80) {
		    val |= ~0xFF;
		}
		checksum += val;
	    }
	}
#endif
    }
    DBUG_RETURN (checksum);
}

#endif	/* FAST_CHKSUM */


/*
 *  FUNCTION
 *
 *	chksum   compute checksum for a block
 *
 *  SYNOPSIS
 *
 *	CHKSUM chksum (blkp)
 *	union blk *blkp;
 *
 *  DESCRIPTION
 *
 *	Computes the checksum for a block.  Note that the
 *	checksum DOES NOT include the ascii characters for the checksum
 *	itself.  The checksum string in the block is replaced
 *	with a '0', preceeded with blanks, before the checksum is computed.
 *	The old value (meaningful or not) is then restored.
 *
 *  NOTES
 *
 *	We cannot use the FROMHEX and TOHEX macros to convert the checksum
 *	string because if the bytes don't represent a valid hex number
 *	then the conversion/restoration process will not preserve the
 *	bytes properly.  Therefore we must save the bytes via an image
 *	copy.
 *
 *	When the fast flag is set, this function simply returns 0 as the
 *	checksum, thus effectively eliminating checksum functionality.
 *	Archives made with the fast flag option must also be read with
 *	the fast flag option.  Beware that this also disables the
 *	automatic byte swapping feature, and other features, that depend
 *	on checksum computation.
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin chksum
 *	    Default checksum is zero
 *	    If fast flag is not set then
 *		Remember old checksum
 *		Replace existing checksum with 0
 *		Compute the block checksum.
 *		Restore previous value of checksum
 *	    Endif
 *	    Return checksum to caller
 *	End chksum
 *
 */

CHKSUM chksum (blkp)
union blk *blkp;
{
    CHKSUM checksum = 0L;
    char oldsum[sizeof(blkp -> HD.h_chk)];

    DBUG_ENTER ("chksum");
    if (!flags.Fflag) {
	(VOID) s_memcpy (oldsum, blkp -> HD.h_chk, sizeof (blkp -> HD.h_chk));
	TOHEX (blkp -> HD.h_chk, 0L);
	checksum = sumblock (blkp -> bytes, ELEMENTS (blkp -> bytes));
	DBUG_PRINT ("checksum", ("computed checksum %#lx", checksum));
	(VOID) s_memcpy (blkp -> HD.h_chk, oldsum, sizeof (blkp -> HD.h_chk));
    }
    DBUG_RETURN (checksum);
}


/*
 *  NAME
 *
 *	chksum_ok    check block checksum
 *
 *  SYNOPSIS
 *
 *	BOOLEAN chksum_ok (blkp)
 *	union blk *blkp;
 *
 *  DESCRIPTION
 *
 *	Given pointer to a block, checks the checksum.
 *	When the "fast flag" is TRUE, this function always
 *	returns TRUE, without doing any checking at all.
 *
 *	When a checksum error is found, the computation is
 *	automatically retried using unsigned arithmetic. If
 *	this one then checks ok, the code is permanently switched
 *	to unsigned mode for future checks.  See the discussion
 *	at the top of this file for why this is necessary, and note
 *	that the first mismatch, due to machine dependencies
 *	introduced by the possibility of versions of bru with totally
 *	unsigned character computations, may occur at any block in
 *	the archive.
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin chksum_ok
 *	    Set default return to FALSE
 *	    If block pointer is null then
 *		Tell user about bug
 *	    Else
 *		If fast flag is set then
 *		    Always return true without checking
 *		Else
 *		    Convert ascii checksum to internal format
 *		    Test against actual computed checksum
 *		    If test failed then
 *			Set up to try with other mode
 *			Test against actual computed checksum
 *			If test failed then
 *			    Return to original mode
 *			Endif
 *		    Endif
 *		Endif
 *	    End if
 *	    Return result
 *	End chksum_ok
 *
 */

BOOLEAN chksum_ok (blkp)
union blk *blkp;
{
    BOOLEAN result;		/* Result of checksum test */
    CHKSUM checksum;		/* Decoded ascii checksum */

    DBUG_ENTER ("chksum_ok");
    result = FALSE;
    if (blkp == NULL) {
	bru_message (MSG_BUG, "chksum_ok");
    } else {
	if (flags.Fflag) {
	    result = TRUE;
	} else {
	    checksum = FROMHEX (blkp -> HD.h_chk);
	    DBUG_PRINT ("dir", ("read checksum %#lx", checksum));
	    result = (checksum == chksum (blkp));
	    if (!result) {
		uadds = !uadds;
		DBUG_PRINT ("uadds", ("retry with uadds = %d", uadds));
		result = (checksum == chksum (blkp));
		if (!result) {
		    DBUG_PRINT ("uadds", ("looks like a real error"));
		    uadds = !uadds;
		}
	    }		
	}
    }
    DBUG_PRINT ("chk", ("result %d", result));
    DBUG_RETURN (result);
}


