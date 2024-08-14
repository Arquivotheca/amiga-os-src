/******************************************************************************
*
*   $Id: startup.c,v 39.17 93/04/15 14:28:43 spence Exp $
*
*   $Locker:  $
*
******************************************************************************/

/* display library initialization */

#include "/modeid.h"
#include "/displayinfo.h"	/* remove this */
#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/memory.h>
/*#include <graphics/gfxbase.h>*/
#include "/gfxbase.h"		/* remove this */
#include "/gfxpragmas.h"

#include "d.protos"
#include "/displayinfo_internal.h"
#include "/macros.h"
#include "init.h"
#include "proginfo.h"

void *getmustmem();

/*#define DEBUG*/

#ifdef DEBUG
#define D(x) {x};
#else
#define D(x)
#endif

int __regargs tag_copy(tag,buf)
struct TagItem *tag;
struct TagItem *buf;
{
	int count = 0;

	while( (++count) && (tag->ti_Tag != TAG_DONE) )
	{
	    /* stupid compiler ! substiture for brain dead switch -- bart */

	    if( tag->ti_Tag == TAG_SKIP )
	    {
		ULONG skip = tag->ti_Data + 1;

		--count; 
		do 
		{
		    count++;
		    if( buf ) *buf++ = *tag; 
		    tag++;
		}   while( --skip );
		continue;
	    } 

	    if( tag->ti_Tag == TAG_MORE )
	    {
		--count; tag = (struct TagItem *)tag->ti_Data;
		continue;
	    } 	

	    /* default case */
	    {
		if( buf ) *buf++ = *tag; tag++;
		continue;
	    }
	}  

	if( buf ) *buf++ = *tag++; /* terminator */

	return( count );
}

