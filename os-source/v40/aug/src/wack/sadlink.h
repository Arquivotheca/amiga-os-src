/*
 * Amiga Grand Wack
 *
 * sadlink.h - Definitions
 *
 * $Id: sadlink.h,v 1.3 93/04/27 14:38:11 peter Exp $
 *
 */

#define MYSADERR_ALLOCFAIL	5001	/* remote_error for allocation failure */
#define MYSADERR_LOSTCONTACT	5002	/* remote_error for broken connection */

struct SADContextFrame
{
    /* The first three are READ-ONLY...  Mainly used to make it
     * easier to understand what is going on in the system.
     */
    ULONG sad_VBR;		/* Current VBR (always 0 on 68000 CPUs) */
    ULONG sad_AttnFlags;	/* ULONG copy of the flags (UPPER WORD==0) */
    ULONG sad_ExecBase;		/* ExecBase */
    /* These fields are the user registers...  The registers are
     * restored from these fields on exit from SAD...
     * Note that USP is only valid if SR was *NOT* supervisor...
     */
    ULONG sad_USP;		/* User stack pointer */
    ULONG sad_D0;		/* User register d0 */
    ULONG sad_D1;		/* User register d1 */
    ULONG sad_D2;		/* User register d2 */
    ULONG sad_D3;		/* User register d3 */
    ULONG sad_D4;		/* User register d4 */
    ULONG sad_D5;		/* User register d5 */
    ULONG sad_D6;		/* User register d6 */
    ULONG sad_D7;		/* User register d7 */
    ULONG sad_A0;		/* User register a0 */
    ULONG sad_A1;		/* User register a1 */
    ULONG sad_A2;		/* User register a2 */
    ULONG sad_A3;		/* User register a3 */
    ULONG sad_A4;		/* User register a4 */
    ULONG sad_A5;		/* User register a5 */
    ULONG sad_A6;		/* User register a6 */
    /* This is for SAD internal use...  It is the prompt that
     * SAD is using...  Changing this will have no effect on SAD.
     */
    ULONG sad_Prompt;		/* SAD Prompt Longword...  (internal use) */
    /* From here on down is the standard exception frame
     * The first two entries (SR and PC) are standard on all 680x0 CPUs
     */
    UWORD sad_SR;		/* Status register (part of exception frame) */
    ULONG sad_PC;		/* Return address (part of exception frame) */
};
