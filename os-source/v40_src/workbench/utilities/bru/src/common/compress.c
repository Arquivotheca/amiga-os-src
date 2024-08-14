/*
 *  FILE
 *
 *	compress.c    file compression ala IEEE Computer, June 1984.
 *
 *  DESCRIPTION
 *
 *	Algorithm from "A Technique for High Performance Data Compression",
 *	Terry A. Welch, IEEE Computer Vol 17, No 6 (June 1984), pp 8-19.
 *
 *	Algorithm:
 *
 * 	Modified Lempel-Ziv method (LZW).  Basically finds common
 *	substrings and replaces them with a variable size code.  This is
 *	deterministic, and can be done on the fly.  Thus, the decompression
 *	procedure needs no input table, but tracks the way the table was built.
 *
 *  NOTES
 *
 *	This code is derived from public domain code and is not subject
 *	to any of the restrictions of the rest of the bru source code
 *	with respect to copyright or licensing.  The contributions of
 *	the original authors, noted below, are gratefully acknowledged.
 *
 *	Based on: compress.c,v 4.0 85/07/30 12:50:00 joe Release $";
 *
 *  AUTHORS
 *
 *	Spencer W. Thomas	decvax!harpo!utah-cs!utah-gr!thomas
 *	Jim McKie		decvax!mcvax!jim
 *	Steve Davies		decvax!vax135!petsd!peora!srd
 *	Ken Turkowski		decvax!decwrl!turtlevax!ken
 *	James A. Woods		decvax!ihnp4!ames!jaw
 *	Joe Orost		decvax!vax135!petsd!joe
 *
 */

#include "globals.h"

#ifdef pdp11
# define NO_UCHAR	/* also if "unsigned char" functions as signed char */
#endif			/* pdp11 *//* don't forget to compile with  -i */

#ifdef z8000
# undef vax			/* weird preprocessor */
#endif	/* z8000 */

/*
 * a code_int must be able to hold 2**maxbits values of type int, and also -1
 */

typedef int code_int;
typedef long int count_int;

#ifdef NO_UCHAR
typedef char char_type;
#else
typedef unsigned char char_type;
#endif /* UCHAR */

/* Defines for third byte of header */

#define BIT_MASK	0x1f
#define BLOCK_MASK	0x80

/*
 * Masks 0x40 and 0x20 are free.  I think 0x20 should mean that there is
 * a fourth header byte (for expansion).
 */

#define INIT_BITS 9		/* initial number of bits/code */

/* Define upper and lower limits on number of bits */

#define MINMAXBITS 12

#ifdef lint
#define MAXMAXBITS (16)		/* shut up lint */
#else
#define MAXMAXBITS (sizeof(int) > 2 ? 16 : 15)
#endif

#define MAXCODE(n_bits)	((1 << (n_bits)) - 1)

/*
 * To save much memory, we overlay the table used by compress() with those
 * used by decompress().  The tab_prefix table is the same size and type
 * as the codetab.  The tab_suffix table needs 2**maxbits characters.  We
 * get this from the beginning of htab.  The output stack uses the rest
 * of htab, and contains characters.  There is plenty of room for any
 * possible stack (stack used to be 8000 characters).
 */

#define tab_prefixof(i)		codetabof(i)
#define htabof(i)		htab[i]
#define codetabof(i)		codetab[i]
#define tab_suffixof(i)		((char_type *)(htab))[i]
#define de_stack		((char_type *)&tab_suffixof(1<<maxbits))

/*
 * the next two codes should not be changed lightly, as they must not
 * lie within the contiguous general code space.
 */

#define FIRST	257		/* first free entry */
#define	CLEAR	256		/* table clear output code */

#define CHECK_GAP 10000		/* ratio check interval */

static int offset;

static char_type magic_header[] = {
    "\037\235"
}; /* 1F 9D */

static int n_bits;			/* current number of bits/code */
static int maxbits;			/* user setable max number bits/code */
static code_int maxcode;		/* maximum code, given n_bits */

static code_int maxmaxcode;		/* should NEVER generate this code */

static count_int *htab;
static unsigned short *codetab;

static code_int free_ent = 0;		/* first unused entry */

static BOOLEAN initdone = FALSE;	/* Flag for initialization done */
static BOOLEAN inittried = FALSE;	/* Flag for initialization tried */

