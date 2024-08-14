#ifndef WORLDMAP_H
#define WORLDMAP_H

/*****************************************************************************/

#include "pe_custom.h"
#include "bitmapimage.h"

/*****************************************************************************/

/* position of world map in window */
#define WM_LEFT   203	/* Was 193 */
#define WM_TOP    113
#define WM_WIDTH  342	/* Was 342 */
#define WM_HEIGHT 100

#define WM_SCROLLERWIDTH  18
#define WM_IMAGEWIDTH     323	/* was 322 */
#define WM_IMAGEHEIGHT    172

/*****************************************************************************/

#define	MAX_ZONES	33

/*****************************************************************************/

extern struct BitMap World;
extern struct BitMapImage WorldI;
extern struct BitMap Masks;
extern struct BitMapImage MasksI;
extern struct Images Masks_images[];

/*****************************************************************************/

VOID PutMap(EdDataPtr ed);
VOID PickTimeZone(EdDataPtr ed, USHORT x, USHORT y);

/*****************************************************************************/

#endif /* WORLDMAP_H */
