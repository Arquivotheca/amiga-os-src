
#include "exec/types.h"
#include "expansion.h"

ULONG ExpansionBase;

main()
{
    long seg;
    long res;

    ExpansionBase = OpenLibrary( EXPANSIONNAME, 0 );

    kprintf( "Expansion library = 0x%lx\n", ExpansionBase );

    if( ExpansionBase ) {
	ConfigChain( 0xe80000 );
    }

}

