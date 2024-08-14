#include <exec/types.h>
#include <exec/exec.h>
#include <libraries/expansion.h>
#include <libraries/expansionbase.h>
#include <janus/janus.h>
#include <janus/janusreg.h>

#include <proto/exec.h>
#include <proto/expansion.h>
/* #include <local/sccs.h> */

#include "share.h"

/* static char SccsId[] = SCCS_STRING; */

struct Library *ExpansionBase;

UBYTE * SwitchRegister;
UBYTE * john_reg;

int OpenJanus(void);
void DiskToPC(void);
void DiskToAmiga(void);
void DiskStaticPC(void);

int FindJanus(void) 
{
    struct ConfigDev * current;

    ExpansionBase = (struct Library *) OpenLibrary("expansion.library",0);
    if(ExpansionBase == NULL) {
        Debug0("Can't open expansion.library!\n");
        return 0;
    }

/*
    current = FindConfigDev(0, 513, 3);
    if(current == NULL) {
        current = FindConfigDev(0, 513, 1);
        if(current == NULL) {
		current = FindConfigDev(0, 513, 103);
		if (current == NULL) {
	            Debug0("No Turbo Board found!\n");
	            CloseLibrary(ExpansionBase);
	            return 0;
		}
        }
    }
*/
	current = FindConfigDev(0, 513, 103);
	if (current == NULL) {
            Debug0("No 386 Board found!\n");
            CloseLibrary(ExpansionBase);
            return 0;
	}

    SwitchRegister = (UBYTE *) ( (long) current->cd_BoardAddr \
        + IoAccessOffset + IoRegOffset + jio_Control);

	john_reg = (UBYTE *) ( (long) current->cd_BoardAddr + 0x7ff9f);

    Debug1("Turbo found, Switch at %lx\n", (ULONG) SwitchRegister);
    CloseLibrary(ExpansionBase);
    return 1;
}

void writemode(UBYTE bits)
{
	*SwitchRegister = 0x9f | (bits << 5);
}
