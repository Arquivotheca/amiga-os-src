€ˆLIBRARIES_EXPANSIONBASE_H€LIBRARIES_EXPANSIONBASE_HˆEXEC_TYPES_HŒ"exec/types.h"‡ˆEXEC_LIBRARIES_HŒ"exec/libraries.h"‡ˆEXEC_INTERRUPTS_HŒ"exec/interrupts.h"‡ˆEXEC_SEMAPHORES_HŒ"exec/semaphores.h"‡ˆLIBRARIES_CONFIGVARS_HŒ"libraries/configvars.h"‡€TOTALSLOTS 256
ƒExpansionInt
{
‰IntMask;
‰ArrayMax;
‰ArraySize;
};
ƒ­
{
ƒLibrary LibNode;
Š¦;
Špad;
”ExecBase;
”SegList;
ƒCurrentBinding CurrentBinding;
ƒ®BoardList;
ƒ®MountList;
ŠAllocTable[TOTALSLOTS];
ƒSignalSemaphore BindSemaphore;
ƒInterrupt Int2List;
ƒInterrupt Int6List;
ƒInterrupt Int7List;
};‡