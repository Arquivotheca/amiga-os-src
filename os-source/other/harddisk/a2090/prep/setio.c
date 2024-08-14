#include "exec/types.h"
#include "exec/io.h"

/* C version of SetIO */
SetIO(iob, command, data, length, offset, unit, device)
struct IOStdReq *iob;
UWORD command;
APTR data;
ULONG length;
ULONG offset;
struct Unit *unit;
struct Device *device;
{
    iob->io_Command = command;
    iob->io_Data = data;
    iob->io_Length = length;
    iob->io_Offset = offset;
    iob->io_Unit = unit;
    iob->io_Device = device;
}
