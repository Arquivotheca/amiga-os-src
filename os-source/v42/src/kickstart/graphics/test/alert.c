#include <exec/types.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <stdio.h>

UBYTE *alertMsg =
    "\x00\xF0\x10OH NO, NOT AGAIN!\x00\x01"
    "\x00\xA0\x20PRESS LEFT MOUSE BUTTON TO CONTINUE.\x00\x00";

struct IntuitionBase *IntuitionBase;

VOID main(int argc, char **argv)
{
if (NULL != (IntuitionBase =
    (struct IntuitionBase *)OpenLibrary("intuition.library",33)))
    {
    if (DisplayAlert(RECOVERY_ALERT, alertMsg, 52))
        printf("Alert returned TRUE\n");
    else
        printf("Alert returned FALSE\n");
    CloseLibrary(IntuitionBase);
    }
}
