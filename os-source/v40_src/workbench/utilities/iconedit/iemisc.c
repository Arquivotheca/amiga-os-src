
/* includes */
#include <dos/dosextens.h>
#include <string.h>
#include <stdio.h>

/* prototypes */
#include <clib/intuition_protos.h>
#include <clib/dos_protos.h>
#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>

/* direct ROM interface */
#include <pragmas/intuition_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/exec_pragmas.h>

/* application includes */
#include "iemisc.h"
#include "iemain.h"
#include "texttable.h"
#include "ieiff.h"


/*****************************************************************************/


extern struct Library *DOSBase;
extern struct Library *SysBase;
extern struct Library *IntuitionBase;
extern struct Library *GfxBase;


/*****************************************************************************/


UWORD __chip template_idata[] =
{
/* Plane 0 */
    0x0000,0x0000,0x0000,0x0400,0x0000,0x0000,0x0000,0x0C00,
    0x0000,0x0000,0x0000,0x0C00,0x0000,0x0000,0x0000,0x0C00,
    0x0000,0x0000,0x0000,0x0C00,0x0000,0x0000,0x0000,0x0C00,
    0x0000,0x0000,0x0000,0x0C00,0x0000,0x0000,0x0000,0x0C00,
    0x0000,0x0000,0x0000,0x0C00,0x0000,0x0000,0x0000,0x0C00,
    0x0000,0x0000,0x0000,0x0C00,0x0000,0x0000,0x0000,0x0C00,
    0x0000,0x0000,0x0000,0x0C00,0x0000,0x0000,0x0000,0x0C00,
    0x0000,0x0000,0x0000,0x0C00,0x0000,0x0000,0x0000,0x0C00,
    0x0000,0x0000,0x0000,0x0C00,0x0000,0x0000,0x0000,0x0C00,
    0x0000,0x0000,0x0000,0x0C00,0x0000,0x0000,0x0000,0x0C00,
    0x0000,0x0000,0x0000,0x0C00,0x7FFF,0xFFFF,0xFFFF,0xFC00,
/* Plane 1 */
    0xFFFF,0xFFFF,0xFFFF,0xF800,0xD555,0x5555,0x5555,0x5000,
    0xD555,0x5555,0x5555,0x5000,0xD555,0x5555,0x5555,0x5000,
    0xD555,0x5555,0x5555,0x5000,0xD555,0x5555,0x5555,0x5000,
    0xD555,0x5555,0x5555,0x5000,0xD555,0x5555,0x5555,0x5000,
    0xD555,0x5555,0x5555,0x5000,0xD555,0x5555,0x5555,0x5000,
    0xD555,0x5555,0x5555,0x5000,0xD555,0x5555,0x5555,0x5000,
    0xD555,0x5555,0x5555,0x5000,0xD555,0x5555,0x5555,0x5000,
    0xD555,0x5555,0x5555,0x5000,0xD555,0x5555,0x5555,0x5000,
    0xD555,0x5555,0x5555,0x5000,0xD555,0x5555,0x5555,0x5000,
    0xD555,0x5555,0x5555,0x5000,0xD555,0x5555,0x5555,0x5000,
    0xD555,0x5555,0x5555,0x5000,0x8000,0x0000,0x0000,0x0000,
};

struct Image template_data =
{
    0, 0,			/* Upper left corner */
    54, 22, 2,			/* Width, Height, Depth */
    template_idata,		/* Image data */
    0x0003, 0x0001,		/* PlanePick, PlaneOnOff */
    NULL			/* Next image */
};


/*****************************************************************************/


UWORD __chip Cross_sdata[] =
{
    0x0000, 0x0000,

    0x0100, 0x0000,
    0x0000, 0x0100,
    0x0100, 0x0000,
    0x0000, 0x0100,
    0x0100, 0x0000,
    0x0000, 0x0100,
    0x0100, 0x0000,
    0xAAAA, 0x5454,
    0x0100, 0x0000,
    0x0000, 0x0100,
    0x0100, 0x0000,
    0x0000, 0x0100,
    0x0100, 0x0000,
    0x0000, 0x0100,
    0x0100, 0x0000,
    0x0000, 0x0000,
    0x0000, 0x0000,
    0x0000, 0x0000,
    0x0000, 0x0000,
    0x0000, 0x0000,
    0x0000, 0x0000,
    0x0000, 0x0000,
    0x0000, 0x0000,
    0x0000, 0x0000,
    0x0000, 0x0000,
    0x0000, 0x0000,
    0x0000, 0x0000,
    0x0000, 0x0000,
    0x0000, 0x0000,
    0x0000, 0x0000,
    0x0000, 0x0000,
    0x0000, 0x0000,

    0x0000, 0x0000
};

