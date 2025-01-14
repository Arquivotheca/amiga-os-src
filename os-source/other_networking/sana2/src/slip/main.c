/*
** $Id: main.c,v 38.1 94/02/17 14:13:56 kcd Exp $
** $State: Exp $
** $Revision: 38.1 $
** $Date: 94/02/17 14:13:56 $
** $Author: kcd $
**
** Amiga SANA-II SLIP device driver.
**
** (C) Copyright 1992,1993 Commodore-Amiga, Inc.
**
** OwnDevUnit support added by Christopher A. Wichura.
*/

#include "slip_device.h"

#include <dos/dostags.h>
#include <dos/rdargs.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/utility_protos.h>
#include <clib/timer_protos.h>
#include <clib/alib_stdio_protos.h>
#include <clib/intuition_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/timer_pragmas.h>
#include <pragmas/intuition_pragmas.h>

#include <string.h>

#ifndef _GENPROTO
#include "main_protos.h"
#include "slcompress_protos.h"
#endif

/*
** To Do:
**
**     Try using an address field size of 0.
**
** For speed purposes:
**
**     Downcode the packet encode/decode routines.
*/

/*
** External variables and functions
**
*/

extern struct Library *ExtDeviceBase;
extern VOID (*DevProcEntry)();
extern ULONG IPToNum(STRPTR ipstr);

/*
** Device Open vector
**
** a1 - SANA2 IO Request
** a6 - Pointer to our device base
** d0 - Unit number
** d1 - Flags
**
*/

ULONG ASM DevOpen(REG(a1) struct IOSana2Req *ios2,
                  REG(d0) ULONG s2unit,
                  REG(d1) ULONG s2flags,
                  REG(a6) struct SLIPDevice *slipDevice)
{
    struct SLIPUnit *slipUnit;
    struct TagItem *bufftag;
    struct Library *UtilityBase;
    struct BufferManagement *bm;
    ULONG returncode;
    BOOL status = FALSE;

    slipDevice->sd_Device.lib_OpenCnt++;        /* So we won't expunge ourselves... */

    /* Make sure our open remains single-threaded. */
    ObtainSemaphore(&slipDevice->sd_Lock);      /* Enforce single threading since we may need to
                                                   Wait() when starting up the Unit process */

    if(s2unit < SD_MAXUNITS)    /* Legal Unit? */
    {
        if(slipUnit = InitSLIPUnit(s2unit,slipDevice))     /* Initialize the Unit */
        {
            if(UtilityBase = OpenLibrary("utility.library",37L)) /* For Tag functions */
            {
                /* Allocate a structure to store the pointers to the callback routines. */

                if(bm = AllocMem(sizeof(struct BufferManagement),MEMF_CLEAR|MEMF_PUBLIC))
                {
                    /* Note: I don't complain if I can't find pointers to the callback routines.
                             This is because there are some programs that may need to open me, but
                             will never use any device commands that require the callbacks. */

                    if(bufftag = FindTagItem(S2_CopyToBuff, (struct TagItem *)ios2->ios2_BufferManagement))
                    {
                        bm->bm_CopyToBuffer = (SANA2_CTB) bufftag->ti_Data;
                    }

                    if(bufftag = FindTagItem(S2_CopyFromBuff, (struct TagItem *)ios2->ios2_BufferManagement))
                    {
                        bm->bm_CopyFromBuffer = (SANA2_CFB) bufftag->ti_Data;
                    }

                    /* New SANA-II V2 Addition */
                    if(bufftag = FindTagItem(S2_PacketFilter, (struct TagItem *)ios2->ios2_BufferManagement))
                    {
                        bm->bm_PacketFilterHook = (struct Hook *) bufftag->ti_Data;
                    }
                    else /* For backwards compatibility */
                    {
                    	bm->bm_PacketFilterHook = &slipDevice->sd_DummyPFHook;
                    }

                    /* Init the list for CMD_READ requests */

                    NewList((struct List *)&bm->bm_RxQueue);

                    AddTail((struct List *)&slipUnit->su_BuffMgmt,(struct Node *)bm);

                    /* Everything went okay. */
                    status = TRUE;
                    returncode = 0;
                    slipDevice->sd_Device.lib_OpenCnt++;
                    slipDevice->sd_Device.lib_Flags &=~LIBF_DELEXP;
                    slipUnit->su_Unit.unit_OpenCnt++;

                    /* Fix up the initial io request */
                    ios2->ios2_BufferManagement = (void *)bm;
                    ios2->ios2_Req.io_Error = 0;
                    ios2->ios2_Req.io_Message.mn_Node.ln_Type = NT_REPLYMSG;
                    ios2->ios2_Req.io_Unit = (struct Unit *)slipUnit;
                    ios2->ios2_Req.io_Device = (struct Device *)slipDevice;
                }
                CloseLibrary(UtilityBase);
            }
        }
    }
    /* See if something went wrong. */
    if(!status)
    {
        ios2->ios2_Req.io_Error = IOERR_OPENFAIL;
        ios2->ios2_Req.io_Unit = (struct Unit *) -1;
        ios2->ios2_Req.io_Device = (struct Device *) -1;
        returncode = IOERR_OPENFAIL;
    }
    ReleaseSemaphore(&slipDevice->sd_Lock);
    slipDevice->sd_Device.lib_OpenCnt--;

    return(returncode);
}

/*
** Device Close vector.
**
** a1 - IOReq
** a6 - Device Pointer
**
*/

BPTR ASM DevClose(REG(a1) struct IOSana2Req *ios2,
                  REG(a6) struct SLIPDevice *slipDevice)
{
    struct SLIPUnit *slipUnit;
    BPTR seglist = 0L;

    ObtainSemaphore(&slipDevice->sd_Lock);

    slipUnit    = (struct SLIPUnit *)ios2->ios2_Req.io_Unit;

    /* Trash the io_Device and io_Unit fields so that any attempt to use this
       request will die immediatly. */

    ios2->ios2_Req.io_Device = (struct Device *) -1;
    ios2->ios2_Req.io_Unit = (struct Unit *) -1;

    /* I always shut the unit process down if the open count drops to zero.
       That way, if I need to expunge, I never have to Wait(). */

    slipUnit->su_Unit.unit_OpenCnt--;
    if(!slipUnit->su_Unit.unit_OpenCnt)
    {
        ExpungeUnit(slipUnit,slipDevice);
    }

    slipDevice->sd_Device.lib_OpenCnt--;

    ReleaseSemaphore(&slipDevice->sd_Lock);

    /* Check to see if we've been asked to expunge. */
    if(slipDevice->sd_Device.lib_Flags & LIBF_DELEXP)
        seglist = DevExpunge(slipDevice);

    return(seglist);
}


/*
** Device Expunge vector
**
** a6 - Device base
**
** Note: You may NEVER EVER Wait() in expunge. Period.
**       Don't even *think* about it.
*/

BPTR ASM DevExpunge(REG(a6) struct SLIPDevice *slipDevice)
{
    BPTR seglist;
    ULONG devbase;
    LONG devbasesize;

    if(slipDevice->sd_Device.lib_OpenCnt)
    {
        /* Sorry, we're busy.  We'll expunge later on
           if we can. */
        slipDevice->sd_Device.lib_Flags |= LIBF_DELEXP;
        seglist = (BPTR)0L;
    }
    else
    {
        /* Free up our library base and function table after
           removing ourselves from the library list. */
        Remove((struct Node *)slipDevice);
        seglist = slipDevice->sd_SegList;

        devbase = (ULONG) slipDevice;

        devbasesize = (ULONG)slipDevice->sd_Device.lib_NegSize;
        devbase = devbase - devbasesize;

        devbasesize += (ULONG)slipDevice->sd_Device.lib_PosSize;

        FreeMem((APTR)devbase,devbasesize);
    }
    return(seglist);
}

/*
** InitSLIPUnit
**
** Initialize (if needed) a new SLIP device Unit and process.
**
*/

struct SLIPUnit *InitSLIPUnit(ULONG s2unit,
                              struct SLIPDevice *slipDevice)
{
    struct SLIPUnit *slipUnit;
    struct TagItem NPTags[]={NP_Entry, 0, NP_Name, 0, NP_Priority, SLIP_PRI , TAG_DONE, 0};
    struct MsgPort *replyport;

    /* Check to see if the Unit is already up and running.  If
       it is, just drop through.  If not, try to start it up. */

