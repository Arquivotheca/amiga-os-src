
/* includes */
#include <string.h>
#include <stdio.h>

/* prototypes */
#include <clib/macros.h>
#include <clib/dos_protos.h>
#include <clib/icon_protos.h>

/* direct ROM interface */
#include <pragmas/dos_pragmas.h>
#include <pragmas/icon_pragmas.h>

/* application includes */
#include "iesrc.h"
#include "iemain.h"
#include "iemisc.h"
#include "texttable.h"
#include "ieutils.h"


extern struct Library *DOSBase;
extern struct Library *IconBase;

extern struct DiskObject Listing;

VOID printFlags(BPTR fh, STRPTR name, SHORT lvl, UBYTE * pref);
VOID parseFlags(BPTR fh, ULONG flags, struct ulongFlags * uf, UBYTE * pref, UBYTE * sufx);
VOID parseGadFlags(BPTR fh, USHORT flags);
VOID ConvertStr(BPTR fh, STRPTR str);


UBYTE *ityp[] =
{
    NULL, "DISK", "DRAWER", "TOOL", "PROJECT", "GARBAGE", "DEVICE", "KICK",
};


LONG fprint(BPTR fh, UBYTE * format, LONG data,...)
{
    return (VFPrintf (fh, format, (ULONG *) &data));
}


/*  iname	Icon name
 *  oname	Output name
 *  dob		Icon to write source for
 *  icon	Create an icon for the listing?
 *
 */
