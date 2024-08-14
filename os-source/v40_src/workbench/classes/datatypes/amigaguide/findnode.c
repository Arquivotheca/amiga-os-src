/* findnode.c
 *
 */

#include "amigaguidebase.h"
#include <intuition/icclass.h>
#include "hosthandle.h"

/*****************************************************************************/

#define	DB(x)	;

/*****************************************************************************/

Object *ObtainNode (struct AGLib *ag, Class *cl, Object *o, STRPTR name, LONG line, LONG column)
{
    struct DTSpecialInfo *si = (struct DTSpecialInfo *) G (o)->SpecialInfo;
    struct ClientData *cd = INST_DATA (cl, o);
    struct DatabaseData *dbd;
    struct TextAttr *tattr;
    struct HostHandle *dnh;
    struct MinList *mlh;
    struct NodeData *nd;
    struct IBox *domain;
    BOOL found = FALSE;
    struct XRef *xr;
    Object *member;
    Object *ostate;
    STRPTR dbname;
    Object *ndbo;

    DB (kprintf ("obtain '%s'\n", name));

    getdtattrs (ag, o,
		DTA_Domain,	(ULONG *)&domain,
	        DTA_TextAttr,	(ULONG *)&tattr,
	        TAG_DONE);

    dbd = INST_DATA (ag->ag_DatabaseClass, cd->cd_CurrentDatabase);
    stcgfp (cd->cd_WorkPath, name);
    stcgfn (cd->cd_WorkNode, name);
    dnh = NULL;

    /*********************************/
    /* SEARCH THE IMMEDIATE DATABASE */
    /*********************************/
    if ((cd->cd_WorkPath[0] == 0) ||
	((cd->cd_WorkPath[0] != 0) && (Stricmp (cd->cd_WorkPath, dbd->dbd_Name) == 0)))
    {
        /* Search through local database first */
        ostate = (Object *)dbd->dbd_NodeList.mlh_Head;
        while ((member = NextObject (&ostate)) && !found)
        {
            nd = INST_DATA (ag->ag_NodeClass, member);
            if (Stricmp (nd->nd_Name, cd->cd_WorkNode) == 0)
            {
		dnh = dbd->dbd_HH;
                found = TRUE;
            }
        }
    }

    /******************************************/
    /* SEARCH THE GLOBAL CROSS REFERENCE LIST */
    /******************************************/
    if (!found)
    {
	/* Search through global cross reference list */
	mlh = &(ag->ag_Token->agt_XRefList);
	if (xr = (struct XRef *) FindNameI ((struct List *) mlh, name))
	{
	    si->si_TopVert = line = xr->xr_Line;
	    switch (xr->xr_Node.ln_Type)
	    {
		case XR_INCLUDE:
		case XR_MACRO:
		case XR_STRUCT:
		case XR_FIELD:
		case XR_TYPEDEF:
		case XR_DEFINE:
		    strcpy (cd->cd_WorkPath, xr->xr_File);
		    strcpy (cd->cd_WorkNode, "Main");
		    break;

		default:
		    strcpy (cd->cd_WorkPath, xr->xr_File);
		    strcpy (cd->cd_WorkNode, xr->xr_Name);
		    break;
	    }
	    found = TRUE;
	}
    }

    /************************************/
    /* SEARCH THE GLOBAL NODE HOST LIST */
    /************************************/
    if (!found)
    {
	ULONG (*ASM he) (REG (a0) struct Hook * h, REG (a2) VOID * obj, REG (a1) VOID * msg);
	struct DatabaseData *hdbd;
	struct TagItem tags[4];
	struct HostHandle *hh;
	struct opFindHost ofh;
	struct Rectangle rect;
	BOOL search = TRUE;
	struct Node *node;
	Object *no;

	/* Initialize the tag list */
	tags[0].ti_Tag  = AGA_HelpGroup;
	tags[0].ti_Data = cd->cd_HelpGroup;
	tags[1].ti_Tag  = TAG_DONE;

	/* If we have a window, then we have more tags */
	if (cd->cd_Window)
	{
	    rect = *((struct Rectangle *)&cd->cd_Window->LeftEdge);
	    tags[1].ti_Tag  = HTNA_Screen;
	    tags[1].ti_Data = (ULONG) cd->cd_Window->WScreen;
	    tags[2].ti_Tag  = HTNA_Rectangle;
	    tags[2].ti_Data = (ULONG) &rect;
	    tags[3].ti_Tag  = TAG_DONE;
	}

	/* Prepare the message structure */
	memset (&ofh, 0, sizeof (struct opFindHost));
	ofh.MethodID  = HM_FINDNODE;
	ofh.ofh_Attrs = tags;
	ofh.ofh_Node  = name;
	ofh.ofh_Title = name;

	mlh = &ag->ag_HostList;
	if (mlh->mlh_TailPred != (struct Node *) mlh)
	{
	    /* Step through the list of hosts */
	    for (node = (struct Node *) mlh->mlh_Head; (node->ln_Succ && search); node = node->ln_Succ)
	    {
		/* Cast the node to a HostHandle */
		hh = (struct HostHandle *) node;

		/* Get a pointer to the entry point */
		he = hh->hh_Dispatcher.h_Entry;

		/* Get a pointer to the database object */
		hdbd = INST_DATA (ag->ag_DatabaseClass, hh->hh_DB);

		/* Ask the node host if it belongs to it. */
		if ((*he) (hh, hdbd->dbd_Name, &ofh))
		{
		    /* Create the node object. */
		    if (no = newobject (ag, ag->ag_NodeClass, NULL,
					HNA_Name,	(ULONG) ofh.ofh_Node,
					HNA_TOC,	(ULONG) ofh.ofh_TOC,
					HNA_Title,	(ULONG) ofh.ofh_Title,
					HNA_Next,	(ULONG) ofh.ofh_Next,
					HNA_Previous,	(ULONG) ofh.ofh_Prev,
					HNA_Database, 	hh->hh_DB,
					HNA_HostHandle,	hh,
					TAG_DONE))
		    {
			/* Remember the names */
			strcpy (cd->cd_WorkPath, hdbd->dbd_Name);
			strcpy (cd->cd_WorkNode, ofh.ofh_Node);
			dnh = hh;

			/* Show that we found it successfully. */
			found = TRUE;
		    }

		    /* Found it, so don't search anymore */
		    search = FALSE;
		}
	    }
	}
    }

    if (!found)
    {
	/* No other checks */
    }

    /* External database? */
    if (strlen (cd->cd_WorkPath))
    {
	dbname = cd->cd_WorkPath;
	DB (kprintf ("WorkPath='%s'\n", dbname));
    }
    else
    {
	dbd    = INST_DATA (ag->ag_DatabaseClass, cd->cd_CurrentDatabase);
	dbname = dbd->dbd_Name;
	dnh    = dbd->dbd_HH;
	DB (kprintf ("WorkPath=NULL, dbd_Name='%s'\n", dbname));
    }

    /* Attempt to open the database */
    DB (kprintf ("open database '%s', cd=%08lx, dnh=%08lx\n", dbname, cd, dnh));
    if (ndbo = newobject (ag, ag->ag_DatabaseClass, NULL,
			  DBA_Name,		dbname,
			  DBA_ClientData,	cd,
			  DBA_HostHandle,	dnh,
			  TAG_DONE))
    {
        /* Get a pointer to the current database data */
        dbd = INST_DATA (ag->ag_DatabaseClass, ndbo);

        /* Search through the list for the node */
        ostate = (Object *)dbd->dbd_NodeList.mlh_Head;
        while (member = NextObject (&ostate))
        {
            nd = INST_DATA (ag->ag_NodeClass, member);
            if (Stricmp (nd->nd_Name, cd->cd_WorkNode) == 0)
            {
		/* See if we have a command to run on open */
		if (nd->nd_OnOpen)
		{
		    STRPTR buffer;
		    ULONG msize;
		    LONG i = 10;

		    msize = strlen (nd->nd_OnOpen) + strlen (dbd->dbd_FullName) + strlen (nd->nd_Name) + 5;
		    if (buffer = AllocMem (msize, MEMF_PUBLIC))
		    {
			sprintf (buffer, "%s %s %s", nd->nd_OnOpen, dbd->dbd_FullName, nd->nd_Name);
			i = m_sendCmd (ag, buffer);
			FreeMem (buffer, msize);
		    }

		    if (i > 0)
			break;
		}

		ObtainSemaphore (&nd->nd_Lock);
		nd->nd_UseCnt++;
		ReleaseSemaphore (&nd->nd_Lock);

		cd->cd_CurrentNode = member;

		si->si_TopVert  = line;
		si->si_TopHoriz = column;

		if (nd->nd_Object)
		{
		    if (cd->cd_Window)
		    {
			RectFill (&cd->cd_Clear,
				  domain->Left,
				  cd->cd_Top,
				  domain->Left + domain->Width - 1,
				  domain->Top + domain->Height - 1);

		    }

		    /* Connect the object */
#if 0
		    tattr = (tattr) ? tattr : &nd->nd_TextAttr;
#endif
		    setattrs (ag, nd->nd_Object,
				  DTA_TopVert,	line,
				  DTA_TopHoriz,	column,
				  DTA_TextAttr,	tattr,
				  ICA_TARGET,	cd->cd_Glue,
				  TAG_DONE);

		    /* Tell the model who the embedded object is */
		    setattrs (ag, cd->cd_Glue, DBA_EmbeddedObject, nd->nd_Object, TAG_DONE);
		}

		cd->cd_Flags |= AGF_RENDER;

		/* Set the new settings */
		si->si_TopVert  = line;
		si->si_TopHoriz = column;

		DB (kprintf ("return %08lx\n", member));
                return (member);
            }
        }

	DB (kprintf ("couldn't open '%s'\n", name));
	cd->cd_ErrorLevel  = RETURN_WARN;
	cd->cd_ErrorNumber = DTERROR_COULDNT_OPEN;
	cd->cd_ErrorString = (ULONG) name;

	/* Dispose of the object */
	DisposeObject (ndbo);
    }
    else
    {
	DB (kprintf ("couldn't open database '%s'\n", dbname));
    }

    return (NULL);
}

