��WORKBENCH_WORKBENCH_H�WORKBENCH_WORKBENCH_H�EXEC_TYPES_H�"exec/types.h"����"exec/nodes.h"����"exec/lists.h"��EXEC_TASKS_H�"exec/tasks.h"��INTUITION_INTUITION_H�"intuition/intuition.h"��WBDISK 1�WBDRAWER 2�WBTOOL 3�WBPROJECT 4�WBGARBAGE 5�WBDEVICE 6�WBKICK 7
�DrawerData{
�NewWindow dd_NewWindow;
�dd_CurrentX;
�dd_CurrentY;
};�DRAWERDATAFILESIZE (sizeof(�DrawerData))
�DiskObject{
�do_Magic;
�do_Version;
��do_Gadget;
�do_Type;
�*do_DefaultTool;
�**do_ToolTypes;
�do_CurrentX;
�do_CurrentY;
�DrawerData*do_DrawerData;
�*do_ToolWindow;
�do_StackSize;
};�WB_DISKMAGIC 0xe310�WB_DISKVERSION 1
�FreeList{
�fl_NumFree;
��fl_MemList;
};�MTYPE_PSTD 1�MTYPE_TOOLEXIT 2�MTYPE_DISKCHANGE 3�MTYPE_TIMER 4�MTYPE_CLOSEDOWN 5�MTYPE_IOPROC 6�GADGBACKFILL 1�NO_ICON_POSITION (0x80000000)