#include <exec/types.h>

#define	FRT_METHOD_DEFAULT	0L

struct FlashROMTable {
	STRPTR	frt_Manufacturer;	/* Name of manufacturer, and relevant info	*/
	ULONG	frt_TotalSize;		/* Total size of this Flash ROM			*/
	ULONG	frt_ZoneSize;		/* Zone Size (must be integer divisible of Total*/
	ULONG	frt_NanoSpeed;		/* Speed of card in nano-seconds		*/
	UWORD	frt_ProgramMethod;	/* Nominal is 0; other methods if needed	*/
	UBYTE	frt_Identifier;		/* Intelligent identifier byte			*/
	UBYTE	frt_DeviceCode;		/* Device code (if supported)			*/
	UBYTE	frt_Reserved;		/* whatever					*/
	UBYTE	frt_NameSpace;		/* Place for manufacturer string .. trailing	*/
};

