/**/
/*  Refer: rutgers!sqm.dec.com!jmsynge@cbmvax  (James M Synge, DTN 381-1545)
/*  USENET:  {decvax, ucbvax, allegra}!decwrl!sqm.dec.com!jmsynge
/*  Number: B4198
/*  Date: 5/27/87
/*  Phone: (603) 891-0163 (Home)  (603) 881-1545 (Work)
/*   
/*  Brief Bug Description:
/*  
/*  	OpenLibrary creates the Library structure for a disk based shared 
/*  library, then checks the name of the library.  If the case of the name is
/*  wrong (i.e. ICON.library instead of icon.library) then the code is
/*  removed (with UnloadSeg) but the Library structure is not removed from the
/*  exec library list.  This can lead to a guru long after the library was
/*  'not' openned.
/*  
/*  Run this program with the following command line:
/*  
/*  ltest ICON.library
/*  
/*  then use WACK to list exec's list of libraries (I used Aztec's DB, but WACK
/*  should produce the same result).  It may appear to be correct; if so do
/*  something which allocates memory, then re-display the library list.  I found
/*  that if I did a "DIR C:" I would see the name of the library change to the
/*  name of one of the files.  Very odd looking.
/**/
#include "exec/types.h"
#include "exec/libraries.h"

main(argc, argv)
int argc;
char *argv[];
{
    struct Library  *LibBasePtr, /* Library Base Pointer. */
		    *OpenLibrary();
	
	if (argc != 2) exit(argc);

	LibBasePtr = OpenLibrary(argv[1], 0L);
	if (LibBasePtr == 0L) {
		printf("Unable to open %s\n", argv[1]);
		exit(10);
	}

	/* Finally, close the library. */
	
	CloseLibrary( LibBasePtr );

	exit(0);
}
