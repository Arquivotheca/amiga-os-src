/*** main.c *****************************************************************
 *
 *  photocd.library Test
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright © 1993 Commodore-Amiga, Inc.
 *
 ****************************************************************************/

/*
 *  Amiga includes
 */

#include <exec/types.h>
#include <exec/memory.h>

#include <dos/dos.h>
#include <dos/datetime.h>

#include <utility/tagitem.h>

#include <graphics/modeid.h>

#include <intuition/intuition.h>

#include <libraries/lowlevel.h>

#include <libraries/photocd.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/utility_protos.h>
#include <clib/intuition_protos.h>
#include <clib/lowlevel_protos.h>
#include <clib/photocd_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/lowlevel_pragmas.h>
#include <pragmas/photocd_pragmas.h>

/*
 *  ANSI includes
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
 *  Local definitions
 */

/* Unpack minute, second, and frame from 32-bit unsigned long word
   or cd.device/struct RMSF */
#define MSFMinute(msf)  ( (msf) >> 16 & 0xFF )
#define MSFSecond(msf)  ( (msf) >> 8 & 0xFF )
#define MSFFrame(msf)   ( (msf) & 0xFF )

struct RDArgs *rdArgs=NULL;

struct Library *PhotoCDBase=NULL,
    *IntuitionBase=NULL,
    *LowLevelBase=NULL,
    *UtilityBase=NULL;

extern struct Library *SysBase, *DOSBase;

void test(void);
int main(int argc,char *argv[]);
void goodbye(int returnCode);
BOOL writeQRT(STRPTR fileName,UBYTE *imageBuffer,ULONG cbImageBuffer,
    UWORD width,UWORD height);

/*
 *  Local constants
 */

#define FORMAT_DEF  (FORMAT_CDN+1)  /* Default format; should be
                                       defined in dos/datetime.h */
#define PROGRAM_NAME    "PhotoCD"   /* Program name */

#define KICKSTART_VERSION   40
#define WORKBENCH_VERSION   40

/********************************************************************
 *                                                                  *
 *  test()  -   test of photocd.library functions                   *
 *                                                                  *
 ********************************************************************/

