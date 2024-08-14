#ifndef	LIBBASE_H
#define	LIBBASE_H

/*****************************************************************************/

#include <exec/types.h>
#include <exec/ports.h>
#include <exec/memory.h>
#include <exec/lists.h>
#include <exec/semaphores.h>
#include <exec/execbase.h>
#include <exec/interrupts.h>
#include <devices/cd.h>
#include <utility/tagitem.h>

#include <clib/macros.h>
#include <clib/exec_protos.h>
#include <clib/utility_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/utility_pragmas.h>

/*****************************************************************************/

struct LibBase
{
    struct Library		 lb_Lib;
    UWORD			 lb_Pad;
    struct Library		*lb_SysBase;
    struct Library		*lb_UtilityBase;
    ULONG			*lb_SegList;

    /* For the expunge handler */
    struct Interrupt		 lb_IS;
    struct SignalSemaphore	 lb_Lock;
    struct MinList		 lb_HandleList;

    /* Remember the CD-I information */
};

/*****************************************************************************/

#define SysBase			lb->lb_SysBase
#define UtilityBase		lb->lb_UtilityBase

/*****************************************************************************/

struct KarInfoHeader
{
    UBYTE kih_IdString[11];	/* Identification string "KARINFO.BIH" */
    UBYTE kih_NumCoded;		/* Number of code text files */
    UBYTE kih_NumTracks;	/* Number of tracks on the disc */
    UBYTE kih_Reserved[19];	/* Reserved bytes */
};

/*****************************************************************************/

struct KarInfoSeq
{
    UBYTE kis_CharSet;		/* Character set code */
    UBYTE kis_Country[2];	/* Country code */
    UBYTE kis_Reserved1[2];	/* Reserved bytes */
    UBYTE kis_Offset;		/* File offset */
    UBYTE kis_Length;		/* File length */
    UBYTE kis_Flags[8];		/* Disc and sequence information flags */
    UBYTE kis_Reserved2[17];	/* Reserved bytes */
};

/*****************************************************************************/

struct InfoVCD
{
    UBYTE			 iv_SysID[8];		/* VIDEO_CD */
    UBYTE			 iv_Version[2];		/* Version number */
    UBYTE			 iv_AlbumID[16];	/* Album ID */
    UWORD			 iv_NumVolumes;		/* Number of volumes */
    UWORD			 iv_Volume;		/* Volume ID */
};

/*****************************************************************************/

struct Entry
{
    UBYTE			 e_Track;		/* Track */
    UBYTE			 e_MSF[3];		/* mm:ss:ff address */
};

/*****************************************************************************/

struct EntriesVCD
{
    UBYTE			 ev_SysID[8];		/* ENTRYVCD */
    UBYTE			 ev_Version[2];
    UWORD			 ev_NumEntries;		/* Number of entries */
    ULONG			 ev_Entries[500];
//    struct Entry		 ev_Entries[500];	/* Array of entries */
};

/*****************************************************************************/

struct CDIVolume
{
    UBYTE		 cd_Type;		/*   1: Record Type 0x01 */
    UBYTE		 cd_ID[5];		/*   2: Standard ID */
    UBYTE		 cd_Version;		/*   7: Version number */
    UBYTE		 cd_Flags;		/*   8: Volume flags */
    UBYTE		 cd_SystemID[32];	/*   9: System ID */
    UBYTE		 cd_VolumeID[32];	/*  41: Volume ID */
    UBYTE		 cd_Pad1[12];		/*  73: Reserved */
    ULONG		 cd_Space;		/*  85: Volume space */
    UBYTE		 cd_CharSet[32];	/*  89: Code Character Set */
    UBYTE		 cd_Pad2[2];		/* 121: Reserved */
    UWORD		 cd_NumVolumes;		/* 123: Number of volumes */
    UBYTE		 cd_Pad3[2];		/* 125: Reserved */
    UWORD		 cd_Volume;		/* 127: Volume number */
    UBYTE		 cd_Pad4[2];		/* 129: Reserved */
    UWORD		 cd_BlockSize;		/* 131: Block size */
    UBYTE		 cd_Pad5[4];		/* 133: Reserved */
    ULONG		 cd_PathTableSize;	/* 137: Path Table Size */
    UBYTE		 cd_Pad6[8];		/* 141: Reserved */
    ULONG		 cd_PathTableAddress;	/* 149: Path Table Address */
    UBYTE		 cd_Pad7[38];		/* 153: Reserved */
    UBYTE		 cd_AlbumID[128];	/* 191: Album Identifier */
    UBYTE		 cd_PublishID[128];	/* 319: Publisher ID */
    UBYTE		 cd_DataPrepID[128];	/* 447: Data Preparer ID */
    UBYTE		 cd_ApplicationID[128];	/* 575: Application ID */
    UBYTE		 cd_Copyright[32];	/* 703: Copyright file name */
    UBYTE		 cd_Pad8[5];		/* 735: Reserved */
    UBYTE		 cd_Abstract[32];	/* 740: Abstract file name */
};

