/* aspool - nostartup - CLI/WorkBench - SAS/C 6.50 compatible */

#include <exec/execbase.h>
#include   <exec/types.h>
#include     <exec/memory.h>
#include <dos/dos.h>
#include   <dos/dosextens.h>
#include     <dos/rdargs.h>
#include <intuition/intuition.h>
#include <graphics/gfxbase.h>
#include <iff/iff.h>
#include   <iff/8svx.h>
#include <devices/audio.h>

/*  Extarnal Prototypes  */
#include <clib/exec_protos.h>
#include <clib/alib_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/utility_protos.h>

/*  Pragmas  */
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/utility_pragmas.h>

/*  Local Prototypes  */
void ParseAudioIFF(UBYTE *filename);
void ParseChunks8SVX(struct MyInst *CurrrentInst);
void PlayInst8SVX(struct MyInst *CurrentInst);
void ReadFileData(struct FileHandle *fh, UBYTE *bufptr, ULONG bufsize);
void ReportFault(UBYTE *ErrMsg);

#define CLI_TEMPLATE "8SVXNAME/A"
#define MAX_CHIP_AUDIO 0x1FFFE


typedef struct MyArg {
  UBYTE *filename;
  };

typedef struct MyInst {
  ID instType;
  BYTE *instHeader;
  BYTE *instData;
  BYTE *instData2;
  struct FileHandle *instFile;
  ULONG instFileUnread;
  };

/*  Global Data  */
ULONG ErrLevel = RETURN_OK;
ULONG ErrCode = NULL;
UBYTE *ErrMsg = NULL;  /*  Error message, altered by Walk../Emit..  */
ULONG resultES;
ULONG AudioClock;
struct ExecBase *SysBase;
struct Library *DOSBase = NULL;
struct Library *IntuitionBase = NULL;
struct Library *UtilityBase = NULL;
struct GfxBase *GfxBase = NULL;
struct WBStartup *wbMsg = NULL;
struct EasyStruct myES = {
  sizeof(struct EasyStruct),
  0,
  "aspool",
  "",
  ""
  };
struct FileHandle *audiofilehandle;
UBYTE verstiontag[] = "$VER: aspool 1.2 (02.18.94)";


/*  ---------------------------- cmd_startup -----------------------------  */

ULONG cmd_startup(void) {
  struct Process *Process;
  struct RDArgs *rdArgs = NULL;
  struct MyArg myArgs;

  SysBase = (*((struct ExecBase **) 4));
  Process = (struct Process *) SysBase->ThisTask;
  if (!(Process->pr_CLI)) {
    WaitPort(&Process->pr_MsgPort);
    wbMsg = (struct WBStartup *) GetMsg(&Process->pr_MsgPort);
    }
  DOSBase = OpenLibrary("dos.library",37);
  if (!DOSBase) {
    ErrLevel = RETURN_FAIL;
    ErrCode = ERROR_INVALID_RESIDENT_LIBRARY;
    goto ShutDown;
    }
  UtilityBase = OpenLibrary("utility.library",37);
  if (!UtilityBase) {
    ErrLevel = RETURN_FAIL;
    ErrCode = ERROR_INVALID_RESIDENT_LIBRARY;
    goto ShutDown;
    }

  /* ----- custom code here ------ */
  if (!wbMsg) {
    myArgs.filename = NULL;
    rdArgs = ReadArgs(CLI_TEMPLATE, (LONG *) &myArgs, NULL);
    if (rdArgs == NULL) {
      ReportFault("ReadArgs");
      goto ShutDown;
      }
    }
  else {
    IntuitionBase = OpenLibrary("intuition.library",37);
    if (IntuitionBase == NULL) {
      ErrLevel = RETURN_FAIL;
      ErrCode = ERROR_INVALID_RESIDENT_LIBRARY;
      goto ShutDown;
      }
    ReportFault("workbench interface disabled");
    goto ShutDown;
    }
  if (!wbMsg) {
    Printf("    What is the purpose of MEMF_PUBLIC!\n");
    Printf("  Attempt to load audio file: %s\n",myArgs.filename);
    }

  /*  Determine AudioClock value  */
  GfxBase = (struct GfxBase *) OpenLibrary("graphics.library",37);
  if (GfxBase == NULL) {
    ErrLevel = RETURN_FAIL;
    ErrCode = ERROR_INVALID_RESIDENT_LIBRARY;
    goto ShutDown;
    }
  if (GfxBase->DisplayFlags & REALLY_PAL)
    AudioClock = 3546895;  /*  PAL  */
  else
    AudioClock = 3579545;  /* NTSC  */
  CloseLibrary((struct Library *) GfxBase);

  /* Examine multiple files here?  */

  /*  Walk through Audio Data  */
  ParseAudioIFF(myArgs.filename);
  if (ErrMsg) {
    ReportFault(ErrMsg);
    goto ShutDown;
    }

ShutDown:
  if (rdArgs)
    FreeArgs(rdArgs);
  if (IntuitionBase)
    CloseLibrary(IntuitionBase);
  if (UtilityBase)
    CloseLibrary(UtilityBase);
  if (DOSBase)
    CloseLibrary(DOSBase);
  if (wbMsg) {
    Forbid();
    ReplyMsg((struct Message *) wbMsg);
    }
  if (ErrCode)
    Process->pr_Result2 = ErrCode;
  return(ErrLevel);
  }


