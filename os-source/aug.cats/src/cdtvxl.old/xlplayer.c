
#define	VER 1
#define	REV 10


#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/lists.h>
#include <exec/interrupts.h>
#include <exec/io.h>
#include <devices/audio.h>
#include <libraries/dos.h>
#include <devices/cdtv.h>
#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#include <graphics/view.h>
#include <graphics/copper.h>
#include <hardware/custom.h>
#include <hardware/dmabits.h>
#include <hardware/intbits.h>

#include <intuition/intuition.h>
#include <intuition/screens.h>

#include <dos/dos.h>
#include <dos/dosextens.h>

#include <libraries/iffparse.h>

#if 1
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/alib_protos.h>
#include <clib/iffparse_protos.h>

#include <pragmas/exec_old_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/iffparse_pragmas.h>
#endif

#include "pan.h"
#include "xlplayer_rev.h"

/***********************************************************************
***
***  Definitions
***
************************************************************************/

typedef		short			SHORT;
typedef		char *			CSTR;

#define		INTDIV( a, b )		( ( (a) + ( (b) / 2 ) ) / (b) )

#define		CDTV_SECTOR_SIZE	2048
#define		CDXL_SPEED_BPS		( 75 * CDTV_SECTOR_SIZE )

// Buffering
#define		MAX_NODES		3
#define		NEXT_FRAME( n, m )	( ( (n) == ( m - 1 ) ) ? 0 : ( (n) + 1 ) )

// Copper lists
#define		COPPER_BUFFER_SIZE	2048
#define		COPPER_END		0xfffffffe
#define		COPPER_MOVE_COLOR0	( 0x0180 )

// Audio has different periods on NTSC vs PAL
#define		NTSC_FREQ		3579545
#define		PAL_FREQ		3546895

// Hardware Interrupt control
#define		ENABLE_INTR( f )	CustomPtr->intena = (INTF_SETCLR | (f) )
#define		DISABLE_INTR( f )	CustomPtr->intena = (f)
#define		CLEAR_INTR( f )		CustomPtr->intreq = (f)
#define		ENABLE_DMA( f )		CustomPtr->dmacon = (DMAF_SETCLR | DMAF_MASTER | (f) )
#define		DISABLE_DMA( f )	CustomPtr->dmacon = (f)


/***********************************************************************
***
***  Function Prototypes
***
***********************************************************************/

void	main( LONG, CSTR * );
void	__interrupt __saveds AudioIntrCode( void );
void	__interrupt __saveds __asm CDXLIntrCode( register __a2 struct CDXL * );
void	CDXLPlay( ULONG, ULONG, ULONG, LONG );
void	CopyColors( UWORD *, UWORD *, SHORT );
UWORD	*CopyCopper( UWORD * );
void	Error( CSTR );
void	Init( void );
void	InitAudio( void );
void	InitCDTV( void );
void	InitCDXL( void );
void	InitViewInfo( void );
void 	Quit(LONG Code);
void	QuitAudio( void );
void	QuitCDTV( void );
void	QuitCDXL( void );
void	QuitViewInfo( void );
void	SetViewInfo( LONG, PAN *, ULONG );
void	StartAudio( void );
void	StopAudio( void );

extern void loadpic(char *ilbmname);
extern void cleaniff(void);

/***********************************************************************
***
***  Global Variables
***
***********************************************************************/


extern	struct Library *DOSBase;
struct	GfxBase *GfxBase		= NULL;
struct	IntuitionBase *IntuitionBase	= NULL;
struct	Library *IFFParseBase	 	= NULL;

// Program
struct	Task *OurTask			= NULL;

// cdtv.device
struct	IOStdReq *CDTVIOReq		= NULL;
struct	MsgPort *CDTVIOPort		= NULL;

// CDXL
SHORT	MaxNodes			= MAX_NODES;
struct	MinList	CDXLList;
struct	CDXL	CDXLNodes[ MAX_NODES ];

// Files
CSTR	CDXLFile			= NULL;
ULONG	CDXLSector			= 0;
ULONG	CDXLNumSectors			= 0;
ULONG	CDXLNumFrames			= 0;

// CDXL Frame
PAN	FirstPan;
UBYTE	*FrameBuffer[ MAX_NODES ];
UWORD	*FrameCMap[ MAX_NODES ];
UWORD	*FrameAudio[ MAX_NODES ];
ULONG	Count				= 0;