    if(!slipDevice->sd_Units[s2unit])
    {
        /* Open up dos.library */
        if(slipDevice->sd_DOSBase = OpenLibrary("dos.library",37L))
        {
            /* Allocate a new Unit structure */
            if(slipUnit = AllocMem(sizeof(struct SLIPUnit), MEMF_CLEAR|MEMF_PUBLIC))
            {
                /* Do some initialization on the Unit structure */

                NewList(&slipUnit->su_Unit.unit_MsgPort.mp_MsgList);

                slipUnit->su_Unit.unit_MsgPort.mp_Node.ln_Type = NT_MSGPORT;
                slipUnit->su_Unit.unit_MsgPort.mp_Flags = PA_IGNORE;
                slipUnit->su_Unit.unit_MsgPort.mp_Node.ln_Name = "slip.device";

                slipUnit->su_UnitNum = s2unit;
                slipUnit->su_Device     = (struct Device *) slipDevice;

                NewList((struct List *)&slipUnit->su_Events);
                NewList((struct List *)&slipUnit->su_BuffMgmt);
                NewList((struct List *)&slipUnit->su_Track);

#ifdef OWNDEVUNIT_SUPPORT
		sprintf(slipUnit->su_OwnerName, "slip.device unit %ld", s2unit);
#endif
                /* Try to read in our configuration file */
                if(ReadConfig(slipUnit,slipDevice))
                {
                    /* Start up the unit process */
                    if(replyport = CreateMsgPort())
                    {
                        slipDevice->sd_Startup.Msg.mn_ReplyPort = replyport;
                        slipDevice->sd_Startup.Device = (struct Device *) slipDevice;
                        slipDevice->sd_Startup.Unit = (struct Unit *)slipUnit;

                        NPTags[0].ti_Data = (ULONG) &DevProcEntry;       /* Assembly entry point for the unit process. */

                        /* Maybe this should be different for each unit? */
                        NPTags[1].ti_Data = (ULONG) "slip.device";      /* Process name */

                        ExtDeviceBase = (struct Library *)slipDevice;

                        if(slipUnit->su_Proc = CreateNewProc(NPTags))
                        {
                            PutMsg(&slipUnit->su_Proc->pr_MsgPort,(struct Message *)&slipDevice->sd_Startup);
                            WaitPort(replyport);
                            GetMsg(replyport);
                        }
                        DeleteMsgPort(replyport);
                    }
                }
                if(!slipUnit->su_Proc)
                {
                    /* The Unit process couldn't start for some reason, so free the Unit structure. */
                    FreeMem(slipUnit,sizeof(struct SLIPUnit));
                }
                else
                {
                    /* Set up the Unit structure pointer in the device base */
                    slipDevice->sd_Units[s2unit] = (struct Unit *)slipUnit;
                }
            }
            CloseLibrary(slipDevice->sd_DOSBase);
        }
    }
    return((struct SLIPUnit *)slipDevice->sd_Units[s2unit]);
}

/*
**
** ExpungeUnit
**
** Tells a unit process to go away...
**
** This function is called from the DevClose routine when the open count for a
** unit reaches zero.  This routine signals the unit process to exit and then
** waits for the unit process to acknowledge.  The unit structure is then
** freed.
*/

VOID ExpungeUnit(struct SLIPUnit *slipUnit,
                 struct SLIPDevice *slipDevice)
{
    struct Task *unittask;

    unittask = (struct Task *)slipUnit->su_Proc;

    slipUnit->su_Proc = (struct Process *)FindTask(0L);

    Signal(unittask,SIGBREAKF_CTRL_F);
    Wait(SIGBREAKF_CTRL_F);

    slipDevice->sd_Units[slipUnit->su_UnitNum] = NULL;

    FreeMem(slipUnit, sizeof(struct SLIPUnit));
}

/*
**
** ReadConfig
**
** Attempt to read in and parse the driver's configuration file.
**
** The files are named by ENV:SANA2/slipX.config where X is the decimal
** representation of the device's unit number.
**
*/

BOOL ReadConfig(struct SLIPUnit *slipUnit,
                struct SLIPDevice *slipDevice)
{
    UBYTE *linebuff,buff[40];
    STRPTR termchar;
    struct RDArgs *rdargs;
    BPTR ConfigFile;
    LONG args[11];

    BOOL status = FALSE;
    ULONG linenum=0;

    /* Create the name of our config file.. */
    sprintf(buff,"ENV:SANA2/slip%ld.config",(ULONG)slipUnit->su_UnitNum);

    /* ...and open it. */
    if(ConfigFile = Open(buff,MODE_OLDFILE))
    {
        /* Here, I use ReadArgs() to do the file parsing for me. */

        if(linebuff = AllocMem(256,MEMF_CLEAR|MEMF_PUBLIC))
        {
            if(rdargs = AllocDosObject(DOS_RDARGS, NULL))
            {
                while(FGets(ConfigFile, linebuff, 255))
                {
                    linenum++;
                    if(linebuff[0] == '#') /* Skip comment lines */
                        continue;

                    rdargs->RDA_Source.CS_Buffer = linebuff;
                    rdargs->RDA_Source.CS_Length = 256;
                    rdargs->RDA_Source.CS_CurChr = 0;

                    /* ReadArgs() requires that the line be null-terminated
                       or funny things happen. */

                    termchar = (STRPTR) linebuff + strlen(linebuff);
                    *termchar = '\n';
                    termchar++;
                    *termchar = 0;

                    memset ((char *) args, 0, sizeof(args));

                    /* Parse the line...*/

                    if(ReadArgs("SERNAME/A,SERUNIT/A/N,SERBAUD/A/N,IPSTR/A,CD=CARRIERDETECT/S,7WIRE/S,COMP/S,AUTO/S,MTU/K/N,SERBUF/K/N,USEODU/S",args,rdargs))
                    {
                        strcpy(slipUnit->su_SerDevName,(STRPTR)args[0]);
                        slipUnit->su_SerUnitNum = *((ULONG *)args[1]);
                        slipUnit->su_BaudRate = *((ULONG *)args[2]);
			slipUnit->su_StAddr = 0;
                        slipUnit->su_DefaultAddr  = IPToNum((STRPTR)args[3]);

                        if(args[4])
                            slipUnit->su_State |= SLIPUF_CD;
                        if(args[5])
                            slipUnit->su_State |= SLIPUF_7WIRE;
                        if(args[6])
                            slipUnit->su_State |= SLIPUF_MAYCOMPRESS;
                        if(args[7])
                            slipUnit->su_State |= SLIPUF_AUTOCOMP;
                        if(args[8])
                            slipUnit->su_MTU = *((ULONG *)args[8]);
                         else
                            slipUnit->su_MTU = DEFAULT_SLIP_MTU;
                        if(args[9])
                            slipUnit->su_RBufLen = *((ULONG *)args[9]);
                        else
                            slipUnit->su_RBufLen = 16384L;
#ifdef OWNDEVUNIT_SUPPORT
			if (args[10])
			    slipUnit->su_ODUBase = OpenLibrary(ODU_NAME, 0L);
			 else
			    slipUnit->su_ODUBase = (struct Library *)NULL;
#endif
                        status = TRUE;
                        FreeArgs(rdargs);
                        break;
                    }
                    else
                    {
                        struct Library *IntuitionBase;
                        struct EasyStruct es;
                        if(IntuitionBase = OpenLibrary("intuition.library",37L))
                        {
                            es.es_StructSize=sizeof(struct EasyStruct);
                            es.es_Flags=0;
                            es.es_Title="Slip.device";
                            es.es_TextFormat="Error in configuration file on line %ld.";
                            es.es_GadgetFormat="Okay";
                            EasyRequestArgs(NULL, &es, 0, &linenum);
                            CloseLibrary(IntuitionBase);
                        }
                        break;
                    }

                }
                FreeDosObject(DOS_RDARGS,rdargs);
            }
            FreeMem(linebuff, 256);
        }
        Close(ConfigFile);
    }
    return(status);
}


/*
**
** BeginIO
**
** This is the dispatch point for the driver's incoming IORequests.
**
** Registers at entry:
**
** a1 = IORequest
** a6 = Device Base
**
*/

#define SLIP_IMMEDIATES 0L

VOID ASM DevBeginIO(REG(a1) struct IOSana2Req *ios2,
                    REG(a6) struct SLIPDevice *slipDevice)
{
    ios2->ios2_Req.io_Message.mn_Node.ln_Type = NT_MESSAGE;

    if(ios2->ios2_Req.io_Command < S2_END)
    {
        if((1L << ios2->ios2_Req.io_Command) & SLIP_IMMEDIATES)
        {
            PerformIO(ios2,slipDevice);
        }
        else
        {
            ios2->ios2_Req.io_Flags &= ~IOF_QUICK;
            PutMsg((struct MsgPort *)ios2->ios2_Req.io_Unit,(struct Message *)ios2);
        }
    }
    else
    {
        ios2->ios2_Req.io_Error = IOERR_NOCMD;
        TermIO(ios2,slipDevice);
    }
}