/*  --------------------------- ParseAudioIFF ----------------------------  */

void ParseAudioIFF(UBYTE *filename) {
  struct MyInst *CurrentInst = NULL;
  ULONG formsize;
  GroupHeader groupheader;

  /*  Open audio file  */
  audiofilehandle = (struct FileHandle *) Open(filename,MODE_OLDFILE);
  if (audiofilehandle == NULL) {
    ErrMsg = "Open";
    goto Exit_ParseAudioIFF;
    }

  /*  Read in composite header chunk (IFF file type) */
  ReadFileData(audiofilehandle,(BYTE *) &groupheader, sizeof(GroupHeader));
  if (ErrMsg)
    goto Exit_ParseAudioIFF;

  /*  Allocate/Init Inst structure  */
  CurrentInst = AllocVec(sizeof(struct MyInst), MEMF_PUBLIC | MEMF_CLEAR);
  if (!CurrentInst) {
    ErrMsg = "Error, Low Memory";
    goto Exit_ParseAudioIFF;
    }
  CurrentInst->instFile = audiofilehandle;

  /*  Process IFF FORM  */
  if (groupheader.ckID == FORM) {
    /*  Declare amount of unread FORM data  */
    formsize = groupheader.ckSize - sizeof(ID);
    if (!formsize) {
      ErrMsg = "NULL IFF FORM";
      goto Exit_ParseAudioIFF;
      }
    CurrentInst->instFileUnread = formsize;

    /*  Process IFF FORM 8SVX  */
    if (groupheader.grpSubID == ID_8SVX) {
      ParseChunks8SVX(CurrentInst);
      if (ErrMsg)
        goto Exit_ParseAudioIFF;
      }

    goto Exit_ParseAudioIFF;
    }

  ErrMsg = "Error, Unknown/Unsupported IFF Type";

Exit_ParseAudioIFF:
  if (CurrentInst) {
    if (CurrentInst->instHeader)
      FreeVec(CurrentInst->instHeader);
    if (CurrentInst->instData)
      FreeVec(CurrentInst->instData);
    if (CurrentInst->instData2)
      FreeVec(CurrentInst->instData2);
    FreeVec(CurrentInst);
    }
  if (audiofilehandle)
    Close((BPTR) audiofilehandle);
  }


/*  -------------------------- ParseChunks8SVX ---------------------------  */

