/*
 * MKSoft Development Amiga ToolKit V1.0
 *
 * Copyright (c) 1985,86,87,88,89,90,91 by MKSoft Development
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

#ifndef	MKS_MKS_LIST_H
#define	MKS_MKS_LIST_H

#include	<exec/types.h>
#include	<exec/lists.h>
#include	<exec/nodes.h>
#include	<graphics/text.h>
#include	<intuition/intuition.h>

/*
 * The DisplayList structure returned by InitListGadget
 *
 * This structure is fully managed by the ListGadget
 * code.  These are the only structure elements that
 * are visible and they are *READ ONLY*
 */
struct	DisplayList
{
	struct	Node	*Selected;	/* These are the only PUBLIC fields... */
};

/*
 * This routine deallocates the DisplayList allocated below...
 * It does not do any unlinking...
 */
void FreeListGadget(struct DisplayList *);

/*
 * This routine allocates and initializes a DisplayList gadget...
 * It will link the gadgets into the gadget list given...
 *
 * Arguments:  TextAttr,**Gadget,TextPen,Highlight,Shadow,Left,Top,Width,Height
 */
struct DisplayList *InitListGadget(struct TextAttr *,struct Gadget **,UBYTE,UBYTE,UBYTE,SHORT,SHORT,SHORT,SHORT);

/*
 * This routine will initialize a fresh list.  It clears Selected,
 * sets TopNum to 0, counts the entries, and displays the list...
 */
void FreshList(struct Window *,struct DisplayList *,struct MinList *);

/*
 * This is the InputEvent filter...
 * Call this routine with each message.  If it returns a pointer to
 * a DisplayList, a message was processed, if it returns NULL, nothing
 * was done with the message and you may process it.
 * The message is NOT ReplyMsg()ed...
 */
struct DisplayList *Check_ListGadget(struct Window *,struct IntuiMessage *);

#endif	/* MKS_MKS_LIST_H */
