#include <exec/lists.h>
#include <exec/nodes.h>
#include <dos/rdargs.h>
#include <utility/tagitem.h>
#include <libraries/gadtools.h>
#include <graphics/modeid.h>
#include <clib/dos_protos.h>
#include <clib/graphics_protos.h>
#include <clib/gadtools_protos.h>
#include "db_tm.h"
#include <string.h>
#include <stdio.h>

/* BestID() reads a list of arguments from a string gadget, and builds a TagList
 * to pass to BestModeID().
 *
 * Returns the BestModeID to the caller, and highlights the ID in the listview.
 */

ULONG BestID(struct TMData *TMData)
{
	#define TEMPLATE "MUST/N,MUSTNOT/N,V/N,W/N,H/N,DW/N,DH/N,D/N,MID/N,SID/N,RBITS/N,GBITS/N,BBITS/N"
	#define OPT_MUST	0
	#define OPT_MUSTNOT	1
	#define OPT_VP		2
	#define OPT_W		3
	#define OPT_H		4
	#define OPT_DW		5
	#define OPT_DH		6
	#define OPT_D		7
	#define OPT_MID		8 
	#define OPT_SID		9
	#define OPT_RBITS	10
	#define OPT_GBITS	11
	#define OPT_BBITS	12
	#define OPT_COUNT	14

	ULONG ID = INVALID_ID;
	LONG result[OPT_COUNT];
	LONG *val;
	struct RDArgs *rdargs, *MyRdArgs;
	struct TagItem ti[OPT_COUNT+1];
	struct TagItem *next = ti;
	UBYTE instring[256];
	int i;

	if (MyRdArgs = AllocDosObjectTags(DOS_RDARGS, TAG_DONE))
	{
		int len;
		len = strlen(((struct StringInfo *)gadget_BESTMODE->SpecialInfo)->Buffer);
		strcpy(instring, ((struct StringInfo *)gadget_BESTMODE->SpecialInfo)->Buffer);
		instring[len++] = 10;	/* LF */
		instring[len] = 0;
		MyRdArgs->RDA_Source.CS_Buffer = instring;
		MyRdArgs->RDA_Source.CS_Length = len;
		MyRdArgs->RDA_DAList = NULL;
	}

	for (i=0; i<OPT_COUNT;ti[i].ti_Tag = TAG_DONE, result[i] = NULL, i++);
	if (MyRdArgs && (rdargs = ReadArgs(TEMPLATE, result, MyRdArgs)))
	{
		if (val = (LONG *)result[OPT_MUST])
		{
			next->ti_Tag = BIDTAG_DIPFMustHave;
			next->ti_Data = *val;
			next++;
		}
		if (val = (LONG *)result[OPT_MUSTNOT])
		{
			next->ti_Tag = BIDTAG_DIPFMustNotHave;
			next->ti_Data = *val;
			next++;
		}
		if (val = (LONG *)result[OPT_VP])
		{
			next->ti_Tag = BIDTAG_ViewPort;
			next->ti_Data = *val;
			next++;
		}
		if (val = (LONG *)result[OPT_W])
		{
			next->ti_Tag = BIDTAG_NominalWidth;
			next->ti_Data = *val;
			next++;
		}
		if (val = (LONG *)result[OPT_H])
		{
			next->ti_Tag = BIDTAG_NominalHeight;
			next->ti_Data = *val;
			next++;
		}
		if (val = (LONG *)result[OPT_DW])
		{
			next->ti_Tag = BIDTAG_DesiredWidth;
			next->ti_Data = *val;
			next++;
		}
		if (val = (LONG *)result[OPT_DH])
		{
			next->ti_Tag = BIDTAG_DesiredHeight;
			next->ti_Data = *val;
			next++;
		}
		if (val = (LONG *)result[OPT_D])
		{
			next->ti_Tag = BIDTAG_Depth;
			next->ti_Data = *val;
			next++;
		}
		if (val = (LONG *)result[OPT_MID])
		{
			next->ti_Tag = BIDTAG_MonitorID;
			next->ti_Data = ((*val << 16) | 0x1000);
			next++;
		}
		if (val = (LONG *)result[OPT_SID])
		{
			next->ti_Tag = BIDTAG_SourceID;
			next->ti_Data = *val;
			next++;
		}
		if (val = (LONG *)result[OPT_RBITS])
		{
			next->ti_Tag = BIDTAG_RedBits;
			next->ti_Data = *val;
			next++;
		}
		if (val = (LONG *)result[OPT_GBITS])
		{
			next->ti_Tag = BIDTAG_GreenBits;
			next->ti_Data = *val;
			next++;
		}
		if (val = (LONG *)result[OPT_BBITS])
		{
			next->ti_Tag = BIDTAG_BlueBits;
			next->ti_Data = *val;
			next++;
		}
		next->ti_Tag = TAG_DONE;
		ID = BestModeIDA(ti);
		FreeArgs(rdargs);
	}
	if (MyRdArgs)
	{
		FreeArgs(MyRdArgs);
		FreeDosObject(DOS_RDARGS, MyRdArgs);
	}

	/* Now find the ID in the list */	
	if (ID == INVALID_ID)
	{
		DisplayBeep(window_GFXDATAB->WScreen);
	}
	else
	{
		struct List *list;
		struct Node *node;
		ULONG ordinal = 0;
		BOOL found = FALSE;

		sprintf(instring, "0x%08lx", ID);
		GT_GetGadgetAttrs(gadget_MODEIDS, window_GFXDATAB, NULL,
		                  GTLV_Labels, (ULONG)&list,
		                  TAG_DONE);
		if (list)
		{
			node = list->lh_Head;
			while ((node) && (!found))
			{
				found = (!strcmp(instring, node->ln_Name));
				node = node->ln_Succ;
				ordinal++;
			}
		}
		if (found)
		{
			ordinal--;
			GT_SetGadgetAttrs(gadget_MODEIDS, window_GFXDATAB, NULL,
			                  GTLV_Selected, ordinal,
			                  GTLV_Top, ordinal,
			                  TAG_DONE);
		}
	}

	return(ID);
}