void ParseChunks8SVX(struct MyInst *CurrentInst) {
  ChunkHeader chunkheader;

  do {
    /*  Read in chunk header */
    ReadFileData(CurrentInst->instFile, (UBYTE *) &chunkheader,
      sizeof(ChunkHeader));
    if (ErrMsg)
      goto Exit_ParseChunks8SVX;

    /*  Decrement amount of unread FORM data  */
    CurrentInst->instFileUnread -= sizeof(ChunkHeader);
    if (chunkheader.ckSize & 1)
      chunkheader.ckSize++;  /*  pad IFF chunk size to WORD boundary  */

    switch (chunkheader.ckID) {

      case ID_VHDR:  /*  Read in chunk data into Inst Header -------------  */
      if (!wbMsg)
        Printf("  8SVX Header: %ld\n",chunkheader.ckSize);
      /*  Purge/Allocate Inst Header structure?  */
      if (CurrentInst->instHeader) {
        FreeVec(CurrentInst->instHeader);
        CurrentInst->instHeader = NULL;
      }
      CurrentInst->instHeader = AllocVec(chunkheader.ckSize, MEMF_PUBLIC);
      if (!CurrentInst->instHeader) {
        ErrMsg = "Error, Low Memory";
        goto Exit_ParseChunks8SVX;
        }
      ReadFileData(CurrentInst->instFile,CurrentInst->instHeader,
        chunkheader.ckSize);
      if (ErrMsg)
        goto Exit_ParseChunks8SVX;
      break;

      case ID_BODY:  /*  Read in chunk data into Inst Data area ----------  */
      if (!wbMsg)
        Printf("  8SVX Body\n");
      /*  Play must read entire remainder of chunk unless error  */
      PlayInst8SVX(CurrentInst);  /*  ------- PLAY SOMETHING -------------  */
      if (ErrMsg)
        goto Exit_ParseChunks8SVX;
      break;

      default:  /*  Seek to next header chunk ----------------------------  */
      if (!wbMsg)
        Printf("  8SVX unknown?\n");
      ReadFileData(CurrentInst->instFile, NULL, chunkheader.ckSize);
      if (ErrMsg)
        goto Exit_ParseChunks8SVX;
      break;

      }

    CurrentInst->instFileUnread -= chunkheader.ckSize;
    } while (CurrentInst->instFileUnread);

Exit_ParseChunks8SVX:
  }


/*  --------------------------- PlayInst8SVX -----------------------------  */

