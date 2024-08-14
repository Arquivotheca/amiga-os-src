/*
**	$Id: lowlevel_pragmas.h,v 40.6 93/07/30 16:11:56 vertex Exp $
**
**	SAS/C format pragma files.
**
**	Contains private definitions. COMMODORE INTERNAL USE ONLY!
*/

/* "lowlevel.library" */
/*--- functions in V40 or higher (Release 3.1) ---*/

/* CONTROLLER HANDLING */

#pragma libcall LowLevelBase ReadJoyPort 1e 001

/* LANGUAGE HANDLING */

#pragma libcall LowLevelBase GetLanguageSelection 24 00
#ifdef LOWLEVEL_PRIVATE_PRAGMAS
#pragma libcall LowLevelBase SetLanguageSelection 2a 101
#endif

/* KEYBOARD HANDLING */

#pragma libcall LowLevelBase GetKey 30 00
#pragma libcall LowLevelBase QueryKeys 36 1802
#pragma libcall LowLevelBase AddKBInt 3c 9802
#pragma libcall LowLevelBase RemKBInt 42 901

/* SYSTEM HANDLING */

#pragma libcall LowLevelBase SystemControlA 48 901
#ifdef __SASC_60
#pragma tagcall LowLevelBase SystemControl 48 901
#endif

/* TIMER HANDLING */

#pragma libcall LowLevelBase AddTimerInt 4e 9802
#pragma libcall LowLevelBase RemTimerInt 54 901
#pragma libcall LowLevelBase StopTimerInt 5a 901
#pragma libcall LowLevelBase StartTimerInt 60 10903
#pragma libcall LowLevelBase ElapsedTime 66 801

/* VBLANK HANDLING */

#pragma libcall LowLevelBase AddVBlankInt 6c 9802
#pragma libcall LowLevelBase RemVBlankInt 72 901

/* Private LVOs for use by nonvolatile.library */

#ifdef LOWLEVEL_PRIVATE_PRAGMAS
#pragma libcall LowLevelBase KillReq 78 00
#endif
#ifdef LOWLEVEL_PRIVATE_PRAGMAS
#pragma libcall LowLevelBase RestoreReq 7e 00
#endif

/* MORE CONTROLLER HANDLING */

#pragma libcall LowLevelBase SetJoyPortAttrsA 84 9002
#ifdef __SASC_60
#pragma tagcall LowLevelBase SetJoyPortAttrs 84 9002
#endif

/* Reserved entries */

#ifdef LOWLEVEL_PRIVATE_PRAGMAS
#pragma libcall LowLevelBase Reserved2 8a 00
#endif
#ifdef LOWLEVEL_PRIVATE_PRAGMAS
#pragma libcall LowLevelBase Reserved3 90 00
#endif
#ifdef LOWLEVEL_PRIVATE_PRAGMAS
#pragma libcall LowLevelBase Reserved4 96 00
#endif
#ifdef LOWLEVEL_PRIVATE_PRAGMAS
#pragma libcall LowLevelBase Reserved5 9c 00
#endif
#ifdef LOWLEVEL_PRIVATE_PRAGMAS
#pragma libcall LowLevelBase Reserved6 a2 00
#endif
