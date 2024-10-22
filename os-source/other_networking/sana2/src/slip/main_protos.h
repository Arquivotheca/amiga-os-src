/* Prototypes for functions defined in
main.c
 */

ULONG __asm DevOpen(register __a1 struct IOSana2Req * , register __d0 ULONG , register __d1 ULONG , register __a6 struct SLIPDevice * );

BPTR __asm DevClose(register __a1 struct IOSana2Req * , register __a6 struct SLIPDevice * );

BPTR __asm DevExpunge(register __a6 struct SLIPDevice * );

struct SLIPUnit * InitSLIPUnit(ULONG , struct SLIPDevice * );

void ExpungeUnit(struct SLIPUnit * , struct SLIPDevice * );

BOOL ReadConfig(struct SLIPUnit * , struct SLIPDevice * );

void __asm DevBeginIO(register __a1 struct IOSana2Req * , register __a6 struct SLIPDevice * );

void PerformIO(struct IOSana2Req * , struct SLIPDevice * );

void __asm GetSpecialStats(register __a2 struct IOSana2Req * , register __a4 struct SLIPUnit * , register __a6 struct SLIPDevice * );

void __asm GetGlobalStats(register __a2 struct IOSana2Req * , register __a4 struct SLIPUnit * , register __a6 struct SLIPDevice * );

void __asm GetTypeStats(register __a2 struct IOSana2Req * , register __a4 struct SLIPUnit * , register __a6 struct SLIPDevice * );

void __asm TrackType(register __a2 struct IOSana2Req * , register __a4 struct SLIPUnit * , register __a6 struct SLIPDevice * );

void __asm UnTrackType(register __a2 struct IOSana2Req * , register __a4 struct SLIPUnit * , register __a6 struct SLIPDevice * );

void PacketReceived(ULONG , struct SLIPUnit * );

void PacketSent(ULONG , struct SLIPUnit * );

void PacketOverrun(struct SLIPUnit * , struct SLIPDevice * );

void ReceivedGarbage(struct SLIPUnit * , struct SLIPDevice * );

void PacketDropped(struct SLIPUnit * , struct SLIPDevice * );

void __asm TermIO(register __a2 struct IOSana2Req * , register __a6 struct SLIPDevice * );

ULONG __asm DevAbortIO(register __a1 struct IOSana2Req * , register __a6 struct SLIPDevice * );

ULONG AbortReq(struct MinList * , struct IOSana2Req * , struct SLIPDevice * );

void __asm ConfigInterface(register __a2 struct IOSana2Req * , register __a4 struct SLIPUnit * , register __a6 struct SLIPDevice * );

void __asm GetStationAddress(register __a2 struct IOSana2Req * , register __a4 struct SLIPUnit * , register __a6 struct SLIPDevice * );

extern struct DQInfo DQTable[9];

void __asm DeviceQuery(register __a2 struct IOSana2Req * , register __a4 struct SLIPUnit * , register __a6 struct SLIPDevice * );

void __asm WritePacket(register __a2 struct IOSana2Req * , register __a4 struct SLIPUnit * , register __a6 struct SLIPDevice * );

void __asm SendPacket(register __a2 struct IOSana2Req * , register __a4 struct SLIPUnit * , register __a6 struct SLIPDevice * );

ULONG EncodeSLIP(UBYTE * , UBYTE * , ULONG );

void __asm ReadPacket(register __a2 struct IOSana2Req * , register __a4 struct SLIPUnit * , register __a6 struct SLIPDevice * );

BOOL InitSerial(struct SLIPUnit * , struct SLIPDevice * );

void DeinitSerial(struct SLIPUnit * , struct SLIPDevice * );

void __asm OnEvent(register __a2 struct IOSana2Req * , register __a4 struct SLIPUnit * , register __a6 struct SLIPDevice * );

BOOL OpenSerial(struct SLIPUnit * , struct SLIPDevice * );

void MarkTimeOnline(struct SLIPUnit * , struct SLIPDevice * );

void CloseSerial(struct SLIPUnit * , struct SLIPDevice * );

void __asm Online(register __a2 struct IOSana2Req * , register __a4 struct SLIPUnit * , register __a6 struct SLIPDevice * );

void __asm Offline(register __a2 struct IOSana2Req * , register __a4 struct SLIPUnit * , register __a6 struct SLIPDevice * );

void DoEvent(ULONG , struct SLIPUnit * , struct SLIPDevice * );

void DoOffline(struct SLIPUnit * , struct SLIPDevice * );

void ServiceTxPort(struct SLIPUnit * , struct SLIPDevice * );

void DoSerial(struct IOExtSer * , struct SLIPUnit * , struct SLIPDevice * );

void GotPacket(ULONG , struct SLIPUnit * , struct SLIPDevice * );

void QueueSerRequest(struct SLIPUnit * , struct SLIPDevice * );

void __asm DevProcCEntry(register __a6 struct SLIPDevice * );

void NewList(struct List * );

