#include "exec/types.h"
#include "exec/io.h"

/* C version of SetIO */
SetIO(iob, command, data, length, offset)
struct IOStdReq *iob;
UWORD command;
APTR data;
ULONG length;
ULONG offset;
{
    iob->io_Command = command;
    iob->io_Data = data;
    iob->io_Length = length;
    iob->io_Offset = offset;
}