// Display
struct	View	 CDXLView[ MAX_NODES ];
struct	ViewPort CDXLVP[ MAX_NODES ];
struct	BitMap	 CDXLBM[ MAX_NODES ];
struct	RasInfo	 CDXLRI[ MAX_NODES ];
UWORD	*Copper[ MAX_NODES ];
UWORD	*Color[ MAX_NODES ];
int	XLoc	= 0;
int	YLoc	= 0;

extern struct   Screen         *scr;         /* for ptr to screen structure */
extern struct   Window         *win;         /* for ptr to window structure */
extern struct   RastPort       *wrp;         /* for ptr to RastPort  */
extern struct   ViewPort       *vp;          /* for ptr to Viewport  */
extern struct 	BitMap	*origbitmap, *altbitmap;

// Signals
LONG	SwitchSignal			= -1;
LONG	SwitchSignalBit			= -1;

// Misc
struct	Custom *CustomPtr		= (struct Custom *) 0x00dff000;

// Audio
SHORT	AudioHandlerInUse 		= FALSE;
struct	Interrupt AudioInterrupt	= { NULL, NULL, NT_INTERRUPT, 5, "AI_Example",
					    NULL, (void (*)( void )) &AudioIntrCode };
struct	Interrupt *OldAudioVector	= NULL;
struct	IOAudio AudioMain;
struct	MsgPort *AudioMainPort		= NULL;
ULONG	AudioFrame			= 0;


// argument parsing 

#define TEMPLATE    "FROM/A,COUNT/N,X/K/N,Y/K/N,BACKGROUND/K" VERSTAG " Andy Finkel, Flying Cat., Inc."
#define OPT_FROM	0
#define	OPT_PLAY	1
#define	OPT_X		2
#define	OPT_Y		3
#define	OPT_BACKGROUND	4
#define	OPT_COUNT	5

long    opts[OPT_COUNT];
struct RDArgs   *rdargs= NULL;

/***********************************************************************
***
***  main
***
***	main entry point
***
***********************************************************************/

void main(LONG argc,CSTR argv[])
{
	LONG NumPlays = 1;

	// workbench
	if ( argc == 0 )exit( RETURN_FAIL );


//	( "CDXL Example Player %d.%02d by Pantaray, Inc. Ukiah CA\n", VER, REV );
	if(((int)((struct Library *)DOSBase)->lib_Version) < 36) {
	    puts("XLPlayer requires V36 or higher\n");
	    exit(10);
	}


      memset((char *)opts, 0, sizeof(opts));
      rdargs = ReadArgs(TEMPLATE, opts, NULL);

      if (rdargs == NULL) {
         PrintFault(IoErr(), NULL);
	 exit(10);
      }

    CDXLFile = (char *)opts[OPT_FROM];

    if(opts[OPT_PLAY]) {
	NumPlays = *(long *)opts[OPT_PLAY];
	if ( NumPlays < 1 )NumPlays = -1;
    }

    if(opts[OPT_X])XLoc = *(long *)opts[OPT_X];
    if(opts[OPT_Y])YLoc = *(long *)opts[OPT_Y];


    Init();


    CDXLPlay( CDXLSector, CDXLNumSectors, CDXLNumFrames, NumPlays );

    Quit( RETURN_OK );
}


/***********************************************************************
***
***  AudioIntrCode
***
************************************************************************/

void __interrupt __saveds AudioIntrCode(
	void
	)
	{
	CLEAR_INTR( INTF_AUD0 );

	AudioFrame = NEXT_FRAME( AudioFrame, MaxNodes );
	CustomPtr->aud[ 0 ].ac_ptr = CustomPtr->aud[ 1 ].ac_ptr = FrameAudio[ AudioFrame ];
	}


/***********************************************************************
***
***  CDXLIntrCode
***
************************************************************************/

void __interrupt __saveds __asm CDXLIntrCode(
	register __a2 struct CDXL *NodeCompleted
	)
	{
	Count++;
	Signal( OurTask, SwitchSignal );
	}


/***********************************************************************
***
***  CDXLPlay
***
***	Note:
***	This example assumes a CDXL file with all frames equal.
***	Video with audio uses triple buffering, vs. double buffering.
***	Play until (NumFrames-1)
***
************************************************************************/

