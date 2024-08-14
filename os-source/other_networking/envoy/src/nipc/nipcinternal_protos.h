#ifndef NIPC_INTPROTOS_H
#define NIPC_INTPROTOS_H
/*
**      $Id: nipcinternal_protos.h,v 1.35 93/09/03 17:45:43 kcd Exp Locker: kcd $
**
**      Prototypes for all nipc.library internal functions.
**
**      (C) Copyright 1992 Commodore-Amiga, Inc.
**          All Rights Reserved
*/

/*------------------------------------------------------------------------*/

#ifndef NIPC_NIPCINTERNAL_H
#include "nipcinternal.h"
#endif

/*------------------------------------------------------------------------*/

/* Prototypes for functions defined in amp.c */

extern BOOL InitANMP(VOID);
extern VOID DeinitANMP(VOID);
extern BOOL __asm BeginTransaction(register __a0 struct Entity * dest_entity, register __a1 struct Entity * src_entity, register __a2 struct Transaction * trans);
extern BOOL __asm DoTransaction(register __a0 struct Entity * dest_entity, register __a1 struct Entity * src_entity, register __a2 struct Transaction * trans);
extern VOID __asm AbortTransaction(register __a1 struct Transaction * trans);
extern APTR __asm CreateEntityA(register __a0 struct TagItem * taglist);
extern VOID __asm DeleteEntity(register __a0 APTR entin);
extern VOID __asm WaitEntity(register __a0 APTR entin);
extern APTR __asm FindEntity(register __a0 UBYTE * hostname, register __a1 UBYTE * portname, register __a2 APTR src_entity, register __a3 ULONG * detailerror);
extern BOOL ask_input(struct RDPConnection *c, struct Buffer *b);
extern BOOL ask_status(struct RDPConnection *c);
extern VOID __asm LoseEntity(register __a0 APTR entin);
extern struct Transaction * __asm AllocTransactionA(register __a0 struct TagItem * taglist);
extern VOID __asm FreeTransaction(register __a1 struct Transaction * oldtrans);
extern struct Transaction * __asm GetTransaction(register __a0 APTR from_entity);
extern VOID __asm ReplyTransaction(register __a1 struct Transaction * trans);
extern VOID __asm WaitTransaction(register __a1 struct Transaction * trans);
extern BOOL __asm CheckTransaction(register __a1 struct Transaction * trans);
extern ULONG __asm PingEntity(register __a0 struct Entity *re, register __d0 ULONG limit);
extern VOID __asm SetEntityAttrsA(register __a0 struct Entity *e, register __a1 struct TagItem *);
extern ULONG __asm GetEntityAttrsA(register __a0 struct Entity *e, register __a1 struct TagItem *);
extern BOOL ResolverIn(struct RDPConnection *c, struct Buffer *buff);
extern BOOL ResolverStatus(struct RDPConnection *c);
extern BOOL AddSigBit(struct RDPConnection *c);
extern VOID DelSigBit(struct RDPConnection *c);
extern VOID Sleep(struct RDPConnection *c);
extern VOID Wake(struct RDPConnection *c);
extern VOID WaitConnect(struct RDPConnection *c);
extern VOID EntityInput(struct RDPConnection *c, struct Buffer *buff);
extern VOID REntityInput(struct RDPConnection *c, struct Buffer *buff);
extern BOOL EntityStatus(struct RDPConnection *c);
extern BOOL REntityStatus(struct RDPConnection *c);
extern BOOL __asm GetEntityName(register __a0 APTR entity, register __a1 UBYTE * newstr, register __d0 ULONG spacefree);
extern VOID DelayedDelete(struct Entity *e);
extern BOOL __asm GetHostName(register __a0 struct Entity *ent, register __a1 UBYTE *str, register __d0 ULONG availsize);
extern VOID ANMPTimeout(VOID);
extern struct Node *FindNameI(struct List *searchlist, STRPTR target);

/*------------------------------------------------------------------------*/

/* Prototypes for functions defined in udp.c */

extern BOOL UDP_Init(VOID);
extern VOID UDP_Deinit(VOID);
extern VOID UDP_Timeout(VOID);
extern VOID UDP_Input(struct Buffer *buff, ULONG length, struct sanadev *dev);
extern BOOL  __asm UDP_Output(register __a0 UBYTE *dataptr,register __d0 UWORD length, register __d1 ULONG srcip, register __d2 ULONG destip,register __d3 UWORD srcport, register __d4 UWORD destport);
extern struct UDPConnection *__asm OpenUDPPort(register __d0 UWORD localport,register __a0 UDP_DATA DataIn);
extern VOID  __asm CloseUDPPort(register __a0 struct UDPConnection *conn);

