/******************************************************************************
 *
 * COPYRIGHT: Unless otherwise noted, all files are Copyright (c) 1992, 1993
 * Commodore-Amiga, Inc.  All rights reserved.
 *
 * DISCLAIMER: This software is provided "as is".  No representations or
 * warranties are made with respect to the accuracy, reliability, performance,
 * currentness, or operation of this software, and all use is at your own risk.
 * Neither commodore nor the authors assume any responsibility or liability
 * whatsoever with respect to your use of this software.
 *
 ******************************************************************************
 * dispatch.c
 *
 */

#include "classbase.h"

/*****************************************************************************/

#if DEBUG_ME
#define	DB(x)	x
#else
#define	DB(x)	;
#endif

/*****************************************************************************/

#define	G(o)	((struct Gadget *)o)

/*****************************************************************************/

Class *initClass (struct ClassBase * cb)
{
    Class *cl;

    if (cl = MakeClass (WAVDTCLASS, SOUNDDTCLASS, NULL, NULL, 0L))
    {
	cl->cl_Dispatcher.h_Entry = Dispatch;
	cl->cl_UserData = (ULONG) cb;
	AddClass (cl);
    }

    return (cl);
}

/*****************************************************************************/

ULONG ASM Dispatch (REG (a0) Class * cl, REG (a2) Object * o, REG (a1) Msg msg)
{
    struct ClassBase *cb = (struct ClassBase *) cl->cl_UserData;
    ULONG retval = 0L;

    switch (msg->MethodID)
    {
	case OM_NEW:
	    if (retval = DoSuperMethodA (cl, o, msg))
	    {
		if (!(ConvertObjectData (cb, cl, (Object *) retval, ((struct opSet *) msg)->ops_AttrList)))
		{
		    CoerceMethod (cl, (Object *) retval, OM_DISPOSE);
		    retval = NULL;
		}
	    }
	    break;

	    /* Let the superclass handle everything else */
	default:
	    retval = (ULONG) DoSuperMethodA (cl, o, msg);
	    break;
    }

    return (retval);
}

/*****************************************************************************/

#define	SWAPW(a)	(WORD)(((UWORD)a>>8)+((((UWORD)a&0xff)<<8)))
#define	SWAPU(a)	(UWORD)(((UWORD)a>>8)+((((UWORD)a&0xff)<<8)))
#define	SWAPL(a)	(LONG)(((ULONG)a>>24)+(((ULONG)a&0xff0000)>>8)+(((ULONG)a&0xff00)<<8)+(((ULONG)a&0xff)<<24))

/*****************************************************************************/

#define	fmt_ID	MAKE_ID('f','m','t',' ')
#define	data_ID	MAKE_ID('d','a','t','a')

/*****************************************************************************/

/* RIFF chunk */
struct RIFFChunk
{
    ULONG	 rc_ID;
    ULONG	 rc_Size;
    STRPTR	 rc_Buffer;
};

#define	RC_SIZE	(sizeof (struct RIFFChunk))

/*****************************************************************************/

struct WaveFormat
{
    WORD	 wf_Format;
    WORD	 wf_Channels;
    ULONG	 wf_SamplesPerSec;
    ULONG	 wf_AvgBytesPerSec;
    WORD	 wf_BlockAlign;
};

#define	WF_SIZE	(sizeof (struct WaveFormat))

/* wf_Format values */
#define	WAVE_FORMAT_PCM		1

/*****************************************************************************/

struct PCMData
{
    UWORD	 pd_BitsPerSample;
};

/*****************************************************************************/

#if DEBUG_ME
STRPTR IDtoStr (ULONG id, STRPTR buff)
{
    register UWORD i;

    for (i = 0; i < 4; i++)
	buff[i] = (id >> ((3 - i) * 8)) & 0xFF;
    buff[4] = 0;
    return buff;
}
#endif

/*****************************************************************************/

