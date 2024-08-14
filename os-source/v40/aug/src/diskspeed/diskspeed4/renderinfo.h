/*
 * MKSoft Development Amiga ToolKit V1.0
 *
 * Copyright (c) 1985,86,87,88,89,90 by MKSoft Development
 *
 *                          DiskSpeed v4.2
 *                          ScsiSpeed v4.2
 *                                by
 *                           Michael Sinz
 *
 *            Copyright (c) 1989-1992 by MKSoft Development
 *
 *			MKSoft Development
 *			163 Appledore Drive
 *			Downingtown, PA 19335
 *
 * Yes, this is yet another disk speed testing program, but with a few
 * differences.  It was designed to give the most accurate results of the
 * true disk performance in the system.  For this reason many of
 * DiskSpeed's results may look either lower or higher than current disk
 * performance tests.
 *
 ******************************************************************************
 *									      *
 *	Reading legal mush can turn your brain into guacamole!		      *
 *									      *
 *		So here is some of that legal mush:			      *
 *									      *
 * Permission is hereby granted to distribute this program's source	      *
 * executable, and documentation for non-commercial purposes, so long as the  *
 * copyright notices are not removed from the sources, executable or	      *
 * documentation.  This program may not be distributed for a profit without   *
 * the express written consent of the author Michael Sinz.		      *
 *									      *
 * This program is not in the public domain.				      *
 *									      *
 * Fred Fish is expressly granted permission to distribute this program's     *
 * source and executable as part of the "Fred Fish freely redistributable     *
 * Amiga software library."						      *
 *									      *
 * Permission is expressly granted for this program and it's source to be     *
 * distributed as part of the Amicus Amiga software disks, and the	      *
 * First Amiga User Group's Hot Mix disks.				      *
 *									      *
 ******************************************************************************
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

	UBYTE		WindowTitle;	/* Window title size	*/	/* includes border */
	UBYTE		junk_pad;
};

VOID CleanUp_RenderInfo(struct RenderInfo *);
struct RenderInfo *Get_RenderInfo(struct Screen *);

#endif	/* MKS_RENDERINFO_H */
