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
 * Make border structures with the correct box info...
 */

#include	<exec/types.h>
#include	<intuition/intuition.h>

#include	"MakeBoxes.h"

/*
 * Note:  The routines do fill in the '0' values even though
 *	  the array was, most likely, already zero'd
 */

/*
 * This function makes a top-left border array based on the
 * x/y size of the box...
 */
VOID	FillTopLeft_Border(struct Border *bd,SHORT xSize,SHORT ySize)
{
register	SHORT	*xy;

	xy=bd->XY;
	xy[0]=xSize-2;		xy[1]=0;
	xy[2]=0;		xy[3]=0;
	xy[4]=0;		xy[5]=ySize-1;
	xy[6]=1;		xy[7]=ySize-2;
	xy[8]=1;		xy[9]=1;
}

/*
 * This function makes a bottom-right border array based on the
 * x/y size of the box...
 */
VOID	FillBottomRight_Border(struct Border *bd,SHORT xSize,SHORT ySize)
{
register	SHORT	*xy;

	xy=bd->XY;
	xy[0]=1;		xy[1]=ySize-1;
	xy[2]=xSize-1;		xy[3]=ySize-1;
	xy[4]=xSize-1;		xy[5]=0;
	xy[6]=xSize-2;		xy[7]=1;
	xy[8]=xSize-2;		xy[9]=ySize-2;
}