static UWORD __chip FillData[] =
{
    0x0000,0x0000,		/* ctrl data */

    0x0   ,0x100 ,
    0x100 ,0x280 ,
    0x380 ,0x440 ,
    0x7c0 ,0x820 ,
    0xfe0 ,0x1010,
    0x1ff0,0x2008,
    0x3ff8,0x4004,
    0x1ff0,0x2008,
    0xfe0 ,0x1010,
    0x7c0 ,0x820 ,
    0x380 ,0x440 ,
    0x0   ,0x380 ,
    0x100 ,0xaa0 ,
    0x380 ,0x440 ,
    0x100 ,0x280 ,
    0x0   ,0x100 ,
    0xee0 ,0x0   ,

    0x0000,0x0000
};

static UWORD __chip WaitPointer[] =
{
    0x0000, 0x0000,

    0x0400, 0x07C0,
    0x0000, 0x07C0,
    0x0100, 0x0380,
    0x0000, 0x07E0,
    0x07C0, 0x1FF8,
    0x1FF0, 0x3FEC,
    0x3FF8, 0x7FDE,
    0x3FF8, 0x7FBE,
    0x7FFC, 0xFF7F,
    0x7EFC, 0xFFFF,
    0x7FFC, 0xFFFF,
    0x3FF8, 0x7FFE,
    0x3FF8, 0x7FFE,
    0x1FF0, 0x3FFC,
    0x07C0, 0x1FF8,
    0x0000, 0x07E0,

    0x0000, 0x0000,		/* reserved, must be NULL */
};


/*****************************************************************************/


VOID SetBusyPointer(struct Window * window)
{
#if 1
    SetWindowPointer (window, WA_BusyPointer, TRUE, WA_PointerDelay, TRUE, TAG_DONE);
#else
    SetPointer(window,WaitPointer,16,16,-6,0);
#endif
}


/*****************************************************************************/


VOID SetFillPointer(struct Window *window)
{
    SetPointer(window,FillData,17,16,-8,-16);
}


/*****************************************************************************/


VOID SetCrossPointer(struct Window *window)
{
    SetPointer(window,Cross_sdata,32,16,-8,-7);
}


/*****************************************************************************/


UBYTE ErrorBuffer[512];

SHORT EasyReq(struct Window *window, STRPTR title, STRPTR text,
              STRPTR gadgets, STRPTR args, ...)
{
struct EasyStruct es;

    es.es_StructSize   = sizeof(struct EasyStruct);
    es.es_Flags        = NULL;
    es.es_Title        = title;
    es.es_TextFormat   = text;
    es.es_GadgetFormat = gadgets;

    return ((SHORT) (EasyRequestArgs(window,&es,NULL,&args)));
}


/*****************************************************************************/


VOID NotifyUser(AppStringsID msg, STRPTR string)
{
STRPTR cont,text;

    if ((msg == MSG_IE_NOTHING) && (string == NULL))
    {
	return;
    }

    if (string == NULL)
    {
	text = GetString(msg);
        if (msg == MSG_IE_ERROR_NO_FREE_STORE)
        {
            SetIoErr(ERROR_NO_FREE_STORE);
            text = NULL;
        }
    }
    else
    {
	text = string;
    }

    cont = GetString(MSG_IE_CONTINUE_GAD);

    if (IoErr())
    {
        /* Get the extended error text */
        Fault (IoErr(), NULL, ErrorBuffer, 510);

        if (text)
        {
            EasyReq(NULL,VNAM,"%s\n%s",cont,text,ErrorBuffer);
        }
        else
        {
            EasyReq(NULL,VNAM,"%s",cont,ErrorBuffer);
        }

        SetIoErr(0);
    }
    else
    {
        EasyReq(NULL,VNAM,"%s",cont,text);
    }
}


/*****************************************************************************/


/* Get the file requester set up properly */
VOID FixFileAndPath(struct FileRequester *rf, STRPTR name)
{
    if (rf)
    {
	if (name)
	{
	    stcgfp (rf->rf_Dir, name);
	    stcgfn (rf->rf_File, name);
	}
	else
	{
	    WORD dlen = (strlen (rf->rf_Dir) - 1);

	    if ((dlen > 0) && (rf->rf_Dir[dlen] == '/'))
	    {
		rf->rf_Dir[dlen] = 0;
	    }
	}
    }
}
