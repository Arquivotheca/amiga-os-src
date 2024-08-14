
/* includes */
#include <exec/devices.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/ports.h>
#include <exec/execbase.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <devices/audio.h>
#include <prefs/sound.h>
#include <intuition/classes.h>
#include <intuition/classusr.h>
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
#include "pe_custom.h"

#define SysBase ed->ed_SysBase

/*****************************************************************************/

VOID __stdargs BeginIO (struct IORequest * io);
LONG __stdargs TimeDelay (long unit, unsigned long secs, unsigned long microsecs);
ULONG setdtattrs (EdDataPtr ed, Object * o, ULONG data,...);

/*****************************************************************************/

static char __chip soundtable[] =
{
    0x00, 0x30, 0x56, 0x6E, 0x78, 0x72, 0x56, 0x2C,
    0xFF, 0xD3, 0xAC, 0x8F, 0x84, 0x90, 0xAC, 0xD1
};

static char voltable[] =
{
    0x40, 0x3C, 0x38, 0x34, 0x30, 0x2C, 0x28, 0x24,
    0x20, 0x1C, 0x18, 0x14, 0x10, 0x0C, 0x08, 0x04
};

/*****************************************************************************/

EdStatus DoAudio (EdDataPtr ed, STRPTR name, BOOL real)
{
    UBYTE audioChannels[] = {3, 6, 1, 2, 4, 8};
    struct dtTrigger dtt;
    struct IOAudio ioa2;
    struct IOAudio ioa;
    APTR sample = NULL;

    if (ed->ed_PrefsWork.sop_AudioType == SPTYPE_SAMPLE)
    {
	setdtattrs (ed, ed->ed_DataObject,
		    SDTA_Period, ed->ed_PrefsWork.sop_AudioPeriod,
		    SDTA_Volume, ed->ed_PrefsWork.sop_AudioVolume,
		    TAG_DONE);

	dtt.MethodID = DTM_TRIGGER;
	dtt.dtt_Function = STM_PLAY;
	DoMethodA (ed->ed_DataObject, &dtt);
    }
    else if (real && ed->ed_PrefsWork.sop_AudioQueue)
    {
	ioa.ioa_Request.io_Message.mn_Node.ln_Pri = 85;
	ioa.ioa_Request.io_Message.mn_Node.ln_Type = NT_MESSAGE;
	ioa.ioa_Request.io_Message.mn_Length = sizeof (struct IOAudio);
	ioa.ioa_Request.io_Message.mn_ReplyPort = &((struct Process *) (SysBase->ThisTask))->pr_MsgPort;
	ioa.ioa_Data = audioChannels;
	ioa.ioa_Length = sizeof (audioChannels);

	if (!OpenDevice ("audio.device", 0, &ioa, 0))
	{
	    register ULONG delay;
	    register UWORD vol;

	    ioa.ioa_Request.io_Flags = ADIOF_PERVOL;
	    ioa.ioa_Request.io_Command = CMD_WRITE;
	    ioa.ioa_Volume = ed->ed_PrefsWork.sop_AudioVolume;
	    ioa.ioa_Period = ed->ed_PrefsWork.sop_AudioPeriod;

	    ioa.ioa_Cycles = 0;
	    ioa.ioa_Length = sizeof (soundtable);
	    ioa.ioa_Data = (APTR) & soundtable;
	    ioa.ioa_Volume = (voltable[0] * ed->ed_PrefsWork.sop_AudioVolume) / 64;

	    ioa2 = ioa;
	    ioa2.ioa_Request.io_Command = ADCMD_PERVOL;

	    BeginIO (&ioa);

	    delay = ed->ed_PrefsWork.sop_AudioDuration * 400;

	    for (vol = 1; vol < sizeof (voltable); vol++)
	    {
		ioa2.ioa_Volume = (voltable[vol] * ed->ed_PrefsWork.sop_AudioVolume) / 64;
		ioa2.ioa_Request.io_Flags = IOF_QUICK | ADIOF_SYNCCYCLE;
		BeginIO (&ioa2);
		WaitIO (&ioa2);
		TimeDelay (UNIT_MICROHZ, 0, delay);
	    }

	    AbortIO (&ioa);
	    WaitIO (&ioa);

	    CloseDevice (&ioa);
	    FreeVec (sample);
	}
    }
    return (ES_NORMAL);
}