/*
** This routine is used to dispatch an IO request either from BeginIO
** or from the Unit process.
*/

VOID PerformIO(struct IOSana2Req *ios2,
               struct SLIPDevice *slipDevice)
{
    struct SLIPUnit *slipUnit;

    slipUnit    = (struct SLIPUnit *)ios2->ios2_Req.io_Unit;

    ios2->ios2_Req.io_Error = 0;

    switch(ios2->ios2_Req.io_Command)
    {
        case CMD_READ:              ReadPacket(ios2,slipUnit,slipDevice);
                                    break;

        case CMD_WRITE:             WritePacket(ios2,slipUnit,slipDevice);
                                    break;

        case S2_DEVICEQUERY:        DeviceQuery(ios2,slipUnit,slipDevice);
                                    break;

        case S2_GETSTATIONADDRESS:  GetStationAddress(ios2,slipUnit,slipDevice);
                                    break;

        case S2_CONFIGINTERFACE:    ConfigInterface(ios2,slipUnit,slipDevice);
                                    break;

        /* These commands are not appropriate for SLIP/CSLIP */
        case S2_READORPHAN:
        case S2_ADDMULTICASTADDRESS:
        case S2_DELMULTICASTADDRESS:
        case S2_MULTICAST:          ios2->ios2_Req.io_Error = S2ERR_NOT_SUPPORTED;
                                    ios2->ios2_WireError = S2WERR_GENERIC_ERROR;
                                    TermIO(ios2,slipDevice);
                                    break;

        case S2_BROADCAST:          WritePacket(ios2,slipUnit,slipDevice);
                                    break;

        case S2_TRACKTYPE:          TrackType(ios2,slipUnit,slipDevice);
                                    break;

        case S2_UNTRACKTYPE:        UnTrackType(ios2,slipUnit,slipDevice);
                                    break;

        case S2_GETTYPESTATS:       GetTypeStats(ios2,slipUnit,slipDevice);
                                    break;

        case S2_GETSPECIALSTATS:    GetSpecialStats(ios2,slipUnit,slipDevice);
                                    break;

        case S2_GETGLOBALSTATS:     GetGlobalStats(ios2,slipUnit,slipDevice);
                                    break;

        case S2_ONEVENT:            OnEvent(ios2,slipUnit,slipDevice);
                                    break;

        case S2_ONLINE:             Online(ios2,slipUnit,slipDevice);
                                    break;

        case S2_OFFLINE:            Offline(ios2,slipUnit,slipDevice);
                                    break;

        default:                    ios2->ios2_Req.io_Error = IOERR_NOCMD;
                                    TermIO(ios2,slipDevice);
                                    break;
    }
}

/*
** This function returns any device specific statistics that
** we may have.  Unfortunately, we don't have and SLIP specific
** statistics.
*/
VOID ASM GetSpecialStats(STDSLIPARGS)
{
    struct Sana2SpecialStatHeader *stats;

    stats = (struct Sana2SpecialStatHeader *)ios2->ios2_StatData;

    stats->RecordCountSupplied = 0;
    TermIO(ios2,slipDevice);
}

/*
** This function returns the global statistics for the
** slip device.
*/
VOID ASM GetGlobalStats(STDSLIPARGS)
{
    struct Sana2DeviceStats *stats;

    stats = (struct Sana2DeviceStats *)ios2->ios2_StatData;

    stats->PacketsReceived      = slipUnit->su_Stats.PacketsReceived;
    stats->PacketsSent          = slipUnit->su_Stats.PacketsSent;
    stats->BadData              = slipUnit->su_Stats.BadData;
    stats->Overruns             = slipUnit->su_Stats.Overruns;
    stats->UnknownTypesReceived = slipUnit->su_Stats.UnknownTypesReceived;
    stats->Reconfigurations     = slipUnit->su_Stats.Reconfigurations;
    stats->LastStart.tv_secs    = slipUnit->su_Stats.LastStart.tv_secs;
    stats->LastStart.tv_micro   = slipUnit->su_Stats.LastStart.tv_micro;

    ios2->ios2_Req.io_Error = 0;

    TermIO(ios2,slipDevice);
}

/*
** This function returns statistics for a specific
** type of packet that is being tracked.  Unfortunately,
** SLIP can't differentiate between different packet
** types, which makes packet type tracking essentially
** useless as the numbers would essentially be the same
** as those found via S2_GETGLOBALSTATS.
**
** Just to be thourough, I have arbitrarily picked
** the packet type for SLIP IP packets to be 2048, the
** same as tha used for Ethernet.  This will at least
** allow you to track IP packets.
*/
VOID ASM GetTypeStats(STDSLIPARGS)
{
    struct Sana2PacketTypeStats *stats;
    struct SuperS2PTStats *sstats;

    ObtainSemaphore(&slipUnit->su_ListLock);

    stats = (struct Sana2PacketTypeStats *)ios2->ios2_StatData;
    sstats = (struct SuperS2PTStats *)slipUnit->su_Track.mlh_Head;

    while(sstats->ss_Node.mln_Succ)
    {
        if(ios2->ios2_PacketType == sstats->ss_PType)
        {
            stats->PacketsSent = sstats->ss_Stats.PacketsSent;
            stats->PacketsReceived = sstats->ss_Stats.PacketsReceived;
            stats->BytesSent = sstats->ss_Stats.BytesSent;
            stats->BytesReceived = sstats->ss_Stats.BytesReceived;
            stats->PacketsDropped = sstats->ss_Stats.PacketsDropped;
            break;
        }
        sstats = (struct SuperS2PTStats *)sstats->ss_Node.mln_Succ;
    }
    ReleaseSemaphore(&slipUnit->su_ListLock);
    if(!sstats->ss_Node.mln_Succ)
    {
        ios2->ios2_Req.io_Error = S2ERR_BAD_STATE;
        ios2->ios2_WireError = S2WERR_NOT_TRACKED;
    }
    TermIO(ios2,slipDevice);
}

/*
** This function adds a packet type to the list
** of those that are being tracked.
*/
VOID ASM TrackType(STDSLIPARGS)
{
    struct SuperS2PTStats *stats;

    ObtainSemaphore(&slipUnit->su_ListLock);

    stats = (struct SuperS2PTStats *)slipUnit->su_Track.mlh_Head;

    while(stats->ss_Node.mln_Succ)
    {
        if(ios2->ios2_PacketType == stats->ss_PType)
        {
            ios2->ios2_Req.io_Error = S2ERR_BAD_STATE;
            ios2->ios2_WireError = S2WERR_ALREADY_TRACKED;
            break;
        }
        stats = (struct SuperS2PTStats *)stats->ss_Node.mln_Succ;
    }
    if(!stats->ss_Node.mln_Succ)
    {
        if(stats = AllocMem(sizeof(struct SuperS2PTStats),MEMF_CLEAR|MEMF_PUBLIC))
        {
            stats->ss_PType = ios2->ios2_PacketType;
            if(ios2->ios2_PacketType == 2048)
                slipUnit->su_IPTrack = stats;
            AddTail((struct List *)&slipUnit->su_Track,(struct Node     *)stats);
        }
    }
    ReleaseSemaphore(&slipUnit->su_ListLock);

    TermIO(ios2,slipDevice);
}

/*
** This function removes a packet type from the
** list of those that are being tracked.
*/
VOID ASM UnTrackType(STDSLIPARGS)
{
    struct SuperS2PTStats *stats;
    struct SuperS2PTStats *stats_next;

    ObtainSemaphore(&slipUnit->su_ListLock);

    stats = (struct SuperS2PTStats *)slipUnit->su_Track.mlh_Head;

    while(stats->ss_Node.mln_Succ)
    {
        stats_next = (struct SuperS2PTStats *)stats->ss_Node.mln_Succ;
        if(ios2->ios2_PacketType == stats->ss_PType)
        {
            if(ios2->ios2_PacketType == 2048)
                slipUnit->su_IPTrack = NULL;
            Remove((struct Node *)stats);
            FreeMem(stats,sizeof(struct SuperS2PTStats));
            stats = NULL;
            break;
        }
        stats = stats_next;
    }
    if(stats)
    {
        ios2->ios2_Req.io_Error = S2ERR_BAD_STATE;
        ios2->ios2_WireError = S2WERR_NOT_TRACKED;
    }
    ReleaseSemaphore(&slipUnit->su_ListLock);

    TermIO(ios2,slipDevice);
}

/*
** This function is called whenever we receive a packet
** from the serial device driver.
**
*/
VOID PacketReceived(ULONG length,
		    struct SLIPUnit *slipUnit)
{
    slipUnit->su_Stats.PacketsReceived++;