void bind_record( argc, argv, record, tags, cn, RawStuff)
int argc;
char *argv[];
struct DisplayInfoRecord *record;
UBYTE tags[];
struct RomCompressedNode *cn;
ULONG RawStuff;
{
    struct TagItem *copytags;
    struct TagItem dummytag;
    struct RawMonitorInfo rawdata;	/* RawMonitor is the largest of the squeezed Raw... structs */
    APTR sqz;			/* squeezed data */
    ULONG data;
    int size;
    UBYTE entry = tags[cn->tag];

    if (cn->tag)
    {
	dummytag.ti_Tag = TAG_MORE;
        switch (RawStuff)
        {
        	case DTAG_DISP: 
		{	/* unsqueeze it */
			struct RawDisplayInfo *rd = (struct RawDisplayInfo *)&rawdata;

			sqz = (APTR)(DispData + entry);
			rd->Header.Length = RAWDISP_SKIP;
			rd->NotAvailable = DI_AVAIL_NOMONITOR;
			if (((struct SqueezedDisplayInfo *)sqz)->PropertyFlags & (DIPF_IS_ECS | DIPF_IS_AA))
			{
				rd->NotAvailable |= DI_AVAIL_NOCHIPS;
			}
			rd->PropertyFlags = ((struct SqueezedDisplayInfo *)sqz)->PropertyFlags;
			rd->Resolution.x = (WORD)(((struct SqueezedDisplayInfo *)sqz)->Resolution.x);
			rd->Resolution.y = (WORD)(((struct SqueezedDisplayInfo *)sqz)->Resolution.y);
			rd->PixelSpeed = (UWORD)(((struct SqueezedDisplayInfo *)sqz)->PixelSpeed);
			if (rd->PropertyFlags & DIPF_IS_PANELLED)
			{
				rd->NumStdSprites = 1;
				rd->RedBits = 2; rd->GreenBits = 3; rd->BlueBits = 1;
			}
			else
			{
				rd->NumStdSprites = 8;
				rd->RedBits = rd->GreenBits = rd->BlueBits = 
				((rd->PixelSpeed == 35) ? 2 : 4);
			}
			rd->PaletteRange = ((struct SqueezedDisplayInfo *)sqz)->PaletteRange;
			rd->SpriteResolution.x = (WORD)(((struct SqueezedDisplayInfo *)sqz)->SpriteResolution.x);
			rd->SpriteResolution.y = (WORD)(((struct SqueezedDisplayInfo *)sqz)->SpriteResolution.y);
			rd->ModeID = ((struct SqueezedDisplayInfo *)sqz)->ModeID;
			rd->pad2[0] = rd->pad2[1] = rd->pad2[2] = rd->pad2[3] = rd->pad2[4] = 0;
			rd->reserved[0] = rd->reserved[1] = 0;
			rawdata.Header.StructID = DTAG_DISP;
			data = (ULONG)rd;
			break;
		}
        	case DTAG_DIMS: 
		{
			struct SqueezedDimensionInfo *sdi;
			struct RawDimensionInfo *rd = (struct RawDimensionInfo *)&rawdata;
			UBYTE DimsRange;

			sqz = (APTR)(DimsData + entry);
			sdi = (struct SqueezedDimensionInfo *)sqz;
			DimsRange = sdi->DimsRange;
			/* Unsqueeze it to Raw data */
			rd->Header.Length = RAWDIMS_SKIP;
			rd->MinRasterHeight = 1;
			rd->MaxRasterWidth = 16368;
			rd->MaxRasterHeight = 16384;
			switch (DimsRange & 7)	/* DIMS_RANGE values = 0-4 */
			{
				case DIMS_RANGE_LORES : rd->MinRasterWidth = 16; break;
				case DIMS_RANGE_HIRES : rd->MinRasterWidth = 32; break;
				case DIMS_RANGE_SHIRES : rd->MinRasterWidth = 64; break;
				case DIMS_RANGE_A2024NTSC : 
				{
					rd->MinRasterWidth = rd->MaxRasterWidth = 1008;
					rd->MinRasterHeight = rd->MaxRasterHeight = 800;
				}
				break;
				case DIMS_RANGE_A2024PAL : 
				{
					rd->MinRasterWidth = rd->MaxRasterWidth = 1008;
					rd->MinRasterHeight = rd->MaxRasterHeight = 1024;
				}
				break;
			}
			rd->MaxDepth = (UWORD)(((struct SqueezedDimensionInfo *)sqz)->MaxDepth);
			rd->HWMaxDepth = ((DimsRange & DIMS_DEPTH_HW6) ? 6 : 8);

			rd->Nominal.MinX = (LONG)sdi->Nominal.MinX;
			rd->Nominal.MinY = (LONG)sdi->Nominal.MinY;
			rd->Nominal.MaxX = (LONG)sdi->Nominal.MaxX;
			rd->Nominal.MaxY = (LONG)sdi->Nominal.MaxY;

			rd->MaxOScan.MinX = (LONG)sdi->MaxOScan.MinX;
			rd->MaxOScan.MinY = (LONG)sdi->MaxOScan.MinY;
			rd->MaxOScan.MaxX = (LONG)sdi->MaxOScan.MaxX;
			rd->MaxOScan.MaxY = (LONG)sdi->MaxOScan.MaxY;

			rd->VideoOScan.MinX = (LONG)sdi->VideoOScan.MinX;
			rd->VideoOScan.MinY = (LONG)sdi->VideoOScan.MinY;
			rd->VideoOScan.MaxX = (LONG)sdi->VideoOScan.MaxX;
			rd->VideoOScan.MaxY = (LONG)sdi->VideoOScan.MaxY;

			rd->pad[0] = rd->pad[1] = rd->pad[2] = rd->pad[3] = rd->pad[4] = 0;
			rd->reserved[0] = rd->reserved[1] = 0;
			rawdata.Header.StructID = DTAG_DIMS;
			data = (ULONG)&rawdata;
       			break;
       		}
        	case DTAG_MNTR: data = (ULONG)(MntrData + entry); break;
        	case DTAG_VEC:
		{	/* unsqueeze it */
			struct RawVecInfo *rd = (struct RawVecInfo *)&rawdata;

			rd->Header.Length = RAWVEC_SKIP;
			rd->Vec = rd->Data = NULL;
			rd->Type = entry;
			rd->pad[0] = rd->pad[1] = rd->pad[2] = 0;
			rd->reserved[0] = rd->reserved[1] = 0;
			rawdata.Header.StructID = DTAG_VEC;
			data = (ULONG)&rawdata;
			break;
		}
	};
	rawdata.Header.DisplayID = INVALID_ID;
	rawdata.Header.SkipID = TAG_SKIP;
	dummytag.ti_Data = data;
    }
    else
    {
	dummytag = NullData[0];
    }

    D(kprintf("bind_record(). ti_Tag = 0x%lx, ti_Data - 0x%lx\n", dummytag.ti_Tag, dummytag.ti_Data);)

    size = ( tag_copy( &dummytag, NULL ) * sizeof( struct TagItem ) );
    copytags = getmustmem( size, MEMF_PUBLIC );

    if( copytags )
    {
	int (*doit)() = NULL;

	tag_copy( &dummytag, copytags ); /* decompress data */

	if(argc && (size > sizeof(struct TagItem)))
	{
	    for( ;(doit = (int (*)())(*argv)) && (argc--); argv++) 
	    {
		(*doit)(copytags); /* massage chunkdata */
	    }
	}

	gfx_AddDisplayInfoData(record, (UBYTE *)copytags,size,copytags->ti_Tag,INVALID_ID);

	freemustmem( copytags, size );    /* delete temporary copy */
    }
}

