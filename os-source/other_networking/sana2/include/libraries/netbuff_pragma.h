#pragma libcall NetBuffBase IntAllocSegments  1E 8002
#pragma libcall NetBuffBase AllocSegments  24 8002
#pragma libcall NetBuffBase IntFreeSegments	 2A 801
#pragma libcall NetBuffBase FreeSegments  30 801
#pragma libcall NetBuffBase SplitNetBuff  36 90803
#pragma libcall NetBuffBase TrimNetBuff  3C 0802
#pragma libcall NetBuffBase CopyToNetBuff  42 190804
#pragma libcall NetBuffBase CopyFromNetBuff  48 190804
#pragma libcall NetBuffBase CopyNetBuff  4E 9802
#pragma libcall NetBuffBase CompactNetBuff  54 801
#pragma libcall NetBuffBase ReadyNetBuff  5A 0802
#pragma libcall NetBuffBase IsContiguous  60 10803
#pragma libcall NetBuffBase NetBuffAppend  66 9802
#pragma libcall NetBuffBase PrependNetBuff  6C 9802