
/*
COPYRIGHT: Unless otherwise noted, all files are Copyright (c) 1992-1993
Commodore-Amiga, Inc.  All rights reserved.

DISCLAIMER: This software is provided "as is".  No representations or
warranties are made with respect to the accuracy, reliability, performance,
currentness, or operation of this software, and all use is at your own risk.
Neither commodore nor the authors assume any responsibility or liability
whatsoever with respect to your use of this software.
*/


/****************************************************************************/


#ifndef REQTEST_H
#define REQTEST_H


/****************************************************************************/


#include <exec/types.h>


/****************************************************************************/


struct CommonData
{
    UWORD RefObject;
    UWORD InitialLeftEdge;
    UWORD InitialTopEdge;
    UWORD InitialWidth;
    UWORD InitialHeight;
    char  TitleText[128];
    char  PositiveText[128];
    char  NegativeText[128];
    char  FontName[32];
    UWORD FontSize;
    char  Locale[128];
    BOOL  PrivateIDCMP;
    BOOL  SleepWindow;
    BOOL  IntuiMsgFunc;
    BOOL  FilterFunc;
};

struct FileReqData
{
    char InitialFile[128];
    char InitialDrawer[128];
    char InitialPattern[128];
    char AcceptPattern[128];
    char RejectPattern[128];
    BOOL DoSaveMode;
    BOOL DoMultiSelect;
    BOOL DoPatterns;
    BOOL DrawersOnly;
    BOOL RejectIcons;
    BOOL FilterDrawers;
};

struct FontReqData
{
    char  InitialName[32];
    UWORD InitialSize;
    UBYTE InitialFrontPen;
    UBYTE InitialBackPen;
    UBYTE InitialDrawMode;
    UBYTE InitialStyle;
    BOOL  DoFrontPen;
    BOOL  DoBackPen;
    BOOL  DoDrawMode;
    BOOL  DoStyle;
    BOOL  FixedWidthOnly;
    UWORD MinHeight;
    UWORD MaxHeight;
    UBYTE MaxFrontPen;
    UBYTE MaxBackPen;
    BOOL  FrontPens;
    BOOL  BackPens;
    BOOL  ModeList;
};

struct ScreenModeReqData
{
    ULONG InitialDisplayID;
    UWORD InitialDisplayWidth;
    UWORD InitialDisplayHeight;
    UWORD InitialDisplayDepth;
    UWORD InitialOverscanType;
    UWORD InitialInfoLeftEdge;
    UWORD InitialInfoTopEdge;
    BOOL  InitialInfoOpened;
    BOOL  InitialAutoScroll;
    BOOL  DoWidth;
    BOOL  DoHeight;
    BOOL  DoDepth;
    BOOL  DoOverscanType;
    BOOL  DoAutoScroll;
    BOOL  CustomSMList;
    ULONG PropertyFlags;
    ULONG PropertyMask;
    UWORD MinWidth;
    UWORD MaxWidth;
    UWORD MinHeight;
    UWORD MaxHeight;
    UWORD MinDepth;
    UWORD MaxDepth;
};

typedef unsigned long (*HOOKFUNC)();


/*****************************************************************************/


VOID TestRequester(struct CommonData *cd,
                   struct FileReqData *frd,
                   struct FontReqData *fod,
                   struct ScreenModeReqData *smrd,
                   ULONG requesterType);


/****************************************************************************/


#endif /* REQTEST_H */
