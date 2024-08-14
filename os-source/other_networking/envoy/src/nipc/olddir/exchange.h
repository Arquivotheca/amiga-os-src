
#include <exec/ports.h>
#include <exec/devices.h>

struct xstart
   {
   struct Message   xs_Msg;
   BYTE             xs_Command;
   BYTE             xs_Filler;
   struct Device   *xs_Device;
   struct Unit     *xs_Unit;
   BOOL            (*xs_CTB)();
   struct MsgPort  *xs_Link;
   };

#define XCMD_START 0
#define XCMD_END 1



