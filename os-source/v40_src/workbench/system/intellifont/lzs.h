/*
**	$Id: lzs.h,v 38.8 92/06/01 12:40:49 darren Exp $
**
**	Fountain/lzs.h -- replacable strings
**
**	(C) Copyright 1990 Robert R. Burns
**	    All Rights Reserved
*/

/* Language Invariant Strings */

#define  VERSION_TEXT		"38.16"
#define  RESOURCE_FOUNTAIN	"fountain.resource"
#define  DEVICE_ASL		"asl.library"
#define  DEVICE_DISKFONT	"diskfont.library"
#define  DEVICE_GADTOOLS	"gadtools.library"
#define  DEVICE_GRAPHICS	"graphics.library"
#define  DEVICE_ICON		"icon.library"
#define  DEVICE_INTUITION	"intuition.library"
#define  DEVICE_UTILITY		"utility.library"
#define  DOSDEV_TRACKDISK	"\021trackdisk.device"
#define  FONT_TOPAZ		"topaz.font"
#define  ENV_FOUNTAIN		"Env:Sys/Intellifont"
#define  ENV_DISKFONT		"Env:Sys/diskfont"
#define  TOOLARG_VALIDATE	"VALIDATE"
#define  RDARGS_FOUNTAIN	"VALIDATE/S"
#define  RDARGS_FOUNTAINENV	"SIZE/M"
#define  RDARGS_DISKFONTENV	"XDPI/N,YDPI/N,XDOTP/N,YDOTP/N,SYMSET"
#define  VOL_FONTS		"Fonts"
#define  DIR_BULLET		"_Bullet"
#define  DIR_OUTLINES		"_Bullet_Outlines"
#define  FILE_IFFNT		"if.fnt"
#define  FILE_PLUGIN		"plugin.types"
#define  FILE_SCREEN		"screen.types"
#define  FILE_HQ3FNT		"hq3.fnt"
#define  FILE_HQ3UPDT		"hq3updt.types"
#define  FILE_IFSS		"if.ss"
#define  FILE_IFUC		"if.uc"
#define  FILE_FONTINDX		"fontindx.fi"
#define  SUFFIX_FONT		".font"
#define  SUFFIX_OTAG		OTSUFFIX
#define  SUFFIX_TYPE		".type"

#define  SIZEOF_HorizStyle	8
#define  SIZEOF_StrokeWeight	16
#define  SIZEOF_Posture		3

/* Language Dependent Strings */
enum {
    /* Window Titles */
    TITLE_Screen = 1,		/* "Intellifont <VERSION_TEXT>", */
    TITLE_Initializing,		/* "Intellifont Initializing...", */
    TITLE_Validating,		/* "Intellifont Validating...", */
    TITLE_Main,			/* "Intellifont", */
    TITLE_Install,		/* "Intellifont Installing...", */
    TITLE_Modify,		/* "Intellifont: Modify Existing Typefaces", */
    TITLE_Help,			/* "Intellifont Help", */
    TITLE_Validate,		/* "Intellifont Validation", */
    TITLE_Warning,		/* "Intellifont Warning", */
    TITLE_Error,		/* "Intellifont Error", */
    TITLE_EndGame,		/* "Intellifont Termination", */

    /* Requester Titles */
    TITLE_OutlineSource,	/* "Outline Font Source", */
    TITLE_DestinationDrawer,	/* "Destination Font Drawer", */

    /* Gadget Strings */
    GADGET_VanillaIndex,	/* "SACDD", */
    GADGET_OK,			/* "OK", */
    GADGET_ContinueCancel,	/* "Continue|Cancel", */
    GADGET_ConfirmIgnore,	/* "Confirm Abort|Ignore", */

    GADGET_RDirNext,		/* "Fonts: Path Component #%-2d", */
    GADGET_NotFontsPath,	/* "(Not in Fonts: Path)", */
    GADGET_NotValidPath,	/* "(Not a valid path)", */
    GADGET_OutlineSource,	/* "Outline Font Source", */
    GADGET_DestinationDrawer,	/* "Destination Font Drawer", */
    GADGET_SourceTypefaces,	/* "Source Typefaces", */
    GADGET_ExistingFT,		/* "Existing Fonts & Typefaces", */
    GADGET_Install,		/* "Install Marked Typefaces", */
    GADGET_Modify,		/* "Modify Existing Typefaces...", */