    if(slipUnit->su_IPTrack)
    {
        slipUnit->su_IPTrack->ss_Stats.PacketsReceived++;
        slipUnit->su_IPTrack->ss_Stats.BytesReceived+=length;
    }
}

/*
** This function is called whenever a packet is
** sent to the serial device driver.
*/
VOID PacketSent(ULONG length,
		struct SLIPUnit *slipUnit)
{
    slipUnit->su_Stats.PacketsSent++;

    if(slipUnit->su_IPTrack)
    {
        slipUnit->su_IPTrack->ss_Stats.PacketsSent++;
        slipUnit->su_IPTrack->ss_Stats.BytesSent+=length;
    }
}

/*
** This function is called whenever a packet that
** is too large is received.
*/
VOID PacketOverrun(struct SLIPUnit *slipUnit,
                   struct SLIPDevice *slipDevice)
{
    slipUnit->su_Stats.Overruns++;
    DoEvent(S2EVENT_ERROR|S2EVENT_RX,slipUnit,slipDevice);
}

/*
** This function is called whenever a packet with
** garbage data is encountered.
*/
VOID ReceivedGarbage(struct SLIPUnit *slipUnit,
                     struct SLIPDevice *slipDevice)
{
    slipUnit->su_Stats.BadData++;
    DoEvent(S2EVENT_ERROR|S2EVENT_RX,slipUnit,slipDevice);
}

/*
** This function is called whenever a packet
** is dropped by the SLIP driver.
*/
VOID PacketDropped(struct SLIPUnit *slipUnit,
		   struct SLIPDevice *slipDevice)
{
    if(slipUnit->su_IPTrack)
    {
        slipUnit->su_IPTrack->ss_Stats.PacketsDropped++;
    }
    DoEvent(S2EVENT_ERROR|S2EVENT_RX,slipUnit,slipDevice);
}

/*
** This function is used to return an IO request
** back to the sender.
*/
VOID ASM TermIO(REG(a2) struct IOSana2Req *ios2,
            REG(a6) struct SLIPDevice *slipDevice)
{
    if(!(ios2->ios2_Req.io_Flags & IOF_QUICK))
        ReplyMsg((struct Message *)ios2);
}

/*
** The device AbortIO() entry point.
**
** A1 - The IO request to be aborted.
** A6 - The device base.
*/
ULONG ASM DevAbortIO(REG(a1) struct IOSana2Req *ios2,
                     REG(a6) struct SLIPDevice *slipDevice)
{
    ULONG result = 0L;
    struct SLIPUnit *slipUnit;
    struct BufferManagement *bm;

    slipUnit = (struct SLIPUnit *)ios2->ios2_Req.io_Unit;
    ObtainSemaphore(&slipUnit->su_ListLock);
    if(ios2->ios2_Req.io_Message.mn_Node.ln_Type != NT_REPLYMSG)
    {
        switch(ios2->ios2_Req.io_Command)
        {
                /* Walk the list of BufferManagement structs, trying AbortReq()
                   on each. */

                case CMD_READ:  bm = (struct BufferManagement *)slipUnit->su_BuffMgmt.mlh_Head;
                                while(bm->bm_Node.mln_Succ)
                                {
                                    /* If result is 0, the IORequest was found,
                                       so we're done. */
                                    if(!(result=AbortReq(&bm->bm_RxQueue,ios2,slipDevice)))
                                        break;

                                    bm = (struct BufferManagement *)bm->bm_Node.mln_Succ;
                                }
                                break;

                case CMD_WRITE: Disable();
                                if(!(result=AbortReq((struct MinList *)&slipUnit->su_Tx->mp_MsgList,ios2,slipDevice)))
                                    result=AbortReq((struct MinList *)&slipUnit->su_TxFast->mp_MsgList,ios2,slipDevice);
                                Enable();
                                break;

                case S2_ONEVENT:        result=AbortReq(&slipUnit->su_Events,ios2,slipDevice);
                                        break;

                default:                result=IOERR_NOCMD;
                                        break;
        }
    }
    ReleaseSemaphore(&slipUnit->su_ListLock);
    return(result);
}

/*
** This funcion is used to locate an IO request in a linked
** list and abort it if found.
*/
ULONG AbortReq(struct MinList *minlist,
               struct IOSana2Req *ios2,
               struct SLIPDevice *slipDevice)
{
    struct Node *node, *next;
    ULONG result=IOERR_NOCMD;

    node = (struct Node *)minlist->mlh_Head;

    while(node->ln_Succ)
    {
        next = node->ln_Succ;

        if(node == (struct Node *)ios2)
        {
            Remove((struct Node *)ios2);
            ios2->ios2_Req.io_Error = IOERR_ABORTED;
            TermIO(ios2,slipDevice);
            result = 0;
            break;
        }
        node = next;
    }
    return(result);
}

/*
** This function handles S2_CONFIGINTERFACE commands.
*/
VOID ASM ConfigInterface(STDSLIPARGS)
{
    /* Note: we may only be configured once. */
    if(!(slipUnit->su_State & SLIPUF_CONFIG))
    {
        if(OpenSerial(slipUnit,slipDevice))
        {
            /* FIXED - Now sets our address to whatever the
               protocol stack says to use. */
            CopyMem(&ios2->ios2_SrcAddr,&slipUnit->su_StAddr,4);
            slipUnit->su_State |= SLIPUF_CONFIG;
        }
    }
    else
    {
        /* Sorry, we're already configured. */
        ios2->ios2_Req.io_Error = S2ERR_BAD_STATE;
        ios2->ios2_WireError = S2WERR_IS_CONFIGURED;
    }
    TermIO(ios2,slipDevice);
}

/*
** This function handles S2_GETSTATIONADDRESS commands.
**
** We don't really have a hardware address, so we will
** just clear the source address field.
*/

VOID ASM GetStationAddress(STDSLIPARGS)
{
    UBYTE i;

    for(i=0; i< SANA2_MAX_ADDR_BYTES; i++)
    {
        ios2->ios2_SrcAddr[i]=0;
        ios2->ios2_DstAddr[i]=0;
    }
    CopyMem(&slipUnit->su_StAddr,&ios2->ios2_SrcAddr,4);
    CopyMem(&slipUnit->su_DefaultAddr,&ios2->ios2_DstAddr,4);

    TermIO(ios2,slipDevice);
}

/*
** Init Table for the Sana2DeviceQuery structure.
*/

struct DQInfo DQTable[9] =
{
        DQ_CLONG,       4,      0,      /* This and the next are placeholders. */
        DQ_CLONG,       4,      0,
        DQ_CLONG,       4,      0,
        DQ_CLONG,       4,      0,
        DQ_CWORD,       2,      32,
        DQ_MTU,         4,      0,
        DQ_BPS,         4,      0,
        DQ_CLONG,       4,      S2WireType_SLIP,
        0,              0,      0
};

/*
** This function handles S2_DEVICEQUERY comands.
*/
VOID ASM DeviceQuery(STDSLIPARGS)
{
    ULONG size, supplied;
    UWORD i;
    UBYTE *w_byte;
    struct DQInfo *dqi;

    struct Sana2DeviceQuery *sdq;

    sdq = (struct Sana2DeviceQuery *)ios2->ios2_StatData;
    size = sdq->SizeAvailable;
    supplied = 0;
    dqi = &DQTable[0];

    for(i = 0,w_byte = (UBYTE *)ios2->ios2_StatData; (dqi->Size && (dqi->Size <= size)); i++)
    {
        size -= dqi->Size;
        supplied += dqi->Size;
        dqi = &DQTable[i];

        switch(dqi->Type)
        {
                /* Ant of these are constants */

                case DQ_CWORD : *(UWORD *)w_byte = dqi->Data;
                                w_byte = (UBYTE *)((ULONG)w_byte + 2);
                                break;

                case DQ_CLONG : *(ULONG *)w_byte = dqi->Data;
                                w_byte = (UBYTE *)((ULONG)w_byte + 4);
                                break;

                /* These two are configuration dependant. */

                case DQ_MTU  :  *(ULONG *)w_byte = slipUnit->su_MTU;
                                w_byte = (UBYTE *)((ULONG)w_byte + 4);
                                break;

                case DQ_BPS  :  *(ULONG *)w_byte = slipUnit->su_BaudRate;
                                w_byte = (UBYTE *)((ULONG)w_byte + 4);
                                break;

        }
    }

    /* I shouldn't have to check this, but I do it anyway for a sanity check. */
    if(sdq->SizeAvailable >= 8)
    {
        sdq->SizeSupplied = supplied;
    }
    TermIO(ios2,slipDevice);
}

