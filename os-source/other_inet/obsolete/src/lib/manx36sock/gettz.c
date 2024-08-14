/*
** get timezone
*/

#include <config.h>

get_tz()
{
	register struct config *cf;

	GETCONFIG(cf);
	if(!cf){
		return -1;
	}

	return cf->tz_offset;
}