/*------------------------------------------------------------------------*/

/* Prototypes for functions defined in route.c */

extern ULONG NetNum(ULONG ipa);
extern BOOL IsBroadcast(ULONG dest);
extern BOOL IsLocalDevice(ULONG dest);
extern struct sanadev *LocalNet(ULONG DestIP);
extern BOOL NetMatch(ULONG dst, ULONG net, ULONG mask, BOOL islocal);
extern ULONG NetMask(ULONG net);
extern ULONG RtHash(ULONG net);
extern struct NIPCRoute *Route(ULONG dest, BOOL local);
extern BOOL InitRoute(VOID);
extern VOID DeinitRoute(VOID);
extern BOOL  __asm AddRoute(register __d0 ULONG network, register __d1 ULONG gateway, register __d2 UWORD hops, register __d3 WORD ttl);
extern VOID  __asm DeleteRoute(register __d0 ULONG network);

/*------------------------------------------------------------------------*/

/* Prototypes for functions defined in center_custom.c */

extern VOID __asm InitLock(register __a6 struct Library * libbase);
extern VOID __asm StartLibrary(register __a6 struct Library * libbase);
extern VOID __asm __stdargs LibraryProcess(VOID);
extern VOID ProtocolInput(struct Buffer *inpbuff, ULONG length, UWORD ptype, struct sanadev *dev);
extern VOID HeartBeat(VOID);
extern VOID KillOffNIPC(VOID);

/*------------------------------------------------------------------------*/

/* Prototypes for functions defined in arp.c */

extern BOOL InitARP(VOID);
extern VOID DeinitARP(VOID);
extern VOID SendARPRequest(struct sanadev *dev, ULONG ip_addr);
extern VOID SendARPReply(struct sanadev *dev, ULONG ip_to, UBYTE *ether_to, ULONG ip_us, UBYTE *ether_us);
extern VOID ARPInput(struct Buffer *arpbuff, ULONG length, struct sanadev *dev);
extern VOID AddARPEntry(ULONG ip, UBYTE *ether);
extern BOOL FindArpEntry(struct sanadev *dev, ULONG IP, UBYTE *etherbuff);
extern VOID DoARPSends(ULONG ipnum);
extern VOID AddBuffList(struct Buffer *b, ULONG address, struct sanadev *dev, ULONG gateway, UWORD ptype);
extern VOID arp_timeout(VOID);

/*-------------------------------------------------- ----------------------*/

/* Prototypes for functions defined in ip_in.c */

extern VOID ip_in(struct Buffer *ibuff, struct sanadev *dev);
extern VOID FragTimeoutDev(struct sanadev *dev);
extern VOID FragmentTimeout(VOID);

/*------------------------------------------------------------------------*/

/* Prototypes for functions defined in ip_out.c */

extern VOID ip_out(struct Buffer *obuff, ULONG srcip, ULONG destip, UBYTE protocol, struct sanadev *iface, ULONG gateway, ULONG frags, BOOL morefrags, UWORD inident, UBYTE ttl, BOOL dbc, BOOL fraglast);
extern VOID ip_sendsrc(struct Buffer *obuff, ULONG srcip, ULONG destip, UBYTE protocol);
extern VOID multiplex_in(struct Buffer *ibuff, struct sanadev *dev);
extern VOID multiplex(struct Buffer *obuff, ULONG headerlength, struct sanadev *dev, ULONG srcip, ULONG destip, UBYTE protocol, ULONG frags, BOOL morefrags, UWORD inident, UBYTE ttl, BOOL fraglast);

/*------------------------------------------------------------------------*/

/* Prototypes for functions defined in s2io.c */

extern char nibvert(char ichar);
extern ULONG ip_to_num(UBYTE *instr);
extern BOOL InitSana2(VOID);
extern VOID DeinitSana2(VOID);
extern VOID CheckCmdPort(VOID);
extern VOID CheckDataPort(VOID);
/* extern BOOL SwitchPacket(struct IOSana2ReqSS *ioreq); */
extern VOID PacketWrite(struct sanadev *dev, ULONG gateway, struct Buffer *buffer, ULONG destination, UWORD packettype, BOOL brc, BOOL ether);
extern VOID PacketBroadcast(struct sanadev *dev, struct Buffer *buffer, UWORD packettype);
extern VOID PollPort(VOID);
extern VOID QueueRead(struct sanadev *dev, UWORD pt);
/* extern VOID ProcessReq(struct IOSana2ReqSS *ioreq); */
extern BOOL __stdargs CTB(UBYTE *from, ULONG n, struct Buffer *to);
extern BOOL __stdargs CFB(UBYTE *to, ULONG n, struct Buffer *from);
extern VOID CheckForInet(VOID);
extern VOID NotifyInet(VOID);
extern VOID DevReqTimeout(VOID);
extern struct IORequest *ECreateIORequest(struct MsgPort *rp,ULONG size);
extern VOID EDeleteIORequest(struct IORequest *i);