/*
** This function is used for handling CMD_WRITE
** commands.
*/
VOID ASM WritePacket(STDSLIPARGS)
{
    /* Make sure that we are online. */
    if(slipUnit->su_State & SLIPUF_ONLINE)
    {
        /* Make sure it's a legal length. */
        if(ios2->ios2_DataLength <= slipUnit->su_MTU)
        {
            /* Queue the request for the slip.device process. */

            ios2->ios2_Req.io_Flags &= ~IOF_QUICK;

            PutMsg(slipUnit->su_Tx,(struct Message *)ios2);

        }
        else
        {
            /* Sorry, the packet is too long! */
            ios2->ios2_Req.io_Error = S2ERR_MTU_EXCEEDED;
            ios2->ios2_WireError = S2WERR_GENERIC_ERROR;
            TermIO(ios2,slipDevice);
            DoEvent(S2EVENT_TX,slipUnit,slipDevice);
        }
    }
    else
    {
        /* Sorry, we're offline */
        ios2->ios2_Req.io_Error = S2ERR_OUTOFSERVICE;
        ios2->ios2_WireError = S2WERR_UNIT_OFFLINE;
        TermIO(ios2,slipDevice);
    }
}

VOID ASM SendPacket(STDSLIPARGS)
{
    struct IOExtSer *ioser;
    struct BufferManagement *bm;
    ULONG framelength;
    UBYTE *hdr;
    u_char ptype;

    bm =(struct BufferManagement *) ios2->ios2_BufferManagement;

    /* Copy the data out of the packet into our temporary buffer. */
    if((*bm->bm_CopyFromBuffer)(slipUnit->su_TxBuff,ios2->ios2_Data,ios2->ios2_DataLength))
    {
        PacketSent(ios2->ios2_DataLength,slipUnit);

        /* See if this packet is TCP and we are allowed to compress packets. */

        if((((struct ip *)slipUnit->su_TxBuff)->ip_p == IPPROTO_TCP) &&
           (slipUnit->su_State & SLIPUF_MAYCOMPRESS))
        {
            slipUnit->su_MBuff.m_off = slipUnit->su_TxBuff;
            slipUnit->su_MBuff.m_len = ios2->ios2_DataLength;
            ptype = sl_compress_tcp(&slipUnit->su_MBuff,(struct ip *)slipUnit->su_TxBuff,&slipUnit->su_SLCompress, 1);
            if(ptype == 0x40)
            {
                framelength=EncodeSLIP(slipUnit->su_TxBuff,slipUnit->su_TxSLIP,ios2->ios2_DataLength);
            }
            else
            {
                hdr = slipUnit->su_MBuff.m_off;
                hdr[0] |= ptype;
                framelength=EncodeSLIP(slipUnit->su_MBuff.m_off,slipUnit->su_TxSLIP,slipUnit->su_MBuff.m_len);
            }
        }

        /* Either the packet isn't TCP or we're not allowed to use compression */
        else
        {
            /* Encode the packet in SLIP format */
            framelength=EncodeSLIP(slipUnit->su_TxBuff,slipUnit->su_TxSLIP,ios2->ios2_DataLength);
        }
        ioser = slipUnit->su_SerTx;
        ioser->IOSer.io_Data = slipUnit->su_TxSLIP;
        ioser->IOSer.io_Length = framelength;
        ioser->IOSer.io_Command = CMD_WRITE;
        ioser->IOSer.io_Error = 0;
        ioser->IOSer.io_Message.mn_Node.ln_Type = 0;

        /* Send the packet to the serial device driver */
        SendIO((struct IORequest *)ioser);
    }
    else
    {
        /* Something went wrong...*/
        ios2->ios2_Req.io_Error = S2ERR_NO_RESOURCES;
        ios2->ios2_WireError = S2WERR_BUFF_ERROR;
        DoEvent(S2EVENT_BUFF,slipUnit,slipDevice);
    }
    TermIO(ios2,slipDevice);
}

/*
** This routine encodes a packet in SLIP format.
**
** The format is quite simple.
**
** SLIP Encoding:
**
** SLIP_ESC -> SLIP_ESC SLIP_ESC_ESC
** SLIP_END -> SLIP_ESC SLIP_ESC_END
**
** The packet is preceded and terminated with a SLIP_END as prescribed by
** rfc 1055.
**
*/
ULONG EncodeSLIP(UBYTE *source, UBYTE *dest, ULONG length)
{
    UBYTE ch;
    UBYTE *current;

    current = dest;

    *current = SLIP_END;
    current++;

    while(length--)
    {
        ch = *source;
        source++;

        if(ch == SLIP_ESC)
        {
            *current = SLIP_ESC;
            current++;
            ch = SLIP_ESC_ESC;
        }
        else if(ch == SLIP_END)
        {
            *current = SLIP_ESC;
            current ++;
            ch = SLIP_ESC_END;
        }
        *current = ch;
        current++;
    }
    *current = SLIP_END;
    current++;

    return((ULONG)(current - dest));
}

/*
** This routine handles CMD_READ commands.  We
** always queue these unless we're offline.
*/
VOID ASM ReadPacket(STDSLIPARGS)
{
    struct BufferManagement *bm;

    if(slipUnit->su_State & SLIPUF_ONLINE)
    {
        /* Find the appropriate queue for this IO request */

        ObtainSemaphore(&slipUnit->su_ListLock);
        bm = (struct BufferManagement *)slipUnit->su_BuffMgmt.mlh_Head;
        while(bm->bm_Node.mln_Succ)
        {
            if(bm == (struct BufferManagement *)ios2->ios2_BufferManagement)
            {
                AddTail((struct List *)&bm->bm_RxQueue,(struct Node *)ios2);
                break;
            }
            bm = (struct BufferManagement *)bm->bm_Node.mln_Succ;
        }
        /* Did we fall of the end of the list? */
        if(!bm->bm_Node.mln_Succ)
        {
            ios2->ios2_Req.io_Error = S2ERR_BAD_ARGUMENT;
            ios2->ios2_WireError = S2WERR_GENERIC_ERROR;
            TermIO(ios2,slipDevice);
        }
        ReleaseSemaphore(&slipUnit->su_ListLock);
    }
    else
    {
        /* Sorry, we're offline */
        ios2->ios2_Req.io_Error = S2ERR_OUTOFSERVICE;
        ios2->ios2_WireError = S2WERR_UNIT_OFFLINE;
        TermIO(ios2,slipDevice);
    }
}

/*
** This routine initializes our IO requests and buffers
** for serial i/o and SLIP encoding/decoding.
*/
BOOL InitSerial(struct SLIPUnit *slipUnit,
                struct SLIPDevice *slipDevice)
{
    ULONG *clr;
    BOOL status = FALSE;

    for(clr = (ULONG *) &slipUnit->su_SerRx; clr <= (ULONG *) &slipUnit->su_TxSLIP; clr++)
        *clr = 0L;

    if(slipUnit->su_TxPort = CreateMsgPort())
    {
        if(slipUnit->su_SerTx = CreateIORequest(slipUnit->su_TxPort,sizeof(struct IOExtSer)))
        {
            if(slipUnit->su_RxPort = CreateMsgPort())
            {
                if(slipUnit->su_SerRx = CreateIORequest(slipUnit->su_RxPort,sizeof(struct IOExtSer)))
                {
                    if(slipUnit->su_TxBuff = AllocMem((slipUnit->su_MTU + 128) * 2,MEMF_CLEAR|MEMF_PUBLIC))
                    {
                        slipUnit->su_RxBuff = slipUnit->su_TxBuff + slipUnit->su_MTU + 128;
                        slipUnit->su_RxBuffPtr = slipUnit->su_RxBuff + 128;

                        if(slipUnit->su_TxSLIP = AllocMem((slipUnit->su_MTU*2 + 128) * 2,MEMF_CLEAR|MEMF_PUBLIC))
                        {
                            slipUnit->su_RxSLIP = slipUnit->su_TxSLIP + slipUnit->su_MTU*2 + 128;
                            {
                                status = TRUE;
                            }
                        }
                    }
                }
            }
        }
    }
    if(!status)
        DeinitSerial(slipUnit,slipDevice);
    return(status);
}

/*
** This routine cleans up our serial i/o requests
** and misc. buffers.
*/
VOID DeinitSerial(struct SLIPUnit *slipUnit,
                  struct SLIPDevice *slipDevice)
{
    if(slipUnit->su_SerTx)
        DeleteIORequest(slipUnit->su_SerTx);

    if(slipUnit->su_TxPort)
        DeleteMsgPort(slipUnit->su_TxPort);

    if(slipUnit->su_SerRx)
        DeleteIORequest(slipUnit->su_SerRx);

