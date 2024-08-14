/* Prototypes for functions defined in
isupp2.c
 */

int stretchIFrame(struct Point delta);

int dragIFrame(struct Point delta);

BOOL isTick(struct InputEvent * ie);

BOOL isSelectup(struct InputEvent * ie);

int drawFrames(struct RastPort * RP,
               struct Rectangle * oldrect,
               struct Rectangle * newrect);