void test(void)
{

    struct {

        LONG    quiet;
        LONG    disc;
        LONG    session;
        LONG    buffer;
        LONG    *image;
        LONG    *resolution;
        LONG    all;
        LONG    rgb;
        LONG    file;
        LONG    *lines;
        LONG    screen;

    } cliArgs;
#define cliTemplate "QUIET/S,DISC/S,SESSION/S,BUFFER/S,IMAGE/N,RES=RESOLUTION/N,ALL/S,RGB/S,FILE/S,LINES/N/K,SCREEN/S"

    BOOL fIsPhotoCD;

    void *pcdHandle;

    struct TagItem *discInfo;

    int session, nSessions;
    struct TagItem *sessionInfo;

    int image, nImages;
    struct TagItem *imageInfo;

    int resolution;
    UWORD width, height;
    int lines;

    UBYTE *imageBuffer;

    ULONG errorCode;

    struct DateStamp *pDateStamp;
    struct DateTime dateTime;

    ULONG msfPack;

    UBYTE dayString[LEN_DATSTRING];
    UBYTE dateString[LEN_DATSTRING];
    UBYTE timeString[LEN_DATSTRING];

    ULONG firstImage, lastImage;
    UBYTE firstResolution, lastResolution;

    UBYTE qrtFilename[32];

    struct EClockVal eclockVal;
    ULONG fracSecs;

    struct Screen *screen;

    LONG startLine, endLine;

    /*
     *  Command-line parsing
     */

    memset(&cliArgs,0,sizeof(cliArgs));
    rdArgs=ReadArgs(cliTemplate,(LONG *) &cliArgs,NULL);
    if (!rdArgs) {
        PrintFault(IoErr(),PROGRAM_NAME);
        goodbye(RETURN_ERROR);
    }

    /*
     *  IsPhotoCD()
     */

    if (cliArgs.disc) {
        if (!cliArgs.quiet) {
            printf("IsPhotoCD()\n");
        }
        fIsPhotoCD=IsPhotoCD();
        if (!cliArgs.quiet) {
            printf("    Disc is%sPhoto CD\n",fIsPhotoCD?" ":" not ");
        }
    }

    /*
     *  OpenPhotoCD()
     */

    if (!cliArgs.quiet) {
        printf("OpenPhotoCD()\n");
    }
    pcdHandle=OpenPhotoCD(
        PCD_ErrorCode,&errorCode,
        TAG_DONE,NULL);
    if (!cliArgs.quiet) {
        printf("    =$%08lx\n",pcdHandle);
    }

    /*
     *  ObtainPhotoCDInfo(PCD_Disc)
     */

    if (cliArgs.disc && pcdHandle) {

        if (!cliArgs.quiet) {
            printf("ObtainPhotoCDInfo()\n");
        }

        discInfo=ObtainPhotoCDInfo(pcdHandle,
            PCD_ErrorCode,&errorCode,
            TAG_DONE);
        if (discInfo) {

            nSessions=GetTagData(PCDDisc_nSessions,-1,discInfo);
            nImages=GetTagData(PCDDisc_nImages,-1,discInfo);

            if (!cliArgs.quiet) {
                printf("    PCDDisc_Signature=%s\n",
                    GetTagData(PCDDisc_Signature,(ULONG) "NULL",discInfo));

                printf("    PCDDisc_Version=%d\n",
                    (UBYTE) GetTagData(PCDDisc_Version,-1,discInfo));

                printf("    PCDDisc_Revision=%d\n",
                    (UBYTE) GetTagData(PCDDisc_Revision,-1,discInfo));

                printf("    PCDDisc_SerNo=%s\n",
                    GetTagData(PCDDisc_SerNo,(ULONG) "NULL",discInfo));

                if (pDateStamp=(struct DateStamp *) GetTagData(PCDDisc_CreateStamp,NULL,discInfo)) {

                    dateTime.dat_Stamp=*pDateStamp;
                    dateTime.dat_Format=FORMAT_DEF;
                    dateTime.dat_Flags=NULL;
                    dateTime.dat_StrDay=dayString;
                    dateTime.dat_StrDate=dateString;
                    dateTime.dat_StrTime=timeString;
                    if (DateToStr(&dateTime)) {
                        printf("    PCDDisc_CreateStamp=%s %s %s UTC\n",
                            dateTime.dat_StrDay,
                            dateTime.dat_StrDate,
                            dateTime.dat_StrTime);
                    } else {
                        printf("    PCDDisc_CreateStamp=Error in DateToStr()\n");
                    }

                } else {

                    printf("    PCDDisc_CreateStamp=NULL\n");

                }

                if (pDateStamp=(struct DateStamp *) GetTagData(PCDDisc_ModifyStamp,NULL,discInfo)) {

                    dateTime.dat_Stamp=*pDateStamp;
                    dateTime.dat_Format=FORMAT_DEF;
                    dateTime.dat_Flags=NULL;
                    dateTime.dat_StrDay=dayString;
                    dateTime.dat_StrDate=dateString;
                    dateTime.dat_StrTime=timeString;
                    if (DateToStr(&dateTime)) {
                        printf("    PCDDisc_ModifyStamp=%s %s %s UTC\n",
                            dateTime.dat_StrDay,
                            dateTime.dat_StrDate,
                            dateTime.dat_StrTime);
                    } else {
                        printf("    PCDDisc_ModifyStamp=Error in DateToStr()\n");
                    }

                } else {

                    printf("    PCDDisc_ModifyStamp=NULL\n");

                }

                printf("    PCDDisc_nImages=%d\n",nImages);

                printf("    PCDDisc_IntrlvADPCM=%d\n",GetTagData(PCDDisc_IntrlvADPCM,-1,discInfo));

                printf("    PCDDisc_IntrlvImage=%d\n",GetTagData(PCDDisc_IntrlvImage,-1,discInfo));

                printf("    PCDDisc_MinRes=%d\n",GetTagData(PCDDisc_MinRes,PHOTOCD_RES_BAD,discInfo));

                printf("    PCDDisc_MaxRes=%d\n",GetTagData(PCDDisc_MaxRes,PHOTOCD_RES_BAD,discInfo));

                msfPack=GetTagData(PCDDisc_LeadoutStart,-1,discInfo);
                if (msfPack!=-1) {
                    printf("    PCDDisc_LeadoutStart=%02d:%02d:%02d\n",
                        MSFMinute(msfPack),MSFSecond(msfPack),MSFFrame(msfPack));
                } else {
                    printf("    PCDDisc_LeadoutStart=NULL\n");
                }

                printf("    PCDDisc_nSessions=%d\n",nSessions);

            }

    /*
     *  ReleasePhotoCDInfo()
     */

            if (!cliArgs.quiet) {
                printf("ReleasePhotoCDInfo()\n");
            }
            ReleasePhotoCDInfo(discInfo);

        } else {

            printf("    Error %d\n",errorCode);

        }

    }

    /*
     *  ObtainPhotoCDInfo(PCD_Session)
     */

    if (cliArgs.session && pcdHandle) {

        for (session=1;session<=nSessions;session++) {

            if (!cliArgs.quiet) {
                printf("ObtainPhotoCDInfo(PCD_Session,%d)\n",session);
            }

            sessionInfo=ObtainPhotoCDInfo(pcdHandle,
                PCD_Session, session,
                PCD_ErrorCode,&errorCode,
                TAG_DONE);

            if (sessionInfo) {

                if (!cliArgs.quiet) {

                    printf("    PCDSess_nImages=%d\n",
                        GetTagData(PCDSess_nImages,-1,sessionInfo));

                    msfPack=GetTagData(PCDSess_CDDAStart,-1,sessionInfo);
                    if (msfPack!=-1) {
                        if (!cliArgs.quiet) {
                            printf("    PCDSess_CDDAStart=%02d:%02d:%02d\n",
                                MSFMinute(msfPack),MSFSecond(msfPack),MSFFrame(msfPack));
                        }
                    } else {
                        if (!cliArgs.quiet) {
                            printf("    PCDSess_CDDAStart=NULL\n");
                        }
                    }

                    msfPack=GetTagData(PCDSess_LeadoutStart,-1,sessionInfo);
                    if (msfPack!=-1) {
                        printf("    PCDSess_LeadoutStart=%02d:%02d:%02d\n",
                            MSFMinute(msfPack),MSFSecond(msfPack),MSFFrame(msfPack));
                    } else {
                        printf("    PCDSess_LeadoutStart=NULL\n");
                    }

                    printf("    PCDSess_WrtrVndr=%s\n",
                        GetTagData(PCDSess_WrtrVndr,(ULONG) "NULL",sessionInfo));

                    printf("    PCDSess_WrtrProd=%s\n",
                        GetTagData(PCDSess_WrtrProd,(ULONG) "NULL",sessionInfo));

                    printf("    PCDSess_WrtrVer=%d\n",
                        GetTagData(PCDSess_WrtrVer,-1,sessionInfo));

                    printf("    PCDSess_WrtrRev=%d\n",
                        GetTagData(PCDSess_WrtrRev,-1,sessionInfo));

                    if (pDateStamp=(struct DateStamp *) GetTagData(PCDSess_WrtrDate,NULL,sessionInfo)) {

                        dateTime.dat_Stamp=*pDateStamp;
                        dateTime.dat_Format=FORMAT_DEF;
                        dateTime.dat_Flags=NULL;
                        dateTime.dat_StrDay=dayString;
                        dateTime.dat_StrDate=dateString;
                        dateTime.dat_StrTime=timeString;
                        if (DateToStr(&dateTime)) {
                            printf("    PCDSess_WrtrDate=%s %s %s UTC\n",
                                dateTime.dat_StrDay,
                                dateTime.dat_StrDate,
                                dateTime.dat_StrTime);
                        } else {
                            printf("    PCDSess_WrtrDate=Error in DateToStr()\n");
                        }

                    } else {

                        printf("    PCDSess_WrtrDate=NULL\n");

                    }

                    printf("    PCDSess_WrtrSerNo=%s\n",
                    GetTagData(PCDSess_WrtrSerNo,(ULONG) "NULL",sessionInfo));

                    if (pDateStamp=(struct DateStamp *) GetTagData(PCDSess_CreateStamp,NULL,sessionInfo)) {

                        dateTime.dat_Stamp=*pDateStamp;
                        dateTime.dat_Format=FORMAT_DEF;
                        dateTime.dat_Flags=NULL;
                        dateTime.dat_StrDay=dayString;
                        dateTime.dat_StrDate=dateString;
                        dateTime.dat_StrTime=timeString;
                        if (DateToStr(&dateTime)) {
                            printf("    PCDSess_CreateStamp=%s %s %s UTC\n",
                                dateTime.dat_StrDay,
                                dateTime.dat_StrDate,
                                dateTime.dat_StrTime);
                        } else {
                            printf("    PCDSess_CreateStamp=Error in DateToStr()\n");
                        }

                    } else {

                        printf("    PCDSess_CreateStamp=NULL\n");

                    }

                }

    /*
     *  ReleasePhotoCDInfo()
     */
            if (!cliArgs.quiet) {

                printf("ReleasePhotoCDInfo()\n");

            }
                ReleasePhotoCDInfo(sessionInfo);

            } else {

                printf("    Error %d\n",errorCode);

            }

        }

    }

    /*
     *  ObtainPhotoCDInfo(PCD_Image)
     */

    /* Determine starting/end image */
    if (cliArgs.all) {
        firstImage=1;
        lastImage=nImages;
    } else if (cliArgs.image) {
        firstImage=*cliArgs.image;
        lastImage=*cliArgs.image;
    } else {
        firstImage=1;   /* Effectively no images */
        lastImage=0;
    }

    if (pcdHandle) {

        for (image=firstImage;image<=lastImage;image++) {

            if (!cliArgs.quiet) {
                printf("ObtainPhotoCDInfo(PCD_Image,%d)\n",image);
            }
            imageInfo=ObtainPhotoCDInfo(pcdHandle,
                PCD_Image, image,
                PCD_ErrorCode,&errorCode,
                TAG_DONE);
            if (imageInfo) {

                if (!cliArgs.quiet) {

                    printf("    PCDImg_4BaseHCT=%ld\n",
                        GetTagData(PCDImg_4BaseHCT,-1,imageInfo));

                    printf("    PCDImg_IPE=%d\n",
                        GetTagData(PCDImg_IPE,-1,imageInfo));

                    printf("    PCDImg_ResOrder=%d\n",
                        GetTagData(PCDImg_ResOrder,-1,imageInfo));

                    printf("    PCDImg_Rotation=%d\n",
                        GetTagData(PCDImg_Rotation,-1,imageInfo));

                    printf("    PCDImg_4BaseStop=$%04x\n",
                        (UWORD) GetTagData(PCDImg_4BaseStop,-1,imageInfo));

                    printf("    PCDImg_16BaseStop=$%04x\n",
                        (UWORD) GetTagData(PCDImg_16BaseStop,-1,imageInfo));

                    printf("    PCDImg_IPEStop=$%04x\n",
                        (UWORD) GetTagData(PCDImg_IPEStop,-1,imageInfo));

                    printf("    PCDImg_IntrlvADPCM=%d\n",
                        GetTagData(PCDImg_IntrlvADPCM,-1,imageInfo));

                    printf("    PCDImg_IntrlvImage=%d\n",
                        GetTagData(PCDImg_IntrlvImage,-1,imageInfo));

                    printf("    PCDImg_PrefFast=%d\n",
                        GetTagData(PCDImg_PrefFast,-1,imageInfo));

                    printf("    PCDImg_PrefRes=%d\n",
                        GetTagData(PCDImg_PrefRes,-1,imageInfo));

                    printf("    PCDImg_MagX=%d\n",
                        GetTagData(PCDImg_MagX,-1,imageInfo));

                    printf("    PCDImg_MagY=%d\n",
                        GetTagData(PCDImg_MagY,-1,imageInfo));

                    printf("    PCDImg_MagFactor=%d\n",
                        GetTagData(PCDImg_MagFactor,-1,imageInfo));

                    printf("    PCDImg_DispOffX=%d\n",
                        GetTagData(PCDImg_DispOffX,-1,imageInfo));

                    printf("    PCDImg_DispOffY=%d\n",
                        GetTagData(PCDImg_DispOffY,-1,imageInfo));

                    printf("    PCDImg_Transition=%d\n",
                        GetTagData(PCDImg_Transition,-1,imageInfo));

                    printf("    PCDImg_Signature=%s\n",
                        GetTagData(PCDImg_Signature,(ULONG) "NULL",imageInfo));

                    printf("    PCDImg_SpecVer=%d\n",
                        GetTagData(PCDImg_SpecVer,-1,imageInfo));

                    printf("    PCDImg_SpecRev=%d\n",
                        GetTagData(PCDImg_SpecRev,-1,imageInfo));

                    printf("    PCDImg_PIWVer=%d\n",
                        GetTagData(PCDImg_PIWVer,-1,imageInfo));

                    printf("    PCDImg_PIWRev=%d\n",
                        GetTagData(PCDImg_PIWRev,-1,imageInfo));

                    printf("    PCDImg_16BaseMag=%d.%d\n",
                        GetTagData(PCDImg_16BaseMag,-1,imageInfo)/100,
                        GetTagData(PCDImg_16BaseMag,-1,imageInfo)%100);

                    if (pDateStamp=(struct DateStamp *) GetTagData(PCDImg_ScanStamp,
                        NULL,imageInfo)) {

                        dateTime.dat_Stamp=*pDateStamp;
                        dateTime.dat_Format=FORMAT_DEF;
                        dateTime.dat_Flags=NULL;
                        dateTime.dat_StrDay=dayString;
                        dateTime.dat_StrDate=dateString;
                        dateTime.dat_StrTime=timeString;
                        if (DateToStr(&dateTime)) {
                            printf("    PCDImg_ScanStamp=%s %s %s UTC\n",
                                dateTime.dat_StrDay,
                                dateTime.dat_StrDate,
                                dateTime.dat_StrTime);
                        } else {
                            printf("    PCDImg_ScanStamp=Error in DateToStr()\n");
                        }

                    } else {

                        printf("    PCDSess_ScanStamp=NULL\n");

                    }

                    if (pDateStamp=(struct DateStamp *) GetTagData(PCDImg_ModifyStamp,
                        NULL,imageInfo)) {

                        dateTime.dat_Stamp=*pDateStamp;
                        dateTime.dat_Format=FORMAT_DEF;
                        dateTime.dat_Flags=NULL;
                        dateTime.dat_StrDay=dayString;
                        dateTime.dat_StrDate=dateString;
                        dateTime.dat_StrTime=timeString;
                        if (DateToStr(&dateTime)) {
                            printf("    PCDImg_ModifyStamp=%s %s %s UTC\n",
                                dateTime.dat_StrDay,
                                dateTime.dat_StrDate,
                                dateTime.dat_StrTime);
                        } else {
                            printf("    PCDImg_ModifyStamp=Error in DateToStr()\n");
                        }

                    } else {

                        printf("    PCDImg_ModifyStamp=NULL\n");

                    }

                    printf("    PCDImg_Medium=%d\n",GetTagData(PCDImg_Medium,-1,imageInfo));

                    printf("    PCDImg_ProdType=%s\n",
                        GetTagData(PCDImg_ProdType,(ULONG) "NULL",imageInfo));

                    printf("    PCDImg_ScnrVndr=%s\n",
                        GetTagData(PCDImg_ScnrVndr,(ULONG) "NULL",imageInfo));

                    printf("    PCDImg_ScnrProd=%s\n",
                        GetTagData(PCDImg_ScnrProd,(ULONG) "NULL",imageInfo));

                    printf("    PCDImg_ScnrVer=%d\n",
                        GetTagData(PCDImg_ScnrVer,-1,imageInfo));

                    printf("    PCDImg_ScnrRev=%d\n",
                        GetTagData(PCDImg_ScnrRev,-1,imageInfo));

                    if (pDateStamp=(struct DateStamp *) GetTagData(PCDImg_ScnrDate,
                        NULL,imageInfo)) {

                        dateTime.dat_Stamp=*pDateStamp;
                        dateTime.dat_Format=FORMAT_DEF;
                        dateTime.dat_Flags=NULL;
                        dateTime.dat_StrDay=dayString;
                        dateTime.dat_StrDate=dateString;
                        dateTime.dat_StrTime=timeString;
                        if (DateToStr(&dateTime)) {
                            printf("    PCDImg_ScnrDate=%s %s %s UTC\n",
                                dateTime.dat_StrDay,
                                dateTime.dat_StrDate,
                                dateTime.dat_StrTime);
                        } else {
                            printf("    PCDImg_ScnrDate=Error in DateToStr()\n");
                        }

                    } else {

                        printf("    PCDImg_ScnrDate=NULL\n");

                    }

                    printf("    PCDImg_ScnrSerNo=%s\n",
                        GetTagData(PCDImg_ScnrSerNo,(ULONG) "NULL",imageInfo));

                    printf("    PCDImg_ScnrPixel=%d.%d\n",
                        GetTagData(PCDImg_ScnrPixel,-1,imageInfo)/100,
                        GetTagData(PCDImg_ScnrPixel,-1,imageInfo)%100);

                    printf("    PCDImg_PIWMfgr=%s\n",
                        GetTagData(PCDImg_PIWMfgr,(ULONG) "NULL",imageInfo));

                    printf("    PCDImg_PhtfinCharSet=%d\n",
                        GetTagData(PCDImg_PhtfinCharSet,-1,imageInfo));

                    printf("    PCDImg_PhtfinEscape=%s\n",
                        GetTagData(PCDImg_PhtfinEscape,(ULONG) "NULL",imageInfo));

                    printf("    PCDImg_PhtfinName=%s\n",
                        GetTagData(PCDImg_PhtfinName,(ULONG) "NULL",imageInfo));

                    printf("    PCDImg_SBAVer=%d\n",
                        GetTagData(PCDImg_SBAVer,-1,imageInfo));

                    printf("    PCDImg_SBARev=%d\n",
                        GetTagData(PCDImg_SBARev,-1,imageInfo));

                    printf("    PCDImg_SBACommand=%d\n",
                        GetTagData(PCDImg_SBACommand,-1,imageInfo));

                    printf("    PCDImg_SBAData=$%08lx\n",
                        GetTagData(PCDImg_SBAData,NULL,imageInfo));

                    printf("    PCDImg_Copyright=%s\n",
                        GetTagData(PCDImg_Copyright,(ULONG) "NULL",imageInfo));

                }

    /*
     *  ReleasePhotoCDInfo()
     */

                if (!cliArgs.quiet) {
                    printf("ReleasePhotoCDInfo()\n");
                }

                ReleasePhotoCDInfo(imageInfo);

            } else {

                printf("    Error %d\n",errorCode);

            }

        }

    }

    /*
     *  PCDImageBufferSize()
     */

    if (cliArgs.buffer) {

        if (!cliArgs.quiet) {
            printf("PCDImageBufferSize()\n");
            printf("     192 x  128=%9d\n",PCDImageBufferSize(192,128));
            printf("     384 x  256=%9d\n",PCDImageBufferSize(384,256));
            printf("     768 x  512=%9d\n",PCDImageBufferSize(768,512));
            printf("    1536 x 1024=%9d\n",PCDImageBufferSize(1536,1024));
            printf("    3072 x 2048=%9d\n",PCDImageBufferSize(3072,2048));
        }

    }

    if (cliArgs.buffer) {

        for (resolution=PHOTOCD_RES_BASE16;
             resolution<=PHOTOCD_RES_16BASE;
             resolution++) {

    /*
     *  GetPCDResolution()
     */

            if (!cliArgs.quiet) {

                printf("GetPCDResolution(resolution=%d,...)\n",(int) resolution);

            }

            if (GetPCDResolution(resolution,&width,&height)) {

                if (!cliArgs.quiet) {
                    printf("    Width =%4d\n",width);
                    printf("    Height=%4d\n",height);
                }

            } else {

                if (!cliArgs.quiet) {
                    printf("    Fail\n");
                }

            }

    /*
     *  AllocPCDImageBuffer()
     */

            if (!cliArgs.quiet) {
                printf("AllocPCDImageBuffer(resolution=%d)\n",(int) resolution);
            }
            imageBuffer=AllocPCDImageBuffer(resolution,
                TAG_DONE);
            if (!cliArgs.quiet) {
                printf("    =$%08lx\n",imageBuffer);
            }
            if (!imageBuffer) {

                if (!cliArgs.quiet) {
                    printf(
                        "AllocPCDImageBuffer(resolution=%d,PCD_Lines,16)\n",
                            (int) resolution);
                }
                imageBuffer=AllocPCDImageBuffer(resolution,
                    PCD_Lines, 16,
                    TAG_DONE);
                if (!cliArgs.quiet) {
                    printf("    =$%08lx\n",imageBuffer);
                }

            }

            if (imageBuffer) {

                /*
                 *  FreePCDImageBuffer()
                 */

                if (!cliArgs.quiet) {
                    printf("FreePCDImageBuffer()\n");
                }
                FreePCDImageBuffer(imageBuffer);

            }

        }

    }

    /*
     *  GetPCDImageData()
     */

    /* Determine resolution */
    if (cliArgs.resolution) {
        firstResolution=lastResolution=*cliArgs.resolution;
    } else {
        firstResolution=PHOTOCD_RES_BASE16;
        lastResolution=PHOTOCD_RES_16BASE;
    }

    if (pcdHandle) {

        for (image=firstImage;image<=lastImage;image++) {

            for (resolution=firstResolution;
                 resolution<=lastResolution;
                 resolution++) {

                if (cliArgs.screen) {
                    screen=OpenScreenTags(NULL,
                        SA_Left, 0,
                        SA_Top, 0,
                        SA_Depth, 8,
                        SA_DisplayID, HIRESLACE_KEY,
                        SA_Overscan, OSCAN_MAX,
                        SA_ErrorCode, &errorCode,
                        TAG_DONE);
                    if (!screen) {
                        printf("Unable to open screen (error %lu) -- continuing\n",
                            errorCode);
                    }
                } else {
                    screen=NULL;
                }

                GetPCDResolution(resolution,&width,&height);

                if (cliArgs.lines) {
                    lines=*cliArgs.lines;
                } else {
                    lines=height;
                }

                if (!cliArgs.quiet) {
                    printf("AllocPCDImageBuffer(resolution=%d,TAG_DONE)",
                        (int) resolution);
                }
                imageBuffer=AllocPCDImageBuffer(resolution,
                    PCD_Lines, lines,
                    TAG_DONE);
                if (!cliArgs.quiet) {
                    printf("    =$%08lx\n",imageBuffer);
                }
                if (imageBuffer) {

                    if (!cliArgs.quiet) {
                        printf("GetPCDImageData(pcdHandle=$%08lx,imageBuffer=$%08lx,\n  PCD_Image, %d,\n    PCD_Resolution, %d\n    PCD_Format, %d, ...\n)\n",
                            pcdHandle,
                            imageBuffer,
                            image,resolution,
                            cliArgs.rgb?PHOTOCD_FORMAT_RGB:PHOTOCD_FORMAT_YUV);
                    }

                    ElapsedTime(&eclockVal);

                    for (startLine=0;startLine<height;startLine+=lines) {

                        endLine=startLine+lines-1;

                        if (!GetPCDImageData(pcdHandle,imageBuffer,
                            PCD_Image, image,
                            PCD_Resolution, resolution,
                            PCD_Format, cliArgs.rgb?PHOTOCD_FORMAT_RGB:PHOTOCD_FORMAT_YUV,
                            PCD_StartLine, startLine,
                            PCD_EndLine, endLine,
                            PCD_ErrorCode, &errorCode)) {
                            printf("Error %lu\n",errorCode);
                        }

                    }

                    fracSecs=ElapsedTime(&eclockVal);
                    printf("Elapsed time=%lu.%lu\n",
                        fracSecs>>16,
                        fracSecs&0x0000FFFF);

                    if (!cliArgs.quiet) {
                        printf("    Success\n");
                    }

                    if (cliArgs.file) {
                        sprintf(qrtFilename,"img%04d.qrt",image);
                        if (!writeQRT(qrtFilename,
                            imageBuffer,
                            PCDImageBufferSize(width,lines),
                            width,lines)) {
                            printf("    Error writing %s\n",qrtFilename);
                        }
                    }

                    FreePCDImageBuffer(imageBuffer);

                    if (screen) {
                        CloseScreen(screen);
                    }

                }

            }

        }

    }

    /*
     *  ClosePhotoCD()
     */

    if (pcdHandle) {
        if (!cliArgs.quiet) {
            printf("ClosePhotoCD(pcdHandle=$%08lx)\n",pcdHandle);
        }
        ClosePhotoCD(pcdHandle);
    }

    /*
     *  Terminate test
     */

    /* Release dos.library/ReadArgs() control structure */
    if (rdArgs) {
        FreeArgs(rdArgs);
    }

}

