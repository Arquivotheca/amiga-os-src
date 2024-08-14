/* ========================================================================= *
 * Instsum.c - checksum calculator for use with the Installer                *
 * By Joe Pearce. (c) 1990 Sylvan Technical Arts                             *
 * ========================================================================= */

#ifndef EXEC_TYPES_H
#include <libraries/dos.h>
#endif

#include "functions.h"
#include "xfunctions.h"

int main(int argc,char *argv[])
{	struct FileHandle	*fh;
	long				nval,
						actual,
						sum = 0;

	if (argc < 2 || (argc == 2 && argv[1][0] == '?'))
	{
		Puts("USAGE: instsum filename");
		return 5;
	}

	if ( !(fh = Open(argv[1],MODE_OLDFILE)) )
	{
		Puts("Cannot open file!");
		return 5;
	}

		/* calculate a simple checksum... */
	do
	{	nval = 0;
		actual = Read(fh,&nval,4L);
		if (actual > 0) sum += nval;
	} while	(actual == 4);

	Close(fh);

	if (actual < 0)
	{
		Puts("Error in reading file - cannot compute checksum.");
		return 10;
	}

	Printf("Checksum of file \"%s\": %ld\n",argv[1],sum);
	return 0;
}