/*------------------------------------------------------------------------*/

/* Prototypes for functions defined in rdp.c */

extern BOOL rdp_init(VOID);
extern VOID rdp_deinit(VOID);
extern VOID rdp_timeout(VOID);
extern VOID rdp_input(struct Buffer *buff, ULONG length, struct sanadev *dev);
extern BOOL StateMachine(struct RDPConnection *rc, UBYTE flags, struct rdphdr *packet, ULONG destip, ULONG srcip);
extern VOID SendFlags(struct RDPConnection *c, UBYTE flags);
extern VOID PacketInput(struct RDPConnection *c, struct Buffer *b);
extern UWORD FindSequential(struct RDPConnection *c);
extern struct RDPConnection *OpenActive(ULONG address, ULONG port, RDP_DATA DataIn, RDP_STATUS Status);
extern struct RDPConnection *OpenPassive(ULONG reqport, RDP_DATA DataIn, RDP_STATUS Status);
extern BOOL rdp_output(struct RDPConnection *conn, struct Buffer *b);
extern BOOL rdp_output_lame(struct RDPConnection *conn, UBYTE *dataptr, ULONG length);
extern VOID CloseConnection(struct RDPConnection *c);
extern VOID rdp_ack_timeout(VOID);

/*-- For the 64/32 math routine in math.asm ------------------------------*/
extern ULONG __asm Div64by32(register __d0 ULONG msb, register __d1 ULONG lsb, register __d2 ULONG divisor);


/*------------------------------------------------------------------------*/

/* Prototypes for functions defined in icmp.c */

extern BOOL init_icmp(VOID);
extern VOID icmp_timeout(VOID);
extern VOID icmp_deinit(VOID);
extern VOID icmp_input(struct Buffer *buff, ULONG length, struct sanadev *dev);

/*------------------------------------------------------------------------*/

/* Prototypes for functions defined in memory.c */

extern BOOL InitMemory(void);
extern void DeinitMemory(void);
extern struct BuffEntry *AllocBuffEntry(UWORD size);
extern VOID FreeBuffEntry(struct BuffEntry *buffe);
extern struct Buffer *AllocBuffer(UWORD i_size);
extern VOID __asm FreeBuffer(register __a0 struct Buffer *buff);
extern UBYTE *BuffPointer(struct Buffer *buff, ULONG offset, struct BuffEntry **buffe);
extern BOOL CopyToBuffer(struct Buffer *targetbuff, UBYTE *source, ULONG length);
extern VOID __asm CopyFromBuffer(register __a1 UBYTE *target, register __a0 struct Buffer *sourcebuff,register __d0 ULONG length);
extern ULONG __asm DataLength(register __a0 struct Buffer *buffer);
extern UWORD CalcChecksumBuffer(struct Buffer *buffer);
extern BOOL CopyBuffer(struct Buffer *ibuff, ULONG offset, ULONG length, struct Buffer *obuff);
extern struct Buffer *MergeBuffer(struct Buffer *buff1, struct Buffer *buff2);
extern struct Buffer *CloneBuffer(struct Buffer *ibuff);
extern ULONG CountEntries(struct Buffer *buffer);

/*------------------------------------------------------------------------*/

/* Prototypes for functions defined in resolver.c */

extern BOOL InitResolver(VOID);
extern VOID DeinitResolver(VOID);
extern VOID DoResolver(VOID);
extern ULONG ResolveName(UBYTE *hostname);
extern VOID ResolverInput(struct UDPConnection *conn, struct Buffer *buff,struct sanadev *dev);
extern VOID ResolverTimeout(VOID);
extern struct MsgPort * __asm DNSInit(register __d0 ULONG ServerIP);
extern void __asm DNSDeinit(void);
extern void __asm DNSTimer(void);
extern void __asm DoDNS(void);
void ClearNames(struct NameInfo *ni);
ULONG __asm RNHookFunc(register __a0 struct Hook *hook,
                      register __a2 struct Task *task,
                      register __a1 struct TagItem *tagList);
