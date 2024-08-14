/*
** inet.library export definitions
*/

#ifndef NETINET_INET_H
#define NETINET_INET_H

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif
#ifndef EXEC_LISTS_H
#include <exec/lists.h>
#endif
#ifndef EXEC_LIBRARIES_H
#include <exec/libraries.h>
#endif
#ifndef EXEC_IO_H
#include <exec/io.h>
#endif

struct InetNode
{
    struct  Library lib;
    short   nlsize;             /* size of nlist array		*/
    struct nlist *names;        /* nlist entries		*/
    ULONG   ml_SegList;
    APTR    ml_ExecBase;        /* pointer to exec base         */
    LONG    ml_Data;            /* Global data                  */

};

extern struct InetNode *InetBase;
#define INETNAME	"inet.library"
#endif


#ifdef undef
	{"_mbstat",	MBSTAT},	{"_ipstat",	IPSTAT},
	{"_tcb",	TCB},		{"_tcpstat",	TCPSTAT},
	{"_udp",	UDB},		{"_udpstat",	UDPSTAT},
	{"_rawcb",	RAWCB},		{"_ifnet",	IFNET},
	{"imp_softc",	IMP_SOFTC},	{"_rthost",	RTHOST},
	{"_rtnet",	RTNET},		{"_icmpstat",	ICMPSTAT},
	{"_rtsat",	RTSTAT},	{"_nfile",	NFILE},
	{"_file",	FILEV},		{"_unixsw",	UNIXSW},
	{"_rthashsize", RTHASHSIZE},	{"_nspcb",	NSPCB},
	{"_idpstat",	IDPSTAT},	{"_spp_istat",	SPP_ISTAT},
	{"_ns_errstat",	NS_ERRSTAT},	{"_nimp",	NIMP},
	{"_arptab",	ARPTAB},	{"_arptab_size",ARPTAB_SIZE},
#endif
