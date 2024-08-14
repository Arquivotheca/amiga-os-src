/* stripsnd.c
 * silly little IFF (or raw) to a 'nuther simple format
 * Written by David N. Junod
 *
 */

/* includes */
#include <exec/devices.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/ports.h>
#include <exec/execbase.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <devices/audio.h>
#include <prefs/sound.h>
#include <libraries/iffparse.h>

/* prototypes */
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/iffparse_protos.h>

/* direct ROM interface */
#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/iffparse_pragmas.h>

/*****************************************************************************/

extern struct Library *DOSBase;
extern struct ExecBase *SysBase;

struct Library *IFFParseBase;

/*****************************************************************************/

struct Sample
{
    ULONG	 s_Length;	/* Sample length */
    ULONG	 s_Period;	/* Sample period */
};

/*****************************************************************************/

#define CMP_NONE     0
#define CMP_FIBDELTA 1

struct VoiceHeader
{
    ULONG vh_OneShotHiSamples;
    ULONG vh_RepeatHiSamples;
    ULONG vh_SamplesPerHiCycle;
    UWORD vh_SamplesPerSec;
    UBYTE vh_Octaves;
    UBYTE vh_Compression;
    ULONG vh_Volume;
};

#define ID_8SVX MAKE_ID('8','S','V','X')
#define ID_VHDR MAKE_ID('V','H','D','R')
#define ID_BODY MAKE_ID('B','O','D','Y')

/*****************************************************************************/

/* Fibonacci delta encoding for sound data */
BYTE far codeToDelta[16] =
{-34, -21, -13, -8, -5, -3, -2, -1, 0, 1, 2, 3, 5, 8, 13, 21};

/*****************************************************************************/

/* Unpack Fibonacci-delta encoded data from n byte source
 * buffer into 2*n byte dest buffer, given initial data
 * value x.  It returns the lats data value x so you can
 * call it several times to incrementally decompress the data.
 */

BYTE D1Unpack (BYTE source[], LONG n, BYTE dest[], BYTE x)
{
    UBYTE d;
    LONG i, lim;

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

int main (int argc, char **argv)
{
    struct StoredProperty *sp;
    struct ContextNode *cn;
    struct VoiceHeader vh;
    struct IFFHandle *iff;
    APTR sample = NULL;
    ULONG sampleLength;
    ULONG sampleperiod;
    BPTR fp;
    APTR data;
    STRPTR name;

    struct Sample s;
    BPTR fh;

    BOOL got = FALSE;

    IFFParseBase = OpenLibrary ("iffparse.library", 0);

    if (argc > 1)
    {
	name = argv[1];

	if (fp = Open (name, MODE_OLDFILE))
	{
	    if (iff = AllocIFF ())
	    {
		iff->iff_Stream = fp;
		InitIFFasDOS (iff);

		if (!OpenIFF (iff, IFFF_READ))
		{
		    if (!PropChunk (iff, ID_8SVX, ID_VHDR))
		    {
			if (!StopChunk (iff, ID_8SVX, ID_BODY))
			{
			    if (!StopOnExit (iff, ID_8SVX, ID_FORM))
			    {
				if (!ParseIFF (iff, IFFPARSE_STEP))
				{
				    while (TRUE)
				    {
					if (ParseIFF (iff, IFFPARSE_SCAN))
					    break;

					if (!(sp = FindProp (iff, ID_8SVX, ID_VHDR)))
					    break;

					vh = *((struct VoiceHeader *) sp->sp_Data);

					printf ("one sht: %ld\n", vh.vh_OneShotHiSamples);
					printf (" repeat: %ld\n", vh.vh_RepeatHiSamples);
					printf ("octaves: %d\n", vh.vh_Octaves);

					cn = CurrentChunk (iff);

					sampleLength = cn->cn_Size;
					if (vh.vh_Compression == CMP_FIBDELTA)
					    sampleLength = (sampleLength - 2) * 2;

					if (!(sample = AllocVec (sampleLength, MEMF_CLEAR | MEMF_CHIP)))
					    break;

					data = (APTR) ((ULONG) sample + sampleLength - cn->cn_Size);

					if (ReadChunkBytes (iff, data, cn->cn_Size) != cn->cn_Size)
					{
					    FreeVec (sample);
					    break;
					}

					if (vh.vh_Compression == CMP_FIBDELTA)
					    DUnpack (data, cn->cn_Size, sample);
				    }

				    got = TRUE;

				    if (fh = Open ("stripsnd.out", MODE_NEWFILE))
				    {
					s.s_Length = sampleLength;
					s.s_Period = (ULONG) vh.vh_SamplesPerSec;

					printf (" length: %ld\n", s.s_Length);
					printf (" period: %ld (%ld)\n", s.s_Period, sampleperiod);

					if (Write (fh, &s, sizeof (struct Sample)) == sizeof (struct Sample))
					{
					    if (Write (fh, sample, sampleLength) == sampleLength)
					    {
						printf ("successful\n");
					    }
					    else
					    {
						printf ("couldn't write data\n");
					    }
					}
					else
					    printf ("couldn't write header\n");

					Close (fh);
				    }
				}
			    }
			}
		    }
		    CloseIFF (iff);
		}
		FreeIFF (iff);
	    }

	    if (!got)
	    {
		/* Seek back to the beginning */
		if (Seek (fp, 0, OFFSET_END) >= 0)
		{
		    if ((sampleLength = Seek (fp, 0, OFFSET_BEGINNING)) > 0)
		    {
			if (sample = AllocVec (sampleLength, NULL))
			{
			    if (Read (fp, sample, sampleLength) == sampleLength)
			    {
				    if (fh = Open ("stripsnd.out", MODE_NEWFILE))
				    {
					s.s_Length = sampleLength;
					s.s_Period = 320;

					printf (" length: %ld\n", s.s_Length);
					printf (" period: %ld (%ld)\n", s.s_Period, sampleperiod);

					if (Write (fh, &s, sizeof (struct Sample)) == sizeof (struct Sample))
					{
					    if (Write (fh, sample, sampleLength) == sampleLength)
					    {
						printf ("successful\n");
					    }
					    else
					    {
						printf ("couldn't write data\n");
					    }
					}
					else
					    printf ("couldn't write header\n");

					Close (fh);
				    }
				    else
					printf ("couldn't create out file\n");
			    }
			    else
				printf ("couldn't read %ld bytes\n", sampleLength);

			    FreeVec (sample);
			}
			else
			    printf ("couldn't allocated %ld bytes\n", sampleLength);
		    }
		    else
			printf ("couldn't get size\n");
		}
		else
		    printf ("couldn't seek to end\n");
	    }

	    Close (fp);
	}
	else
	    printf ("couldn't open '%s'\n", name);
    }
    CloseLibrary (IFFParseBase);
}
