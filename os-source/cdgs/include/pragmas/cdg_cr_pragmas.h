/*
**	$Id: cdg_cr_pragmas.h,v 1.2 93/04/15 08:51:03 darren Exp $
**
**	SAS/C format pragma files.
**
**	Contains private definitions. COMMODORE INTERNAL USE ONLY!
*/

/* "cdg.library" */
#pragma libcall CDGBase CDGBegin 1e 801
#pragma libcall CDGBase CDGEnd 24 00
#pragma libcall CDGBase CDGFront 2a 00
#pragma libcall CDGBase CDGBack 30 00
#pragma libcall CDGBase CDGDraw 36 001
#pragma libcall CDGBase CDGChannel 3c 001
#pragma libcall CDGBase CDGPause 42 00
#pragma libcall CDGBase CDGStop 48 00
#pragma libcall CDGBase CDGPlay 4e 001
#pragma libcall CDGBase CDGNextTrack 54 00
#pragma libcall CDGBase CDGPrevTrack 5a 00
#pragma libcall CDGBase CDGFastForward 60 00
#pragma libcall CDGBase CDGRewind 66 00
#pragma libcall CDGBase CDGClearScreen 6c 00
#pragma libcall CDGBase CDGDiskRemoved 72 00
#pragma libcall CDGBase CDGUserPack 78 801
#pragma libcall CDGBase CDGAllocPrefs 7e 00
#pragma libcall CDGBase CDGFreePrefs 84 901
