/*
**	$Id: diskfont_pragmas.h,v 38.0 92/06/18 11:23:09 darren Exp $
**
**	SAS/C format pragma files.
**
**	Contains private definitions. COMMODORE INTERNAL USE ONLY!
*/

/* "diskfont.library" */
#pragma libcall DiskfontBase OpenDiskFont 1e 801
#pragma libcall DiskfontBase AvailFonts 24 10803
/*--- functions in V34 or higher (Release 1.3) ---*/
#pragma libcall DiskfontBase NewFontContents 2a 9802
#pragma libcall DiskfontBase DisposeFontContents 30 901
/*--- functions in V36 or higher (Release 2.0) ---*/
#pragma libcall DiskfontBase NewScaledDiskFont 36 9802
