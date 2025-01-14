/*
**	$Header: V38:src/workbench/libs/diskfont/RCS/strings.c,v 37.0 90/11/26 12:14:12 kodiak Exp $
**
**      diskfont.library string support
**
**      (C) Copyright 1989 Commodore-Amiga, Inc.
**          All Rights Reserved
*/
#include	<exec/types.h>

unsigned char FoldC(ch)
unsigned char ch;
{
    if (((ch >= 0x61) && (ch <= 0x7a)) ||
	    ((ch >= 0xe0) && (ch != 0xf7) && (ch != 0xff)))
	return((unsigned char) (ch & 0xdf));
    return(ch);
}

int StrEquNC(src1, src2)
unsigned char *src1, *src2;
{
    unsigned char c1;

    while (c1 = *src1++) {
	if (FoldC(c1) != FoldC(*src2++))
	    return(FALSE);
    }
    if (*src2)
	return(FALSE);
    return(TRUE);
}
