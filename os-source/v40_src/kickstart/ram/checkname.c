/* checkname.c */

#include "ram_includes.h"
#include "ram.h"

LONG
checkname (name)
	char *name;
{
	short i = 0;
	UBYTE ch;

	fileerr = ERROR_INVALID_COMPONENT_NAME;
	while (i++ <= MAX_FILENAME)
	{
		ch = (UBYTE) *name++;
		if (ch == '\0')
			break;

		if (ch < ' ' || ch == '/' || ch == ':')
			return FALSE;
	}
	if (i == 1)
		return FALSE;	/* no characters! */

	fileerr = 0;
	return TRUE;
}