/********************************************************************
 *                                                                  *
 * goodbye() - exit                                                 *
 *                                                                  *
 ********************************************************************/

void goodbye(int returnCode)
{

    /*
     *  Close libraries
     */

    if (PhotoCDBase) {
        CloseLibrary(PhotoCDBase);
    }

    if (IntuitionBase) {
        CloseLibrary(IntuitionBase);
    }

    if (LowLevelBase) {
        CloseLibrary(LowLevelBase);
    }

    /*
     *  Exit
     */

    exit(returnCode);

}

/********************************************************************
 *                                                                  *
 * main() - entry point                                             *
 *                                                                  *
 ********************************************************************/

int main(int argc,char *argv[])
{

    /*
     *  Open libraries
     */

    UtilityBase=OpenLibrary("utility.library",KICKSTART_VERSION);
    if (!UtilityBase) {
        printf("Error opening utility.library V%d\n",KICKSTART_VERSION);
        goodbye(RETURN_FAIL);
    }

    LowLevelBase=OpenLibrary("lowlevel.library",KICKSTART_VERSION);
    if (!LowLevelBase) {
        printf("Error opening lowlevel.library V%d\n",KICKSTART_VERSION);
        goodbye(RETURN_FAIL);
    }

    IntuitionBase=OpenLibrary("intuition.library",KICKSTART_VERSION);
    if (!IntuitionBase) {
        printf("Error opening intuition.library V%d\n",KICKSTART_VERSION);
        goodbye(RETURN_FAIL);
    }

    PhotoCDBase=OpenLibrary("photocd.library",KICKSTART_VERSION);
    if (!PhotoCDBase) {
        printf("Error opening photocd.library V%d\n",WORKBENCH_VERSION);
        goodbye(RETURN_FAIL);
    }

    /*
     *  Run test
     */

    test();

    /*
     *  Fall-through
     */

    goodbye(RETURN_OK);

}