/*
 * Various hsize's, indexed by current maxbits.
 */

static code_int hsize[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		/* 0-11 bits */
    5003,					/* 12 bits (80% occupancy) */
    9001,					/* 13 bits (91% occupancy) */
    18013,					/* 14 bits (91% occupancy) */
    35023,					/* 15 bits (94% occupancy) */
    69001					/* 16 bits (95% occupancy) */
};

/*
 * block compression parameters -- after all codes are used up,
 * and compression rate changes, start over.
 */

static int block_compress = BLOCK_MASK;
static int clear_flg = 0;
static long int ratio = 0;

static count_int checkpoint = CHECK_GAP;

static long int in_count = 1;		/* length of input */
static long int bytes_out;		/* length of compressed output */
static long int out_count = 0;		/* # of codes output (for debugging) */

static char_type lmask[9] = {
    0xff, 0xfe, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0, 0x80, 0x00
};

static char_type rmask[9] = {
    0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff
};

static char buf[MAXMAXBITS];

static int resultflag;

static BOOLEAN alloctables PROTO((void));
static VOID output PROTO((code_int code, struct finfo *fip));
static VOID cl_block PROTO((struct finfo *fip));
static VOID cl_hash PROTO((count_int clhsize));
static code_int getcode PROTO((struct finfo *fip));
static int fillinbuf PROTO((struct finfo *fip, int zflag));
static VOID flushoutbuf PROTO((struct finfo *fip, int zflag));
static VOID io_init PROTO((void));

/*
 *	The following macros and variables allow us to do our own
 *	buffering of input and output streams, without involving
 *	stdio.  This gains us slightly better error handling and
 *	fits in more smoothly with the rest of the bru code
 *	(low level I/O when possible).
 */

#if CRAMPED
#define IOSIZE (256)
#else
#define IOSIZE (1024)		/* Chunk size for I/O */
#endif

static unsigned char inbuf[IOSIZE];
static unsigned char outbuf[IOSIZE];
static int incnt = 0;
static int outcnt = 0;
static unsigned char *inbufp = inbuf;
static unsigned char *outbufp = outbuf;

#define PUTC(ch,fip)	*outbufp++=(ch); if(++outcnt==IOSIZE)flushoutbuf(fip,0)
#define GETC(fip)	(incnt-- > 0 ? (int) *inbufp++ : fillinbuf (fip,0))
#define FFLUSH(fip)	flushoutbuf(fip,0)

#define ZPUTC(ch,fip)	*outbufp++=(ch); if(++outcnt==IOSIZE)flushoutbuf(fip,1)
#define ZGETC(fip)	(incnt-- > 0 ? (int) *inbufp++ : fillinbuf (fip,1))
#define ZFFLUSH(fip)	flushoutbuf(fip,1)

/*
 *	In order to reduce the amount of space allocated statically
 *	(helps us to squeeze bru into split I&D model on a 286), we
 *	allocate the htab and codetab arrays dynamically during
 *	initialization.  This is slightly better practice anyway...
 */

static BOOLEAN alloctables ()
{
    unsigned int nbytes;
    BOOLEAN result = TRUE;

    DBUG_ENTER ("alloctables");
    DBUG_PRINT ("maxbits", ("maxbits = %d", maxbits));
    maxmaxcode = 1 << maxbits;
    DBUG_PRINT ("maxmaxcode", ("maxmaxcode = %d", maxmaxcode));
    nbytes = hsize[maxbits] * sizeof (unsigned short);
    if ((codetab = (unsigned short *) get_memory (nbytes, FALSE)) == NULL) {
	bru_message (MSG_MALLOC, nbytes);
	result = FALSE;
    }
    nbytes = hsize[maxbits] * sizeof (count_int);
    if ((htab = (count_int *) get_memory (nbytes, FALSE)) == NULL) {
	bru_message (MSG_MALLOC, nbytes);
	result = FALSE;
	if (codetab != NULL) {
	    s_free ((char *) codetab);
	}
    }
    if (!result) {
	if (flags.Zflag & (Z_ON | Z_AUTO)) {
	    bru_message (MSG_NOZ);
	}
	flags.Zflag = Z_OFF;
    }
    inittried = TRUE;
    initdone = result;
    DBUG_RETURN (result);
}

