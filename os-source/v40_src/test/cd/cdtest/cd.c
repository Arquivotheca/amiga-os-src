/*
**  cd.device Test
**  Written by John J. Szucs
**  Copyright © 1993 Commodore-Amiga, Inc.
**  All Rights Reserved
*/

/*
**  ANSI includes
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/*
**  System includes
*/

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/interrupts.h>
#include <exec/io.h>

#include <utility/tagitem.h>

#include <dos/dos.h>

#include <cd/cd.h>

#include <devices/trackdisk.h>

#include <rexx/rxslib.h>
#include <rexx/storage.h>
#include <rexx/errors.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/rexxsyslib_protos.h>
#include <clib/alib_protos.h>

/*
**  Local includes
*/

#include "simplerexx.h"
#include "cdtest.h"

/****** cd/cdAddChangeInt ******************************************
*
*   NAME
*       cdAddChangeInt -- add change interrupt
*
*   SYNOPSIS
*       result=cdAddChangeInt(rxMsg,args,error);
*
*       STRPTR cdAddChangeInt(struct RexxMsg *rxMsg,STRPTR args,LONG *error);
*
*   FUNCTION
*       Add change interrupt.
*
*   INPUTS
*       rxMsg   -   ARexx message
*       args    -   arguments from ARexx
*       error   -   pointer to error value
*
*   RESULT
*       *error  -   Error code (if any)
*       result  -   result string
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*       Normal procedure for handling exec.library I/O requests
*       requires that an I/O request be completed before it is
*       re-used. However, for historical reasons, the *_ADDCHANGEINT
*       and *_ADDFRAMEINT commands of trackdisk.device, scsi.device,
*       and cd.device require the caller to send the I/O request
*       to add the interrupt and re-use the I/O request (without
*       calling exec.library/WaitIO()) with a *_REMCHANGEINT
*       or *_REMFRAMEINT command to remove the interrupt.
*
*       This causes IOTorture hits and is, in my humble opinion,
*       a Bad Thing(TM). However, it is apparently well beyond the
*       point where this behavior could be changed.
*
*   SEE ALSO
*       cdRemChangeInt
*
******************************************************************************
*
*/

STRPTR cdAddChangeInt(struct RexxMsg *rxMsg,STRPTR args,LONG *error)
{

    /* Set-up disk change interrupt */
    changeInterrupt.is_Node.ln_Name="CD Disk Change";
    changeInterrupt.is_Data=(APTR) COOKIE_CHANGEINT;
    changeInterrupt.is_Code=(void *) changeInterruptCode;
    changeIntCall=changeIntBadData=0L;

    /* Add disk change interrupt */
    cdRequest->io_Command=CD_ADDCHANGEINT;
    cdRequest->io_Length=sizeof(changeInterrupt);
    cdRequest->io_Data=&changeInterrupt;
    SendIO(cdRequest);

    /* Return w/o result */
    return(NULL);

}

/****** cd/cdAddFrameInt ******************************************
*
*   NAME
*       cdAddChangeInt -- add frame interrupt
*
*   SYNOPSIS
*       result=cdAddFrameInt(rxMsg,args,error);
*
*       STRPTR cdAddFrameInt(struct RexxMsg *rxMsg,STRPTR args,LONG *error);
*
*   FUNCTION
*       Add frame interrupt.
*
*   INPUTS
*       rxMsg   -   ARexx message
*       args    -   arguments from ARexx
*       error   -   pointer to error value
*
*   RESULT
*       *error  -   Error code (if any)
*       result  -   result string
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*       Normal procedure for handling exec.library I/O requests
*       requires that an I/O request be completed before it is
*       re-used. However, for historical reasons, the *_ADDCHANGEINT
*       and *_ADDFRAMEINT commands of trackdisk.device, scsi.device,
*       and cd.device require the caller to send the I/O request
*       to add the interrupt and re-use the I/O request (without
*       calling exec.library/WaitIO()) with a *_REMCHANGEINT
*       or *_REMFRAMEINT command to remove the interrupt.
*
*       This causes IOTorture hits and is, in my humble opinion,
*       a Bad Thing(TM). However, it is apparently well beyond the
*       point where this behavior could be changed.
*
*   SEE ALSO
*       cdRemFrameInt
*
******************************************************************************
*
*/

STRPTR cdAddFrameInt(struct RexxMsg *rxMsg,STRPTR  args,LONG *error)
{

    if (debugMode) {
        printf("Initializing frameInterrupt\n");
    }

    /* Set-up frame interrupt */
    frameInterrupt.is_Node.ln_Pri=0;
    frameInterrupt.is_Node.ln_Name="CD Frame";
    frameInterrupt.is_Data=(APTR) COOKIE_FRAMEINT;
    frameInterrupt.is_Code=(void *) frameInterruptCode;
    frameIntCall=frameIntBadData=0L;

    if (debugMode) {
        printf("frameInterrupt initialized: is_Data=$%08lx, is_Code=$%08lx\n",
            frameInterrupt.is_Data,frameInterrupt.is_Code);
    }

    /* Add frame interrupt */
    cdRequest->io_Command=CD_ADDFRAMEINT;
    cdRequest->io_Length=sizeof(frameInterrupt);
    cdRequest->io_Data=&frameInterrupt;
    SendIO(cdRequest);
    if (debugMode) {
        printf("CD_ADDFRAMEINT good\n");
    }

    /* Return w/o result */
    return(NULL);

}

STRPTR cdAttenuate(struct RexxMsg *rxMsg,STRPTR args,LONG *error)
{

    static struct options {
        LONG *duration;
        LONG *target;
    } options;
    struct RDArgs *rdArgs;

    BYTE ioError;

    STRPTR result=NULL;

    /* Parse arguments */
    rdArgs=obtainArgs(args,"DURATION/N/A,TARGET/N/A",
        &options,sizeof(options));
    if (!rdArgs) {
        *error=RC_ERROR;
        return(NULL);
    }

    cdRequest->io_Command=CD_ATTENUATE;
    cdRequest->io_Length=*options.duration;
    cdRequest->io_Offset=*options.target;
    if (noCD) {
        ioError=-1;
    } else {
        ioError=DoIO(cdRequest);
    }
    if (ioError) {
        if (debugMode) {
            printf("CD_ATTENUATE I/O error %d\n",ioError);
        }
        *error=RC_ERROR;
        result=NULL;
    } else {
        /* Return current volume */
        bsprintf(rexxResult,"%ld",cdRequest->io_Actual);
        result=rexxResult;
    }

    /* Free argument parsing structure */
    releaseArgs(rdArgs);

    /* Return result */
    return(result);

}

