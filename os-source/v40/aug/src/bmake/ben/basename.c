/*	basename.c
 *  (c) Copyright 1991 by Ben Eng, All Rights Reserved
 *
 */

#include <graphics/gfxbase.h>
#include <dos/dos.h>
#include <clib/dos_protos.h>

extern struct GfxBase *GfxBase;

char *
basename( char *pathname )
{
	char *ptr, *filename;

	if( GfxBase->LibNode.lib_Version >= 36L ) {
		filename = FilePart( pathname );		
	}
	else {
		for( filename = ptr = pathname; *ptr; ptr++ )
			if( *ptr == '/'		/* directory delimiter */
				|| *ptr == ':'	/* device delimiter */
				) filename = ptr+1;

	}
	return( filename );
}

