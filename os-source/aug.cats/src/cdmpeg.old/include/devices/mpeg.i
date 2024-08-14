	IFND	MPEG_I
MPEG_I	SET	1

**
**	$Id: mpeg.i,v 40.3 93/10/23 01:12:48 kcd Exp Locker: kcd $
**
**	CD32 MPEG Device Assembly include file.
**
**	(C) Copyright 1993 Commodore-Amiga, Inc.
**	    All Rights Reserved
**

	IFND	EXEC_IO_I
	INCLUDE	"exec/io.i"
	ENDC

**
** IOMpegReq structure
**

    STRUCTURE	IOMPEGReq,IOSTD_SIZE

	; MPEG Specific Stuff
	UWORD	iomr_MPEGError		; Extended Error Information
	UBYTE	iomr_Version		; Must be set to 0 for this spec
	UBYTE	iomr_StreamType		; Type of stream to play
	ULONG	iomr_MPEGFlags		; Flags.  See below
	ULONG	iomr_Arg1
	ULONG	iomr_Arg2

	UWORD	iomr_PTSHigh            ; Bits 32-30 of this data's PTS
	UWORD	iomr_PTSMid		; Bits 29-15 of this data's PTS
	UWORD	iomr_PTSLow		; Bits 14-9  of this data's PTS

	; Private Device Information

	UWORD	iomr_Private0
	ULONG	iomr_Private1
	ULONG	iomr_Private2

	UWORD	iomr_Private3
	UWORD	iomr_Private4
	UWORD	iomr_Private5

	LABEL	IOMPEGReq_SIZE

**
** Handy equates
**
iomr_SlowSpeed  	EQU     iomr_Arg1
iomr_PauseMode          EQU	iomr_Arg1
iomr_DisplayType        EQU	iomr_Arg1
iomr_SearchSpeed        EQU	iomr_Arg1
iomr_SectorSize         EQU	iomr_Arg1
iomr_StreamStart        EQU	iomr_Arg2

**
** Defined Stream Types
**
MPEGSTREAM_VIDEO	EQU	1	; Raw Video bitstream
MPEGSTREAM_AUDIO	EQU	2	; Raw Audio bitstream
MPEGSTREAM_SYSTEM	EQU	3	; ISO 1172 System Stream

**
** MPEG Error Values
**
MPEGERR_BAD_STATE	EQU	1	; Command is illegal for the current device state
MPEGERR_BAD_PARAMETER	EQU	2	; Some parameter was illegal
MPEGERR_CMD_FAILED	EQU	3	; Catch-All
MPEGERR_CD_ERROR	EQU	4	; Error while attempting cd.device I/O

**
** Extended error values.
**
MPEGEXTERR_STREAM_MISMATCH	EQU	1	/* Stream type not appropriate */
MPEGEXTERR_MICROCODE_FAILURE	EQU	2	/* MicroCode failed to respond */
MPEGEXTERR_BAD_STREAM_TYPE	EQU	3	/* Command is incompatible with current stream type */

**
** Defined MPEG Flags
**
	BITDEF	MPEG,VALID_PTS,31	; This piece of data has a valid PTS
	BITDEF	MPEG,ONESHOT,2		; One-Shot scan
**
** MPEG Device Commands
**

	DEVINIT

	; Low level commands

	DEVCMD	MPEGCMD_PLAY
	DEVCMD	MPEGCMD_PAUSE
	DEVCMD	MPEGCMD_SLOWMOTION
	DEVCMD	MPEGCMD_SINGLESTEP
	DEVCMD	MPEGCMD_SCAN
	DEVCMD	MPEGCMD_RECORD
	DEVCMD	MPEGCMD_GETDEVINFO
	DEVCMD	MPEGCMD_SETWINDOW
	DEVCMD	MPEGCMD_SETBORDER
	DEVCMD	MPEGCMD_GETVIDEOPARAMS
	DEVCMD	MPEGCMD_SETVIDEOPARAMS
	DEVCMD	MPEGCMD_SETAUDIOPARAMS

	; Higher level commands

	DEVCMD	MPEGCMD_PLAYLSN
	DEVCMD	MPEGCMD_SEEKLSN
	DEVCMD	MPEGCMD_READFRAMEYUV

	DEVCMD	MPEGCMD_MPEG_END

