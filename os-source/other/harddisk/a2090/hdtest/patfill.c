#include <exec/types.h>
#include <lattice/stdio.h>

extern	char	patno;		/* selected pattern number */
extern	char	shift_count;	/* patt_fill uses for pattern shift */

patt_fill(pattern,len,blockno)
char	*pattern;		/* holds the pattern currently in use */
register long len;		/* number of bytes to fill */
register long blockno;		/* block number - pattern "F" only */
{
	register char	*cp;
	register UWORD	*wp;
	register int	i;
	register long	*lp;
	register int	sect_count;

	cp = pattern;		/* initialize character pointer */
	wp = pattern;		/* initialize word pointer */
	lp = pattern;		/* initialize long pointer */
	switch (patno) {	/* select specified pattern */
	case 0:
		for (sect_count = 0; sect_count < (int)(len / 512);
			sect_count++)
		{
			for (i = 0; i < 256; i++)
				*cp++ = i;
			for (i = 255; i >= 0; i--)
				*cp++ = i;
		}
		break;
	case 1:	/* all 0x00 */
		for (i = 0; i < (len>>1); ++i)
			*wp++ = 0x0000;
		break;
	case 2:					/* pattern for Eric */
		for (sect_count = 0; sect_count < (int)(len / 512);sect_count++)
		{
			for (i=0; i <= 255; i++) {
				cp[2*i] = 0xFF;	/* data bits 15:8 */
				cp[2*i+1] = i;
			}
			cp += 512;
		}
		break;
	case 3:
		for (sect_count = 0; sect_count < (int)(len / 512);sect_count++)
		{
			for (i=0; i <= 255; i++) {
				cp[2*i] = i;
				cp[2*i+1] = 0xFF;
			}
			cp += 512;
		}
		break;
	case 4:
		shift_count %= 3;
		for (i = 0; i < (len>>1); ++i) {
			switch (shift_count) {
			case 0:
				*wp++ = 0x6db6;
				break;
			case 1:
				*wp++ = 0xdb6d;
				break;
			case 2:
				*wp++ = 0xb6db;
				break;
			} /* end switch */
		    } /* end for */
		break;
	case 5 :/* write block number */
		for (i = 0; i < (len>>2); ++i)
			*lp++ = blockno;
		break;
	case 6:	/* all FF */
		for (i = 0; i < (len>>1); ++i)
			*wp++ = 0xFFFF;
		break;
	case 7: /* Do nothing here - done elsewhere */
		break;
	default:
		puts("\nIllegal Pattern\n");
		break;
	}
}
