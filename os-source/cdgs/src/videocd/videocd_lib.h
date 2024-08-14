#ifndef	LIBRARIES_VIDEOCD_H
#define	LIBRARIES_VIDEOCD_H

/*
**	$Id: videocd_lib.h,v 40.6 93/11/19 15:41:12 davidj Exp $
**
**	(C) Copyright 1993 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/

/*****************************************************************************/

#ifndef	EXEC_NODES_H
#include <exec/nodes.h>
#endif

#ifndef	UTILITY_TAGITEMS_H
#include <utility/tagitem.h>
#endif

/*****************************************************************************/

/* Types returned by GetCDTypeA() */
#define	CDT_ERROR	-1
#define	CDT_UNKNOWN	0
#define	CDT_MUSIC	1
#define	CDT_KARAOKE	2
#define	CDT_GENERIC	3
#define	CDT_VIDEOCD	4
#define	CDT_CDI		5
#define	CDT_CDIFMV	6

/*****************************************************************************/

struct CDText
{
    ULONG		 cs_Length;	/* Number of characters in string */
    STRPTR		 cs_Text;	/* Pointer to the string */
};

/*****************************************************************************/

/* Tags used by GetVideoCDInfo() */
#define	GVCD_Dummy			(TAG_USER+0x1000)

#define	GVCD_ErrorCode			(GVCD_Dummy+1)
    /* (ULONG *) Pointer to error code storage location */
#define	GVCD_GetSequenceOffset		(GVCD_Dummy+2)
    /* (BOOL) Used to obtain the sequence sector offset */

/* Code Set Information */
#define	GVCD_CodeSet			(GVCD_Dummy+5)
    /* (ULONG) Code Set */

#define	VCCS_ISO_646		0x01
#define	VCCS_ISO_8859_1		0x02
#define	VCCS_JIS		0x03
#define	VCCS_SHIFT_JIS		0x04

#define	GVCD_GetCodeSets		(GVCD_Dummy+6)
    /* (BOOL) Used to obtain a list of available code sets */
#define	GVCD_NumCodeSets		(GVCD_Dummy+7)
    /* (ULONG) Number of available code sets */
#define	GVCD_CodeSets			(GVCD_Dummy+8)
    /* (struct MinList *) List of available code sets */

/* Disc Information */
#define	GVCD_DiscTitle			(GVCD_Dummy+10)
    /* (struct CDText *) Title of the disc */
#define	GVCD_DiscCatalogNum		(GVCD_Dummy+11)
    /* (struct CDText *) Disc catalog number */
#define	GVCD_DiscNumSequences		(GVCD_Dummy+12)
    /* (ULONG) Number of sequences on the disc */
#define	GVCD_DiscDescription		(GVCD_Dummy+13)
    /* (struct CDText *) Disc description */
#define	GVCD_DiscUser4			(GVCD_Dummy+14)
#define	GVCD_DiscUser5			(GVCD_Dummy+15)
#define	GVCD_DiscUser6			(GVCD_Dummy+16)
#define	GVCD_DiscUser7			(GVCD_Dummy+17)
    /* (struct CDText *) User definable terms */

/* Sequence Information */
#define	GVCD_SequenceISRCCode		(GVCD_Dummy+18)
    /* (struct CDText *) ISRC code of the sequence */
#define	GVCD_SequenceTitle		(GVCD_Dummy+19)
    /* (struct CDText *) Sequence title */
#define	GVCD_SequenceTitleSort		(GVCD_Dummy+20)
    /* (struct CDText *) Sequence title for sorting */
#define	GVCD_SequencePerformer		(GVCD_Dummy+21)
    /* (struct CDText *) Performers name */
#define	GVCD_SequencePerformerSort	(GVCD_Dummy+22)
    /* (struct CDText *) Performers name for sorting */
#define	GVCD_SequenceWriter		(GVCD_Dummy+23)
    /* (struct CDText *) Name of the song writer(s) */
