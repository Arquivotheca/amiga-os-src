
#include <exec/types.h>
#include <intuition/intuition.h>
#include <workbench/workbench.h>
#include <internal/commands.h>
#include <string.h>
#include <stdio.h>

#include <clib/icon_protos.h>
#include <pragmas/icon_pragmas.h>

#include "iconpos_rev.h"


/*****************************************************************************/


#define TEMPLATE	"FILE/A,XPOS/N,YPOS/N,TYPE/K,DXPOS/N,DYPOS/N,DWIDTH/N,DHEIGHT/N,CREATE/S,FREEX/S,FREEY/S,IMAGE/K" CMDREV
#define OPT_FILE	0
#define OPT_XPOS	1
#define OPT_YPOS	2
#define OPT_TYPE	3
#define	OPT_DX		4
#define	OPT_DY		5
#define	OPT_DW		6
#define	OPT_DH		7
#define	OPT_CREATE	8
#define	OPT_FREEX	9
#define	OPT_FREEY	10
#define	OPT_IMAGE	11
#define OPT_COUNT	12


/*****************************************************************************/


LONG main(VOID)
{
struct Library    *SysBase = (*((struct Library **) 4));
struct DosLibrary *DOSBase;
struct Library    *IconBase;
struct DiskObject *tdobj = NULL;
struct DiskObject *dobj;
struct DrawerData *dd;
struct RDargs     *rdargs;
long               opts[OPT_COUNT];
char               buffer[256];
int rc, num, len, newtype = 0, newpos = 0;

    char *inames[] =
    {
	"UNKNOWN",
	"DISK",
	"DRAWER",
	"TOOL",
	"PROJECT",
	"GARBAGE",
	"DEVICE",
	"KICK",
	"APPICON",
	"OTHER"
    };

    rc = RETURN_FAIL;
    if (((DOSBase = (struct DosLibrary *) OpenLibrary ("dos.library", 36))) &&
	((IconBase = OpenLibrary ("icon.library", 0))))
    {

	memset ((char *) opts, 0, sizeof (opts));
	rdargs = ReadArgs (TEMPLATE, opts, NULL);
	if (rdargs == NULL)
	{
	    PrintFault (IoErr (), NULL);
	}
	else
	{
	    rc = RETURN_OK;

	    if (opts[OPT_TYPE])
	    {
		if (!stricmp ((char *) opts[OPT_TYPE], "disk"))
		    newtype = WBDISK;
		if (!stricmp ((char *) opts[OPT_TYPE], "drawer"))
		    newtype = WBDRAWER;
		if (!stricmp ((char *) opts[OPT_TYPE], "tool"))
		    newtype = WBTOOL;
		if (!stricmp ((char *) opts[OPT_TYPE], "project"))
		    newtype = WBPROJECT;
		if (!stricmp ((char *) opts[OPT_TYPE], "garbage"))
		    newtype = WBGARBAGE;
	    }

	    if ((len = strlen ((char *) opts[OPT_FILE])) > 5)
	    {
		if (!strnicmp (&((char *) opts[OPT_FILE])[len - 5], ".info", 5))
		{
		    ((char *) opts[OPT_FILE])[len - 5] = 0;
		}
	    }

	    dobj = GetDiskObject ((char *) opts[OPT_FILE]);

	    if (opts[OPT_IMAGE])
	    {
		if (!(tdobj = GetDiskObject ((char *) opts[OPT_IMAGE])))
		{
		    PutStr("Couldn't open image source!\n");
		    rc = RETURN_FAIL;
		}
	    }

	    if (rc == RETURN_OK)
	    {
		if (!dobj && opts[OPT_CREATE])
		{
		    if (newtype)
		    {
			if (dobj = GetDefDiskObject (newtype))
			{
			    PutStr("Create icon\n");
			}
		    }
		    else
		    {
			PutStr("Must specify icon type!\n");
		    }
		}

		if (dobj)
		{
		    if (opts[OPT_XPOS])
		    {
			num = *(long *) opts[OPT_XPOS];
			dobj->do_CurrentX = (num < 0) ? NO_ICON_POSITION : num;
			newpos = TRUE;
		    }

		    if (opts[OPT_FREEX])
		    {
			dobj->do_CurrentX = NO_ICON_POSITION;
			newpos = TRUE;
		    }

		    if (opts[OPT_YPOS])
		    {
			num = *(long *) opts[OPT_YPOS];
			dobj->do_CurrentY = (num < 0) ? NO_ICON_POSITION : num;
			newpos = TRUE;
		    }

		    if (opts[OPT_FREEY])
		    {
			dobj->do_CurrentY = NO_ICON_POSITION;
			newpos = TRUE;
		    }

		    if (opts[OPT_TYPE])
		    {
			dobj->do_Type = newtype;
		    }

		    sprintf (buffer,"%s type=%s ",
			     (char *) opts[OPT_FILE], inames[dobj->do_Type]);
                    PutStr(buffer);

		    if (dobj->do_CurrentX == NO_ICON_POSITION)
		    {
			PutStr("FREEX FREEY");
		    }
		    else
		    {
			sprintf(buffer,"%ld %ld", (long) dobj->do_CurrentX, (long) dobj->do_CurrentY);
			PutStr(buffer);
		    }

		    if (dd = dobj->do_DrawerData)
		    {
			if (opts[OPT_DX])
			{
			    num = *(long *) opts[OPT_DX];
			    dd->dd_NewWindow.LeftEdge = num;
			    newpos = TRUE;
			}

			if (opts[OPT_DY])
			{
			    num = *(long *) opts[OPT_DY];
			    dd->dd_NewWindow.TopEdge = num;
			    newpos = TRUE;
			}

			if (opts[OPT_DW])
			{
			    num = *(long *) opts[OPT_DW];
			    dd->dd_NewWindow.Width = num;
			    newpos = TRUE;
			}

			if (opts[OPT_DH])
			{
			    num = *(long *) opts[OPT_DH];
			    dd->dd_NewWindow.Height = num;
			    newpos = TRUE;
			}

			sprintf (buffer," DXPOS %ld DYPOS %ld DWIDTH %ld DHEIGHT %ld\n",
				 (LONG) dd->dd_NewWindow.LeftEdge,
				 (LONG) dd->dd_NewWindow.TopEdge,
				 (LONG) dd->dd_NewWindow.Width,
				 (LONG) dd->dd_NewWindow.Height);
                        PutStr(buffer);
		    }
		    PutStr("\n");

		    if (tdobj)
		    {
			dobj->do_Gadget.GadgetRender = tdobj->do_Gadget.GadgetRender;
			dobj->do_Gadget.SelectRender = tdobj->do_Gadget.SelectRender;
			dobj->do_Gadget.Width = tdobj->do_Gadget.Width;
			dobj->do_Gadget.Height = tdobj->do_Gadget.Height;
			newtype = TRUE;
		    }

		    if (newtype || newpos)
		    {
			PutDiskObject ((char *) opts[OPT_FILE], dobj);
		    }

		    FreeDiskObject (dobj);
		}
		else
		{
		    PrintFault (IoErr (), NULL);
		}

		if (tdobj)
		    FreeDiskObject (tdobj);
	    }
	}

	CloseLibrary (IconBase);
	CloseLibrary ((struct Library *) DOSBase);
    }
    else
    {
	OPENFAIL
    };

    return (rc);
}