    GADGET_ExistingT,		/* "Existing Outline Typefaces", */
    GADGET_Sizes,		/* "Size & Bitmap  ", */
    GADGET_Size,		/* "_Size", */
    GADGET_AddSize,		/* "_Add Size", */
    GADGET_DeleteSize,		/* "_Delete Size", */
    GADGET_CreateBitmap,	/* "_Create Bitmap", */
    GADGET_DeleteBitmap,	/* "_Delete Bitmap", */
    GADGET_DeleteFace,		/* "Delete Typeface", */
    GADGET_PerformChanges,	/* "Perform Changes", */
    GADGET_Cancel,		/* "Cancel", */

    /* Validation */
    VALIDATE_Partial,		/* "The typeface\n" */
				/* "%s\n" */
				/* "is partially installed in the" */
				/* " directory\n" */
				/* "%s\n" */
				/* "Not enough exists to complete" */
				/* " the installation.", */
    VALIDATE_Install,		/* "The typeface\n" */
				/* "%s\n" */
				/* "is partially installed in the" */
				/* " directory\n" */
				/* "%s\n", */
    VALIDATE_Conflict,		/* "The typeface\n" */
				/* "%s\n" */
				/* "is partially installed in the" */
				/* " directory\n" */
				/* "%s\n" */
				/* "There appears to be a conflict with an\n" */
				/* "Amiga font with the same name.\n" */
				/* "Completing the installation may cause\n" */
				/* "loss of that Amiga font.", */

    VALIDATE_DeleteIgnore,	/* "DELETE PARTIAL INSTALLATION|IGNORE", */
    VALIDATE_DelAddIgnore,	/* "DELETE PARTIAL INSTALLATION" */
				/* "|COMPLETE INSTALLATION|IGNORE", */

    /* Warning Strings */
    WARNING_INoDestSpace,	/* "There does not appear to be enough\n" */
				/* "disk space in the destination\n" */
				/* "%s.\n" */
				/* "Approximately %ldK more disk space" */
				/* " is needed.", */
    WARNING_IContinueInstall,	/* "Should the installation process\n" */
				/* "continue to be attempted\n" */
				/* "for the next font\n" */
				/* "%s,\n" */
				/* "or should it be cancelled now?\n", */
    WARNING_IOverwrite,		/* "Installation of this font\n" */
				/* "%s\n" */
				/* "will overwrite the existing outline" */
				/* " font", */
    WARNING_IOverwriteStd,	/* "Installation of this font\n" */
				/* "%s\n" */
				/* "will overwrite the existing font\n\n" */
				/* "NOTE: The existing font is not an\n" */
				/* "      outline font.  You need to\n" */
				/* "      rename it if you want to\n" */
				/* "      have both it and this new\n" */
				/* "      outline font on the system\n" */
				/* "      together\n", */
    WARNING_Hidden,		/* "This font\n" */
				/* "%s\n" */
				/* "exists earlier in the Fonts: assign" */
				/* " path at\n" */
				/* "%s.\n" */
				/* "This installation would be hidden\n" */
				/* "if Fonts: doesn't change.\n", */
    WARNING_Hides,		/* "This font\n" */
				/* "%s\n" */
				/* "exists later in the Fonts: assign" */
				/* " path at\n" */
				/* "%s.\n" */
				/* "This installation would hide that font\n" */
				/* "if Fonts: doesn't change.\n", */
    WARNING_Modify,		/* "Perform the following changes?\n" */
				/* "    %s\n    %s\n    %s\n    %s", */
    WARNING_CreateBMFail,	/* "A bitmap creation has failed, but more\n" */
				/* "bitmap(s) remain to be created.\n" */
				/* "Should creation continue?", */

    /* Arg Strings */
    ARG_Bitmap,			/* "bitmap", */
    ARG_DeleteType,		/* "Delete Typefaces", */
    ARG_UpdateSizes,		/* "Update Size Descriptions", */
    ARG_CreateBitmap,		/* "Create Bitmaps", */
    ARG_DeleteBitmap,		/* "Delete Bitmaps", */

