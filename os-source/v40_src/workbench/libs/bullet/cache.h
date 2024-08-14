/* cache.h */
/* Copyright (C) 1989, 1990 by Agfa Compugraphic, Inc. All rights reserved. */
/*
 *    05-Aug-90  awr   added bitMapCount in FONTCONTEXT to improve cache
 *                     performance. If last bitmap is freed from a font
 *                     and it's not the current font, then the font is freed.
 */
/*----------------------*/
/*      Font            */
/*----------------------*/

#define SSMAX         256

typedef struct _FONT
{
    struct MinNode mn;    /* doubly linked list pointers- this slot may be
                           * in an active least recently used list or it may
                           * be unused in the avail list.
                           */
    UWORD bitmapCount;    /* Number of bitmaps in this font */
    FONTCONTEXT fc;
    IFBITMAP *buf[SSMAX];  /* buffer handles of cached characters */
}
FONT;
