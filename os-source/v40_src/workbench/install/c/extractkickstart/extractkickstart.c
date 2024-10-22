
/* when run, returns:
 *   0 - success
 *   5 - not a kickstart disk in drive
 *  10 - failed when reading disk
 *  15 - failed when writing file
 *  20 - general failure (no memory or bad cmd-line)
 */

#include <exec/types.h>
#include <exec/memory.h>
#include <devices/trackdisk.h>
#include <string.h>
#include <stdlib.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>

#include "extractkickstart_rev.h"


/****************************************************************************/


#define TEMPLATE      "DEVICE/A,TO/A,1.3/S" VERSTAG
#define OPT_DEVICE    0
#define OPT_TO        1
#define OPT_13        2
#define OPT_COUNT     3


/****************************************************************************/


#define ID_KICK_DISK       ((ULONG)('K'<<24)|('I'<<16)|('C'<<8)|('K'))
#define ID_SUP0_DISK       ((ULONG)('S'<<24)|('U'<<16)|('P'<<8)|('0'))

#define DISK_BUF_SIZE	   (0x40000)

#define HEADER_SIZE        (0x400)
#define K1_3_SIZE          (0x40000)
#define K1_4_SIZE          (0x80000)

#define HEADER_OFFSET      (0x0)
#define K1_3_OFFSET        (0x400)
#define K1_4_OFFSET        (0x40400)
#define BONUS13_OFFSET     (0xC1400)
#define BONUS14_OFFSET     (0xC0400)


/****************************************************************************/


BOOL TDRead(struct IOExtTD *ioReq, ULONG byteOffset,
            APTR buffer, ULONG numBytes);


/****************************************************************************/


