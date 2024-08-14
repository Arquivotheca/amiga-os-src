
/* includes */
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/libraries.h>
#include <exec/io.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <devices/clipboard.h>
#include <libraries/iffparse.h>
#include <datatypes/datatypes.h>
#include <datatypes/datatypesclass.h>
#include <datatypes/pictureclass.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>

/* prototypes */
#include <clib/macros.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/datatypes_protos.h>
#include <clib/icon_protos.h>

/* direct ROM interface */
#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/iffparse_pragmas.h>
#include <pragmas/datatypes_pragmas.h>
#include <pragmas/icon_pragmas.h>

/*****************************************************************************/

extern struct Library *SysBase;
extern struct Library *DOSBase;
struct Library *GfxBase;
struct Library *IntuitionBase;
struct Library *IFFParseBase;
struct Library *DataTypesBase;
struct Library *IconBase;

struct GlobalData
{
    struct Screen *gd_Screen;
    struct Window *gd_Window;
    Object *gd_DataObject;
};

/*****************************************************************************/

ULONG __stdargs DoMethodA (Object * o, Msg msg);

/*****************************************************************************/

APTR newdtobject (STRPTR name, Tag tag1,...)
{

    return (NewDTObjectA (name, (struct TagItem *) & tag1));
}

/*****************************************************************************/

ULONG getdtattrs (Object * o, ULONG data,...)
{

    return (GetDTAttrsA (o, (struct TagItem *) & data));
}

/*****************************************************************************/

ULONG setdtattrs (Object * o, ULONG data,...)
{

    return (SetDTAttrsA (o, NULL, NULL, (struct TagItem *) & data));
}

/*****************************************************************************/

Object *loadPicture (struct GlobalData * gd, APTR name, ULONG stype, BOOL layout)
{
    struct gpLayout gpl;
    Object *o;

    /* Get a pointer to the object */
    if (o = newdtobject (name,
			 DTA_SourceType, stype,
			 DTA_GroupID, GID_PICTURE,	/* Only load pictures */
			 PDTA_Screen, gd->gd_Screen,	/* What screen to remap to */
			 PDTA_Remap, layout,
			 TAG_DONE))
    {
	/* Tell the object to layout */
	gpl.MethodID = DTM_PROCLAYOUT;
	gpl.gpl_GInfo = NULL;
	gpl.gpl_Initial = 1;
	if (!DoMethodA (o, &gpl))			/* Layout the object */
	{
	    printf ("Couldn't layout object\n");
	    DisposeDTObject (o);
	    o = NULL;
	}
    }
    else
    {
	printf ("Couldn't load object, error=%ld\n", IoErr ());
    }

    return (o);
}

/*****************************************************************************/

BOOL savePicture (struct GlobalData * gd, Object * o, STRPTR filename, ULONG stype)
{
    BOOL result = FALSE;

    if (o)
    {
	if (stype == DTST_CLIPBOARD)
	{
	    struct dtGeneral dtg;

	    /* Copy it to the clipboard */
	    dtg.MethodID = DTM_COPY;
	    DoDTMethodA (o, NULL, NULL, &dtg);

	    /* Show that we were successful */
	    result = TRUE;
	}
	else if (stype == DTST_FILE)
	{
	    struct DiskObject *dob;
	    struct dtWrite dtw;
	    BPTR fh;

	    if (fh = Open (filename, MODE_NEWFILE))
	    {
		dtw.MethodID = DTM_WRITE;
		dtw.dtw_GInfo = NULL;
		dtw.dtw_FileHandle = fh;
		dtw.dtw_Mode = DTWM_IFF;
		dtw.dtw_AttrList = NULL;
		if (DoMethodA (o, &dtw))
		{
		    /* Get the an Icon for the file */
		    if (dob = GetDiskObject (filename))
		    {
			FreeDiskObject (dob);
		    }
		    else if (dob = GetDiskObject ("ENV:Sys/def_ilbm"))
		    {
			PutDiskObject (filename, dob);
			FreeDiskObject (dob);
		    }
		    else if (dob = GetDefDiskObject (WBPROJECT))
		    {
			PutDiskObject (filename, dob);
			FreeDiskObject (dob);
		    }

		    /* Show that we were successful */
		    result = TRUE;
		}

		Close (fh);
	    }
	}
    }

    return (result);
}

/*****************************************************************************/

int main (int argc, char **argv)
{
    struct BitMapHeader *bmhd;
    struct GlobalData *gd;
    struct BitMap *bm;

    printf ("This simple example loads a picture into the clipboard\n");

    if (argc < 2)
    {
	printf ("A file name is required\n");
    }
    else if (DataTypesBase = OpenLibrary ("datatypes.library", 39))
    {
	if (IFFParseBase = OpenLibrary ("iffparse.library", 37))
	{
	    GfxBase = OpenLibrary ("graphics.library", 39);
	    IntuitionBase = OpenLibrary ("intuition.library", 39);
	    IconBase = OpenLibrary ("icon.library", 39);

	    if (gd = AllocVec (sizeof (struct GlobalData), MEMF_CLEAR))
	    {
		if (gd->gd_Screen = LockPubScreen (NULL))
		{

		    /*
		     * The screen and layout is NOT need for this example.  I
		     * just included the code to show you how it is done.
		     */
		    printf ("loading %s\n", argv[1]);
		    if (gd->gd_DataObject = loadPicture (gd, (APTR) argv[1], DTST_FILE, FALSE))
		    {
			/* Get the picture information */
			if ((getdtattrs (gd->gd_DataObject,
					 PDTA_BitMapHeader, &bmhd,
					 PDTA_BitMap, &bm,
					 TAG_DONE) == 2) && bm)
			{

			    /*
			     * The BitMapHeader contains the sizing information
			     * of the picture. The actual structure is defined
			     * in <datatypes/pictureclass.h>. The BitMap
			     * contains the picture data
			     */
			}

			/* Save the object to the clipboard */
			printf ("copying to the clipboard\n");
			savePicture (gd, gd->gd_DataObject, NULL, DTST_CLIPBOARD);

			DisposeDTObject (gd->gd_DataObject);
		    }
		    printf ("done\n");
		    UnlockPubScreen (NULL, gd->gd_Screen);
		}
		FreeVec (gd);
	    }
	    CloseLibrary (DataTypesBase);
	}
	else
	{
	    printf ("requires iffparse.library V37\n");
	}

	CloseLibrary (IconBase);
	CloseLibrary (IntuitionBase);
	CloseLibrary (GfxBase);
	CloseLibrary (IFFParseBase);
	CloseLibrary (DataTypesBase);
    }
    else
    {
	printf ("requires datatypes.library V39\n");
    }
}
