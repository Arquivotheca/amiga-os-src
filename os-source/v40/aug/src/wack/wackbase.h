/*
 * Amiga Grand Wack
 *
 * wackbase.h -- Definitions for Wack fake library
 *
 * $Id: wackbase.h,v 39.3 93/11/05 15:05:53 jesup Exp $
 *
 */

#include <exec/types.h>


/* Currently have 17 vectors defined:
 *
 *	VPrintf()
 *	ReadCurrAddr()
 *	WriteCurrAddr()
 *	ReadSpareAddr()
 *	WriteSpareAddr()
 *	ReadByte()
 *	ReadWord()
 *	ReadLong()
 *	ReadBlock()
 *	WriteByte()
 *	WriteWord()
 *	WriteLong()
 *	FindLibrary()
 *	ReadString()
 *	ReadBSTR()
 *	ReadContextAddr()
 *	WriteBlock()
 *	CallFunction()
 */

#define WACK_CUSTOMENTRIES	18

/* Skip 5 vectors: Open, Close, Expunge, Reserved, and ARexx */
#define WACK_SKIPENTRIES 5

#define WACK_ENTRIES ( WACK_SKIPENTRIES + WACK_CUSTOMENTRIES )
#define PAD ( WACK_ENTRIES%2 ? 2 : 0 )

#define WACKTABLESIZE (6 * WACK_ENTRIES + PAD)

struct WackLibBase
{
    struct MsgPort wl_MsgPort;
    char wl_PortNameBuffer[24];
};

struct WackFakeLibrary
{
    UBYTE wfl_JumpTable[ WACKTABLESIZE ];
    struct WackLibBase wfl_LibBase;
};

