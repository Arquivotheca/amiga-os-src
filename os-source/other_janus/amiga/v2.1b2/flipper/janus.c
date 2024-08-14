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

    current = FindConfigDev(0, 513, 3);
    if(current == NULL) {
        current = FindConfigDev(0, 513, 1);
        if(current == NULL) {
            Debug0("No Turbo Board found!\n");
            CloseLibrary(ExpansionBase);
            return 0;
        }
    }
    SwitchRegister = (UBYTE *) ( (long) current->cd_BoardAddr \
        + IoAccessOffset + IoRegOffset + jio_Control);
    Debug1("Turbo found, Switch at %lx\n", (ULONG) SwitchRegister);
    CloseLibrary(ExpansionBase);
    return 1;
}

void DiskToPC(void)
{
    register UBYTE temp;

#if 0
    temp = *SwitchRegister;
    temp |= JCNTRLF_DISKTOAMIGA | JCNTRLF_DISKTOPC;
    temp &= ~JCNTRLF_DISKTOPC;
#else
	temp = ~JCNTRLF_DISKTOPC;
#endif
    Debug0("PC gets Disk\n");
    *SwitchRegister = temp;
}

void DiskToAmiga(void)
{
    register UBYTE temp;

#if 0
    temp = *SwitchRegister;
    temp |= JCNTRLF_DISKTOAMIGA | JCNTRLF_DISKTOPC;
    temp &= ~JCNTRLF_DISKTOAMIGA;
#else
    temp = ~JCNTRLF_DISKTOAMIGA;
#endif
    Debug0("Amiga gets Disk\n");
    *SwitchRegister = temp;
}

void DiskStaticPC()
{
    register UBYTE temp;

#if 0
    temp = *SwitchRegister;
    temp &= ~(JCNTRLF_DISKTOAMIGA | JCNTRLF_DISKTOPC);
#else
    temp = ~(JCNTRLF_DISKTOAMIGA | JCNTRLF_DISKTOPC);
#endif
    Debug0("PC gets Disk, no Diskchange info. Byebye disk\n");
    *SwitchRegister = temp;
}
