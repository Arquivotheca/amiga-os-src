/* makeanim.c
 * Written by David N. Junod
 *
 */

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

#include "anim.h"

/*****************************************************************************/

#include "makeanim_iprotos.h"

/*****************************************************************************/

extern struct Library *SysBase;
extern struct Library *DOSBase;
struct Library *GfxBase;
struct Library *IntuitionBase;
struct Library *IFFParseBase;
struct Library *DataTypesBase;
struct Library *IconBase;

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

Object *loadPicture (STRPTR name)
{
    struct gpLayout gpl;
    Object *o;

    /* Get a pointer to the object */
    if (o = newdtobject ((APTR) name,
			 DTA_SourceType, DTST_FILE,
			 DTA_GroupID, GID_PICTURE,	/* Only load pictures */
			 PDTA_Remap, FALSE,
			 TAG_DONE))
    {
	/* Tell the object to layout */
	gpl.MethodID = DTM_PROCLAYOUT;
	gpl.gpl_GInfo = NULL;
	gpl.gpl_Initial = 1;
	if (!DoMethodA (o, (Msg) & gpl))	/* Layout the object */
	{
	    DisposeDTObject (o);
	    o = NULL;
	}
    }

    return (o);
}

/*****************************************************************************/

LONG *CreateDeltaData (struct BitMap * bm1, struct BitMap * bm2, LONG * deltan)
{
    LONG j, jj[8], nBpP, ib, nb;
    register WORD i;
    UBYTE *outstuff;
    LONG nBpR, nRpP;
    LONG *delta;

    nBpR = (LONG) bm2->BytesPerRow;
    nRpP = (LONG) bm2->Rows;
    nBpP = nBpR * nRpP;

    j = 64;
    for (i = 0; i < bm2->Depth; i++)
    {
	nb = 0;
	for (ib = 0; ib < nBpP; ib++)
	    if (*(bm2->Planes[i] + ib) != *(bm1->Planes[i] + ib))
		nb++;
	if (nb)
	{
	    jj[i] = skip_count_plane (bm2->Planes[i], bm1->Planes[i], nBpR, nRpP);
	    j += jj[i];
	}
	else
	    jj[i] = 0;
    }

    if (delta = AllocVec (j, MEMF_CLEAR))
    {
	outstuff = (UBYTE *) (&delta[16]);
	*deltan = 64;
	for (i = 0; i < bm2->Depth; i++)
	{
	    if (jj[i])
	    {
		delta[i] = *deltan;
		*deltan += jj[i];
		outstuff = skip_comp_plane (bm2->Planes[i], bm1->Planes[i], outstuff, nBpR, nRpP);
	    }
	}
    }
    return (delta);
}

/*****************************************************************************/

BOOL saveDelta (struct IFFHandle * iff, struct BitMap * bm1, struct BitMap * bm2)
{
    struct AnimHeader anhd;
    UBYTE retval = 0;
    LONG *delta;
    LONG ndelta;

    if (PushChunk (iff, ID_ILBM, ID_FORM, IFFSIZE_UNKNOWN) == 0)
    {
	if (PushChunk (iff, ID_ILBM, ID_ANHD, IFFSIZE_UNKNOWN) == 0)
	{
	    /* Initialize the AnimHeader */
	    memset (&anhd, 0, sizeof (struct AnimHeader));
	    anhd.operation = 5;
	    anhd.reltime = 30;

	    /* Write the AnimHeader */
	    if (WriteChunkBytes (iff, &anhd, sizeof (struct AnimHeader)) == sizeof (struct AnimHeader))
		retval++;

	    /* Pop the form chunk */
	    PopChunk (iff);
	}

	if (PushChunk (iff, ID_ILBM, ID_DLTA, IFFSIZE_UNKNOWN) == 0)
	{
	    if (delta = CreateDeltaData (bm1, bm2, &ndelta))
	    {
		if (WriteChunkBytes (iff, delta, ndelta) == ndelta)
		    retval++;
		FreeVec (delta);
	    }

	    /* Pop the form chunk */
	    PopChunk (iff);
	}

	/* Pop the form chunk */
	PopChunk (iff);
    }

    return ((BOOL) ((retval == 2) ? TRUE : FALSE));
}

