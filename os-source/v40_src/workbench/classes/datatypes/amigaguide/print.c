/* print.c
 *
 */

#include "amigaguidebase.h"

#define	DL(x)	;

/*****************************************************************************/

ULONG dumpNode (struct AGLib * ag, ULONG mode, APTR oh, Object * o, struct ClientData * cd)
{
    struct TextFont *font = NULL;
    struct NodeData *nd;
    LONG pstatus = 0L;
    ULONG retval = 0L;
    struct Line *line;
    UBYTE spaces[80];
    UBYTE attrs[80];
    ULONG style = ~0;
    BYTE fgpen = ~0;
    BYTE bgpen = ~0;
    UBYTE tmp[16];
    ULONG tspace;
    ULONG bsig;
    ULONG len;
    UWORD lx;
    UWORD i;
    UWORD j;

    struct Preferences *prefs;
    struct PrinterData *pd;
    union printerIO *pio;

    cd->cd_Flags |= AGF_PRINTING;

    if (cd->cd_CurrentNode)
    {
	nd = INST_DATA (ag->ag_NodeClass, cd->cd_CurrentNode);

	if (mode == 255)
	{
	    pio = (union printerIO *) oh;
	    pd = (struct PrinterData *) pio->iodrp.io_Device;
	    prefs = &pd->pd_Preferences;
	}

	/* Initialize the tab line */
	for (j = 0; j < 80; j++)
	    spaces[j] = ' ';

	if (nd->nd_Font)
	    font = nd->nd_Font;
	else if (cd->cd_Font)
	    font = cd->cd_Font;

	/* Compute the tab size */
	tspace = (ULONG) (((font == NULL) || (font->tf_CharSpace && font->tf_CharKern)) ? 8 : font->tf_XSize);

	/* Initialize variables */
	lx = 0;
	i = 1;

	line = (struct Line *) cd->cd_LineList.mlh_Head;
	while (line->ln_Link.mln_Succ && (pstatus == 0L) && !(bsig = CheckSignal (SIGBREAKF_CTRL_C)))
	{
	    /* Handle TAB spacing */
	    if (line->ln_Box.Left > lx)
	    {
		/* Spaces from previous character divided by size of space */
		j = (line->ln_Box.Left - lx) / tspace;
		spaces[j] = 0;

		len = strlen (spaces);
		if (mode == DTWM_IFF)
		    pstatus = (WriteChunkBytes ((struct IFFHandle *) oh, spaces, len) == len) ? 0 : 1;
		else if (mode == DTWM_RAW)
		    pstatus = (Write ((BPTR) oh, spaces, len) == len) ? 0 : 1;
		else if (mode == 255)
		{
		    pio->ios.io_Length = -1;
		    pio->ios.io_Data = (APTR) spaces;
		    pio->ios.io_Command = CMD_WRITE;
		    DoIO ((struct IORequest *) pio);
		    pstatus = (LONG) pio->ios.io_Error;
		}
		spaces[j] = ' ';
	    }

	    /* Handle attribute changes */
	    if ((pstatus == 0L) &&
		((style != line->ln_Style) || (fgpen != line->ln_FgPen) || (bgpen != line->ln_BgPen)))
	    {
		/* Set font attributes */
		style = line->ln_Style;
		strcpy (attrs, "\033[0m");
		if (style & FSF_BOLD)
		    strcat (attrs, "\033[1m");
		if (style & FSF_ITALIC)
		    strcat (attrs, "\033[3m");
		if (style & FSF_UNDERLINED)
		    strcat (attrs, "\033[4m");

		/* Set foreground color attributes */
		fgpen = line->ln_FgPen;
		sprintf (tmp, "\033[3%ldm", (LONG) fgpen);
		strcat (attrs, tmp);

		/* Set background color attributes */
		bgpen = line->ln_BgPen;
		sprintf (tmp, "\033[4%ldm", (LONG) bgpen);
		strcat (attrs, tmp);

		len = strlen (attrs);
		if (mode == DTWM_IFF)
		    pstatus = (WriteChunkBytes ((struct IFFHandle *) oh, attrs, len) == len) ? 0 : 1;
		else if (mode == DTWM_RAW)
		    pstatus = (Write ((BPTR) oh, attrs, len) == len) ? 0 : 1;
		else if (mode == 255)
		{
		    pio->ios.io_Length = -1;
		    pio->ios.io_Data = (APTR) attrs;
		    pio->ios.io_Command = CMD_WRITE;
		    DoIO ((struct IORequest *) pio);
		    pstatus = (LONG) pio->ios.io_Error;
		}
	    }

	    /* Handle text segments */
	    if (pstatus == 0L)
	    {
		len = line->ln_TextLen;
		if (mode == DTWM_IFF)
		    pstatus = (WriteChunkBytes ((struct IFFHandle *) oh, line->ln_Text, len) == len) ? 0 : 1;
		else if (mode == DTWM_RAW)
		    pstatus = (Write ((BPTR) oh, line->ln_Text, len) == len) ? 0 : 1;
		else if (mode == 255)
		{
		    pio->ios.io_Length = line->ln_TextLen;
		    pio->ios.io_Data = (APTR) line->ln_Text;
		    pio->ios.io_Command = CMD_WRITE;
		    DoIO ((struct IORequest *) pio);
		    pstatus = (LONG) pio->ios.io_Error;
		}

		lx = line->ln_Box.Left + line->ln_Box.Width;

		/* Did we encounter a line feed? */
		if ((pstatus == 0L) && (line->ln_Flags & LNF_LF))
		{
		    len = 1;
		    if (mode == DTWM_IFF)
			pstatus = (WriteChunkBytes ((struct IFFHandle *) oh, "\n", len) == len) ? 0 : 1;
		    else if (mode == DTWM_RAW)
			pstatus = (Write ((BPTR) oh, "\n", len) == len) ? 0 : 1;
		    else if (mode == 255)
		    {
			pio->ios.io_Length = 1;
			pio->ios.io_Data = (APTR) "\n";
			pio->ios.io_Command = CMD_WRITE;
			DoIO ((struct IORequest *) pio);
			pstatus = (LONG) pio->ios.io_Error;
		    }

		    lx = 0;
		    i++;

		    /* If we are going to the printer, then do a form feed when we reach the line count. */
		    if (mode == 255)
		    {
			if ((i > prefs->PaperLength) && (pstatus == 0L))
			{
			    pio->ios.io_Length = 1;
			    pio->ios.io_Data = (APTR) "";
			    pio->ios.io_Command = CMD_WRITE;
			    DoIO ((struct IORequest *) pio);
			    pstatus = (LONG) pio->ios.io_Error;
			    i = 1;
			}
		    }
		}
	    }
	    line = (struct Line *) line->ln_Link.mln_Succ;
	}

	/* Show that we tried to print */
	retval = (ULONG) pstatus;
    }

    cd->cd_Flags &= ~AGF_PRINTING;

    return retval;
}