BOOL IconToC (STRPTR iname, STRPTR oname, struct DiskObject * dob, BOOL icon)
{
    struct DrawerData *d;
    struct Gadget *g;
    struct NewWindow *n;
    STRPTR pname;
    UBYTE fname[255], tmp[255], msgs[255];
    STRPTR name = iname;
    BPTR fh;
    register int i;
    BOOL retval = FALSE;
    UBYTE revision;

    if ((name == NULL) || (strlen (name) < 1))
    {
	name = GetString(MSG_IE_UNTITLED);
    }

    for (i = 0; i < 255; i++)
    {
	fname[i] = tmp[i] = 0;
    }

    pname = FilePart(name);

    if (oname && (strlen (oname) > 0))
    {
	strcpy (fname, oname);
    }
    else
    {
	sprintf (fname, "RAM:%s_icon.c", pname);
    }

    if (fh = Open (fname, MODE_NEWFILE))
    {
	revision = WB_DISKREVISIONMASK & ((UBYTE) dob->do_Gadget.UserData);

	if (icon)
	{
	    struct DiskObject *dob;

	    if (dob = GetDiskObject (fname))
	    {
		FreeDiskObject (dob);
	    }
	    else if (dob = GetDiskObject ("ENV:Sys/def_c"))
	    {
		PutDiskObject (fname, dob);
		FreeDiskObject (dob);
	    }
	    else
	    {
		PutDiskObject (fname, &Listing);
	    }
	}

	/*--- Headers ---*/
	fprint (fh, "/* %s --- Data for %s icon\n", (LONG) fname, (LONG) name);
	fprint (fh, " *\n", NULL);
	fprint (fh, " */\n\n", NULL);
	fprint (fh, "#include <exec/types.h>\n", NULL);
	fprint (fh, "#include <intuition/intuition.h>\n", NULL);
	fprint (fh, "#include <workbench/workbench.h>\n\n", NULL);

	/*--- write the image structures & data ---*/
	g = &dob->do_Gadget;

	/* Do we have a normal image? */
	if (g->GadgetRender)
	{
	    sprintf (tmp, "%sI1", pname);
	    ImageToC (fh, tmp, g->GadgetRender);
	}

	/* Do we have a alternate image */
	if ((g->Flags & GADGHIMAGE) && g->SelectRender)
	{
	    sprintf (tmp, "%sI2", pname);
	    ImageToC (fh, tmp, g->SelectRender);
	}

	/*--- the tool types ---*/
	if (dob->do_ToolTypes && dob->do_ToolTypes[0])
	{
	    fprint (fh, "UBYTE *%sTools[] =\n{\n", (ULONG)pname);

	    for (i = 0; dob->do_ToolTypes[i] > NULL; i++)
	    {
		sprintf (tmp, "%s\000", dob->do_ToolTypes[i]);

		ConvertStr (fh, tmp);
	    }

	    fprint (fh, "    NULL\n};\n\n", NULL);
	}

	/*--- the drawerdata structure ---*/
	d = dob->do_DrawerData;
	if (d)
	{
	    /* NewWindow Structure */
	    n = &d->dd_NewWindow;
	    sprintf (tmp, "New%sWindow", pname);

	    /* DrawerData Structure */
	    fprint (fh, "struct DrawerData %sdr =\n{\n", (ULONG)pname);

	    NewWindowToC (fh, tmp, n);

	    if (revision > 0)
	    {
		fprint (fh, "    %ldL, %ldL,			/* Origin */\n",
			(LONG) d->dd_CurrentX, (LONG) d->dd_CurrentY);
		if (d->dd_Flags == NULL)
		    fprint (fh, "    NULL,			/* Flags for drawer */\n",
			    (LONG) d->dd_Flags);
		else
		    fprint (fh, "    0x%lx,			/* Flags for drawer */\n",
			    (LONG) d->dd_Flags);
		if (d->dd_ViewModes == NULL)
		    fprint (fh, "    NULL			/* ViewModes */\n",
			    (LONG) d->dd_ViewModes);
		else
		    fprint (fh, "    0x%lx			/* ViewModes */\n",
			    (LONG) d->dd_ViewModes);
	    }
	    else
	    {
		fprint (fh, "    %ldL, %ldL			/* Origin */\n",
			(LONG) d->dd_CurrentX, (LONG) d->dd_CurrentY);
	    }
	    fprint (fh, "};\n\n", NULL);
	}

	/*--- the diskobject structure ---*/
	fprint (fh, "struct DiskObject %s =\n{\n", (ULONG)pname);

	if (dob->do_Magic == WB_DISKMAGIC)
	    fprint (fh, "    WB_DISKMAGIC,		/* Magic Number */\n", NULL);
	else
	    fprint (fh, "    0x%04lx,			/* Magic Number */\n", (LONG) dob->do_Magic);
	if (dob->do_Version == WB_DISKVERSION)
	    fprint (fh, "    WB_DISKVERSION,		/* Version */\n", (LONG) dob->do_Version);
	else
	    fprint (fh, "    %ld,			/* Version */\n", (LONG) dob->do_Version);

	/*--- Embedded gadget structure ---*/
	GadgetToC (fh, pname, g);

	/*--- Remainder of the diskobject structure ---*/
	fprint (fh, "    WB%s,			/* Icon Type */\n", (LONG) ityp[dob->do_Type]);

        if (dob->do_DefaultTool)
        {
  	    sprintf (tmp, "%s\000", dob->do_DefaultTool);
	    if ((strlen (tmp)) > 0)
	        fprint (fh, "    (char *)\042%s\042,		/* Default Tool */\n", (LONG) tmp);
	    else
	        fprint (fh, "    NULL,			/* Default Tool */\n", NULL);
        }
	else
	{
	    fprint (fh, "    NULL,			/* Default Tool */\n", NULL);
	}

	if ((dob->do_ToolTypes) && (dob->do_ToolTypes[0]))
	    fprint (fh, "    %sTools,			/* Tool Type Array */\n", (ULONG)pname);
	else
	    fprint (fh, "    NULL,			/* Tool Type Array */\n", NULL);

	if (dob->do_CurrentX == NO_ICON_POSITION)
	    fprint (fh, "    NO_ICON_POSITION,		/* Current X */\n", (LONG) dob->do_CurrentX);
	else
	    fprint (fh, "    %ld,				/* Current X */\n", (LONG) dob->do_CurrentX);

	if (dob->do_CurrentX == NO_ICON_POSITION)
	    fprint (fh, "    NO_ICON_POSITION,		/* Current Y */\n", (LONG) dob->do_CurrentX);
	else
	    fprint (fh, "    %ld,				/* Current Y */\n", (LONG) dob->do_CurrentY);

	if (dob->do_DrawerData)
	{
	    fprint (fh, "    &%sdr,			/* Drawer Structure */\n", (ULONG)pname);
	}
	else
	{
	    fprint (fh, "    NULL,			/* Drawer Structure */\n", NULL);
	}

	if (dob->do_Type == WBTOOL && dob->do_ToolWindow)
	    fprint (fh, "    %s,				/* Tool Window */\n", (LONG) dob->do_ToolWindow);
	else
	    fprint (fh, "    NULL,			/* Tool Window */\n", NULL);
	fprint (fh, "    %ld				/* Stack Size */\n", (LONG) dob->do_StackSize);
	fprint (fh, "};\n", NULL);

	/*--- Close the file ---*/
	Close (fh);

	/* Clear the execute file attribute */
	SetProtection (fname, FIBF_EXECUTE);

	retval = TRUE;
    }
    else
    {
	sprintf (msgs, GetString(MSG_IE_ERROR_CANT_CREATE), fname);
	NotifyUser(MSG_IE_NOTHING,msgs);
    }

    return (retval);
}

