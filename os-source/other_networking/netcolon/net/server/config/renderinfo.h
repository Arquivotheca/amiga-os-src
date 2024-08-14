/*
 * MKSoft Development Amiga ToolKit V1.0
 *
 * Copyright (c) 1985,86,87,88,89,90 by MKSoft Development
 *
 * $Id: renderinfo.h,v 1.1 91/04/09 16:48:10 dlarson Exp $
 *
 * $Source: HOGNET:src/net/server/config/renderinfo.h,v $
 *
 * $Date: 91/04/09 16:48:10 $
 *
 * $Revision: 1.1 $
 *
 * $Log:	renderinfo.h,v $
 * Revision 1.1  91/04/09  16:48:10  dlarson
 * Initial revision
 * 
 * Revision 1.2  90/05/20  12:18:36  mks
 * New functions to allocate and free RenderInfo structure.
 * These should be used insted of the old FillIn_RenderInfo...
 * Now has a TextAttr in the RenderInfo
 * Now has the screen Width/Height in RenderInfo
 * Now can be passed a screen pointer
 * 
 * Revision 1.1  90/05/09  21:45:05  mks
 * Source now fully under RCS...
 *
 */

/*
 * This file contains the definition of the rendering information
 * for elements on the screen.  This information is used to generate
 * the correct pen colours for items on the screen...
 *
 * Note, that to call this function you MUST have Intuition and Graphics
 * libraries open...
 */

#ifndef	MKS_RENDERINFO_H
#define	MKS_RENDERINFO_H

#include	<exec/types.h>
#include	<graphics/text.h>
#include	<intuition/screens.h>

struct RenderInfo
{
	UBYTE		Highlight;	/* Standard Highlight	*/
	UBYTE		Shadow;		/* Standard Shadow	*/
	UBYTE		TextPen;	/* Requester Text Pen	*/
	UBYTE		BackPen;	/* Requester Back Fill	*/

	UBYTE		WindowTop;	/* Top border of window	*/
	UBYTE		WindowLeft;	/* Left border		*/
	UBYTE		WindowRight;	/* Right border		*/
	UBYTE		WindowBottom;	/* Bottom border	*/

	UBYTE		WindowTitle;	/* Window title size	*/	/* includes border */
	UBYTE		junk_pad;

	SHORT		ScreenWidth;	/* Width of the screen */
	SHORT		ScreenHeight;	/* Height of the screen */

	USHORT		FontSize;	/* Font size for string gadgets	*/

struct	TextFont	*TheFont;	/* Font TextFont */
struct	TextAttr	TextAttr;	/* Font TextAttr */
};

VOID CleanUp_RenderInfo(struct RenderInfo *);
struct RenderInfo *Get_RenderInfo(struct Screen *);

/* This is for old programs...  Should not be used... */
VOID FillIn_RenderInfo(struct RenderInfo *);

#endif	/* MKS_RENDERINFO_H */
