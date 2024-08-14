
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <exec/libraries.h>
#include <devices/audio.h>
#include <string.h>

#include <clib/exec_protos.h>
#include <clib/alib_protos.h>

#include <pragmas/exec_pragmas.h>

#include "cduibase.h"


/*****************************************************************************/


extern UBYTE fanfare[];


/*****************************************************************************/


struct Sample
{
    ULONG s_Length;		/* Sample length */
    ULONG s_Period;		/* Sample period */
};


/*****************************************************************************/


/* We need it in stereo */
UBYTE audioLChannels[] = {2, 4};
UBYTE audioRChannels[] = {1, 8};

/* Max playable sample in one IO request is 128K */
#define MAXSAMPLE 131072


/*****************************************************************************/


#undef SysBase

VOID FanfarePlayer(VOID)
{
struct Sample *s;
STRPTR buffer;
struct IOAudio *aio[4];
struct IOAudio ioaL1;
struct IOAudio ioaL2;
struct IOAudio ioaR1;
struct IOAudio ioaR2;
UWORD sndPeriod;
LONG req, reqn;
BYTE *samptr;
LONG size;
struct CDUILib  *CDUIBase;
struct ExecBase *SysBase;
struct MsgPort  *port;

    SysBase  = (*((struct ExecBase **) 4));
    CDUIBase = SysBase->ThisTask->tc_UserData;

    port = CreateMsgPort();

    /* Initialize the audio request */
    memset (&ioaL1, 0, sizeof (struct IOAudio));
    ioaL1.ioa_Request.io_Message.mn_Node.ln_Pri = 89;
    ioaL1.ioa_Request.io_Message.mn_Node.ln_Type = NT_MESSAGE;
    ioaL1.ioa_Request.io_Message.mn_Length = sizeof (struct IOAudio);
    ioaL1.ioa_Request.io_Message.mn_ReplyPort = port;
    ioaL1.ioa_Request.io_Command = ADCMD_ALLOCATE;
    ioaL1.ioa_Request.io_Flags = ADIOF_NOWAIT;
    ioaL1.ioa_Data = audioLChannels;
    ioaL1.ioa_Length = sizeof (audioLChannels);

    /* Open the audio device */
    OpenDevice ("audio.device", 0, (struct IORequest *) &ioaL1, 0);

    /* Copy the IO request */
    CopyMem (&ioaL1, &ioaL2, sizeof (struct IOAudio));

    /* Initialize the audio request */
    memset (&ioaR1, 0, sizeof (struct IOAudio));
    ioaR1.ioa_Request.io_Message.mn_Node.ln_Pri = 89;
    ioaR1.ioa_Request.io_Message.mn_Node.ln_Type = NT_MESSAGE;
    ioaR1.ioa_Request.io_Message.mn_Length = sizeof (struct IOAudio);
    ioaR1.ioa_Request.io_Message.mn_ReplyPort = port;
    ioaR1.ioa_Request.io_Command = ADCMD_ALLOCATE;
    ioaR1.ioa_Request.io_Flags = ADIOF_NOWAIT;
    ioaR1.ioa_Data = audioRChannels;
    ioaR1.ioa_Length = sizeof (audioRChannels);

    /* Open the audio device */
    if (OpenDevice ("audio.device", 0, (struct IORequest *) &ioaR1, 0) != 0)
	return;

    CopyMem (&ioaR1, &ioaR2, sizeof (struct IOAudio));

    s = (struct Sample *) fanfare;

    if (buffer = AllocVec (s->s_Length, MEMF_CHIP))
    {
        CopyMem((APTR)((ULONG)s + sizeof(struct Sample)),buffer,s->s_Length);

        /* Initialize the data */
        samptr = buffer;
        sndPeriod = (SysBase->ex_EClockFrequency * 5) / s->s_Period;

        Wait(SIGBREAKF_CTRL_D);
        Signal(CDUIBase->cb_CDUITask,SIGF_SINGLE);

        if (s->s_Length < MAXSAMPLE)
        {
            ioaL1.ioa_Request.io_Command = CMD_WRITE;
            ioaL1.ioa_Request.io_Flags = ADIOF_PERVOL;
            ioaL1.ioa_Data   = samptr;
            ioaL1.ioa_Length = s->s_Length;
            ioaL1.ioa_Period = sndPeriod;
            ioaL1.ioa_Volume = 63;
            ioaL1.ioa_Cycles = 1;

            ioaR1.ioa_Request.io_Command = CMD_WRITE;
            ioaR1.ioa_Request.io_Flags = ADIOF_PERVOL;
            ioaR1.ioa_Data   = samptr;
            ioaR1.ioa_Length = s->s_Length;
            ioaR1.ioa_Period = sndPeriod;
            ioaR1.ioa_Volume = 63;
            ioaR1.ioa_Cycles = 1;

            BeginIO ((struct IORequest *) &ioaL1);
            BeginIO ((struct IORequest *) &ioaR1);

            WaitIO ((struct IORequest *) &ioaL1);
            WaitIO ((struct IORequest *) &ioaR1);
        }
        else
        {
            req = 0;
            reqn = 1;

            aio[0] = &ioaL1;
            aio[1] = &ioaL2;
            aio[2] = &ioaR1;
            aio[3] = &ioaR2;

            aio[req+0]->ioa_Request.io_Command = CMD_WRITE;
            aio[req+0]->ioa_Request.io_Flags = ADIOF_PERVOL;
            aio[req+0]->ioa_Data   = samptr;
            aio[req+0]->ioa_Length = MAXSAMPLE;
            aio[req+0]->ioa_Period = sndPeriod;
            aio[req+0]->ioa_Volume = 63;
            aio[req+0]->ioa_Cycles = 1;

            aio[req+2]->ioa_Request.io_Command = CMD_WRITE;
            aio[req+2]->ioa_Request.io_Flags = ADIOF_PERVOL;
            aio[req+2]->ioa_Data   = samptr;
            aio[req+2]->ioa_Length = MAXSAMPLE;
            aio[req+2]->ioa_Period = sndPeriod;
            aio[req+2]->ioa_Volume = 63;
            aio[req+2]->ioa_Cycles = 1;

            BeginIO ((struct IORequest *) aio[req+0]);
            BeginIO ((struct IORequest *) aio[req+2]);

            for (samptr = samptr + MAXSAMPLE, size = s->s_Length - MAXSAMPLE; size > 0; samptr += MAXSAMPLE)
            {
                reqn = req ^ 1;

                aio[reqn+0]->ioa_Request.io_Command = CMD_WRITE;
                aio[reqn+0]->ioa_Request.io_Flags = ADIOF_PERVOL;
                aio[reqn+0]->ioa_Data   = samptr;
                aio[reqn+0]->ioa_Length = (size > MAXSAMPLE) ? MAXSAMPLE : size;
                aio[reqn+0]->ioa_Period = sndPeriod;
                aio[reqn+0]->ioa_Volume = 63;
                aio[reqn+0]->ioa_Cycles = 1;

                aio[reqn+2]->ioa_Request.io_Command = CMD_WRITE;
                aio[reqn+2]->ioa_Request.io_Flags = ADIOF_PERVOL;
                aio[reqn+2]->ioa_Data   = samptr;
                aio[reqn+2]->ioa_Length = (size > MAXSAMPLE) ? MAXSAMPLE : size;
                aio[reqn+2]->ioa_Period = sndPeriod;
                aio[reqn+2]->ioa_Volume = 63;
                aio[reqn+2]->ioa_Cycles = 1;

                BeginIO ((struct IORequest *) aio[reqn+0]);
                BeginIO ((struct IORequest *) aio[reqn+2]);

                WaitIO ((struct IORequest *) aio[req+0]);
                WaitIO ((struct IORequest *) aio[req+2]);

                size = (size > MAXSAMPLE) ? size - MAXSAMPLE : 0;
                req = reqn;
            }
            WaitIO ((struct IORequest *) aio[reqn+0]);
            WaitIO ((struct IORequest *) aio[reqn+2]);
        }
        FreeVec (buffer);
    }

    CloseDevice((struct IORequest *) &ioaL1);
    CloseDevice((struct IORequest *) &ioaR1);
    DeleteMsgPort(port);

    Forbid();
    Wait(SIGBREAKF_CTRL_C);
    Signal(CDUIBase->cb_CDUITask,SIGF_SINGLE);
    CDUIBase->cb_FanfareTask = NULL;
}

#define SysBase CDUIBase->cb_SysLib