    if(slipUnit->su_RxPort)
        DeleteMsgPort(slipUnit->su_RxPort);

    if(slipUnit->su_TxBuff)
        FreeMem(slipUnit->su_TxBuff,(slipUnit->su_MTU + 128) * 2);

    if(slipUnit->su_TxSLIP)
        FreeMem(slipUnit->su_TxSLIP,(slipUnit->su_MTU*2 + 128) * 2);
}

/*
** This routine handles S2_ONEVNET commands.
*/
VOID ASM OnEvent(STDSLIPARGS)
{

    /* Two special cases. S2EVENT_ONLINE and S2EVENT_OFFLINE are supposed to
       retun immediately if we are already in the state that they are waiting
       for. */

    if( ((ios2->ios2_WireError & S2EVENT_ONLINE) && (slipUnit->su_State & SLIPUF_ONLINE)) ||
        ((ios2->ios2_WireError & S2EVENT_OFFLINE) && (!(slipUnit->su_State & SLIPUF_ONLINE))))
    {
        ios2->ios2_Req.io_Error = 0;
        ios2->ios2_WireError &= (S2EVENT_ONLINE|S2EVENT_OFFLINE);
        TermIO(ios2,slipDevice);
        return;
    }

    /* Queue anything else */

    ObtainSemaphore(&slipUnit->su_ListLock);
    AddTail((struct List *)&slipUnit->su_Events,(struct Node *)ios2);
    ReleaseSemaphore(&slipUnit->su_ListLock);
}

/*
** This routine opens the serial device driver and attempts to bring
** the device online.
*/
BOOL OpenSerial(struct SLIPUnit *slipUnit,
                struct SLIPDevice *slipDevice)
{
    BOOL status = TRUE;
    ULONG odflags = 0;
#ifdef OWNDEVUNIT_SUPPORT
    STRPTR oduError;
#endif
    slipUnit->su_SerRx->IOSer.io_Device = NULL;

    sl_compress_init(&slipUnit->su_SLCompress);

#ifdef OWNDEVUNIT_SUPPORT
    if(slipUnit->su_ODUBase)
    {
	oduError = AttemptDevUnit(slipUnit->su_SerDevName, slipUnit->su_SerUnitNum, slipUnit->su_OwnerName, 0L);

	if(oduError)
	{
	        struct Library *IntuitionBase;
	        struct EasyStruct es;
	        ULONG args[3];
		STRPTR msg;
	        args[0]=(ULONG)slipUnit->su_SerDevName;
	        args[1]=(ULONG)slipUnit->su_SerUnitNum;
		args[2]=(ULONG)oduError;
		if (oduError[0] == ODUERR_LEADCHAR[0])
		{
		    args[2]++;
		    msg = "Error locking %s unit %ld: %s";
		}
		else
		    msg = "%s unit %ld already owned by %s.";

	        if(IntuitionBase = OpenLibrary("intuition.library",37L))
	        {
	            es.es_StructSize=sizeof(struct EasyStruct);
	            es.es_Flags=0;
	            es.es_Title="Slip.device";
	            es.es_TextFormat=msg;
	            es.es_GadgetFormat="Okay";
	            EasyRequestArgs(NULL, &es, 0, (APTR)args);
	            CloseLibrary(IntuitionBase);
	        }

		return FALSE;
	}
    }
#endif
    if(slipUnit->su_State &     SLIPUF_7WIRE)
        odflags = SERF_7WIRE;

    slipUnit->su_SerTx->io_SerFlags = odflags;

    if(!OpenDevice(slipUnit->su_SerDevName,slipUnit->su_SerUnitNum,(struct IORequest *)slipUnit->su_SerTx,odflags))
    {
        /* Set up our serial parameters */
        slipUnit->su_SerRx->IOSer.io_Device     = slipUnit->su_SerTx->IOSer.io_Device;
        slipUnit->su_SerRx->IOSer.io_Unit =     slipUnit->su_SerTx->IOSer.io_Unit;

        slipUnit->su_SerTx->IOSer.io_Command = SDCMD_SETPARAMS;
        slipUnit->su_SerTx->io_Baud     = slipUnit->su_BaudRate;
        slipUnit->su_SerTx->io_RBufLen = slipUnit->su_RBufLen;
        slipUnit->su_SerTx->io_ReadLen = 8;
        slipUnit->su_SerTx->io_WriteLen = 8;
        slipUnit->su_SerTx->io_StopBits = 1;
        slipUnit->su_SerTx->io_SerFlags = SERF_XDISABLED|SERF_RAD_BOOGIE;

        if(!DoIO((struct IORequest *)slipUnit->su_SerTx))
        {
            /* Assume we're now online */
            slipUnit->su_State |= SLIPUF_ONLINE;

            /* If we are running with auto-compression on, clear the
               current value of our COMPRESSION flag. */

            if(slipUnit->su_State &     SLIPUF_AUTOCOMP)
                slipUnit->su_State &= ~SLIPUF_MAYCOMPRESS;

            /* Are we checking for serial detect? */
            if(slipUnit->su_State &     SLIPUF_CD)
            {
                slipUnit->su_SerTx->IOSer.io_Command = SDCMD_QUERY;
                if(!DoIO((struct IORequest *)slipUnit->su_SerTx))
                {
                    if(slipUnit->su_SerTx->io_Status & (1<<5))
                    {
                        /* Sorry, no carrier, shut down the serial driver
                           and set our state to offline. */
                        CloseDevice((struct IORequest *)slipUnit->su_SerTx);
#ifdef OWNDEVUNIT_SUPPORT
			if (slipUnit->su_ODUBase)
			    FreeDevUnit(slipUnit->su_SerDevName, slipUnit->su_SerUnitNum);
#endif
                        slipUnit->su_State &= ~SLIPUF_ONLINE;
                    }
                }
                else
                    status = FALSE;
            }
            if(status)
            {
                /* Queue up the initial CMD_READ command for the
                   serial driver. */
                slipUnit->su_SerRx->IOSer.io_Command = CMD_READ;
                slipUnit->su_SerRx->IOSer.io_Length     = 1;
                slipUnit->su_SerRx->IOSer.io_Data =     slipUnit->su_RxSLIP;
                SendIO((struct IORequest *)slipUnit->su_SerRx);
            }
        }
        else
            status = FALSE;

        if(!status)
            CloseDevice((struct IORequest *)slipUnit->su_SerTx);
    }
    else
    {
        struct Library *IntuitionBase;
        struct EasyStruct es;
        ULONG args[2];
        args[0]=(ULONG)slipUnit->su_SerDevName;
        args[1]=(ULONG)slipUnit->su_SerUnitNum;
        if(IntuitionBase = OpenLibrary("intuition.library",37L))
        {
            es.es_StructSize=sizeof(struct EasyStruct);
            es.es_Flags=0;
            es.es_Title="Slip.device";
            es.es_TextFormat="Couldn't open %s unit %ld.";
            es.es_GadgetFormat="Okay";
            EasyRequestArgs(NULL, &es, 0, (APTR)args);
            CloseLibrary(IntuitionBase);
        }
        status = FALSE;
    }

    if(slipUnit->su_State &     SLIPUF_ONLINE)
        MarkTimeOnline(slipUnit,slipDevice);

#ifdef OWNDEVUNIT_SUPPORT
    if(!status && slipUnit->su_ODUBase)
	FreeDevUnit(slipUnit->su_SerDevName, slipUnit->su_SerUnitNum);
#endif

    return(status);
}

/*
** This routine gets the current system time and stores
** it in our global statistics structure.
*/

VOID MarkTimeOnline(struct SLIPUnit *slipUnit,
                    struct SLIPDevice *slipDevice)
{
    register struct Library *TimerBase;
    struct timerequest *treq;

    if(treq = (struct timerequest *)AllocMem(sizeof(struct timerequest),MEMF_PUBLIC|MEMF_CLEAR))
    {
        if(!OpenDevice("timer.device",UNIT_MICROHZ,(struct IORequest *)treq,0L))
        {
            TimerBase = (struct Library *)treq->tr_node.io_Device;
            GetSysTime(&slipUnit->su_Stats.LastStart);
            CloseDevice((struct IORequest *)treq);
        }
        FreeMem(treq,sizeof(struct timerequest));
    }
}

/*
** This routine aborts any pending activity with the serial
** device driver and then brings the slip driver offline.
*/
VOID CloseSerial(struct SLIPUnit *slipUnit,
                 struct SLIPDevice *slipDevice)
{
    AbortIO((struct IORequest *)slipUnit->su_SerRx);
    WaitIO((struct IORequest *)slipUnit->su_SerRx);

    while(GetMsg(slipUnit->su_RxPort));

