#ifndef INTERNAL_PHOTOCDCS_H
#define	INTERNAL_PHOTOCDCS_H

/*
**	$VER: photocdcs.h 42.1 (2.11.93)
**	Photo CD player client/server communications
**
**	(C) Copyright 1991-1993 Commodore-Amiga, Inc.
**	    This information is confidential and proprietary
**	    All Rights Reserved.
*/

/*****************************************************************************/

#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifndef EXEC_PORTS_H
#include <exec/ports.h>
#endif

/*****************************************************************************/

/* Photo CD Player command types */
#define	PCDT_NONE		0
#define	PCDT_THUMBNAIL		1	/* Make thumbnail visible */
#define	PCDT_IMAGE		2	/* Make image visible */
#define	PCDT_RESET		3	/* Reset image state to default */
#define	PCDT_PAN		4	/* Make area of image visible */
#define	PCDT_ROTATE		5	/* Rotate the image */
#define	PCDT_ZOOM		6	/* Zoom into the image */
#define	PCDT_MIRROR		7	/* Mirror the image */
#define	PCDT_MARK		8	/* Display the selection box */
#define	PCDT_UNMARK		9	/* Hide the selection box */

/*****************************************************************************/

/* Photo CD Player command message */
struct PCDMessage
{
    struct Message	 pcd_Message;	/* Embedded message */
    ULONG		 pcd_Type;	/* Command type */
    ULONG		 pcd_Result;	/* Command result level */
    ULONG		 pcd_Result2;	/* Command result number */

    union {

	/* Thumbnail arguments */
	struct
	{
	    UWORD	 pcdu_Image;	/* Image number */
        } pcd_ThumbNail;

	/* Image arguments */
	struct
	{
	    UWORD	 pcdu_Image;	/* Image number  */
	    UBYTE	 pcdu_Res;	/* Resolution (one of PHOTOCD_RES_*) */
	    UWORD	 pcdu_X;	/* Position into view */
	    UWORD	 pcdu_Y;
	} pcd_Image;

	/* Reset arguments */

	/* Pan arguments */
	struct
	{
	    UWORD	 pcdu_X;	/* Position into view */
	    UWORD	 pcdu_Y;
	} pcd_Pan;

	/* Rotate arguments */
	struct
	{
	    UWORD	 pcdu_Angle;	/* Absolute counter-clockwise angle */
	} pcd_Rotate;

	/* Zoom arguments */
	struct
	{
	    UWORD	 pcdu_X;	/* Origin */
	    UWORD	 pcdu_Y;
	    UBYTE	 pcdu_Res;	/* Resolution (one of PHOTOCD_RES_*) */
	} pcd_Zoom;

        /* Mirror arguments */

        /* Mark arguments */
        struct {
	    UWORD	 pcdu_X;	/* Selection position */
	    UWORD	 pcdu_Y;
	    UWORD	 pcdu_W;	/* Selection size */
	    UWORD	 pcdu_H;
        } pcd_Mark;

        /* Unmark arguments */
    } pcd_Args;
};

/*****************************************************************************/

#endif

