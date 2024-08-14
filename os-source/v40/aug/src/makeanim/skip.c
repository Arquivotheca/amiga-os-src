/***************************************************************************
*
*  This contains routines to compress images one of two ways:
*      1. Vertical Byte Run Length (VRUN)
*      2. Vertical Byte Run Length With Skips (SKIP)
*   from amiga/programs #274, dated jul 8 1987...
*
* The routines here compress a single bit-plane.  There's routines to
* see how big the compressed result will be (xxxx_count_plane() )
* and routines to actually compress the result into a buffer in memory
* (xxxx_comp_plane() )  where xxxx is either vrun or skip depending....
*
*
* VRUN compression format:
*    a VRUN-compressed plane is a concatenation of VRUN-compressed columns.
*    Each column is an op-count followed by a concatenation of op-count op's.
*    An op is in one of these two formats:
*        SAME_OP:  1-byte-count  followed by 1-byte to repeat count times.
*        UNIQ_OP:  1-byte-count  followed by count bytes of data to copy
*    The counts in either op must be 127 or less.  If it's a UNIQ_OP the
*    hi bit of the count op is set (it's masked with 127 to find the actual
*    count).
*
* SKIP compression format:
*    a SKIP compressed plane is a concatenation of SKIP-compressed columns.
*    Like a VRUN this is an op-count followed by a concatenation of op;s.
*    However in this one we have 3 op formats:
*       SKIP_OP:  Hi bit clear.  Count of bytes to skip.
*       UNIQ_OP:  Hi bit set.  Remainder is count of data to copy. Data
*                 follows immediately.
*       SAME_OP:  A zero followed by a one-byte-count followed by one byte
*                 of data to repeat count times.
*
***************************************************************************/

#include <exec/types.h>
#include <graphics/gfx.h>
#include <stdio.h>

/*****************************************************************************/

#define MAXRUN 127

/*****************************************************************************/

static LONG linebytes = 40;
static LONG uniq_count;
static LONG op_count;
static UBYTE *uniq;

/*****************************************************************************/

/* count up how many in a column are the same between in and last_in ...
   ie how big of a "skip" can we go on. */
static LONG vskip (UBYTE *in, UBYTE *last_in, LONG count)
{
    register LONG skips;

    skips = 0;
    while (--count >= 0)
    {
	if (*in !=*last_in)
	    break;
	in +=linebytes;

	last_in += linebytes;
	skips++;
    }
    return ((LONG)skips);
}

/*****************************************************************************/

/* vsame() - count up how many in a row (vertically) are the same as the
   first one ... ie how big of a "same" op we can have */
static LONG vsame (UBYTE *in, LONG count)
{
    register LONG same;
    register UBYTE c;

    c = *in;
    in +=linebytes;

    --count;
    same = 1;
    while (--count >= 0)
    {
	if (*in !=c)
	    break;
	same++;
	in +=linebytes;
    }
    return ((LONG) same);
}

/*****************************************************************************/

/* skip_count_line() - figure out what size this column will compress
   to using vertical-byte-run-length-with-skips encoding */
static LONG skip_count_line (UBYTE *in, UBYTE *last_in, LONG count)
{
    LONG uniq_count = 0;
    LONG comp_count = 1;	/* one for the op count */
    LONG local_count;
    LONG run_length;
    LONG a_run;

    if (vskip (in, last_in, count) == count)	/* skip whole column? */
	return (1);

    for (;;)
    {
	if (count <= 0)
	    break;

	local_count = (count < MAXRUN ? count : MAXRUN);
	a_run = 0;
	if ((run_length = vskip (in, last_in, local_count)) > 1)
	{
	    count -= run_length;
	    if (count > 0)	/* the last skip disappears */
		comp_count += 1;
	    a_run = 1;
	}
	else if ((run_length = vsame (in, local_count)) > 3)
	{
	    count -= run_length;
	    a_run = 1;
	    comp_count += 3;
	}

	if (a_run)
	{
	    in +=run_length * linebytes;

	    last_in += run_length * linebytes;
	    if (uniq_count > 0)
	    {
		comp_count += uniq_count + 1;
		uniq_count = 0;
	    }
	}
	else
	{
	    in +=linebytes;

	    last_in += linebytes;
	    uniq_count++;
	    count -= 1;
	    if (uniq_count == MAXRUN)
	    {
		comp_count += uniq_count + 1;
		uniq_count = 0;
	    }
	}
    }

    if (count != 0)
	printf ("weird end count %d in skip_line_count\n");

    if (uniq_count != 0)
	comp_count += uniq_count + 1;

    return (comp_count);
}