    AbortIO((struct IORequest *)slipUnit->su_SerTx);
    WaitIO((struct IORequest *)slipUnit->su_SerTx);

    while(GetMsg(slipUnit->su_TxPort));

    CloseDevice((struct IORequest *)slipUnit->su_SerRx);
#ifdef OWNDEVUNIT_SUPPORT
    if (slipUnit->su_ODUBase)
        FreeDevUnit(slipUnit->su_SerDevName, slipUnit->su_SerUnitNum);
#endif
    slipUnit->su_State &= ~SLIPUF_ONLINE;
}

/*
** This routime handles CMD_ONLINE commands.
*/
VOID ASM Online(STDSLIPARGS)
{

    if(!(slipUnit->su_State     & SLIPUF_ONLINE))
    {
        /* We're offline. Try to go online. */
        if(OpenSerial(slipUnit,slipDevice))
        {
            if(slipUnit->su_State &     SLIPUF_ONLINE)
            {
                /* In case someone wants to know...*/
                DoEvent(S2EVENT_ONLINE,slipUnit,slipDevice);
            }
            else
            {
                /* Sorry, the attempt to go online failed. */
                ios2->ios2_Req.io_Error = S2ERR_OUTOFSERVICE;
                ios2->ios2_WireError = S2WERR_UNIT_OFFLINE;
            }
        }
        else
        {
            /* A general problem occured. */
            ios2->ios2_Req.io_Error = S2ERR_NO_RESOURCES;
            ios2->ios2_WireError = S2WERR_GENERIC_ERROR;
        }
    }
    TermIO(ios2,slipDevice);
}

/*
** This routine handles CMD_OFFLINE commands.
*/
VOID ASM Offline(STDSLIPARGS)
{
    TermIO(ios2,slipDevice);

    if(slipUnit->su_State &     SLIPUF_ONLINE)
    {
        /* We're online, so shut everything down. */
        CloseSerial(slipUnit,slipDevice);
        DoOffline(slipUnit,slipDevice);
        DoEvent(S2EVENT_OFFLINE,slipUnit,slipDevice);
    }
}

/*
** This routine is called whenever an "important"
** SANA-II event occurs.
*/
VOID DoEvent(ULONG events,
             struct SLIPUnit *slipUnit,
             struct SLIPDevice *slipDevice)
{
    struct IOSana2Req *ios2;
    struct IOSana2Req *ios2_next;

    ObtainSemaphore(&slipUnit->su_ListLock);

    ios2 = (struct IOSana2Req *)slipUnit->su_Events.mlh_Head;

    while(ios2->ios2_Req.io_Message.mn_Node.ln_Succ)
    {
        ios2_next = (struct IOSana2Req *)ios2->ios2_Req.io_Message.mn_Node.ln_Succ;

        /* Are they waiting for any of these events? */
        if(ios2->ios2_WireError & events)
        {
            ios2->ios2_Req.io_Error = 0;
            ios2->ios2_WireError = events;
            Remove((struct Node *)ios2);
            TermIO(ios2,slipDevice);
        }
        ios2 = ios2_next;
    }
    ReleaseSemaphore(&slipUnit->su_ListLock);
}

/*
** This routine is called whenever the device needs to
** be taken offline.  We return any pending CMD_READ's
** or CMD_WRITE's to their senders.
*/
VOID DoOffline(struct SLIPUnit *slipUnit,
               struct SLIPDevice *slipDevice)
{
    struct IOSana2Req *ios2;
    struct BufferManagement *bm;
    ObtainSemaphore(&slipUnit->su_ListLock);

    bm = (struct BufferManagement *)slipUnit->su_BuffMgmt.mlh_Head;
    while(bm->bm_Node.mln_Succ)
    {
        while(ios2 = (struct IOSana2Req *)RemHead((struct List *)&bm->bm_RxQueue))
        {
            ios2->ios2_Req.io_Error = S2ERR_OUTOFSERVICE;
            ios2->ios2_WireError = S2WERR_UNIT_OFFLINE;
            TermIO(ios2,slipDevice);
        }
        bm = (struct BufferManagement *)bm->bm_Node.mln_Succ;
    }
    while(ios2 = (struct IOSana2Req *)GetMsg(slipUnit->su_Tx))
    {
        ios2->ios2_Req.io_Error = S2ERR_OUTOFSERVICE;
        ios2->ios2_WireError = S2WERR_UNIT_OFFLINE;
        TermIO(ios2,slipDevice);
    }
    ReleaseSemaphore(&slipUnit->su_ListLock);
}

/*
** This routine is called whenever a CMD_WRITE request
** has returned from the serial driver.
*/
VOID ServiceTxPort(struct SLIPUnit *slipUnit,
                   struct SLIPDevice *slipDevice)
{
    struct IOSana2Req *ios2;

    /* See if our IORequest is busy. If not, check for a pending
       CMD_WRITE request and send it. */

    if(CheckIO((struct IORequest *)slipUnit->su_SerTx))
    {
        WaitIO((struct IORequest *)slipUnit->su_SerTx);
        if(ios2 = (struct IOSana2Req *)GetMsg(slipUnit->su_Tx))
        {
            SendPacket(ios2, slipUnit, slipDevice);
            slipUnit->su_Continue =     TRUE;
        }
    }
}

/*
** This routine is called whenever a CMD_READ request
** returns from the serial driver.  It decodes the
** packet data and tries to build complete packets.
*/
VOID DoSerial(struct IOExtSer *ioSer,
              struct SLIPUnit *slipUnit,
              struct SLIPDevice *slipDevice)
{
    UBYTE *rx_ptr,*packet_ptr;
    UBYTE rx_byte, packet_byte;
    ULONG length,startptr;

    startptr = (ULONG)slipUnit->su_RxBuff;
    packet_ptr = slipUnit->su_RxBuffPtr;
    rx_ptr = slipUnit->su_RxSLIP;

    length = ioSer->IOSer.io_Actual;

    while(length--)
    {
        rx_byte = packet_byte = *rx_ptr;
        rx_ptr++;

        /* Handle SLIP packet decoding...*/
        if(slipUnit->su_Escape)
        {
            if(rx_byte == SLIP_ESC_ESC)
                packet_byte = SLIP_ESC;
            else if(rx_byte == SLIP_ESC_END)
                packet_byte = SLIP_END;
            else
                ReceivedGarbage(slipUnit,slipDevice);   /* This packet may be hosed */

            slipUnit->su_Escape = FALSE;
        }
        else if(rx_byte == SLIP_ESC)
        {
            slipUnit->su_Escape = TRUE;
            continue;
        }
        else if(rx_byte == SLIP_END)
        {
            GotPacket((ULONG)(packet_ptr - startptr) - 128, slipUnit, slipDevice);
            packet_ptr = slipUnit->su_RxBuff + 128;
            continue;
        }
        *packet_ptr = packet_byte;
        packet_ptr++;

        if(((ULONG)(packet_ptr - startptr) - 128) > slipUnit->su_MTU)
        {
            packet_ptr = (UBYTE *)(startptr + 128);
            PacketOverrun(slipUnit,slipDevice);
        }
    }
    slipUnit->su_RxBuffPtr = packet_ptr;

    /* Queue up another CMD_READ request...*/
    QueueSerRequest(slipUnit,slipDevice);
}

/*
** This routine is called whenever we think we've got
** a complete packet to satisfy a CMD_READ request.
*/
VOID GotPacket(ULONG length,
               struct SLIPUnit *slipUnit,
               struct SLIPDevice *slipDevice)
{
    struct IOSana2Req *ios2;
    struct BufferManagement *bm;
    struct ip *iphdr;
    UBYTE *hdr;
    UBYTE type;

    /* We ignore zero-length packets. These occur
       in between legal SLIP packets due to the
       way SLIP packets are framed. */

