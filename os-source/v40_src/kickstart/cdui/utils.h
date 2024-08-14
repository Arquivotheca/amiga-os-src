#ifndef UTILS_H
#define UTILS_H


/*****************************************************************************/


#include <exec/types.h>


/*****************************************************************************/


/* topaz-8 */
extern struct TextAttr topazAttr;


/* fade the display from black to the colors in the library base */
VOID FadeIn(struct CDUILib *CDUIBase);

/* fade the display from the colors in the library base to black */
VOID FadeOut(struct CDUILib *CDUIBase);

VOID GetColors(struct CDUILib *CDUIBase, APTR backData, APTR spriteData);

VOID GetBM(struct CDUILib *CDUIBase, APTR deboxData,
           struct BitMap **resultBM, struct BMInfo **resultBMInfo);

VOID FreeBM(struct CDUILib *CDUIBase, struct BitMap *bitMap, struct BMInfo *bmInfo);

VOID WaitBeam(struct CDUILib *CDUIBase, ULONG pos);


VOID PlaySample(struct CDUILib *CDUIBase, UBYTE *data);


VOID CloseScreenQuiet(struct CDUILib *CDUIBase, struct Screen *screen);


VOID SyncSignal(struct CDUILib *CDUIBase, struct Task *task, ULONG sigMask);


/*****************************************************************************/


#endif  /* UTILS_H */