/*****************************************************************************/

/* skip_count_plane() - figure out what size this plane will compress
   to using vertical-byte-run-length-with-skips encoding */
LONG skip_count_plane (UBYTE *in, UBYTE *last_in, LONG next_line, LONG rows)
{
    LONG comp_count;
    LONG i;

    linebytes = next_line;
    comp_count = 0;
    i = next_line;
    while (--i >= 0)
    {
	comp_count += skip_count_line (in, last_in, rows);
	in ++;

	last_in++;
    }
    return (comp_count);
}

/*****************************************************************************/

static LONG copy_line_to_chars (UBYTE *in, UBYTE *out, LONG linebytes, LONG count)
{
    while (count--)
    {
	*out = *in;

	out++;
	in +=linebytes;
    }
    return (0);
}

/*****************************************************************************/

/* flush_uniq() - write out the "uniq" run that's been accumulating while
   we've been looking for skips and "same" runs. */
static UBYTE *flush_uniq (UBYTE *stuff)
{
    if (uniq_count > 0)
    {
	op_count++;
	*stuff++ = (uniq_count | 0x80);
	copy_line_to_chars (uniq, stuff, linebytes, uniq_count);
	stuff += uniq_count;
	uniq_count = 0;
    }
    return (stuff);
}

/*****************************************************************************/

/* skip_comp_line() - Compress "in" into "out" using vertical-byte-run-
   with-skips encodeing. Return pointer to "out"'s next free space. */
static UBYTE *skip_comp_line (UBYTE *in, UBYTE *last_in, UBYTE *out, LONG count)
{
    register UBYTE *stuffit;
    LONG local_count;
    LONG run_length;
    LONG a_run;

    /* if can skip over whole column, then compact a bit by just setting the
     * "op count" for this column to zero */
    if (vskip (in, last_in, count) == count)	/* skip whole column? */
    {
	*out++ = 0;
	return (out);
    }

    op_count = 0;		/* haven't done any op's yet */

    /* initialize variables which keep track of how many uniq bytes we've gone
     * past, and where uniq run started. */
    uniq_count = 0;
    uniq = in;

    stuffit = out + 1;		/* skip past "op-count" slot in out array */
    for (;;)
    {
	if (count <= 0)
	    break;
	local_count = (count < MAXRUN ? count : MAXRUN);
	a_run = 0;		/* first assume not a skip or a same run */

	/* see how much could skip from here.  Two or more is worth skipping! */
	if ((run_length = vskip (in, last_in, local_count)) > 1)
	{
	    a_run = 1;
	    count -= run_length;
	    stuffit = flush_uniq (stuffit);	/* flush pending "uniq" run */
	    if (count > 0)	/* last skip vanishes */
	    {
		op_count++;
		*stuffit++ = run_length;
	    }
	}
	/* four or more of the same byte in a row compresses too */
	else if ((run_length = vsame (in, local_count)) > 3)
	{
	    a_run = 1;
	    count -= run_length;
	    op_count++;
	    stuffit = flush_uniq (stuffit);	/* flush pending "uniq" run */
	    *stuffit++ = 0;
	    *stuffit++ = run_length;
	    *stuffit++ = *in;
	}

	/* If it's a run of some sort update in and last_in pointer,
	 * and reset the uniq pointer to the current position */
	if (a_run)
	{
	    in +=run_length * linebytes;

	    last_in += run_length * linebytes;
	    uniq = in;

	    /* Otherwise just continue accumulating stuff in uniq for later
	     * flushing or immediate if it get's past MAXRUN. */
	}
	else
	{
	    in +=linebytes;

	    last_in += linebytes;
	    uniq_count++;
	    count -= 1;
	    if (uniq_count == MAXRUN)
	    {
		stuffit = flush_uniq (stuffit);
		uniq = in;
	    }
	}
    }

    /* if came to end of column within a uniq-run still have to flush it */
    if (uniq_count != 0)
	stuffit = flush_uniq (stuffit);

    if (count != 0)
	printf ("weird end count %d in skip_line_count\n", count);

    /* and stuff the first byte of this (compressed) column with the op_count */
    *out = op_count;
    return (stuffit);
}

