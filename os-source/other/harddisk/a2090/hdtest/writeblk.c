#include "ddefs.h"
extern int	wdwrite();
void write_block(block_no,data_addr)
ULONG block_no;			/* Number of block to write	*/
register char  *data_addr;	/* Pointer to data to be written*/
{
	register char *cp;	/* Pointer to buffer		*/

	for (cp = (char *)BUFPADDR;cp < (char *)(BUFPADDR+512);)
			*cp++ = *data_addr++; /* Copy data to buffer */
	wdwrite(block_no);	/* Write block to disk		*/
}