/*****************************************************************************/

BOOL savePicture (STRPTR filename, struct IFFHandle * iff, Object * o)
{

#if 1
    BOOL result = FALSE;
    struct dtWrite dtw;
    UBYTE tmp[300];
    UBYTE *buffer;
    ULONG msize;
    BPTR fh;

    sprintf (tmp, "%s.t", filename);
    if (fh = Open (tmp, MODE_NEWFILE))
    {
	memset (&dtw, 0, sizeof (struct dtWrite));
	dtw.MethodID = DTM_WRITE;
	dtw.dtw_FileHandle = fh;
	dtw.dtw_Mode = DTWM_IFF;
	if (DoMethodA (o, (Msg) & dtw))
	{
	    Seek (fh, 0, OFFSET_END);
	    msize = Seek (fh, 0, OFFSET_BEGINNING);
	    if (buffer = AllocVec (msize, NULL))
	    {
		if (Read (fh, buffer, msize) == msize)
		    if (WriteChunkBytes (iff, buffer, msize) == msize)
			result = TRUE;

		FreeVec (buffer);
	    }
	}
	Close (fh);
	DeleteFile (tmp);
    }
#else
    BOOL result = FALSE;
    struct dtWrite dtw;

    memset (&dtw, 0, sizeof (struct dtWrite));
    dtw.MethodID = DTM_WRITE;
    dtw.dtw_FileHandle = iff;
    dtw.dtw_Mode = DTWM_ENCAPS_IFF;
    if (DoMethodA (o, (Msg) & dtw))
    {
	/* Show that we were successful */
	result = TRUE;
    }

#endif
    return (result);
}

/*****************************************************************************/

struct BitMap *allocbitmap (UWORD w, UWORD h, UBYTE d, ULONG attr)
{
    struct BitMap *bm;
    register UBYTE i;
    ULONG size;

    if (bm = AllocVec (sizeof (struct BitMap), NULL))
    {
	size = (ULONG)w * (ULONG)((h + 15) >> 3 & 0xFFFE);
	InitBitMap (bm, d, w, h);

	for (i = 0; i < d; i++)
	{
	    if (!(bm->Planes[i] = AllocVec (size, MEMF_CLEAR | attr)))
	    {
		freebitmap (bm);
		return NULL;
	    }
	}
    }
    return (bm);
}

/*****************************************************************************/

void freebitmap (struct BitMap * bm)
{
    register UBYTE i;

    if (bm)
    {
	for (i = 0; i < bm->Depth; i++)
	    FreeVec (bm->Planes[i]);

	FreeVec (bm);
    }
}

/*****************************************************************************/

void CopyBitMap (struct BitMap * bm1, struct BitMap * bm2)
{
    register LONG nl, i, *ss, *dd;
    WORD ip, width, height, depth;

    width = bm1->BytesPerRow << 3;
    height = bm1->Rows;
    depth = bm1->Depth;

    /* convert bits to longs */
    nl = ((long) width * (long) height) >> 5;

    for (ip = 0; ip < depth; ip++)
    {
	ss = (LONG *) bm1->Planes[ip];
	dd = (LONG *) bm2->Planes[ip];

	for (i = 0; i < nl; i++)
	{
	    *dd = *ss;
	    dd++;
	    ss++;
	}
    }
}

/*****************************************************************************/

