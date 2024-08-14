/*** fonts.c *****************************************************************
 *
 *  all the intuition font stuff that could be extracted
 *
 *  $Id: fonts.c,v 38.2 92/04/02 12:02:49 peter Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (C) 1985, Commodore-Amiga, Inc.
 *  All Rights Reserved
 ****************************************************************************/
#include "intuall.h"

#define D(x)	;
#define DTE(x)	;

/*
 * copies a TextAttr to a TAttrBuff (i.e., copies string, too)
 * can also serve to copy a TAttrBuff.
 */
copyTAttr( ta, tab )
struct TextAttr	*ta;	/* source	*/
struct TAttrBuff	*tab;	/* dest */
{
    tab->tab_TAttr = *ta;
    jstrncpy( tab->tab_NameBuff, ta->ta_Name, MAXFONTNAME );
    tab->tab_TAttr.ta_Name = tab->tab_NameBuff;
}


#if 0
screenFontHeight( screen )
struct Screen	*screen;
{
    int	height;

    return (screen? screen->RastPort.TxHeight:
	fetchIBase()->SysFontPrefs[0].ifp_TABuff.tab_TAttr.ta_YSize);
}
#endif

/** MODIFY: jimm 11/7/85 **/
/* ISetFont() now returns a pointer to any TextFont that it must open.
 * This is so that the font can be closed when no longer needed.
 * See Also: ICloseFont()
 *
 *
 * Note that if font is NULL, this function will NOT CHANGE rp.
 * There is no default.
 */

struct	TextFont *
ISetFont(rp, font)
struct RastPort *rp;
struct TextAttr *font;
{
    struct TextFont *OpenFont();
    struct TextFont *fp = NULL;


	/* jimm: 11/12/85 ** don't set NULL font */
    if (font != NULL && (fp = OpenFont(font)) != NULL)
    {
	SetFont(rp, fp);
	SetSoftStyle(rp, font->ta_Style, 0xFF);
    }

    return (fp);
}

ICloseFont(fp)
struct TextFont *fp;
{
    if (fp) CloseFont(fp);
}

/*** intuition.library/IntuiTextLength ***/

ULONG
IntuiTextLength(itext)
struct IntuiText *itext;
{
    struct RastPort RPort;
    InitRastPort(&RPort);	/* system default font */

#ifdef DUPLICATECODE
    struct	TextAttr	*font;
    struct	TextFont	*tmpfp;
    int	len;

    tmpfp = ISetFont(&RPort, itext->ITextFont);
    /* won't change RPort if NULL font */
    len =TextLength(&RPort, itext->IText, strlen(itext->IText));
    ICloseFont(tmpfp);	/* jimm: 6/26/86: */
    return (len);
#else
    return ( RastPortITextLength(&RPort, itext) );
#endif
}

/* find length for text as it will appear in a specified rast port */
RastPortITextLength(rp, itext)
struct	RastPort *rp;
struct IntuiText *itext;
{
#if 1	/* use RPITE(), unless it's too slow */
    struct TextExtent	textent;

    RastPortITextExtent( rp, itext, &textent );
    return ( textent.te_Width );
#else
    struct	RastPort	clonerp;
    struct	TextFont	*tmpfp;
    int		length;

    cloneRP( &clonerp, rp );

    tmpfp = ISetFont( &clonerp, itext->ITextFont);

    length = TextLength( &clonerp, itext->IText, strlen(itext->IText));
	
    ICloseFont(tmpfp);

    return (length);
#endif
}

/*
 * returns extent, relative to 0,0 of itext,
 * respecting itext offset.
 */
RastPortITextExtent( rp, itext, te )
struct	RastPort 	*rp;
struct IntuiText 	*itext;
struct TextExtent	*te;
{
    struct	RastPort	clonerp;
    struct	TextFont	*tmpfp;

    cloneRP( &clonerp, rp );

    tmpfp = ISetFont( &clonerp, itext->ITextFont);

    DTE( printf("RPITE: text: %s.\n", itext->IText ) );

    TextExtent( &clonerp, itext->IText, strlen( itext->IText ), te );

    DTE( dumpTExtent( "RPITExtent", te ) );

    /* make extent relative to origin of IntuiText,
     * *including* IntuiText internal offsets
     */
    DTE( printf("now offset: leftedge %lx, topedge %lx\n", 
	itext->LeftEdge, itext->TopEdge ));
    offsetRect( &te->te_Extent, itext->LeftEdge, 	
	    itext->TopEdge + clonerp.TxBaseline );

    DTE( dumpTExtent( "RPITExtent after offset", te ) );
	
    ICloseFont(tmpfp);
}