UWORD __chip listingI1Data[] =
{
/* Plane 0 */
    0x0000,0x0000,0x0000,0x0400,0x0000,0x0000,0x0000,0x0C00,
    0x0000,0x0000,0x0000,0x0C00,0x0FFF,0xFFFF,0xFFE0,0x0C00,
    0x0800,0x0000,0x0030,0x0C00,0x0980,0x0000,0x0028,0x0C00,
    0x0800,0x0000,0x0024,0x0C00,0x098A,0xA000,0x0022,0x0C00,
    0x0800,0x0000,0x003F,0x0C00,0x098A,0xA800,0x000C,0x8C00,
    0x0800,0x0000,0x0000,0x8C00,0x0980,0x2AAA,0xA80C,0x8C00,
    0x0800,0x0000,0x0000,0x8C00,0x0980,0x2AAA,0xA00C,0x8C00,
    0x0800,0x0000,0x0000,0x8C00,0x098A,0xA800,0x000C,0x8C00,
    0x0800,0x0000,0x0000,0x8C00,0x098A,0x0000,0x000C,0x8C00,
    0x0FFF,0xFFFF,0xFFFF,0x8C00,0x0000,0x0000,0x0000,0x0C00,
    0x0000,0x0000,0x0000,0x0C00,0x7FFF,0xFFFF,0xFFFF,0xFC00,
/* Plane 1 */
    0xFFFF,0xFFFF,0xFFFF,0xF800,0xD555,0x5555,0x5555,0x5000,
    0xD555,0x5555,0x5555,0x5000,0xD000,0x0000,0x0015,0x5000,
    0xD7FF,0xFFFF,0xFFC5,0x5000,0xD67F,0xFFFF,0xFFD5,0x5000,
    0xD7FF,0xFFFF,0xFFD9,0x5000,0xD675,0x5FFF,0xFFDD,0x5000,
    0xD7FF,0xFFFF,0xFFC0,0x5000,0xD675,0x57FF,0xFFF3,0x5000,
    0xD7FF,0xFFFF,0xFFFF,0x5000,0xD67F,0xD555,0x57F3,0x5000,
    0xD7FF,0xFFFF,0xFFFF,0x5000,0xD67F,0xD555,0x5FF3,0x5000,
    0xD7FF,0xFFFF,0xFFFF,0x5000,0xD675,0x57FF,0xFFF3,0x5000,
    0xD7FF,0xFFFF,0xFFFF,0x5000,0xD675,0xFFFF,0xFFF3,0x5000,
    0xD000,0x0000,0x0000,0x5000,0xD555,0x5555,0x5555,0x5000,
    0xD555,0x5555,0x5555,0x5000,0x8000,0x0000,0x0000,0x0000,
};

struct Image listingI1 =
{
    0, 0,			/* Upper left corner */
    54, 22, 2,			/* Width, Height, Depth */
    listingI1Data,		/* Image data */
    0x0003, 0x0000,		/* PlanePick, PlaneOnOff */
    NULL			/* Next image */
};

