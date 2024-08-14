/* nipc.c
 *
 */

#include "graffiti.h"

/*****************************************************************************/

#define	DB(x)	;

/*****************************************************************************/

ULONG ASM FindServer (REG (a0) struct Hook * h, REG (a2) struct Task * tc, REG (a1) struct TagItem * attrs)
{
    struct GlobalData *gd = (struct GlobalData *) h->h_Data;
    STRPTR host;

    /* Get the server name */
    if (attrs)
    {
	if (host = (STRPTR) GetTagData (QUERY_HOSTNAME, NULL, attrs))
	{
	    stcgfp (gd->gd_ServerRealm, host);
	    stcgfn (gd->gd_ServerHost,  host);
	}
	DB (kprintf ("realm=\"%s\", host=\"%s\"\n", gd->gd_ServerRealm, gd->gd_ServerHost));
    }
    else
    {
	DB (kprintf ("no host\n"));
    }

    /* Signal the end */
    Signal (tc, SIGBREAKF_CTRL_E);

    return FALSE;
}

/*****************************************************************************/

ULONG BroadCastMsg (struct GlobalData * gd, STRPTR msg)
{
    struct List *list = &gd->gd_ClientList;
    struct Transaction *trans;
    struct Node *node;
    struct Client *c;
    ULONG cnt = 0;

    /* Show the message locally */
    SetViewWindowAttrs (gd->gd_WinClass, gd->gd_Window, WOA_Title, msg, TAG_DONE);

#if 0
    if (gd->gd_Output)
    {
	sprintf (gd->gd_Message, "\033[1mserver\033[0m> %s\n", msg);
	Write (gd->gd_Output, gd->gd_Message, strlen (gd->gd_Message));
    }
#endif

    /* Make sure there are entries in the list */
    if (list->lh_TailPred != (struct Node *) list)
    {
	/* Step through the list */
	for (node = (struct Node *) list->lh_Head; node->ln_Succ; node = node->ln_Succ)
	{
	    c = (struct Client *) node;

	    if (trans = alloctransaction (gd, TRN_AllocReqBuffer, strlen (msg) + 1, TAG_DONE))
	    {
		/* Initialize the message */
		trans->trans_Command = CMD_MESSAGE;
		strcpy ((char *) trans->trans_RequestData, msg);
		trans->trans_ReqDataActual = strlen (msg) + 1;
		trans->trans_Timeout = 15;

		/* Send the message */
		BeginTransaction (c->c_Entity, gd->gd_HEntity, trans);

		/* Increment the out counter */
		cnt++;
	    }
	}
    }
    return (cnt);
}

/*****************************************************************************/

ULONG BroadCastCommand (struct GlobalData * gd, ULONG cmd, APTR msg, ULONG msize)
{
    struct List *list = &gd->gd_ClientList;
    struct Transaction *trans;
    struct Node *node;
    struct Client *c;
    ULONG cnt = 0;

    if (gd->gd_SEntity)
    {
	/* Allocate the message */
	if (trans = alloctransaction (gd, TRN_AllocReqBuffer, msize, TAG_DONE))
	{
	    /* Initialize the message */
	    trans->trans_Command = cmd;
	    trans->trans_ReqDataActual = msize;
	    trans->trans_Timeout = 15;
	    CopyMem (msg, trans->trans_RequestData, msize);

	    /* Send the message */
	    BeginTransaction (gd->gd_SEntity, gd->gd_HEntity, trans);
	}
    }
    /* Make sure there are entries in the list */
    else if (list->lh_TailPred != (struct Node *) list)
    {
	/* Step through the list */
	for (node = (struct Node *) list->lh_Head; node->ln_Succ; node = node->ln_Succ)
	{
	    c = (struct Client *) node;

	    if (trans = alloctransaction (gd, TRN_AllocReqBuffer, msize, TAG_DONE))
	    {
		/* Initialize the message */
		trans->trans_Command = cmd;
		trans->trans_ReqDataActual = msize;
		trans->trans_Timeout = 15;
		CopyMem (msg, trans->trans_RequestData, msize);

		/* Send the message */
		BeginTransaction (c->c_Entity, gd->gd_HEntity, trans);

		/* Increment the out counter */
		cnt++;
	    }
	}
    }
    return (cnt);
}

