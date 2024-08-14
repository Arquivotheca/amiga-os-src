
#include <stdio.h>
#include <exec/execbase.h>

#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>

#include "setpatchcd_rev.h"

extern struct ExecBase *SysBase;

char *VTAG = VERSTAG;

void main(void) {

struct IntVector *iv = &SysBase->IntVects[3];
struct Node      *cd;
struct Library   *Lib;

    Forbid();                                                                                   // Disable task switching

    if (Lib = (struct Library *)FindName((struct List *)&SysBase->DeviceList, "cd.device")) {   // Find cd.device

        if ((Lib->lib_Version <= 40) && (Lib->lib_Revision <= 17)) {                            // Check the version/revision

            if (cd = FindName((struct List *)iv->iv_Data, "cd.device")) {                       // Find first cd.device interrupt

                if (cd = FindName((struct List *)cd, "cd.device")) {                            // Find second cd.device interrupt

                    Disable();                                                                  // Remove the interrupt
                    Remove(cd);
                    Enable();

                    printf("cd.device has now been patched\n");
                    }
                }
            }
        }

    Permit();                                                                                   // Reenable task switching
    }

