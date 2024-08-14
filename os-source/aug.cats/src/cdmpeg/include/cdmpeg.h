/*******************
      cdmpeg.h

    W.D.L 931014
********************/


#define	MPEG_NAME	"cd32mpeg.device"
#define CD_NAME		"cd.device"

// Protos

DoQuery( UBYTE * filename, DISP_DEF * disp_def );
DoILBM( UBYTE * filename, DISP_DEF * disp_def );

OpenDisplay( DISP_DEF * disp_def, UBYTE * ilbmfile );
VOID CloseDisplay( DISP_DEF * disp_def );
VOID DoBorder(void);

#define	MPEG_LMBABORT		0x10000000
#define	MPEG_RMBABORT		0x20000000
#define	MPEG_FIREABORT		0x40000000
#define	MPEG_BUTTONABORTMASK	0x70000000
#define	MPEG_INFO		0x00000001