/*
 * Algorithm:  use open addressing double hashing (no chaining) on the 
 * prefix code / next character combination.  We do a variant of Knuth's
 * algorithm D (vol. 3, sec. 6.4) along with G. Knott's relatively-prime
 * secondary probe.  Here, the modular division first probe is gives way
 * to a faster exclusive-or manipulation.  Also do block compression with
 * an adaptive reset, whereby the code table is cleared when the compression
 * ratio decreases, but after the table fills.  The variable-length output
 * codes are re-sized at this point, and a special CLEAR code is generated
 * for the decompressor.  Late addition:  construct the table according to
 * file size for noticeable speed improvement on small files.  Please direct
 * questions about this implementation to ames!jaw.
 */

int compress (fip)
struct finfo *fip;
{
    long fcode;
    code_int i;
    int c;
    code_int ent;
    int disp;
    code_int hsize_reg;
    int hshift;

    DBUG_ENTER ("compress");
    io_init ();
    maxbits = nzbits;
    DBUG_PRINT ("maxbits", ("use maxbits = %d", maxbits));
    if (maxbits < MINMAXBITS) maxbits = MINMAXBITS;
    if (maxbits > MAXMAXBITS) maxbits = MAXMAXBITS;
    if (inittried && !initdone) DBUG_RETURN (0);
    if (!inittried && !alloctables ()) DBUG_RETURN (0);
    resultflag = 1;
    DBUG_PRINT ("zresult", ("result = %d", resultflag));
    ZPUTC ((char) magic_header[0], fip);
    ZPUTC ((char) magic_header[1], fip);
    ZPUTC ((char) (maxbits | block_compress), fip);
    offset = 0;
    bytes_out = 3;		/* includes 3-byte header mojo */
    out_count = 0;
    clear_flg = 0;
    ratio = 0;
    in_count = 1;
    checkpoint = CHECK_GAP;
    maxcode = MAXCODE (n_bits = INIT_BITS);
    free_ent = ((block_compress) ? FIRST : 256);

    ent = GETC (fip);

    hshift = 0;
    for (fcode = (long) hsize[maxbits]; fcode < 65536L; fcode *= 2L) {
	hshift++;
    }
    hshift = 8 - hshift;	/* set hash code range bound */

    hsize_reg = hsize[maxbits];
    cl_hash ((count_int) hsize_reg);	/* clear hash table */

    while ((c = GETC (fip)) != EOF) {
	DBUG_PRINT ("Z", ("next input = %#x", c));
	in_count++;
	fcode = (long) (((long) c << maxbits) + ent);
	i = ((c << hshift) ^ ent);	/* xor hashing */
	DBUG_PRINT ("Z", ("i = %d", i));
	if (htabof (i) == fcode) {
	    ent = codetabof (i);
	    DBUG_PRINT ("Z", ("continue with ent %#x", ent));
	    continue;
	} else if ((long) htabof (i) < 0) {	/* empty slot */
	    DBUG_PRINT ("Z", ("goto nomatch"));
	    goto nomatch;
	}
	disp = hsize_reg - i;	/* secondary hash (after G. Knott) */
	if (i == 0) {
	    disp = 1;
	}
probe: 
	DBUG_PRINT ("Z", ("at probe"));
	if ((i -= disp) < 0) {
	    i += hsize_reg;
	}	
	DBUG_PRINT ("Z", ("i = %d", i));
	if (htabof (i) == fcode) {
	    ent = codetabof (i);
	    DBUG_PRINT ("Z", ("continue with ent %#x", ent));
	    continue;
	}
	if ((long) htabof (i) > 0) {
	    DBUG_PRINT ("Z", ("goto probe"));
	    goto probe;
	}
nomatch: 
	DBUG_PRINT ("Z", ("at nomatch"));
	output ((code_int) ent, fip);
	out_count++;
	DBUG_PRINT ("Z", ("new out_count %d", out_count));
	ent = c;
	if (free_ent < maxmaxcode) {
	    codetabof (i) = free_ent++;		/* code -> hashtable */
	    htabof (i) = fcode;
	} else if ((count_int) in_count >= checkpoint && block_compress) {
	    cl_block (fip);
	}
    }
    /* 
     * Put out the final code.
     */
    output ((code_int) ent, fip);
    out_count++;
    DBUG_PRINT ("Z", ("final out_count %d", out_count));
    output ((code_int) -1, fip);
    DBUG_PRINT ("cmpr", ("%ld chars in", in_count));
    DBUG_PRINT ("cmpr", ("%ld codes (%ld bytes) out", out_count, bytes_out));
    DBUG_PRINT ("cmpr", ("largest code was %d (%d bits)", free_ent - 1, n_bits));
    ZFFLUSH (fip);
    DBUG_RETURN (resultflag);
}