    /* Error Strings */
    ERROR_ISrcEquDest,		/* "The installation cannot proceed.\n" */
				/* "The destination is the same as" */
				/* " the source\n" */
				/* "%s", */
    ERROR_ICreateDirFail,	/* "The installation cannot proceed.\n" */
				/* "The creation of the required" */
				/* " subdirectory\n" */
				/* "%s\n" */
				/* "failed.\n", */
    ERROR_IInternal,		/* "The installation cannot proceed\n" */
				/* "due to an internal error\n" */
				/* "[function %s]\n", */
    ERROR_INotDir,		/* "The installation cannot proceed.\n" */
				/* "The destination specified,\n" */
				/* "%s,\n" */
				/* "is not a directory.\n", */
    ERROR_INoDest,		/* "The installation cannot proceed.\n" */
				/* "The destination specified,\n" */
				/* "%s,\n" */
				/* "does not exist.\n", */
    ERROR_IInternalFAIS,	/* "Installation of this font\n" */
				/* "%s\n" */
				/* "failed due to an internal error\n" */
				/* "[FAIS Loader error %ld]\n", */
    ERROR_IOpenFail,		/* "Installation of this font\n" */
				/* "%s\n" */
				/* "failed.  Could not open the file\n" */
				/* "%s\n", */
    ERROR_ICreateFail,		/* "Installation of this font\n" */
				/* "%s\n" */
				/* "failed.  Could not create the file\n" */
				/* "%s\n", */
    ERROR_IFileRead,		/* "Installation of this font\n" */
				/* "%s\n" */
				/* "failed due to a file read failure\n\n" */
				/* "[file \"%s\", request %ld, actual %ld," */
				/* " error %ld]", */
    ERROR_IFileWrite,		/* "Installation of this font\n" */
				/* "%s\n" */
				/* "failed due to a file write failure\n\n" */
				/* "[file \"%s\", request %ld, actual %ld," */
				/* " error %ld]", */
    ERROR_NoTypefaces,		/* "No Outline Typefaces exist in this\n" */
				/* "Font Drawer to modify", */
    ERROR_NoFaceSelected,	/* "Select a typeface before performing a\n" */
				/* "%s", */
    ERROR_NoSizeSelected,	/* "%s can only be performed\n" */
				/* "on a size already in the" */
				/* " Size & Bitmap list\n", */
    ERROR_NonPositiveSize,	/* "The size field must contain a number\n" */
				/* "greater than zero", */
    ERROR_POpenFont,		/* "Bitmap creation for\n" */
				/* "%s %ld\n" */
				/* "failed due to an internal error\n" */
				/* "[function OpenDiskFont]\n", */
    ERROR_POpenFontMem,		/* "Bitmap creation for\n" */
				/* "%s %ld\n" */
				/* "failed: not enough memory was available", */
    ERROR_POpenFontHunk,	/* "Bitmap creation for\n" */
				/* "%s %ld\n" */
				/* "failed due to an internal error\n" */
				/* "[function OpenDiskFont: # hunks != 1]\n", */
    ERROR_PCreateFail,		/* "Change to font\n" */
				/* "%s %ld\n" */
				/* "failed.  Could not create the file\n" */
				/* "%s\n", */
    ERROR_PFailWrite,		/* "Change to font\n" */
				/* "%s %ld\n" */
				/* "failed due to a file write failure\n\n" */
				/* "[file \"%s\", request %ld, actual %ld," */
				/* " error %ld]", */

    /* EndGame Strings */
    ENDGAME_DuplicateRun,	/* "Intellifont already running.\n" */
				/* "Only one copy may be running at once.", */
    ENDGAME_BadInstallation,	/* "Bullet not correctly installed --"
				/* " Please re-install.\n\n" */
				/* "[file %s missing]", */
    ENDGAME_NoFontsAssign,	/* "An Assign for Fonts: is required\n" */
				/* "by Intellifont\n", */
    ENDGAME_BadFontContents,	/* "Font Contents file\n" */
				/* "%s\n" */
				/* "is corrupt.  Run FixFonts first.", */
    ENDGAME_NoMemory,		/* "Out of Memory\n\n" */
				/* "[%ld bytes unavailable]", */
    ENDGAME_FileRead,		/* "File Read Failure\n\n" */
				/* "[file \"%s\", request %ld, actual %ld," */
				/* " error %ld]", */
    ENDGAME_Internal,		/* "Internal Error\n\n" */
				/* "[function %s]", */
    ENDGAME_InternalNArg,	/* "Internal Error\n\n" */
				/* "[function %s %ld]", */
    ENDGAME_InternalResult,	/* "Internal Error\n\n" */
				/* "[function %s result %ld]", */
    ENDGAME_LibraryError,	/* "Cannot open %s\n" */
				/* "version %ld", */
    ENDGAME_FontMetricError,	/* "Screen too short" */
				/* " or Screen font too tall", */
    ENDGAME_ControlC,		/* "User Break", */

