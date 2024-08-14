€ˆWORKBENCH_WORKBENCH_H€WORKBENCH_WORKBENCH_HˆEXEC_TYPES_HŒ"exec/types.h"‡ˆµŒ"exec/nodes.h"‡ˆ·Œ"exec/lists.h"‡ˆEXEC_TASKS_HŒ"exec/tasks.h"‡ˆINTUITION_INTUITION_HŒ"intuition/intuition.h"‡€WBDISK 1€WBDRAWER 2€WBTOOL 3€WBPROJECT 4€WBGARBAGE 5€WBDEVICE 6€WBKICK 7
ƒDrawerData{
ƒNewWindow dd_NewWindow;
’dd_CurrentX;
’dd_CurrentY;
};€DRAWERDATAFILESIZE (sizeof(ƒDrawerData))
ƒDiskObject{
‰do_Magic;
‰do_Version;
ƒ»do_Gadget;
Šdo_Type;
„*do_DefaultTool;
„**do_ToolTypes;
’do_CurrentX;
’do_CurrentY;
ƒDrawerData*do_DrawerData;
„*do_ToolWindow;
’do_StackSize;
};€WB_DISKMAGIC 0xe310€WB_DISKVERSION 1
ƒFreeList{
˜fl_NumFree;
ƒ®fl_MemList;
};€MTYPE_PSTD 1€MTYPE_TOOLEXIT 2€MTYPE_DISKCHANGE 3€MTYPE_TIMER 4€MTYPE_CLOSEDOWN 5€MTYPE_IOPROC 6€GADGBACKFILL 1€NO_ICON_POSITION (0x80000000)‡