LONG main(VOID)
{
struct Library *DOSBase;
struct Library *SysBase = (*((struct Library **) 4));
struct Library *UtilityBase;
LONG            failureLevel = RETURN_FAIL;
LONG            failureCode = ERROR_INVALID_RESIDENT_LIBRARY;
struct RDArgs  *rdargs;
LONG            opts[OPT_COUNT];
STRPTR          drivename;
ULONG           unit;
struct MsgPort *ioPort;
struct IOExtTD *ioReq;
UBYTE          *buffer;
BPTR            output;
LONG           *lptr;
LONG            Bonus13_size;
LONG            Bonus14_size;

    DOSBase     = OpenLibrary("dos.library",36);
    UtilityBase = OpenLibrary("utility.library",36);

    if (DOSBase && UtilityBase)
    {
        memset(opts,0,sizeof(opts));
        if (rdargs = ReadArgs(TEMPLATE, opts, NULL))
        {
            drivename = (STRPTR)opts[OPT_DEVICE];

            if ((drivename[0] == 'd' || drivename[0] == 'D')
            &&  (drivename[1] == 'f' || drivename[1] == 'F')
            &&  (drivename[2] >= '0' && drivename[2] <= '9')
            &&  (drivename[3] == ':'))
            {
                unit = drivename[2] - '0';

                if (buffer = AllocVec(DISK_BUF_SIZE,MEMF_ANY))
                {
                    ioPort = CreateMsgPort();
                    if (ioReq = CreateIORequest(ioPort,sizeof(struct IOStdReq)))
                    {
                        Inhibit((STRPTR)opts[OPT_DEVICE],DOSTRUE);

                        if (OpenDevice("trackdisk.device",unit,ioReq,0) == 0)
                        {
                            failureLevel = 10;
                            if (TDRead(ioReq,0,buffer,512))
                            {
                                if (((*((ULONG *)&buffer[0])) != ID_KICK_DISK) || ((*((ULONG *)&buffer[4])) != ID_SUP0_DISK))
                                {
                                    failureLevel = 5;
                                }
                                else if (output = Open((STRPTR)opts[OPT_TO],MODE_NEWFILE))
                                {
                                    if (TDRead(ioReq,HEADER_OFFSET,buffer,HEADER_SIZE))
                                    {
                                        lptr         = (ULONG *)&buffer[8];
                                        Bonus13_size = *lptr;
                                        lptr         = (ULONG *)&buffer[12];
                                        Bonus14_size = *lptr;

                                        if (opts[OPT_13])
                                        {
                                            if (TDRead(ioReq,K1_3_OFFSET,buffer,K1_3_SIZE))
                                            if (Write(output,buffer,K1_3_SIZE) > 0)
                                            {
                                                if (TDRead(ioReq,BONUS13_OFFSET,buffer,Bonus13_size))
                                                if (Write(output,buffer,Bonus13_size) > 0)
                                                {
                                                    failureLevel = 0;
                                                }
                                                else
                                                {
                                                    failureLevel = 15;
                                                }
                                            }
                                            else
                                            {
                                                failureLevel = 15;
                                            }
                                        }
                                        else
                                        {
                                            if (TDRead(ioReq,K1_4_OFFSET,buffer,DISK_BUF_SIZE))
                                            if (Write(output,buffer,DISK_BUF_SIZE) > 0)
                                            {
                                                if (TDRead(ioReq,K1_4_OFFSET+DISK_BUF_SIZE,buffer,DISK_BUF_SIZE))
                                                if (Write(output,buffer,DISK_BUF_SIZE) > 0)
                                                {
                                                    if (TDRead(ioReq,BONUS14_OFFSET,buffer,Bonus14_size))
                                                    if (Write(output,buffer,Bonus14_size) > 0)
                                                    {
                                                        failureLevel = 0;
                                                    }
                                                    else
                                                    {
                                                        failureLevel = 15;
                                                    }
                                                }
                                                else
                                                {
                                                    failureLevel = 15;
                                                }
                                            }
                                            else
                                            {
                                                failureLevel = 15;
                                            }
                                        }
                                    }
                                    Close(output);
                                    SetProtection((STRPTR)opts[OPT_TO],FIBF_EXECUTE);
                                }
                                else
                                {
                                    failureLevel = 15;
                                }
                            }

                            if (failureLevel == 15)
                            {
                                failureCode = IoErr();
                                PutStr("Couldn't write ");
                                PutStr((STRPTR)opts[OPT_TO]);
                                PutStr(" - ");
                            }
                            else if (failureLevel == 10)
                            {
                                failureCode = ERROR_SEEK_ERROR;
                                PutStr("Couldn't read from ");
                                PutStr((STRPTR)opts[OPT_DEVICE]);
                                PutStr(" - ");
                            }
                            else if (failureLevel == 5)
                            {
                                failureCode = ERROR_OBJECT_WRONG_TYPE;
                                PutStr((STRPTR)opts[OPT_DEVICE]);
                                PutStr(" does not contain a SuperKickstart disk - ");
                            }
                            else
                            {
                                failureCode = 0;
                            }

                            CloseDevice(ioReq);
                        }
                        else
                        {
                            failureCode  = ERROR_DEVICE_NOT_MOUNTED;
                            failureLevel = 10;
                        }

                        Inhibit((STRPTR)opts[OPT_DEVICE],DOSFALSE);
                    }

                    DeleteMsgPort(ioPort);
                    DeleteIORequest(ioReq);
                    FreeVec(buffer);
                }
                else
                {
                    failureCode = ERROR_NO_FREE_STORE;
                }
            }
            else
            {
                failureCode = ERROR_OBJECT_WRONG_TYPE;
            }

            FreeArgs(rdargs);
        }
        else
        {
            failureCode = IoErr();
        }
    }

    if (DOSBase)
    {
        if (failureCode)
            PrintFault(failureCode,NULL);

        CloseLibrary(UtilityBase);
        CloseLibrary(DOSBase);
    }

    return(failureLevel);
}


/****************************************************************************/


BOOL TDRead(struct IOExtTD *ioReq, ULONG byteOffset,
            APTR buffer, ULONG numBytes)
{
struct Library *SysBase = (*((struct Library **) 4));
BOOL            result;

   ioReq->iotd_Req.io_Length  = numBytes;
   ioReq->iotd_Req.io_Data    = buffer;
   ioReq->iotd_Req.io_Command = CMD_READ;
   ioReq->iotd_Req.io_Offset  = byteOffset;
   result = (DoIO(ioReq) == 0);

   ioReq->iotd_Req.io_Length  = 0;
   ioReq->iotd_Req.io_Command = TD_MOTOR;
   DoIO(ioReq);

   return(result);
}
