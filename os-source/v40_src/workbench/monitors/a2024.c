/******************************************************************************
*
*	$Id: a2024.c,v 39.12 93/04/27 16:30:47 spence Exp $
*
******************************************************************************/

	#include <exec/types.h>
	#include <exec/interrupts.h>
	#include <exec/memory.h>
	#include <hardware/intbits.h>
	#include <graphics/displayinfo.h>
	#include <graphics/gfxbase.h>
	#include <graphics/monitor.h>
	#include <internal/vp_internal.h>
	#include "a2024vb.h"

	#include <clib/exec_protos.h>
	#include <pragmas/exec_pragmas.h>
	#include <clib/graphics_protos.h>
	#include <pragmas/graphics_pragmas.h>

	#include <stdio.h>

extern struct DosBase *DOSBase;
extern struct GfxBase *GfxBase;
extern struct ExecBase *SysBase;
extern char FirstAddress;
extern char LastAddress;

APTR MakePermanent();
BOOL LoadA2024Driver();

#pragma libcall GfxBase SetDisplayInfoData 2ee 2109805
ULONG SetDisplayInfoData( DisplayInfoHandle handle, UBYTE * buf, ULONG size, ULONG tagID, ULONG displayID );

/*#define _DEBUG*/

#ifdef _DEBUG
#define D(x) {x}
#else
#define D(x)
#endif

APTR MakePermanent()
{
    APTR perm;
    ULONG size;
    /* Do some funky stuff,
     * like copy the driver to another part of RAM.
     */

    size = (&LastAddress - &FirstAddress);

    if (perm = AllocVec(size, 0))
    {
	CopyMem(&FirstAddress, perm, size);
    }
    D(kprintf("Permanent = 0x%lx\n", perm);)

    return(perm);
}

extern void * __stdargs A2024MoveSprite();
extern void * __stdargs A2024ChangeSprite();
extern void * __stdargs A2024ScrollVP();
extern void * __stdargs A2024PokeColours();
extern void * __stdargs A2024BuildColours();
extern void * __stdargs A2024BuildNmlCopList();
extern void * __stdargs A2024vbasm();
extern void * __stdargs A2024MrgCop();
extern void * __stdargs A2024LoadView();
extern void * __stdargs A2024KillView();
extern APTR GfxBaseMrgCop;
extern APTR GfxBaseLoadView;
extern APTR VBlankStruct;
extern UBYTE *A2024vbname;
extern void *hsprites[8];

#define MAXSPHEIGHT 50

#define REALADR(x) ((void *)(perm-&FirstAddress+((char *) x)))
#define MAKEITS 11

