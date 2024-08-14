
#include <exec/types.h>
#include <exec/ports.h>
#include <exec/libraries.h>
#include <exec/devices.h>
#include <exec/io.h>
#include <exec/execbase.h>
#include <exec/errors.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/filehandler.h>
#include <devices/serial.h>
#include <devices/parallel.h>
#include <devices/printer.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/utility_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/utility_pragmas.h>

#include "port-handler_rev.h"


/*****************************************************************************/


struct IOBlock
{
    union
    {
        struct IOStdReq    ios;
        struct IOPrtCmdReq iopc;
        struct IOExtSer    ioser;
        struct IOExtPar    iopar;
    } devIO;

    struct DosPacket *sourcePacket;
};

#define IO(iob)  ((struct IOStdReq *)iob)
#define SER(iob) ((struct IOExtSer *)iob)


/*****************************************************************************/


struct GlobalData
{
    struct ExecBase  *gd_SysBase;
    struct MsgPort   *gd_PacketPort;
    struct DosPacket *gd_Packet;
};

#define SysBase    gd->gd_SysBase
#define packet     gd->gd_Packet
#define packetPort gd->gd_PacketPort


/*****************************************************************************/


VOID kprintf(STRPTR,...);
static VOID ReturnPkt(struct GlobalData *gd, LONG res1, LONG res2);


/*****************************************************************************/