/*****************************************************************************/

void SendBitMap (struct GlobalData * gd, struct Client * c)
{
    struct Transaction *trans;
    ULONG i, msize, rsize;
    struct Dimensions d;
    char *ptr;

    /* Initialize the dimension structure */
    d.d_Width = gd->gd_Width;
    d.d_Height = gd->gd_Height;

    /* Calculate the amount of memory needed */
    rsize = RASSIZE (d.d_Width, d.d_Height);
    msize = sizeof (struct Dimensions) + sizeof (struct BitMap) + (rsize * gd->gd_BitMap->Depth) + 8;

    /* Create the message */
    if (trans = alloctransaction (gd, TRN_AllocReqBuffer, msize, TAG_DONE))
    {
	/* Show that we are sending a bitmap */
	trans->trans_Command = CMD_BITMAP;
	trans->trans_Timeout = 15;

	/* Where does the data start... */
	ptr = (char *) trans->trans_RequestData;

	/* Copy the dimensions */
	CopyMem (&d, ptr, sizeof (struct Dimensions));
	ptr += sizeof (struct Dimensions);

	/* Copy the bitmap into the structure */
	CopyMem (gd->gd_BitMap, ptr, sizeof (struct BitMap));
	ptr += sizeof (struct BitMap);

	/* Copy the planes */
	for (i = 0; i < gd->gd_BitMap->Depth; i++)
	{
	    CopyMem (gd->gd_BitMap->Planes[i], ptr, rsize);
	    ptr += rsize;
	}

	/* Remember the total size */
	trans->trans_ReqDataActual = msize;

	/* Send the bitmap */
	BeginTransaction (c->c_Entity, gd->gd_HEntity, trans);
    }
}

/*****************************************************************************/

ULONG BroadCastPoints (struct GlobalData * gd, struct PlotPoints * pp)
{
    struct List *list = &gd->gd_ClientList;
    struct Transaction *trans;
    struct Node *node;
    struct Client *c;
    ULONG cnt = 0;
    ULONG msize;

    if (!pp)
	pp = (struct PlotPoints *) & gd->gd_FGPen;

    /* How much memory do we need? */
    msize = sizeof (struct PlotPointsHeader) + (sizeof (struct Plot) * (pp->pp_NumPlot + 1));

    if (gd->gd_SEntity)
    {
	/* Allocate the message */
	if (trans = alloctransaction (gd, TRN_AllocReqBuffer, msize, TAG_DONE))
	{
	    /* Initialize the message */
	    trans->trans_Command = CMD_PLOTPOINTS;
	    trans->trans_ReqDataActual = msize;
	    trans->trans_Timeout = 15;
	    CopyMem (pp, trans->trans_RequestData, msize);

	    /* Send the message */
	    BeginTransaction (gd->gd_SEntity, gd->gd_HEntity, trans);
	}
    }
    /* Make sure there are entries in the list */
    else if (list->lh_TailPred != (struct Node *) list)
    {
	/* Step through the list */
	for (node = (struct Node *) list->lh_Head; node->ln_Succ; node = node->ln_Succ)
	{
	    /* Cast the node to a Client */
	    c = (struct Client *) node;

	    /* Allocate the message */
	    if (trans = alloctransaction (gd, TRN_AllocReqBuffer, msize, TAG_DONE))
	    {
		/* Increment the out counter */
		cnt++;

		/* Initialize the message */
		trans->trans_Command = CMD_PLOTPOINTS;
		trans->trans_ReqDataActual = msize;
		trans->trans_Timeout = 15;
		CopyMem (pp, trans->trans_RequestData, msize);

		/* Send the message */
		BeginTransaction (c->c_Entity, gd->gd_HEntity, trans);
	    }
	}
    }
    return (cnt);
}

/*****************************************************************************/