struct DisplayInfoRecord __regargs *new_record(cn)
struct RomCompressedNode *cn;
{
    struct DisplayInfoRecord *record = (struct DisplayInfoRecord *)
    AllocMem(sizeof(struct DisplayInfoRecord),MEMF_PUBLIC|MEMF_CLEAR);

    if( record ) 
    {
	record->rec_MinorKey = cn->cid;
    }

    return( record );
}

int __regargs decompress(argc,argv,parent, tags, cn, RawStuff)
int argc;
char *argv[];
struct DisplayInfoRecord *parent;
UBYTE tags[];
struct RomCompressedNode *cn;
ULONG RawStuff;
{
	int count = 0;
	BYTE num = cn->cno;

	if(parent)
	{
		while(!(num-- < 0 ))
		{
			struct DisplayInfoRecord *child = find_key( (struct DisplayInfoRecord *)(SubRecord(parent)), cn->cid, (UWORD)~0 );

			if((child) || (child = add_record( parent, new_record(cn) )))
			{
				count++;

				bind_record( argc, argv, child, tags, cn, RawStuff );

				while(!(num-- < 0 ))
				{
					count += decompress( argc, argv, child, tags, cn+count, RawStuff );
				}
			}
		}
	}
	return( count );
}

int __regargs new_display(argc, argv, data, tags, root, RawStuff)
int argc;
char *argv[];
struct RomCompressedNode data[];
UBYTE tags[];
struct DisplayInfoRecord *root;
ULONG RawStuff;
{
	int total = 0;

	total += decompress(argc,argv,root,tags,data, RawStuff);
	return(total); 
}

void make_aa(dinfo)
struct RawDisplayInfo *dinfo;
{
    if ((GBASE->ChipRevBits0 & GFXF_AA_LISA) &&
        (!(dinfo->PropertyFlags & DIPF_IS_PANELLED)))	/* don't do for non-AA and A2024 */
    {
	/* New for AA - Attached sprites now work on all modes (not true of ECS).
	 * Sprite resolution can be 140, 70 or 35ns.
	 */

	if (dinfo->PropertyFlags & DIPF_IS_SPRITES)
	{
		dinfo->PropertyFlags |= (DIPF_IS_SPRITES_ATT | DIPF_IS_SPRITES_CHNG_RES |
		DIPF_IS_SPRITES_BORDER | DIPF_IS_SPRITES_CHNG_BASE | DIPF_IS_SPRITES_CHNG_PRI);
	}

	/* Palette Range is no longer representative. The best we can make it
	 * is 0xFFFF.
	 * NB - need to avoid EHB special case
	 */
	dinfo->PaletteRange = 0xffff;

	dinfo->RedBits = dinfo->GreenBits = dinfo->BlueBits = 8;
    }
}