STRPTR cdChangeNum(struct RexxMsg *rxMsg,STRPTR args,LONG *error)
{

    BYTE ioError;

    STRPTR result=NULL;

    cdRequest->io_Command=CD_CHANGENUM;
    if (noCD) {
        ioError=-1;
    } else {
        ioError=DoIO(cdRequest);
    }
    if (ioError) {
        if (debugMode) {
            printf("CD_CHANGENUM I/O error %d\n",ioError);
        }
        *error=RC_ERROR;
        result=NULL;
    } else {
        /* Return disk-change counter */
        bsprintf(rexxResult,"%ld",cdRequest->io_Actual);
        result=rexxResult;
    }

    /* Return result */
    return(result);

}

STRPTR cdChangeState(struct RexxMsg *rxMsg,STRPTR args,LONG *error)
{

                    BYTE ioError;

    STRPTR result=NULL;

    cdRequest->io_Command=CD_CHANGESTATE;
    if (noCD) {
        ioError=-1;
    } else {
        ioError=DoIO(cdRequest);
    }
    if (ioError) {
        if (debugMode) {
            printf("CD_CHANGESTATE I/O error %d\n",ioError);
        }
        *error=RC_ERROR;
        result=NULL;
    } else {
        /* Return disk-change state */
        bsprintf(rexxResult,"%ld",cdRequest->io_Actual);
        result=rexxResult;
    }

    /* Return result */
    return(result);

}

/*
 *  CD_CONFIG TAGCD_* definitions conflict with standard system tag definitions
 *  in utility/utility.h.
 *
 *  As a result, this function is implemented in a less than optimal manner.
 *
 */


STRPTR cdConfig(struct RexxMsg *rxMsg,STRPTR args,LONG *error)
{

    static struct options {
        LONG *playSpeed;
        LONG *readSpeed;
        LONG *readXLSpeed;
        LONG *sectorSize;
        LONG xlECC;
        LONG noXLECC;
        LONG ejectReset;
        LONG noEjectReset;
    } options;
    struct RDArgs *rdArgs;

    BYTE ioError;

    STRPTR result=NULL;

    static struct TagItem configTags[CDCONFIG_MAXTAGS];
    int thisTag=0;

    /* Parse arguments */
    rdArgs=obtainArgs(args,
       "PLAYSPEED/N,READSPEED/N,READXLSPEED/N,SECTORSIZE/N,XLECC/S,NOXLECC/S,EJECTRESET/S,NOEJECTRESET/S",
       &options,sizeof(options));
    if (!rdArgs) {
       *error=RC_ERROR;
       return(NULL);
    }

    /* Play speed */
    if (options.playSpeed) {
        configTags[thisTag].ti_Tag=TAGCD_PLAYSPEED;
        configTags[thisTag].ti_Data=*options.playSpeed;
        thisTag++;
    }
    if (debugMode) {
        printf(options.playSpeed?"PlaySpeed=%d\n":"PlaySpeed=Not specified\n",
            options.playSpeed?*options.playSpeed:NULL);
    }

    /* Read speed */
    if (options.readSpeed) {
        configTags[thisTag].ti_Tag=TAGCD_READSPEED;
        configTags[thisTag].ti_Data=*options.readSpeed;
        thisTag++;
    }
    if (debugMode) {
        printf(options.readSpeed?"ReadSpeed=%d\n":"ReadSpeed=Not specified\n",
            options.readSpeed?*options.readSpeed:NULL);
    }

    /* XL Read speed */
    if (options.readXLSpeed) {
        configTags[thisTag].ti_Tag=TAGCD_READSPEED;
        configTags[thisTag].ti_Data=*options.readXLSpeed;
        thisTag++;
    }
    if (debugMode) {
        printf(options.readXLSpeed?"ReadXLSpeed=%d\n":"ReadXLSpeed=Not specified\n",
            options.readXLSpeed?*options.readXLSpeed:NULL);
    }

    /* Sector size */
    if (options.sectorSize) {
        configTags[thisTag].ti_Tag=TAGCD_SECTORSIZE;
        configTags[thisTag].ti_Data=*options.sectorSize;
        thisTag++;
    }
    if (debugMode) {
        printf(options.sectorSize?"SectorSize=%d\n":"SectorSize=Not specified\n",
            options.sectorSize?*options.sectorSize:NULL);
    }

    /* XL ECC */
    if (options.xlECC|options.noXLECC) {
        configTags[thisTag].ti_Tag=TAGCD_XLECC;
        configTags[thisTag].ti_Data=options.xlECC?TRUE:FALSE;
        thisTag++;
    }
    if (debugMode) {
        printf(options.xlECC?"XLECC=%d\n":"XLECC=Not specified\n",
            options.xlECC?"TRUE":"FALSE");
    }

    /* Reset on eject */
    if (options.ejectReset|options.noEjectReset) {
        configTags[thisTag].ti_Tag=TAGCD_EJECTRESET;
        configTags[thisTag].ti_Data=
            (options.ejectReset&options.noEjectReset)?0xFFFFFFFF:(options.ejectReset?TRUE:FALSE);
        thisTag++;
    }
    if (debugMode) {
        printf(options.ejectReset?"EjectReset=%d\n":"EjectReset=Not specified\n",
            options.ejectReset?"TRUE":"FALSE");
    }

    /* Terminate tag array */
    configTags[thisTag].ti_Tag=TAG_DONE;

    cdRequest->io_Command=CD_CONFIG;
    cdRequest->io_Data=configTags;
    cdRequest->io_Length=0;
    if (noCD) { ioError=-1; } else { ioError=DoIO(cdRequest); }
    if (ioError) {
        if (debugMode) {
            printf("CD_CONFIG I/O error %d\n",ioError);
        }
        *error=RC_ERROR;
    }

    /* Free argument parsing structure */
    releaseArgs(rdArgs);

    /* Return result */
    return(result);

}