int main (int argc, char **argv)
{
    struct BitMap *bm1, *bm2, *bm3, *bm4;
    struct BitMapHeader *bmhd;
    struct IFFHandle *iff;
    BOOL error = FALSE;
    UBYTE buffer[300];
    UBYTE output[300];
    Object *o;
    ULONG i;

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

	    if (iff = AllocIFF ())
	    {
		sprintf (output, "%s.anim", argv[1]);
		if (iff->iff_Stream = (ULONG) Open (output, MODE_NEWFILE))
		{
		    InitIFFasDOS (iff);
		    if (OpenIFF (iff, IFFF_WRITE) == 0)
		    {
			/* Build the name of the first frame */
			i = 1;
			sprintf (buffer, "%s%03ld", argv[1], i++);

			/* Load the first frame */
			if (o = loadPicture (buffer))
			{
			    /* Build the name of the second frame */
			    sprintf (buffer, "%s%03ld", argv[1], i++);

			    /* Make sure we have a bitmap and its information */
			    if (getdtattrs (o, PDTA_BitMap, &bm1, PDTA_BitMapHeader, &bmhd, TAG_DONE) && bm1)
			    {
				/* Create the previous bitmap */
				bm2 = allocbitmap (bmhd->bmh_Width, bmhd->bmh_Height, bmhd->bmh_Depth, NULL);
				bm3 = allocbitmap (bmhd->bmh_Width, bmhd->bmh_Height, bmhd->bmh_Depth, NULL);
				bm4 = allocbitmap (bmhd->bmh_Width, bmhd->bmh_Height, bmhd->bmh_Depth, NULL);

				if (bm2 && bm3 && bm4)
				{
				    /* Initialize them */
				    CopyBitMap (bm1, bm2);
				    CopyBitMap (bm1, bm3);
				    CopyBitMap (bm1, bm4);

				    /* Show that we are writing an Animation */
				    if (PushChunk (iff, ID_ANIM, ID_FORM, IFFSIZE_UNKNOWN) == 0)
				    {
					/* Save the first frame as a complete ILBM */
					printf (".");
					error = !savePicture (output, iff, o);

					/* Get rid of the object */
					DisposeDTObject (o);

					/* Keep on going as long as we can load pictures */
					while ((o = loadPicture (buffer)) && !error)
					{
					    /* Get the picture information */
					    if (getdtattrs (o, PDTA_BitMap, &bm1, TAG_DONE) && bm1)
					    {
						printf (".");

						/* Shuffle the frames */
						CopyBitMap (bm3, bm4);
						CopyBitMap (bm2, bm3);
						CopyBitMap (bm1, bm2);

						/* Save the deltas */
						if (!saveDelta (iff, bm4, bm2))
						{
						    printf ("couldn't save delta\n");
						    error = TRUE;
						}
					    }
					    else
					    {
						printf ("couldn't get frame information\n");
						error = TRUE;
					    }

					    /* Dispose of the previous frame */
					    DisposeDTObject (o);

					    /* Build the next frame name */
					    sprintf (buffer, "%s%03ld", argv[1], i++);
					}

					/* Clear the pointer */
					o = NULL;

					/* Pop the form chunk */
					PopChunk (iff);
				    }
				    else
					printf ("couldn't push the ANIM FORM\n");
				}
				else
				    printf ("not enough memory\n");

				freebitmap (bm2);
				freebitmap (bm3);
				freebitmap (bm4);
			    }
			    else
				printf ("couldn't get frame information\n");

			    DisposeDTObject (o);

			    printf ("\ndone\n");
			}
			else
			    printf ("couldn't load first frame, \"%s\".\n", buffer);

			CloseIFF (iff);
		    }
		    else
			printf ("couldn't open \"%s\" for writing.\n", output);

		    Close (iff->iff_Stream);
		}
		else
		    printf ("couldn't create \"%s\".\n", output);

		FreeIFF (iff);
	    }
	    else
		printf ("not enough memory\n");
	}
	else
	    printf ("requires iffparse.library V37\n");

	CloseLibrary (IconBase);
	CloseLibrary (IntuitionBase);
	CloseLibrary (GfxBase);
	CloseLibrary (IFFParseBase);
	CloseLibrary (DataTypesBase);
    }
    else
	printf ("requires datatypes.library V39\n");
}
