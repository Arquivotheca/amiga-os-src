#include <exec/types.h>
#include <exec/errors.h>
#include <graphics/rastport.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>

#include "printer.h"
#include "prtbase.h"
#include "prtgfx.h"
#include "internal.h"

#include "printer_iprotos.h"

#define DEBUG0		0	/* general debuging */
#define FREEDEBUG	0	/* debugging of freed memory */

int Backout(error, ior, PInfo)
int error; /* the reason why we are here */
struct IODRPReq *ior;  /* pointer to the input request */
struct PrtInfo *PInfo; /* all kinds of useful info */
{
	extern void FreeMem();

	struct RastPort *rp;
	struct BitMap *bm;
	UBYTE *mem;
	BYTE err;

#if DEBUG0
	extern struct PrinterData *PD;

	kprintf(
		"BO: enter, PrintBuf=%08lx, flags=%08lx\n", PD->pd_PrintBuf, PInfo->pi_flags);
#endif
	if ((err = error) == PDERR_TOOKCONTROL) { /* if special abort error */

#if DEBUG0
		kprintf("BO: err was TOOKCONTROL, switching to NOERR\n");
#endif

		err = PDERR_NOERR; /* pretend that there was no error */
	}
	/* fix up any vars the driver may have temporarely altered */
#if DEBUG0
	kprintf("BO: calling render to fix up vars\n");
#endif
	(*PInfo->pi_render)(0, 0, 0, 7);

	/* release any memory we may have allocated */
	if ((rp = PInfo->pi_temprp) != NULL) { /* if we allocated this */
		if ((bm = rp->BitMap) != NULL) { /* if we allocated this */
			if ((mem = bm->Planes[0]) != NULL) { /* if we allocated this */
#if FREEDEBUG
				kprintf("BO: freeing %ld bytes for Planes @ %lx\n",
					bm->BytesPerRow * bm->Depth, mem);
#endif
				FreeMem(mem, bm->BytesPerRow * bm->Depth);
			}
#if FREEDEBUG
			kprintf("BO: freeing %ld bytes for bm @ %lx\n",
				sizeof(struct BitMap) + bm->Depth * sizeof(PLANEPTR), bm);
#endif
			FreeMem(bm, sizeof(struct BitMap) + bm->Depth * sizeof(PLANEPTR));
		}
#if FREEDEBUG
		kprintf("BO: freeing %ld bytes for rp @ %lx\n",
			sizeof(struct RastPort), rp);
#endif
		FreeMem(rp, sizeof(struct RastPort));
	}
	if (PInfo->pi_ColorMap != NULL) { /* if we allocated this */
#if FREEDEBUG
		kprintf("BO: freeing %ld bytes for ColorMap @ %lx\n",
			PInfo->pi_ColorMapSize, PInfo->pi_ColorMap);
#endif
		FreeMem(PInfo->pi_ColorMap, PInfo->pi_ColorMapSize); /* release the mem */
	}
	if (PInfo->pi_RowBuf != NULL) { /* if we allocated this */
#if FREEDEBUG
		kprintf("BO: freeing %ld bytes for RowBuf @ %lx\n",
			PInfo->pi_RowBufSize, PInfo->pi_RowBuf);
#endif
		FreeMem(PInfo->pi_RowBuf, PInfo->pi_RowBufSize); /* release the mem */
	}

	if (PInfo->pi_TopBuf != NULL) { /* if we allocated this */
#if FREEDEBUG
		kprintf("BO: freeing %ld bytes for TopBuf @ %lx\n",
			PInfo->pi_RowBufSize, PInfo->pi_TopBuf);
#endif
		FreeMem(PInfo->pi_TopBuf, PInfo->pi_RowBufSize); /* release the mem */
	}
	if (PInfo->pi_BotBuf != NULL) { /* if we allocated this */
#if FREEDEBUG
		kprintf("BO: freeing %ld bytes for BotBuf @ %lx\n",
			PInfo->pi_RowBufSize, PInfo->pi_BotBuf);
#endif
		FreeMem(PInfo->pi_BotBuf, PInfo->pi_RowBufSize); /* release the mem */
	}

