
/* includes */
#include <exec/devices.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/ports.h>
#include <exec/execbase.h>
#include <dos/dos.h>
#include <devices/audio.h>
#include <prefs/sound.h>
#include <libraries/iffparse.h>
#include <datatypes/datatypes.h>
#include <datatypes/datatypesclass.h>
#include <datatypes/soundclass.h>

/* prototypes */
#include <clib/exec_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/dos_protos.h>
#include <clib/datatypes_protos.h>

/* direct ROM interface */
#include <pragmas/exec_pragmas.h>
#include <pragmas/iffparse_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/datatypes_pragmas.h>

/* application includes */
#include "sound.h"
#include "iprefs.h"
#include "pread.h"
#include "backdrop.h"


/*****************************************************************************/


extern struct ExecBase *SysBase;
extern struct Library  *IntuitionBase;
extern struct Library  *IFFParseBase;
extern struct Library  *DOSBase;
extern struct Library  *DataTypesBase;
extern struct MsgPort   audioPort;


/*****************************************************************************/


VOID __stdargs BeginIO(struct IORequest *io);
LONG __stdargs TimeDelay( long unit, unsigned long secs, unsigned long microsecs );
ULONG __stdargs DoMethodA (Object *obj, Msg message);


/*****************************************************************************/


VOID (* __asm oldDisplayBeep)(register __a0 APTR,register __a6 APTR);
struct IOAudio     ioa;
struct SoundPrefs  sp;
BOOL               doFlash;
Object            *soundObj;


/*****************************************************************************/


static UBYTE audioChannels[] = {3,6,1,2,4,8};

static char __chip BeepSamples[] =
{
    0x00, 0x30, 0x56, 0x6E, 0x78, 0x72, 0x56, 0x2C,
    0xFF, 0xD3, 0xAC, 0x8F, 0x84, 0x90, 0xAC, 0xD1
};

static char BeepVolume[] =
{
    0x40, 0x3C, 0x38, 0x34, 0x30, 0x2C, 0x28, 0x24,
    0x20, 0x1C, 0x18, 0x14, 0x10, 0x0C, 0x08, 0x04
};


/*****************************************************************************/


VOID DoAudio(VOID)
{
ULONG            delay;
UWORD            vol;
struct IOAudio   ioa2;
struct dtTrigger dtt;

/* This causes a sound NOT to interrupt any currently playing sound
 *
 *   if (ioa.ioa_Request.io_Device)
 *      return;
 */

    CleanUpAudio();

    if (sp.sop_AudioQueue)
    {
        if ((sp.sop_AudioType == SPTYPE_SAMPLE) && soundObj)
        {
            SetDTAttrs(soundObj,NULL,NULL,SDTA_Period, sp.sop_AudioPeriod,
                                          SDTA_Volume, sp.sop_AudioVolume,
                                          TAG_DONE);

            dtt.MethodID     = DTM_TRIGGER;
            dtt.dtt_Function = STM_PLAY;
            DoMethodA(soundObj,&dtt);
        }
        else
        {
            ioa.ioa_Request.io_Message.mn_Node.ln_Pri  = 85;
            ioa.ioa_Request.io_Message.mn_Node.ln_Type = NT_MESSAGE;
            ioa.ioa_Request.io_Message.mn_Length       = sizeof(struct IOAudio);
            ioa.ioa_Request.io_Message.mn_ReplyPort    = &audioPort;
            ioa.ioa_Data                               = audioChannels;
            ioa.ioa_Length                             = sizeof(audioChannels);

            if (!OpenDevice("audio.device",0,&ioa,0))
            {
                ioa.ioa_Request.io_Flags   = ADIOF_PERVOL;
                ioa.ioa_Request.io_Command = CMD_WRITE;
                ioa.ioa_Volume             = sp.sop_AudioVolume;
                ioa.ioa_Period             = sp.sop_AudioPeriod;
                ioa.ioa_Cycles             = 0;
                ioa.ioa_Length             = sizeof(BeepSamples);
                ioa.ioa_Data               = (APTR)&BeepSamples;
                ioa.ioa_Volume             = sp.sop_AudioVolume;

                ioa2                        = ioa;
                ioa2.ioa_Request.io_Command = ADCMD_PERVOL;

                if (!(SetSignal(0,0) & SIGBREAKF_CTRL_F))
                {
                    BeginIO(&ioa);

                    delay = sp.sop_AudioDuration * 400;

                    for (vol = 1; vol < sizeof(BeepVolume); vol++)
                    {
                        if (SetSignal(0,0) & SIGBREAKF_CTRL_F)
                           break;

                        ioa2.ioa_Volume           = (BeepVolume[vol] * sp.sop_AudioVolume) / 64;
                        ioa2.ioa_Request.io_Flags = IOF_QUICK | ADIOF_SYNCCYCLE;
                        BeginIO(&ioa2);
                        TimeDelay(UNIT_MICROHZ,0,delay);
                    }
                    AbortIO(&ioa);
                }
                else
                {
                    CloseDevice(&ioa);
                    ioa.ioa_Request.io_Device = NULL;
                }
            }
            else
            {
                (*oldDisplayBeep)(NULL,IntuitionBase);
            }
        }
    }
}


/*****************************************************************************/


VOID CleanUpAudio(VOID)
{
    if (ioa.ioa_Request.io_Device)
    {
        AbortIO(&ioa);
        WaitIO(&ioa);
        CloseDevice(&ioa);
        ioa.ioa_Request.io_Device = NULL;
    }
}


/*****************************************************************************/


BOOL ChangeAudio(VOID)
{
BOOL result = FALSE;

    CleanUpAudio();

    if (soundObj)
    {
        DisposeDTObject(soundObj);
        soundObj = NULL;
        AttemptCloseDT();
    }

    if (sp.sop_DisplayQueue)
        doFlash = TRUE;
    else
        doFlash = FALSE;

    if ((!sp.sop_AudioQueue) || (sp.sop_AudioType == SPTYPE_BEEP))
        return(TRUE);

    if (AttemptOpenDT())
    {
        if (soundObj = NewDTObject(sp.sop_AudioFileName,DTA_SourceType, DTST_FILE,
                                                        TAG_DONE))
        {
            return(TRUE);
        }
        UserProblems(MSG_IP_ERROR_NO_SOUND,sp.sop_AudioFileName);
    }

    return(result);
}
