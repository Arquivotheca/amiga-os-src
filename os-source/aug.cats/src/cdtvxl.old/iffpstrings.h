#ifndef TEXTTABLE_H
#define TEXTTABLE_H


/****************************************************************************/


/* This file was created automatically by CatComp.
 * Do NOT edit by hand!
 */


#include <exec/types.h>


/****************************************************************************/


#ifdef IFFP_MODULES
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

#endif /* IFFP_MODULES */


/****************************************************************************/


#ifdef STRINGARRAY

struct AppString
{
    LONG   as_ID;
    STRPTR as_Str;
};

struct AppString AppStrings[] =
{
#ifdef IFFP_MODULES
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
#endif /* IFFP_MODULES */
};


#endif /* STRINGARRAY */


/****************************************************************************/


#endif /* TEXTTABLE_H */
