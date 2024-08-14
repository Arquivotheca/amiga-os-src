/******************************************************************************
*
*	$Id: addmonitor.c,v 40.2 93/03/10 13:45:06 spence Exp $
*
******************************************************************************/

#define STRINGARRAY /* for <localestr/monitors.h> */

/* includes */
#include <exec/types.h>
#include <graphics/displayinfo.h>
#include <internal/displayinfo_internal.h>
#include <internal/vp_internal.h>
#include <exec/memory.h>
#include <graphics/gfxbase.h>
#include <graphics/view.h>
#include <graphics/monitor.h>
#include <graphics/displayinfo.h>
#include <libraries/locale.h>
#include <utility/tagitem.h>
#include <workbench/icon.h>
#include <workbench/startup.h>
#include <workbench/workbench.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <localestr/monitors.h>
#include <string.h>
#include <stdlib.h>

/* prototypes */
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/graphics_protos.h>
#include <clib/icon_protos.h>
#include <clib/utility_protos.h>
#include <clib/locale_protos.h>

/* direct ROM interface */
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/icon_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/locale_pragmas.h>

/* application includes */
#include "mspcinit.h"
#include "monitorstuff.h"

void do_db_startup(struct DisplayInfoRecord *root, UWORD MonitorID);

/*#define _DEBUG*/

#ifdef _DEBUG
#include <stdio.h>
#define D(x) {x}
#else
#define D(x)
#endif

/*****************************************************************************/

#ifndef DO_MULTISCAN_MONITOR
#define DO_MULTISCAN_MONITOR 0
#endif

#ifndef DO_DOUBLE_NTSC_MONITOR
#define DO_DOUBLE_NTSC_MONITOR   0
#endif

#ifndef DO_NTSC_MONITOR
#define DO_NTSC_MONITOR      0
#endif

#ifndef DO_PAL_MONITOR
#define DO_PAL_MONITOR       0
#endif

#ifndef DO_A2024_MONITOR
#define DO_A2024_MONITOR     0
#endif

#ifndef DO_EURO36_MONITOR
#define DO_EURO36_MONITOR    0
#endif

#ifndef DO_EURO72_MONITOR
#define DO_EURO72_MONITOR    0
#endif

#ifndef DO_SUPER72_MONITOR
#define DO_SUPER72_MONITOR   0
#endif

#ifndef DO_DOUBLE_PAL_MONITOR
#define DO_DOUBLE_PAL_MONITOR   0
#endif

#ifndef DO_MOTIVATOR_MONITOR
#define DO_MOTIVATOR_MONITOR	0
#endif

#ifndef DO_MOTIVATIM_MONITOR
#define DO_MOTIVATIM_MONITOR	0
#endif

#if (DO_NTSC_MONITOR)
#include "ntsc.h"
#include "ntsc_rev.h"
#endif

#if (DO_PAL_MONITOR)
#include "pal.h"
#include "pal_rev.h"
#endif

#if (DO_MULTISCAN_MONITOR)
#include "multiscan.h"
#include "multiscan_rev.h"
#endif

#if (DO_DOUBLE_NTSC_MONITOR)
#include "dblntsc.h"
#include "dblntsc_rev.h"
#endif

#if (DO_DOUBLE_PAL_MONITOR)
#include "dblpal.h"
#include "dblpal_rev.h"
#endif

#if (DO_SUPER72_MONITOR)
#include "super72.h"
#include "super72_rev.h"
#endif

#if (DO_EURO72_MONITOR)
#include "euro72.h"
#include "euro72_rev.h"
#endif

#if (DO_EURO36_MONITOR)
#include "euro36.h"
#include "euro36_rev.h"
#endif

#if (DO_A2024_MONITOR)
#include "a2024.h"
#include "a2024_rev.h"
#endif

#if (DO_MOTIVATOR_MONITOR)
#include "motivator.h"
#include "motivator_rev.h"
#endif

#if (DO_MOTIVATIM_MONITOR)
#include "motivatim.h"
#include "motivatim_rev.h"
#endif