ULONG BroadCastTalk (struct GlobalData * gd, STRPTR text)
{
    struct List *list = &gd->gd_ClientList;
    struct Transaction *trans;
    struct Node *node;
    struct Client *c;
    ULONG cnt = 0;
    ULONG msize;

    msize = strlen (text) + 1;

    if (gd->gd_SEntity)
    {
	/* Allocate the message */
	if (trans = alloctransaction (gd, TRN_AllocReqBuffer, msize, TAG_DONE))
	{
	    /* Initialize the message */
	    trans->trans_Command = CMD_TALK;
	    trans->trans_ReqDataActual = msize;
	    trans->trans_Timeout = 15;
	    strcpy ((char *) trans->trans_RequestData, text);

	    /* Send the message */
	    BeginTransaction (gd->gd_SEntity, gd->gd_HEntity, trans);
	}
    }
    /* Make sure there are entries in the list */
    else if (list->lh_TailPred != (struct Node *) list)
    {
	/* Step through the list */
	for (node = (struct Node *) list->lh_Head; node->ln_Succ; node = node->ln_Succ)
	{
	    c = (struct Client *) node;

	    if (trans = alloctransaction (gd, TRN_AllocReqBuffer, msize, TAG_DONE))
	    {
		/* Initialize the message */
		trans->trans_Command = CMD_TALK;
		trans->trans_ReqDataActual = msize;
		trans->trans_Timeout = 15;
		CopyMem (text, trans->trans_RequestData, msize - 1);

		/* Send the message */
		BeginTransaction (c->c_Entity, gd->gd_HEntity, trans);

		/* Increment the out counter */
		cnt++;
	    }
	}
    }
    return (cnt);
}

/*****************************************************************************/

void ShowTransError (struct GlobalData *gd, struct Transaction *trans)
{
    UBYTE tmp[128];

    if (trans->trans_Error)
    {
	switch (trans->trans_Error)
	{
	    case ENVOYERR_ABORTED:
		strcpy (tmp, "This Transaction has been AbortTransaction()'d.");
		break;

	    case ENVOYERR_CANTDELIVER:
		strcpy (tmp, "The library has given up on delivering this Transaction");
		break;

	    case ENVOYERR_ABOUTTOWAIT:
		strcpy (tmp, "The library would have to block on delivering this Transaction (PLANNED)");
		break;

	    case ENVOYERR_REFUSED:
		strcpy (tmp, "The server's library refused this Transaction (because of resource limitations) MIGHT be implemented]");
		break;

	    case ENVOYERR_TIMEOUT:
		strcpy (tmp, "This Transaction has timed out");
		break;

	    default:
		sprintf (tmp, "Unknown error %ld", trans->trans_Error);
		break;
	}

	SetViewWindowAttrs (gd->gd_WinClass, gd->gd_Window, WOA_Title, tmp, TAG_DONE);
    }
}

/*****************************************************************************/