/*****************************************************************************/

BOOL writeObject (struct AGLib *ag, APTR handle, Object * o, struct ClientData *cd, struct GadgetInfo * gi, LONG mode)
{
    struct IFFHandle *iff = (struct IFFHandle *) handle;
    BPTR fh = (BPTR) handle;
    BOOL success = FALSE;
    struct IBox *sel;

    GetAttr (DTA_SelectDomain, o, (ULONG *) & sel);

    if (sel == NULL)
    {
	if (mode == DTWM_IFF)
	{
	    if (OpenIFF (iff, IFFF_WRITE) == 0)
	    {
		if (PushChunk (iff, ID_FTXT, ID_FORM, IFFSIZE_UNKNOWN) == 0)
		{
		    if (PushChunk (iff, 0, ID_CHRS, IFFSIZE_UNKNOWN) == 0)
		    {
			if (dumpNode (ag, mode, (APTR) iff, o, cd) >= 0)
			{
			    PopChunk (iff);
			    success = TRUE;
			}
		    }
		    PopChunk (iff);
		}
		CloseIFF (iff);
	    }
	}
	else if (mode == DTWM_RAW)
	{
	    if (dumpNode (ag, mode, (APTR) fh, o, cd) >= 0)
		success = TRUE;
	}
    }
    else
    {
	ULONG s = MIN (cd->cd_AWord, cd->cd_EWord);
	ULONG e = MAX (cd->cd_AWord, cd->cd_EWord);
	ULONG len;

	len = e - s;

	if (mode == DTWM_IFF)
	{
	    if (OpenIFF (iff, IFFF_WRITE) == 0)
	    {
		if (PushChunk (iff, ID_FTXT, ID_FORM, IFFSIZE_UNKNOWN) == 0)
		{
		    if (PushChunk (iff, 0, ID_CHRS, len) == 0)
			if (WriteChunkBytes (iff, (STRPTR) s, len) == len)
			    if (PopChunk (iff) == 0)
				success = TRUE;
		    PopChunk (iff);
		}
		CloseIFF (iff);
	    }
	}
	else if (mode == DTWM_RAW)
	{
	    if (Write (fh, (STRPTR) s, len) == len)
		success = TRUE;
	}

	DoMethod (o, DTM_CLEARSELECTED, gi);
    }

    if (!success)
	DisplayBeep (NULL);

    return (success);
}

/*****************************************************************************/

ULONG writeMethod (struct AGLib *ag, Class * cl, Object * o, struct dtWrite * msg)
{
    struct ClientData *cd = INST_DATA (cl, o);
    struct IFFHandle *iff;
    BOOL success = FALSE;

    if (msg->dtw_Mode == DTWM_IFF)
    {
	if (iff = AllocIFF ())
	{
	    if (iff->iff_Stream = msg->dtw_FileHandle)
	    {
		InitIFFasDOS (iff);
		success = writeObject (ag, iff, o, cd, msg->dtw_GInfo, DTWM_IFF);
	    }
	    FreeIFF (iff);
	}
    }
    else if (msg->dtw_Mode == DTWM_RAW)
    {
	success = writeObject (ag, (APTR) msg->dtw_FileHandle, o, cd, msg->dtw_GInfo, DTWM_RAW);
    }

    return ((ULONG) success);
}
/*****************************************************************************/

ULONG copyMethod (struct AGLib * ag, Class * cl, Object * o, struct dtGeneral * msg)
{
    struct ClientData *cd = INST_DATA (cl, o);
    struct IFFHandle *iff;
    BOOL success = FALSE;

    if (iff = AllocIFF ())
    {
	if (iff->iff_Stream = (ULONG) OpenClipboard (0L))
	{
	    InitIFFasClip (iff);
	    success = writeObject (ag, iff, o, cd, msg->dtg_GInfo, DTWM_IFF);
	    CloseClipboard ((struct ClipboardHandle *) iff->iff_Stream);
	}
	FreeIFF (iff);
    }
    return ((ULONG) success);
}
