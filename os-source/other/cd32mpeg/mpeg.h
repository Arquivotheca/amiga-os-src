#ifndef	DEVICES_MPEG_H
#define DEVICES_MPEG_H

/*
**	$Id: mpeg.h,v 40.4 94/01/26 11:56:17 kcd Exp $
**
**	CD32 MPEG Device Driver C Header File
**
**	(C) Copyright 1993 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/

#ifndef	EXEC_IO_H
#include <exec/io.h>
#endif /* EXEC_IO_H */

struct IOMPEGReq
{
	/* Standard Stuff */

	struct IOStdReq	iomr_Req;

	/* Global MPEG fields */

	UWORD		iomr_MPEGError;		/* Extended Error Information */
	UBYTE		iomr_Version;		/* Must be set to 0 for this spec */
	UBYTE		iomr_StreamType;
	ULONG		iomr_MPEGFlags;
	LONG		iomr_Arg1;
	ULONG		iomr_Arg2;

	UWORD		iomr_PTSHigh;		/* Bits 32-30 of this data's PTS */
	UWORD		iomr_PTSMid;		/* Bits 29-15 of this data's PTS */
	UWORD		iomr_PTSLow;		/* Bits 14-9  of this data's PTS */

	/* Private Device Information */

	UWORD		iomr_Private0;
	ULONG		iomr_Private1;
	ULONG		iomr_Private2;

	UWORD		iomr_Private3;
	UWORD		iomr_Private4;
	UWORD		iomr_Private5;

};

/*
** Handy defines
**
*/

#define	iomr_SlowSpeed		iomr_Arg1
#define iomr_PauseMode		iomr_Arg1
#define iomr_DisplayType	iomr_Arg1
#define iomr_SearchSpeed	iomr_Arg1
#define iomr_SectorSize		iomr_Arg1
#define iomr_StreamStart	iomr_Arg2

/*
** Defined Stream Types
*/
#define	MPEGSTREAM_VIDEO	1	/* Raw Video bitstream */
#define MPEGSTREAM_AUDIO	2	/* Raw Audio bitstream */
#define MPEGSTREAM_SYSTEM	3	/* ISO 1172 System Stream */

/*
** MPEG Error Values
*/
#define MPEGERR_BAD_STATE	1	/* Command is illegal for the current device state */
#define MPEGERR_BAD_PARAMETER	2	/* Some parameter was illegal */
#define MPEGERR_CMD_FAILED	3	/* Command Failed */
#define MPEGERR_CDERROR		4	/* Problem with cd.device I/O */

/*
** Extended error values.
*/
#define MPEGEXTERR_STREAM_MISMATCH	1	/* Stream type not appropriate */
#define MPEGEXTERR_MICROCODE_FAILURE	2	/* MicroCode failed to respond */
#define MPEGEXTERR_BAD_STREAM_TYPE	3	/* Stream/Command incompatible */

/*
** Flags for iomr_MPEGFlags
*/

#define MPEGB_VALID_PTS			31	/* This bit of data has a valid PTS */
#define MPEGB_ONESHOT			2	/* One-Shot scan */

#define MPEGF_VALID_PTS			(1L << MPEGB_VALID_PTS)
#define MPEGF_ONESHOT			(1L << MPEGB_ONESHOT)

/*
** MPEG Device Commands
*/
#define MPEGCMD_PLAY		(CMD_NONSTD + 0)
#define MPEGCMD_PAUSE		(CMD_NONSTD + 1)
#define MPEGCMD_SLOWMOTION	(CMD_NONSTD + 2)
#define MPEGCMD_SINGLESTEP	(CMD_NONSTD + 3)
#define MPEGCMD_SEARCH		(CMD_NONSTD + 4)
#define MPEGCMD_RECORD		(CMD_NONSTD + 5)
#define MPEGCMD_GETDEVINFO	(CMD_NONSTD + 6)
#define MPEGCMD_SETWINDOW	(CMD_NONSTD + 7)
#define MPEGCMD_SETBORDER	(CMD_NONSTD + 8)
#define MPEGCMD_GETVIDEOPARAMS	(CMD_NONSTD + 9)
#define MPEGCMD_SETVIDEOPARAMS	(CMD_NONSTD + 10)
#define MPEGCMD_SETAUDIOPARAMS	(CMD_NONSTD + 11)
#define MPEGCMD_PLAYLSN		(CMD_NONSTD + 12)
#define MPEGCMD_SEEKLSN		(CMD_NONSTD + 13)
#define MPEGCMD_READFRAMEYUV	(CMD_NONSTD + 14)