BOOL HandleNIPCEvents (struct GlobalData * gd)
{
    struct Transaction *trans;
    BOOL quit = FALSE;
    struct Client *c;
    UBYTE tmp[128];
    UBYTE name[64];

    while (trans = GetTransaction (gd->gd_HEntity))
    {
	setwindowpointer (gd, gd->gd_Window, WA_BusyPointer, TRUE, TAG_DONE);

	if (trans->trans_SourceEntity != gd->gd_HEntity)
	{
	    switch (trans->trans_Command)
	    {
		case CMD_LOGIN:
		    /* Create a client structure */
		    if (c = AllocVec (sizeof (struct Client), MEMF_CLEAR))
		    {
			/* Initialize the Client structure */
			strcpy (c->c_Name, (char *) trans->trans_RequestData);
			c->c_Node.ln_Name = c->c_Name;

			/* What are we looking for? */
			sprintf (tmp, "%s.%s", gd->gd_Server, trans->trans_RequestData);
			stcgnm (name, (char *) trans->trans_RequestData);

			/* Find the entity so that we can communicate with them */
			c->c_Entity = FindEntity (name, tmp, gd->gd_HEntity, NULL);
			DB (lprintf (gd, "Entity for %s is %08lx.\n", (void *) tmp, (void *) c->c_Entity));

			/* Broadcast arrival message */
			sprintf (gd->gd_Message, "%s arrived.", trans->trans_RequestData);
			BroadCastMsg (gd, gd->gd_Message);

			/* Add the client to our list */
			AddTail (&gd->gd_ClientList, &c->c_Node);

			/* Increment the client count */
			gd->gd_NumClients++;

			/* Send the bitmap to the client */
			SendBitMap (gd, c);
		    }
		    else
			lprintf (gd, "Couldn't allocate memory for new client %s.\n", (void *) trans->trans_RequestData);
		    break;

		case CMD_LOGOUT:
		    /* Find the client structure */
		    if (c = (struct Client *) FindName (&gd->gd_ClientList, trans->trans_RequestData))
		    {
			/* Remove the node from our client list */
			Remove (&c->c_Node);
			gd->gd_NumClients--;

			/* Lose the entity now that we are done with them */
			LoseEntity (c->c_Entity);

			/* Done with the memory, so free it */
			FreeVec (c);

			/* Broadcast departure */
			sprintf (gd->gd_Message, "%s departed.", trans->trans_RequestData);
			BroadCastMsg (gd, gd->gd_Message);
		    }
		    else
		    {
			lprintf (gd, "Couldn't find client.\n", NULL);
		    }
		    break;

		case CMD_BITMAP:
		    {
			char *ptr = (char *) trans->trans_RequestData;
			struct Dimensions *d;
			BOOL error = FALSE;
			struct BitMap *bm;
			ULONG rsize;
			UWORD i, j;

			d = (struct Dimensions *) ptr;
			ptr += sizeof (struct Dimensions);

			bm = (struct BitMap *) ptr;
			ptr += sizeof (struct BitMap);

			/* Set the size */
			gd->gd_Width = d->d_Width;
			gd->gd_Height = d->d_Height;
			rsize = RASSIZE (d->d_Width, d->d_Height);

			/* Allocate the bitmap */
			if (gd->gd_BitMap = AllocVec (sizeof (struct BitMap), MEMF_CLEAR))
			{
			    /* Allocate the planes */
			    j = bm->Depth;
			    InitBitMap (gd->gd_BitMap, j, gd->gd_Width, gd->gd_Height);
			    for (i = 0; (i < j) && (error == FALSE); i++)
			    {
				if (gd->gd_BitMap->Planes[i] = AllocRaster (gd->gd_Width, gd->gd_Height))
				{
				    CopyMem (ptr, gd->gd_BitMap->Planes[i], rsize);
				    ptr += rsize;
				}
				else
				    error = TRUE;
			    }

			    if (error)
			    {
				DeleteDemoData (gd);
			    }
			    else
			    {
				/* Show it */
				BltBitMapRastPort (gd->gd_BitMap, gd->gd_HTop, gd->gd_VTop,
						   gd->gd_Window->RPort,
						   gd->gd_Window->BorderLeft, gd->gd_Window->BorderTop,
						   gd->gd_InnerWidth, gd->gd_InnerHeight, 0xC0);

				/* Compute the maximum size for the window */
				gd->gd_Window->MaxWidth = gd->gd_Screen->WBorLeft + gd->gd_Width + 18;
				gd->gd_Window->MaxHeight = gd->gd_Screen->BarHeight + 1 + gd->gd_Height + 10;

				/* Refresh the visuals */
				NewSizeFunc (gd);
			    }
			}
		    }
		    break;

		case CMD_PLOTPOINTS:
		    {
			struct PlotPoints *pp = (struct PlotPoints *) trans->trans_RequestData;
			UWORD ox1, oy1;
			UWORD ox2, oy2;
			UWORD px, py;
			ULONG i;

			if (pp->pp_NumPlot >= 1024)
			    break;

			if (gd->gd_SEntity == NULL)
			    BroadCastPoints (gd, pp);

			/* Make sure we have the RastPort initialized correctly. */
			gd->gd_RPort.BitMap = gd->gd_BitMap;

			ox1 = gd->gd_Window->RPort->cp_x;
			oy1 = gd->gd_Window->RPort->cp_y;
			ox2 = gd->gd_RPort.cp_x;
			oy2 = gd->gd_RPort.cp_y;

			px = pp->pp_Plot[0].p_X;
			py = pp->pp_Plot[0].p_Y;

			SetAPen (gd->gd_Window->RPort, pp->pp_FGPen);
			SetBPen (gd->gd_Window->RPort, pp->pp_BGPen);
			SetDrMd (gd->gd_Window->RPort, pp->pp_DrMode);
			Move (gd->gd_Window->RPort, gd->gd_Window->BorderLeft + px - gd->gd_HTop, gd->gd_Window->BorderTop + py - gd->gd_VTop);
			Draw (gd->gd_Window->RPort, gd->gd_Window->BorderLeft + px - gd->gd_HTop, gd->gd_Window->BorderTop + py - gd->gd_VTop);

			SetAPen (&gd->gd_RPort, pp->pp_FGPen);
			SetBPen (&gd->gd_RPort, pp->pp_BGPen);
			SetDrMd (&gd->gd_RPort, pp->pp_DrMode);
			Move (&gd->gd_RPort, px, py);
			Draw (&gd->gd_RPort, px, py);

			for (i = 0; i < pp->pp_NumPlot; i++)
			{
			    px = pp->pp_Plot[i].p_X;
			    py = pp->pp_Plot[i].p_Y;
			    Draw (gd->gd_Window->RPort, gd->gd_Window->BorderLeft + px - gd->gd_HTop, gd->gd_Window->BorderTop + py - gd->gd_VTop);
			    Draw (&gd->gd_RPort, px, py);
			}

			SetAPen (gd->gd_Window->RPort, gd->gd_FGPen);
			SetBPen (gd->gd_Window->RPort, gd->gd_BGPen);
			SetDrMd (gd->gd_Window->RPort, gd->gd_DrMode);
			Move (gd->gd_Window->RPort, ox1, oy1);

			SetAPen (&gd->gd_RPort, gd->gd_FGPen);
			SetBPen (&gd->gd_RPort, gd->gd_BGPen);
			SetDrMd (&gd->gd_RPort, gd->gd_DrMode);
			Move (&gd->gd_RPort, ox2, oy2);
		    }
		    break;

		case CMD_MESSAGE:
		    SetViewWindowAttrs (gd->gd_WinClass, gd->gd_Window, WOA_Title, trans->trans_RequestData, TAG_DONE);
#if 0
		    if (gd->gd_Output)
		    {
			sprintf (gd->gd_Message, "\033[1mserver\033[0m> %s\n", trans->trans_RequestData);
			Write (gd->gd_Output, gd->gd_Message, strlen (gd->gd_Message));
		    }
#endif
		    break;

		case CMD_CLEAR:
		    ClearFunc (gd, (UBYTE) trans->trans_RequestData);

		    if (gd->gd_SEntity == NULL)
			BroadCastCommand (gd, CMD_CLEAR, &trans->trans_RequestData, trans->trans_ReqDataActual);

		    break;

		case CMD_TALK:
		    TalkFunc (gd, trans->trans_RequestData, trans->trans_ReqDataActual - 1);

		    if (gd->gd_SEntity == NULL)
			BroadCastTalk (gd, trans->trans_RequestData);
		    break;
	    }

	    /* Reply to the Transaction */
	    ReplyTransaction (trans);
	}
	else
	{
	    if (trans->trans_Command == CMD_LOGOUT)
		gd->gd_Going = FALSE;

	    /* Show error message, if there is one. */
	    ShowTransError (gd, trans);

	    /* Free the Transaction */
	    FreeTransaction (trans);
	}

	/* Clear the window pointer */
	setwindowpointer (gd, gd->gd_Window, WA_Pointer, NULL, TAG_DONE);
    }

    return quit;
}