BOOL __asm NIPCInquiryA(register __a0 struct Hook *hook,
                     register __d0 ULONG maxTime,
                     register __d1 ULONG maxResponses,
                     register __a1 struct TagItem *tagList);
void QueueNameResponse(APTR buff, ULONG length, ULONG dest);

/*------------------------------------------------------------------------*/

/* Prototypes for functions defined in timer.c */

extern BOOL InitTimer(VOID);
extern VOID TimeOut(VOID);
extern VOID DeinitTimer(VOID);

/*------------------------------------------------------------------------*/

/* Prototypes for functions defined in libinit.c */

extern ULONG __saveds __asm __stdargs _LibInit(register __a0 APTR seglist, register __d0 struct MyLibrary * libbase);
extern LONG  __saveds __asm __stdargs _LibOpen(register __a6 struct MyLibrary * libbase);
extern ULONG __saveds __asm __stdargs _LibClose(register __a6 struct MyLibrary * libbase);
extern ULONG __saveds __asm __stdargs _LibExpunge(register __a6 struct MyLibrary * libbase);

/*------------------------------------------------------------------------*/

/* Prototypes for functions defined in monitor.c */

extern struct NIPCMonitorInfo * __asm StartMonitor(VOID);
extern VOID __asm StopMonitor(VOID);

/*------------------------------------------------------------------------*/

/* Prototypes for functions defined in iff.c */

extern BOOL OpenConfig(VOID);
extern BPTR PrefOpen(VOID);
extern void CloseConfig(VOID);

/*------------------------------------------------------------------------*/

/* Prototypes for functions defined in statistics.c */

extern VOID __asm StartStats(register __a0 struct stats * statptr);
extern VOID __asm EndStats(VOID);

/*------------------------------------------------------------------------*/

/* Assembly Prototypes */

extern UWORD __stdargs CalcChecksum(UBYTE *r,ULONG bytes, UWORD previous);
extern VOID NewList(struct List *list );
extern UWORD __asm makefastrand(register __d0 UWORD seed);

/*------------------------------------------------------------------------*/

/* Debug.lib Prototypes */

extern VOID __stdargs kprintf(UBYTE *fmt, ...);

/*------------------------------------------------------------------------*/

/* nipcproc.asm Prototypes */

extern ULONG __asm DivSASSucks(register __d0 ULONG a, register __d1 UWORD b);

/*------------------------------------------------------------------------*/

/* pools.asm Prototypes */

extern APTR __asm ECreatePool(register __d0 unsigned long requirements, register __d1 unsigned long puddleSize,
        register __d2 unsigned long threshSize );
extern void __asm EDeletePool( register __a0 APTR poolHeader );
extern APTR __asm EAllocPooled( register __a0 APTR poolHeader, register __d0 unsigned long memSize );
extern void __asm EFreePooled( register __a0 APTR poolHeader, register __a1 APTR memory, register __d0 unsigned long memSize );

/*------------------------------------------------------------------------*/

/* nipcbuff.c Prototypes */

extern VOID __asm FreeNIPCBuffEntry(register __a0 struct NIPCBuffEntry *entry);
extern VOID __asm FreeNIPCBuff(register __a0 struct NIPCBuff *buff);
extern __stdargs struct NIPCBuffEntry *AllocNIPCBuffEntry(VOID);
extern struct NIPCBuff * __asm AllocNIPCBuff(register __d0 ULONG entries);
extern ULONG __asm CopyNIPCBuff(register __a0 struct NIPCBuff *srcbuff,
			 register __a1 struct NIPCBuff *dstbuff,
			 register __d0 ULONG srcoffset,
			 register __d1 ULONG dstoffset,
			 register __d2 ULONG length);
extern ULONG __asm CopyToNIPCBuff(register __a0 UBYTE *src,
			   register __a1 struct NIPCBuff *dstbuff,
			   register __d0 ULONG dstoffset,
			   register __d1 ULONG length);
extern ULONG __asm CopyFromNIPCBuff(register __a0 struct NIPCBuff *srcbuff,
			     register __a1 UBYTE *dst,
			     register __d0 ULONG srcoffset,
			     register __d1 ULONG length);
extern ULONG __asm NIPCBuffLength(register __a0 struct NIPCBuff *buffer);
extern UBYTE * __asm NIPCBuffPointer(register __a0 struct NIPCBuff *buff,
			      register __d0 ULONG offset,
			      register __a1 struct NIPCBuffEntry **entry);
extern struct NIPCBuff * __asm AppendNIPCBuff(register __a0 struct NIPCBuff *first,
			               register __a1 struct NIPCBuff *second);


#endif  /* NIPC_INTPROTOS_H */
