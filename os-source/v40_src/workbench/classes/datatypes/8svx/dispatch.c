/* dispatch.c
 *
 */

#include "classbase.h"

/*****************************************************************************/

#define	G(o)	((struct Gadget *)o)

#define BREAKPOINT 16384

/*****************************************************************************/

ULONG setdtattrs (struct ClassBase *cb, Object * o, ULONG data,...)
{
    return (SetDTAttrsA (o, NULL, NULL, (struct TagItem *) &data));
}

/*****************************************************************************/

ULONG getdtattrs (struct ClassBase *cb, Object * o, ULONG data,...)
{
    return (GetDTAttrsA (o, (struct TagItem *) &data));
}

/*****************************************************************************/

Class *initClass (struct ClassBase *cb)
{
    Class *cl;

    if (cl = MakeClass (EIGHTSVXDTCLASS, SOUNDDTCLASS, NULL, NULL, 0L))
    {
	cl->cl_Dispatcher.h_Entry = (ULONG (*)())Dispatch;
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
		if (!(Get8SVX (cb, cl, (Object *) retval, ((struct opSet *) msg)->ops_AttrList)))
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

/* Fibonacci delta encoding for sound data */
BYTE far codeToDelta[16] =
{-34, -21, -13, -8, -5, -3, -2, -1, 0, 1, 2, 3, 5, 8, 13, 21};

/* Unpack Fibonacci-delta encoded data from n byte source
 * buffer into 2*n byte dest buffer, given initial data
 * value x.  It returns the lats data value x so you can
 * call it several times to incrementally decompress the data.
 */

BYTE D1Unpack (BYTE source[], LONG n, BYTE dest[], BYTE x)
{
    LONG i, lim;
    UBYTE d;

    lim = n << 1;
    for (i = 0; i < lim; ++i)
    {
	/* Decode a data nibble, high nibble then low nibble */
	d = source[i >> 1];	/* get a pair of nibbles */
	if (i & 1)		/* select low or high nibble */
	    d &= 0xf;		/* mask to get the low nibble */
	else
	    d >>= 4;		/* shift to get the high nibble */

	x += codeToDelta[d];	/* add in the decoded delta */
	dest[i] = x;		/* store a 1 byte sample */
    }

    return (x);
}

/*****************************************************************************/

/* Unpack Fibonacci-delta encoded data from n byte
 * source buffer into 2*(n-2) byte dest buffer.
 * Source buffer has a pad byte, an 8-bit initial
 * value, followed by n-2 bytes comprising 2*(n-2)
 * 4-bit encoded samples.
 */

VOID DUnpack (UBYTE * source, ULONG n, APTR dest)
{
    D1Unpack (source + 2, n - 2, dest, source[1]);
}

/*****************************************************************************/

#define ID_CHAN MAKE_ID('C','H','A','N')

/*****************************************************************************/

ULONG ASM Get8SVX (REG (a6) struct ClassBase * cb, REG (a0) Class * cl, REG (a2) Object * o, REG (a1) struct TagItem * attrs)
{
    struct StoredProperty *sp;
    struct VoiceHeader *vhdr;
    struct ContextNode *cn;
    struct IFFHandle *iff;
    LONG sampleType = 0;
    ULONG samplelength;
    LONG error = 0;
    ULONG memflags;
    STRPTR title;
    ULONG period;
    APTR sample;
    APTR data;

    /* Get a pointer to the default title for this sound sample */
    title = FilePart ((STRPTR) GetTagData (DTA_Name, NULL, attrs));

    /* Get the attributes that we need */
    getdtattrs (cb, o, SDTA_VoiceHeader, &vhdr, DTA_Handle, &iff, TAG_DONE);

    /* Make sure we got the attributes that we needed */
    if (iff && vhdr)
    {
	/* Set up the property chunks */
	PropChunk (iff, ID_8SVX, ID_VHDR);
	PropChunk (iff, ID_8SVX, ID_NAME);
	PropChunk (iff, ID_8SVX, ID_CHAN);

	/* Set up the stop chunk */
	if ((error = StopChunk (iff, ID_8SVX, ID_BODY)) == 0)
	{
	    /* Parse the file */
	    if ((error = ParseIFF (iff, IFFPARSE_SCAN)) == 0)
	    {
		/* Make sure we have a voice header chunk */
		if (sp = FindProp (iff, ID_8SVX, ID_VHDR))
		{
		    /* Just so that we can break out of here... */
		    while (TRUE)
		    {
			/* Copy the voice header */
			*vhdr = *((struct VoiceHeader *) sp->sp_Data);

			/* Get the name chunk */
			if (sp = FindProp (iff, ID_8SVX, ID_NAME))
			    title = (STRPTR) sp->sp_Data;

			/* Get the channel information */
			if (sp = FindProp (iff, ID_8SVX, ID_CHAN))
			    sampleType = *((ULONG *) sp->sp_Data);

			/* Get a pointer to the body chunk */
			cn = CurrentChunk (iff);

			/* Get the sample length and check compression type */
			samplelength = cn->cn_Size;
			if (vhdr->vh_Compression == CMP_FIBDELTA)
			    samplelength = (samplelength - 2) * 2;
			else if (vhdr->vh_Compression != CMP_NONE)
			{
			    error = DTERROR_UNKNOWN_COMPRESSION;
			    break;
			}

			/* Allocate room for the sample */
			memflags = MEMF_CLEAR | MEMF_CHIP;
			if (samplelength > BREAKPOINT)
			    memflags = MEMF_CLEAR;
			if (!(sample = AllocVec (samplelength, memflags)))
			{
			    error = ERROR_NO_FREE_STORE;
			    break;
			}

			/* Get a pointer to the data buffer */
			data = (APTR) ((ULONG) sample + samplelength - cn->cn_Size);

			/* Read in the data */
			if (ReadChunkBytes (iff, data, cn->cn_Size) != cn->cn_Size)
			{
			    error = DTERROR_NOT_ENOUGH_DATA;
			    FreeVec (sample);
			    break;
			}

			/* If we are compressed, then unpack the data */
			if (vhdr->vh_Compression == CMP_FIBDELTA)
			    DUnpack (data, cn->cn_Size, sample);

			/* If we are stereo, then adjust the length so that the
			 * sample doesn't repeat */
			if (sampleType == 6)
			    samplelength >>= 1;

			/* Calculate the period */
			period = (ULONG) (SysBase->ex_EClockFrequency * 5) / (ULONG) vhdr->vh_SamplesPerSec;

			/* Tell the superclass about the attributes */
			setdtattrs (cb, o,
				    DTA_ObjName, title,
				    SDTA_Sample, sample,
				    SDTA_SampleLength, samplelength,
				    SDTA_Period, period,
				    TAG_DONE);

			return (TRUE);
		    }
		}
		else
		    error = ERROR_REQUIRED_ARG_MISSING;
	    }
	}
    }
    else
	error = ERROR_REQUIRED_ARG_MISSING;

    /* Convert IFF parse errors */
    if (error < 0)
    {
	error = DTERROR_INVALID_DATA;
	if (error == IFFERR_NOMEM)
	    error = ERROR_NO_FREE_STORE;
    }

    SetIoErr (error);

    return (FALSE);
}
