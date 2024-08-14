#ifndef TEXTTABLE_H
#define TEXTTABLE_H


/****************************************************************************/


/* This file was created automatically by CatComp.
 * Do NOT edit by hand!
 */


#include <exec/types.h>


/****************************************************************************/


#ifdef DISPLAY
#define MSG_IFFP_NOCLIP_D 5000
#define MSG_IFFP_NOCLIP_D_STR "Clipboard open of unit %ld failed\n"

#define MSG_IFFP_NOFILE 5001
#define MSG_IFFP_NOFILE_STR "File not found\n"

#define MSG_IFFP_NOFILE_S 5002
#define MSG_IFFP_NOFILE_S_STR "%s: File open failed\n"

#define MSG_IFFP_NOTOP 5003
#define MSG_IFFP_NOTOP_STR "Parsing error; no top chunk\n"

#define MSG_IFFP_STDFIRST 5010
#define MSG_IFFP_STDFIRST_STR "End of file (not an error)"

#define MSG_IFFP_STD2 5011
#define MSG_IFFP_STD2_STR "End of context (not an error)"

#define MSG_IFFP_STD3 5012
#define MSG_IFFP_STD3_STR "No lexical scope"

#define MSG_IFFP_STD4 5013
#define MSG_IFFP_STD4_STR "Insufficient memory"

#define MSG_IFFP_STD5 5014
#define MSG_IFFP_STD5_STR "Stream read error"

#define MSG_IFFP_STD6 5015
#define MSG_IFFP_STD6_STR "Stream write error"

#define MSG_IFFP_STD7 5016
#define MSG_IFFP_STD7_STR "Stream seek error"

#define MSG_IFFP_STD8 5017
#define MSG_IFFP_STD8_STR "File is corrupt"

#define MSG_IFFP_STD9 5018
#define MSG_IFFP_STD9_STR "IFF syntax error"

#define MSG_IFFP_STD10 5019
#define MSG_IFFP_STD10_STR "Not an IFF file"

#define MSG_IFFP_STD11 5020
#define MSG_IFFP_STD11_STR "Required hook vector missing"

#define MSG_IFFP_STDLAST 5021
#define MSG_IFFP_STDLAST_STR "Return to client"

#define MSG_IFFP_CLIENTERR 5030
#define MSG_IFFP_CLIENTERR_STR "Client error"

#define MSG_IFFP_NOIFFFILE 5031
#define MSG_IFFP_NOIFFFILE_STR "File not found or wrong type"

#define MSG_IFFP_UNKNOWNERR_D 5032
#define MSG_IFFP_UNKNOWNERR_D_STR "Unknown parse error %ld"

#define MSG_IFFP_OSNOMEM 5040
#define MSG_IFFP_OSNOMEM_STR "Not enough memory"

#define MSG_IFFP_OSNOCHIPMEM 5041
#define MSG_IFFP_OSNOCHIPMEM_STR "Not enough chip memory"

#define MSG_IFFP_OSNOMONITOR 5042
#define MSG_IFFP_OSNOMONITOR_STR "monitor not available"

#define MSG_IFFP_OSNOCHIPS 5043
#define MSG_IFFP_OSNOCHIPS_STR "required chipset not available (//)"

#define MSG_IFFP_OSPUBNOTUNIQUE 5044
#define MSG_IFFP_OSPUBNOTUNIQUE_STR "public screen already open"

#define MSG_IFFP_OSUNKNOWNMODE 5045
#define MSG_IFFP_OSUNKNOWNMODE_STR "mode ID is unknown"

#define MSG_IFFP_OSUNKNOWNERR_D 5046
#define MSG_IFFP_OSUNKNOWNERR_D_STR "unknown OpenScreen error %ld"

#define MSG_IFFP_NOMEM 5050
#define MSG_IFFP_NOMEM_STR "Not enough memory\n"

#define MSG_IFFP_NOILBM 5051
#define MSG_IFFP_NOILBM_STR "Not an ILBM\n"

#define MSG_IFFP_NOBMHD 5052
#define MSG_IFFP_NOBMHD_STR "No ILBM.BMHD chunk\n"

#define MSG_IFFP_NOBODY 5053
#define MSG_IFFP_NOBODY_STR "No ILBM.BODY chunk\n"