void CDXLPlay(ULONG Sector, ULONG NumSectors, ULONG NumFrames, LONG NumPlays )
{
	ULONG BufferNum;
	LONG PortSignal;
	LONG SignalMask;
	LONG SignalIn;
	SHORT FramesToBuffer;
	SHORT NumColors		= ( CMAP_SIZE( &FirstPan ) >> 1 );
	BOOL ForEver		= ( NumPlays == -1 );
	BOOL AudioStarted;
	BOOL StopProgram	= FALSE;
	int toggle = 0;
	struct BitMap *curbitmap;

	do {
		// CDXL counters and flags
		Count		= 0;
		BufferNum	= 0;
		AudioFrame	= 0;
		AudioStarted	= FALSE;
		FramesToBuffer	= ( ( MaxNodes > 2 ) ? ( MaxNodes - 2 ) : 0 );

		// Signal from device, make sure it is clear.
		PortSignal	= ( 1 << CDTVIOPort->mp_SigBit );
		SetSignal( 0, PortSignal );
		SignalMask	= ( SwitchSignal | PortSignal | SIGBREAKF_CTRL_C );

		// Start CDXL with cdtv.device
		CDTVIOReq->io_Command = CD_READXL;
		CDTVIOReq->io_Offset  = Sector;
		CDTVIOReq->io_Length  = NumSectors;
		CDTVIOReq->io_Data    = (APTR) CDXLList.mlh_Head;
		SendIO( (struct IORequest *) CDTVIOReq );

		while ( Count < ( NumFrames - 1 ) ) {
			SignalIn = Wait( SignalMask );
			if ( SignalIn & SIGBREAKF_CTRL_C ) {
				StopProgram = TRUE;
				break;
				}

			if ( SignalIn & PortSignal )break;

			// May need to buffer some frames
			if ( FramesToBuffer > 0 ) {
				FramesToBuffer--;
				continue;
			}

			// Start audio only once
			if ( ( ! AudioStarted ) && FirstPan.AudioSize ) {
				AudioStarted = TRUE;
				StartAudio();
			}
			if(!opts[OPT_BACKGROUND]) {
			    // Insert colors and display frame
			    CopyColors( FrameCMap[ BufferNum ], Color[ BufferNum ], NumColors );
			    GfxBase->LOFlist = Copper[ BufferNum ];
			}
			else {
			    /* blit to bitmap not being shown */
			    curbitmap = (toggle == 0) ? altbitmap : origbitmap;
			    BltBitMap(CDXLRI[ BufferNum ].BitMap,0,0,
			    curbitmap, XLoc,YLoc,
			    CDXLVP[ BufferNum ].DWidth,
			    CDXLVP[ BufferNum ].DHeight,
			    0xC0, 0xFF, NULL);

			}
			// Switch to other buffer
			BufferNum = NEXT_FRAME( BufferNum, MaxNodes );
			if(opts[OPT_BACKGROUND]) {
			    toggle ^= 1;
			    if(toggle) {	// switch to alt bitmap 
			    	scr->RastPort.BitMap = altbitmap;
			    	scr->ViewPort.RasInfo->BitMap = altbitmap;
			    }
			    else {
			    	scr->RastPort.BitMap = origbitmap;
			    	scr->ViewPort.RasInfo->BitMap = origbitmap;
			    }
			    MakeScreen(scr);
			    RethinkDisplay();
			}
		}
		// May need to turn off audio
		if ( AudioStarted ) {
			StopAudio();
			AudioStarted = FALSE;
		}

		// This correctly terminates a CDXL
		AbortIO( (struct IORequest *) CDTVIOReq );
		WaitIO(  (struct IORequest *) CDTVIOReq );

		// Then a SEEK is required to force the drive to stop (sometimes necessary)
		CDTVIOReq->io_Command = CDTV_SEEK;
		CDTVIOReq->io_Offset  = 0;
		CDTVIOReq->io_Length  = 0;
		CDTVIOReq->io_Data    = NULL;
		(void) DoIO( (struct IORequest *) CDTVIOReq );

		if ( StopProgram )break;

	} while ( ForEver || ( --NumPlays > 0 ) );
}


