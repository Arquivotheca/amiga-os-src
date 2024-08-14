/* printer.c
 *
 */

#include "multiview.h"

/*****************************************************************************/

/* the printer i/o routines we use */
union printerIO *CreatePrtReq (struct GlobalData *gd)
{
    union printerIO *pio;
    struct MsgPort *mp;

    if (mp = CreateMsgPort ())
    {
	if (pio = (union printerIO *) CreateIORequest (mp, sizeof (union printerIO)))
	    return (pio);
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

void PrintObject (struct GlobalData *gd, LONG mode)
{
    struct TagItem tags[2];
    struct EasyStruct ez;
    struct dtPrint dtp;

    PrintMenus (gd, FALSE);

    /* Build status window */
    ez.es_StructSize   = sizeof (struct EasyStruct);
    ez.es_Flags        = 0;
    ez.es_Title        = GetString (gd, MV_TITLE_MULTIVIEW);
    ez.es_TextFormat   = GetString (gd, MV_TITLE_PRINTING);
    ez.es_GadgetFormat = GetString (gd, MV_LABEL_ABORT);

    if (gd->gd_PIO = CreatePrtReq (gd))
    {
	if (OpenDevice ("printer.device", 0, (struct IORequest *) gd->gd_PIO, 0) == 0)
	{
	    /* Prepare the tags */
	    tags[0].ti_Tag   = TAG_IGNORE;
	    tags[0].ti_Data  = (ULONG) gd->gd_Window->RPort;
	    tags[1].ti_Tag   = TAG_DONE;

	    if (gd->gd_Flags & GDF_BITMAP)
		tags[0].ti_Tag = DTA_RastPort;

	    /* Fill out the boopsi message */
	    dtp.MethodID     = DTM_PRINT;
	    dtp.dtp_GInfo    = NULL;
	    dtp.dtp_PIO      = gd->gd_PIO;
	    dtp.dtp_AttrList = tags;

	    /* Start the print here... */
	    if (PrintDTObjectA (gd->gd_DataObject, gd->gd_Window, NULL, &dtp))
	    {
		/* Build the status window */
		gd->gd_PrintWin = BuildEasyRequestArgs (gd->gd_Window, &ez, NULL, NULL);
		return;
	    }
	    CloseDevice ((struct IORequest *) gd->gd_PIO);
	}
	DeletePrtReq (gd, gd->gd_PIO);
    }
    PrintMenus (gd, TRUE);
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

	PrintMenus (gd, TRUE);
    }
}
