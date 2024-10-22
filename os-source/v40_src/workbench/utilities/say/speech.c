
#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <devices/narrator.h>
#include <string.h>

#include <clib/exec_protos.h>
#include <clib/translator_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/translator_pragmas.h>

#include "speech.h"


/*****************************************************************************/


#define BUFSIZE     4096
#define SAYPORTNAME "Say"

BYTE channelMask[] = {3, 5, 10, 12};


/*****************************************************************************/


extern struct Library *SysBase;
APTR                   phonBuffer;
struct MsgPort        *narratorPort;
struct narrator_rb    *narratorReq;
struct Library        *TranslatorBase;
BOOL                   speechStarted = FALSE;


/*****************************************************************************/


enum SayStatus OpenSpeech(VOID)
{
enum SayStatus result;

    result = SS_NOMEMORY;

    Forbid();
    if (narratorPort = FindPort(SAYPORTNAME))
    {
        Signal(narratorPort->mp_SigTask,SIGBREAKF_CTRL_C);
        result = SS_DOUBLEINVOCATION;
    }
    else
    {
        if (narratorReq = CreateIORequest(narratorPort = CreateMsgPort(),sizeof(struct narrator_rb)))
        {
            narratorPort->mp_Node.ln_Name = SAYPORTNAME;
            AddPort(narratorPort);
        }
    }
    Permit();

    if (result != SS_DOUBLEINVOCATION)
    {
        if (phonBuffer = AllocVec(BUFSIZE,MEMF_PUBLIC))
        {
            result = SS_NOLIBRARY;
            if (TranslatorBase = OpenLibrary("translator.library",37))
            {
                result = SS_NODEVICE;
                if (OpenDevice("narrator.device",0,narratorReq,0) == 0)
                {
                    return(SS_NORMAL);
                }
            }
        }

        CloseSpeech();
    }

    return(result);
}


/*****************************************************************************/


VOID CloseSpeech(VOID)
{
    if (narratorReq)
    {
        RemPort(narratorPort);
        WaitSpeech();
        CloseDevice(narratorReq);
    }

    FreeVec(phonBuffer);
    CloseLibrary(TranslatorBase);
    DeleteMsgPort(narratorPort);
    DeleteIORequest(narratorReq);
}


/*****************************************************************************/


enum SayStatus Say(STRPTR string, UWORD sex, UWORD pitch, UWORD rate, UWORD mode)
{
UWORD i;

    i = 0;
    while (string[i] != 0)
    {
        if (string[i] == '\n')
            string[i] = ' ';
        i++;
    }

    WaitSpeech();
    if (Translate(string,i,phonBuffer,BUFSIZE) == 0)
    {
        narratorReq->message.io_Message.mn_Length = sizeof(struct narrator_rb);
        narratorReq->message.io_Command           = CMD_WRITE;
        narratorReq->message.io_Offset            = 0;
        narratorReq->message.io_Data              = phonBuffer;
        narratorReq->message.io_Length            = strlen(phonBuffer);
        narratorReq->ch_masks                     = channelMask;
        narratorReq->nm_masks                     = sizeof(channelMask);
        narratorReq->sex                          = sex;
        narratorReq->pitch                        = pitch;
        narratorReq->rate                         = rate;
        narratorReq->mode                         = mode;
        SendIO(narratorReq);
        speechStarted = TRUE;

        return(SS_NORMAL);
    }

    return(SS_STRINGTOOLONG);
}


/*****************************************************************************/


VOID WaitSpeech(VOID)
{
    if (speechStarted)
    {
        WaitIO(narratorReq);
        speechStarted = FALSE;
    }
}


/*****************************************************************************/


VOID StopSpeech(VOID)
{
    if (speechStarted)
    {
        AbortIO(narratorReq);
        WaitSpeech();
    }
}