BOOL ConvertObjectData (struct ClassBase * cb, Class * cl, Object * o, struct TagItem * attrs)
{
    struct FileInfoBlock *fib;
    struct VoiceHeader *vhdr;
    BOOL retval = FALSE;
    ULONG samplelength;
    STRPTR title;
    ULONG period;
    APTR sample;
    LONG size;
    APTR data;
    BPTR fh;

    /* RIFF related */
    struct WaveFormat *wf = NULL;
    struct RIFFChunk *rc;
    struct PCMData *pd;
    STRPTR buffer, ptr;
    LONG pos, len;
    UBYTE buff[5];

    title = (STRPTR) GetTagData (DTA_Name, NULL, attrs);

    getdtattrs (cb, o,
		SDTA_VoiceHeader, &vhdr,
		DTA_Handle, &fh,
		TAG_DONE);

    if (fh && vhdr)
    {
	/* Allocate a temporary file info block */
	if (fib = (struct FileInfoBlock *) AllocMem (sizeof (struct FileInfoBlock), NULL))
	{
	    /* Get the size of the file */
	    if (ExamineFH (fh, fib))
	    {
		size = fib->fib_Size;
	    }
	    else
	    {
		Seek (fh, 0, OFFSET_END);
		size = Seek (fh, 0, OFFSET_BEGINNING);
	    }

	    /* Free the temporary file info block */
	    FreeMem (fib, sizeof (struct FileInfoBlock));

	    /* Buffered IO block */
	    if (buffer = ptr = AllocVec (size, MEMF_CLEAR))
	    {
		/* Read the whole file into the buffer */
		if (Read (fh, buffer, size) == size)
		{
		    DB (kprintf ("file is %ld bytes long\n", size));

		    /* Get the first RIFF chunk */
		    rc = (struct RIFFChunk *) ptr;
		    len = SWAPL (rc->rc_Size) + 8;
		    DB (kprintf ("%s, %ld\n", IDtoStr (rc->rc_ID, buff), len));

		    /* Skip past the first chunk now that we read it */
		    ptr += RC_SIZE;

		    /* Make sure we have the right size */
		    if (len == size)
		    {
			retval = TRUE;
			pos = RC_SIZE + 12;
			rc = (struct RIFFChunk *) ptr;
			while (rc && retval)
			{
			    /* Get the new chunk and size */
			    rc  = (struct RIFFChunk *) ptr;
			    len = SWAPL (rc->rc_Size) + 8;

			    DB (kprintf ("\"%s\", %ld\n", IDtoStr (rc->rc_ID, buff), len));

			    /* Handle the chunk types */
			    switch (rc->rc_ID)
			    {
				/* WaveForm structure */
				case fmt_ID:
				    wf = (struct WaveFormat *) &rc->rc_Buffer;
				    DB (kprintf ("  Wave Format\n"));
				    DB (kprintf ("    Format: %ld\n", (LONG) SWAPW (wf->wf_Format)));
				    DB (kprintf ("  Channels: %ld\n", (LONG) SWAPW (wf->wf_Channels)));
				    DB (kprintf ("       SPS: %ld\n", SWAPL (wf->wf_SamplesPerSec)));
				    DB (kprintf ("   Avg.BPS: %ld\n", SWAPL (wf->wf_AvgBytesPerSec)));
				    DB (kprintf ("     Align: %ld\n", (LONG) SWAPW (wf->wf_BlockAlign)));

				    /* Fill in the VoiceHeader */
				    vhdr->vh_SamplesPerSec = (UWORD) SWAPL (wf->wf_SamplesPerSec);
				    vhdr->vh_Octaves       = 1;
				    vhdr->vh_Compression   = 0;
				    vhdr->vh_Volume        = 63;

				    /* We can only handle this one format (Only known one at the
				     * time this was written */
				    if (WAVE_FORMAT_PCM == SWAPW (wf->wf_Format))
				    {
					pd = (struct PCMData *) ((UBYTE *)&rc->rc_Buffer + WF_SIZE);
					DB (kprintf ("       BPS: %ld\n", (LONG) SWAPW (pd->pd_BitsPerSample)));

					/* Make sure we have something that we can really handle */
					if ((SWAPW (pd->pd_BitsPerSample) != 8) || (SWAPW (wf->wf_Channels) != 1))
					{
					    SetIoErr (ERROR_OBJECT_WRONG_TYPE);
					    DB (kprintf ("unsupported type\n"));
					    retval = FALSE;
					}
				    }
				    else
				    {
					SetIoErr (ERROR_OBJECT_WRONG_TYPE);
					DB (kprintf ("unknown format\n"));
					retval = FALSE;
				    }
				    break;

				/* Sound data */
				case data_ID:
				    /* Make sure we have a WaveForm structure */
				    if (wf)
				    {
					samplelength = len - 8;
					if (sample = AllocVec (samplelength, MEMF_CHIP | MEMF_CLEAR))
					{
					    register ULONG i;
					    BYTE *tmp;

					    /* Convert the data (change from [0 - 255] to [-128 - 128] */
					    tmp = (BYTE *) &rc->rc_Buffer;
					    for (i = 0; i < samplelength; i++)
						tmp[i] ^= 128;

					    /* Copy the sample to chip memory */
					    CopyMem ((APTR) &rc->rc_Buffer, (APTR) sample, samplelength);

					    /* Compute the attributes */
					    data = (APTR) sample;
					    period = (ULONG) (SysBase->ex_EClockFrequency * 5) / (ULONG) vhdr->vh_SamplesPerSec;

					    /* Tell the super-class about the attributes */
					    DB (kprintf ("set sample to %08lx\n", sample));
					    setdtattrs (cb, o,
							DTA_ObjName,		title,
							SDTA_Sample,		sample,
							SDTA_SampleLength,	samplelength,
							SDTA_Period,		period,
							SDTA_Volume,		(ULONG) vhdr->vh_Volume,
							SDTA_Cycles,		1,
							TAG_DONE);
					}
					else
					{
					    SetIoErr (ERROR_NO_FREE_STORE);
					    DB (kprintf ("not enough memory\n"));
					    retval = FALSE;
					}
				    }
				    else
				    {
					SetIoErr (ERROR_REQUIRED_ARG_MISSING);
					DB (kprintf ("no waveform structure\n"));
					retval = FALSE;
				    }
				    break;
			    }

			    /* Increment the pointer and the size */
			    ptr += len;
			    pos += len;

			    /* Are we at the end? */
			    if (pos > size)
				rc = NULL;
			}
		    }
		    else
		    {
			SetIoErr (ERROR_BAD_HUNK);
			DB (kprintf ("mangled file\n"));
		    }
		}
		else
		{
		    DB (kprintf ("couldn't read %ld bytes : error=%ld\n", size, IoErr ()));
		}

		/* Free the buffer */
		FreeVec (buffer);
	    }
	    else
	    {
		SetIoErr (ERROR_NO_FREE_STORE);
		DB (kprintf ("not enough memory\n"));
	    }
	}
	else
	{
	    SetIoErr (ERROR_NO_FREE_STORE);
	    DB (kprintf ("not enough memory\n"));
	}
    }
    return (retval);
}

/*****************************************************************************/

ULONG setdtattrs (struct ClassBase * cb, Object * o, ULONG data,...)
{
    return (SetDTAttrsA (o, NULL, NULL, (struct TagItem *) & data));
}

/*****************************************************************************/

ULONG getdtattrs (struct ClassBase * cb, Object * o, ULONG data,...)
{
    return (GetDTAttrsA (o, (struct TagItem *) & data));
}
