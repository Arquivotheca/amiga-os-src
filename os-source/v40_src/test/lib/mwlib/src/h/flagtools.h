/****i* MWLib.header/FlagTools ************************************************

    NAME
        FlagTools -- struct(s) for FlagTools

    VERSION
        1.00    23 Apr 1992 - Converted to modern library.

    AUTHOR
        Mitchell/Ware Systems and John Szucs

******************************************************************************/

#ifndef _FLAG_TOOLS_H_
#define _FLAG_TOOLS_H_ 1

#include <exec/types.h>

typedef struct KeyMode {    // used by modules that require multiple flags
	char *k;    // string - after the last entry, add one more and make this NULL
	ULONG km;   // bit pattern/value associated with this string
	} KEYMODE, KeyMode;

#endif