#define	GVCD_SequenceComposer		(GVCD_Dummy+24)
    /* (struct CDText *) Name of the composer(s) */
#define	GVCD_SequenceArranger		(GVCD_Dummy+25)
    /* (struct CDText *) Name of the arranger(s) */
#define	GVCD_SequencePlayer		(GVCD_Dummy+26)
    /* (struct CDText *) Name of the player(s) */
#define	GVCD_SequenceTextHeader		(GVCD_Dummy+27)
    /* (struct CDText *) Header part of the song text */
#define	GVCD_SequenceText		(GVCD_Dummy+28)
    /* (struct CDText *) Song text */
#define	GVCD_SequenceKaraokiKey		(GVCD_Dummy+29)
    /* (struct CDText *) Karaoke key */
#define	GVCD_SequenceOriginalKey	(GVCD_Dummy+30)
    /* (struct CDText *) Original key */
#define	GVCD_SequenceDescription	(GVCD_Dummy+31)
    /* (struct CDText *) Description of the contents of the sequence */
#define	GVCD_SequenceUser22		(GVCD_Dummy+32)
#define	GVCD_SequenceUser23		(GVCD_Dummy+32)
#define	GVCD_SequenceUser24		(GVCD_Dummy+32)
#define	GVCD_SequenceUser25		(GVCD_Dummy+32)
#define	GVCD_SequenceUser26		(GVCD_Dummy+32)
#define	GVCD_SequenceUser27		(GVCD_Dummy+32)
#define	GVCD_SequenceUser28		(GVCD_Dummy+32)
#define	GVCD_SequenceUser29		(GVCD_Dummy+32)
#define	GVCD_SequenceUser30		(GVCD_Dummy+32)
#define	GVCD_SequenceUser31		(GVCD_Dummy+32)
    /* (struct CDText *) User definable items */

#define	GVCD_SequenceStartSector	(GVCD_Dummy+40)
    /* (ULONG) Sector offset of the sequence */
#define	GVCD_SequenceEndSector		(GVCD_Dummy+41)


/* info.vcd information */
#define	GVCD_InfoVersion		(GVCD_Dummy+101)
    /* Specification version number */
#define	GVCD_InfoAlbumID		GVCD_DiscTitle
    /* Album identification */
#define	GVCD_InfoNumberVolumes		(GVCD_Dummy+103)
    /* Number of volumes in album */
#define	GVCD_InfoVolumeNumber		(GVCD_Dummy+104)
    /* Volume number */

/* entries.vcd information */
#define	GVCD_EntriesVersion		(GVCD_Dummy+111)
    /* Specification version number */
#define	GVCD_EntriesUsed		(GVCD_Dummy+112)
    /* Entries used */
#define	GVCD_EntriesArray		(GVCD_Dummy+113)
    /* NULL terminated array of sector addresses */

#define	GVCD_DigitalVideoTrackList	(GVCD_Dummy+114)
    /* (struct MinList *) of MTracks */

#define	GVCD_NumDigitalVideoTracks	(GVCD_Dummy+115)
    /* (ULONG) number of digital video tracks */

/*****************************************************************************/

/* Digital Video Track */
struct MTrack
{
    struct MinNode	 mt_Node;
    ULONG		 mt_Block;
    ULONG		 mt_Size;
};

/*****************************************************************************/

struct CodeSetNode
{
    struct MinNode	 csn_Node;
    UBYTE		 csn_CharSet;		/* Character set code */
    UBYTE		 csn_Country[2];	/* Country code */
    UBYTE		 csn_Reserved1[2];	/* Reserved bytes */
    UBYTE		 csn_Offset;		/* File offset */
    UBYTE		 csn_Length;		/* File length */
    UBYTE		 csn_Flags[8];		/* Disc and sequence information flags */
    UBYTE		 csn_Reserved2[17];	/* Reserved bytes */
};

/*****************************************************************************/

#endif
