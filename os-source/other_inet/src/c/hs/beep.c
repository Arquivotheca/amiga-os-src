/* -----------------------------------------------------------------------
 * beep.c		handshake_src
 *
 * $Locker:  $
 *
 * $Id: beep.c,v 1.1 91/05/09 16:05:07 bj Exp $
 *
 * $Revision: 1.1 $
 *
 * $Header: HOG:Other/inet/src/c/hs/RCS/beep.c,v 1.1 91/05/09 16:05:07 bj Exp $
 *
 * $Log:	beep.c,v $
 * Revision 1.1  91/05/09  16:05:07  bj
 * Initial revision
 * 
 *
 *------------------------------------------------------------------------
 */

/*****************************************************************************
 *
 * Stereo Sound Example
 *
 * Sam Dicker
 * 3 December 1985
 * (created:  17 October 1985)
 *
 * This program demonstrates allocating a stereo pair of channels using the
 * allocation/arbitration commands.  For simplicity, it uses no hardware
 * control commands and writes directly to the hardware registers.  To prevent
 * another task from stealing the channels before writing to the registers it
 * locks the channels.
 *
 *****************************************************************************/

#include "termall.h"

/* audio channel assignment */
#define LEFT0B      0
#define RIGHT0B     1
#define RIGHT1B     2
#define LEFT1B      3
#define LEFT0F      1
#define RIGHT0F     2
#define RIGHT1F     4
#define LEFT1F      8

/* used by example sound */
#define WAVELENGTH  2
#define CLOCK       3579545
#define SOUNDPREC   -40

/* four possible stereo pairs */
static UBYTE allocationMap[] =
  {
    LEFT0F | RIGHT0F,
    LEFT0F | RIGHT1F,
    LEFT1F | RIGHT0F,
    LEFT1F | RIGHT1F
  };

static struct IOAudio     *allocIOB = 0; /* used by cleanUp to determine 
                                          * what needs to be 'cleaned up' */
static struct IOAudio     *lockIOB = 0;
static struct Device      *device = 0;
static struct MsgPort     *port = 0,
                          *lockport = 0,
                          *beep_port;
static BYTE               *squareWaveData = 0;
static struct timerequest beep_req;


void CleanUp ();