struct Library *SysBase;
struct GfxBase *GfxBase;
struct Library *UtilityBase;
struct Library *DOSBase;

/*****************************************************************************/


/* private calls in gfx lib */
#pragma libcall GfxBase SetDisplayInfoData 2ee 2109805
#pragma libcall GfxBase AddDisplayInfoDataA 2e8 2109805

ULONG SetDisplayInfoData( DisplayInfoHandle handle, UBYTE * buf, ULONG size, ULONG tagID, ULONG displayID );
ULONG AddDisplayInfoDataA( DisplayInfoHandle handle, UBYTE * buf, ULONG size, ULONG tagID, ULONG displayID );


/*****************************************************************************/


#define	LIB_VERSION 37
#define GFXLIB_VERSION 39

/*****************************************************************************/


#define TEMPLATE        "HBSTRT/K,HBSTOP/K,VBSTRT/K,VBSTOP/K,MINROW/K,MINCOL/K,TOTROWS/K,TOTCLKS/K,BEAMCON0/K" VERSTAG
#define OPT_HBSTRT	0
#define OPT_HBSTOP	1
#define OPT_VBSTRT	2
#define OPT_VBSTOP	3
#define OPT_MINROW	4
#define OPT_MINCOL	5
#define OPT_TOTROWS	6
#define OPT_TOTCLKS	7
#define OPT_BEAMCON0	8
#define OPT_COUNT       9


/*****************************************************************************/


BOOL AddMonitor(ULONG *opts, LONG *optsX, struct GfxBase *GfxBase);
BOOL ConvToNum(STRPTR hexString, ULONG *value, APTR UtilityBase);
VOID BindNames(APTR GfxBase);


/*****************************************************************************/


int main(int argc, char *argv[])
{
struct WBArg      *wbarg;
BPTR               oldLock;
struct DiskObject *diskObj;
struct RDArgs     *rdargs = NULL;
struct Process	  *process;
STRPTR             argument;
LONG               opts[OPT_COUNT];
ULONG              optsX[OPT_COUNT];
LONG               failureLevel = RETURN_FAIL;
char               arg[100];
SHORT		   i;
SHORT		   cnt;
BOOL		   error = FALSE;

struct Library *IconBase;
struct WBStartup *WBenchMsg = NULL;

    SysBase = (*((struct SysBase **) 4));

    process = (struct Process *)FindTask(NULL);
    if (!(process->pr_CLI))
    {
        WaitPort(&process->pr_MsgPort);
        WBenchMsg = (struct WBStartup *) GetMsg(&process->pr_MsgPort);
    }

    if (DOSBase = OpenLibrary(DOSNAME,LIB_VERSION))
    {
        if (GfxBase = (struct GfxBase *)OpenLibrary(GRAPHICSNAME,GFXLIB_VERSION))
        {
            if ((!(GfxBase->GfxFlags & NEW_DATABASE)) && /* ONLY work with the old database implementation */
	        (UtilityBase = OpenLibrary("utility.library",LIB_VERSION)))
            {
                    if (IconBase = OpenLibrary(ICONNAME,LIB_VERSION))
                    {
                        failureLevel = RETURN_OK;
                        memset(opts,0,sizeof(opts));

                        if (!WBenchMsg)
                        {
                            rdargs = ReadArgs(TEMPLATE,opts,NULL);
                            diskObj = GetDiskObject(ICON_NAME);
                            oldLock = process->pr_CurrentDir;
                        }
                        else
                        {
                            wbarg = WBenchMsg->sm_ArgList;

                            oldLock = CurrentDir(wbarg->wa_Lock);
                            diskObj = GetDiskObject(wbarg->wa_Name);
                        }

                        if (diskObj)
                        {
                            D(kprintf("have diskobj\n");)
                            if (argv = (BYTE **) diskObj->do_ToolTypes)
                            {
                                while (argument = *argv++)
                                {
                                    i = 0;
                                    while ((argument[i] != NULL) && (argument[i] != '=') && (i < 100))
                                    {
                                        arg[i] = argument[i];
                                        i++;
                                    }
                                    arg[i] = NULL;

                                    if ((cnt = FindArg(TEMPLATE,arg)) >= 0)
                                    {
                                        D(kprintf("Found Arg %s, cnt = %ld, opts[cnt] = 0x%lx\n", arg, cnt, opts[cnt]);)
                                        if ((argument[i] == '=') && (opts[cnt] == NULL))
                                        {
                                            opts[cnt] = (ULONG)&argument[i+1];
                                        }
                                    }
                                }

                            }
                        }

                        cnt   = OPT_HBSTRT;
                        error = FALSE;
                        while (cnt < OPT_COUNT)
                        {
                            if (opts[cnt])
                            {
                                if (!(ConvToNum((STRPTR)opts[cnt],&optsX[cnt],UtilityBase)))
                                {
                                    D(kprintf("Conv error, cnt = %ld, opts[cnt] = 0x%lx\n", cnt, opts[cnt]);)
                                    error = TRUE;
                                }
                            }
                            cnt++;
                        }
                        if (diskObj)
                        {
                            FreeDiskObject(diskObj);
                        }
                        if (rdargs)
                        {
                            FreeArgs(rdargs);
                        }
                        CloseLibrary(IconBase);
                        CurrentDir(oldLock);
                    }

                    if (error == FALSE)
                    {
                        if (AddMonitor(opts,optsX,GfxBase))
                            BindNames(GfxBase);
                    }

                CloseLibrary(UtilityBase);
            }
            CloseLibrary(GfxBase);
        }
        CloseLibrary(DOSBase);
    }

    if (WBenchMsg)
    {
    	Forbid();
    	ReplyMsg(WBenchMsg);
    }

    return(failureLevel);
}


