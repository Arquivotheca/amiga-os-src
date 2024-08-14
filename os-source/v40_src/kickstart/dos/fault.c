/* fault.c */

#include <exec/types.h>
#include "dos/dosextens.h"

#include "klib_protos.h"

extern UBYTE dos_errstrs[];

char * ASM
mygetstring (REG(d1) LONG code)
{
	struct ErrorString *es;
	UBYTE *ptr;
	LONG curr,*entry,end;

	es    = dosbase()->dl_Errors;
	ptr   = es->estr_Strings;
	entry = es->estr_Nums;

	do {
		end = *(entry+1);
		for (curr = *entry; curr <= end; curr++)
		{
			if (curr == code)
			{
				return (char *) ptr+1;
			}
			ptr = ptr + 1 + *ptr;	/* offset to next string */
		}
		entry += 2;
	} while (*entry);

	return NULL;	
}

LONG errornums[] = {
	-162,-100,
	103,103,
	105,105,
	114,122,
	202,207,
	209,226,
	232,236,
	240,243,
	303,305,
	0,
};

struct ErrorString dos_errors = {
	errornums,
	dos_errstrs
};

#ifdef OLD_FAULT
/* move to asm */
struct ErrorString dos_errors[] = {
-130, "Unable to create background process\n",
-129, "New CLI process %ld\n",
-128, "Cannot open From file %s\n",
-127, "Suspend|Reboot",
-126, "Retry|Cancel",
-125, "too many > or <",
-124, "Command too long",
-123, "Shell error:",
-122, "Error in command name",
-121, "Unknown command",
-120, "Unable to load",
-119, "syntax error",
-118, "unable to open redirection file",
-117, "Error ",
-116, "Software error - program stopped",
-115, "Disk corrupt - task stopped",
-114, " Finish ALL disk activity",
-113, " before selecting 'Reboot'",
-112, "in device %s%s",
-111, "in unit %ld%s",
-110, "You MUST replace volume",
-109, "has a read/write error",
-108, "No disk present",
-107, "Not a DOS disk",
-106, "in any drive",
-105, "Please replace volume",
-104, "Please insert volume",
-103, "is full",
-102, "is write protected",
-101, "is not validated",
-100, "Volume",

 103, "not enough memory available",

 105, "process table full",

 114, "bad template",
 115, "bad number",
 116, "required argument missing",
 117, "argument after '=' missing",
 118, "too many arguments",
 119, "unmatched quotes",
 120, "argument line invalid or too long",
 121, "file is not executable",
 122, "invalid resident library",

 202, "object is in use",
 203, "object already exists",
 204, "directory not found",
 205, "object not found",
 206, "invalid window description",
 207, "object too large",

 209, "packet request type unknown",
 210, "object name invalid",
 211, "invalid object lock",
 212, "object is not of required type",
 213, "disk not validated",
 214, "disk is write-protected",
 215, "rename across devices attempted",
 216, "directory not empty",
 217, "too many levels",
 218, "device (or volume) is not mounted",
 219, "seek failure",
 220, "comment is too long",
 221, "disk is full",
 222, "object is protected from deletion",
 223, "file is write protected",
 224, "file is read protected",
 225, "not a valid DOS disk",
 226, "no disk in drive",

 232, "no more entries in directory",
 233, "object is soft link",
 234, "object is linked",

 240, "record not locked",
 241, "record lock collision",
 242, "record lock timeout",
 243, "record unlock error",

 303, "buffer overflow",
 304, "***Break",
 305, "file not executable",

 0,NULL,
};

#endif