STRPTR cdEject(struct RexxMsg *rxMsg,STRPTR args,LONG *error)
{

    static struct options {
        LONG open;
        LONG close;
    } options;
    struct RDArgs *rdArgs;

    BYTE ioError;

    STRPTR result=NULL;

    /* Parse arguments */
    rdArgs=obtainArgs(args,"OPEN/S,CLOSE/S",
        &options,sizeof(options));
    if (!rdArgs) {
        *error=RC_ERROR;
        return(NULL);
    }

    cdRequest->io_Command=CD_EJECT;
    cdRequest->io_Length=options.open?1:(options.close?0:-1);
    if (noCD) { ioError=-1; } else { ioError=DoIO(cdRequest); }
    if (ioError) {
        if (debugMode) {
            printf("CD_EJECT I/O error %d\n",ioError);
        }
        *error=RC_ERROR;
        result=NULL;
    } else {
        /* Return current door state */
        bsprintf(rexxResult,"%ld",cdRequest->io_Actual);
        result=rexxResult;
    }

    /* Free argument parsing structure */
    releaseArgs(rdArgs);

    /* Return result */
    return(result);

}

STRPTR cdGetGeometry(struct RexxMsg *rxMsg,STRPTR args,LONG *error)
{

    static struct options {
        STRPTR stem;
    } options;
    struct RDArgs *rdArgs;

    BYTE ioError;

    struct DriveGeometry driveGeometry;

    /* Parse arguments */
    rdArgs=obtainArgs(args,"STEM/A",&options,sizeof(options));
    if (!rdArgs) {
        *error=RC_ERROR;
        return(NULL);
    }

    /* Fetch drive geometry */
    cdRequest->io_Command=CD_GETGEOMETRY;
    cdRequest->io_Length=sizeof(driveGeometry);
    cdRequest->io_Data=&driveGeometry;
    if (noCD) { ioError=-1; } else { ioError=DoIO(cdRequest); }
    if (ioError) {
        if (debugMode) {
            printf("CD_GETGEOMETRY I/O error %d\n",ioError);
        }
        *error=RC_ERROR;
    } else {

        /* Return drive geometry in ARexx stem variable */
        setStemVarInt(rxMsg,options.stem,"SectorSize",
            driveGeometry.dg_SectorSize);
        setStemVarInt(rxMsg,options.stem,"TotalSectors",
            driveGeometry.dg_TotalSectors);
        setStemVarInt(rxMsg,options.stem,"Cylinders",
            driveGeometry.dg_Cylinders);
        setStemVarInt(rxMsg,options.stem,"CylSectors",
            driveGeometry.dg_CylSectors);
        setStemVarInt(rxMsg,options.stem,"Heads",
            driveGeometry.dg_Heads);
        setStemVarInt(rxMsg,options.stem,"TrackSectors",
            driveGeometry.dg_TrackSectors);
        setStemVarInt(rxMsg,options.stem,"BufMemType",
            driveGeometry.dg_BufMemType);
        setStemVarInt(rxMsg,options.stem,"DeviceType",
            driveGeometry.dg_DeviceType);
        setStemVarInt(rxMsg,options.stem,"Removable",
            driveGeometry.dg_Flags&DGF_REMOVABLE);

    }

    /* Free argument parsing structure */
    releaseArgs(rdArgs);

    /* Return */
    return(NULL);

}