    if(length)
    {
        PacketReceived(length,slipUnit);

        /* Check out what type of packet we have. */

        hdr = (UBYTE *)slipUnit->su_RxBuff + 128;
        type = hdr[0];

        if(type & 0x80)     /* Compressed TCP */
        {
            type = 0x80;
        }
        else
        {
            if(type >= 0x70)        /* Uncompressed TCP */
            {
                hdr[0] = type & 0xCF;       /* Fix IP header */
                type = 0x70;
            }
        }
        if((type == 0x70) || (type == 0x80))        /* TCP Packet */
        {
            if(slipUnit->su_State &     SLIPUF_AUTOCOMP)
                slipUnit->su_State |= SLIPUF_MAYCOMPRESS;

            slipUnit->su_MBuff.m_off = hdr;
            slipUnit->su_MBuff.m_len = length;
            length = sl_uncompress_tcp(&slipUnit->su_MBuff.m_off,length,type,&slipUnit->su_SLCompress);
            hdr = slipUnit->su_MBuff.m_off;
        }
        else        /* Just a normal SLIP IP Packet */
        {
            hdr = slipUnit->su_RxBuff + 128;
        }
        iphdr = (struct ip *)hdr;

        if(length)
        {
            BOOL dropped = TRUE; /* Set to false if at least one protocol stack
                                  * managed to get the packet.  I don't keep
                                  * individual statistics for each protocol
                                  * stack. */

            ObtainSemaphore(&slipUnit->su_ListLock);
            bm = (struct BufferManagement *)slipUnit->su_BuffMgmt.mlh_Head;

            while(bm->bm_Node.mln_Succ)
            {
                if(ios2 = (struct IOSana2Req *)RemHead((struct List *)&bm->bm_RxQueue))
                {
                    /* Make the IO request look like it would if they
                     * accept the packet. */

                    ios2->ios2_DataLength = length;         /* Length */
                    ios2->ios2_PacketType = 2048;	    /* IP Packet Type */
                    *(ULONG *)ios2->ios2_SrcAddr = iphdr->ip_src.s_addr;
                    *(ULONG *)ios2->ios2_DstAddr = iphdr->ip_dst.s_addr;

                    /* If they don't want the packet, go on to the next
                     * protocol stack. */

                    if((*(HOOK_FUNC)bm->bm_PacketFilterHook->h_Entry)(bm->bm_PacketFilterHook,ios2,hdr))
                    {
                        /* Copy the data into the protocol stack's buffer using its
                           supplied callback routine. */

                        if((*bm->bm_CopyToBuffer)(ios2->ios2_Data,hdr,length))
                        {
                            dropped = FALSE;
                        }
                        else
                        {
                            ios2->ios2_DataLength = 0;
                            ios2->ios2_Req.io_Error = S2ERR_NO_RESOURCES;
                            ios2->ios2_WireError = S2WERR_BUFF_ERROR;
                            DoEvent(S2EVENT_RX|S2EVENT_BUFF,slipUnit,slipDevice);
                        }
                        TermIO(ios2,slipDevice);

                    }
                    else /* They didn't want it, so put the IOSana2Req back on the list */
                    {
                        AddHead((struct List *)&bm->bm_RxQueue,(struct Node *)ios2);
                    }
                }
                bm = (struct BufferManagement *)bm->bm_Node.mln_Succ;
            }
            ReleaseSemaphore(&slipUnit->su_ListLock);

            if(dropped)
            {
                PacketDropped(slipUnit,slipDevice);
            }
        }
    }
    slipUnit->su_Escape = 0;
}

/*
** This routine is called whenever we need to
** get more data from the serial port.  We first
** to a SDCMD_QUERY to see how much data is available,
** if any.
*/
VOID QueueSerRequest(struct SLIPUnit *slipUnit,
                     struct SLIPDevice *slipDevice)
{
    slipUnit->su_SerRx->IOSer.io_Command = SDCMD_QUERY;
    DoIO((struct IORequest *)slipUnit->su_SerRx);

    if(slipUnit->su_State & SLIPUF_CD)
    {
        if(slipUnit->su_SerRx->io_Status & (1<<5))
        {
            /* Oops! We've lost carrier. Go offline. */
            CloseSerial(slipUnit,slipDevice);
            DoOffline(slipUnit,slipDevice);
            DoEvent(S2EVENT_OFFLINE,slipUnit,slipDevice);
            return;
        }
    }
    slipUnit->su_SerRx->IOSer.io_Command = CMD_READ;
    slipUnit->su_SerRx->IOSer.io_Data = slipUnit->su_RxSLIP;
    slipUnit->su_SerRx->IOSer.io_Length = min(slipUnit->su_SerRx->IOSer.io_Actual,slipUnit->su_MTU);

    /* If the number of bytes available is zero, queue a request
       for one byte. */
    if(!slipUnit->su_SerRx->IOSer.io_Length)
        slipUnit->su_SerRx->IOSer.io_Length = 1;

    SendIO((struct IORequest *)slipUnit->su_SerRx);
}

/*
** This is the C entry point for the Unit process.
** A6 has been set up by the assembly stub in slip_dev.asm.
*/

VOID ASM DevProcCEntry(REG(a6) struct SLIPDevice *slipDevice)
{
    struct Process *proc;
    struct SLIPUnit *slipUnit;
    struct IOExtSer *ioser;
    struct StartupMessage *sm;
    struct BufferManagement *bm;
    struct IOSana2Req *ios2;
    ULONG waitmask,signals;
    UBYTE signalbit;

    /* Find our Process pointer and wait for our startup
       message to arrive. */

    proc = (struct Process *)FindTask(0L);

    WaitPort(&proc->pr_MsgPort);

    /* Pull the startup message off of our process messageport. */
    sm = (struct StartupMessage *)GetMsg(&proc->pr_MsgPort);

    /* Grab our Unit pointer. */
    slipUnit    = (struct SLIPUnit *)sm->Unit;

    /* Attempt to allocate a signal bit for our Unit MsgPort. */
    signalbit = AllocSignal(-1L);
    if(signalbit != -1)
    {
        /* Set up our Unit's MsgPort. */
        slipUnit->su_Unit.unit_MsgPort.mp_SigBit = signalbit;
        slipUnit->su_Unit.unit_MsgPort.mp_SigTask =     (struct Task *)proc;
        slipUnit->su_Unit.unit_MsgPort.mp_Flags = PA_SIGNAL;

        /* Initialize our list semaphore */
        InitSemaphore(&slipUnit->su_ListLock);

        /* Initialize our linked lists. */
        slipUnit->su_Tx = CreateMsgPort();

        /* Initialize the serial stuff.  If all goes okay,
           set slipUnit->su_Proc to pointer to our unit process.
           This will let the Unit init code know that were
           are okay. */

        if(InitSerial(slipUnit,slipDevice) && slipUnit->su_Tx)
            slipUnit->su_Proc = proc;
    }

    /* Reply to our startup message */
    ReplyMsg((struct Message *)sm);

    /* Check slipUnit->su_Proc to see if everything went okay up
       above. */
    if(slipUnit->su_Proc)
    {
        waitmask = (1L<<signalbit) | (1L<<slipUnit->su_RxPort->mp_SigBit) |
                   (1L<<slipUnit->su_TxPort->mp_SigBit) | (1L<<slipUnit->su_Tx->mp_SigBit)
                   | SIGBREAKF_CTRL_F;

        /* Loop...*/

        while(TRUE)
        {
            signals = Wait(waitmask);

            /* Have we been signaled to shut down? */
            if(signals & SIGBREAKF_CTRL_F)
                break;

	    do
            {
                slipUnit->su_Continue = FALSE;

                if(ios2 = (struct IOSana2Req *)GetMsg((struct MsgPort *)slipUnit))
                {
                    slipUnit->su_Continue = TRUE;
                    PerformIO(ios2,slipDevice);
                }
                if(ioser = (struct IOExtSer *)GetMsg(slipUnit->su_RxPort))
                {
                    slipUnit->su_Continue = TRUE;
                    if(slipUnit->su_State & SLIPUF_ONLINE)
                    {
                        DoSerial(ioser, slipUnit, slipDevice);
                    }
                }
                if(slipUnit->su_State & SLIPUF_ONLINE)
                {
                    ServiceTxPort(slipUnit,slipDevice);
                }

            }
            while(slipUnit->su_Continue);
        }
        /* If we're online, we need to shut everything down. */
        if(slipUnit->su_State & SLIPUF_ONLINE)
        {
            CloseSerial(slipUnit,slipDevice);
            FreeSignal(signalbit);
            while(bm = (struct BufferManagement *)RemHead((struct List *)&slipUnit->su_BuffMgmt))
                FreeMem(bm,sizeof(struct BufferManagement));

        }
        DeinitSerial(slipUnit,slipDevice);

#ifdef OWNDEVUNIT_SUPPORT
        if (slipUnit->su_ODUBase)
            CloseLibrary(slipUnit->su_ODUBase);
#endif
        /* Signal the other side we're exiting.  Make sure that
           we exit before they wake up by using the same trick
           when replying to Workbench startup messages. */

        Forbid();
        Signal((struct Task *)slipUnit->su_Proc,SIGBREAKF_CTRL_F);
    }
    /* Something went wrong in the init code.  Drop out. */
    else
    {
        if(signalbit)
            FreeSignal(signalbit);
    }
}

/* List init routine. */
VOID NewList(struct List *list)
{
    list->lh_Head = (struct Node *)&list->lh_Tail;
    list->lh_Tail = NULL;
    list->lh_TailPred = (struct Node *)list;
}
