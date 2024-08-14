/*****************************************************************************
    DosFix.h
    
    Mitchell/Ware Systems           Version 1.00            08-Dec-88
    
    Extra structures not present in dosextens.h
*****************************************************************************/

#include <libraries/dos.h>
#include <libraries/dosextens.h>

typedef union  {   /* Device Info list, union of the 2 types */
    struct DevInfo dev;
    struct DeviceList vol;
    } DevInfo;
    
    