/***********************************************************************
***
***  chkabort
***
***	Remove Lattice's CTRL-C
***
************************************************************************/

int chkabort(void) { return(0); }  /* really */

/***********************************************************************
***
***  CopyColors
***
************************************************************************/

void CopyColors(
	UWORD *CMap,
	UWORD *CopperCMap,
	SHORT NumColors
	)
	{
	do
		{
		// Skip over Load instruction
		CopperCMap++;

		// Copy color
		*CopperCMap++ = *CMap++;
		} while ( --NumColors );
	}


/***********************************************************************
***
***  CopyCopper
***
************************************************************************/

UWORD *CopyCopper(
	UWORD *Copper
	)
	{
	UWORD *Colors	 = ( GfxBase->LOFlist + 3 );
	ULONG *Ptr	 = (ULONG *) GfxBase->LOFlist;
	ULONG *CopperPtr = (ULONG *) Copper;
	SHORT i;

	for ( i = 0; i < ( COPPER_BUFFER_SIZE >> 2 ); i++ )
		{
		*CopperPtr = *Ptr;

		// Look for start of color table
		if ( ( *( (UWORD *) Ptr ) & 0x1f ) == COPPER_MOVE_COLOR0 )
			Colors = (UWORD *) CopperPtr;

		// Check for end of copper
		if ( *Ptr == COPPER_END )
			break;

		Ptr++;
		CopperPtr++;
		}

	return( Colors );
	}


/***********************************************************************
***
***  CXBRK
***
***	Remove Lattice's CTRL-C
***
************************************************************************/

int CXBRK(void)
{
return 0;
}



/***********************************************************************
***
***  Error
***
***********************************************************************/

void Error(CSTR str)
{
    puts( str );
    Quit( RETURN_FAIL );
}


/***********************************************************************
***
***  Init
***
***********************************************************************/

void Init(void)
{
char *mem;

    if ( ! ( IntuitionBase = (struct IntuitionBase *) OpenLibrary( "intuition.library", 0 ) ) )
	Error( "Intuition not available" );

	/* force library flush */
    if((mem = AllocMem( ( (ULONG) -12),MEMF_PUBLIC)))FreeMem(mem,(ULONG)-12);
    if((mem = AllocMem(((ULONG) -12),MEMF_PUBLIC)))FreeMem(mem,(ULONG)-12);
    RemakeDisplay();

    if ( ! ( GfxBase = (struct GfxBase *) OpenLibrary( "graphics.library", 0 ) ) )
	Error( "NOT an Amiga System" );

    if ( ! ( IFFParseBase = OpenLibrary( "iffparse.library", 0L ) ) )
	Error( "iffparse.library is not available" );

    /* load the backdrop picture */
//    if(opts[OPT_BACKGROUND])loadpic((char *)opts[OPT_BACKGROUND]);

    // Setup for communications with CDTV
    InitCDTV();

    // Setup CDXL variables and read first pan for sizes
    InitCDXL();

    // Audio
    InitAudio();

    // setup for view
    if(opts[OPT_BACKGROUND])loadpic((char *)opts[OPT_BACKGROUND]);
    InitViewInfo();

}


/***********************************************************************
***
***  InitAudio
***
***	Four audio requests because we are double buffering left & right.
***	Ask for left and right
***
***	left,	right,	right,	left
***	1,	2,	4,	8
***
************************************************************************/