/*****************************************************************************/

/* skip_comp_plane() - Compress "in" into "out" using vertical-byte-run-
   with-skips encodeing. Return pointer to "out"'s next free space. */
UBYTE *skip_comp_plane (UBYTE *in, UBYTE *last_in, UBYTE *out, LONG next_line, LONG rows)
{
    UBYTE *last_out = out;
    LONG i;

    linebytes = next_line;
    i = next_line;
    while (--i >= 0)
    {
	out = skip_comp_line (in, last_in, out, rows);
	last_out = out;
	in ++;

	last_in++;
    }
    return (out);
}

/*****************************************************************************/

/* vrun_count_line() - figure out what size this column will compress
   to using vertical-byte-run-length encoding */
static LONG vrun_count_line (UBYTE *in, LONG count)
{
    LONG uniq_count = 0;
    LONG comp_count = 1;	/* one for the op count */
    LONG local_count;
    LONG run_length;
    LONG a_run;

    for (;;)
    {
	if (count <= 0)
	    break;
	local_count = (count < MAXRUN ? count : MAXRUN);
	a_run = 0;
	if ((run_length = vsame (in, local_count)) > 2)
	{
	    a_run = 1;
	    comp_count += 2;
	}
	if (a_run)
	{
	    in +=run_length * linebytes;

	    count -= run_length;
	    if (uniq_count > 0)
	    {
		comp_count += uniq_count + 1;
		uniq_count = 0;
	    }
	}
	else
	{
	    in +=linebytes;

	    uniq_count++;
	    count -= 1;
	    if (uniq_count == MAXRUN)
	    {
		comp_count += uniq_count + 1;
		uniq_count = 0;
	    }
	}
    }

    if (count != 0)
	printf ("weird end count %d in vrun_line_count\n");

    if (uniq_count != 0)
	comp_count += uniq_count + 1;

    return (comp_count);
}

/*****************************************************************************/

/* vrun_count_plane() - figure out what size this plane will compress
   to using vertical-byte-run-length encoding */
static LONG vrun_count_plane (UBYTE *in, LONG next_line, LONG rows)
{
    LONG comp_count;
    LONG i;

    linebytes = next_line;
    comp_count = 0;
    i = next_line;
    while (--i >= 0)
    {
	comp_count += vrun_count_line (in, rows);
	in ++;
    }
    return (comp_count);
}

/*****************************************************************************/

/* vrun_comp_line() - Compress "in" into "out" using vertical-byte-run
   encodeing. Return pointer to "out"'s next free space. */
static UBYTE *vrun_comp_line (UBYTE *in, UBYTE *out, LONG count)
{
    register UBYTE *stuffit;
    LONG local_count;
    LONG run_length;
    LONG a_run;

    uniq_count = op_count = 0;
    uniq = in;

    stuffit = out + 1;
    for (;;)
    {
	if (count <= 0)
	    break;
	local_count = (count < MAXRUN ? count : MAXRUN);
	a_run = 0;
	if ((run_length = vsame (in, local_count)) > 2)
	{
	    a_run = 1;
	    stuffit = flush_uniq (stuffit);
	    *stuffit++ = run_length;
	    *stuffit++ = *in;
	}
	if (a_run)
	{
	    op_count++;
	    in +=run_length * linebytes;

	    count -= run_length;
	    uniq = in;
	}
	else
	{
	    in +=linebytes;

	    uniq_count++;
	    count -= 1;
	    if (uniq_count == MAXRUN)
	    {
		stuffit = flush_uniq (stuffit);
		uniq = in;
	    }
	}
    }

    if (uniq_count != 0)
	stuffit = flush_uniq (stuffit);

    if (count != 0)
	printf ("weird end count %d in vrun_line_count\n", count);

    *out = op_count;
    return (stuffit);
}

/*****************************************************************************/

/* vrun_comp_plane() - Compress "in" into "out" using vertical-byte-run
   encodeing. Return pointer to "out"'s next free space. */
static UBYTE *vrun_comp_plane (UBYTE *in, UBYTE *out, LONG next_line, LONG rows)
{
    UBYTE *last_out = out;
    LONG i;

    linebytes = next_line;
    i = next_line;
    while (--i >= 0)
    {
	out = vrun_comp_line (in, out, rows);
	last_out = out;
	in ++;
    }
    return (out);
}