/********************************************************************
 *                                                                  *
 *  writeQRT()  -   write QRT file                                  *
 *                                                                  *
 ********************************************************************/

#define ByteSwap(word) ((((word&0x00FF)<<8)|(word&0xFF00)>>8))

BOOL writeQRT(STRPTR fileName,UBYTE *imageBuffer,ULONG cbImageBuffer,
    UWORD width,UWORD height)
{

    /* Note:    This code, in a word, sucks. However, my goal for
                today was to successfully display a Photo CD image
                (using an image viewer) to test the basic operation
                of the library. Since 24-bit ILBM files are a
                pain in the ass, this is what I'm doing. So sue me.

                    jjszucs 12 Nov 93
    */

    UWORD y, x;
    UWORD swap;

    BPTR fileHandle;

    fileHandle=Open(fileName,MODE_NEWFILE);
    if (!fileHandle) {
        return FALSE;
    }

    /* Image header is:

        (UWORD) Width (Intel byte order)
        (UWORD) Height (Intel byte order)

    */
    swap=ByteSwap(width);
    Write(fileHandle,&swap,sizeof(swap));
    swap=ByteSwap(height);
    Write(fileHandle,&swap,sizeof(swap));
    Flush(fileHandle);

    /* Loop through lines */
    for (y=0;y<height;y++) {

        /* Line header is
            (UWORD) Line number */
        swap=ByteSwap(y);
        Write(fileHandle,&swap,sizeof(swap));
        Flush(fileHandle);

        /* Write red data */
        for (x=0;x<width;x++) {
            FPutC(fileHandle,imageBuffer[(y*width*3)+(x*3)]);
        }

        /* Write green data */
        for (x=0;x<width;x++) {
            FPutC(fileHandle,imageBuffer[(y*width*3)+(x*3)+1]);
        }

        /* Write blue data */
        for (x=0;x<width;x++) {
            FPutC(fileHandle,imageBuffer[(y*width*3)+(x*3)+2]);
        }

        Flush(fileHandle);

    }

    Close(fileHandle);

}