struct DiskObject Listing =
{
    WB_DISKMAGIC,		/* Magic Number */
    WB_DISKVERSION,		/* Version */
    {				/* Embedded Gadget Structure */
	NULL,			/* Next Gadget Pointer */
	0, 0, 54, 23,		/* Left,Top,Width,Height */
	GADGIMAGE | GADGHCOMP,	/* Flags */
	RELVERIFY,		/* Activation Flags */
	BOOLGADGET,		/* Gadget Type */
	(APTR)&listingI1,	/* Render Image */
	NULL,			/* Select Image */
	NULL,			/* Gadget Text */
	NULL,			/* Mutual Exclude */
	NULL,			/* Special Info */
	0,			/* Gadget ID */
	NULL,			/* User Data */
    },
    WBPROJECT,			/* Icon Type */
    (char *)"Ed",		/* Default Tool */
    NULL,			/* Tool Type Array */
    NO_ICON_POSITION,		/* Current X */
    NO_ICON_POSITION,		/* Current Y */
    NULL,			/* Drawer Structure */
    NULL,			/* Tool Window */
    0				/* Stack Size */
};

VOID ConvertStr (BPTR fh, UBYTE * str)
{
    UBYTE outp[514];
    register int i, j;
    int k;

    k = MIN (strlen (str), 512);
    j = 0;

    for (i = 0; i < k; i++)
    {
	switch (str[i])
	{
	    case '"':
		outp[j++] = '\\';
		outp[j++] = '0';
		outp[j++] = '4';
		outp[j++] = '2';
		break;

	    default:
		outp[j++] = str[i];
		break;
	}
    }

    /* End the string */
    outp[j] = 0;

    fprint (fh, "    \042%s\042,\n", (LONG) outp);
}

/* a flag parsing method... convert Flag to Text */
struct ulongFlags
{
    ULONG uf_Flag;
    UBYTE *uf_Text;
};

struct ulongFlags IDCMPFlags[] =
{
    {0x00000001, "IDCMP_SIZEVERIFY"},
    {0x00000002, "IDCMP_NEWSIZE"},
    {0x00000004, "IDCMP_REFRESHWINDOW"},
    {0x00000008, "IDCMP_MOUSEBUTTONS"},

    {0x00000010, "IDCMP_MOUSEMOVE"},
    {0x00000020, "IDCMP_GADGETDOWN"},
    {0x00000040, "IDCMP_GADGETUP"},
    {0x00000080, "IDCMP_REQSET"},

    {0x00000100, "IDCMP_MENUPICK"},
    {0x00000200, "IDCMP_CLOSEWINDOW"},
    {0x00000400, "IDCMP_RAWKEY"},
    {0x00000800, "IDCMP_REQVERIFY"},

    {0x00001000, "IDCMP_REQCLEAR"},
    {0x00002000, "IDCMP_MENUVERIFY"},
    {0x00004000, "IDCMP_NEWPREFS"},
    {0x00008000, "IDCMP_DISKINSERTED"},

    {0x00010000, "IDCMP_DISKREMOVED"},
    {0x00020000, "IDCMP_WBENCHMESSAGE"},
    {0x00040000, "IDCMP_ACTIVEWINDOW"},
    {0x00080000, "IDCMP_INACTIVEWINDOW"},

    {0x00100000, "IDCMP_DELTAMOVE"},
    {0x00200000, "IDCMP_VANILLAKEY"},
    {0x00400000, "IDCMP_INTUITICKS"},
    {0, NULL}
};

struct ulongFlags ActiveFlags[] =
{
    {0x00000001, "GACT_RELVERIFY"},
    {0x00000002, "GACT_GADGIMMEDIATE"},
    {0x00000004, "GACT_ENDGADGET"},
    {0x00000008, "GACT_FOLLOWMOUSE"},

    {0x00000010, "GACT_RIGHTBORDER"},
    {0x00000020, "GACT_LEFTBORDER"},
    {0x00000040, "GACT_TOPBORDER"},
    {0x00000080, "GACT_BOTTOMBORDER"},

    {0x00000100, "GACT_TOGGLESELECT"},
    {0x00000200, "GACT_STRINGCENTER"},
    {0x00000400, "GACT_STRINGRIGHT"},
    {0x00000800, "GACT_LONGINT"},

    {0x00001000, "GACT_ALTKEYMAP"},
    {0x00002000, "GACT_BOOLEXTEND"},
    {0, NULL}
};

/*    {0x0000000, ""}, */

struct ulongFlags WinFlags[] =
{
    {0x00000001, "WFLG_SIZEGADGET"},
    {0x00000002, "WFLG_DRAGBAR"},
    {0x00000004, "WFLG_DEPTHGADGET"},
    {0x00000008, "WFLG_CLOSEGADGET"},

