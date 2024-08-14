#ifndef SCREENMODEREQ_H
#define SCREENMODEREQ_H


/*****************************************************************************/


#include <exec/types.h>
#include <intuition/intuition.h>
#include <utility/tagitem.h>

#include "aslbase.h"
#include "requtils.h"


/*****************************************************************************/


struct ExtSMReq
{
    /* stuff needed for all requesters */
    struct ReqInfo      sm_ReqInfo;

    /* public portion of structure, must match definition in asl.h */
    ULONG               sm_DisplayID;
    ULONG               sm_DisplayWidth;
    ULONG               sm_DisplayHeight;
    UWORD               sm_DisplayDepth;
    UWORD               sm_OverscanType;
    BOOL                sm_AutoScroll;

    ULONG               sm_BitMapWidth;
    ULONG               sm_BitMapHeight;

    struct IBox         sm_Coords;

    BOOL                sm_InfoOpened;
    struct IBox         sm_InfoCoords;

    APTR                sm_UserData;

    /* private portion of structure */
    struct Window      *sm_InfoWindow;
    struct Gadget      *sm_InfoGadgets;
    struct Gadget      *sm_PropertyListGad;

    ULONG               sm_PropertyFlags;
    ULONG               sm_PropertyMask;
    struct List        *sm_CustomSMList;
    struct Hook        *sm_FilterFunc;
    ULONG               sm_MinWidth;
    ULONG		sm_MaxWidth;
    ULONG		sm_MinHeight;
    ULONG		sm_MaxHeight;
    UWORD               sm_MinDepth;
    UWORD               sm_MaxDepth;

    UWORD               sm_Flags;

    struct DisplayMode *sm_CurrentMode;
    struct List         sm_DisplayModes;
    struct List         sm_Properties;
    struct Node         sm_PropertyNodes[13];
    STRPTR              sm_OverscanLabels[5];

    ULONG               sm_OriginalDisplayID;
    ULONG               sm_OriginalDisplayWidth;
    ULONG               sm_OriginalDisplayHeight;
    UWORD               sm_OriginalDisplayDepth;
    UWORD               sm_OriginalOverscanType;
    BOOL                sm_OriginalAutoScroll;

    char                sm_ScanRates[40];
};


#define SMB_DOOVERSCAN   0
#define SMB_DOAUTOSCROLL 1
#define SMB_DOWIDTH      2
#define SMB_DOHEIGHT     3
#define SMB_DODEPTH      4

#define SMF_DOOVERSCAN   (1<<0)
#define SMF_DOAUTOSCROLL (1<<1)
#define SMF_DOWIDTH      (1<<2)
#define SMF_DOHEIGHT     (1<<3)
#define SMF_DODEPTH      (1<<4)


#define PUBLIC_SMR(smr) ((struct ScreenModeRequester *)((ULONG)smr+sizeof(struct ReqInfo)))


/*****************************************************************************/


struct ExtSMReq *AllocSMRequest(struct TagItem *tagList);
APTR SMRequest(struct ExtSMReq *requester, struct TagItem *tagList);
VOID FreeSMRequest(struct ExtSMReq *requester);


/*****************************************************************************/


#endif /* SCREENMODEREQ_H */
