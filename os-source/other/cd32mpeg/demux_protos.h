#ifndef	DEMUX_PROTOS_H
#define DEMUX_PROTOS_H 1

/*
** Routines defined in demux.asm
*/

#ifndef	MPEG_DEV_H
#include "mpeg_device.h"
#endif

extern ASM SystemDemux(REG(a2) struct IOMPEGReq *iomr,
		       REG(a4) struct MPEGUnit *mpegUnit,
		       REG(a6) struct MPEGDevice *mpegDevice);

#endif	/* DEMUX_PROTOS_H */