    {0x00000010, "WFLG_SIZEBRIGHT"},
    {0x00000020, "WFLG_SIZEBBOTTOM"},
    {0x00000040, "WFLG_SIMPLE_REFRESH"},
    {0x00000080, "WFLG_SUPER_BITMAP"},

    {0x00000100, "WFLG_BACKDROP"},
    {0x00000200, "WFLG_REPORTMOUSE"},
    {0x00000400, "WFLG_GIMMEZEROZERO"},
    {0x00000800, "WFLG_BORDERLESS"},

    {0x00001000, "WFLG_ACTIVATE"},
    {0x00002000, "WFLG_WINDOWACTIVE"},
    {0x00004000, "WFLG_INREQUEST"},
    {0x00008000, "WFLG_MENUSTATE"},

    {0x00010000, "WFLG_RMBTRAP"},
    {0x00020000, "WFLG_NOCAREREFRESH"},

    {0x01000000, "WFLG_WINDOWREFRESH"},
    {0x02000000, "WFLG_WBENCHWINDOW"},
    {0x04000000, "WFLG_WINDOWTICKED"},

    {0, NULL}
};

VOID printFlags (BPTR fh, UBYTE * name, SHORT lvl, UBYTE * pref)
{
    static SHORT tlen = 0;

    if (lvl > 0)
    {
	if (tlen > 40)
	{
	    fprint (fh, "\n%s", (LONG) pref);
	    tlen = 0;
	}
	fprint (fh, " | %s", (LONG) name);
    }
    else
    {
	tlen = 0;
	fprint (fh, "%s", (LONG) name);
    }

    tlen += strlen (name);
}

VOID parseFlags (BPTR fh, ULONG flags, struct ulongFlags * uf,
		  UBYTE * pref, UBYTE * sufx)
{
    USHORT j = 0;

    fprint (fh, "%s", (LONG) pref);
    do
    {
	if (flags & uf->uf_Flag)
	{
	    printFlags (fh, uf->uf_Text, j++, pref);
	    flags &= ~uf->uf_Flag;	/* strip out used flags */
	}
	*uf++;
    } while (uf->uf_Text);

    if (j == 0)
    {
	if (flags == 0L)
	    fprint (fh, "NULL", NULL);
	else
	    fprint (fh, "0x0%lx", (LONG) flags);
    }
    else if (flags > 0L)
    {
	/* print out leftover flags */
	fprint (fh, " | 0x0%lx", (LONG) flags);
    }

    fprint (fh, "%s", (LONG) sufx);
}

VOID parseGadFlags (BPTR fh, USHORT flags)
{
    SHORT i = 0;
    UBYTE *pref = "	";

    fprint (fh, pref, NULL);

    if (flags & GFLG_GADGIMAGE)
    {
	printFlags (fh, "GFLG_GADGIMAGE", i++, pref);
    }

    flags &= ~GFLG_GADGIMAGE;

    if (flags == GFLG_GADGHCOMP)
	printFlags (fh, "GFLG_GADGHCOMP", i, pref);
    else if (flags & GADGBACKFILL)
	printFlags (fh, "GADGBACKFILL", i, pref);
    else if (flags & GFLG_GADGHIMAGE)
	printFlags (fh, "GFLG_GADGHIMAGE", i, pref);

    fprint (fh, ",	/* Flags */\n", NULL);
}

VOID
ImageToC (BPTR fh, STRPTR name, struct Image * i)
{
    int w, h, d, c, t, cp;
    register int j, k;
    USHORT *data;

    /* Image data portion */
    fprint (fh, "UWORD __chip %sData[] =\n{\n", (LONG) name);

    w = i->Width;
    h = i->Height;
    d = i->Depth;
    c = ((w + 15) / 16) * h;
    cp = 0;
    data = i->ImageData;
    for (k = 0; k < d; k++)
    {
	if (k > 0)
	    fprint (fh, "\n", NULL);
	fprint (fh, "/* Plane %ld */\n    ", (LONG) k);
	t = 0;
	for (j = 0; j < c; j++)
	{
	    if (t > 7)
	    {
		fprint (fh, "\n    ", NULL);
		t = 0;
	    }
	    t++;
	    fprint (fh, "0x%04lx,", (LONG) data[cp++]);
	}
    }
    fprint (fh, "\n};\n\n", NULL);

    /* Image structure portion */
    fprint (fh, "struct Image %s =\n{\n", (LONG) name);
    fprint (fh, "    %ld, %ld,			/* Upper left corner */\n",
	    (LONG) i->LeftEdge, (LONG) i->TopEdge);
    fprint (fh, "    %ld, %ld, %ld,			/* Width, Height, Depth */\n",
	    (LONG) w, (LONG) h, (LONG) d);
    fprint (fh, "    %sData,		/* Image data */\n", (LONG) name);
    fprint (fh, "    0x%04lx, 0x%04lx,		/* PlanePick, PlaneOnOff */\n",
	    (LONG) i->PlanePick, (LONG) i->PlaneOnOff);
    fprint (fh, "    NULL			/* Next image */\n", NULL);
    fprint (fh, "};\n\n", NULL);
}

