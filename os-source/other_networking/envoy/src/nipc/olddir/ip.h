
#include <exec/types.h>
#include <exec/nodes.h>

 typedef unsigned int u_int;

#define IPFLAG_MOREFRAGMENTS 1

/* For building and splitting those wonderful IP packets together and
 * into pieces.
 */

struct iphdr
   {
   u_int    iph_Version:4;
   u_int    iph_IHL:4;
   u_int    iph_TypeOfService:8;
   u_int    iph_Length:16;
   u_int    iph_Ident:16;
   u_int    iph_Flags:3;
   u_int    iph_FragmentOffset:13;
   u_int    iph_TimeToLive:8;
   u_int    iph_Protocol:8;
   u_int    iph_Checksum:16;
   ULONG    iph_SrcAddr;
   ULONG    iph_DestAddr;

   };

/* The structure that IP uses to keep track of all of the
 * protocols that sit on top of IP
 */

struct ipproto
   {
   struct MinNode ipproto_link;
   UWORD    ipproto_ProtocolNumber;
   void     (*ipproto_Input)();
   void     (*ipproto_Deinit)();
   void     (*ipproto_Timeout)();
   };