STRPTR cdInfo(struct RexxMsg *rxMsg,STRPTR args,LONG *error)
{

    static struct options {
        STRPTR stem;
    } options;
    struct RDArgs *rdArgs;

    BYTE ioError;

    STRPTR result=NULL;

    struct CDInfo cdInfo;

    /* Parse arguments */
    rdArgs=obtainArgs(args,"STEM/A",&options,sizeof(options));
    if (!rdArgs) {
        *error=RC_ERROR;
        return(NULL);
    }

    /* Fetch device information */
    cdRequest->io_Command=CD_INFO;
    cdRequest->io_Length=sizeof(cdInfo);
    cdRequest->io_Data=&cdInfo;
    if (noCD) { ioError=-1; } else { ioError=DoIO(cdRequest); }
    if (ioError) {
        if (debugMode) {
            printf("CD_INFO I/O error %d\n",ioError);
        }
        *error=RC_ERROR;
    } else {

        /* Return device information in ARexx stem variable */
        setStemVarInt(rxMsg,options.stem,"PlaySpeed",
            cdInfo.PlaySpeed);
        if (debugMode) {
            printf("PlaySpeed=%d\n",cdInfo.PlaySpeed);
        }
        setStemVarInt(rxMsg,options.stem,"ReadSpeed",
            cdInfo.ReadSpeed);
        if (debugMode) {
            printf("ReadSpeed=%d\n",cdInfo.ReadSpeed);
        }
        setStemVarInt(rxMsg,options.stem,"ReadXLSpeed",
            cdInfo.ReadXLSpeed);
        if (debugMode) {
            printf("ReadXLSpeed=%d\n",cdInfo.ReadXLSpeed);
        }
        setStemVarInt(rxMsg,options.stem,"SectorSize",
            cdInfo.SectorSize);
        if (debugMode) {
            printf("SectorSize=%d\n",cdInfo.SectorSize);
        }
        setStemVarInt(rxMsg,options.stem,"XLECC",
            cdInfo.XLECC);
        if (debugMode) {
            printf("XLECC=%d\n",cdInfo.XLECC);
        }
        setStemVarInt(rxMsg,options.stem,"EjectReset",
            cdInfo.EjectReset);
        if (debugMode) {
            printf("EjectReset=%d\n",cdInfo.EjectReset);
        }
        setStemVarInt(rxMsg,options.stem,"MaxSpeed",
            cdInfo.MaxSpeed);
        if (debugMode) {
            printf("MaxSpeed=%d\n",cdInfo.MaxSpeed);
        }
        setStemVarInt(rxMsg,options.stem,"AudioPrecision",
            cdInfo.AudioPrecision);
        if (debugMode) {
            printf("AudioPrecision=%d\n",cdInfo.AudioPrecision);
        }
        if (debugMode) {
            printf("Status=$%04x\n",cdInfo.Status);
        }
        setStemVarInt(rxMsg,options.stem,"Closed",
            cdInfo.Status&CDSTSF_CLOSED);
        if (debugMode) {
            printf("Closed=%d\n",cdInfo.Status&CDSTSF_CLOSED);
        }
        setStemVarInt(rxMsg,options.stem,"Disk",
            cdInfo.Status&CDSTSF_DISK);
        if (debugMode) {
            printf("Disk=%d\n",cdInfo.Status&CDSTSF_DISK);
        }
        setStemVarInt(rxMsg,options.stem,"Spin",
            cdInfo.Status&CDSTSF_SPIN);
        if (debugMode) {
            printf("Spin=%d\n",cdInfo.Status&CDSTSF_SPIN);
        }
        setStemVarInt(rxMsg,options.stem,"TOC",
            cdInfo.Status&CDSTSF_TOC);
        if (debugMode) {
            printf("TOC=%d\n",cdInfo.Status&CDSTSF_TOC);
        }
        setStemVarInt(rxMsg,options.stem,"CDROM",
            cdInfo.Status&CDSTSF_CDROM);
        if (debugMode) {
            printf("CDROM=%d\n",cdInfo.Status&CDSTSF_CDROM);
        }
        setStemVarInt(rxMsg,options.stem,"Playing",
            cdInfo.Status&CDSTSF_PLAYING);
        if (debugMode) {
            printf("Playing=%d\n",cdInfo.Status&CDSTSF_PLAYING);
        }
        setStemVarInt(rxMsg,options.stem,"Paused",
            cdInfo.Status&CDSTSF_PAUSED);
        if (debugMode) {
            printf("Paused=%d\n",cdInfo.Status&CDSTSF_PAUSED);
        }
        setStemVarInt(rxMsg,options.stem,"Search",
            cdInfo.Status&CDSTSF_SEARCH);
        if (debugMode) {
            printf("Search=%d\n",cdInfo.Status&CDSTSF_SEARCH);
        }
        setStemVarInt(rxMsg,options.stem,"Direction",
            cdInfo.Status&CDSTSF_DIRECTION);
        if (debugMode) {
            printf("Direction=%d\n",cdInfo.Status&CDSTSF_DIRECTION);
        }

    }

    /* Free argument parsing structure */
    releaseArgs(rdArgs);

    /* Return */
    return(result);

}

STRPTR cdMotor(struct RexxMsg *rxMsg,STRPTR args,LONG *error)
{

    static struct options {
        LONG on;
        LONG off;
    } options;
    struct RDArgs *rdArgs;

    BYTE ioError;

    STRPTR result=NULL;

    /* Parse arguments */
    rdArgs=obtainArgs(args,"ON/S,OFF/S",
        &options,sizeof(options));
    if (!rdArgs) {
        *error=RC_ERROR;
        return(NULL);
    }

    cdRequest->io_Command=CD_MOTOR;
    cdRequest->io_Length=options.on?1:(options.off?0:-1);
    if (noCD) { ioError=-1; } else { ioError=DoIO(cdRequest); }
    if (ioError) {
        if (debugMode) {
            printf("CD_EJECT I/O error %d\n",ioError);
        }
        *error=RC_ERROR;
        result=NULL;
    } else {
        /* Return current motor state */
        bsprintf(rexxResult,"%ld",cdRequest->io_Actual);
        result=rexxResult;
    }

    /* Free argument parsing structure */
    releaseArgs(rdArgs);

    /* Return result */
    return(result);

}

STRPTR cdPause(struct RexxMsg *rxMsg,STRPTR args,LONG *error)
{

    static struct options {
        LONG on;
        LONG off;
    } options;
    struct RDArgs *rdArgs;

    BYTE ioError;

    STRPTR result=NULL;

    /* Parse arguments */
    rdArgs=obtainArgs(args,"ON/S,OFF/S",
        &options,sizeof(options));
    if (!rdArgs) {
        *error=RC_ERROR;
        return(NULL);
    }

    cdRequest->io_Command=CD_PAUSE;
    cdRequest->io_Length=options.on?1:(options.off?0:-1);
    if (noCD) { ioError=-1; } else { ioError=DoIO(cdRequest); }
    if (ioError) {
        if (debugMode) {
            printf("CD_PAUSE I/O error %d\n",ioError);
        }
        *error=RC_ERROR;
        result=NULL;
    } else {

        /* Return pause state */
        bsprintf(rexxResult,"%ld",cdRequest->io_Actual);
        result=rexxResult;

    }

    /* Free argument parsing structure */
    releaseArgs(rdArgs);

    /* Return result */
    return(result);

}

STRPTR cdPlayLSN(struct RexxMsg *rxMsg,STRPTR args,LONG *error)
{

    static struct options {
        LONG *start;
        LONG *length;
    } options;
    struct RDArgs *rdArgs;

    BYTE ioError;

    STRPTR result=NULL;

    /* Parse arguments */
    rdArgs=obtainArgs(args,"START/N/A,LENGTH/N/A",&options,sizeof(options));
    if (!rdArgs) {
        *error=RC_ERROR;
        return(NULL);
    }

    /* Play audio */
    cdRequest->io_Command=CD_PLAYLSN;
    cdRequest->io_Length=*options.length;
    cdRequest->io_Offset=*options.start;
    if (noCD) { ioError=-1; } else { ioError=DoIO(cdRequest); }
    if (ioError) {
        if (debugMode) {
            printf("CD_PLAYLSN I/O error %d\n",ioError);
        }
        *error=RC_ERROR;
        result=NULL;
    } else {
        result=NULL;
    }

    /* Free argument parsing structure */
    releaseArgs(rdArgs);

    /* Return result */
    return(result);

}

