/* L64111_Chip.h - Definitions for use with the L64111 Audio Decoder */

#include <exec/types.h>


struct L64111Regs
{
	volatile UWORD	Data;
	volatile UWORD	Control_1;
	volatile UWORD	Control_2;
	volatile UWORD	Control_3;
	volatile UWORD	Status_Int_1;
	volatile UWORD	Status_Int_2;
	volatile UWORD	Timer_CountDown;
	volatile UWORD	Timer_OffsetHi;
	volatile UWORD	Timer_OffsetLo;
	volatile UWORD	Parametric_I;
	volatile UWORD	Parametric_II;
	volatile UWORD	Parametric_III;
	volatile UWORD	Presentation_I;
	volatile UWORD	Presentation_II;
	volatile UWORD	Presentation_III;
	volatile UWORD	Presentation_IV;
	volatile UWORD	Presentation_V;
	volatile UWORD	DataFIFO;
	volatile UWORD	Channel_Stat;
	volatile UWORD	Channel_ReadCtr;
	volatile UWORD	Channel_WriteCtr;
};

typedef struct L64111Regs L64111Regs;

/* Bit defines for CONTROL_1. */
#define ANC_DATA_FIFO_RST 0x80
#define SER_PAR           0x40
#define BYPASS            0x20
#define BUF_RST           0x04
#define SOFT_RST          0x02
#define DEC_STRT          0x01

/* Bit defines for CONTROL_2. */
#define ERROR_REPEAT 0x80
#define	ERROR_MUTE   0x40
#define	ERROR_IGNORE 0x00
#define SOFT_MUTE    0x20
#define SEL_16       0x10
#define AUDIO_ONLY   0x08
#define I2S	     0x04
#define PCM64FS	     0x02
#define PCM48FS	     0x01
#define PCM32FS	     0x00

/* Bit defines for CONTROL_3. */
#define AUDIO_STREAM_ID_USE 0x80
#define AUDIO_STREAM_ID_BIT 0x1F

/* Bit defines for STATUS_INTERRUPT_1. */
#define ANC_DATA_VALID      0x80
#define ANC_DATA_FIFO_OVRFL 0x40
#define ANC_DATA_FIFO_HFF   0x20
#define ERR_BUFF_UNDFLW     0x10
#define ERR_BUFF_OVL        0x08
#define	BUFF_ALMST_EMTY     0x04

/* Bit defines for STATUS_INTERRUPT_2. */

#define SYNTAX_ERR_DET_W	0x80
#define PTS_AVAILABLE_W		0x40
#define SYNC_AUD_W		0x20
#define SYNC_SYS_W		0x10
#define FRAME_DETECT_IN_W	0x08
#define ERR_CRC_W		0x04
#define NEW_FRAME_OUT_W		0x02

#define SYNTAX_ERR_DET_R	0x40
#define PTS_AVAILABLE_R		0x20
#define SYNC_AUD_R		0x10
#define SYNC_SYS_R		0x08
#define FRAME_DETECT_IN_R	0x04
#define ERR_CRC_R		0x02
#define NEW_FRAME_OUT_R		0x01

/* Bit mask for TIMER_COUNT_DOWN. */
#define TCR_MASK 0x0F

/* Common Bit mask for all CHANNEL_BUF registers. */
#define CHANNEL_BUF_MASK 0x7F


#define TCR_VALUE(sclk_khz,wclko_khz) (((unsigned int)((sclk_khz)/(wclko_khz))) >> 6)

#define TORH_VALUE(sclk_khz,wclko_khz) (((unsigned int)(((sclk_khz) << 2)/(wclko_khz))) & 0xFF)
#define TORL_VALUE(sclk_khz,wclko_khz) (((unsigned int) (((sclk_khz) << 10)/(wclko_khz))) & 0xFF)

#define L64111_UPPER_THRESHOLD	0x30