void InitAudio(void)
{
	static UBYTE AskChannel[] =
		{
		( 1 + 2 ),
		( 1 + 4 ),
		( 4 + 8 ),
		( 2 + 8 )
		};
	ULONG Frequency;
	LONG AudioRate;
	UWORD AudioPeriod;

	// May not be necessary
	if ( FirstPan.AudioSize == 0 )
		return;

	// Setup request for initial channel allocation
	if ( ! ( AudioMainPort = CreatePort( 0, 0 ) ) )
		Error( "Could not create port for audio" );

	AudioMain.ioa_Request.io_Message.mn_ReplyPort	= AudioMainPort;
	AudioMain.ioa_Request.io_Message.mn_Node.ln_Pri	= 127;
	AudioMain.ioa_Request.io_Command		= ADCMD_ALLOCATE;
	AudioMain.ioa_Request.io_Flags			= ADIOF_NOWAIT;
	AudioMain.ioa_AllocKey				= 0;
	AudioMain.ioa_Data				= AskChannel;
	AudioMain.ioa_Length				= sizeof( AskChannel );

	if ( OpenDevice( "audio.device", 0, (struct IORequest *) &AudioMain, 0 ) )
		Error( "Cannot obtain audio channels" );

	// Setup audio interrupt
	CLEAR_INTR( INTF_AUD0 );
	OldAudioVector	  = SetIntVector( INTB_AUD0, &AudioInterrupt );
	AudioHandlerInUse = TRUE;

	// Calculate period
	Frequency	  = ( ( GfxBase->DisplayFlags & PAL ) ? PAL_FREQ : NTSC_FREQ );
	AudioRate	  = INTDIV( ( CDXL_SPEED_BPS * FirstPan.AudioSize ), FirstPan.Size );
	AudioPeriod	  = (UWORD) INTDIV( Frequency, AudioRate );

	// Set hardware
	CustomPtr->aud[ 0 ].ac_vol = CustomPtr->aud[ 1 ].ac_vol = 60;
	CustomPtr->aud[ 0 ].ac_per = CustomPtr->aud[ 1 ].ac_per = AudioPeriod;
	CustomPtr->aud[ 0 ].ac_ptr = CustomPtr->aud[ 1 ].ac_ptr = FrameAudio[ 0 ];
	CustomPtr->aud[ 0 ].ac_len = CustomPtr->aud[ 1 ].ac_len = ( FirstPan.AudioSize / sizeof( UWORD) );
	}


/***********************************************************************
***
***  InitCDTV
***
************************************************************************/

void InitCDTV(void)
{
	if ( ! ( CDTVIOPort = CreatePort( 0, 0 ) ) )
		Error( "Cannot create a CDTV port" );

	if ( ! ( CDTVIOReq = CreateStdIO( CDTVIOPort ) ) )
		Error( "Cannot attach CDTV StdIO" );

	if ( OpenDevice( CDTV_NAME, 0, (struct IORequest *) CDTVIOReq, 0 ) )
		Error( "CDTV Device will not open" );
}


/***********************************************************************
***
***  InitCDXL
***
***	- Get the sector for CDXL (where it starts)
***	- Get size (# of sectors, # of frames)
***	- Read first pan for sizes
***	- Double buffer for video, triple buffer if using audio as well
***	- Allocate frame buffers and prepare CDXL list
***	- Signal for frame changing
***
************************************************************************/

void InitCDXL(void)
{
	struct	FileLock *FileLock;
	struct	FileInfoBlock *fib;
	ULONG	Size;
	LONG	fsLock;
	LONG	File;
	UBYTE	fibbuf[ ( sizeof( struct FileInfoBlock ) + 3 ) ];
	SHORT	i;

	if ( ! ( fsLock = Lock( CDXLFile, ACCESS_READ ) ) )
		Error( "CDXL file not available" );

	FileLock = (struct FileLock *) BADDR( fsLock );

	// Pluck the sector right out of public part of the structure
	CDXLSector = FileLock->fl_Key;

	// Get size of file.
	// (Force longword alignment of fib pointer.)
	fib = (struct FileInfoBlock *) ( (LONG) ( fibbuf + 3 ) & ~3L );
	Examine( fsLock, fib );
	Size = fib->fib_Size;

	UnLock( fsLock );

	// Get the first PAN
	if ( ! ( File = Open( CDXLFile, MODE_OLDFILE ) ) )
		Error( "CDXL file not available" );

	if ( Read( File, &FirstPan, PAN_SIZE ) != PAN_SIZE )
		{
		Close( File );
		Error( "Not a CDXL file" );
		}

	Close( File );

	CDXLNumSectors = ( Size / CDTV_SECTOR_SIZE );
	CDXLNumFrames  = ( Size / FirstPan.Size );

	// Video with audio need triple buffering, video just double
	MaxNodes = ( ( FirstPan.AudioSize ) ? 3 : 2 );

	// Now that we know size of frame, allocate our two buffers, set ptrs,
	// and prepare CDXL List (static)
	NewList( (struct List *) &CDXLList );

	for ( i = 0; i < MaxNodes; i++ )
		{
		if ( ! ( FrameBuffer[ i ] = AllocMem( FirstPan.Size, ( MEMF_CHIP | MEMF_CLEAR ) ) ) )
			Error( "Not enough memory for CDXL frame buffers" );

		FrameCMap[ i ]	= (UWORD *) ( FrameBuffer[ i ] + PAN_SIZE );
		FrameAudio[ i ]	= (UWORD *) ( FrameBuffer[ i ] + FRAME_SIZE( &FirstPan ) - FirstPan.AudioSize );

		AddTail( (struct List *) &CDXLList, (struct Node *) &CDXLNodes[ i ] );
		CDXLNodes[ i ].Buffer	= FrameBuffer[ i ];
		CDXLNodes[ i ].Length	= FirstPan.Size;
		CDXLNodes[ i ].DoneFunc	= CDXLIntrCode;
		}

	// Tie list together to give us endless loop
	CDXLList.mlh_Head->mln_Pred     = CDXLList.mlh_TailPred;
	CDXLList.mlh_TailPred->mln_Succ = CDXLList.mlh_Head;

	// Signal used when frame is ready
	if ( ( SwitchSignalBit = AllocSignal( -1 ) ) == -1 )
		Error( "Cannot get signals" );

	SwitchSignal = ( 1 << SwitchSignalBit );

	// setup for CDXLIntrCode
	OurTask = FindTask( NULL );
	}


