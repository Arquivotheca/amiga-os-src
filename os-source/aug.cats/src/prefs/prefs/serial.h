/****************************************************************************
 **									   **
 **  pref.h								   **
 **									   **
 **  Confidential Information: Commodore-Amiga, Inc.			   **
 **  Copyright (c) 1989 Commodore-Amiga, Inc.				   **
 **									   **
 ****************************************************************************
 *
 * SOURCE CONTROL
 *
 * $Header: /usr/gilligan/eric/amiga/src/editors/serial/RCS/pref.h,v 36.0 90/07/18 10:24:31 eric Exp Locker: eric $
 *
 * $Locker: eric $
 *
 ****************************************************************************/

#ifndef PREF_H
#define PREF_H

#include <exec/types.h>
#include <graphics/text.h>

/* ===== Serial.prefs ===================================================== */


#define ENV_NAME "ENV:Sys/serial.prefs"
#define ARC_NAME "ENVARC:Sys/serial.prefs"
#define PRE_DIR  "Sys:Prefs/Presets"
#define PRE_NAME "Serial.pre"
#define REQ_LOAD "Load Serial Preferences"
#define REQ_SAVE "Save Serial Preferences"
#define F_LENGTH (sizeof(PREF))

#define ENV_TITLE	"Serial Preferences"
#define TASK_NAME	"serial_pref_task"
#define TOOL_NAME	"SYS:Prefs/Serial"

/* Parity */
#define PARITY_NONE	0
#define PARITY_EVEN	1
#define PARITY_ODD	2
#define PARITY_MARK	3		/* Future enhancement */
#define PARITY_SPACE	4		/* Future enhancement */

/* Handshaking */
#define HSHAKE_XON	0
#define HSHAKE_RTS	1
#define HSHAKE_NONE	2
    
#define ENV_DEF		{ 0L, 0L, 0L, 0L, 9600L, 512L, 512L, HSHAKE_XON, HSHAKE_XON, PARITY_NONE, 8, 1 }

typedef struct SerialPref {
    LONG  sp_Reserved[4];		/* System reserved */

    ULONG sp_BaudRate;			/* Baud rate */

    ULONG sp_InputBuffer;		/* Input buffer: 0 - 16000 */
    ULONG sp_OutputBuffer;		/* Future: Output: 0 - 16000, def 0 */

    UBYTE sp_InputHandshake;		/* Input handshaking */
    UBYTE sp_OutputHandshake;		/* Future: Output handshaking */

    UBYTE sp_Parity;			/* Parity */

    UBYTE sp_BitsPerChar;		/* I/O bits per character */
    UBYTE sp_StopBits;			/* Stop bits */

} PREF;

#endif