/*
 * TAG( output )
 *
 * Output the given code.
 * Inputs:
 * 	code:	A n_bits-bit integer.  If == -1, then EOF.  This assumes
 *		that n_bits =< (long)wordsize - 1.
 * Outputs:
 * 	Outputs code to the file.
 * Assumptions:
 *	Chars are 8 bits long.
 * Algorithm:
 * 	Maintain a maxbits character long buffer (so that 8 codes will
 *	fit in it exactly). When the buffer fills up empty it and start over.
 */

static VOID output (code, fip)
code_int code;
struct finfo *fip;
{
    int r_off = offset;
    int bits = n_bits;
    char *bp = buf;

    DBUG_ENTER ("output");
    DBUG_PRINT ("Z", ("output code %#x", code));
    if (code >= 0) {
	/* 
	 * Get to the first byte.
	 */
	bp += (r_off >> 3);
	r_off &= 7;
	/* 
	 * Since code is always >= 8 bits, only need to mask the first
	 * hunk on the left.
	 */
	*bp = (*bp & rmask[r_off]) | (code << r_off) & lmask[r_off];
	bp++;
	bits -= (8 - r_off);
	code >>= 8 - r_off;
	/* Get any 8 bit parts in the middle (<=1 for up to 16 bits). */
	if (bits >= 8) {
	    *bp++ = (char) code;
	    code >>= 8;
	    bits -= 8;
	}
	/* Last bits. */
	if (bits) {
	    *bp = (char) code;
	}
	offset += n_bits;
	if (offset == (n_bits << 3)) {
	    bp = buf;
	    bits = n_bits;
	    bytes_out += bits;
	    do {
		ZPUTC (*bp++, fip);
	    } while (--bits);
	    offset = 0;
	}
	/* 
	 * If the next entry is going to be too big for the code size,
	 * then increase it, if possible.
	 */
	if (free_ent > maxcode || (clear_flg > 0)) {
	    /* 
	     * Write the whole buffer, because the input side won't
	     * discover the size increase until after it has read it.
	     */
	    if (offset > 0) {
		ZFFLUSH (fip);
		DBUG_PRINT ("Z", ("write %d bytes out", n_bits));
		if (s_write (fip -> zfildes, buf, (UINT) n_bits) != n_bits) {
		    bru_message (MSG_WRITE, fip -> zfname);
		    resultflag = 0;
		    DBUG_PRINT ("zresult", ("result = %d", resultflag));
		}
		bytes_out += n_bits;
	    }
	    offset = 0;
	    if (clear_flg) {
		maxcode = MAXCODE (n_bits = INIT_BITS);
		clear_flg = 0;
	    } else {
		n_bits++;
		if (n_bits == maxbits) {
		    maxcode = maxmaxcode;
		} else {
		    maxcode = MAXCODE (n_bits);
		}
	    }
	    DBUG_PRINT ("cmpr", ("change to %d bits", n_bits));
	}
    } else {
	/* 
	 * At EOF, write the rest of the buffer.
	 */
	r_off = (offset + 7) / 8;
	if (offset > 0) {
	    ZFFLUSH (fip);
	    DBUG_PRINT ("Z", ("write %d bytes out", r_off));
	    if (s_write (fip -> zfildes, buf, (UINT) r_off) != r_off) {
		bru_message (MSG_WRITE, fip -> zfname);
		resultflag = 0;
		DBUG_PRINT ("zresult", ("result = %d", resultflag));
	    }
	}
	bytes_out += r_off;
	offset = 0;
	ZFFLUSH (fip);
    }
    DBUG_VOID_RETURN;
}

static VOID cl_block (fip)
struct finfo *fip;
{
    long int rat;
    
