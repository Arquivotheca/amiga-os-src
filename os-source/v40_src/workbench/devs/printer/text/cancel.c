#include	"exec/types.h"
#include	"exec/nodes.h"
#include	"exec/lists.h"
#include	"exec/interrupts.h"
#include	"exec/memory.h"
#include	"exec/ports.h"
#include	"exec/tasks.h"
#include	"exec/libraries.h"
#include	"exec/devices.h"
#include	"exec/io.h"

#include	"../printer/printer.h"

main()
{
    int error, signal;

    signal = AllocSignal(-1);
    printf("pOpen(%ld): %ld\n", signal, pOpen(signal));
    error = pWrite("Cancel Test.\n");
    printf("pWrite(\"Cancel Test.\\n\"): %ld\n", error);
    pClose();
    FreeSignal(signal);
}