**
** This structure is returned form a MPEGCMD_GETDEVINFO command. Use this
** to determine what the device driver is capable of doing.  Not all devices
** will support all commands/features.
**
	STRUCT	MPEGDevInfo,0
	UWORD	mdi_Version
	UWORD	mdi_Flags
	ULONG	mdi_BoardCapabilities
	STRUCT	mdi_BoardDesc,256
	LABEL	mdi_SIZE

**
** This structure is used with the MPEGCMD_SETWINDOW command.
**
	STRUCT	MPEGWindowParams,0
	UWORD	mwp_XOffset		; Hi-Res Pixels
	UWORD	mwp_YOffset		; Non-interlaced scanlines
	UWORD	mwp_Width		; Hi-Res Pixels
	UWORD	mwp_Height		; Non-interlaced scanlines
	LABEL	mwp_SIZE

**
** This structure is used with the MPEGCMD_SETBORDER command.
**
	STRUCT	MPEGBorderParams,0
	UWORD	mbp_BorderLeft
	UWORD	mbp_BorderTop
	UBYTE	mbp_BorderRed
	UBYTE	mbp_BorderGreen
	UBYTE	mbp_BorderBlue
	LABEL	mbp_SIZE

**
** MPEGFrameStore is used by the MPEGCMD_READFRAMEYUV command.
** Color components with NULL pointers will not be written to.
**

	STRUCT	MPEGFrameStore,0
	UWORD	mfs_Width
	UWORD	mfs_Height
	APTR	mfs_Luma
	APTR	mfs_Cr
	APTR	mfs_Cb
	LABEL	mfs_SIZE

**
** NB: Not all devices support the full functionality that these
**     structure provide for.  Please take this into account.
**

	STRUCT	MPEGVideoParamsSet,0
	UWORD	mvp_Fade		;Fade level. 0 = no MPEG video,
					;65535 = full saturation.
	UWORD	mvp_DisplayType		;0 = No Change, 3 = PAL (50Hz),
					;4 = NTSC (60Hz)
	LABEL	mvps_SIZE


	STRUCT	MPEGVideoParamsGet,0
	UWORD	mvp_PictureWidth	;Width in lo-res pixels
	UWORD	mvp_PictureHeight	;Height in non-interlaced scanlines
	UWORD	mvp_PictureRate		;Pictures per second code
	LABEL	mvpg_SIZE

	STRUCT	MPEGAudioParams,0
	UWORD	map_VolumeLeft		;Left Channel Volume (0=Mute, 65535 = Loudest)
	UWORD	map_VolumeRight		;Right Channel Volume
	UWORD	map_StreamID		;MPEG Audio stream ID. ~0 for all streams
	LABEL	map_SIZE

**
** Board Capabilities
**
	BITDEF	MPEGC,PLAYRAWVIDEO,16	;Can play a raw video stream
	BITDEF	MPEGC,PLAYRAWAUDIO,17	;Can play a raw audio stream
	BITDEF	MPEGC,PLAYSYSTEM,18	;Can play an ISO-1172 system stream
	BITDEF	MPEGC,WINDOWVIDEO,19	;Can do window subpositioning
	BITDEF	MPEGC,SCALEVIDEO,20	;Can scale video
	BITDEF	MPEGC,STEPPLAY,21	;Can single-step
	BITDEF	MPEGC,SCANPLAY,22	;Can scan
	BITDEF	MPEGC,SLOWPLAY,23	;Can do slow-motion
	BITDEF	MPEGC,READFRAME,24	;Can read digital frame data

	ENDC	; MPEG_I