/* this guy is being started as a BCPL handler, startup packet is all ready for us */
VOID __asm Handler(register __a0 struct DosPacket *startupPacket)
{
struct GlobalData  global;
struct GlobalData *gd;
struct Library    *DOSBase;
struct Library    *UtilityBase;
struct Message    *message;
struct FileHandle *fh;
struct IOBlock    *ioReq;
struct MsgPort    *ioPort;
struct DosList    *devNode;
struct DosList    *dl;
LONG               result1;
LONG               result2;
BOOL               printerRaw;
BOOL               printer;
BOOL               serial;
BOOL               doIO;
BOOL               setparms;
BOOL               fail;
STRPTR             device;
ULONG              unit;
ULONG              flags;
STRPTR             control;
ULONG              baud;
UWORD              bits;
char               parity;
UWORD              stopBits;
UWORD              i;
LONG               num;
STRPTR             name;
struct FileSysStartupMsg *fssm;
struct DosEnvec          *env;

    gd      = &global;
    SysBase = (*((struct ExecBase **) 4));

    packet  = startupPacket;
    devNode = (struct DosList *)BADDR(packet->dp_Arg3);

    /* We need V37 to run, so fail with anything older */
    if (SysBase->LibNode.lib_Version < 37)
    {
        fail = TRUE;
    }
    else
    {
        fail       = FALSE;
        ioPort     = CreateMsgPort();
        packetPort = CreateMsgPort();

        if ((!ioPort) || (!packetPort))
        {
            DeleteMsgPort(ioPort);
            DeleteMsgPort(packetPort);
            fail = TRUE;
        }
    }

    if (fail)
    {
        Forbid();                             /* needed? */
        ReturnPkt(gd,DOSFALSE,RETURN_FAIL);   /* return startup packet */
        return;                               /* fall off the face of the earth */
    }

    DOSBase     = OpenLibrary("dos.library" VERSTAG,37);
    UtilityBase = OpenLibrary("utility.library",37);

    /* scan for all device nodes which have the same handler string, and
     * patch their seglist field to point to us. Since we're pure, no sense
     * in reloading the code all the time...
     */
    dl = LockDosList(LDF_DEVICES|LDF_WRITE);
    while (dl = NextDosEntry(dl,LDF_DEVICES|LDF_WRITE))
    {
        if (dl->dol_misc.dol_handler.dol_Handler)
        {
            if (Stricmp(BADDR(dl->dol_misc.dol_handler.dol_Handler),BADDR(devNode->dol_misc.dol_handler.dol_Handler)) == 0)
            {
                dl->dol_misc.dol_handler.dol_SegList = devNode->dol_misc.dol_handler.dol_SegList;
            }
        }
    }

    UnLockDosList(LDF_DEVICES|LDF_WRITE);

    /* reply the startup packet, indicating we're ready for business */
    devNode->dol_Task = packetPort; /* send packets here please... */
    ReturnPkt(gd,DOSTRUE,0);

    while (TRUE)
    {
        while (message = GetMsg(packetPort))
        {
            packet = (struct DosPacket *)message->mn_Node.ln_Name;

            /* true for every packet we care about, except for ACTION_FINDXXX ones */
            ioReq  = (struct IOBlock *)packet->dp_Arg1;

            switch (packet->dp_Type)
            {
                case ACTION_FINDINPUT :
                case ACTION_FINDOUTPUT:
                case ACTION_FINDUPDATE:

                           printer    = FALSE;
                           serial     = FALSE;

                           /* set some defaults */
                           unit       = 0;
                           flags      = 0;

                           setparms   = FALSE;
                           baud       = 9600;
                           bits       = 8;
                           parity     = 'N';
                           stopBits   = 1;
                           printerRaw = FALSE;

                           /* figure out what we are talking to */
                           switch (devNode->dol_misc.dol_handler.dol_Startup)
                           {
                               case 1 : device = "parallel.device";
                                        break;

                               case 2 : device  = "printer.device";
                                        printer = TRUE;
                                        break;

                               default: device = "serial.device";
                                        serial = TRUE;

                                        fssm = (struct FileSysStartupMsg *)BADDR(devNode->dol_misc.dol_handler.dol_Startup);
                                        if (((ULONG)fssm > 64) && (TypeOfMem(fssm)))
                                        {
                                            device = (STRPTR)((ULONG)BADDR(fssm->fssm_Device) + 1);
                                            unit   = fssm->fssm_Unit;
                                            flags  = fssm->fssm_Flags;

                                            if (env = (struct DosEnvec *)BADDR(fssm->fssm_Environ))
                                            {
                                                setparms = TRUE;
                                                if (env->de_TableSize >= DE_BAUD)
                                                    baud = env->de_Baud;

                                                if (env->de_TableSize >= DE_CONTROL)
                                                {
                                                    /* the following gets all confused if the string is not in the right format */
                                                    if (control = (STRPTR)BADDR(env->de_Control))
                                                    {
                                                        /* string looks like: "\x03\"8N1\"" */
                                                        bits = control[2] - '0';

                                                        parity = control[3];

                                                        stopBits = 0;
                                                        if (control[4] == '1')
                                                            stopBits = 1;
                                                    }
                                                }
                                            }
                                        }
                           }

                           /* skip over part of name that has a colon */
                           name = (STRPTR)((ULONG)BADDR(packet->dp_Arg3) + 1);
                           i = 0;
                           while ((name[i] != ':') && name[i])
                               i++;

                           if (name[i])
                               i++;

                           if (name[i])
                           {
                              if (printer)
                              {
                                  if (Stricmp(&name[i],"RAW") == 0)
                                      printerRaw = TRUE;
                              }
                              else if (serial)
                              {
                                  num = StrToLong(&name[i],&baud);
                                  if (num >= 0)
                                  {
                                      setparms = TRUE;
                                      i += num;

                                      if (name[i++] == '/')
                                      {
                                          if (name[i])
                                          {
                                              bits = name[i++] - '0';
                                              if (name[i])
                                              {
                                                  parity = name[i++];
                                                  if (name[i])
                                                  {
                                                      stopBits = 0;
                                                      if (name[i] == '1')
                                                          stopBits = 1;
                                                  }
                                              }
                                          }
                                      }
                                  }
                              }
                           }

                           result1 = DOSFALSE;
                           result2 = ERROR_NO_FREE_STORE;

                           if (ioReq = CreateIORequest(ioPort,sizeof(struct IOBlock)))
                           {
                               if (OpenDevice(device,unit,IO(ioReq),flags) == 0)
                               {
                                   /* mark as interactive, don't ask me why... */
                                   fh          = (struct FileHandle *) BADDR(packet->dp_Arg1);
                                   fh->fh_Port = (struct MsgPort *)DOSTRUE;
                                   fh->fh_Arg1 = (LONG)ioReq;

                                   doIO = FALSE;

                                   if (printerRaw)
                                   {
                                       IO(ioReq)->io_Command = CMD_WRITE;
                                       IO(ioReq)->io_Data    = "\033[20l";
                                       IO(ioReq)->io_Length  = 5;
                                       doIO = TRUE;
                                   }

                                   if (setparms)
                                   {
                                       SER(ioReq)->IOSer.io_Command = SDCMD_SETPARAMS;
                                       SER(ioReq)->io_Baud          = baud;
                                       SER(ioReq)->io_ReadLen       = bits;
                                       SER(ioReq)->io_WriteLen      = bits;
                                       SER(ioReq)->io_StopBits      = stopBits;

                                       switch (parity)
                                       {
                                           case 'O': SER(ioReq)->io_SerFlags |=  SERF_PARTY_ON | SERF_PARTY_ODD; /* enable parity on and set parity odd */
		                                     SER(ioReq)->io_ExtFlags &= ~SEXTF_MSPON; /* disable mark/space parity */
		                                     break;

		                           case 'E': SER(ioReq)->io_SerFlags =  (SER(ioReq)->io_SerFlags & ~SERF_PARTY_ODD) | SERF_PARTY_ON; /* clear parity odd and enable parity on */
		                                     SER(ioReq)->io_ExtFlags &= ~SEXTF_MSPON; /* disable mark/space parity */
		                                     break;

                                           case 'N': SER(ioReq)->io_SerFlags &= ~SERF_PARTY_ON; /* disable parity on */
		                                     SER(ioReq)->io_ExtFlags &= ~SEXTF_MSPON; /* disable mark/space parity */
		                                     break;

	                                   case 'M': SER(ioReq)->io_ExtFlags |= SEXTF_MSPON | SEXTF_MARK; /* enable mark/space parity and set mark parity */
	                                             break;

	                                   case 'S': SER(ioReq)->io_ExtFlags = (SER(ioReq)->io_ExtFlags &~SEXTF_MARK) | SEXTF_MSPON; /* clear mark parity and enable mark/space parity */
	                                             break;
	                               }

                                       doIO = TRUE;
                                   }

                                   if (doIO)
                                   {
                                       if (DoIO(IO(ioReq)) != 0)  /* synchronous! */
                                       {
                                           result2 = IO(ioReq)->io_Error;
                                           CloseDevice(IO(ioReq));
                                           DeleteIORequest(IO(ioReq));
                                           break;
                                       }
                                   }

                                   result1 = DOSTRUE;
                                   result2 = 0;
                               }
                               else
                               {
                                   result2 = ERROR_OBJECT_NOT_FOUND;
                                   if (IO(ioReq)->io_Error == IOERR_UNITBUSY)
                                       result2 = ERROR_OBJECT_IN_USE;

                                   DeleteIORequest(IO(ioReq));
                               }
                           }
                           break;

                case ACTION_END  : CloseDevice(IO(ioReq));
                                   DeleteIORequest(IO(ioReq));
                                   result1 = DOSTRUE;
                                   result2 = 0;
                                   break;

                case ACTION_WRITE: IO(ioReq)->io_Command = CMD_WRITE;

                case ACTION_READ : if (packet->dp_Type == ACTION_READ)
                                       IO(ioReq)->io_Command = CMD_READ;

                                   IO(ioReq)->io_Data   = (APTR)packet->dp_Arg2;
                                   IO(ioReq)->io_Length = packet->dp_Arg3;
                                   ioReq->sourcePacket  = packet;
                                   SendIO(IO(ioReq));
                                   packet = NULL; /* don't reply packet until IO comes back */
                                   break;

                case ACTION_IS_FILESYSTEM: result1 = DOSFALSE;
                                           result2 = 0;
                                           break;

                default          : result1 = DOSFALSE;
                                   result2 = ERROR_ACTION_NOT_KNOWN;
                                   break;
            }

            if (packet)
                ReturnPkt(gd,result1,result2);
        }

        /* Device IO request returning. Extract associated packet, and return
         * the packet to the originator
         */
        while (ioReq = (struct IOBlock *)GetMsg(ioPort))
        {
            packet = ioReq->sourcePacket;
            if (IO(ioReq)->io_Error)
                ReturnPkt(gd,-1,IO(ioReq)->io_Error);
            else
                ReturnPkt(gd,IO(ioReq)->io_Actual,0);
        }

        Wait((1 << packetPort->mp_SigBit) | (1 << ioPort->mp_SigBit));
    }
}


/*****************************************************************************/


/* Use this instead of the ROM one, since we need a different reply port than
 * the process port, plus this particular function has to work in 1.3 so we
 * can fail init
 */
static VOID ReturnPkt(struct GlobalData *gd, LONG res1, LONG res2)
{
struct Message *message;
struct MsgPort *port;

    packet->dp_Res1          = res1;
    packet->dp_Res2          = res2;
    port                     = packet->dp_Port;
    message                  = packet->dp_Link;
    packet->dp_Port          = packetPort;
    message->mn_Node.ln_Name = (STRPTR)packet;

    PutMsg(port,message);
}