STRPTR cdPlayMSF(struct RexxMsg *rxMsg,STRPTR args,LONG *error)
{

    static struct options {
        STRPTR start;
        STRPTR length;
    } options;
    struct RDArgs *rdArgs;
    ULONG startMSF, lengthMSF;

    BYTE ioError;

    STRPTR result=NULL;

    /* Parse arguments */
    rdArgs=obtainArgs(args,"START/A,LENGTH/A",&options,sizeof(options));
    if (!rdArgs) {
        *error=RC_ERROR;
        return(NULL);
    }
    if (debugMode) {
        printf("cdPlayMSF -- start=%s, length=%s\n",options.start,
            options.length);
    }
    startMSF=strToMSF(options.start);
    lengthMSF=strToMSF(options.length);
    if (debugMode) {
        printf("cdPlayMSF -- start=$%08lx, length=$%08lx\n",startMSF,lengthMSF);
    }

    /* Play audio */
    cdRequest->io_Command=CD_PLAYMSF;
    cdRequest->io_Length=lengthMSF;
    cdRequest->io_Offset=startMSF;
    if (noCD) { ioError=-1; } else { ioError=DoIO(cdRequest); }
    if (ioError) {
        if (debugMode) {
            printf("CD_PLAYMSF I/O error %d\n",ioError);
        }
        *error=RC_ERROR;
        result=NULL;
    } else {
        result=NULL;
    }

    /* Free argument parsing structure */
    releaseArgs(rdArgs);

    /* Return result */
    return(result);

}

STRPTR cdPlayTrack(struct RexxMsg *rxMsg,STRPTR args,LONG *error)
{

    static struct options {
        LONG *start;
        LONG *length;
    } options;
    struct RDArgs *rdArgs;

    BYTE ioError;

    STRPTR result=NULL;

    /* Parse arguments */
    rdArgs=obtainArgs(args,"START/N/A,LENGTH/N/A",&options,sizeof(options));
    if (!rdArgs) {
        *error=RC_ERROR;
        return(NULL);
    }

    /* Play track */
    cdRequest->io_Command=CD_PLAYTRACK;
    cdRequest->io_Length=*options.length;
    cdRequest->io_Offset=*options.start;
    if (noCD) { ioError=-1; } else { ioError=DoIO(cdRequest); }
    if (ioError) {
        if (debugMode) {
            printf("CD_PLAYTRACK I/O error %d\n",ioError);
        }
        *error=RC_ERROR;
        result=NULL;
    } else {
        result=NULL;
    }

    /* Free argument parsing structure */
    releaseArgs(rdArgs);

    /* Return result */
    return(result);


}

STRPTR cdProtStatus(struct RexxMsg *rxMsg,STRPTR args,LONG *error)
{

    BYTE ioError;

    STRPTR result=NULL;

    cdRequest->io_Command=CD_PROTSTATUS;
    if (noCD) { ioError=-1; } else { ioError=DoIO(cdRequest); }
    if (ioError) {
        if (debugMode) {
            printf("CD_PROTSTATUS I/O error %d\n",ioError);
        }
        *error=RC_ERROR;
        result=NULL;
    } else {
        /* Return protection status */
        bsprintf(rexxResult,"%ld",cdRequest->io_Actual);
        result=rexxResult;
    }

    /* Return result */
    return(result);

}

STRPTR cdQCodeLSN(struct RexxMsg *rxMsg,STRPTR args,LONG *error)
{

    static struct options {
        STRPTR stem;
    } options;
    struct RDArgs *rdArgs;

    BYTE ioError;

    struct QCode qCode;

    /* Parse arguments */
    rdArgs=obtainArgs(args,"STEM/A",&options,sizeof(options));
    if (!rdArgs) {
        *error=RC_ERROR;
        return(NULL);
    }

    /* Fetch Q-Code */
    cdRequest->io_Command=CD_QCODELSN;
    cdRequest->io_Length=0;
    cdRequest->io_Data=&qCode;
    if (noCD) { ioError=-1; } else { ioError=DoIO(cdRequest); }
    if (ioError) {
        if (debugMode) {
            printf("CD_QCODELSN I/O error %d\n",ioError);
        }
        *error=RC_ERROR;
    } else {

        /* Return QCode data in ARexx stem variable */
        setStemVarInt(rxMsg,options.stem,"CtlAdr",qCode.CtlAdr);
        setStemVarInt(rxMsg,options.stem,"Track",qCode.Track);
        setStemVarInt(rxMsg,options.stem,"Index",qCode.Index);
        setStemVarInt(rxMsg,options.stem,"Zero",qCode.Zero);
        setStemVarInt(rxMsg,options.stem,"TrackPosition",
            qCode.TrackPosition.LSN);
        setStemVarInt(rxMsg,options.stem,"DiskPosition",
            qCode.DiskPosition.LSN);

    }

    /* Free argument parsing structure */
    releaseArgs(rdArgs);

    /* Return */
    return(NULL);

}