/*****************************************************************************/


BOOL ConvToNum(STRPTR hexString, ULONG *value, APTR UtilityBase)
{
char  c;
SHORT i;

    D(kprintf("Converting %s\n", hexString);)
    if ((hexString[0] != '0') || (ToUpper(hexString[1]) != 'X'))
    {
        return(FALSE);
    }

    i      = 2;
    *value = 0;
    for (;;)
    {
        c = ToUpper(hexString[i++]);
        if ((c >= '0') && (c <= '9'))
        {
            *value = *value*16 + c - '0';
        }
        else if ((c >= 'A') && (c <= 'F'))
        {
            *value = *value*16 + c - 'A' + 10;
        }
        else if (c != NULL)
        {
            return(FALSE);
        }
        else
        {
            return(TRUE);
        }
    }
}


/*****************************************************************************/


#if (DO_EURO36_MONITOR || DO_SUPER72_MONITOR || DO_EURO72_MONITOR || DO_DOUBLE_NTSC_MONITOR || DO_DOUBLE_PAL_MONITOR || DO_MULTISCAN_MONITOR || DO_A2024_MONITOR || DO_MOTIVATOR_MONITOR || DO_MOTIVATIM_MONITOR)
VOID db_avail(ULONG ID, struct DisplayInfoRecord *record, struct RawDisplayInfo *dinfo)
{
	/* not vanilla */

	struct DimensionInfo dims;
	BOOL HRDenise, Lisa;

	HRDenise = (GfxBase->ChipRevBits0 & GFXF_HR_DENISE);
	Lisa = (GfxBase->ChipRevBits0 & GFXF_AA_LISA);
	record->rec_Control = dinfo->ModeID;

	D(kprintf("ModeID = 0x%lx\n", dinfo->ModeID);)
	D(kprintf("initial availability = 0x%lx\n", dinfo->NotAvailable);)
	if (HRDenise && (dinfo->PropertyFlags & DIPF_IS_ECS))
	{
		dinfo->NotAvailable &= ~DI_AVAIL_NOCHIPS;
	}
	if ((Lisa) && (dinfo->PropertyFlags & DIPF_IS_AA))
	{
		/* Assume we can clear the NOCHIPS flag */
		dinfo->NotAvailable &= ~DI_AVAIL_NOCHIPS;

		/* but if we don't have the badwidth for the HAM or EHB... */
		if (dinfo->ModeID & (HAM | EXTRA_HALFBRITE))
		{
			if ((! (((dinfo->ModeID & SUPERHIRES) && (GfxBase->MemType == BANDWIDTH_4X))
			     ||
			    ((dinfo->ModeID & HIRES) && GfxBase->MemType)) )
			    &&
			    (dinfo->ModeID & (SUPERHIRES | HIRES)))
			{
				dinfo->NotAvailable |= DI_AVAIL_NOCHIPS;
				D(kprintf("NoChips\n");)
			}
		}
	}

	if(GetDisplayInfoData(record, (APTR)&dims, sizeof(struct DimensionInfo), DTAG_DIMS, 0))
	{
                D(kprintf("db_avail() - VideoOScan = (%ld, %ld) - (%ld, %ld)\n", dims.VideoOScan.MinX, dims.VideoOScan.MinY, dims.VideoOScan.MaxX, dims.VideoOScan.MaxY);)
		record->rec_ClipOScan.MinX =
			dims.VideoOScan.MinX;
		record->rec_ClipOScan.MaxX =
			dims.VideoOScan.MaxX;
		record->rec_ClipOScan.MinY =
			dims.VideoOScan.MinY;
		record->rec_ClipOScan.MaxY =
			dims.VideoOScan.MaxY;
	}
	if(GfxBase->system_bplcon0 & GENLOCK_VIDEO)
	{
		if(!(dinfo->PropertyFlags & DIPF_IS_GENLOCK))
		{
			dinfo->NotAvailable |= DI_AVAIL_NOTWITHGENLOCK;
		}
	}
}
#endif