void make_pf2pri(dinfo)
struct RawDisplayInfo *dinfo;
{
    dinfo->PropertyFlags |= DIPF_IS_PF2PRI;
    dinfo->ModeID |= PFBA;
    make_aa(dinfo);
}

void make_dualpf(dinfo)
struct RawDisplayInfo *dinfo;
{
    dinfo->PropertyFlags |= DIPF_IS_DUALPF;
    dinfo->PropertyFlags &= ~DIPF_IS_WB;
    dinfo->ModeID |= DUALPF;
    make_aa(dinfo);
}

void make_lace(dinfo)
struct RawDisplayInfo *dinfo;
{
    dinfo->PropertyFlags |= DIPF_IS_LACE;
    dinfo->ModeID |= LACE;
    dinfo->Resolution.y >>= 1;
    make_aa(dinfo);
}

void make_sdbl(dinfo)
struct RawDisplayInfo *dinfo;
{
    dinfo->PropertyFlags |= (DIPF_IS_SCANDBL | DIPF_IS_AA);
    dinfo->PropertyFlags &= ~DIPF_IS_WB;
    dinfo->ModeID |= DOUBLESCAN;
    dinfo->Resolution.y <<= 1;
    make_aa(dinfo);
}

void (*massage_dinfo[])() =
{
    make_pf2pri,
    make_dualpf,
    make_lace,
    make_aa,
    make_sdbl,
};

void limit_blit(dims)
struct RawDimensionInfo *dims;
{
    if(!(GBASE->ChipRevBits0 & GFXF_BIG_BLITS))
    {
	dims->MaxRasterWidth  = 1008;
	dims->MaxRasterHeight = 1024;
    }
}

void increase_depth(dims)
struct RawDimensionInfo *dims;
{
    UWORD depth;

    /* MaxDepth of this mode is dims->MaxDepth * BandWidth available in
     * GB->MemType
     * ie if MemType = 0, *1
     *                 1, *2
     *                 2, *2
     *                 3, *4
    */

    depth = dims->MaxDepth << GBASE->bwshifts[GBASE->MemType];
    dims->MaxDepth = MIN(depth, (dims->HWMaxDepth ? dims->HWMaxDepth : 8));
}

void (*massage_dims[])() =
{
    limit_blit,
    increase_depth,
};

void correct_driver(vinfo)
struct RawVecInfo *vinfo;
{
    struct ProgInfo *pinfo;
    UBYTE crb = GBASE->ChipRevBits0;

    pinfo = vinfo->Data = (APTR)&ProgData[vinfo->Type];

    if (crb & GFXF_AA_LISA)
    {
	/* Use AA Vectors */
	vinfo->Vec = (APTR)(VecLists[pinfo->MakeItType]);
    }
    else
    {
	if (crb & GFXF_HR_DENISE)
	{
		/* Use ECS Vectors */
		vinfo->Vec = (APTR)(VecLists[pinfo->MakeItType + AAVECS]);
	}
	else
	{
		/* Use A Vectors */
		vinfo->Vec = (APTR)(VecLists[pinfo->MakeItType + AAVECS + ECSVECS]);
	}
    }
}


void (*massage_pinfo[])() =
{
    correct_driver,
};

