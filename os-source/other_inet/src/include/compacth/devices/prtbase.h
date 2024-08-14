€ˆDEVICES_PRTBASE_H€DEVICES_PRTBASE_HˆµŒ"exec/nodes.h"‡ˆ·Œ"exec/lists.h"‡ˆ¸Œ"exec/ports.h"‡ˆEXEC_LIBRARIES_HŒ"exec/libraries.h"‡ˆEXEC_TASKS_HŒ"exec/tasks.h"‡ˆDEVICES_PARALLEL_HŒ"devices/parallel.h"‡ˆDEVICES_SERIAL_HŒ"devices/serial.h"‡ˆDEVICES_TIMER_HŒ"devices/timer.h"‡ˆLIBRARIES_DOSEXTENS_IŒ"libraries/dosextens.h"‡ˆINTUITION_INTUITION_HŒ"intuition/intuition.h"‡
ƒDeviceData{
ƒLibrary dd_Device;
”dd_Segment;
”dd_ExecBase;
”dd_CmdVectors;
”dd_CmdBytes;
‰dd_NumCommands;
};€P_STKSIZE 0x800€P_BUFSIZE 256€P_SAFESIZE 128
ƒPrinterData{
ƒDeviceData pd_Device;
ƒ©pd_Unit;
¡pd_PrinterSegment;
‰pd_PrinterType;
ƒPrinterSegment*pd_SegmentData;
Š*pd_PrintBuf;
‚(*pd_PWrite)();
‚(*pd_PBothReady)();
«{
ƒIOExtPar pd_p0;
ƒIOExtSer pd_s0;
}pd_ior0;€pd_PIOR0 pd_ior0.pd_p0€pd_SIOR0 pd_ior0.pd_s0
«{
ƒIOExtPar pd_p1;
ƒIOExtSer pd_s1;
}pd_ior1;€pd_PIOR1 pd_ior1.pd_p1€pd_SIOR1 pd_ior1.pd_s1
ƒtimerequest pd_TIOR;
ƒ©pd_IORPort;
ƒTask pd_TC;
Špd_Stk[P_STKSIZE];
Špd_Flags;
Špd_pad;
ƒPreferences pd_Preferences;
Špd_PWaitEnabled;
};€PPCB_GFX 0€PPCF_GFX 1€PPCB_COLOR 1€PPCF_COLOR 2€PPC_BWALPHA 0€PPC_BWGFX 1€PPC_COLORALPHA 2€PPC_COLORGFX 3€PCC_BW 1€PCC_YMC 2€PCC_YMC_BW 3€PCC_YMCB 4€PCC_4COLOR 4€PCC_ADDITIVE 8€PCC_WB 9€PCC_BGR 10€PCC_BGR_WB 11€PCC_BGRW 12€PCC_MULTI_PASS 16
ƒPrinterExtendedData{
„*ped_PrinterName;
VOID(*ped_Init)();
VOID(*ped_Expunge)();
‚(*ped_Open)();
VOID(*ped_Close)();
Šped_PrinterClass;
Šped_ColorClass;
Šped_MaxColumns;
Šped_NumCharSets;
‰ped_NumRows;
—ped_MaxXDots;
—ped_MaxYDots;
‰ped_XDotsInch;
‰ped_YDotsInch;
„***ped_Commands;
‚(*ped_DoSpecial)();
‚(*ped_Render)();
’ped_TimeoutSecs;
„**ped_8BitChars;
’ped_PrintMode;
‚(*ped_ConvFunc)();
};
ƒPrinterSegment{
—ps_NextSegment;
—ps_runAlert;
‰ps_Version;
‰ps_Revision;
ƒPrinterExtendedData ps_PED;
};‡