    DBUG_ENTER ("cl_block");
    checkpoint = in_count + CHECK_GAP;
    DBUG_PRINT ("cmpr", ("count = %ld", in_count));
    if (in_count > 0x007fffff) {	/* shift will overflow */
	rat = bytes_out >> 8;
	if (rat == 0) {			/* Don't divide by zero */
	    rat = 0x7fffffff;
	} else {
	    rat = in_count / rat;
	}
    } else {
	rat = (in_count << 8) / bytes_out;
	/* 8 fractional bits */
    }
    if (rat > ratio) {
	ratio = rat;
    } else {
	ratio = 0;
	cl_hash ((count_int) hsize[maxbits]);
	free_ent = FIRST;
	clear_flg = 1;
	output ((code_int) CLEAR, fip);
    }
    DBUG_VOID_RETURN;
}
    
/* reset code table */

static VOID cl_hash (clhsize)
count_int clhsize;
{
    count_int *htab_p;
    long i;
    long m1 = -1;
	
    DBUG_ENTER ("cl_hash");
    DBUG_PRINT ("clhsize", ("clear %ld entries", clhsize));
    htab_p = htab + clhsize;
    i = clhsize - 16;
    do {	/* might use Sys V memset(3) here */
	*(htab_p - 16) = m1;
	*(htab_p - 15) = m1;
	*(htab_p - 14) = m1;
	*(htab_p - 13) = m1;
	*(htab_p - 12) = m1;
	*(htab_p - 11) = m1;
	*(htab_p - 10) = m1;
	*(htab_p - 9) = m1;
	*(htab_p - 8) = m1;
	*(htab_p - 7) = m1;
	*(htab_p - 6) = m1;
	*(htab_p - 5) = m1;
	*(htab_p - 4) = m1;
	*(htab_p - 3) = m1;
	*(htab_p - 2) = m1;
	*(htab_p - 1) = m1;
	htab_p -= 16;
    } while ((i -= 16) >= 0);
    for (i += 16; i > 0; i--) {
	*--htab_p = m1;
    }
    DBUG_VOID_RETURN;
}
    
/*
 * Decompress fip.  This routine adapts to the codes in the
 * file building the "string" table on-the-fly; requiring no table to
 * be stored in the compressed file.  The tables used herein are shared
 * with those of the compress() routine.  See the definitions above.
 *
 * Use the number of LZW bits stored in the file to override any
 * existing value for number of bits, even if given on command line.
 */

int decompress (fip)
struct finfo *fip;
{
    char_type *stackp;
    int finchar;
    code_int code;
    code_int oldcode;
    code_int incode;

    DBUG_ENTER ("decompress");
    io_init ();
    resultflag = 1;
    DBUG_PRINT ("zresult", ("result = %d", resultflag));
    /* Check the magic number */
    if ((ZGETC (fip) != (magic_header[0] & 0xFF))
	|| (ZGETC (fip) != (magic_header[1] & 0xFF))) {
	resultflag = 0;
	DBUG_PRINT ("zresult", ("result = %d", resultflag));
    } else {
	maxbits = ZGETC (fip);	/* set number of bits from file */
	block_compress = maxbits & BLOCK_MASK;
	maxbits &= BIT_MASK;
	DBUG_PRINT ("maxbits", ("got maxbits = %d from file", maxbits));
	if (maxbits < MINMAXBITS) maxbits = MINMAXBITS;
	if (maxbits > MAXMAXBITS) {
	    bru_message (MSG_BADBITS, maxbits, MAXMAXBITS);
	    DBUG_RETURN (0);
	}
	if (inittried && !initdone) DBUG_RETURN (0);
	if (!inittried && !alloctables ()) DBUG_RETURN (0);
	/* 
	 * Initialize the first 256 entries in the table.
	 */
	maxcode = MAXCODE (n_bits = INIT_BITS);
	for (code = 255; code >= 0; code--) {
	    tab_prefixof (code) = 0;
	    tab_suffixof (code) = (char_type) code;
	}
	free_ent = ((block_compress) ? FIRST : 256);
	
	finchar = oldcode = getcode (fip);
	if (oldcode == -1) {	/* EOF already? */
	    DBUG_RETURN (0);			/* Get out of here */
	}
	PUTC ((char) finchar, fip);
	/* first code must be 8 bits = char */
	stackp = de_stack;
	
	while ((code = getcode (fip)) > -1) {
	    DBUG_PRINT ("code", ("next code = %#x", code));
	    if ((code == CLEAR) && block_compress) {
		for (code = 255; code >= 0; code--) {
		    tab_prefixof (code) = 0;
		}
		clear_flg = 1;
		free_ent = FIRST - 1;
		if ((code = getcode (fip)) == -1) {
		    DBUG_PRINT ("code", ("next code = %#x", code));
		    /* O, untimely death! */
		    DBUG_PRINT ("code", ("terminate from getcode gets EOF"));
		    break;
		}
	    }
	    incode = code;
	    /* 
	     * Special case for KwKwK string.
	     */
	    if (code >= free_ent) {
		*stackp++ = (char_type) finchar;
		code = oldcode;
	    }
	    /* 
	     * Generate output characters in reverse order
	     */
	    while (code >= 256) {
		*stackp++ = tab_suffixof (code);
		code = tab_prefixof (code);
	    }
	    *stackp++ = finchar = tab_suffixof (code);
	    /* 
	     * And put them out in forward order
	     */
	    do {
		PUTC ((char) *--stackp, fip);
		DBUG_PRINT ("code", ("*stackp = %#x", *stackp));
	    } while (stackp > de_stack);
	    /* 
	     * Generate the new entry.
	     */
	    if ((code = free_ent) < maxmaxcode) {
		tab_prefixof (code) = (unsigned short) oldcode;
		tab_suffixof (code) = (char_type) finchar;
		free_ent = code + 1;
	    }
	    /* 
	     * Remember previous code.
	     */
	    oldcode = incode;
	}
	FFLUSH (fip);
    }
    DBUG_RETURN (resultflag);
}