void MakeBeep ( freq, volume )
unsigned long int  freq;
unsigned long int  volume;
{
    UBYTE channels;
    struct AudChannel *leftRegs, *rightRegs;


    /*
    * Do audio bell stuff.
    */
    if ( beeping )
        return;
    beeping = 1;
    
    if ( nvmodes.bell_type & 2 )   /* Visual bell */
       DisplayBeep ( Screen );
    
    if ( !( nvmodes.bell_type & 1 ) )  /* If no audio bell return now> */
       return;
       
    /* allocate I/O blocks from chip public memory and initialize to zero */

    if (((allocIOB = (struct IOAudio *)HSAllocMem(sizeof(struct IOAudio),
        MEMF_PUBLIC | MEMF_CLEAR)) == 0) ||
        ((lockIOB = (struct IOAudio *)HSAllocMem(sizeof(struct IOAudio),
            MEMF_PUBLIC | MEMF_CLEAR)) == 0))
      {
        CleanUp ();
        return;
      }

    /* open the audio device */

    if (OpenDevice(AUDIONAME, 0, (struct IORequest *) allocIOB, 0) != 0)
      {
        CleanUp ();
        return;
      }
    device = allocIOB->ioa_Request.io_Device;

    /* initialize I/O block for channel allocation */

    if ((port = CreatePort(/*"sound example"*/ NULL, 0)) == 0)
      {
        CleanUp ();
        return;
      }
    allocIOB->ioa_Request.io_Message.mn_ReplyPort = port;
    allocIOB->ioa_Request.io_Command = CMD_RESET;
    SendIO ( (struct IORequest *) allocIOB );
    WaitIO ( (struct IORequest *) allocIOB );
    
    allocIOB->ioa_Request.io_Message.mn_Node.ln_Pri = SOUNDPREC;
    allocIOB->ioa_Request.io_Command = ADCMD_ALLOCATE;

    /* if no channel is available immediately, abandon allocation */
    allocIOB->ioa_Request.io_Flags = ADIOF_NOWAIT;
    allocIOB->ioa_Data = allocationMap;
    allocIOB->ioa_Length = sizeof(allocationMap);

    /* allocate channels now.  Alternatively, ADCMD_ALLOCATE could have been
     * preformed when audio was first OpenDevice'd by setting up ioa_Data and
     * ioa_Length before OpenDevice'ing */

    SendIO ( (struct IORequest *) allocIOB);
    if (WaitIO ( (struct IORequest *) allocIOB))
      {
        CleanUp ();
        return;
      }

    /* initialize I/O block for to lock channels */

    if ((lockport = CreatePort(/*"sound example"*/ NULL, 0)) == 0)
      {
        CleanUp ();
        return;
      }
    lockIOB->ioa_Request.io_Message.mn_ReplyPort = lockport;
    lockIOB->ioa_Request.io_Device = device;

    /* one lock command to lock both channels */
    lockIOB->ioa_Request.io_Unit = allocIOB->ioa_Request.io_Unit;
    lockIOB->ioa_Request.io_Command = ADCMD_LOCK;
    lockIOB->ioa_AllocKey = allocIOB->ioa_AllocKey;

    /* lock the channels */
    SendIO ( (struct IORequest *) lockIOB);

    /* if lock returned there is an error */
    if (CheckIO ( (struct IORequest *) lockIOB))
      {
        CleanUp ();
        return;
      }

    /* compute the hardware register addresses */

    channels = (ULONG)(allocIOB->ioa_Request.io_Unit);
    leftRegs  = (struct AudChannel *) ((channels & LEFT0F)  ? 0xdff0a0 : 0xdff0d0);
    rightRegs = (struct AudChannel *) ((channels & RIGHT0F) ? 0xdff0b0 : 0xdff0c0);

    /* allocate waveform memory from chip addressable ram.  HSAllocMem always
     * allocates memory on a word boundary which is necessary for audio
     * waveform data */

    if ((squareWaveData = (BYTE *)HSAllocMem(WAVELENGTH, MEMF_CHIP)) == 0)
      {
        CleanUp ();
        return;
      }

    /* a two cycle square wave (how complex!) */

    squareWaveData[0] = 127;
    squareWaveData[1] = -127;

    /* these registers are described in detail in the Amiga Hardware Manual */

    /* write only hardware registers must be loaded separately. 
     * <reg1> = <reg2> = <data>  may not work with some compilers */
    leftRegs->ac_ptr   = (UWORD *)squareWaveData;
    rightRegs->ac_ptr  = (UWORD *)squareWaveData;
    leftRegs->ac_len   = WAVELENGTH / 2L;
    rightRegs->ac_len  = WAVELENGTH / 2L;

    /* a slightly different frequency is used in each channel to make the
     * sound a bit more interesting */

    leftRegs->ac_per  = CLOCK / freq / WAVELENGTH;
    rightRegs->ac_per = CLOCK / freq / WAVELENGTH;

    leftRegs->ac_vol   = volume;
    rightRegs->ac_vol  = volume;
    *(UWORD *)0xdff096 = DMAF_SETCLR | channels << DMAB_AUD0;

    /* play sound. */

    beep_port = InitTimer ( &beep_req, "Beep Timer" );
    beep_req.tr_time.tv_secs  = 0;
    beep_req.tr_time.tv_micro = 50000;
    beep_req.tr_node.io_Command = TR_ADDREQUEST;
    DoIO ( (struct IORequest *) &beep_req );
    CloseDevice ( (struct IORequest *) &beep_req );
    DeletePort ( beep_port );

    /* free any allocated audio channels.  In this instance explicitly
     * performing the ADCMD_FREE command is unnecessary.  CloseDevice'ing
     * with allocIOB performs it and frees the channels automaticly */

    allocIOB->ioa_Request.io_Command = ADCMD_FREE;
    DoIO ( (struct IORequest *) allocIOB);
    WaitIO ( (struct IORequest *) lockIOB);

    /* free up resources and exit */
    CleanUp ();
}


/* print an error message and free allocated resources */

void CleanUp ()
{
    if (squareWaveData != 0)
      {
        HSFreeMem(squareWaveData, WAVELENGTH);
        squareWaveData = 0;
      }
    if (port != 0)
      {
        DeletePort(port);
        port = 0;
      }
    if (lockport != 0)
      {
        DeletePort(lockport);
        lockport = 0;
      }
    if (device != 0)
      {
        CloseDevice ( (struct IORequest *) allocIOB);
      }
    if (lockIOB != 0)
      {
        HSFreeMem( (char *) lockIOB, sizeof(struct IOAudio));
        lockIOB = 0;
      }
    if (allocIOB != 0)
      {
        HSFreeMem( (char *) allocIOB, sizeof(struct IOAudio));
        allocIOB = 0;
      }
    beeping = 0;
}