/*****************************************************************************/


BOOL InitMonitor(struct MonitorSpec *mspc, STRPTR name, ULONG *opts, LONG *optsX, struct GfxBase *GfxBase)
{
    struct SpecialMonitor *sm;
    struct Library        *SysBase = (*((struct Library **) 4));
    BOOL IsAA = (GfxBase->ChipRevBits0 & GFXF_AA_LISA);

    sm = mspc->ms_Special;

    mspc->total_rows = TOTROWS;
    if (opts[OPT_TOTROWS])
    {
        mspc->total_rows = optsX[OPT_TOTROWS];
    }

    mspc->total_colorclocks = TOTCLKS;
    if (opts[OPT_TOTCLKS])
    {
        mspc->total_colorclocks = optsX[OPT_TOTCLKS];
    }

    mspc->min_row = MINROW;
    if (opts[OPT_MINROW])
    {
        mspc->min_row = optsX[OPT_MINROW];
    }

    mspc->BeamCon0 = BEAMCON0;
    if (opts[OPT_BEAMCON0])
    {
        mspc->BeamCon0 = optsX[OPT_BEAMCON0];
    }

    /* ratioh = 1/2 */
    mspc->ratioh = (mspc->total_colorclocks<<RATIO_FIXEDPART)/STANDARD_COLORCLOCKS;

    /* ratiov = 2 */
    mspc->ratiov = (STANDARD_COLORCLOCKS<<RATIO_FIXEDPART)/mspc->total_colorclocks;

    /* reset denise max displayable */
    mspc->DeniseMaxDisplayColumn = MAX_DENISE;
    mspc->DeniseMinDisplayColumn = MIN_DENISE;
    if (opts[OPT_MINCOL])
    {
        mspc->DeniseMinDisplayColumn = optsX[OPT_MINCOL];
    }

    /* set specific  hblank, hsync, beamcon0 for double-scan mode */

    /* monitor specific voodoo */

    if (sm)
    {
        sm->hblank.asi_Start    = HBSTRT;
        sm->hsync.asi_Start     = HSSTRT;
        sm->hsync.asi_Stop      = HSSTOP;
        sm->hblank.asi_Stop     = HBSTOP;
        sm->vblank.asi_Start    = VBSTRT;
        sm->vsync.asi_Start     = VSSTRT;
        sm->vsync.asi_Stop      = VSSTOP;
        sm->vblank.asi_Stop     = VBSTOP;

        if (opts[OPT_HBSTRT])
        {
            sm->hblank.asi_Start = optsX[OPT_HBSTRT];
        }

        if (opts[OPT_HBSTOP])
        {
            sm->hblank.asi_Stop = optsX[OPT_HBSTOP];
        }

        if (opts[OPT_VBSTRT])
        {
            sm->vblank.asi_Start = optsX[OPT_VBSTRT];
        }

        if (opts[OPT_VBSTOP])
        {
            sm->vblank.asi_Stop = optsX[OPT_VBSTOP];
        }
    }


#if (DO_A2024_MONITOR)
    GfxBase->crb_reserved[3] |= 0x80; /* V35 compatibile */
#endif

    if (mspc->ms_Node.xln_Name = AllocMem(strlen(name)+1,MEMF_PUBLIC))
    {
	strcpy(mspc->ms_Node.xln_Name,name);
	return(TRUE);
    }

    return(FALSE);
}