/*
 * TAG( getcode )
 *
 * Read one code from the standard input.  If EOF, return -1.
 * Inputs:
 * 	fip
 * Outputs:
 * 	code or -1 is returned.
 */

static code_int getcode (fip)
struct finfo *fip;
{
    code_int code;
    static int size = 0;
    static int offset = 0;	/* Appears to be needless redefinition */
    static char_type buf[MAXMAXBITS];	/* Appears to be needless redefinition */
    int r_off;
    int bits;
    char_type *bp = buf;
    int nextch;
    
    DBUG_ENTER ("getcode");
    if (clear_flg > 0 || offset >= size || free_ent > maxcode) {
	/* 
	 * If the next entry will be too big for the current code
	 * size, then we must increase the size.  This implies reading
	 * a new buffer full, too.
	 */
	if (free_ent > maxcode) {
	    n_bits++;
	    if (n_bits == maxbits) {
		maxcode = maxmaxcode;	    /* won't get any bigger now */
	    } else {
		maxcode = MAXCODE (n_bits);
	    }
	}
	if (clear_flg > 0) {
	    maxcode = MAXCODE (n_bits = INIT_BITS);
	    clear_flg = 0;
	}
	bp = buf;
	for (size = 0; size < n_bits; size++) {
	    if ((nextch = ZGETC (fip)) == EOF) {
		break;
	    } else {
		DBUG_PRINT ("code", ("next byte from input = %#x", nextch));
		*bp++ = (char_type) nextch;
	    }
	}
	if (size == 0) {
	    DBUG_PRINT ("code", ("return code %#x", -1));
	    DBUG_RETURN (-1);
	}
	bp = buf;
	offset = 0;
	/* Round size down to integral number of codes */
	size = (size << 3) - (n_bits - 1);
    }
    r_off = offset;
    bits = n_bits;
    /* 
     * Get to the first byte.
     */
    bp += (r_off >> 3);
    r_off &= 7;
    /* Get first part (low order bits) */
#ifdef NO_UCHAR
    code = ((*bp++ >> r_off) & rmask[8 - r_off]) & 0xff;
#else
    code = (*bp++ >> r_off);
#endif	/* NO_UCHAR */
    bits -= (8 - r_off);
    r_off = 8 - r_off;		/* now, offset into code word */
    /* Get any 8 bit parts in the middle (<=1 for up to 16 bits). */
    if (bits >= 8) {
#ifdef NO_UCHAR
	code |= (*bp++ & 0xff) << r_off;
#else
	code |= *bp++ << r_off;
#endif /* NO_UCHAR */
	r_off += 8;
	bits -= 8;
    }
    /* high order bits. */
    code |= (*bp & rmask[bits]) << r_off;
    offset += n_bits;
    DBUG_PRINT ("code", ("return code %#x", code));
    DBUG_RETURN (code);
}



