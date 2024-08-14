/* :ts=8                                                                 **/
/*                                                                       **/
/*       debox_lib.fd                                                    **/
/*                                                                       **/
/*       William A. Ware                 9006.20                         **/
/*                                                                       **/
/**************************************************************************/
/*   This information is CONFIDENTIAL and PROPRIETARY                    **/
/*   Copyright (C) 1990, Silent Software Incorporated.                   **/
/*   All Rights Reserved.                                                **/
/**************************************************************************/
/**/
/* Programs */
/**/
#pragma libcall PlayerPrefsBase DoPlayer 1E 0
#pragma libcall PlayerPrefsBase DoPrefs 24 0
/**/
/**/
/* Title Screen*/
/**/
#pragma libcall PlayerPrefsBase CDTVTitle 2A 001
#pragma libcall PlayerPrefsBase SetVersionStr 30 801
/**/
/**/
/* Prefs Function*/
/**/
#pragma libcall PlayerPrefsBase FillCDTVPrefs 36 801
#pragma libcall PlayerPrefsBase ConfigureCDTV 3C 801
/**/
/**/
/* Screen Blanker*/
/**/
#pragma libcall PlayerPrefsBase InstallScreenSaver 42 0
#pragma libcall PlayerPrefsBase ScreenSaverCommand 48 001
/**/
/* Key click module.*/
/**/
#pragma libcall PlayerPrefsBase InstallKeyClick 4E 0
#pragma libcall PlayerPrefsBase KeyClickCommand 54 43218006
/**/
/* Utilities*/
/* */
#pragma libcall PlayerPrefsBase SafeWaitIO 5A 801
#pragma libcall PlayerPrefsBase FreeIORequest 60 801
#pragma libcall PlayerPrefsBase AllocIORequest 66 210804
#pragma libcall PlayerPrefsBase CenterCDTVView 6C 109804
/**/
/* Bookmark Utilities*/
/**/
#pragma libcall PlayerPrefsBase SaveCDTVPrefs 72 801
#pragma libcall PlayerPrefsBase DeleteBookMark 78 001
#pragma libcall PlayerPrefsBase WriteBookMark 7E 210804
#pragma libcall PlayerPrefsBase ReadBookMark 84 10803
/**/
/* TRADEMARK SCREEN*/
/**/
#pragma libcall PlayerPrefsBase PrepareTM 8A 0
#pragma libcall PlayerPrefsBase FreeTM 90 0
#pragma libcall PlayerPrefsBase DisplayTM 96 0
/**/
/* Color map manipulation*/
/**/
#pragma libcall PlayerPrefsBase FindViewRGB 9C 09803
#pragma libcall PlayerPrefsBase LoadFoundRGB A2 09803
#pragma libcall PlayerPrefsBase LoadFoundRGBFade A8 3A2109807
#pragma libcall PlayerPrefsBase LevelColor AE 1002
#pragma libcall PlayerPrefsBase BetweenColor B4 21003
#pragma libcall PlayerPrefsBase FadeCMap BA 10A9805
/**/
/**/
/* Extra Bit map routines */
/**/
#pragma libcall PlayerPrefsBase GrabBm C0 CBA9805
#pragma libcall PlayerPrefsBase FreeGrabBm C6 A9803
#pragma libcall PlayerPrefsBase CreateMask CC 90803
#pragma libcall PlayerPrefsBase FillBackDrop D2 A9803
#pragma libcall PlayerPrefsBase FillForeground D8 B10A9806
#pragma libcall PlayerPrefsBase BuildBg DE A9803
/**/
/**/
/**/
#pragma libcall PlayerPrefsBase StartViewMgr E4 0BA9805
#pragma libcall PlayerPrefsBase CloseViewMgr EA 801
#pragma libcall PlayerPrefsBase LoadCycleView F0 A9803
/**/
/**/
/* Other - Very private routines.*/
/**/
#pragma libcall PlayerPrefsBase WriteTinyStr F6 9210805
#pragma libcall PlayerPrefsBase DecompTradeMark FC 9802
/**/
/* Added Misc Routines.*/
/**/
#pragma libcall PlayerPrefsBase SafeDoIO 102 801
/**/
/* JoyMouse Routines*/
/**/
#pragma libcall PlayerPrefsBase InstallJoyMouse 108 0
#pragma libcall PlayerPrefsBase RemoveJoyMouse 10E 0