void PlayInst8SVX(struct MyInst *CurrentInst) {
  struct IOAudio *AudioIOA = NULL;
  struct IOAudio *AudioIOB = NULL;
  struct MsgPort *AudioPortA = NULL;
  struct MsgPort *AudioPortB = NULL;
  struct MsgPort *curAudioPort = NULL;
  struct Message *AudioMessage;
  Voice8Header *v8h;
  ULONG AudioDevice;
  ULONG curSampleSize,remainSampleSize;
  UWORD curOctave = 1;
  UBYTE ach[] = {1,2,4,8};  /*  Audio Channel selection (any)  */

  v8h = (Voice8Header *) CurrentInst->instHeader;

  if (!wbMsg) {
    Printf("  oneshot:%7ld",          v8h->oneShotHiSamples);
    Printf("%7ld Hz\n",               v8h->samplesPerSec);

    Printf("   repeat:%7ld",          v8h->repeatHiSamples);
    Printf("%7ld Samples/Hi Cycle\n", v8h->samplesPerHiCycle);

    Printf("  Octaves:%2ld",          v8h->ctOctave);
    Printf("  Comp:%2ld",             v8h->sCompression);
    Printf("  Volume $%lx\n",         v8h->volume);
    }
  if (v8h->sCompression) {
    if (!wbMsg)
      Printf("  Bonk!  Unsupported Compression.\n");
    goto Exit_PlayInst8SVX;
    }
  else if (!v8h->samplesPerSec) {
    if (!wbMsg)
      Printf("  Bonk!  Illegal sample period of 0.\n");
    goto Exit_PlayInst8SVX;
    }

  AudioIOA = AllocVec(sizeof(struct IOAudio), MEMF_PUBLIC | MEMF_CLEAR);
  if (!AudioIOA) {
    ErrMsg = "Error, Low Memory";
    goto Exit_PlayInst8SVX;
    }
  AudioPortA = CreateMsgPort();
  if (!AudioPortA) {
    ErrMsg = "Error, No more ports";
    goto Exit_PlayInst8SVX;
    }
  AudioIOB = AllocVec(sizeof(struct IOAudio), MEMF_PUBLIC | MEMF_CLEAR);
  if (!AudioIOB) {
    ErrMsg = "Error, Low Memory";
    goto Exit_PlayInst8SVX;
    }
  AudioPortB = CreateMsgPort();
  if (!AudioPortB) {
    ErrMsg = "Error, No more ports";
    goto Exit_PlayInst8SVX;
    }
  AudioIOA->ioa_Request.io_Message.mn_ReplyPort = AudioPortA;
  AudioIOA->ioa_Request.io_Message.mn_Node.ln_Pri = 0;  /*  for now  */
  AudioIOA->ioa_AllocKey = 0;
  AudioIOA->ioa_Data = ach;
  AudioIOA->ioa_Length = sizeof(ach);
  AudioDevice = OpenDevice(AUDIONAME, 0, AudioIOA, 0);
  if (AudioDevice) {
    ErrMsg = "Error, No audio channels free";
    goto Exit_PlayInst8SVX;
    }

  for (curOctave = 0; curOctave < v8h->ctOctave; curOctave++) {

    if (!wbMsg) {
      Printf("  Attempting to play 8SVX audio octave:%ld",curOctave + 1);
      Flush(Output());
      }

    /*  Read in one-shot data --------------------------------------------  */

    if (curSampleSize = v8h->oneShotHiSamples) {
      if (CurrentInst->instData) {  /*  Purge/Allocate Inst Data ?  */
        FreeVec(CurrentInst->instData);
        CurrentInst->instData = NULL;
        }
      remainSampleSize = curSampleSize << curOctave;
      if (remainSampleSize > MAX_CHIP_AUDIO)
        curSampleSize = MAX_CHIP_AUDIO;
      else
        curSampleSize = remainSampleSize;
      CurrentInst->instData = AllocVec(curSampleSize, MEMF_CHIP);
      if (!CurrentInst->instData) {
        ErrMsg = "Error, Low CHIP Memory";
        goto Exit_PlayInst8SVX;
        }
      ReadFileData(CurrentInst->instFile, CurrentInst->instData, curSampleSize);
      if (ErrMsg)
        goto Exit_PlayInst8SVX;
      remainSampleSize -= curSampleSize;

      if (!wbMsg) {
        Printf(" oneshot");
        Flush(Output());
        }
      AudioIOA->ioa_Request.io_Message.mn_ReplyPort = AudioPortA;
      AudioIOA->ioa_Request.io_Command = CMD_WRITE;
      AudioIOA->ioa_Request.io_Flags = ADIOF_PERVOL;
      AudioIOA->ioa_Data = CurrentInst->instData;
      AudioIOA->ioa_Length = curSampleSize;
      AudioIOA->ioa_Period = UDivMod32(AudioClock,v8h->samplesPerSec);
      AudioIOA->ioa_Volume = 64;
      AudioIOA->ioa_Cycles = 1;
      BeginIO(AudioIOA);
      curAudioPort = AudioPortA;
      *AudioIOB = *AudioIOA;  /*  share everything except ports  */

      /*  oneshot data >128K?  */

      if (remainSampleSize) {
        if (!wbMsg) {
          Printf(" >128K");
          Flush(Output());
          }
        if (CurrentInst->instData2) {  /*  Purge/Allocate Inst Data2 ?  */
          FreeVec(CurrentInst->instData2);
          CurrentInst->instData2 = NULL;
          }
        if (remainSampleSize > MAX_CHIP_AUDIO)
          curSampleSize = MAX_CHIP_AUDIO;
        else
          curSampleSize = remainSampleSize;
        CurrentInst->instData2 = AllocVec(curSampleSize, MEMF_CHIP);
        if (!CurrentInst->instData2) {
          ErrMsg = "Error, Low CHIP Memory";
          goto Exit_PlayInst8SVX;
          }
        }

      while (remainSampleSize) {
        if (!wbMsg) {  /*  progress monitor  */
          Printf(" .");
          Flush(Output());
          }
        if (remainSampleSize > MAX_CHIP_AUDIO)
          curSampleSize = MAX_CHIP_AUDIO;
        else
          curSampleSize = remainSampleSize;

        if (curAudioPort == AudioPortA) {
          ReadFileData(CurrentInst->instFile, CurrentInst->instData2, curSampleSize);
          if (ErrMsg)
            goto Exit_PlayInst8SVX;
          remainSampleSize -= curSampleSize;
          AudioIOB->ioa_Request.io_Message.mn_ReplyPort = AudioPortB;
          AudioIOB->ioa_Data = CurrentInst->instData2;
          AudioIOB->ioa_Length = curSampleSize;
          BeginIO(AudioIOB);
          WaitPort(AudioPortA);
          AudioMessage = GetMsg(AudioPortA);
          curAudioPort = AudioPortB;
          }

        else {
          ReadFileData(CurrentInst->instFile, CurrentInst->instData, curSampleSize);
          if (ErrMsg)
            goto Exit_PlayInst8SVX;
          remainSampleSize -= curSampleSize;
          AudioIOA->ioa_Request.io_Message.mn_ReplyPort = AudioPortA;
          AudioIOA->ioa_Data = CurrentInst->instData;
          AudioIOA->ioa_Length = curSampleSize;
          BeginIO(AudioIOA);
          WaitPort(AudioPortB);
          AudioMessage = GetMsg(AudioPortB);
          curAudioPort = AudioPortA;
          }

        }

      if (!(v8h->repeatHiSamples)) {
        WaitPort(curAudioPort);
        AudioMessage = GetMsg(curAudioPort);
        curAudioPort = NULL;
        if (!wbMsg)
          Printf("\n");
        }

      }  /*  if oneShot  */

    /*  Read/Skip repeat data (if any) -----------------------------------  */

    if (curSampleSize = v8h->repeatHiSamples) {
      curSampleSize = curSampleSize << curOctave;

      if (curSampleSize > MAX_CHIP_AUDIO) {
        ReadFileData(CurrentInst->instFile, NULL, curSampleSize);
        if (!ErrMsg) {
          remainSampleSize -= curSampleSize;
          ErrMsg = "  Skipped:  8SVX repeat portion >128K";
          }
        goto Exit_PlayInst8SVX;
        }

      if (curAudioPort == AudioPortA) {

        /*  If short oneShot followed by repeat, then need to allocate Data2  */
        if (CurrentInst->instData2) {  /*  Purge/Allocate Inst Data2 ?  */
          FreeVec(CurrentInst->instData2);
          CurrentInst->instData2 = NULL;
          }
        CurrentInst->instData2 = AllocVec(curSampleSize, MEMF_CHIP);
        if (!CurrentInst->instData2) {
          ErrMsg = "Error, Low CHIP Memory";
          goto Exit_PlayInst8SVX;
          }

        ReadFileData(CurrentInst->instFile, CurrentInst->instData2,
          curSampleSize);
        if (ErrMsg)
          goto Exit_PlayInst8SVX;
        remainSampleSize -= curSampleSize;
        AudioIOB->ioa_Request.io_Message.mn_ReplyPort = AudioPortB;
        AudioIOB->ioa_Data = CurrentInst->instData2;
        AudioIOB->ioa_Length = curSampleSize;
        AudioIOB->ioa_Period = UDivMod32(AudioClock,v8h->samplesPerSec);
        AudioIOB->ioa_Cycles = 10;
        AudioIOB->ioa_Request.io_Command = CMD_WRITE;
        AudioIOB->ioa_Request.io_Flags = ADIOF_PERVOL;
        AudioIOB->ioa_Volume = 64;
        BeginIO(AudioIOB);
        if (v8h->oneShotHiSamples) {
          WaitPort(curAudioPort);
          AudioMessage = GetMsg(curAudioPort);
          }
        curAudioPort = AudioPortB;
        }

      else {

        /*  If short oneShot followed by repeat, then need to allocate Data  */
        if (CurrentInst->instData) {  /*  Purge/Allocate Inst Data ?  */
          FreeVec(CurrentInst->instData);
          CurrentInst->instData2 = NULL;
          }
        CurrentInst->instData = AllocVec(curSampleSize, MEMF_CHIP);
        if (!CurrentInst->instData) {
          ErrMsg = "Error, Low CHIP Memory";
          goto Exit_PlayInst8SVX;
          }

        ReadFileData(CurrentInst->instFile, CurrentInst->instData,
          curSampleSize);
        if (ErrMsg)
          goto Exit_PlayInst8SVX;
        remainSampleSize -= curSampleSize;
        AudioIOA->ioa_Request.io_Message.mn_ReplyPort = AudioPortA;
        AudioIOA->ioa_Data = CurrentInst->instData;
        AudioIOA->ioa_Length = curSampleSize;
        AudioIOA->ioa_Period = UDivMod32(AudioClock,v8h->samplesPerSec);
        AudioIOA->ioa_Cycles = 10;
        AudioIOA->ioa_Request.io_Command = CMD_WRITE;
        AudioIOA->ioa_Request.io_Flags = ADIOF_PERVOL;
        AudioIOA->ioa_Volume = 64;
        BeginIO(AudioIOA);
        if (v8h->oneShotHiSamples) {
          WaitPort(curAudioPort);
          AudioMessage = GetMsg(curAudioPort);
          }
        curAudioPort = AudioPortA;
        }

      if (!wbMsg)
        Printf(" repeat\n");
      WaitPort(curAudioPort);
      AudioMessage = GetMsg(curAudioPort);
      curAudioPort = NULL;

      }  /*  if repeat  */

    }  /*  for curOctave  */

Exit_PlayInst8SVX:
  if (curAudioPort) {
    if (!wbMsg)
      Printf("\n");
    WaitPort(curAudioPort);
    AudioMessage = GetMsg(curAudioPort);
    }
  if (!AudioDevice)
    CloseDevice(AudioIOA);
  if (AudioPortA)
    DeleteMsgPort(AudioPortA);
  if (AudioPortB)
    DeleteMsgPort(AudioPortB);
  if (AudioIOA)
    FreeVec(AudioIOA);
  if (AudioIOB)
    FreeVec(AudioIOB);
  }


/*  --------------------------- ReadFileData -----------------------------  */

void ReadFileData(struct FileHandle *fh, UBYTE *bufptr, ULONG bufsize) {
  ULONG iocount;

  if (bufptr) {
    iocount = Read((BPTR) fh, bufptr, bufsize);
    if (iocount == -1)
      ErrMsg = "Read Error";
    else if (iocount < bufsize)
      ErrMsg = "Read Error, Incomplete IFF";
    }

  /*  bufptr == NULL, then seek (no data transfer)  */
  else {
    iocount = Seek((BPTR) fh, bufsize, OFFSET_CURRENT);
    if (iocount == -1)
      ErrMsg = "Seek Error";
    }
  }


/*  ---------------------------- ReportFault -----------------------------  */

void ReportFault(UBYTE *ErrMsg) {
  ULONG e2;

  if (!wbMsg) {
    Printf("  ");
    if (e2 = IoErr())
      PrintFault(e2,ErrMsg);
    else
      Printf("%s\n",ErrMsg);
    }
  else {
    myES.es_TextFormat = ErrMsg;
    myES.es_GadgetFormat = "abort";
    resultES = EasyRequest(NULL, &myES, NULL);
    }
  ErrLevel = RETURN_FAIL;
  }