#define MSG_IFFP_NODISPLAY 5054
#define MSG_IFFP_NODISPLAY_STR "Failed to open display\n"

#define MSG_IFFP_NORASTER 5055
#define MSG_IFFP_NORASTER_STR "Failed to allocate raster\n"

#define MSG_IFFP_NOCOLORS 5056
#define MSG_IFFP_NOCOLORS_STR "No colortable allocated\n"

#define MSG_DI_USEFIRST 5100
#define MSG_DI_USEFIRST_STR "CLI Usage:"

#define MSG_DI_USE2 5101
#define MSG_DI_USE2_STR "Display [from filelist] OR file [file file...] [opt [mlbpaenv][t=n]]"

#define MSG_DI_USE3 5102
#define MSG_DI_USE3_STR "opts: <m>ouseadvance <l>oop <b>ack <p>rint <a>utoscroll"

#define MSG_DI_USE4 5103
#define MSG_DI_USE4_STR "      <e>hb <n>otransb <v>ideo t=seconds"

#define MSG_DI_USE5 5104
#define MSG_DI_USE5_STR "WB Usage: Via Default Tool, Extend Select, or text FileList Project"

#define MSG_DI_USE6 5105
#define MSG_DI_USE6_STR "ToolTypes: Display/Filelist: TIMER=n, Boolean MOUSE,LOOP,PRINT,BACK"

#define MSG_DI_USE7 5106
#define MSG_DI_USE7_STR "FileList: FILELIST=TRUE   Pics: Boolean EHB,AUTOSCROLL,NOTRANSB,VIDEO"

#define MSG_DI_USE8 5107
#define MSG_DI_USE8_STR "Click toggles bar, CTRL-P prints screen"

#define MSG_DI_USELAST 5108
#define MSG_DI_USELAST_STR "Close display in upper left, or CTRL/C.  CTRL/D to break a script"

#define MSG_DI_SCREENTITLE 5120
#define MSG_DI_SCREENTITLE_STR " <- Close when title hidden, or CTRL-C"

#define MSG_DI_NOIFF 5121
#define MSG_DI_NOIFF_STR "Can't AllocIff"

#define MSG_DI_NOMEM 5122
#define MSG_DI_NOMEM_STR "Not enough memory"

#define MSG_DI_NOMEMSIG 5123
#define MSG_DI_NOMEMSIG_STR "Can't alloc memory or signal"

#define MSG_DI_NOIFFPARSE 5124
#define MSG_DI_NOIFFPARSE_STR "Can't open iffparse.library"

#define MSG_DI_NOFILELIST 5125
#define MSG_DI_NOFILELIST_STR "Can't read filelist"

#define MSG_DI_NOILBM 5126
#define MSG_DI_NOILBM_STR "Not a FORM ILBM"

#define MSG_DI_PRESSRET 5127
#define MSG_DI_PRESSRET_STR "Press <RET> to exit:"

#define MSG_DI_HUNTED 5128
#define MSG_DI_HUNTED_STR "ILBM is embedded in complex file"

#define MSG_DI_TOODEEP 5129
#define MSG_DI_TOODEEP_STR "planes, displaying initial planes"

#define MSG_DI_PRTTROUBLE_D 5130
#define MSG_DI_PRTTROUBLE_D_STR "Printer error %ld\n"

#endif /* DISPLAY */


/****************************************************************************/


#ifdef STRINGARRAY

struct AppString
{
    LONG   as_ID;
    STRPTR as_Str;
};

