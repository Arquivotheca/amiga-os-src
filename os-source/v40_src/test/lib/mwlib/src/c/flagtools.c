/*
    Copyright © 1989 Mitchell/Ware Systems, Inc.

    May be used by Commodore-Amiga, Inc. for in-house purposes only.
    No part of this program, or any parts or modifications thereof, either
    in source-code or object-code, or library form, may be used by
    Commodore, or any of its employees, either indepently, or in conjunction
    with Commordore, as, or as a part of , any public-domain or commercial
    product, program, or any type of software, teaching aid, etc. without
    the prior written permission of Mitchell/Ware Systems.

    This copyright notice must not be removed from the document of which
    it is attached.
*/
/****** MWLib.library/FlagTools ***********************************************

    NAME
        FlagTools -- Cli parameter handling for old Test Suites

    AUTHOR
        Mitchell/Ware Systems & John Szucs

    VERSION
        1.04    9/26/89
        1.05    11/30/89    - Mod by John Szucs to DoFlagState to process
                              multiple states seperated by the '|' character.

        1.06    23 Apr 1992 - Cleanup

    FUNCTION
        Flag Processor for command-line options.

*****i* MWLib.library/FlagTools/Pseudocode ************************************

    ULONG DoFlagBits(keymode_array, string)
            KEYMODE *keymode_array;
            UBYTE *string;

                DoFlagBits require a string of zero or more 2-character
                bit ids

                NOTE: All the entries in keymode_array must be in all
                      uppercase.

    ULONG DoFlagState(keymode_array, string)
            KEYMODE *keymode_array;
            UBYTE *string;

                DoFlagState requires a single-name string representing
                the required state.

                DoFlagState now allows multiple-name strings delimited
                by '|' to allow for multiple states to be ORed toghether. (JJS)

                NOTE: All the entries in keymode_array should be in all
                      lowercase.
******************************************************************************/

#include <exec/types.h>
#include "Tools.h"

ULONG DoFlagBits(struct KeyMode *kma, UBYTE *str)
{
	KEYMODE *k;
	ULONG flags = NULL;
	WORD w, wq;
	char *s;

	if (!(str = s = strdup(str)))
		fatal("DoFlagBits:Out Of Memory");

	strupr(s);
	for (w = *(WORD *)str; *str; str += 2, w = *(WORD *)str)
	{
		for (k = kma, wq = *(WORD *)k->k; k->k; ++k, wq = *(WORD *)k->k)
			if (w == wq)
				{ flags |= k->km; break; }

		if (!k->k)
			fatal2("DoFlagBits: Invalid Flag:", str);
	}

	free(s);
	return flags;
}

/* Internal -- Do not use under penalty of death */
ULONG _DoFlagValue(struct KeyMode *kma, UBYTE *str, WORD start, WORD end)
{
    KEYMODE *k;

    for (k = kma; k->k; ++k)
        if (!strnicmp(&(str[start]), k->k,end-start))
             return(k->km);

    fatal2("DoFlagState: Invalid Screen Mode:", str);

    return(0L); /* to stop the compiler from bitching */
}

ULONG DoFlagState(struct KeyMode *kma, UBYTE *str)
{
    WORD StartOfKW, EndOfKW;
    ULONG Flags=NULL;

    StartOfKW=0;
    for (EndOfKW=0;str[EndOfKW]!='\0';EndOfKW++)
        if (str[EndOfKW]=='|') {
            Flags|=_DoFlagValue(kma,str,StartOfKW,EndOfKW);
            EndOfKW++;
            StartOfKW=EndOfKW;
        }

    EndOfKW--;
    Flags|=_DoFlagValue(kma,str,StartOfKW,EndOfKW);

    return(Flags);
}