/*****************************************************************************/

VOID ReleaseNode (struct AGLib *ag, Class *cl, Object *o, Object *node)
{
    ULONG (*ASM he) (REG (a0) struct Hook * h, REG (a2) VOID * obj, REG (a1) VOID * msg);
    struct ClientData *cd = INST_DATA (cl, o);
    struct DatabaseData *dbd;
    struct DTSpecialInfo *si;
    struct NodeData *nd;
    struct IBox *domain;

    if (node)
    {
	nd  = INST_DATA (ag->ag_NodeClass, node);
	dbd = INST_DATA (ag->ag_DatabaseClass, nd->nd_Database);

	if (nd->nd_Object)
	{
	    si = (struct DTSpecialInfo *) G(nd->nd_Object)->SpecialInfo;

	    /* Tell the model that there is no embedded object */
	    setattrs (ag, cd->cd_Glue, DBA_EmbeddedObject, NULL, TAG_DONE);

	    /* Remove it from the glue */
	    setattrs (ag, nd->nd_Object, ICA_TARGET, NULL, TAG_DONE);

	    if (cd->cd_Window)
	    {
		getdtattrs (ag, o, DTA_Domain, (ULONG *)&domain, TAG_DONE);
		RectFill (&cd->cd_Clear,
			  domain->Left,
			  cd->cd_Top,
			  domain->Left + domain->Width - 1,
			  domain->Top + domain->Height - 1);
	    }

	    /* Can't remove object while it is still laying out.  Still a window for layout to
	     * to start before we actually remove the object. */
	    while (si->si_Flags & (DTSIF_LAYOUTPROC))
		Delay (2);

	    /* Tell the object it is being removed */
	    DoMethod (nd->nd_Object, DTM_REMOVEDTOBJECT, NULL);
	}

	/* See if we have a command to run on close */
	if (nd->nd_OnClose)
	{
	    STRPTR buffer;
	    ULONG msize;

	    msize = strlen (nd->nd_OnClose) + strlen (dbd->dbd_FullName) + strlen (nd->nd_Name) + 5;
	    if (buffer = AllocMem (msize, MEMF_PUBLIC))
	    {
		sprintf (buffer, "%s %s %s", nd->nd_OnClose, dbd->dbd_FullName, nd->nd_Name);
		m_sendCmd (ag, buffer);
		FreeMem (buffer, msize);
	    }
	}

	ObtainSemaphore (&nd->nd_Lock);
	if (nd->nd_UseCnt)
	    nd->nd_UseCnt--;

	/* SHOULD THIS BE OVER IN NDDISPATCH.C IN THE OM_DISPOSE SECTION??? */
	if ((nd->nd_UseCnt == 0) && (dbd->dbd_HH))
	{
	    struct TagItem tags[2];

	    /* Initialize the tag list */
	    tags[0].ti_Tag  = AGA_HelpGroup;
	    tags[0].ti_Data = cd->cd_HelpGroup;
	    tags[1].ti_Tag  = TAG_DONE;

	    /* Prepare the message */
	    nd->nd_ONM.MethodID  = HM_CLOSENODE;
	    nd->nd_ONM.onm_Attrs = tags;
	    nd->nd_ONM.onm_Node  = nd->nd_Name;
	    nd->nd_ONM.onm_Flags = nd->nd_Flags;

	    /* Get a pointer to the entry point */
	    he = dbd->dbd_HH->hh_Dispatcher.h_Entry;
	    (*he) (dbd->dbd_HH, dbd->dbd_Name, &nd->nd_ONM);

	    /* Clear the buffer */
	    if (nd->nd_Flags & HNF_FREEBUFFER)
		FreeVec (nd->nd_Buffer);
	    nd->nd_Buffer = NULL;
	    nd->nd_BufferLen = 0;
	}

	if (!DoMethod (nd->nd_Database, OM_DISPOSE))
	    ReleaseSemaphore (&nd->nd_Lock);
    }
}
