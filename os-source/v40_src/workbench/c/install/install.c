
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* |_o_o|\\ Copyright (c) 1989 The Software Distillery.  All Rights Reserved */
/* |. o.| || This program may not be distributed without the permission of   */
/* | .	| || the authors:			      BBS: (919) 460-7430    */
/* | o	| ||   Dave Baker      Alan Beale	  Jim Cooper		     */
/* |  . |//    Jay Denebeim    Bruce Drake	  Gordon Keener 	     */
/* ======      John Mainwaring Andy Mercier	  Jack Rouse		     */
/*	       John Toebes     Mary Ellen Toebes  Doug Walker	Mike Witcher */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**	(C) Copyright 1989 Commodore-Amiga, Inc.
 **	    All Rights Reserved
**/

#include "internal/commands.h"
#include <devices/trackdisk.h>
#include "install_rev.h"

/****************************************************************************/


VOID kprintf(STRPTR fmt,...);
#define D_S(type,name) char a_##name[sizeof(type)+3]; \
		       type *name = (type *)((LONG)(a_##name+3) & ~3);


/****************************************************************************/


#define MSG_INSTALL_FAIL   "Install failed\n"
#define MSG_NO_BOOTBLOCK   "No bootblock installed\n"
#define MSG_BOOTBLOCK	   "Appears to be %s %sFS bootblock\n"
#define MSG_OLD_OP	   "1.2/1.3"
#define MSG_NEW_OP	   "2.0"
#define MSG_OFS 	   "O"
#define MSG_FFS 	   "F"
#define MSG_DC_OFS 	   "DC-O"
#define MSG_DC_FFS 	   "DC-F"
#define MSG_BOOT_ABNORM    "May not be a standard bootblock\n"
#define MSG_BAD_BLOCK_SIZE "Volume does not have 512 bytes per sector\n"
#define MSG_BAD_DISK       "Unable to read from or write to volume\n"


/****************************************************************************/


#define TEMPLATE   "DRIVE/A,NOBOOT/S,CHECK/S,FFS/S" CMDREV
#define OPT_DRIVE  0
#define OPT_NOBOOT 1
#define OPT_CHECK  2
#define OPT_FFS    3   /* ignored */
#define OPT_COUNT  4

#define OB_LEN	   42		/* number of bytes in the 1.x BootBlock image */
#define BB_LEN	   86		/*   "    "    "    "  "  2.x     "       "   */


/****************************************************************************/


/* because <dos/dos.h> has these incorrectly defined in 2.04 */
#ifdef ID_INTER_DOS_DISK
#undef ID_INTER_DOS_DISK
#endif

#ifdef ID_INTER_FFS_DISK
#undef ID_INTER_FFS_DISK
#endif

#define ID_INTER_DOS_DISK (0x444F5302L)	/* 'DOS\2' */
#define ID_INTER_FFS_DISK (0x444F5303L)	/* 'DOS\3' */
#define ID_DC_DOS_DISK    (0x444F5304L)	/* 'DOS\4' */
#define ID_DC_FFS_DISK    (0x444F5305L)	/* 'DOS\5' */

#define ERROR_BAD_DISK    1

#define CCD_NAME "carddisk.device";


/****************************************************************************/


LONG main(VOID)
{
struct Library       *SysBase = (*((struct Library **) 4));
struct Library       *DOSBase;
LONG                  i;
STRPTR                msg;
LONG                  msg2[2];
ULONG                *bblock;
ULONG                 temp;
ULONG                 opts[OPT_COUNT];
BPTR                  dlock;
ULONG                 disk_id;
struct MsgPort       *diskport;
struct IOStdReq      *diskreq;
struct RDargs        *rdargs;
struct DriveGeometry  geometry;
STRPTR                devicename;
STRPTR                drivename;
LONG                  failureCode;
LONG                  failureLevel;
/* Image of the BootBlock of a 1.2/1.3 bootable disk.                      */
char *ob_img = "\0\0\3\x70\x43\xFA\0\x18\x4E\xAE\xFF\xA0\x4A\x80\x67\x0A\x20\x40\x20\x68\0\x16\x70\0\x4E\x75\x70\xFF\x60\xFAdos.library\0";
/* Image of the BootBlock of a 2.0 bootable disk.                          */
char *bb_img = "\0\0\3\x70\x43\xFA\0\x3E\x70\x25\x4E\xAE\xFD\xD8\x4A\x80\x67\xC\x22\x40\x8\xE9\0\6\0\x22\x4E\xAE\xFE\x62\x43\xFA\0\x18\x4E\xAE\xFF\xA0\x4A\x80\x67\xA\x20\x40\x20\x68\0\x16\x70\0\x4E\x75\x70\xFF\x4E\x75dos.library\0\expansion.library\0";
D_S(struct InfoData,idata);

    failureLevel = RETURN_FAIL;
    msg          = MSG_INSTALL_FAIL;
    bblock       = NULL;
    dlock        = NULL;
    diskport     = &THISPROC->pr_MsgPort;

    if (DOSBase = OpenLibrary("dos.library",37))
    {
        memset((char *)opts, 0, sizeof(opts));
        rdargs = ReadArgs(TEMPLATE, opts, NULL);

        if ((rdargs == NULL)
        || ((dlock = Lock((char *)opts[OPT_DRIVE],ACCESS_READ)) == NULL)
        || (Info(dlock,idata) == NULL))
        {
            failureCode = IoErr();
        }
        else
        {
            failureCode = ERROR_NO_FREE_STORE;
            if (diskreq = CreateIORequest(diskport,sizeof(struct IOStdReq)))
            {
                drivename  = (STRPTR)opts[OPT_DRIVE];
                devicename = NULL;

                if ((drivename[0] == 'c' || drivename[0] == 'C')
                &&  (drivename[1] == 'c' || drivename[1] == 'C')
                &&  (drivename[2] >= '0' && drivename[2] <= '9'))
                {
                    devicename = CCD_NAME;
                }
                if ((drivename[0] == 'd' || drivename[0] == 'D')
                &&  (drivename[1] == 'f' || drivename[1] == 'F')
                &&  (drivename[2] >= '0' && drivename[2] <= '9'))
                {
                    devicename = TD_NAME;
                }

                failureCode = ERROR_OBJECT_WRONG_TYPE;
                if (devicename)
                {
                    if (!OpenDevice(devicename,((STRPTR)opts[OPT_DRIVE])[2]-'0',diskreq,0))
                    {
                        diskreq->io_Command = TD_GETGEOMETRY;
                        diskreq->io_Length  = sizeof(struct DriveGeometry);
                        diskreq->io_Data    = &geometry;
                        if ((DoIO(diskreq) == 0) && (geometry.dg_SectorSize != TD_SECTOR))
                        {
                            failureCode = ERROR_OBJECT_WRONG_TYPE;
                            msg         = MSG_BAD_BLOCK_SIZE;
                            goto err;
                        }

                        failureCode = ERROR_NO_FREE_STORE;
                        if (bblock = AllocVec(TD_SECTOR*2,MEMF_CLEAR))
                        {
                            diskreq->io_Command = TD_PROTSTATUS;
                            diskreq->io_Length = 1;
                            if (DoIO(diskreq) == 0)
                            {
                                if ((diskreq->io_Actual) && (!(BOOL)opts[OPT_CHECK]))
                                {
                                    failureCode = ERROR_DISK_WRITE_PROTECTED;
                                    goto err;
                                }
                            }

                            failureCode = ERROR_BAD_DISK;

                            diskreq->io_Length  = TD_SECTOR*2;
                            diskreq->io_Offset  = NULL;
                            diskreq->io_Command = CMD_READ;
                            diskreq->io_Data    = (APTR)bblock;

                            if (DoIO(diskreq) == 0)
                            {
                                if ((BOOL)opts[OPT_CHECK])
                                {
                                    /* just check the existing boot block */
                                    failureLevel = RETURN_OK;
                                    msg          = NULL;

                                    msg2[0] = 0;
                                    if ((bblock[1] == NULL)
                                    ||  (bblock[1] == ID_DOS_DISK)
                                    ||  (bblock[1] == ID_FFS_DISK)
                                    ||  (bblock[1] == ID_INTER_DOS_DISK)
                                    ||  (bblock[1] == ID_INTER_FFS_DISK)
                                    ||  (bblock[1] == ID_DC_DOS_DISK)
                                    ||  (bblock[1] == ID_DC_FFS_DISK))
                                    {
                                        PutStr(MSG_NO_BOOTBLOCK);
                                    }
                                    else
                                    {
                                        if (memcmp((char *)(&bblock[2]),bb_img, BB_LEN) == NULL)
                                        {
                                            msg2[0] = (long)MSG_NEW_OP;
                                        }
                                        else
                                        {
                                            if (memcmp((char *)(&bblock[2]),ob_img, OB_LEN) == NULL)
                                            {
                                                msg2[0] = (long)MSG_OLD_OP;
                                            }
                                        }

                                        if ((bblock[0] == ID_DOS_DISK) || (bblock[0] == ID_INTER_DOS_DISK))
                                        {
                                            msg2[1] = (long)MSG_OFS;
                                        }
                                        else if ((bblock[0] == ID_FFS_DISK) || (bblock[0] == ID_INTER_FFS_DISK))
                                        {
                                            msg2[1] = (long)MSG_FFS;
                                        }
                                        else if (bblock[0] == ID_DC_DOS_DISK)
                                        {
                                            msg2[1] = (long)MSG_DC_OFS;
                                        }
                                        else if (bblock[0] == ID_DC_FFS_DISK)
                                        {
                                            msg2[1] = (long)MSG_DC_FFS;
                                        }

                                        if (msg2[0])
                                        {
                                            VPrintf(MSG_BOOTBLOCK, msg2);
                                        }
                                        else
                                        {
                                            PutStr(MSG_BOOT_ABNORM);
                                        }
                                    }
                                }
                                else
                                {
                                    /* actually write a boot block */
                                    disk_id = bblock[0];

                                    memset((char *)bblock, 0, (TD_SECTOR*2));
                                    if (!(BOOL)opts[OPT_NOBOOT])
                                    {
                                        movmem(bb_img,(char *)&bblock[2], BB_LEN);
                                    }

                                    if ((disk_id != ID_DOS_DISK)
                                    &&  (disk_id != ID_FFS_DISK)
                                    &&  (disk_id != ID_INTER_DOS_DISK)
                                    &&  (disk_id != ID_INTER_FFS_DISK)
                                    &&  (disk_id != ID_DC_DOS_DISK)
                                    &&  (disk_id != ID_DC_FFS_DISK))
                                    {
                                        disk_id = ID_DOS_DISK;
                                    }

                                    bblock[0] = disk_id;

                                    if (!(BOOL)opts[OPT_NOBOOT])
                                    {
                                        /*-----------------------------------------------------------*/
                                        /* Now we need to calculate the CheckSum for the BootBlock.  */
                                        /*-----------------------------------------------------------*/
                                        temp = bblock[1] = bblock[0];
                                        for (i=2; i<(((BB_LEN+3)>>2)+2); i++)
                                        {
                                            bblock[1] += bblock[i];
                                            if (bblock[1] < temp)
                                            {
                                                ++bblock[1];
                                            }
                                            temp = bblock[1];
                                        }
                                        bblock[1] = ~bblock[1];
                                    }

                                    diskreq->io_Command = CMD_WRITE;
                                    diskreq->io_Data = (APTR)bblock;
                                    if (DoIO(diskreq) == 0)
                                    {
                                        diskreq->io_Command = CMD_UPDATE;
                                        if (DoIO(diskreq) == 0)
                                        {
                                            msg          = NULL;
                                            failureLevel = RETURN_OK;
                                        }
                                    }
                                }
                            }

                            diskreq->io_Command = TD_MOTOR;
                            diskreq->io_Length  = 0;
                            DoIO(diskreq);
                        }
                    }

err:                if (diskreq->io_Device)
                        CloseDevice(diskreq);
                }
                DeleteIORequest(diskreq);
            }
        }

        UnLock(dlock);
        FreeVec(bblock);

        if (rdargs)
            FreeArgs(rdargs);

        if (failureLevel)
        {
            if (failureCode == ERROR_BAD_DISK)
            {
                msg = MSG_BAD_DISK;
            }
            else if (failureCode)
            {
                PrintFault(failureCode,NULL);
                SetIoErr(failureCode);
            }
        }

        if (msg)
            PutStr(msg);

        CloseLibrary(DOSBase);
    }

    return(failureLevel);
}
