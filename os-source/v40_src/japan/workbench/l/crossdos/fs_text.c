/* FS_text.c **************************************************************
** Copyright 1991 CONSULTRON
*
*      File System text used in other source files
*
*************************************************************************/

//#include "FS:FS.h"
#include "intuition/intuition.h"

char DeviceName[32];

/* Global EasyStruct for all (almost) requesters */
struct EasyStruct ERS =
{
sizeof (struct EasyStruct),
0,
0,
0,
"%s",
};

char *disk_warn = "Disk Warning!";
