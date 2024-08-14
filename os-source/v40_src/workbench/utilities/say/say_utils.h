#ifndef SAY_UTILS_H
#define SAY_UTILS_H


/*****************************************************************************/


#include <exec/types.h>
#include <intuition/intuition.h>
#include "texttable.h"


/*****************************************************************************/


enum SayCommand
{
    SC_NOP,
    SC_QUIT,
    SC_SEX,
    SC_MODE,
    SC_PITCH,
    SC_RATE
};


enum SayStatus
{
    SS_NORMAL,
    SS_DOUBLEINVOCATION,
    SS_NOLIBRARY,
    SS_NODEVICE,
    SS_NOMEMORY,
    SS_STRINGTOOLONG,
    SS_BADFILE,
    SS_ABORTED
};


struct SayGadget
{
    ULONG           sg_GadgetKind;

    WORD            sg_LeftEdge;
    WORD            sg_TopEdge;
    WORD            sg_Width;
    WORD            sg_Height;

    AppStringsID    sg_Label;
    enum SayCommand sg_Command;
    ULONG           sg_GadgetFlags;
};


/*****************************************************************************/


VOID DrawSayBevelBox(SHORT x, SHORT y, SHORT w, SHORT h, ULONG tags, ...);
VOID SetGadgetAttr(struct Gadget *gad, ULONG tags, ...);
struct Window *OpenSayWindow(ULONG tag1, ...);
struct Gadget *CreateSayGadget(struct SayGadget *sg, ULONG tags, ...);
VOID CenterLine(AppStringsID id, UWORD x, UWORD y, UWORD w);
VOID ShowError(enum SayStatus error);


/*****************************************************************************/


#endif /* SAY_UTILS_H */