BOOL LoadA2024Driver()
{
    /* find the VecTable and MakeItTable for the A2024, and fill it up */
    struct VecInfo vinfo2024, vinfodef;
    struct VecTable *vt, *vtdef;
    ULONG *BuildData, *BuildDatadef;
    char *perm;
    BOOL result = FALSE;

    D(kprintf("LoadA2024Driver()\n");)
    if ((vt = AllocMem(sizeof(struct VecInfo), MEMF_CLEAR)) == NULL)
    {
        return(FALSE);
    }
    if ((BuildData = AllocMem((MAKEITS << 2), MEMF_CLEAR)) == NULL)
    {
    	FreeMem(vt, sizeof(struct VecInfo));
	return(FALSE);
    }

    if (GetDisplayInfoData(NULL, (APTR)&vinfo2024, sizeof(struct VecInfo), DTAG_VEC, A2024TENHERTZ_KEY))
    {
	D(kprintf("Got vinfo2024\n");)
	/* Can't use the Default monitor in case of promotion. NTSC and PAL
	 * both use the same vec table.
	 */
	if (GetDisplayInfoData(NULL, (APTR)&vinfodef, sizeof(struct VecInfo), DTAG_VEC, (NTSC_MONITOR_ID | LORES_KEY)))
	{
		int i;
		for(i=0;i<8;i++)
			hsprites[i]=AllocMem(4*(MAXSPHEIGHT+2),MEMF_CHIP|MEMF_CLEAR);

		D(kprintf("Got vinfodef\n");)
		if (perm = MakePermanent())
		{
			/* get the VecTable */
			vinfo2024.Vec = vt;
			vtdef = vinfodef.Vec;
			D(kprintf("vt = 0x%lx, vtdef = 0x%lx\n", vt, vtdef);)

			/* and load it up */
			vt->MoveSprite = REALADR(&A2024MoveSprite);
			vt->ChangeSprite = REALADR(&A2024ChangeSprite);
			vt->ScrollVP = REALADR(&A2024ScrollVP);
			vt->PokeColors = REALADR(&A2024PokeColours);

			/* now load up the BuildVP table */
			vt->BuildVP = (APTR)BuildData;
			BuildDatadef = (ULONG *)(((struct VecTable *)vinfodef.Vec)->BuildVP);
			D(kprintf("BuildData = 0x%lx, BuildDatadef = 0x%lx\n", BuildData, BuildDatadef);)

			/* take the following from the ROM (ie mode 0x0):
			 * InitMVP()
			 * CalcDispWindow()		
			 * CalcDataFetch()
			 * CalcScroll()
			 * CalcAA/ECSFudge()
			 * CalcModulos()
			 * MakeAA/ECSBplcon()
			 * CleanUpMVP()
			 *
			 * The BuildColors() and BuildCopList() are all from disk.
			 */

			*BuildData++ = *BuildDatadef++;	/* InitMVP */
			*BuildData++ = *BuildDatadef++;	/* CalcDispWindow */
			*BuildData++ = *BuildDatadef++;	/* CalcDataFetch */
			*BuildData++ = *BuildDatadef++;	/* CalcScroll */
			*BuildData++ = *BuildDatadef++;	/* Fudge */
			*BuildData++ = *BuildDatadef++;	/* Modulos */
			*BuildData++ = *BuildDatadef++;	/* Bplcon */
			BuildDatadef++;		/* skip BuildColors */
			*BuildData++ = perm;
			*BuildData++ = *BuildDatadef++;	/* BuildColors */
			*BuildData++ = *BuildDatadef++;	/* BuildCopList */
			*BuildData++ = *BuildDatadef;	/* CleanUpMVP */
			*BuildData = NULL;		/* terminator */
			result = TRUE;
		}
	}
	SetDisplayInfoData(NULL, (APTR)&vinfo2024, sizeof(struct VecInfo), DTAG_VEC, A2024TENHERTZ_KEY);
	if (GetDisplayInfoData(NULL, (APTR)&vinfo2024, sizeof(struct VecInfo), DTAG_VEC, A2024FIFTEENHERTZ_KEY))
	{
		vinfo2024.Vec = vt;
		SetDisplayInfoData(NULL, (APTR)&vinfo2024, sizeof(struct VecInfo), DTAG_VEC, A2024FIFTEENHERTZ_KEY);
	}
    }

    if (result)
    {

	/* Let's allocate the SyncRaster,
	 * add an interrupt server,
	 * and vector LoadView().
	 */

	struct Interrupt *vbi = NULL;
	struct A2024VB *vbdata = NULL;
	APTR loadview, mrgcop;
	ULONG *oldloadview, *oldmrgcop;
	ULONG *vbicache;
	int i;
	BOOL err = TRUE;

	if (GfxBase->a2024_sync_raster = (void *) AllocVec(128,MEMF_CHIP|MEMF_CLEAR))
	{
		for (i = 0;i < (128/4) ; i++)
		{
			GfxBase->a2024_sync_raster[i] = -1;
		}
		GfxBase->a2024_sync_raster[0] = 0x3ffffff; /* bart -- 01.31.89 */

		if (vbi = (struct Interrupt *)AllocVec(sizeof(struct Interrupt), MEMF_PUBLIC | MEMF_CLEAR))
		{
			if (vbdata = (struct A2024VB *)AllocVec(sizeof(struct A2024VB), MEMF_PUBLIC | MEMF_CLEAR))
			{
				struct MonitorSpec *mspec;

				GfxBase->hedley_flags = 0;
				vbdata->gbhc = &GfxBase->hedley_count;
				vbi->is_Data = vbdata;
				vbi->is_Code = REALADR(&A2024vbasm);
				vbi->is_Node.ln_Type = NT_INTERRUPT;
				vbi->is_Node.ln_Pri = 11;	/* one more than gfxlib */
				vbi->is_Node.ln_Name = REALADR(&A2024vbname);

				Forbid();
				if (mspec = OpenMonitor(NULL, 0x0))
				{
					oldloadview = REALADR(&GfxBaseLoadView);
					oldmrgcop = REALADR(&GfxBaseMrgCop);
					*oldloadview = mspec->ms_LoadView;
					*oldmrgcop = mspec->ms_MrgCop;
					CloseMonitor(mspec);
					if (mspec = OpenMonitor("a2024.monitor", 0x41000))
					{
						mspec->ms_MrgCop = REALADR(&A2024MrgCop);
						mspec->ms_LoadView = REALADR(&A2024LoadView);
						mspec->ms_KillView = REALADR(&A2024KillView);
						CloseMonitor(mspec);
						GfxBase->hedley[7] = vbdata;
						vbicache = REALADR(&VBlankStruct);
						*vbicache = vbi;
						err = FALSE;
					}
				}
				Permit();
				
			}
		}
	}
	if (err)
	{
		if (vbi)
			FreeVec(vbi);
		if (GfxBase->a2024_sync_raster)
			FreeVec(GfxBase->a2024_sync_raster);
		if (perm)
			FreeVec(perm);
		result = FALSE;
	}
		
    }
    return(result);
}