    /* Help Strings */
    HELP_Introduction,		/* Help Septant A0 */
				/* Help Septant A1, */
				/* Help Septant A2, */
				/* Help Septant A3, */
				/* Help Septant A4, */
				/* Help Septant A5, */
				/* Help Septant A6, */
				/* Help Septant A7, */
				/* Help Septant B0, */
				/* Help Septant B1, */
				/* Help Septant B2, */
				/* Help Septant B3, */
				/* Help Septant B4, */
				/* Help Septant B5, */
				/* Help Septant B6, */
				/* Help Septant B7, */

    /* Font Suffixes */
    /*   The sum of the maximum lengths for the three  */
    /*   suffix groups must be <= 5.  Here it is == 5. */
    SUFFIX_HorizStyle		/* "UP", "XP", "XC", "C", */
	= HELP_Introduction+14,	/* "", "SE", "E", "XE", */
    SUFFIX_StrokeWeight		/* "UT", "XT", "T", "XL", */
	= SUFFIX_HorizStyle+	/* "L", "DL", "SL", "", */
	SIZEOF_HorizStyle,	/* "", "SB", "DB", "B", */
				/* "XB", "X", "XX", "UX", */
    SUFFIX_Posture		/* "", "I", "R", */
	= SUFFIX_StrokeWeight+
	SIZEOF_StrokeWeight,
    LZS_Last
	= SUFFIX_Posture+
	SIZEOF_Posture
};

