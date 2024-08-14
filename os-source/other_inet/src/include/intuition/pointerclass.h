#ifndef INTUITION_POINTERCLASS_H
#define INTUITION_POINTERCLASS_H
/*
**  $Id: pointerclass.h,v 39.4 92/06/22 13:02:34 peter Exp $
**
**  'boopsi' pointer class interface
**
**  (C) Copyright 1992 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifndef INTUITION_INTUITION_H
#include <intuition/intuition.h>
#endif

#ifndef UTILITY_TAGITEM_H
#include <utility/tagitem.h>
#endif

/* The following tags are recognized at NewObject() time by
 * pointerclass:
 *
 * POINTERA_BitMap (struct BitMap *) - Pointer to bitmap to
 *	get pointer imagery from.  Bitplane data need not be
 *	in chip RAM.
 * POINTERA_XOffset (LONG) - X-offset of the pointer hotspot.
 * POINTERA_YOffset (LONG) - Y-offset of the pointer hotspot.
 * POINTERA_WordWidth (ULONG) - designed width of the pointer in words
 * POINTERA_XResolution (ULONG) - one of the POINTERXRESN_ flags below
 * POINTERA_YResolution (ULONG) - one of the POINTERYRESN_ flags below
 *
 */

#define POINTERA_Dummy	(TAG_USER + 0x39000)

#define POINTERA_BitMap		(POINTERA_Dummy + 0x01)
#define POINTERA_XOffset	(POINTERA_Dummy + 0x02)
#define POINTERA_YOffset	(POINTERA_Dummy + 0x03)
#define POINTERA_WordWidth	(POINTERA_Dummy + 0x04)
#define POINTERA_XResolution	(POINTERA_Dummy + 0x05)
#define POINTERA_YResolution	(POINTERA_Dummy + 0x06)

/* These are the choices for the POINTERA_XResolution attribute which
 * will determine what resolution pixels are used for this pointer.
 *
 * POINTERXRESN_DEFAULT (ECS-compatible pointer width)
 *	= 70 ns if SUPERHIRES-type mode, 140 ns if not
 *
 * POINTERXRESN_SCREENRES
 *	= Same as pixel speed of screen
 *
 * POINTERXRESN_LORES (pointer always in lores-like pixels)
 *	= 140 ns in 15kHz modes, 70 ns in 31kHz modes
 *
 * POINTERXRESN_HIRES (pointer always in hires-like pixels)
 *	= 70 ns in 15kHz modes, 35 ns in 31kHz modes
 *
 * POINTERXRESN_140NS (pointer always in 140 ns pixels)
 *	= 140 ns always
 *
 * POINTERXRESN_70NS (pointer always in 70 ns pixels)
 *	= 70 ns always
 *
 * POINTERXRESN_35NS (pointer always in 35 ns pixels)
 *	= 35 ns always
 */

#define POINTERXRESN_DEFAULT	0
#define POINTERXRESN_140NS	1
#define POINTERXRESN_70NS	2
#define POINTERXRESN_35NS	3

#define POINTERXRESN_SCREENRES	4
#define POINTERXRESN_LORES	5
#define POINTERXRESN_HIRES	6

/* These are the choices for the POINTERA_YResolution attribute which
 * will determine what vertical resolution is used for this pointer.
 *
 * POINTERYRESN_DEFAULT
 *	= In 15 kHz modes, the pointer resolution will be the same
 *	  as a non-interlaced screen.  In 31 kHz modes, the pointer
 *	  will be doubled vertically.  This means there will be about
 *	  200-256 pointer lines per screen.
 *
 * POINTERYRESN_HIGH
 * POINTERYRESN_HIGHASPECT
 *	= Where the hardware/software supports it, the pointer resolution
 *	  will be high.  This means there will be about 400-480 pointer
 *	  lines per screen.  POINTERYRESN_HIGHASPECT also means that
 *	  when the pointer comes out double-height due to hardware/software
 *	  restrictions, its width would be doubled as well, if possible
 *	  (to preserve aspect).
 *
 * POINTERYRESN_SCREENRES
 * POINTERYRESN_SCREENRESASPECT
 *	= Will attempt to match the vertical resolution of the pointer
 *	  to the screen's vertical resolution.  POINTERYRESN_HIGHASPECT also
 *	  means that when the pointer comes out double-height due to
 *	  hardware/software restrictions, its width would be doubled as well,
 *	  if possible (to preserve aspect).
 *
 */

#define POINTERYRESN_DEFAULT		0
#define POINTERYRESN_HIGH		2
#define POINTERYRESN_HIGHASPECT		3
#define POINTERYRESN_SCREENRES		4
#define POINTERYRESN_SCREENRESASPECT	5

#endif
