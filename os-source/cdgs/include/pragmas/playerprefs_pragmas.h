/*****************************************************************************/
/*	This information is CONFIDENTIAL and PROPRIETARY		   **/
/*	Copyright 1991, Commodore International Limited			   **/
/*	All Rights Reserved.						   **/
/*									   **/
/*	Developed for Commodore International by Silent Software, Inc.	   **/
/*****************************************************************************/
/**/
/**/
/* Programs */
/**/
#pragma libcall PlayerPrefsBase DoPlayer 1E 0
#pragma libcall PlayerPrefsBase DoPrefs 24 0
/**/
/**/
/* private routines*/
/**/
/*pragma libcall PlayerPrefsBase PlayerPrefsReserved1 2A 0*/
/*pragma libcall PlayerPrefsBase PlayerPrefsReserved2 30 0*/
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
/* private routines*/
/**/
/*pragma libcall PlayerPrefsBase PlayerPrefsReserved3 8A 0*/
/*pragma libcall PlayerPrefsBase PlayerPrefsReserved4 90 0*/
/*pragma libcall PlayerPrefsBase PlayerPrefsReserved5 96 0*/
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
/* private routines*/
/**/
/*pragma libcall PlayerPrefsBase PlayerPrefsReserved6 E4 0*/
/*pragma libcall PlayerPrefsBase PlayerPrefsReserved7 EA 0*/
/*pragma libcall PlayerPrefsBase PlayerPrefsReserved8 F0 0*/
/*pragma libcall PlayerPrefsBase PlayerPrefsReserved9 F6 0*/
/*pragma libcall PlayerPrefsBase PlayerPrefsReserved10 FC 0*/
/**/
/* Added Misc Routines.*/
/**/
#pragma libcall PlayerPrefsBase SafeDoIO 102 801
/**/
/* JoyMouse Routines*/
/**/
#pragma libcall PlayerPrefsBase InstallJoyMouse 108 0
#pragma libcall PlayerPrefsBase RemoveJoyMouse 10E 0
