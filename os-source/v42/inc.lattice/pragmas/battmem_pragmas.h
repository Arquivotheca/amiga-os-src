/* "battmem.resource" */
#pragma libcall BattMemBase ObtainBattSemaphore 6 00
#pragma libcall BattMemBase ReleaseBattSemaphore c 00
#pragma libcall BattMemBase ReadBattMem 12 10803
#pragma libcall BattMemBase WriteBattMem 18 10803