static int fillinbuf (fip, zflag)
struct finfo *fip;
int zflag;
{
    int firstchar;
    int fildes;
    char *fname;

    DBUG_ENTER ("fillinbuf");
    inbufp = inbuf;
    if (zflag) {
	fildes = fip -> zfildes;
	fname = fip -> zfname;
    } else {
	fildes = fip -> fildes;
	fname = fip -> fname;
    }
    if ((incnt = s_read (fildes, (char *) inbuf, IOSIZE)) > 0) {
	DBUG_PRINT ("inbuf", ("read %d bytes from input", incnt));
	firstchar = *inbufp++;
	incnt--;
    } else if (incnt == -1) {
	bru_message (MSG_READ, fname);
	resultflag = 0;
	DBUG_PRINT ("zresult", ("result = %d", resultflag));
	firstchar = EOF;
	incnt = 0;
    } else {
	DBUG_PRINT ("inbuf", ("found EOF"));
	firstchar = EOF;
	incnt = 0;
    }
    DBUG_RETURN (firstchar);
}


/*
 *	Flush output buffer.  If -S option is enabled (for intelligent
 *	handling of sparse files) create "holes" in the file whenever the
 *	output buffer is all null bytes, rather than actually writing
 *	a block of nulls.
 *
 *	Note that we only allow seeking during decompression, and only
 *	when the buffer we are about to write is NOT the last buffer
 *	for the file.  Otherwise, we would do the seek (which doesn't
 *	actually extend the file) and then close the file, leaving
 *	it truncated to the size it had at the last write of non-null
 *	data.  In other words, if the file ended in a bunch of nulls,
 *	all of the trailing nulls would be truncated if we didn't
 *	actually write the last buffer with "real" nulls.
 *
 */

static VOID flushoutbuf (fip, zflag)
struct finfo *fip;
int zflag;
{
    int fildes;
    char *fname;
    int bytesout;
    S32BIT offset;
    S32BIT size;
    BOOLEAN tryseek = FALSE;

    DBUG_ENTER ("flushoutbuf");
    if (zflag) {
	fildes = fip -> zfildes;
	fname = fip -> zfname;
    } else {
	fildes = fip -> fildes;
	fname = fip -> fname;
	offset = s_lseek (fildes, 0L, 1);
	if (flags.Sflag && (offset != SYS_ERROR)) {
	    DBUG_PRINT ("zoff", ("current outfile offset = %ld", offset));
	    size = fip -> bstatp -> bst_size;
	    DBUG_PRINT ("zsize", ("size of outfile = %ld", size));
	    if ((offset + outcnt < size) && allnulls ((char *) outbuf, outcnt)) {
		tryseek = TRUE;
	    }
	}
    }
    DBUG_PRINT ("outbuf", ("flush %d bytes from outbuf", outcnt));
    if (outcnt > 0) {
	if (tryseek) {
	    if (s_lseek (fildes, (S32BIT) outcnt, 1) == SYS_ERROR) {
		bru_message (MSG_SEEK, fname);
		bytesout = s_write (fildes, (char *) outbuf, (UINT) outcnt);
	    } else {
		bytesout = outcnt;
	    }
	} else {
	    bytesout = s_write (fildes, (char *) outbuf, (UINT) outcnt);
	}
	if (bytesout != outcnt) {
	    bru_message (MSG_WRITE, fname);
	    resultflag = 0;
	    DBUG_PRINT ("zresult", ("result = %d", resultflag));
	}
    }
    outcnt = 0;
    outbufp = outbuf;
    DBUG_VOID_RETURN;
}


/*
 *  FUNCTION
 *
 *	io_init    initialize io for each new file
 *
 *  SYNOPSIS
 *
 *	static VOID io_init ();
 *
 *  DESCRIPTION
 *
 *	Initialize io for each new file to be compressed or decompressed,
 *	rather than depending upon the previous file leaving the local io
 *	parameters in a consistent state.  This fixes a bug whereby failure
 *	to decompress a file on extraction was causing all future files to
 *	fail during decompression also.
 */

static VOID io_init ()
{
    DBUG_ENTER ("io_init");
    incnt = 0;
    inbufp = inbuf;
    outcnt = 0;
    outbufp = outbuf;
    DBUG_VOID_RETURN;
}
