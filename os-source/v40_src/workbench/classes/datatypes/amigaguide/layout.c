/* layout.c
 *
 */

#include "amigaguidebase.h"
#include "hosthandle.h"

/*****************************************************************************/

#define	DB(x)	;

/*****************************************************************************/

ULONG layoutMethod (struct AGLib *ag, Class * cl, Object * o, ULONG initial)
{
    ULONG (*ASM he) (REG (a0) struct Hook * h, REG (a2) VOID * obj, REG (a1) VOID * msg);
    struct DTSpecialInfo *si = (struct DTSpecialInfo *) G (o)->SpecialInfo;
    struct IBox *domain = NULL;
    struct DatabaseData *dbd;
    struct TextFont *font;
    struct ClientData *cd;
    struct WorkLayout *wl;
    struct RastPort trp;
    struct NodeData *nd;
    struct _Object *ob;
    struct Line *line;
    struct IBox lbox;
    LONG i, j, k, x;
    BOOL wordwrap;
    STRPTR label;
    LONG total;
    BPTR lock;
    BPTR fh;

    /* Get the data pointers */
    cd = INST_DATA (cl, o);

    if (cd->cd_CurrentNode == NULL)
	return 0;

    /* Show that we need to compute the rastports again */
    cd->cd_Flags |= AGF_NEWSIZE;

    /* Get the data pointers */
    nd = INST_DATA (ag->ag_NodeClass, cd->cd_CurrentNode);
    ob = _OBJECT (cd->cd_CurrentNode);
    dbd = INST_DATA (ag->ag_DatabaseClass, nd->nd_Database);

    /* Get the font */
    if (nd->nd_Font)
	font = nd->nd_Font;
    else if (cd->cd_Font)
	font = cd->cd_Font;
    else
	font = cd->cd_GInfo.gi_DrInfo->dri_Font;

    /* Make sure we have a font */
    if (font)
    {
	/* Initialize the temporary rastport */
	InitRastPort (&trp);
	SetFont (&trp, font);

	/* Initialize the control rastport */
	cd->cd_Control = *(&trp);
	SetFont (&cd->cd_Control, cd->cd_GInfo.gi_DrInfo->dri_Font);
	cd->cd_ControlHeight = cd->cd_GInfo.gi_DrInfo->dri_Font->tf_YSize + 4;

	/* Initialize a few text variables */
	cd->cd_LineHeight = font->tf_YSize + 2;

	si->si_VertUnit = (LONG) cd->cd_LineHeight;
	si->si_HorizUnit = 1L;

	/* Get the wrap state */
	wordwrap = ((dbd->dbd_Flags & DBDF_WRAP) || (nd->nd_Flags & HNF_WRAP));

	/* No wrap, then we will need a layer */
	if (!wordwrap)
	    dbd->dbd_Flags |= DBDF_LAYER;
	cd->cd_Flags |= AGF_RENDER;

	/* Clear the active line */
	cd->cd_ActiveLine = NULL;

	/* Clear the old line list */
	while (line = (struct Line *) RemHead ((struct List *) &cd->cd_LineList))
	    freepvec (ag, cd->cd_Pool, line);

	/* Get the domain of the object */
	GetAttr (DTA_Domain, o, (ULONG *) & domain);

	/* Set the state of the controls */
	for (i = 0; i < MAX_CONTROLS; i++)
	{
	    cd->cd_Controls[i].c_Flags |= CF_DISABLED;
	    switch (i)
	    {
		case C_CONTENTS:
		    if (nd->nd_TOC)
			cd->cd_Controls[i].c_Flags &= ~CF_DISABLED;
		    else if (Stricmp (nd->nd_Name, "Main") != 0)
			cd->cd_Controls[i].c_Flags &= ~CF_DISABLED;
		    break;

		case C_INDEX:
		    if (dbd->dbd_Index)
			cd->cd_Controls[i].c_Flags &= ~CF_DISABLED;
		    break;

		case C_HELP:
		    if (dbd->dbd_Help)
			cd->cd_Controls[i].c_Flags &= ~CF_DISABLED;
		    else if (Stricmp (dbd->dbd_Name, MASTER_HELP) != 0)
		    {
			if (lock = LVOLockE (ag, NULL, MASTER_HELP, ACCESS_READ))
			{
			    cd->cd_Controls[i].c_Flags &= ~CF_DISABLED;
			    UnLock (lock);
			}
		    }
		    break;

		case C_RETRACE:
		    if (cd->cd_HistoryList.mlh_TailPred != (struct Node *) &cd->cd_HistoryList)
			cd->cd_Controls[i].c_Flags &= ~CF_DISABLED;
		    break;

		case C_BROWSE_PREV:
		    if (nd->nd_Prev)
		    {
			if (Stricmp (nd->nd_Name, nd->nd_Prev) != 0)
			    cd->cd_Controls[i].c_Flags &= ~CF_DISABLED;
		    }
		    else if (ob != (struct _Object *) dbd->dbd_NodeList.mlh_Head)
			cd->cd_Controls[i].c_Flags &= ~CF_DISABLED;
		    break;

		case C_BROWSE_NEXT:
		    if (nd->nd_Next)
		    {
			if (Stricmp (nd->nd_Name, nd->nd_Next) != 0)
			    cd->cd_Controls[i].c_Flags &= ~CF_DISABLED;
		    }
		    else if (ob != (struct _Object *) dbd->dbd_NodeList.mlh_TailPred)
			cd->cd_Controls[i].c_Flags &= ~CF_DISABLED;
		    break;
	    }
	}

	/* See if we have controls */
	if (cd->cd_Flags & AGF_CONTROLS)
	{
	    /* layout the control panel */
	    GetLabelsExtent (ag, &cd->cd_Control, LABEL_CONTENTS, LABEL_BROWSE_NEXT, &lbox);
	    cd->cd_BoxWidth = (ULONG) lbox.Width;
	    for (i = k = x = 0, cd->cd_Top = domain->Top; i < 6; i++, k++)
	    {
		label = LVOGetAGString (ag, LABEL_CONTENTS + i);
		j = (cd->cd_BoxWidth - TextLength (&cd->cd_Control, label, strlen (label))) / 2;

		cd->cd_Controls[i].c_Left = x;
		cd->cd_Controls[i].c_TLeft = x + j;
		cd->cd_Controls[i].c_Top = cd->cd_Top;
		cd->cd_Controls[i].c_ID = STM_CONTENTS + k;
		cd->cd_Controls[i].c_Label = label;
		cd->cd_Controls[i].c_LabelLen = strlen (label);

		if (i == 2)
		{
		    cd->cd_Controls[i].c_ID = STM_HELP;
		    k--;
		}

		x += (cd->cd_BoxWidth + 2);
		if ((x + cd->cd_BoxWidth > domain->Width) && (i < 5))
		{
		    x = 0;
		    cd->cd_Top += (cd->cd_ControlHeight + 1);
		}
		/* Allow at least one line of text */
		if (cd->cd_Top - domain->Top > domain->Height)
		    i = 6;
	    }
	    cd->cd_Top++;
	    cd->cd_Height = domain->Height - cd->cd_Top - 2;
	    cd->cd_Top += cd->cd_ControlHeight + 2;
	}
	else
	{
	    cd->cd_Top    = domain->Top;
	    cd->cd_Height = domain->Height - cd->cd_Top - 2;
	    cd->cd_ControlHeight = 0;
	}

	if (nd->nd_Object)
	{
	    struct gpLayout gpl;

	    /* show that we've need to be refreshed */
	    cd->cd_Flags |= AGF_SYNC | AGF_LAYOUT;

	    /* Tell the object about it's size. Call the object layout method */
	    setattrs (ag, nd->nd_Object,
		      GA_Left, domain->Left,
		      GA_Top, cd->cd_Top,
		      GA_Width, domain->Width,
		      GA_Height, cd->cd_Height - 1,
		      TAG_DONE);

	    gpl.MethodID = DTM_PROCLAYOUT;
	    gpl.gpl_GInfo = (cd->cd_Window) ? &cd->cd_GInfo : NULL;
	    gpl.gpl_Initial = initial;
	    DoMethodA (nd->nd_Object, (Msg) & gpl);

	    /* Ask the object about its size */
	    getdtattrs (ag, nd->nd_Object,
			DTA_VisibleVert, (ULONG *) & si->si_VisVert,
			DTA_TotalVert, (ULONG *) & si->si_TotVert,
			DTA_VisibleHoriz, (ULONG *) & si->si_VisHoriz,
			DTA_TotalHoriz, (ULONG *) & si->si_TotHoriz,
			TAG_DONE);
	}
	else
	{
	    /* Do we have a node buffer? */
	    ObtainSemaphore (&nd->nd_Lock);

	    /* Empty buffer? */
	    if (nd->nd_Buffer == NULL)
	    {
		/* Are we a node host? */
		if (dbd->dbd_HH)
		{
		    struct TagItem tags[4];
		    struct Rectangle rect;

		    /* Initialize the tag list */
		    tags[0].ti_Tag = AGA_HelpGroup;
		    tags[0].ti_Data = cd->cd_HelpGroup;
		    tags[1].ti_Tag = TAG_DONE;

		    /* If we have a window, then we have more tags */
		    if (cd->cd_Window)
		    {
			rect = *((struct Rectangle *) &cd->cd_Window->LeftEdge);
			tags[1].ti_Tag = HTNA_Screen;
			tags[1].ti_Data = (ULONG) cd->cd_Window->WScreen;
			tags[2].ti_Tag = HTNA_Rectangle;
			tags[2].ti_Data = (ULONG) & rect;
			tags[3].ti_Tag = TAG_DONE;
		    }

		    /* Prepare the message */
		    memset (&nd->nd_ONM, 0, sizeof (struct opNodeIO));

		    nd->nd_ONM.MethodID = HM_OPENNODE;
		    nd->nd_ONM.onm_Attrs = tags;
		    nd->nd_ONM.onm_Node = nd->nd_Name;
		    nd->nd_ONM.onm_Flags = nd->nd_Flags;

		    /* Get a pointer to the entry point */
#if 1
		    he = dbd->dbd_HH->hh_Dispatcher.h_Entry;
#else
		    he = (ULONG (ASM *) (REG (a0) struct Hook *, REG (a2) VOID *, REG (a1) VOID *)) dbd->dbd_HH->hh_Dispatcher.h_Entry;
#endif

		    /* Open the node */
		    if ((*he) ((struct Hook *) dbd->dbd_HH, dbd->dbd_Name, &nd->nd_ONM))
		    {
			if (nd->nd_ONM.onm_Flags & HTNF_DONE)
			{
			    /* They don't have any more data! */
			}
			else if (nd->nd_ONM.onm_DocBuffer)
			{
			    nd->nd_Buffer = nd->nd_ONM.onm_DocBuffer;
			    nd->nd_BufferLen = nd->nd_ONM.onm_BuffLen;
			}
			else if (nd->nd_ONM.onm_FileName)
			{
			    if (fh = Open (nd->nd_ONM.onm_FileName, MODE_OLDFILE))
			    {
				/* Seek to the offset */
				if (Seek (nd->nd_FileHandle, 0L, OFFSET_END) >= 0)
				{
				    if ((nd->nd_BufferLen = Seek (nd->nd_FileHandle, 0L, OFFSET_BEGINNING)) >= 0)
				    {
					if (nd->nd_Buffer = AllocVec (nd->nd_BufferLen + 2, MEMF_CLEAR))
					{
					    /* Read from the source file into the buffer */
					    if (Read (nd->nd_FileHandle, nd->nd_Buffer, nd->nd_BufferLen) == nd->nd_BufferLen)
					    {
						nd->nd_Flags |= HNF_FREEBUFFER;
					    }
					    else
					    {
						/* Couldn't read, so clear it */
						FreeVec (nd->nd_Buffer);
						nd->nd_Buffer = NULL;
					    }
					}
				    }
				}
				Close (fh);
			    }
			}
		    }
		}
		/* Allocate the node buffer */
		else if (nd->nd_Buffer = AllocVec (nd->nd_BufferLen + 2, MEMF_CLEAR))
		{
		    /* Seek to the offset */
		    Seek (nd->nd_FileHandle, nd->nd_Offset, OFFSET_BEGINNING);

		    /* Read from the source file into the buffer */
		    if (Read (nd->nd_FileHandle, nd->nd_Buffer, nd->nd_BufferLen) == nd->nd_BufferLen)
		    {
			nd->nd_Flags |= HNF_FREEBUFFER;
		    }
		    else
		    {
			/* Couldn't read, so clear it */
			FreeVec (nd->nd_Buffer);
			nd->nd_Buffer = NULL;
		    }
		}
	    }

	    /* If we have text to layout, then perform layout */
	    if (nd->nd_Buffer)
	    {
		/* Allocate the work structure */
		if (wl = allocpvec (ag, cd->cd_Pool, sizeof (struct WorkLayout), MEMF_CLEAR | MEMF_PUBLIC))
		{
		    /* Set the domain of the object */
		    wl->wl_View = *domain;

		    /* Initialize the NON-ZERO variables */
		    wl->wl_Text = TRUE;

		    /* Set the initial styles */
		    wl->wl_Style = FS_NORMAL;
		    wl->wl_FgPen = cd->cd_GInfo.gi_DrInfo->dri_Pens[TEXTPEN];
		    wl->wl_BgPen = cd->cd_GInfo.gi_DrInfo->dri_Pens[BACKGROUNDPEN];
		    wl->wl_Justify = JLEFT;

		    /* Initialize the RastPort */
		    InitRastPort (&wl->wl_RPort);

		    /* Set the font */
		    wl->wl_Font = font;

		    /* Set the font */
		    SetFont (&wl->wl_RPort, wl->wl_Font);

		    /* Calculate the amount of space a tab takes up */
		    wl->wl_SpaceWidth = TextLength (&wl->wl_RPort, " ", 1);
		    wl->wl_TabSpace = wl->wl_SpaceWidth * nd->nd_DefTabs;

		    /* Is wordwrap on? */
		    if ((dbd->dbd_Flags & DBDF_WRAP) || (nd->nd_Flags & HNF_WRAP))
			wl->wl_DefWrap = wl->wl_Wrap = TRUE;
		    if ((dbd->dbd_Flags & DBDF_SMARTWRAP) || (nd->nd_Flags & HNF_SMARTWRAP))
			wl->wl_DefWrap = wl->wl_Wrap = wl->wl_DefSmartWrap = wl->wl_SmartWrap = TRUE;

		    /* layout the document */
		    rl_layoutRecurse (ag, cd, dbd, nd, wl, nd->nd_Buffer, nd->nd_BufferLen);

		    /* Free the temporary work data */
		    freepvec (ag, cd->cd_Pool, wl);
		}
	    }

	    /* Release the semaphore */
	    ReleaseSemaphore (&nd->nd_Lock);

	    /* Adjust the line count */
	    total = 0;
	    line = (struct Line *) &cd->cd_LineList;
	    while (line->ln_Link.mln_Succ)
	    {
		if (line->ln_Flags & LNF_LF)
		    total++;
		line = (struct Line *) line->ln_Link.mln_Succ;
	    }

	    /* Make sure everyone knows what we are */
	    si->si_VisVert = cd->cd_VisVert = cd->cd_Height / cd->cd_LineHeight;
	    si->si_TotVert = total;

	    si->si_VisHoriz = cd->cd_VisHoriz = domain->Width;
	    si->si_TotHoriz = cd->cd_TotHoriz;

	    /* Check to see if we are visible */
	    if (si->si_TopVert > si->si_TotVert - si->si_VisVert)
		si->si_TopVert = si->si_TotVert - si->si_VisVert;
	    if (si->si_TopVert < 0)
		si->si_TopVert = 0;
	    if (si->si_TopHoriz > si->si_TotHoriz - si->si_VisHoriz)
		si->si_TopHoriz = si->si_TotHoriz - si->si_VisHoriz;
	    if (si->si_TopHoriz < 0)
		si->si_TopHoriz = 0;

	    /* show that we've need to be refreshed */
	    cd->cd_Flags |= AGF_SYNC | AGF_LAYOUT;
	}
    }
    return (1L);
}
