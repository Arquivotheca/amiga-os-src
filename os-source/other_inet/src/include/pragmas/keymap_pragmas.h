/*
**	$Id: keymap_pragmas.h,v 36.4 90/07/19 16:05:16 darren Exp $
**
**	SAS/C format pragma files.
**
**	Contains private definitions. COMMODORE INTERNAL USE ONLY!
*/

/* "keymap.library" */
/*--- functions in V36 or higher (Release 2.0) ---*/
#pragma libcall KeymapBase SetKeyMapDefault 1e 801
#pragma libcall KeymapBase AskKeyMapDefault 24 00
#pragma libcall KeymapBase MapRawKey 2a A19804
#pragma libcall KeymapBase MapANSI 30 A190805