/*****************************************************************************/


VOID BindNames(APTR GfxBase)
{
struct RawNameInfo info;
struct QueryHeader header;
struct DisplayInfo dispInfo;
struct Library     *LocaleBase;
struct Catalog     *catalog = NULL;
ULONG              modeID;
ULONG              monitorID;
ULONG              extendedflag;
ULONG              i;
STRPTR             modeName;

    extendedflag = MONITOR_NUM ? EXTENDED_MODE : NULL;
    monitorID    = (MONITOR_NUM << 16 | extendedflag);

    modeID = INVALID_ID;
    if (LocaleBase = OpenLibrary("locale.library",38))
    {
            catalog = OpenCatalogA(NULL,"sys/monitors.catalog",NULL);
    }
	
    while ((modeID = NextDisplayInfo(modeID)) != INVALID_ID)
    {
	if ((modeID & MONITOR_ID_MASK) == monitorID)
	{
            modeName = NULL;
            i        = sizeof(AppStrings) / sizeof(struct AppString);
            while (!modeName && (i > 0))
            {
                i--;
                if (AppStrings[i].as_ID == modeID)
                    modeName = AppStrings[i].as_Str;
            }

            if (catalog)
            {
                modeName = GetCatalogStr(catalog, modeID, modeName);
            }

            if (modeName)
            {
                if (GetDisplayInfoData(NULL,(APTR)&dispInfo,sizeof(struct DisplayInfo),DTAG_DISP,modeID))
                {
                    if (!(dispInfo.NotAvailable & DI_AVAIL_NOMONITOR))
                    {
                        memset(&info,0,sizeof(info));
                        info.Header.StructID  = DTAG_NAME;
                        info.Header.SkipID    = TAG_SKIP;
                        info.Header.Length    = 4;
                        info.Header.DisplayID = modeID;
                        strncpy(info.Name,modeName,sizeof(info.Name));

                        if (GetDisplayInfoData(NULL,(APTR)&header,sizeof(header),DTAG_NAME,modeID))
                        {
                            SetDisplayInfoData(NULL,(APTR)&info,sizeof(info),DTAG_NAME,modeID);
                        }
                        else
                        {
                            AddDisplayInfoDataA(NULL,(APTR)&info,sizeof(info),DTAG_NAME,modeID);
                        }
                    }
                }
            }
	}
    }
    if (LocaleBase)
    {
    	CloseLibrary(LocaleBase);
    }
}


/*****************************************************************************/


