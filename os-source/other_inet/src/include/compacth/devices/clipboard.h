€ˆDEVICES_CLIPBOARD_H€DEVICES_CLIPBOARD_HˆµŒ"exec/nodes.h"‡ˆ·Œ"exec/lists.h"‡ˆ¸Œ"exec/ports.h"‡€CBD_POST (CMD_NONSTD+0)€CBD_CURRENTREADID (CMD_NONSTD+1)€CBD_CURRENTWRITEID (CMD_NONSTD+2)€CBERR_OBSOLETEID 1
ƒClipboardUnitPartial{
ƒ¬cu_Node;
—cu_UnitNum;
};
ƒIOClipReq{
ƒ¯io_Message;
ƒDevice*io_Device;
ƒUnit*io_Unit;
‰io_Command;
Šio_Flags;
šio_Error;
—io_Actual;
—io_Length;
STRPTR io_Data;
—io_Offset;
’io_ClipID;
};€PRIMARY_CLIP 0
ƒSatisfyMsg{
ƒ¯sm_Msg;
‰sm_Unit;
’sm_ClipID;
};‡