void __regargs do_db_startup(root, tags, entry)
struct DisplayInfoRecord *root;
UBYTE tags[];
int entry;
{
    struct RomCompressedNode *data;
    BOOL isaa = (GBASE->ChipRevBits0 & GFXF_AA_MLISA);

    if (entry > ROM_MONITOR_ENTRIES)
	return;		/* protect against adding monitor from disk - spence Mar 28 1991 */

    D(kprintf("do_db_startup(). entry = 0x%lx tags = 0x%lx\n", entry, tags);)

    data = DataBaseStuff[entry];

    Forbid();

    /* mntr and a2024 */
    data += new_display( 0, NULL, data,
		 tags,
	         root, DTAG_MNTR );

    /* dims */ 
    data += new_display(2, (char **)(&massage_dims[0]), data,
		 tags,
	         root, DTAG_DIMS );
    if (isaa)
    {
	data += new_display(2, (char **)(&massage_dims[0]), data,
			 tags,
	        	 root, DTAG_DIMS );
    }
    else
    {
	data += (data->cno + 1);
    }

    /* normal */
    data += new_display( 1, (char **)(&massage_dinfo[3]), data,
		 tags,
	         root, DTAG_DISP );

    /* lace */
    data += new_display( 1, (char **)(&massage_dinfo[2]), data,
		 tags,
	         root, DTAG_DISP );

    /* dualpf */
    data += new_display( 1, (char **)(&massage_dinfo[1]), data,
		 tags,
	         root, DTAG_DISP );

    /* lace.dualpf */
    data += new_display( 2, (char **)(&massage_dinfo[1]), data,
		 tags,
	         root, DTAG_DISP );

    /* dualpf.pf2pri */
    data += new_display( 2, (char **)(&massage_dinfo[0]), data,
		 tags,
	         root, DTAG_DISP );

    /* lace.dualpf.pf2pri */
    data += new_display( 3, (char **)(&massage_dinfo[0]), data,
		 tags,
	         root, DTAG_DISP );

    /* ham */
    data += new_display( 1, (char **)(&massage_dinfo[3]), data,
		 tags,
	         root, DTAG_DISP );

    /* ham.lace  */
    data += new_display( 1, (char **)(&massage_dinfo[2]), data,
		 tags,
	         root, DTAG_DISP );

    /* extrahalfbrite */
    data += new_display( 1, (char **)(&massage_dinfo[3]), data,
		 tags,
	         root, DTAG_DISP );

    /* extrahalfbrite .lace  */
    data += new_display( 1, (char **)(&massage_dinfo[2]), data,
		 tags,
	         root, DTAG_DISP );

    /* ProgInfo */
    data += new_display( 1, (char **)(&massage_pinfo[0]), data,
                 tags,
                 root, DTAG_VEC );

    if (isaa)
    {
	/* aa.sdbl */
	data += new_display( 1, (char **)(&massage_dinfo[4]), data,
			 tags,
			 root, DTAG_DISP );

	/* aa.ham */
	data += new_display( 1, (char **)(&massage_dinfo[3]), data,
			 tags,
	        	 root, DTAG_DISP );

	/* aa.ham.lace  */
	data += new_display( 1, (char **)(&massage_dinfo[2]), data,
			 tags,
	        	 root, DTAG_DISP );

	/* aa.ham.sdbl  */
	data += new_display( 1, (char **)(&massage_dinfo[4]), data,
			 tags,
	        	 root, DTAG_DISP );

	/* aa.extrahalfbrite */
	data += new_display( 1, (char **)(&massage_dinfo[3]), data,
		 tags,
	         root, DTAG_DISP );

	/* aa.extrahalfbrite .lace  */
	data += new_display( 1, (char **)(&massage_dinfo[2]), data,
		 tags,
	         root, DTAG_DISP );
	/* aa.extrahalfbrite .sdbl  */
	data += new_display( 1, (char **)(&massage_dinfo[4]), data,
		 tags,
	         root, DTAG_DISP );

	/* aa.ProgInfo */
	data += new_display( 1, (char **)(&massage_pinfo[0]), data,
        	     tags,
	             root, DTAG_VEC );
    }

    Permit();

    return;
}


struct DisplayInfoRecord *db_startup()
{
    UBYTE *tags = PalDisplayTags;
    struct DisplayInfoRecord *root = (struct DisplayInfoRecord *)
    AllocMem(sizeof(struct DisplayInfoRecord),MEMF_PUBLIC|MEMF_CLEAR);
	int entry = PAL_ENTRY;

    D(kprintf("db_startup(). MntrData = 0x%lx, DimsData - 0x%lx, NullData = 0x%lx\n", MntrData, DimsData, NullData);)

    /*	 check pal once */
    if (!(GBASE->DisplayFlags & PAL))
	{
		tags = NtscDisplayTags;
		entry = NTSC_ENTRY;
	};

	do_db_startup(root, tags, DEFAULT_ENTRY);
	do_db_startup(root, tags, entry);

    D(kprintf("Goodbye, db_startup().\n");)

    return( root );
}