VOID
GadgetToC (BPTR fh, STRPTR stnm, struct Gadget * g)
{

/*--- the gadget structurer ---*/
    if (fh && stnm && g)
    {
	fprint (fh, "    {				/* Embedded Gadget Structure */\n", NULL);
	fprint (fh, "	NULL,			/* Next Gadget Pointer */\n", NULL);
	fprint (fh, "	0, 0, %ld, %ld,		/* Left,Top,Width,Height */\n",
/*		(LONG) g->LeftEdge, (LONG) g->TopEdge, */
		(LONG)  g->Width, (LONG) g->Height);

	parseGadFlags (fh, g->Flags);

	parseFlags (fh, (ULONG) g->Activation, &ActiveFlags[0],
		    "	", ",		/* Activation Flags */\n");


	fprint (fh, "	BOOLGADGET,		/* Gadget Type */\n", (LONG) g->GadgetType);

	if (g->GadgetRender)
	{
	    fprint (fh, "	(APTR)&%sI1,	/* Render Image */\n", (LONG) stnm);
	}
	else
	{
	    fprint (fh, "	NULL,			/* Render Image */\n", NULL);
	}

	if ((g->Flags & GADGHIMAGE) && g->SelectRender)
	{
	    fprint (fh, "	(APTR)&%sI2,	/* Select Image */\n", (LONG) stnm);
	}
	else
	{
	    fprint (fh, "	NULL,			/* Select Image */\n", NULL);
	}

	fprint (fh, "	NULL,			/* Gadget Text */\n", NULL);
	fprint (fh, "	NULL,			/* Mutual Exclude */\n", NULL);
	fprint (fh, "	NULL,			/* Special Info */\n", NULL);
	fprint (fh, "	%ld,			/* Gadget ID */\n", (LONG) g->GadgetID);

	if (g->UserData)
	    fprint (fh, "	(APTR) 0x%04lx,		/* User Data (Revision) */\n", (LONG) g->UserData);
	else
	    fprint (fh, "	NULL,			/* User Data */\n", NULL);

	fprint (fh, "    },\n", NULL);
    }
}

VOID
NewWindowToC (BPTR fh, STRPTR stnm, struct NewWindow * n)
{
    if (fh && stnm && n)
    {
	fprint (fh, "    %ld, %ld, %ld, %ld,		/* Size */\n",
		(LONG) n->LeftEdge, (LONG) n->TopEdge,
		(LONG) n->Width, (LONG) n->Height);

	fprint (fh, "    %ld, %ld,			/* DetailPen, BlockPen */\n",
		(LONG) n->DetailPen, (LONG) n->BlockPen);

	parseFlags (fh, n->IDCMPFlags, &IDCMPFlags[0],
		    "    ", ",		/* IDCMPFlags */\n");

	parseFlags (fh, (ULONG) n->Flags, &WinFlags[0],
		    "    ", ",		/* Flags */\n");

	fprint (fh, "    NULL,NULL,NULL,NULL,NULL,\n", NULL);
	fprint (fh, "    %ld, %ld,			/* Min Size */\n",
		(LONG) n->MinWidth, (LONG) n->MinHeight);
	fprint (fh, "    %ld, %ld,			/* Max Size */\n",
		(LONG) n->MaxWidth, (LONG) n->MaxHeight);
	fprint (fh, "    WBENCHSCREEN,		/* Type */\n", NULL);
    }
}