STRPTR cdQCodeMSF(struct RexxMsg *rxMsg,STRPTR args,LONG *error)
{

    static struct options {
        STRPTR stem;
    } options;
    struct RDArgs *rdArgs;

    BYTE ioError;

    STRPTR result=NULL;

    struct QCode qCode;

    /* Parse arguments */
    rdArgs=obtainArgs(args,"STEM/A",&options,sizeof(options));
    if (!rdArgs) {
        *error=RC_ERROR;
        return(NULL);
    }

    /* Fetch Q-Code */
    cdRequest->io_Command=CD_QCODEMSF;
    cdRequest->io_Length=0;
    cdRequest->io_Data=&qCode;
    if (noCD) { ioError=-1; } else { ioError=DoIO(cdRequest); }
    if (ioError) {
        if (debugMode) {
            printf("CD_QCODEMSF I/O error %d\n",ioError);
        }
        *error=RC_ERROR;
        result=NULL;
    } else {

        /* Return QCode data in ARexx stem variable */
        setStemVarInt(rxMsg,options.stem,"CtlAdr",qCode.CtlAdr);
        setStemVarInt(rxMsg,options.stem,"Track",qCode.Track);
        setStemVarInt(rxMsg,options.stem,"Index",qCode.Index);
        setStemVarInt(rxMsg,options.stem,"Zero",qCode.Zero);
        setStemVarInt(rxMsg,options.stem,"TrackPosition.Minute",
            qCode.TrackPosition.MSF.Minute);
        setStemVarInt(rxMsg,options.stem,"TrackPosition.Second",
            qCode.TrackPosition.MSF.Second);
        setStemVarInt(rxMsg,options.stem,"TrackPosition.Frame",
            qCode.TrackPosition.MSF.Frame);
        setStemVarInt(rxMsg,options.stem,"DiskPosition.Minute",
            qCode.DiskPosition.MSF.Minute);
        setStemVarInt(rxMsg,options.stem,"DiskPosition.Second",
            qCode.DiskPosition.MSF.Second);
        setStemVarInt(rxMsg,options.stem,"DiskPosition.Frame",
            qCode.DiskPosition.MSF.Frame);

    }

    /* Free argument parsing structure */
    releaseArgs(rdArgs);

    /* Return */
    return(result);

}

STRPTR cdRead(struct RexxMsg *rxMsg,STRPTR args,LONG *error)
{

    static struct options {
        LONG *offset;
        LONG *length;
        LONG byteVerify;
        LONG wordVerify;
        LONG offsetVerify;
        LONG *start;
    } options;
    struct RDArgs *rdArgs;

    BYTE ioError;

    UBYTE *buffer;

    /* Parse arguments */
    if (debugMode) {
        printf("cdRead: Calling obtainArgs()\n");
    }
    rdArgs=obtainArgs(args,
        "OFFSET/N/A,LENGTH/N/A,BV=BYTEVERIFY/S,WV=WORDVERIFY/S,OV=OFFSETVERIFY/S,START/N",
        &options,sizeof(options));
    if (!rdArgs) {
        if (debugMode) {
            printf("cdRead: obtainArgs() failed\n");
        }
        *error=RC_ERROR;
        return(NULL);
    }
    if (debugMode) {
        printf("cdRead: obtainArgs() good\n");
    }

    /* Allocate buffer */
    buffer=AllocVec(*options.length,NULL);
    if (!buffer) {
        if (debugMode) {
            printf("cdRead: Buffer allocation failed\n");
        }
        *error=RC_ERROR;
        return(NULL);
    }
    if (debugMode) {
        printf("cdRead: Allocated %lu-byte buffer at $%08lx\n",
            *options.length,buffer);
    }

    /* Read data */
    cdRequest->io_Command=CD_READ;
    cdRequest->io_Data=buffer;
    cdRequest->io_Length=*options.length;
    cdRequest->io_Offset=*options.offset;
    if (debugMode) {
        printf("cdRead: Calling CD_READ\n");
    }
    if (noCD) { ioError=-1; } else { ioError=DoIO(cdRequest); }
    if (debugMode) {
        printf("cdRead: CD_READ returned %d\n",ioError);
    }
    if (ioError) {
        if (debugMode) {
            printf("CD_READ I/O error %d\n",ioError);
        }
        *error=RC_ERROR;
    } else {
        /* Verify byte pattern */
        if (options.byteVerify) {
            *error=
                byteVerify(buffer,*options.length,
                    options.start?*options.start:0)?
                RC_OK:RC_WARN;
        }
        /* Verify word pattern */
        if (options.wordVerify) {
            *error=
                wordVerify(buffer,*options.length,
                    options.start?*options.start:0)?
                RC_OK:RC_WARN;
        }
        /* Verify offset pattern */
        if (options.offsetVerify) {
            if (debugMode) {
                printf("cdRead: Verifying offset pattern\n");
            }
            *error=
                offsetVerify(buffer,*options.length,
                    options.start?*options.start:0)?
                RC_OK:RC_WARN;
            if (debugMode) {
                printf("cdRead: Offset verify status %d\n",*error);
            }
        }

    }

    /* Free buffer */
    FreeVec(buffer);

    /* Free argument parsing structure */
    releaseArgs(rdArgs);

    /* Return -- no result */
    return(NULL);

}

STRPTR cdReadXL(struct RexxMsg *rxMsg,STRPTR args,LONG *error)
{

    Printf("%s: Not yet implemented\n",PROGRAM_NAME);

    *error=RC_WARN;
    return(NULL);

}

/****** cd/cdRemChangeInt ******************************************
*
*   NAME
*       cdRemChangeInt -- remove frame interrupt
*
*   SYNOPSIS
*       result=cdRemChangeInt(rxMsg,args,error);
*
*       STRPTR cdRemChangeInt(struct RexxMsg *rxMsg,STRPTR args,LONG *error);
*
*   FUNCTION
*       Remove disk change interrupt.
*
*   INPUTS
*       rxMsg   -   ARexx message
*       args    -   arguments from ARexx
*       error   -   pointer to error value
*
*   RESULT
*       *error  -   Error code (if any)
*       result  -   result string
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*       Normal procedure for handling exec.library I/O requests
*       requires that an I/O request be completed before it is
*       re-used. However, for historical reasons, the *_ADDCHANGEINT
*       and *_ADDFRAMEINT commands of trackdisk.device, scsi.device,
*       and cd.device require the caller to send the I/O request
*       to add the interrupt and re-use the I/O request (without
*       calling exec.library/WaitIO()) with a *_REMCHANGEINT
*       or *_REMFRAMEINT command to remove the interrupt.
*
*       This causes IOTorture hits and is, in my humble opinion,
*       a Bad Thing(TM). However, it is apparently well beyond the
*       point where this behavior could be changed.
*
*   SEE ALSO
*       cdAddChangeInt
*
******************************************************************************
*
*/

