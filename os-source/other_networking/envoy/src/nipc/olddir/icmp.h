
 struct icmphdrshort
   {
   UBYTE        icmps_type;
   UBYTE        icmps_code;
   UWORD        icmps_checksum;
   UWORD        icmps_ident;
   UWORD        icmps_seqnum;
   };

 struct icmphdr
   {
   struct iphdr icmp_iphdr;
   struct icmphdrshort icmp_icmp;
   };

#define ICMP_ECHOREPLY       0
#define ICMP_DESTUNREACH     3
#define ICMP_SRCQUENCH       4
#define ICMP_REDIRECT        5
#define ICMP_ECHO            8
#define ICMP_TIMESTAMP      13
#define ICMP_TIMESTAMPREPLY 14
#define ICMP_INFOREQUEST    15
#define ICMP_INFOREPLY      16