struct AppString AppStrings[] =
{
#ifdef DISPLAY
    {MSG_IFFP_NOCLIP_D,MSG_IFFP_NOCLIP_D_STR},
    {MSG_IFFP_NOFILE,MSG_IFFP_NOFILE_STR},
    {MSG_IFFP_NOFILE_S,MSG_IFFP_NOFILE_S_STR},
    {MSG_IFFP_NOTOP,MSG_IFFP_NOTOP_STR},
    {MSG_IFFP_STDFIRST,MSG_IFFP_STDFIRST_STR},
    {MSG_IFFP_STD2,MSG_IFFP_STD2_STR},
    {MSG_IFFP_STD3,MSG_IFFP_STD3_STR},
    {MSG_IFFP_STD4,MSG_IFFP_STD4_STR},
    {MSG_IFFP_STD5,MSG_IFFP_STD5_STR},
    {MSG_IFFP_STD6,MSG_IFFP_STD6_STR},
    {MSG_IFFP_STD7,MSG_IFFP_STD7_STR},
    {MSG_IFFP_STD8,MSG_IFFP_STD8_STR},
    {MSG_IFFP_STD9,MSG_IFFP_STD9_STR},
    {MSG_IFFP_STD10,MSG_IFFP_STD10_STR},
    {MSG_IFFP_STD11,MSG_IFFP_STD11_STR},
    {MSG_IFFP_STDLAST,MSG_IFFP_STDLAST_STR},
    {MSG_IFFP_CLIENTERR,MSG_IFFP_CLIENTERR_STR},
    {MSG_IFFP_NOIFFFILE,MSG_IFFP_NOIFFFILE_STR},
    {MSG_IFFP_UNKNOWNERR_D,MSG_IFFP_UNKNOWNERR_D_STR},
    {MSG_IFFP_OSNOMEM,MSG_IFFP_OSNOMEM_STR},
    {MSG_IFFP_OSNOCHIPMEM,MSG_IFFP_OSNOCHIPMEM_STR},
    {MSG_IFFP_OSNOMONITOR,MSG_IFFP_OSNOMONITOR_STR},
    {MSG_IFFP_OSNOCHIPS,MSG_IFFP_OSNOCHIPS_STR},
    {MSG_IFFP_OSPUBNOTUNIQUE,MSG_IFFP_OSPUBNOTUNIQUE_STR},
    {MSG_IFFP_OSUNKNOWNMODE,MSG_IFFP_OSUNKNOWNMODE_STR},
    {MSG_IFFP_OSUNKNOWNERR_D,MSG_IFFP_OSUNKNOWNERR_D_STR},
    {MSG_IFFP_NOMEM,MSG_IFFP_NOMEM_STR},
    {MSG_IFFP_NOILBM,MSG_IFFP_NOILBM_STR},
    {MSG_IFFP_NOBMHD,MSG_IFFP_NOBMHD_STR},
    {MSG_IFFP_NOBODY,MSG_IFFP_NOBODY_STR},
    {MSG_IFFP_NODISPLAY,MSG_IFFP_NODISPLAY_STR},
    {MSG_IFFP_NORASTER,MSG_IFFP_NORASTER_STR},
    {MSG_IFFP_NOCOLORS,MSG_IFFP_NOCOLORS_STR},
    {MSG_DI_USEFIRST,MSG_DI_USEFIRST_STR},
    {MSG_DI_USE2,MSG_DI_USE2_STR},
    {MSG_DI_USE3,MSG_DI_USE3_STR},
    {MSG_DI_USE4,MSG_DI_USE4_STR},
    {MSG_DI_USE5,MSG_DI_USE5_STR},
    {MSG_DI_USE6,MSG_DI_USE6_STR},
    {MSG_DI_USE7,MSG_DI_USE7_STR},
    {MSG_DI_USE8,MSG_DI_USE8_STR},
    {MSG_DI_USELAST,MSG_DI_USELAST_STR},
    {MSG_DI_SCREENTITLE,MSG_DI_SCREENTITLE_STR},
    {MSG_DI_NOIFF,MSG_DI_NOIFF_STR},
    {MSG_DI_NOMEM,MSG_DI_NOMEM_STR},
    {MSG_DI_NOMEMSIG,MSG_DI_NOMEMSIG_STR},
    {MSG_DI_NOIFFPARSE,MSG_DI_NOIFFPARSE_STR},
    {MSG_DI_NOFILELIST,MSG_DI_NOFILELIST_STR},
    {MSG_DI_NOILBM,MSG_DI_NOILBM_STR},
    {MSG_DI_PRESSRET,MSG_DI_PRESSRET_STR},
    {MSG_DI_HUNTED,MSG_DI_HUNTED_STR},
    {MSG_DI_TOODEEP,MSG_DI_TOODEEP_STR},
    {MSG_DI_PRTTROUBLE_D,MSG_DI_PRTTROUBLE_D_STR},
#endif /* DISPLAY */
};


#endif /* STRINGARRAY */


/****************************************************************************/


#endif /* TEXTTABLE_H */
