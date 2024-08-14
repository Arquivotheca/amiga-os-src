/**/
/* MyLib test library fd file*/
/**/
/**/
/*  These private functions allow a driver to share slots for both a general*/
/*  device driver combined with a network handler.*/
/**/
/*pragma libcall DriverBase private 1e 0*/
/*pragma libcall DriverBase private 24 0*/
/*pragma libcall DriverBase private 2a 0*/
/*pragma libcall DriverBase private 30 0*/
/*pragma libcall DriverBase private 36 0*/
/*pragma libcall DriverBase private 3c 0*/
#pragma libcall DriverBase SDNSerInit 42 a9803
#pragma libcall DriverBase SDNSerTerm 48 801
#pragma libcall DriverBase SDNHanInit 4e a9803
#pragma libcall DriverBase SDNHanTerm 54 801
#pragma libcall DriverBase SDNInitNode 5a a9803
#pragma libcall DriverBase SDNTermNode 60 9802
#pragma libcall DriverBase SDNAccept 66 9802
#pragma libcall DriverBase SDNReply 6c 9802
#pragma libcall DriverBase SDNSend 72 9802
#pragma libcall DriverBase SDNReceive 78 90803
#pragma libcall DriverBase SDNAllocRPacket 7e 9803
#pragma libcall DriverBase SDNDupRPacket 84 9802
#pragma libcall DriverBase SDNFreeRPacket 8a 9802
#pragma libcall DriverBase SDNGetConData 90 9802
#pragma libcall DriverBase SDNSetConData 96 a9803