/***********************************************************************
***
***  InitViewInfo
***
************************************************************************/

void InitViewInfo(void)
{
LONG File;
SHORT i;

    // Setup views
    if ( ! ( File = Open( CDXLFile, MODE_OLDFILE ) ) )
	Error( "CDXL file not available" );

    for ( i = 0; i < MaxNodes; i++ )SetViewInfo( File, &FirstPan, i );
    Close( File );
}


/***********************************************************************
***
***  Quit
***
***********************************************************************/

void Quit(LONG Code)
{
static SHORT Quitting = FALSE;
char *mem;

    if ( Quitting )	return;

    Quitting = TRUE;

    QuitViewInfo();
    QuitAudio();
    QuitCDXL();
    QuitCDTV();

    if(opts[OPT_BACKGROUND])cleaniff();

    if ( IFFParseBase )CloseLibrary( IFFParseBase );
    if ( GfxBase )CloseLibrary( (struct Library *) GfxBase );

	/* force library flush */
    if((mem = AllocMem( ( (ULONG) -12),MEMF_PUBLIC)))FreeMem(mem,(ULONG)-12);
    if((mem = AllocMem(((ULONG) -12),MEMF_PUBLIC)))FreeMem(mem,(ULONG)-12);

    if ( IntuitionBase ) {
	RemakeDisplay();
	CloseLibrary( (struct Library *) IntuitionBase );
    }
    if(rdargs)FreeArgs(rdargs);

    exit( Code );
}


/***********************************************************************
***
***  QuitAudio
***
************************************************************************/

void QuitAudio(
	void
	)
	{
	if ( AudioHandlerInUse )
		{
		StopAudio();
		SetIntVector( INTB_AUD0, OldAudioVector );
		}

	if ( AudioMain.ioa_Request.io_Device )
		{
		AudioMain.ioa_Request.io_Command = ADCMD_FREE;
		DoIO( (struct IORequest *) &AudioMain );

		CloseDevice( (struct IORequest *) &AudioMain );

		if ( AudioMainPort )
			DeletePort( AudioMainPort );
		}
	}


/***********************************************************************
***
***  QuitCDTV
***
************************************************************************/

void QuitCDTV(
	void
	)
	{
	if ( CDTVIOReq )
		{
		if ( CDTVIOReq->io_Device )
			CloseDevice( (struct IORequest *) CDTVIOReq );

		DeleteStdIO( CDTVIOReq );
		}

	if ( CDTVIOPort )
		DeletePort( CDTVIOPort );
	}


/***********************************************************************
***
***  QuitCDXL
***
************************************************************************/

void QuitCDXL(
	void
	)
	{
	SHORT i;

	if ( SwitchSignalBit != -1 )
		FreeSignal( SwitchSignalBit );

	for ( i = 0; i < MaxNodes; i++ )
		if ( FrameBuffer[ i ] )
			FreeMem( FrameBuffer[ i ], FirstPan.Size );
	}


/***********************************************************************
***
***  QuitViewInfo
***
************************************************************************/

