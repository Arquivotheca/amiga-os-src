��DEVICES_PRTBASE_H�DEVICES_PRTBASE_H���"exec/nodes.h"����"exec/lists.h"����"exec/ports.h"��EXEC_LIBRARIES_H�"exec/libraries.h"��EXEC_TASKS_H�"exec/tasks.h"��DEVICES_PARALLEL_H�"devices/parallel.h"��DEVICES_SERIAL_H�"devices/serial.h"��DEVICES_TIMER_H�"devices/timer.h"��LIBRARIES_DOSEXTENS_I�"libraries/dosextens.h"��INTUITION_INTUITION_H�"intuition/intuition.h"�
�DeviceData{
�Library dd_Device;
�dd_Segment;
�dd_ExecBase;
�dd_CmdVectors;
�dd_CmdBytes;
�dd_NumCommands;
};�P_STKSIZE 0x800�P_BUFSIZE 256�P_SAFESIZE 128
�PrinterData{
�DeviceData pd_Device;
��pd_Unit;
�pd_PrinterSegment;
�pd_PrinterType;
�PrinterSegment*pd_SegmentData;
�*pd_PrintBuf;
�(*pd_PWrite)();
�(*pd_PBothReady)();
�{
�IOExtPar pd_p0;
�IOExtSer pd_s0;
}pd_ior0;�pd_PIOR0 pd_ior0.pd_p0�pd_SIOR0 pd_ior0.pd_s0
�{
�IOExtPar pd_p1;
�IOExtSer pd_s1;
}pd_ior1;�pd_PIOR1 pd_ior1.pd_p1�pd_SIOR1 pd_ior1.pd_s1
�timerequest pd_TIOR;
��pd_IORPort;
�Task pd_TC;
�pd_Stk[P_STKSIZE];
�pd_Flags;
�pd_pad;
�Preferences pd_Preferences;
�pd_PWaitEnabled;
};�PPCB_GFX 0�PPCF_GFX 1�PPCB_COLOR 1�PPCF_COLOR 2�PPC_BWALPHA 0�PPC_BWGFX 1�PPC_COLORALPHA 2�PPC_COLORGFX 3�PCC_BW 1�PCC_YMC 2�PCC_YMC_BW 3�PCC_YMCB 4�PCC_4COLOR 4�PCC_ADDITIVE 8�PCC_WB 9�PCC_BGR 10�PCC_BGR_WB 11�PCC_BGRW 12�PCC_MULTI_PASS 16
�PrinterExtendedData{
�*ped_PrinterName;
VOID(*ped_Init)();
VOID(*ped_Expunge)();
�(*ped_Open)();
VOID(*ped_Close)();
�ped_PrinterClass;
�ped_ColorClass;
�ped_MaxColumns;
�ped_NumCharSets;
�ped_NumRows;
�ped_MaxXDots;
�ped_MaxYDots;
�ped_XDotsInch;
�ped_YDotsInch;
�***ped_Commands;
�(*ped_DoSpecial)();
�(*ped_Render)();
�ped_TimeoutSecs;
�**ped_8BitChars;
�ped_PrintMode;
�(*ped_ConvFunc)();
};
�PrinterSegment{
�ps_NextSegment;
�ps_runAlert;
�ps_Version;
�ps_Revision;
�PrinterExtendedData ps_PED;
};