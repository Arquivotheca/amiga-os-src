/*** route.c **************************************************************
*
*   route.c     - Routines for IP Route management
*
*   Copyright 1992, Commodore-Amiga, Inc.
*
*   $Id: route.c,v 1.14 92/05/08 14:32:55 kcd Exp $
*
**************************************************************************/

/*------------------------------------------------------------------------*/

#include "nipcinternal.h"

#include <utility/tagitem.h>
#include <dos/rdargs.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/iffparse_protos.h>

#include "nipcinternal_protos.h"

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/iffparse_pragmas.h>
#include "externs.h"

/*------------------------------------------------------------------------*/

ULONG NetNum(ipa)
ULONG ipa;
{
    ULONG net;

    if (IP_CLASSA(ipa)) net=ipa & 0xff000000;
    if (IP_CLASSB(ipa)) net=ipa & 0xffff0000;
    if (IP_CLASSC(ipa)) net=ipa & 0xffffff00;
    if (IP_CLASSD(ipa)) net=ipa;
    if (IP_CLASSE(ipa)) net=ipa;
    return net;
}

/*------------------------------------------------------------------------*/

BOOL IsBroadcast(dest)
ULONG dest;
{
    struct sanadev *iface;

    if((dest == 0x00000000L) ||
       (dest == 0xffffffffL))
        return TRUE;

    iface = (struct sanadev *) gb->DeviceList.lh_Head;

    while(iface->sanadev_Node.mln_Succ)
    {
        if(dest == (iface->sanadev_IPAddress | ~iface->sanadev_IPMask))
            return TRUE;
        if(iface->sanadev_SValid)
        {
            if(dest == (iface->sanadev_IPSubnet | ~iface->sanadev_SubnetMask))
                return TRUE;
        }

        iface = (struct sanadev *) iface->sanadev_Node.mln_Succ;
    }

    return FALSE;
}

/*------------------------------------------------------------------------*/

struct sanadev *LocalNet(DestIP)
ULONG DestIP;
{
    struct sanadev *iface;

    iface = (struct sanadev *) gb->DeviceList.lh_Head;

    while(iface->sanadev_Node.mln_Succ)
    {
        if(iface->sanadev_SValid)
        {
            if((DestIP & iface->sanadev_SubnetMask) == iface->sanadev_IPSubnet)
                return iface;
        }
        else if((DestIP & iface->sanadev_IPMask) == iface->sanadev_IPNetwork)
            return iface;

        iface = (struct sanadev *) iface->sanadev_Node.mln_Succ;
    }

    return NULL;
}

/*------------------------------------------------------------------------*/

BOOL IsLocalDevice(DestIP)
ULONG DestIP;
{
    struct sanadev *iface;

    iface = (struct sanadev *) gb->DeviceList.lh_Head;

    while(iface->sanadev_Node.mln_Succ)
    {
        if(DestIP == iface->sanadev_IPAddress)
            return TRUE;
        iface = (struct sanadev *) iface->sanadev_Node.mln_Succ;
    }
    return FALSE;
}

/*------------------------------------------------------------------------*/

BOOL NetMatch(dst, net, mask, islocal)
ULONG dst, net, mask;
BOOL islocal;
{
    if ((mask & dst) != net)
        return FALSE;

/*    if(islocal) */
        if(IsBroadcast(dst))
            return !(mask == 0xffffffff);

    return TRUE;
}

/*------------------------------------------------------------------------*/

ULONG NetMask(net)
ULONG net;
{
    ULONG netpart;
    struct sanadev *iface;

    if(!net)
        return 0L;

    netpart=NetNum(net);

    iface = (struct sanadev *) gb->DeviceList.lh_Head;

    while(iface->sanadev_Node.mln_Succ)
    {
        if(iface->sanadev_SValid && (netpart == iface->sanadev_IPNetwork))
            return iface->sanadev_SubnetMask;

        iface = (struct sanadev *) iface->sanadev_Node.mln_Succ;
    }

    if (IP_CLASSA(net)) return 0xff000000;
    if (IP_CLASSB(net)) return 0xffff0000;
    if (IP_CLASSC(net)) return 0xffffff00;
    if (IP_CLASSD(net)) return 0xffffffff;
    if (IP_CLASSE(net)) return 0xffffffff;

}

/*------------------------------------------------------------------------*/

ULONG RtHash(net)
ULONG net;
{
    ULONG hv;
    UBYTE nt[4], bc;
    hv=0;

    nt[0]=net>>24;
    nt[1]=(net & 0x00ff0000)>>16;
    nt[2]=(net & 0x0000ff00)>>8;
    nt[3]=(net * 0x000000ff);

    if(IP_CLASSA(net)) bc=1;
    if(IP_CLASSB(net)) bc=2;
    if(IP_CLASSC(net)) bc=3;
    if(IP_CLASSD(net)) bc=4;
    if(IP_CLASSE(net)) bc=4;

    while(bc--)
        hv += nt[bc] & 0xff;

    return hv % MAX_ROUTES;
}

/*------------------------------------------------------------------------*/

