/*  atest - nostartup - CLI/WorkBench - SAS/C 6.50 compilable */

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <graphics/gfxbase.h>
#include <devices/audio.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/utility_protos.h>
#include <clib/alib_protos.h>

/*  Makes code tight :)  */
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/utility_pragmas.h>

typedef struct WaveDef {
  ULONG wavesize;
  BYTE *wave;
  UWORD f;
  UWORD d;
  UWORD l;
};

typedef struct ChannelDef {
  ULONG length;
  UBYTE *data;
};

/*  function prototypes  */
ULONG EmitAudio(struct WaveDef *EmitWave, struct ChannelDef *EmitCh);

/*  global data  */
ULONG ErrLevel = RETURN_OK;
ULONG ErrCode;  /*  value used by FAILAT, WHY  */
ULONG AudioChipClk;
struct ExecBase *SysBase;
struct Library *DOSBase = NULL;
struct Library *UtilityBase = NULL;
struct GfxBase *GfxBase = NULL;
UBYTE testch[] = {1, 2, 4, 8};
UBYTE versiontag[] = "$VER: atest 0.0 (--.12.93)";

/*  Startup Code  */
ULONG cmd_start(void) {
  struct WBStartup *wbMsg = NULL;
  struct Process *process;
  struct WaveDef EmitWave;
  struct ChannelDef EmitCh;

  SysBase = (*((struct ExecBase **) 4));
  process = (struct Process *) SysBase->ThisTask;
  if (!(process->pr_CLI)) {
    WaitPort(&process->pr_MsgPort);
    wbMsg = (struct WBStartup *) GetMsg(&process->pr_MsgPort);
  }
  DOSBase = OpenLibrary("dos.library",37);
  UtilityBase = OpenLibrary("utility.library",37);
  GfxBase = (struct GfxBase *) OpenLibrary("graphics.library",37);
  if (((ULONG) DOSBase & (ULONG) UtilityBase & (ULONG) GfxBase) == NULL) {
    ErrLevel = RETURN_FAIL;
    ErrCode = ERROR_INVALID_RESIDENT_LIBRARY;
    goto ShutDown;
  }
  if (GfxBase->DisplayFlags & REALLY_PAL)
    AudioChipClk = 3546895; /*  PAL  */
  else
    AudioChipClk = 3579545; /*  NTSC  */
  CloseLibrary((struct Library *) GfxBase);

  /*  Allocate waveform memory, assign wave data, audio channel  */
  EmitCh.length = sizeof(testch);
  EmitCh.data = testch;
  EmitWave.wavesize = 8;
  EmitWave.wave = AllocMem(EmitWave.wavesize, MEMF_CHIP | MEMF_PUBLIC);
  if (EmitWave.wave == NULL) {
    ErrCode = ERROR_NO_FREE_STORE;
    ErrLevel = RETURN_WARN;
    goto ShutDown;
  }
  EmitWave.wave[0] =   64;
  EmitWave.wave[1] =  127;
  EmitWave.wave[2] =   64;
  EmitWave.wave[3] =    0;
  EmitWave.wave[4] =  -64;
  EmitWave.wave[5] = -128;
  EmitWave.wave[6] =  -64;
  EmitWave.wave[7] =    0;
  EmitWave.l = 64;
  EmitWave.f = 440;
  EmitWave.d = 3;
  EmitAudio(&EmitWave, &EmitCh);

ShutDown:
  if (EmitWave.wave)
    FreeMem(EmitWave.wave, EmitWave.wavesize);
  if (UtilityBase)
    CloseLibrary(UtilityBase);
  if (DOSBase)
    CloseLibrary(DOSBase);

  if (wbMsg) {
    Forbid();
    ReplyMsg((struct Message *) wbMsg);
  }

  /* can't use SetIoErr() cause DOS ain't open! */
  process->pr_Result2 = ErrCode;

  return(ErrLevel);
}


ULONG EmitAudio(struct WaveDef *EmitWave, struct ChannelDef *EmitCh) {
  ULONG device;
  struct IOAudio *AudioIO = NULL;
  struct MsgPort *AudioMP = NULL;
  struct Message *AudioMsg;

  AudioIO = (struct IOAudio *) AllocMem(sizeof(struct IOAudio),MEMF_PUBLIC|MEMF_CLEAR);
  AudioMP = CreateMsgPort();
  if (((ULONG) AudioIO & (ULONG) AudioMP) == NULL) {
    ErrLevel = RETURN_WARN;
    ErrCode = ERROR_NO_FREE_STORE;
    goto EmitCleanup;
  }

  AudioIO->ioa_Request.io_Message.mn_ReplyPort = AudioMP;
  AudioIO->ioa_Request.io_Message.mn_Node.ln_Pri = 0;
  AudioIO->ioa_Request.io_Command = ADCMD_ALLOCATE;
  AudioIO->ioa_Request.io_Flags = ADIOF_NOWAIT;
  AudioIO->ioa_AllocKey = 0;
  AudioIO->ioa_Data = EmitCh->data;
  AudioIO->ioa_Length = EmitCh->length;
  device = OpenDevice(AUDIONAME, 0,(struct IORequest *)AudioIO,0);
  if (device) {
    ErrLevel = RETURN_WARN;
    ErrCode = ERROR_OBJECT_IN_USE;
    goto EmitCleanup;
  }

  AudioIO->ioa_Request.io_Message.mn_ReplyPort = AudioMP;
  AudioIO->ioa_Request.io_Command = CMD_WRITE;
  AudioIO->ioa_Request.io_Flags = ADIOF_PERVOL;
  AudioIO->ioa_Data = EmitWave->wave;
  AudioIO->ioa_Length = EmitWave->wavesize;
  /* this code does not handle wavesize > 64K) */
  AudioIO->ioa_Period = UDivMod32(AudioChipClk,(UWORD)EmitWave->wavesize*EmitWave->f);
  AudioIO->ioa_Volume = EmitWave->l;
  AudioIO->ioa_Cycles = EmitWave->f * EmitWave->d;
  BeginIO((struct IORequest *) AudioIO);
  WaitPort(AudioMP);
  AudioMsg = GetMsg(AudioMP);

EmitCleanup:
  if (device == NULL)
    CloseDevice((struct IORequest *) AudioIO);
  if (AudioMP)
    DeleteMsgPort(AudioMP);
  if (AudioIO)
    FreeMem(AudioIO, sizeof(struct IOAudio));

  return(ErrLevel);
}
