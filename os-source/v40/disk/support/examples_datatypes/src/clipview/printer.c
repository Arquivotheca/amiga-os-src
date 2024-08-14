/******************************************************************************
 *
 * COPYRIGHT: Unless otherwise noted, all files are Copyright (c) 1992, 1993
 * Commodore-Amiga, Inc.  All rights reserved.
 *
 * DISCLAIMER: This software is provided "as is".  No representations or
 * warranties are made with respect to the accuracy, reliability, performance,
 * currentness, or operation of this software, and all use is at your own risk.
 * Neither commodore nor the authors assume any responsibility or liability
 * whatsoever with respect to your use of this software.
 *
 ******************************************************************************
 * printer.c
 * Written by David N. Junod
 *
 */

#include "clipview.h"

/*****************************************************************************/

/* the printer i/o routines we use */
union printerIO *CreatePrtReq (struct GlobalData *gd)
{
    union printerIO *pio;
    struct MsgPort *mp;

    if (mp = CreateMsgPort ())
    {
	if (pio = (union printerIO *) CreateIORequest (mp, sizeof (union printerIO)))
	{
	    return (pio);
	}
	DeleteMsgPort (mp);
    }
    return (NULL);
}

/*****************************************************************************/

void DeletePrtReq (struct GlobalData *gd, union printerIO * pio)
{
    struct MsgPort *mp;

    mp = pio->ios.io_Message.mn_ReplyPort;
    DeleteIORequest ((struct IORequest *) pio);
    DeleteMsgPort (mp);
}

/*****************************************************************************/

void PrintObject (struct GlobalData *gd)
{
    struct EasyStruct ez;
    struct dtPrint dtp;

    /* Build status window */
    ez.es_StructSize   = sizeof (struct EasyStruct);
    ez.es_Flags        = 0;
    ez.es_Title        = GetString (gd, TITLE_CLIPVIEW);
    ez.es_TextFormat   = GetString (gd, TITLE_PRINTING);
    ez.es_GadgetFormat = GetString (gd, LABEL_ABORT);

    if (gd->gd_PIO = CreatePrtReq (gd))
    {
	if (OpenDevice ("printer.device", 0, (struct IORequest *) gd->gd_PIO, 0) == 0)
	{
	    /* Fill out the boopsi message */
	    dtp.MethodID     = DTM_PRINT;
	    dtp.dtp_GInfo    = NULL;
	    dtp.dtp_PIO      = gd->gd_PIO;
	    dtp.dtp_AttrList = NULL;

	    /* Start the print here... */
	    if (PrintDTObjectA (gd->gd_DisplayObject, gd->gd_Window, NULL, &dtp))
	    {
		/* Build the status window */
		gd->gd_PrintWin = BuildEasyRequestArgs (gd->gd_Window, &ez, NULL, NULL);
		return;
	    }
	    CloseDevice ((struct IORequest *) gd->gd_PIO);
	}
	DeletePrtReq (gd, gd->gd_PIO);
    }
}

/*****************************************************************************/

void PrintComplete (struct GlobalData *gd)
{
    if (gd->gd_PIO)
    {
	CloseDevice ((struct IORequest *) gd->gd_PIO);
	DeletePrtReq (gd, gd->gd_PIO);
	gd->gd_PIO = NULL;
    }

    if (gd->gd_PrintWin)
    {
	FreeSysRequest (gd->gd_PrintWin);
	gd->gd_PrintWin = NULL;
    }
}