	if (PInfo->pi_Dest1Int != NULL) { /* if we allocated this */
#if FREEDEBUG
		kprintf("BO: freeing %ld bytes for Dest1Int @ %lx\n",
			PInfo->pi_Dest1IntSize, PInfo->pi_Dest1Int);
#endif
		FreeMem(PInfo->pi_Dest1Int, PInfo->pi_Dest1IntSize); /* release mem */
	}
	if (PInfo->pi_Dest2Int != NULL) { /* if we allocated this */
#if FREEDEBUG
		kprintf("BO: freeing %ld bytes for Dest2Int @ %lx\n",
			PInfo->pi_Dest2IntSize, PInfo->pi_Dest2Int);
#endif
		FreeMem(PInfo->pi_Dest2Int, PInfo->pi_Dest2IntSize); /* release mem */
	}

	if (PInfo->pi_ColorInt != NULL) { /* if we allocated this */
#if FREEDEBUG
		kprintf("BO: freeing %ld bytes for ColorInt @ %lx\n",
			PInfo->pi_ColorIntSize, PInfo->pi_ColorInt);
#endif
		FreeMem(PInfo->pi_ColorInt, PInfo->pi_ColorIntSize); /* release the mem */
	}
	if (PInfo->pi_HamInt != NULL) { /* if we allocated this */
#if FREEDEBUG
		kprintf("BO: freeing %ld bytes for HamInt @ %lx\n",
			PInfo->pi_HamIntSize, PInfo->pi_HamInt);
#endif
		FreeMem(PInfo->pi_HamInt, PInfo->pi_HamIntSize); /* release the mem */
	}
	if (PInfo->pi_HamBuf != NULL) { /* if we allocated this */
#if FREEDEBUG
		kprintf("BO: freeing %ld bytes for HamBuf @ %lx\n",
			PInfo->pi_HamBufSize, PInfo->pi_HamBuf);
#endif
		FreeMem(PInfo->pi_HamBuf, PInfo->pi_HamBufSize); /* release the mem */
	}
	if (PInfo->pi_ScaleX != NULL) { /* if we allocated this */
#if FREEDEBUG
		kprintf("BO: freeing %ld bytes for ScaleX @ %lx\n",
			PInfo->pi_ScaleXSize, PInfo->pi_ScaleX);
#endif
		FreeMem(PInfo->pi_ScaleX, PInfo->pi_ScaleXSize); /* release the mem */
	}
	if (PInfo->pi_ScaleXAlt != NULL) { /* if we allocated this */
#if FREEDEBUG
		kprintf("BO: freeing %ld bytes for ScaleXAlt @ %lx\n",
			PInfo->pi_ScaleXAltSize, PInfo->pi_ScaleXAlt);
#endif
		FreeMem(PInfo->pi_ScaleXAlt, PInfo->pi_ScaleXAltSize);
	}
	if (PInfo->pi_flags & PRT_RENDER0) { /* if allocated this */
#if DEBUG0
		kprintf("BO: freeing up render mem\n");
#endif
		(*PInfo->pi_render)(err, PInfo->pi_special, 0, 4); /* release the mem */
	}
	ior->io_Error = err; /* set error code for calling application */
#if DEBUG0
	kprintf("BO: error=%ld, err=%ld, ior=%lx, PInfo=%lx  ",
		error, err, ior, PInfo);
	if (err == PDERR_NOERR) {
		kprintf("BO: PDERR_NOERR  ");
	}
	else if (err == PDERR_CANCEL) {
		kprintf("BO: PDERR_CANCEL  ");
	}
	else if (err == IOERR_ABORTED) {
		kprintf("BO: IOERR_ABORTED  ");
	}
	else if (err == PDERR_BUFFERMEMORY) {
		kprintf("BO: PDERR_BUFFERMEMORY");
	}
	else if (err == PDERR_INTERNALMEMORY) {
		kprintf("BO: PDERR_INTERNALMEMORY");
	}
	else if (err == PDERR_BADDIMENSION) {
		kprintf("BO: PDERR_BADDIMENSION");
	}
	else if (err == PDERR_NOTGRAPHICS) {
		kprintf("BO: PDERR_NOTGRAPHICS");
	}
	kprintf(" BO: exit\n");
#endif DEBUG0
	return((int)err);
}