#ifndef  DEFINE_STRING_STORAGE
extern
#endif
char *LzS[]
#ifdef   DEFINE_STRING_STORAGE
	= {
    0,
    /* Window Titles */
    "Intellifont\256 " VERSION_TEXT,
    "Intellifont\256: Initializing...",
    "Intellifont\256: Validating...",
    "Intellifont\256",
    "Intellifont Installing...",
    "Intellifont: Modify Existing Typefaces...",
    "Intellifont Help",
    "Intellifont Validation",
    "Intellifont Warning",
    "Intellifont Error",
    "Intellifont Termination",

    /* Requester Titles */
    "Outline Font Source",
    "Destination Font Drawer",

    /* Gadget Strings */
    "SACDD",
    "OK",
    "Continue|Cancel",
    "Confirm Termination|Ignore",

    "Fonts: Path Component #%-2d",
    "(Not in Fonts: Path)",
    "(Not a valid path)",
    "Outline Font Source",
    "Destination Font Drawer",
    "Source Typefaces",
    "Existing Fonts & Typefaces",
    "Install Marked Typefaces",
    "Modify Existing Typefaces...",

    "Existing Outline Typefaces",
    "Size & Bitmap  ",
    "_Size",
    "_Add Size",
    "_Delete Size",
    "_Create Bitmap",
    "_Delete Bitmap",
    "Delete Typeface",
    "Perform Changes",
    "Cancel",

    /* Validation */
    "The typeface\n"
	"%s\n"
	"is partially installed in the"
	" directory\n"
	"%s\n"
	"Not enough exists to complete"
	" the installation.",
    "The typeface\n"
	"%s\n"
	"is partially installed in the"
	" directory\n"
	"%s\n",
    "The typeface\n"
	"%s\n"
	"is partially installed in the"
	" directory\n"
	"%s\n"
	"There appears to be a conflict with an\n"
	"Amiga font with the same name.\n"
	"Completing the installation may cause\n"
	"loss of that Amiga font.",
    "DELETE PARTIAL INSTALLATION|IGNORE",
    "DELETE PARTIAL INSTALLATION"
	"|COMPLETE INSTALLATION|IGNORE",

    /* Warning Strings */
    "There does not appear to be enough\n"
	"disk space in the destination\n"
	"%s.\n"
	"Approximately %ldK more disk space"
	" is needed.",
    "Should the installation process\n"
	"continue to be attempted\n"
	"for the next font\n"
	"%s,\n"
	"or should it be cancelled now?\n",
    "Installation of this font\n"
	"%s\n"
	"will overwrite the existing outline"
	" font",
    "Installation of this font\n"
	"%s\n"
	"will overwrite the existing font\n\n"
	"NOTE: The existing font is not an\n"
	"      outline font.  You need to\n"
	"      rename it if you want to\n"
	"      have both it and this new\n"
	"      outline font on the system\n"
	"      together\n",
    "This font\n"
	"%s\n"
	"exists earlier in the Fonts: assign"
	" path at\n"
	"%s.\n"
	"This installation would be hidden\n"
	"if Fonts: doesn't change.\n",
    "This font\n"
	"%s\n"
	"exists later in the Fonts: assign"
	" path at\n"
	"%s.\n"
	"This installation would hide that font\n"
	"if Fonts: doesn't change.\n",
    "Perform the following changes?\n"
	"    %s\n    %s\n    %s\n    %s",
    "A bitmap creation has failed, but more\n"
	"bitmap(s) remain to be created.\n"
	"Should creation continue?",

    /* Arg Strings */
    "bitmap",
    "Delete Typefaces",
    "Update Size Descriptions",
    "Create Bitmaps",
    "Delete Bitmaps",

    /* Error Strings */
    "The installation cannot proceed.\n"
	"The destination is the same as"
	" the source\n"
	"%s",
    "The installation cannot proceed.\n"
	"The creation of the required"
	" subdirectory\n"
	"%s\n"
	"failed.\n",
    "The installation cannot proceed\n"
	"due to an internal error\n"
	"[function %s]\n",
    "The installation cannot proceed.\n"
	"The destination specified,\n"
	"%s,\n"
	"is not a directory.\n",
    "The installation cannot proceed.\n"
	"The destination specified,\n"
	"%s,\n"
	"does not exist.\n",
    "Installation of this font\n"
	"%s\n"
	"failed due to an internal error\n"
	"[FAIS Loader error %ld]\n",
    "Installation of this font\n"
	"%s\n"
	"failed.  Could not open the file\n"
	"%s\n",
    "Installation of this font\n"
	"%s\n"
	"failed.  Could not create the file\n"
	"%s\n",
    "Installation of this font\n"
	"%s\n"
	"failed due to a file read failure\n\n"
	"[file \"%s\", request %ld, actual %ld,"
	" error %ld]",
    "Installation of this font\n"
	"%s\n"
	"failed due to a file write failure\n\n"
	"[file \"%s\", request %ld, actual %ld,"
	" error %ld]",
    "No Outline Typefaces exist in this\n"
	"Font Drawer to modify",
    "Select a typeface before performing a\n"
	"%s",
    "%s can only be performed\n"
	"on a size already in the"
	" Size & Bitmap list\n",
    "The size field must contain a number\n"
	"greater than zero",
    "Bitmap creation for\n"
	"%s %ld\n"
	"failed due to an internal error\n"
	"[function OpenDiskFont]\n",
    "Bitmap creation for\n"
	"%s %ld\n"
	"failed: not enough memory was available",
    "Bitmap creation for\n"
	"%s %ld\n"
	"failed due to an internal error\n"
	"[function OpenDiskFont: # hunks != 1]\n",
    "Change to font\n"
	"%s %ld\n"
	"failed.  Could not create the file\n"
	"%s\n",
    "Change to font\n"
	"%s %ld\n"
	"failed due to a file write failure\n\n"
	"[file \"%s\", request %ld, actual %ld,"
	" error %ld]",

    /* EndGame Strings */
    "Intellifont already running.\n"
	"Only one copy may be running at once.",
    "Bullet not correctly installed --"
	" Please re-install.\n\n"
	"[file %s missing]",
    "An Assign for Fonts: is required\n"
	"by Intellifont\n",
    "Font Contents file\n"
	"%s\n"
	"is corrupt.  Run FixFonts first.",
    "Out of Memory\n\n"
	"[%ld bytes unavailable]",
    "File Read Failure\n\n"
	"[file \"%s\", request %ld, actual %ld,"
	" error %ld]",
    "Internal Error\n\n"
	"[function %s]",
    "Internal Error\n\n"
	"[function %s %ld]",
    "Internal Error\n\n"
	"[function %s result %ld]",
    "Cannot open %s\n"
	"version %ld",
    "Screen too short"
	" or Screen font too tall",
    "User Break",

    /* Help Strings */
#include "help.h"

    /* Font Suffixes */
    "UP", "XP", "XC", "C",
	"", "SE", "E", "XE",
    "UT", "XT", "T", "XL",
	"L", "DL", "SL", "",
	"", "SB", "DB", "B",
	"XB", "X", "XX", "UX",
    "", "I", "R"

}
#endif
;
