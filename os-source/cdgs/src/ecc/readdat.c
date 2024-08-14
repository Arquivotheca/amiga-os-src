/* ================================================================================= */
/*
 * $Id: readdat.c,v 1.1 93/01/14 15:56:54 havemose Exp Locker: havemose $
 *
 * Test module to read ecc data from disk.
 * Created: Jan 3, 1993, Allan Havemose
 * See ecc.doc for technical documentation.
 */
/* ================================================================================= */



#include	<stdio.h>
#include	"ecc.h"

int	Read_ECCData(UBYTE *Buffer, char *name, int block_num)
{

	FILE	*fp;
	int	blocks;
	int	ok=1;

	if (fp=fopen(name,"r"))
	{

		fseek(fp,block_num*SECTOR_LEN,SEEK_SET);
		blocks = fread(Buffer,SECTOR_LEN,1,fp);
		if (blocks != 1)
		{
		  printf("\nError reading %s. %d blocks transferred",name,blocks);
		  ok = 0;
		}

		fclose(fp);
	}
	else
		ok = FALSE;
	return ok;
}