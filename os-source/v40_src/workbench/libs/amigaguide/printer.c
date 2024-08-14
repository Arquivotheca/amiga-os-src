/* printer.c
 *
 */

#include "amigaguidebase.h"

/*****************************************************************************/

/* the printer i/o routines we use */
union printerIO *CreatePrtReq (struct AmigaGuideLib *ag)
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

void DeletePrtReq (struct AmigaGuideLib *ag, union printerIO * pio)
{
    struct MsgPort *mp;

    mp = pio->ios.io_Message.mn_ReplyPort;
    DeleteIORequest ((struct IORequest *) pio);
    DeleteMsgPort (mp);
}

/*****************************************************************************/

void PrintObject (struct AmigaGuideLib *ag, struct Client *cl, LONG mode)
{
    struct EasyStruct ez;
    struct dtPrint dtp;

    /* Can't print again */
    if (cl->cl_PIO)
    {
	return;
    }

    /* Build status window */
    ez.es_StructSize   = sizeof (struct EasyStruct);
    ez.es_Flags        = 0;
    ez.es_Title        = GetAmigaGuideStringLVO (ag, TITLE_AMIGAGUIDE);
    ez.es_TextFormat   = GetAmigaGuideStringLVO (ag, TITLE_PRINTING);
    ez.es_GadgetFormat = GetAmigaGuideStringLVO (ag, LABEL_ABORT);

    if (cl->cl_PIO = CreatePrtReq (ag))
    {
	if (OpenDevice ("printer.device", 0, (struct IORequest *) cl->cl_PIO, 0) == 0)
	{
	    /* Fill out the boopsi message */
	    dtp.MethodID     = DTM_PRINT;
	    dtp.dtp_GInfo    = NULL;
	    dtp.dtp_PIO      = cl->cl_PIO;
	    dtp.dtp_AttrList = NULL;

	    /* Start the print here... */
	    if (PrintDTObjectA (cl->cl_DataObject, cl->cl_Window, NULL, &dtp))
	    {
		/* Build the status window */
		cl->cl_PrintWin = BuildEasyRequestArgs (cl->cl_Window, &ez, NULL, NULL);
		return;
	    }
	    CloseDevice ((struct IORequest *) cl->cl_PIO);
	}
	DeletePrtReq (ag, cl->cl_PIO);
    }
}

/*****************************************************************************/

void PrintComplete (struct AmigaGuideLib *ag, struct Client *cl)
{
    if (cl->cl_PIO)
    {
	CloseDevice ((struct IORequest *) cl->cl_PIO);
	DeletePrtReq (ag, cl->cl_PIO);
	cl->cl_PIO = NULL;
    }

    if (cl->cl_PrintWin)
    {
	FreeSysRequest (cl->cl_PrintWin);
	cl->cl_PrintWin = NULL;
    }
}