#define MPEGCMD_END		(CMD_NONSTD + 15)

/*
** This structure is returned form a MPEGCMD_GETDEVINFO command. Use this
** to determine what the device driver is capable of doing.  Not all devices
** will support all commands/features.
*/

struct MPEGDevInfo
{
	UWORD	mdi_Version;
	UWORD	mdi_Flags;

	ULONG	mdi_BoardCapabilities;
	UBYTE	mdi_BoardDesc[256];
};

struct MPEGVideoParams
{
	UWORD	mvp_Version;		/* Version of this structure */
	UWORD	mvp_Flags;		/* For future use */
	UWORD	mvp_PictureWidth;	/* Width in SIF (Amiga Lo-res) pixels (read-only) */
	UWORD	mvp_PictureHeight;	/* Height in non-interlaced scanlines (read-only) */

};

struct MPEGWindowParams
{
	UWORD	mwp_XOffset;		/* Hi-Res Pixels */
	UWORD	mwp_YOffset;		/* Non-interlaced scanlines */
	UWORD	mwp_Width;		/* Hi-Res Pixels */
	UWORD	mwp_Height;		/* Non-interlaced scanlines */
};

struct MPEGBorderParams
{
	UWORD	mbp_BorderLeft;		/* Hi-Res Pixels */
	UWORD	mbp_BorderTop;		/* Non-interlaced scanlines */
	UBYTE	mbp_BorderRed;		/* Border 8-bit red value */
	UBYTE	mbp_BorderGreen;	/* Border 8-bit green value */
	UBYTE   mbp_BorderBlue;		/* Border 8-bit blue value */
};

/* The structure is used by the MPEGCMD_READFRAMEYUV command. */
/* Color components with NULL pointers will not be written to. */

struct MPEGFrameStore
{
	UWORD	mfs_Width;		/* Picture Width */
	UWORD	mfs_Height;		/* Picture Height */
	UBYTE	*mfs_Luma;		/* Must be Width*Height bytes */
	UBYTE	*mfs_Cr;		/* Must be Width*Height/4 bytes */
	UBYTE	*mfs_Cb;		/* Must be Width*Height/4 bytes */
};

/*
** NB: Not all devices support the full functionality that these
**     structures provide for.  Please take this into account.
*/

struct MPEGVideoParamsSet
{
	UWORD	mvp_Fade;		/* Fade level.  0 = no MPEG video at all, 6535 = full saturation */
	UWORD	mvp_DisplayType;	/* 3 = PAL (50Hz), 4 = NTSC (60Hz) */
};

struct MPEGVideoParamsGet
{
	UWORD	mvp_PictureWidth;	/* Width in lo-res pixels */
	UWORD	mvp_PictureHeight;	/* Width in non-interlaced scanlines */
	UWORD	mvp_PictureRate;	/* Pictures per second */
};

struct MPEGAudioParams
{
	UWORD	map_VolumeLeft;		/* Left Channel Volume (0=Mute, 65535 = loudest) */
	UWORD	map_VolumeRight;	/* Right Channel Volume */
	UWORD	map_StreamID;		/* MPEG Audio stream ID. ~0 for all streams */
};

/* Board Capabilities for playback */

#define MPEGCB_PLAYRAWVIDEO	16
#define MPEGCB_PLAYRAWAUDIO	17
#define MPEGCB_PLAYSYSTEM	18
#define MPEGCB_WINDOWVIDEO	19
#define MPEGCB_SCALEVIDEO	20
#define MPEGCB_STEPPLAY		21
#define MPEGCB_SCANPLAY		22

/* Flag Versions */

#define MPEGCF_PLAYRAWVIDEO	(1L << MPEGCB_PLAYRAWVIDEO)
#define MPEGCF_PLAYRAWAUDIO	(1L << MPEGCB_PLAYRAWAUDIO)
#define MPEGCF_PLAYSYSTEM	(1L << MPEGCB_PLAYSYSTEM)
#define MPEGCF_WINDOWVIDEO	(1L << MPEGCB_WINDOWVIDEO)
#define MPEGCF_SCALEVIDEO	(1L << MPEGCB_SCALEVIDEO)
#define MPEGCF_STEPPLAY		(1L << MPEGCB_STEPPLAY)
#define MPEGCF_SCANPLAY		(1L << MPEGCB_SCANPLAY)

#endif /* DEVICES_MPEG_H */