struct NIPCRoute *Route(dest, local)
ULONG dest;
BOOL local;
{
    register struct NBase *nb = gb;
    struct NIPCRoute *prt;
    struct NIPCRouteInfo *nri;
    struct MinList *tmp;
    ULONG hv;

    nri = (struct NIPCRouteInfo *) &gb->RouteInfo;

    ObtainSemaphore(&nb->RouteInfo.nri_Lock);

    hv = RtHash(dest);
    tmp = (struct MinList *) &nri->nri_Routes[hv];
    for (prt = (struct NIPCRoute *) tmp->mlh_Head; prt->nr_Node.mln_Succ; prt=(struct NIPCRoute *) prt->nr_Node.mln_Succ)
        if(NetMatch(dest,prt->nr_Network,prt->nr_Mask,local))
            break;

    if(prt->nr_Node.mln_Succ == 0)
        prt = gb->RouteInfo.nri_Default;

    if(prt)
    {
        prt->nr_RefCnt++;
        prt->nr_UseCnt++;
    }
    ReleaseSemaphore(&nb->RouteInfo.nri_Lock);
    return prt;
}

/*------------------------------------------------------------------------*/

/* This routine will need to be changed once we have decided on a format
** for the routing information.
*/

BOOL InitRoute()
{
    register struct NBase *nb = gb;
    struct NIPCRoutePrefs *iffroute;
    struct CollectionItem *ci;
    struct sanadev *dev;

    struct NIPCRouteInfo *nri;
    int i;

    nri = &gb->RouteInfo;
    for(i = 0;i < MAX_ROUTES; i++)
        NewList((struct List *)&nri->nri_Routes[i]);

    InitSemaphore(&nri->nri_Lock);

    dev = (struct sanadev *) gb->DeviceList.lh_Head;
    while(dev->sanadev_Node.mln_Succ)
    {
        if(dev->sanadev_SValid)
            AddRoute(dev->sanadev_IPSubnet,dev->sanadev_IPAddress,0,-1);
        else
            AddRoute(dev->sanadev_IPNetwork,dev->sanadev_IPAddress,0,-1);

        dev = (struct sanadev *)dev->sanadev_Node.mln_Succ;
    }

    if(ci = FindCollection(nb->iff,ID_PREF,ID_NIRT))
    {
        while(ci)
        {
            iffroute = (struct NIPCRoutePrefs *)ci->ci_Data;
            AddRoute(iffroute->nrp_DestNetwork,iffroute->nrp_Gateway,iffroute->nrp_Hops,-1);
            ci = ci->ci_Next;
        }
    }
    return TRUE;
}

/*------------------------------------------------------------------------*/

VOID DeinitRoute()
{
    ULONG i;
    struct NIPCRouteInfo *nri;

    nri = &gb->RouteInfo;
    if(nri->nri_Default)
        FreeMem(nri->nri_Default,sizeof(struct NIPCRoute));

    for(i=0;i<MAX_ROUTES;i++)
        while((void *)&nri->nri_Routes[i] != (void *) nri->nri_Routes[i].mlh_TailPred)
            FreeMem(RemHead((struct List *)&nri->nri_Routes[i]),sizeof(struct NIPCRoute));
}

/*------------------------------------------------------------------------*/

BOOL __asm AddRoute(register __d0 ULONG network,
                             register __d1 ULONG gateway,
                             register __d2 UWORD hops,
                             register __d3 WORD ttl)
{
    register struct NBase *nb = gb;
    struct sanadev *dev;
    struct NIPCRoute *route;
    ULONG hash;
    BOOL status;
    struct NIPCRouteInfo *nri;

    nri = &gb->RouteInfo;

    if(route = (struct NIPCRoute *) AllocMem(sizeof(struct NIPCRoute),MEMF_PUBLIC|MEMF_CLEAR))
    {
        route->nr_Network = network;
        route->nr_Gateway = gateway;
        route->nr_Hops = hops;
        route->nr_TTL = ttl;

        dev = (struct sanadev *) gb->DeviceList.lh_Head;

        while(dev->sanadev_Node.mln_Succ)
        {
            if(dev->sanadev_SValid)
            {
                if((dev->sanadev_SubnetMask & route->nr_Gateway) == (dev->sanadev_SubnetMask & dev->sanadev_IPAddress))
                {
                    route->nr_Device=dev;
                    break;
                }
            }
            else
            {
                if((dev->sanadev_IPMask & route->nr_Gateway) == (dev->sanadev_IPMask & dev->sanadev_IPAddress))
                {
                    route->nr_Device=dev;
                    break;
                }
            }

            dev = (struct sanadev *) dev->sanadev_Node.mln_Succ;
        }

        if(route->nr_Device)
        {
            route->nr_Mask = NetMask(route->nr_Network);
            ObtainSemaphore(&nb->RouteInfo.nri_Lock);
            if(route->nr_Network)
            {
                hash = RtHash(route->nr_Network);
                AddHead((struct List *)&nri->nri_Routes[hash],(struct Node *)route);
                status = TRUE;
            }
            else if(!gb->RouteInfo.nri_Default)
            {
                gb->RouteInfo.nri_Default = route;
                status = TRUE;
            }

            ReleaseSemaphore(&nb->RouteInfo.nri_Lock);
        }
        if(!status)
            FreeMem(route,sizeof(struct NIPCRoute));
    }

    return status;
}

/*------------------------------------------------------------------------*/

VOID __asm DeleteRoute(register __d0 ULONG network)
{
    register struct NBase *nb = gb;
    struct NIPCRoute *route;
    if(route=Route(network,FALSE))
    {
        ObtainSemaphore(&nb->RouteInfo.nri_Lock);
        if(route->nr_RefCnt==1)
        {
            if(route != gb->RouteInfo.nri_Default)
                Remove((struct Node *)route);
            else
                gb->RouteInfo.nri_Default=NULL;

            FreeMem(route,sizeof(struct NIPCRoute));
        }
        ReleaseSemaphore(&nb->RouteInfo.nri_Lock);
    }
}