STRPTR cdRemChangeInt(struct RexxMsg *rxMsg,STRPTR args,LONG *error)
{

    BYTE ioError;

    /* Remove disk change interrupt */
    cdRequest->io_Command=CD_REMCHANGEINT;
    cdRequest->io_Length=sizeof(changeInterrupt);
    cdRequest->io_Data=&changeInterrupt;
    if (noCD) { ioError=-1; } else { ioError=DoIO(cdRequest); }
    if (ioError) {
        if (debugMode) {
            printf("CD_REMCHANGEINT I/O error %d\n",ioError);
        }
        *error=RC_ERROR;
    }

    /* Return w/o result */
    return(NULL);

}

/****** cd/cdRemFrameInt ******************************************
*
*   NAME
*       cdRemFrameInt -- remove frame interrupt
*
*   SYNOPSIS
*       result=cdRemFrameInt(rxMsg,args,error);
*
*       STRPTR cdRemFrameInt(struct RexxMsg *rxMsg,STRPTR args,LONG *error);
*
*   FUNCTION
*       Remove frame interrupt.
*
*   INPUTS
*       rxMsg   -   ARexx message
*       args    -   arguments from ARexx
*       error   -   pointer to error value
*
*   RESULT
*       *error  -   Error code (if any)
*       result  -   result string
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*       Normal procedure for handling exec.library I/O requests
*       requires that an I/O request be completed before it is
*       re-used. However, for historical reasons, the *_ADDCHANGEINT
*       and *_ADDFRAMEINT commands of trackdisk.device, scsi.device,
*       and cd.device require the caller to send the I/O request
*       to add the interrupt and re-use the I/O request (without
*       calling exec.library/WaitIO()) with a *_REMCHANGEINT
*       or *_REMFRAMEINT command to remove the interrupt.
*
*       This causes IOTorture hits and is, in my humble opinion,
*       a Bad Thing(TM). However, it is apparently well beyond the
*       point where this behavior could be changed.
*
*   SEE ALSO
*       cdAddFrameInt
*
******************************************************************************
*
*/

STRPTR cdRemFrameInt(struct RexxMsg *rxMsg,STRPTR args,LONG *error)
{

    BYTE ioError;

    /* Remove frame interrupt */
    cdRequest->io_Command=CD_REMFRAMEINT;
    cdRequest->io_Length=sizeof(frameInterrupt);
    cdRequest->io_Data=&frameInterrupt;
    if (noCD) { ioError=-1; } else { ioError=DoIO(cdRequest); }
    if (ioError) {
        if (debugMode) {
            printf("CD_REMFRAMEINT I/O error %d\n",ioError);
        }
        *error=RC_ERROR;
    }

    /* Return w/o result */
    return(NULL);

}

STRPTR cdSearch(struct RexxMsg *rxMsg,STRPTR args,LONG *error)
{

    static struct options {
        LONG normal;
        LONG ffwd;
        LONG frev;
    } options;
    struct RDArgs *rdArgs;

    BYTE ioError;

    /* Parse arguments */
    rdArgs=obtainArgs(args,"NORMAL/S,FFWD/S,FREV/S",
        &options,sizeof(options));
    if (!rdArgs) {
        *error=RC_ERROR;
        return(NULL);
    }

    /* Set search mode */
    cdRequest->io_Command=CD_SEARCH;
    if (options.normal) {
        cdRequest->io_Length=CDMODE_NORMAL;
    }
    if (options.ffwd) {
        cdRequest->io_Length=CDMODE_FFWD;
    }
    if (options.frev) {
        cdRequest->io_Length=CDMODE_FREV;
    }
    if (!(options.normal) & !(options.ffwd) & !(options.frev)) {
        cdRequest->io_Length=-1L;
    }
    if (noCD) { ioError=-1; } else { ioError=DoIO(cdRequest); }
    if (ioError) {
        if (debugMode) {
            printf("CD_SEARCH I/O error %d\n",ioError);
        }
        *error=RC_ERROR;
    }

    /* Free argument parsing structure */
    releaseArgs(rdArgs);

    /* Return w/o result */
    return(NULL);

}

STRPTR cdSeek(struct RexxMsg *rxMsg,STRPTR args,LONG *error)
{

    static struct options {
        LONG *offset;
    } options;
    struct RDArgs *rdArgs;

    BYTE ioError;

    /* Parse arguments */
    rdArgs=obtainArgs(args,"OFFSET/N/A",
        &options,sizeof(options));
    if (!rdArgs) {
        *error=RC_ERROR;
        return(NULL);
    }

    /* Seek */
    cdRequest->io_Command=CD_SEEK;
    cdRequest->io_Offset=*options.offset;
    if (noCD) { ioError=-1; } else { ioError=DoIO(cdRequest); }
    if (ioError) {
        if (debugMode) {
            printf("CD_SEEK I/O error %d\n",ioError);
        }
        *error=RC_ERROR;
    }

    /* Free argument parsing structure */
    releaseArgs(rdArgs);

    /* Return w/o result */
    return(NULL);

}

