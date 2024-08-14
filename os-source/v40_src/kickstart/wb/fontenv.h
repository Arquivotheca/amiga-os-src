/*
 * $Id: fontenv.h,v 38.1 91/06/24 11:35:34 mks Exp $
 *
 * $Log:	fontenv.h,v $
 * Revision 38.1  91/06/24  11:35:34  mks
 * Initial V38 tree checkin - Log file stripped
 * 
 */

#include <exec/types.h>

#include <graphics/text.h>

#define FNAMESIZE (128)

struct FontPref {
    UBYTE fp_FrontPen;
    UBYTE fp_BackPen;
    UBYTE fp_DrawMode;
    struct TextAttr fp_TextAttr;
    BYTE fp_Name[FNAMESIZE];
};