/*****************************************************************************/

struct PathTable
{
    UBYTE pt_Length;		/*  1: Name length */
    UBYTE pt_EALength;		/*  2: Extended Attribute record length */
    ULONG pt_DirBlock;		/*  3: Directory block address */
    UWORD pt_ParentDir;		/*  7: Parent directory number */
};

/*****************************************************************************/

struct DirRecord
{
    UBYTE dr_Length;		/*   1: Record length */
    UBYTE dr_EALength;		/*   2: Extended Attribute record length */
    UBYTE dr_Pad1[4];		/*   3: Reserved */
    ULONG dr_Block;		/*   7: File beginning LBN */
    UBYTE dr_Pad2[4];		/*  11: Reserved */
    ULONG dr_Size;		/*  15: File Size */
    UBYTE dr_Create[6];		/*  19: Creation Date */
    UBYTE dr_Pad3;		/*  25: Reserved */
    UBYTE dr_Flags;		/*  26: File Flags */
    UWORD dr_Interleave;	/*  27: Interleave */
    UBYTE dr_Pad4[2];		/*  29: Reserved */
    UWORD dr_Sequence;		/*  31: Album Sequence number */

    /* Next comes the name size (UBYTE) and the name */
};

/*****************************************************************************/

#define	NUM_TOC_ENTRIES		100

/*****************************************************************************/

struct VideoCDHandle
{
    struct MinNode		 vh_Node;			/* Entry in the handle list */
    struct SignalSemaphore	 vh_Lock;			/* Lock for expunge */
    ULONG			 vh_Flags;			/* Control information */

    struct MsgPort		*vh_MP;				/* Message port */
    struct IOStdReq		*vh_IO;				/* IO request */
    struct CDInfo		 vh_CDI;			/* CD information packet */
    union CDTOC			 vh_TOC[NUM_TOC_ENTRIES];	/* Table of contents */
    ULONG			 vh_Tracks;			/* Number of tracks in contents */
    ULONG			 vh_SectorSize;			/* Sector size */
    ULONG			 vh_Offset;			/* Offset */

    /* Karaoke CD information */
    STRPTR			 vh_Buffer;			/* Basic information */
    struct KarInfoHeader	*vh_KIH;
    UBYTE			 vh_CharSet;			/* Character set to use */
    UBYTE			 vh_CCOffset;
    UBYTE			 vh_CCLength;
    UBYTE			 vh_Pad;
    STRPTR			 vh_CCBuff;			/* CC Buffer */
#define	MAX_TEXT	8192
    STRPTR			 vh_Text;			/* Text work buffer */

    /* Video CD information */
    struct InfoVCD		*vh_IVCD;
    struct EntriesVCD		*vh_EVCD;
    ULONG			*vh_EArray;

    /* CDI Information */
    struct CDIVolume		*vh_CDIV;			/* CD-I Volume label */
    ULONG			 vh_CDILength;			/* Length of the track */
    struct MinList		*vh_CDITrackList;		/* Digital Video track list */
    ULONG			 vh_CDITracks;			/* Number of digital video tracks */
};

/*****************************************************************************/

#define	VCDF_KARINFO	(1L<<0)
#define	VCDF_INFOVCD	(1L<<1)
#define	VCDF_ENTRYVCD	(1L<<2)
#define	VCDF_CDIFMV	(1L<<3)

/*****************************************************************************/

extern void kprintf (void *, ...);
extern void NewList (struct List *);

/*****************************************************************************/

#define MEMORY_FOLLOWING(ptr)	((void *)((ptr)+1))
#define MEMORY_N_FOLLOWING(ptr,n)  ((void *)( ((ULONG)ptr) + n ))
#define ASM				__asm
#define REG(x)				register __ ## x
#define	MSFtoLSNoffset(m,s,f,ofs,ss)	((((((ULONG)m)*60*75)+(((ULONG)s)*75)+((ULONG)f))*((ULONG)ss))-((ULONG)ofs))
#define	MSFtoLSN(m,s,f)			((((ULONG)m)*60*75)+(((ULONG)s)*75)+((ULONG)f))

/*****************************************************************************/

#include "lib_iprotos.h"

/*****************************************************************************/

#endif
