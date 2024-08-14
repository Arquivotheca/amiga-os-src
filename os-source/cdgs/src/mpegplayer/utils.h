#ifndef UTILS_H
#define UTILS_H


/*****************************************************************************/


#include <exec/types.h>


/*****************************************************************************/


VOID ShadowText(struct MPEGPlayerLib *MPEGPlayerBase, struct RastPort *rp, STRPTR text, UWORD len, struct Rectangle *constraint);
VOID ClipText(struct MPEGPlayerLib *MPEGPlayerBase, struct RastPort *rp, STRPTR text, UWORD len, UWORD max);
VOID CenterText(struct MPEGPlayerLib *MPEGPlayerBase, struct RastPort *rp, STRPTR text, UWORD len, UWORD max);
UWORD ShadowTextLength(struct MPEGPlayerLib *MPEGPlayerBase, struct RastPort *rp, STRPTR text, UWORD len);
UWORD ShadowFit(struct MPEGPlayerLib *MPEGPlayerBase, struct RastPort *rp, STRPTR text, UWORD len, UWORD max);

VOID FadeIn(struct MPEGPlayerLib *MPEGPlayerBase, BOOL midWay);
VOID FadeOut(struct MPEGPlayerLib *MPEGPlayerBase, BOOL midWay);
VOID CloseScreenQuiet(struct MPEGPlayerLib *MPEGPlayerBase, struct Screen *screen);

void SwapBuffers(struct MPEGPlayerLib *MPEGPlayerBase);

// same as sprintf(), but with a length limit
LONG nprintf(STRPTR buffer, LONG maxLen, STRPTR template, ...);

VOID GetBM(struct MPEGPlayerLib *MPEGPlayerBase, APTR deboxData, struct BitMap **resultBM, struct BMInfo **resultBMInfo);

void CloseWindowSafely(struct MPEGPlayerLib *MPEGPlayerBase, struct Window *wp);

UWORD Random(WORD range, ULONG *seed);


/*****************************************************************************/


#endif  /* UTILS_H */