STRPTR cdTOCLSN(struct RexxMsg *rxMsg,STRPTR args,LONG *error)
{

    static struct options {
        STRPTR stem;
        LONG *entries;
        LONG *begin;
    } options;
    struct RDArgs *rdArgs;

    BYTE ioError;

    struct TOCEntry *buffer;
    struct TOCSummary *summary;
    int index;

    /* Parse arguments */
    rdArgs=obtainArgs(args,"STEM/A,ENTRIES/N/A,BEGIN/N/A",
        &options,sizeof(options));
    if (!rdArgs) {
        *error=RC_ERROR;
        return(NULL);
    }

    if (debugMode) {
        printf("cdTOCLSN -- Stem=%s, Entries=%ld, Begin=%ld\n",
            options.stem,*options.entries,*options.begin);
    }

    /* Allocate TOC buffer */
    buffer=AllocVec(sizeof(struct TOCEntry)*(*options.entries),MEMF_CLEAR);
    if (!buffer) {
        if (debugMode) {
            printf("Error allocating TOC buffer\n");
        }
        *error=RC_ERROR;
        return(NULL);
    }

    /* Fetch TOC in LSN format */
    cdRequest->io_Command=CD_TOCLSN;
    cdRequest->io_Data=buffer;
    cdRequest->io_Length=*options.entries;
    cdRequest->io_Offset=*options.begin;
    if (noCD) { ioError=-1; } else { ioError=DoIO(cdRequest); }
    if (ioError) {
        if (debugMode) {
            printf("CD_TOCLSN I/O error %d",ioError);
        }
        *error=RC_ERROR;
    }
    if (debugMode) {
        printf("CD_TOCLSN good\n");
    }

    /* Return summary, if requested */
    if (*options.begin==0) {

        summary=(struct TOCSummary *) &(buffer[0]);
        setStemVarInt(rxMsg,options.stem,"Summary.FirstTrack",summary->FirstTrack);
        if (debugMode) {
            printf("    FirstTrack=%ld\n",summary->FirstTrack);
        }
        setStemVarInt(rxMsg,options.stem,"Summary.LastTrack",summary->LastTrack);
        if (debugMode) {
            printf("    LastTrack=%ld\n",summary->LastTrack);
        }
        setStemVarInt(rxMsg,options.stem,"Summary.LeadOut",summary->LeadOut.LSN);
        if (debugMode) {
            printf("    LeadOut=%ld\n",summary->LeadOut.LSN);
        }

    }

    /* Return TOC entries */
    for (index=(*options.begin==0)?1:0;
         index<(*options.entries);
         index++) {
        if (debugMode) {
            printf("Entry %d (detail for track %d)\n",index,buffer[index].Track);
        }
        setStemVarIntArray(rxMsg,options.stem,buffer[index].Track,
            "CtlAdr",buffer[index].CtlAdr);
        setStemVarIntArray(rxMsg,options.stem,buffer[index].Track,
            "Track",buffer[index].Track);
        setStemVarIntArray(rxMsg,options.stem,buffer[index].Track,
            "Position",buffer[index].Position.LSN);
    }

    /* Free argument parsing structure */
    releaseArgs(rdArgs);

    /* Return w/o result */
    return(NULL);

}

STRPTR cdTOCMSF(struct RexxMsg *rxMsg,STRPTR args,LONG *error)
{

    static struct options {
        STRPTR stem;
        LONG *entries;
        LONG *begin;
    } options;
    struct RDArgs *rdArgs;

    BYTE ioError;

    struct TOCEntry *buffer;
    struct TOCSummary *summary;
    int index;

    /* Parse arguments */
    rdArgs=obtainArgs(args,"STEM/A,ENTRIES/N/A,BEGIN/N/A",
        &options,sizeof(options));
    if (!rdArgs) {
        *error=RC_ERROR;
        return(NULL);
    }

    if (debugMode) {
        printf("cdTOCMSF -- Stem=%s, Entries=%ld, Begin=%ld\n",
            options.stem,*options.entries,*options.begin);
    }

    /* Allocate TOC buffer */
    buffer=AllocVec(sizeof(struct TOCEntry)*(*options.entries),MEMF_CLEAR);
    if (!buffer) {
        if (debugMode) {
            printf("Error allocating TOC buffer\n");
        }
        *error=RC_ERROR;
        return(NULL);
    }

    /* Fetch TOC in MSF format */
    cdRequest->io_Command=CD_TOCMSF;
    cdRequest->io_Data=buffer;
    cdRequest->io_Length=*options.entries;
    cdRequest->io_Offset=*options.begin;
    if (noCD) { ioError=-1; } else { ioError=DoIO(cdRequest); }
    if (ioError) {
        if (debugMode) {
            printf("CD_TOCMSF I/O error %d\n",ioError);
        }
        *error=RC_ERROR;
        *error=RC_ERROR;
    }

    /* Return summary, if requested */
    if (*options.begin==0) {

        summary=(struct TOCSummary *) &(buffer[0]);
        setStemVarInt(rxMsg,options.stem,"Summary.FirstTrack",summary->FirstTrack);
        if (debugMode) {
            printf("    FirstTrack=%ld\n",summary->FirstTrack);
        }
        setStemVarInt(rxMsg,options.stem,"Summary.LastTrack",summary->LastTrack);
        if (debugMode) {
            printf("    LastTrack=%ld\n",summary->LastTrack);
        }
        if (debugMode) {
            printf("    LeadOut=%02d:%02d:%02d\n",
                summary->LeadOut.MSF.Minute,summary->LeadOut.MSF.Second,
                summary->LeadOut.MSF.Frame);
        }
        setStemVarInt(rxMsg,options.stem,"Summary.LeadOut.Minute",
            summary->LeadOut.MSF.Minute);
        setStemVarInt(rxMsg,options.stem,"Summary.LeadOut.Second",
            summary->LeadOut.MSF.Second);
        setStemVarInt(rxMsg,options.stem,"Summary.LeadOut.Frame",
            summary->LeadOut.MSF.Frame);

    }

    /* Return TOC entries */
    for (index=(*options.begin==0)?1:0;
         index<*options.entries;
         index++) {

        if (debugMode) {
            printf("Entry %d (detail for track %d)\n",index,buffer[index].Track);
        }
        setStemVarIntArray(rxMsg,options.stem,buffer[index].Track,
            "CtlAdr",buffer[index].CtlAdr);
        setStemVarIntArray(rxMsg,options.stem,buffer[index].Track,
            "Track",buffer[index].Track);
        setStemVarIntArray(rxMsg,options.stem,buffer[index].Track,
            "Position.Minute",buffer[index].Position.MSF.Minute);
        setStemVarIntArray(rxMsg,options.stem,buffer[index].Track,
            "Position.Second",buffer[index].Position.MSF.Second);
        setStemVarIntArray(rxMsg,options.stem,buffer[index].Track,
            "Position.Frame",buffer[index].Position.MSF.Frame);

    }

    /* Free argument parsing structure */
    releaseArgs(rdArgs);

    /* Return w/o result */
    return(NULL);

}
