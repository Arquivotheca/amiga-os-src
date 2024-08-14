/**************************************************************************
**
** udp.c        - UDP Protocol routines for nipc.library
**
** Copyright 1992, Commodore-Amiga, Inc.
**
** $Id: udp.c,v 1.12 92/05/05 17:33:37 kcd Exp $
**
***************************************************************************

/*------------------------------------------------------------------------*/

#include "nipcinternal.h"
#include "externs.h"

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include "nipcinternal_protos.h"

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>

/*------------------------------------------------------------------------*/

BOOL UDP_Init(void)
{
    register struct NBase *nb = gb;

    InitSemaphore(&nb->UDPCLSemaphore);
    NewList((struct List *)&gb->UDPConnectionList);

    gb->UDPProto.ipproto_ProtocolNumber = 17;
    gb->UDPProto.ipproto_Input = &UDP_Input;
    gb->UDPProto.ipproto_Deinit = &UDP_Deinit;
    gb->UDPProto.ipproto_Timeout = &UDP_Timeout;
    AddHead((struct List *)&nb->ProtocolList,(struct Node *)&nb->UDPProto);
    gb->UDPPorts = 1024;

    return(TRUE);
}

/*------------------------------------------------------------------------*/

void UDP_Deinit(void)
{
    register struct NBase *nb = gb;
    Remove((struct Node *)&nb->UDPProto);
}

/*------------------------------------------------------------------------*/

void UDP_Timeout(void)
{
    return;
}

/*------------------------------------------------------------------------*/

void UDP_Input(buff, length, dev)
struct Buffer *buff;
ULONG length;
struct sanadev *dev;
{
    register struct NBase *nb = gb;
    struct BuffEntry *be;
    struct udphdr *udp;
    struct iphdr *ip;
    struct UDPConnection *conn;

    be = (struct BuffEntry *) buff->buff_list.mlh_Head;
    ip = (struct iphdr *) ( (ULONG) be->be_data + (ULONG) be->be_offset);
    udp = (struct udphdr *) ( (ULONG) ip + (ULONG) sizeof(struct iphdr));

    ObtainSemaphore(&nb->UDPCLSemaphore);
    if(!IsListEmpty( (struct List *) &gb->UDPConnectionList))
    {
        conn = (struct UDPConnection *) gb->UDPConnectionList.mlh_Head;
        while(conn->conn_link.mln_Succ)
        {
            if(conn->conn_OurPort == udp->udp_DestPort)
            {
                ULONG length;

                length = ip->iph_Length - (ip->iph_IHL << 2) - sizeof(struct udphdr);
                if(length)
                {
                    if(conn->conn_DataIn)
                        (*conn->conn_DataIn)(conn,buff,dev);
                    else
                        FreeBuffer(buff);
                }
                else
                {
                    FreeBuffer(buff);
                }
                break;
            }
            conn = (struct UDPConnection *) conn->conn_link.mln_Succ;
        }
        if (!conn->conn_link.mln_Succ)
            FreeBuffer(buff);
    }
    else
        FreeBuffer(buff);

    ReleaseSemaphore(&nb->UDPCLSemaphore);
}

/*------------------------------------------------------------------------*/

BOOL __asm UDP_Output(register __a0 UBYTE *dataptr, register __d0 UWORD length,
                               register __d1 ULONG srcip, register __d2 ULONG destip,
                               register __d3 UWORD srcport, register __d4 UWORD destport)
{
    struct Buffer *b;
    struct BuffEntry *be;
    struct udphdr *udp;
    UBYTE *data;

    if(b = AllocBuffer(length + sizeof(struct udphdr)))
    {
        be = (struct BuffEntry *) b->buff_list.mlh_Head;
        be->be_length = (length + (sizeof(struct udphdr)));
        udp = (struct udphdr *) be->be_data;
        data = (UBYTE *) ( (ULONG) udp + (ULONG) sizeof(struct udphdr));

        CopyMem(dataptr,data,length);
        udp->udp_SrcPort = srcport;
        udp->udp_DestPort = destport;
        udp->udp_Length = length + sizeof(struct udphdr);
        udp->udp_Checksum = 0;
        ip_sendsrc(b,srcip,destip,17);
        return TRUE;
    }
    else
        return FALSE;
}

/*------------------------------------------------------------------------*/

struct UDPConnection *__asm OpenUDPPort(register __d0 UWORD localport,
                                                  register __a0 void (*datain)())
{
    register struct NBase *nb = gb;
    struct UDPConnection *conn;

    if(localport == -1)
    {
        Forbid();
        localport = gb->UDPPorts++;
        if (gb->UDPPorts == 0)
            gb->UDPPorts = 1024;
        Permit();
    }

    if(conn = AllocMem(sizeof(struct UDPConnection),MEMF_CLEAR|MEMF_PUBLIC))
    {
        conn->conn_OurPort = localport;
        conn->conn_DataIn = datain;
        ObtainSemaphore(&nb->UDPCLSemaphore);
        AddTail((struct List *)&nb->UDPConnectionList,(struct Node *)conn);
        ReleaseSemaphore(&nb->UDPCLSemaphore);
    }
    return conn;
}

/*------------------------------------------------------------------------*/

void __asm CloseUDPPort(register __a0 struct UDPConnection *conn)
{
    if(conn)
    {
        Forbid();
        Remove((struct Node *)conn);
        Permit();
        FreeMem(conn,sizeof(struct UDPConnection));
    }
}

/*------------------------------------------------------------------------*/