void QuitViewInfo(
	void
	)
	{
	SHORT i;

	for ( i = 0; i < MaxNodes; i++ )
		{
		if ( Copper[ i ] )
			FreeMem( Copper[ i ], COPPER_BUFFER_SIZE );

		FreeVPortCopLists( &CDXLVP[ i ] );

		if ( CDXLVP[ i ].ColorMap )
			FreeColorMap( CDXLVP[ i ].ColorMap );

		if ( CDXLView[ i ].LOFCprList )
			FreeCprList( CDXLView[ i ].LOFCprList );

		if ( CDXLView[ i ].SHFCprList )
			FreeCprList( CDXLView[ i ].SHFCprList );
		}
	}


/***********************************************************************
***
***  SetViewInfo
***
***	Somebody tell me a better way of centering a view, overscan or
***	not...
***
************************************************************************/

void SetViewInfo(LONG File, PAN *Pan, ULONG Num	)
{
	struct View *CurrentView;
	UBYTE *Buffer;
	LONG PlaneSize;
	LONG i;
	SHORT OriginCenterX;
	SHORT OriginCenterY;
	SHORT VP_X;
	SHORT VP_Y;
	SHORT NewX;
	SHORT NewY;
	SHORT GoHiRes = FALSE;
	SHORT GoLace  = FALSE;

	// Init structures
	InitView( &CDXLView[ Num ] );
	InitVPort( &CDXLVP[ Num ] );

	if ( ! ( CDXLVP[ Num ].ColorMap = GetColorMap( 64 ) ) )
		Error( "Not enough memory for ColorMap" );

	if ( ! ( Copper[ Num ] = AllocMem( COPPER_BUFFER_SIZE, MEMF_CHIP ) ) )
		Error( "Not enough memory for Copper Lists" );

	// Pre-Fill Buffers
	if ( Read( File, FrameBuffer[ Num ], Pan->Size ) != Pan->Size )
		Error( "Not a CDXL file" );

	// InterLace and High Resolution guesses
	if ( Pan->XSize >= 640 )
		GoHiRes = TRUE;

	if ( Pan->YSize >= 400 )
		GoLace = TRUE;

	CDXLView[ Num ].ViewPort = &CDXLVP[ Num ];

	// Choose mode
	CDXLView[ Num ].Modes = 0;
	switch ( PI_VIDEO( Pan ) )
		{
		case ( PIV_HAM ):
			CDXLView[ Num ].Modes = HAM;
			break;

		case ( PIV_YUV ):
			Error( "Cannot handle YUV video" );
			break;

		case ( PIV_AVM ):
			GoHiRes = TRUE;
			break;

		case ( PIV_STANDARD ):
			if ( Pan->PixelSize == 6 )
				CDXLView[ Num ].Modes = EXTRA_HALFBRITE;
			break;

		default:
			Error( "Unknown PAN Info" );
			break;
		}

	if ( GoLace )
		CDXLView[ Num ].Modes |= LACE;

	CDXLVP[ Num ].Modes = CDXLView[ Num ].Modes;

	// NOTE: The view does not care about HIRES
	if ( GoHiRes )
		CDXLVP[ Num ].Modes |= HIRES;

	CDXLVP[ Num ].DWidth   = Pan->XSize;
	CDXLVP[ Num ].DHeight  = Pan->YSize;
	CDXLVP[ Num ].RasInfo  = &CDXLRI[ Num ];
	CDXLRI[ Num ].RxOffset = 0;
	CDXLRI[ Num ].RyOffset = 0;
	CDXLRI[ Num ].Next     = 0;

	// Center images (use view and viewport to center small images)
	// Remember:
	//	CurrentView->DxOffset & OriginCenterX are lo-res
	//	CurrentView->DyOffset & OriginCenterY are non-interlace
	//	Views are lo-res specifications ONLY!
	//	ViewPorts may be lo-res or hi-res
	CurrentView	= ViewAddress();
	OriginCenterX	= ( CurrentView->ViewPort->DWidth  >> ( ( CurrentView->ViewPort->Modes & HIRES ) ? 2 : 1 ) );
	OriginCenterY	= ( CurrentView->ViewPort->DHeight >> ( ( CurrentView->ViewPort->Modes & LACE  ) ? 2 : 1 ) );

    // handle arguments passed into the player
    if(opts[OPT_X]) {
	if((XLoc + Pan->XSize) > (GoHiRes ? 640 : 320))
	    XLoc = (GoHiRes ? 640 : 320) - Pan->XSize;
	NewX = XLoc;
    }
    else {
        if(opts[OPT_BACKGROUND]) {
	    XLoc = (((GoHiRes ? 640 : 320) - Pan->XSize) / 2);
	}
	NewX = ( CurrentView->DxOffset + OriginCenterX - ( Pan->XSize >> ( ( GoHiRes ) ? 2 : 1 ) ) );
    }
    if(opts[OPT_Y]) {
	if((YLoc + Pan->YSize) > (GoLace ? 400 : 200))
	    YLoc = (GoLace ? 400 : 200) - Pan->YSize;
	NewY = YLoc;
    }
    else {
        if(opts[OPT_BACKGROUND]) {
	    YLoc  = (((GoLace ? 400 : 200) - Pan->YSize) / 2);
	}
	else NewY = ( CurrentView->DyOffset + OriginCenterY - ( Pan->YSize >> ( ( GoLace  ) ? 2 : 1 ) ) );
    }
	// Check Min for View
	if ( NewX < 98 )NewX = 98;

	if ( NewY < 25 )NewY = 25;

	// Check Max for View
	if ( NewX > 129 ) {
		VP_X = ( NewX - 129 );
		NewX = 129;
	}
	else VP_X = 0;

	if ( NewY > 56 ) {
  	    VP_Y = ( NewY - 56 );
	    NewY = 56;
	}
	else VP_Y = 0;

//printf("newx is %ld, newy is %ld, vpx is %ld, vpy is %ld\n",NewX, NewY, VP_X, VP_Y);

	CDXLView[ Num ].DxOffset = NewX;
	CDXLView[ Num ].DyOffset = NewY;
	CDXLVP[ Num ].DxOffset   = ( VP_X << ( ( GoHiRes ) ? 1 : 0 ) );
	CDXLVP[ Num ].DyOffset   = VP_Y;

	// Set Bit Plane Ptr's, handle interleaved (for display)
	Buffer = ( FrameBuffer[ Num ] + PAN_SIZE + CMAP_SIZE( Pan ) );
	InitBitMap( &CDXLBM[ Num ], Pan->PixelSize, Pan->XSize, Pan->YSize );

	if ( PI_PIXEL( Pan ) == PIF_LINES ) {
	    PlaneSize = CDXLBM[ Num ].BytesPerRow;
	    CDXLBM[ Num ].BytesPerRow *= Pan->PixelSize;
	}
	else PlaneSize = ( CDXLBM[ Num ].BytesPerRow * Pan->YSize );

	for ( i = 0; i < Pan->PixelSize; i++ )
		CDXLBM[ Num ].Planes[ i ] = &Buffer[ ( i * PlaneSize ) ];

	    CDXLRI[ Num ].BitMap = &CDXLBM[ Num ];
	    // Make most of View
	    LoadRGB4( &CDXLVP[ Num ], FrameCMap[ Num ], CMAP_SIZE( Pan ) );
	    MakeVPort( &CDXLView[ Num ], &CDXLVP[ Num ] );
	if(!opts[OPT_BACKGROUND]) {
	    // Make rest of View
	    MrgCop( &CDXLView[ Num ] );
	    LoadView( &CDXLView[ Num ] );
	    Color[ Num ] = CopyCopper( Copper[ Num ] );
	}
}

/***********************************************************************
***
***  StartAudio
***
***	Start interrupts for switching audio buffers
***	Set Ptr and Length
***
************************************************************************/

void StartAudio(
	void
	)
	{
	CLEAR_INTR( INTF_AUD0 );
	ENABLE_DMA( DMAF_AUD0 );
	ENABLE_DMA( DMAF_AUD1 );
	ENABLE_INTR( INTF_AUD0 );
	}


/***********************************************************************
***
***  StopAudio
***
************************************************************************/

void StopAudio(
	void
	)
	{
	DISABLE_DMA( DMAF_AUD0 );
	DISABLE_DMA( DMAF_AUD1 );
	DISABLE_INTR( INTF_AUD0 );
	CLEAR_INTR( INTF_AUD0 );
	}