BOOL AddMonitor(ULONG *opts, LONG *optsX, struct GfxBase *GfxBase)
{
    struct MonitorInfo *mntr;
    struct DisplayInfo *info;
    struct Library     *SysBase = (*((struct Library **) 4));
    struct MonitorSpec *mspc;
    ULONG  extendedflag;
    ULONG  modeID;
    ULONG  ID = INVALID_ID, LastID = INVALID_ID;
    struct DisplayInfo  queryinfo;
    struct MonitorInfo  querymonitor;
    DisplayInfoHandle   handle;
    int numbytes;

#if (DO_A2024_MONITOR)
    BOOL LoadA2024Driver();
#endif

    mntr = &querymonitor;
    info = &queryinfo;

    extendedflag = MONITOR_NUM ? EXTENDED_MODE : NULL;
    modeID       = (MONITOR_NUM << 16 | extendedflag);

    if (mspc = OpenMonitor(MONITOR_NAME,modeID))
    {
	D(kprintf("OpenMOnitr1\n");)
        CloseMonitor(mspc);
        return(TRUE);
    }

    if (mspc = OpenMonitor(NULL,modeID))
    {
	D(kprintf("OpenMOnitr2\n");)
        CloseMonitor(mspc);
        return(TRUE);
    }

#if (!((DO_NTSC_MONITOR) || (DO_PAL_MONITOR)))
    do_db_startup(GfxBase->DisplayInfoDataBase, MONITOR_NUM);
#endif

    /*
    ** If FindDisplayInfo() cannot find the MONITOR, it will try to unload
    ** it from ROM into RAM - spence Feb 21 1991
    **/
    FindDisplayInfo((MONITOR_NUM << 16) | 0x1000);

    if (mspc = (struct MonitorSpec *)GfxNew(MONITOR_SPEC_TYPE))
    {

	D(kprintf("Do mspi\n");)
        MonitorSpecInit(GfxBase, mspc, MSFLAGS);
	D(kprintf("Done mspi\n");)
        if (InitMonitor(mspc,MONITOR_NAME,opts,optsX,GfxBase))
        {
            AddHead(&GfxBase->MonitorList, mspc); /* ok to add monitor to system */

            while ((ID = NextDisplayInfo(ID)) != INVALID_ID)
            {
                if ((ID & ~0xFFFF) == (modeID & ~0xFFFF))
                {
                    if (handle = (DisplayInfoHandle)FindDisplayInfo(ID))
                    {
			LastID = ID;
                        if (numbytes = GetDisplayInfoData(handle,(UBYTE *)info,sizeof(*info),DTAG_DISP,ID ))
                        {
                            if ((((ID & ~0xFFFF) == (NTSC_MONITOR_ID & ~0xFFFF))
                            ||  ( (ID & ~0xFFFF) == (PAL_MONITOR_ID & ~0xFFFF)))
                            &&  (!(GfxBase->ChipRevBits0 & GFXF_HR_AGNUS)))
                            {
                                info->NotAvailable |= DI_AVAIL_NOCHIPS;
                            }
                            info->NotAvailable &= ~DI_AVAIL_NOMONITOR;
#if (!((DO_NTSC_MONITOR) || (DO_PAL_MONITOR)))
			    D(kprintf("do dba\n");)
                            db_avail(ID, (struct DisplayInfoRecord *)handle, (struct RawDisplayInfo *)info);
			    D(kprintf("done dba\n");)
#endif
                            SetDisplayInfoData(handle,(UBYTE *)info,numbytes,DTAG_DISP,ID);
                        }
                    }
                }
            }
            /* Now alter the MonitorInfo. */
            if (numbytes = GetDisplayInfoData(NULL ,(UBYTE *)mntr,sizeof(*mntr),DTAG_MNTR,LastID))
            {
                mntr->Mspc = mspc;

                /* Make sure that the data duplicated in the monitorspec and
                 * MonitorInfo are the same. Ughhh! Lesson 1 in database
                 * design - avoid duplication of data.
                 */
                 mntr->TotalRows = mspc->total_rows;
                 mntr->TotalColorClocks = mspc->total_colorclocks;
                 mntr->MinRow = mspc->min_row;
                 mspc->ms_LegalView = mntr->ViewPositionRange;
                 SetDisplayInfoData(NULL,(UBYTE *)mntr,numbytes,DTAG_MNTR, LastID);
            }
        }
        else
        {
            GfxFree(mspc);
            mspc = NULL;
        }
    }

#if (DO_A2024_MONITOR)
    LoadA2024Driver();
#endif

    if (mspc)
    {
        return(TRUE);
    }

    return(FALSE);
}
