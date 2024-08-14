
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/devices.h>


/* Packet Types for PacketWrite */
#define PACKET_IP  1
#define PACKET_ARP 2

/* internal structure for keeping track of SANA II devices */
struct sanadev
   {
   struct MinNode  sanadev_Node;
   struct Device   *sanadev_Device;
   struct Unit     *sanadev_Unit;
   void            *sanadev_BuffMan;
   struct MinList  sanadev_Fragments;
   ULONG           sanadev_MTU;
   long            sanadev_HardwareType;
   UWORD           sanadev_AddressSize;
   ULONG           sanadev_BPS;
   ULONG           sanadev_IPType;
   ULONG           sanadev_ARPType;
   UWORD           sanadev_Flags;
   UBYTE           sanadev_HardAddress[16];
/* pieces of the structure for things other than the lowest level */
   ULONG           sanadev_IPAddress;
   ULONG           sanadev_IPNetwork;
   ULONG           sanadev_IPSubnet;
   ULONG           sanadev_IPMask;
   ULONG           sanadev_SubnetMask;
   BOOL            sanadev_SValid;
   };

#define S2DB_USEARP  0
#define S2DF_USEARP  1
#define S2DB_OFFLINE 1
#define S2DF_OFFLINE 2
#define S2DB_WAITING 2
#define S2DF_WAITING 4

