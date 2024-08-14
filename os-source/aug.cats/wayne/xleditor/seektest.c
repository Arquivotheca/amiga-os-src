date ; /*

failat 1
SC LINK LIBS=lib:debug.lib INCDIR=include MODIFIED NOSTKCHK NOICONS SeekTest
quit
*/


#include <exec/types.h>
#include <exec/exec.h>
#include <dos/dos.h>
#include <dos/dosextens.h>

#include <graphics/gfx.h>
#include <graphics/modeid.h>

#include <hardware/custom.h>
#include <hardware/intbits.h>

#include <stdio.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/graphics_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>

#include "cdxl/pan.h"

IMPORT struct Library * DOSBase;

main()
{
    BPTR	fh;
    PAN		pan;
    LONG	pos;

    if ( fh = Open( "Work:CDXLFiles/blade.xl", MODE_OLDFILE ) ) {

	Read( fh, &pan, sizeof (pan) );

	kprintf("FRAME_SIZE( &pan )= %ld\n",FRAME_SIZE( &pan ));
	Delay( 80 );

	kprintf("Before Seek 1\n");
	pos = Seek( fh, FRAME_SIZE( &pan ) * 1000, OFFSET_BEGINNING );
	kprintf("After Seek 1 pos= %ld\n",pos);
	Delay( 50 );


	kprintf("Before Seek 2\n");
	pos = Seek( fh, FRAME_SIZE( &pan ), OFFSET_CURRENT );
	kprintf("After Seek 2 pos= %ld\n",pos);
	Delay( 50 );


	kprintf("Before Seek 3\n");
	pos = Seek( fh, FRAME_SIZE( &pan ), OFFSET_CURRENT );
	kprintf("After Seek 3 pos= %ld\n",pos);
	Delay( 50 );


	kprintf("Before Seek 4\n");
	pos = Seek( fh, FRAME_SIZE( &pan ), OFFSET_CURRENT );
	kprintf("After Seek 4 pos= %ld\n",pos);
	Delay( 50 );


///////////

	kprintf("Before Seek BACK 1\n");
	pos = Seek( fh, -FRAME_SIZE( &pan ), OFFSET_CURRENT );
	kprintf("After Seek 2 pos= %ld\n",pos);
	Delay( 50 );


	kprintf("Before Seek BACK 2\n");
	pos = Seek( fh, -FRAME_SIZE( &pan ), OFFSET_CURRENT );
	kprintf("After Seek 3 pos= %ld\n",pos);
	Delay( 50 );


	kprintf("Before Seek BACK 3\n");
	pos = Seek( fh, -FRAME_SIZE( &pan ), OFFSET_CURRENT );
	kprintf("After Seek 4 pos= %ld\n",pos);
	Delay( 50 );

	kprintf("Before Seek BACK 4\n");
	pos = Seek( fh, -FRAME_SIZE( &pan ), OFFSET_CURRENT );
	kprintf("After Seek 4 pos= %ld\n",pos);
	Delay( 50 );


#ifdef	OUTT	// [
//////////


	kprintf("Before Seek to END\n");
	pos = Seek( fh, 0, OFFSET_END );
	kprintf("After Seek 5 pos= %ld\n",pos);
	Delay( 50 );


	kprintf("Before Seek From END 1\n");
	pos = Seek( fh, -FRAME_SIZE( &pan ), OFFSET_END );
	kprintf("After Seek 1 pos= %ld\n",pos);
	Delay( 50 );


	kprintf("Before Seek From END 2\n");
	pos = Seek( fh, 2*-FRAME_SIZE( &pan ), OFFSET_END );
	kprintf("After Seek 2 pos= %ld\n",pos);
	Delay( 50 );


	kprintf("Before Seek From END 3\n");
	pos = Seek( fh, 3*-FRAME_SIZE( &pan ), OFFSET_END );
	kprintf("After Seek 3 pos= %ld\n",pos);
	Delay( 50 );


	kprintf("Before Seek From END 4\n");
	pos = Seek( fh, 4*-FRAME_SIZE( &pan ), OFFSET_END );
	kprintf("After Seek 4 pos= %ld\n",pos);
	Delay( 50 );

#endif		// ]


	Close( fh );
    }

} // main()
