#ifndef STREAMIO_H
#define STREAMIO_H


/*****************************************************************************/


#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifndef EXEC_PORTS_H
#include <exec/ports.h>
#endif

#ifndef DOS_DOS_H
#include <dos/dos.h>
#endif


/*****************************************************************************/


/* This structure is public only by necessity, don't muck with it yourself, or
 * you're looking for trouble
 */

#define CD_SECTOR_SIZE 2324

#define DPFLAGF_ValidSCR	1
#define DPFLAGF_ValidPTS	2

struct DataPacket
{
	struct Message	dp_Msg;
	UWORD		dp_Pad;
	ULONG		dp_DataSize;
	ULONG		dp_Flags;
	UWORD		dp_SCRHi;
	UWORD		dp_SCRMid;
	UWORD		dp_SCRLo;
	UWORD		dp_PTSHi;
	UWORD		dp_PTSMid;
	UWORD		dp_PTSLo;
	UWORD	       *dp_DataStart;
	ULONG		dp_DataLength;
	UBYTE		dp_IsOverFlow;
	UBYTE		dp_OverFlow;
	UBYTE		dp_Packet[CD_SECTOR_SIZE];
};

struct StreamFile
{
    BPTR                  sf_File;
    ULONG		  sf_BlockSize;
    ULONG		  sf_Err;
    struct Process	 *sf_Proc;
    struct MsgPort	 *sf_VideoPort;
    struct MsgPort	 *sf_AudioPort;
    struct MsgPort	 *sf_ReplyPort;
    struct DataPacket	 *sf_VideoBlock;
    struct DataPacket	 *sf_AudioBlock;
    UWORD		  sf_RingCnt;
    UWORD		  sf_RingMax;
    UWORD		  sf_VideoOffset;
    UWORD		  sf_AudioOffset;
    UWORD		 *sf_VideoData;
    UWORD		 *sf_AudioData;
    UWORD		  sf_VideoSize;
    UWORD		  sf_AudioSize;
    UWORD		  sf_A;
    UWORD		  sf_V;
    UBYTE		  sf_IsOverFlow;
    UBYTE		  sf_OverFlow;

};


/*****************************************************************************/


#define MODE_READ   0  /* read an existing file                             */
#define MODE_WRITE  1  /* create a new file, delete existing file if needed */
#define MODE_APPEND 2  /* append to end of existing file, or create new     */

#define MODE_START   -1   /* relative to start of file         */
#define MODE_CURRENT  0   /* relative to current file position */
#define MODE_END      1   /* relative to end of file	     */


/*****************************************************************************/

struct StreamFile *OpenStream(const STRPTR fileName, ULONG blocksize, ULONG *sigs);
LONG ReadVideoWord(struct StreamFile *file);
LONG BurstCL450Fifo(struct StreamFile *file, UWORD *fifo);
LONG BurstL64111Fifo(struct StreamFile *file, UWORD *fifo);
LONG ReadAudioWord(struct StreamFile *file);
VOID CloseStream(struct StreamFile *file);

/*****************************************************************************/

#define	iso_11172_end_code		0x000001B9
#define pack_start_code			0x000001BA
#define system_header_start_code	0x000001BB
#define audio_packet_start_code_prefix	0x000001C0
/* #define audio_packet_start_code_mask	0x000001E0 */
#define audio_packet_start_code_mask	0xFFFFFFE0
#define video_packet_start_code_prefix	0x000001E0
/* #define video_packet_start_code_mask	0x000001F0 */
#define video_packet_start_code_mask	0xFFFFFFF0
#define audio_packet_stream_id_mask	0x0000001F
#define video_packet_stream_id_mask	0x0000000F

struct PackHeader
{
	ULONG	pk_pack_start_code;

	UBYTE	pack_0010:4,
		scr_hi:3,
		marker_bit1:1;
	UWORD	scr_mid:15,
		marker_bit2:1;
	UWORD	scr_lo:15,
		marker_bit3:1;
/*	WORD	junk1; */
	UBYTE	junk2;
/*	ULONG	marker_bit4:1,
		mux_rate:22,
		marker_bit5:1; */
};

struct SystemHeader
{
	ULONG	system_start_code;
	UWORD	header_length;
	ULONG	marker_bit1:1,
		rate_bound:22,
		marker_bit2:1;
	UBYTE	audio_bound:6,
		fixed_flag:1,
		CSPS_flag:1;
	UBYTE	audio_lock_flag:1,
		video_lock_flag:1,
		marker_bit3:1,
		video_bound:5;
	UBYTE	reserved_byte;
};

struct SysHStreamId
{
	UBYTE	stream_id;
	UWORD	marker_bits:2,
		STD_buffer_bound_scale:1,
		STD_buffer_size_bound:13;
};

struct BigSystemHeader
{
	struct SystemHeader sysHead;

	struct	{
		UBYTE	stream_id;
		UWORD	marker_bits:2,
			STD_buffer_bound_scale:1,
			STD_buffer_size_bound:13;
		} stream_ids[100];
};

struct PacketHeader
{
	ULONG	packet_start_code;
	UWORD	packet_length;
};

struct PacketBufferInfo
{
	UWORD	pbi_01:2,
		STD_buffer_scale:1,
		STD_buffer_size:13;
};

struct PacketPTS
{
	UBYTE	pts_0010:4,
		pts_hi:3,
		pts_marker_bit1:1;
	UWORD	pts_mid:15,
		pts_marker_bit2:1;
	UWORD	pts_low:15,
		pts_marker_bit3;
};

struct PacketPTS_DTS
{
	UBYTE	pts_0011:4,
		pts_hi:3,
		pts_marker_bit1:1;
	UWORD	pts_mid:15,
		pts_marker_bit2:1;
	UWORD	pts_low:15,
		pts_marker_bit3;
	UBYTE	dts_0001:4,
		dts_hi:3,
		dts_marker_bit1:1;
	UWORD	dts_mid:15,
		dts_marker_bit2:1;
	UWORD	dts_low:15,
		dts_marker_bit3:1;
};

/*****************************************************************************/
#endif /* STREAMIO_H */
